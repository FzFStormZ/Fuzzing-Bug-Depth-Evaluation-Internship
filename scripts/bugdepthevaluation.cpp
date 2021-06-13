//
// This tool prints some metrics such as bug depth (function and conditional branches)
//

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <utility>
#include <list>
#include <algorithm>

using std::ofstream;
using std::string;
using std::endl;


KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "bugdepthevaluation.out", "specify file name");

ofstream TraceFile;

class COUNTER_BRANCH
{
  public:
    int _branch;
    int _taken;
    bool _complexity;

    COUNTER_BRANCH() : _branch(0), _taken(0), _complexity(false)  {}

};

class COUNTER_CALL
{
  public:
    int _call;
    int _ret;

    COUNTER_CALL() : _call(0), _ret(0)  {}

};

// Registers reference (Jonathan Salwan) but some modifications for my case
struct regRef
{
  std::string       name;
  LEVEL_BASE::REG   ref;
};

static struct regRef regsRef[] =
{
  {"rax", LEVEL_BASE::REG_RAX},
  {"rcx", LEVEL_BASE::REG_RCX},
  {"rdx", LEVEL_BASE::REG_RDX},
  {"eax", LEVEL_BASE::REG_RAX},
  {"ecx", LEVEL_BASE::REG_RCX},
  {"edx", LEVEL_BASE::REG_RDX},
  {"ah",  LEVEL_BASE::REG_RAX},
  {"ch",  LEVEL_BASE::REG_RCX},
  {"dh",  LEVEL_BASE::REG_RDX},
  {"al",  LEVEL_BASE::REG_RAX},
  {"cl",  LEVEL_BASE::REG_RCX},
  {"dl",  LEVEL_BASE::REG_RDX},
  {"",    REG_INVALID()}
};

// variable for conditional branches metric
std::map<ADDRINT, COUNTER_BRANCH> branchesCounter;
static int depthbranchcount = 0;
static int branchcount = 0;
static int complexitycount = 0;

// variable for call graph metric
std::map<std::pair<ADDRINT, ADDRINT>, COUNTER_CALL> callCounter;
std::list<ADDRINT> targetAddressFunction;
static int depthcallcount = 1; // 1 because of the main function
static int uniqcallcount = 0;
static int callcount = 0;

// The IMG binary
IMG MainBinary;

const char * StripPath(const char * path)
{
	const char * file = strrchr(path,'/');
	if (file)
		return file+1;
	else
		return path;
}

// Checks if the instruction comes from the binary being instrumented.
BOOL CheckBounds(ADDRINT addr) {
    if(addr < IMG_HighAddress(MainBinary) && addr > IMG_LowAddress(MainBinary)){
        return true;
    }
    return false;
}

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily
VOID Image(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img)) {
	    MainBinary = img;

        TraceFile << "[*] Main Binary Image: " << IMG_Name(img) << endl;
        TraceFile << "[+] Image limits 0x" << IMG_LowAddress(img) << " - 0x" << IMG_HighAddress(img) << "\n" << endl;
    }
}

// This function is called before every branch is executed
VOID BranchCount(ADDRINT addr, BOOL taken)
{
    branchesCounter[addr]._branch++;
    if (taken)
        branchesCounter[addr]._taken++; 
}

// This function is called before every call instruction is executed
VOID CallCount(ADDRINT addr, ADDRINT addrTarget, BOOL IsCall)
{

    std::pair<ADDRINT, ADDRINT> call (addr, addrTarget);

    if (IsCall) {
        
        callCounter[call]._call++;

    } else {
        callCounter[call]._ret++;
    }
}

// This function is called before every we have a register in the CMP/TEST instruction before a conditional branch
VOID ComplexityBranchCountREG(ADDRINT addr, CONTEXT *ctxt, string diss)
{

    // Get the name of the register
    std::string delimiter = ",";
    std::string token = diss.substr(diss.find(delimiter) + 2);

    // Reference to our registers reference dict
    for (int i = 0; !(regsRef[i].name.empty()); i++) {
        
        if (regsRef[i].name == token.c_str()){

            // Get the register value
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(8) << PIN_GetContextReg(ctxt, regsRef[i].ref);
            std::string reg_value = ss.str();

            int nb_non_zero_byte = 0;

            for (int i = 0; reg_value[i] != '\0'; i++) {
                if (reg_value[i] != '0') {
                    nb_non_zero_byte++;
                }
            }

            // About the complexity of the conditional branch
            if (nb_non_zero_byte <= 2) {
                branchesCounter[addr]._complexity = false;
            } else {
                branchesCounter[addr]._complexity = true;
            }

            nb_non_zero_byte = 0;
        
            break;
        }
    }
}

// This function is called before every we have an immediate value in the CMP/TEST instruction before a conditional branch
VOID ComplexityBranchCountImmediate(ADDRINT addr, UINT64 value)
{
    // Get the immediate value
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << value;
    std::string im_value = ss.str();

    int nb_non_zero_byte = 0;

    for (int i = 0; im_value[i] != '\0'; i++) {
        if (im_value[i] != '0') {
            nb_non_zero_byte++;
        }
    }

    // About the complexity of the conditional branch
    if (nb_non_zero_byte <= 2) {
        branchesCounter[addr]._complexity = false;
    } else {
        branchesCounter[addr]._complexity = true;
    }

    nb_non_zero_byte = 0;
}

VOID Instruction(INS ins, VOID *v)
{

    if (CheckBounds(INS_Address(ins))) {
        
        // Condition to insert a call (conditional branch and in the "main" executable)
        if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {

            // Insert a call to BranchCount before every branch
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchCount, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_END);

            // Get the last CMP or TEST instruction of the current conditional branch
            INS last_comparaison = INS_Prev(ins);
            while(INS_Invalid() != last_comparaison) {

                if (INS_Opcode(last_comparaison) == XED_ICLASS_CMP || INS_Opcode(last_comparaison) == XED_ICLASS_TEST) {
                    break;
                }

                last_comparaison = INS_Prev(last_comparaison);
            }

            // Instrument CMP/TEST instruction for the complexity of the conditional branch
            if (INS_Valid(last_comparaison)) {

                if (INS_OperandIsReg(last_comparaison, 1)) {

                    INS_InsertCall(last_comparaison, IPOINT_BEFORE, (AFUNPTR)ComplexityBranchCountREG,
                                    IARG_ADDRINT, INS_Address(ins),
                                    IARG_CONST_CONTEXT,
                                    IARG_PTR, new string(INS_Disassemble(last_comparaison)), 
                                    IARG_END);
                                        
                } else if (INS_OperandIsImmediate(last_comparaison, 1)) {

                    int value = INS_OperandImmediate(last_comparaison, 1);
                    
                    INS_InsertCall(last_comparaison, IPOINT_BEFORE, (AFUNPTR)ComplexityBranchCountImmediate, 
                                    IARG_ADDRINT, INS_Address(ins), 
                                    IARG_UINT64, value, 
                                    IARG_END);
                
                }
            }
        }

        // Condition to insert a call (call instruction or ret instruction)
        if (INS_IsCall(ins)) {

            if (INS_IsDirectControlFlow(ins)) {

                ADDRINT addrTarget = INS_DirectControlFlowTargetAddress(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, 
                                IARG_INST_PTR, 
                                IARG_ADDRINT, addrTarget, 
                                IARG_BOOL, true, 
                                IARG_END);

            } else {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, 
                                IARG_INST_PTR, 
                                IARG_ADDRINT, 0, 
                                IARG_BOOL, true, 
                                IARG_END);
            }
            
        } else if (INS_IsRet(ins)) {

            if (INS_IsDirectControlFlow(ins)) {

                ADDRINT addrTarget = INS_DirectControlFlowTargetAddress(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, 
                                IARG_INST_PTR, 
                                IARG_ADDRINT, addrTarget, 
                                IARG_BOOL, false, 
                                IARG_END);
                
            } else {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, 
                                IARG_INST_PTR, 
                                IARG_ADDRINT, 0, 
                                IARG_BOOL, false, 
                                IARG_END);
            }
        }
    }
}

// This function is called when the application exits
// It closes the output file.
VOID Fini(INT32 code, VOID *v)
{

    for (std::map<ADDRINT, COUNTER_BRANCH>::iterator it=branchesCounter.begin(); it!=branchesCounter.end(); ++it) {
        if (it->second._taken != 0) {
            TraceFile << "ins address: 0x" << it->first
                    << " => branch count: " << it->second._branch 
                    << " => taken count: "  << it->second._taken 
                    << " => complexe?: " << (it->second._complexity ? "true" : "false") << "\n";

            if (it->second._branch == 1) {
                depthbranchcount++;
            }

            if (it->second._complexity) {
                complexitycount++;
            }

            branchcount += it->second._branch;
        }
    }

for (std::map<std::pair<ADDRINT, ADDRINT>, COUNTER_CALL>::iterator it=callCounter.begin(); it!=callCounter.end(); ++it) {
        if (it->first.second == 0 && it->second._call != 0) {
            TraceFile << "\n" << "call/ret ins address: 0x" << it->first.first << " - target: 0x" << it->first.second
                << " => call count: " << it->second._call
                << " => ret count: "  << it->second._ret 
                << " (call not taken into account)";

        } else {
            TraceFile << "\n" << "call/ret ins address: 0x" << it->first.first << " - target: 0x" << it->first.second
                << " => call count: " << it->second._call
                << " => ret count: "  << it->second._ret;
        }
        
        depthcallcount += it->second._call - it->second._ret;

        // Avoid function without target address
        if (it->first.second != 0 && std::find(targetAddressFunction.begin(), targetAddressFunction.end(), it->first.second) == targetAddressFunction.end()) {
            targetAddressFunction.push_back(it->first.second);
            uniqcallcount++;
        }

        if (it->first.second != 0) {
            callcount += it->second._call;
        }
        
    }

    TraceFile << "\n" << "\n" 
                << "The bug depth with conditional branches = " << depthbranchcount << "\n"
                << "Number of all conditional branches = " << branchcount << "\n"
                << "Complexity to reach the bug (in number) = " << complexitycount << "\n";

    TraceFile   << "The bug depth with call instructions = " << depthcallcount << "\n"
                << "Number of unique functions = " << uniqcallcount << "\n"
                << "Number of all call instructions = " << callcount << "\n" << endl;

    if (TraceFile.is_open()) { TraceFile.close(); }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints some metrics such as bug depth (function and conditional branches)\n"
             + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    TraceFile.open(KnobOutputFile.Value().c_str());
    
    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(Image, 0);

    // Function
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}