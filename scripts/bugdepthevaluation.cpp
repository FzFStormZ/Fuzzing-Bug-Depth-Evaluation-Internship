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
    int _complexity;

    COUNTER_BRANCH() : _branch(0), _taken(0), _complexity(0)  {}

};

class COUNTER_CALL
{
  public:
    int _call;
    int _ret;

    COUNTER_CALL() : _call(0), _ret(0)  {}

};

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

// This function is called before every branch is executed
VOID ComplexityBranchCountREG(ADDRINT addr, CONTEXT *ctxt, string diss, REG reg, ADDRINT *edx)
{

    /*
    UINT32 reg_val; 
    //PIN_GetContextRegval(ctxt, LEVEL_BASE::REG_RAX, &reg_val);
    reg_val = PIN_GetContextReg(ctxt, *reg);

    //std::cout << std::hex << "REG_RAX: 0x" << reg_val << std::endl;

    //ADDRINT *addr_ptr = (ADDRINT*)addr;
    //UINT64 value;
    //PIN_SafeCopy(&value, addr_ptr, sizeof(UINT64));

    printf("diss: %s\n", diss.c_str());
    printf("RAX = %08x \n", reg_val);
    //branchesCounter[addr]._complexity = reg_val;
    */

    std::string delimiter = ",";
    std::string token = diss.substr(diss.find(delimiter) + 2);

    for (int i = 0; !(regsRef[i].name.empty()); i++) {
        

        if (regsRef[i].name == token.c_str()){

            printf("diss: %s\n", token.c_str());
            std::cout << std::hex << std::internal << std::setfill('0') 
            << token.c_str() << " = " << std::setw(16) << PIN_GetContextReg(ctxt, regsRef[i].ref) << std::endl;

            break;
        }
    }

    

}

// This function is called before every branch is executed
VOID ComplexityBranchCountImmediate(ADDRINT addr, UINT64 value)
{
    branchesCounter[addr]._complexity = value;
}

VOID Instruction(INS ins, VOID *v)
{
    if (CheckBounds(INS_Address(ins))) {
        // Condition to insert a call (conditional branch and in the "main" executable)
        if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {

            // Insert a call to BranchCount before every branch
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchCount, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_END);

            // Condition to insert a call (CMP or TEST instruction to get the complexity of the previous conditional branch)
            INS branch = INS_Prev(ins);
            if (INS_Invalid() != branch) {

                if (INS_Opcode(branch) == XED_ICLASS_CMP || INS_Opcode(branch) == XED_ICLASS_TEST) {

                    if (INS_OperandIsReg(branch, 1)) {
                        /*
                        REG reg = INS_OperandReg(branch, 1);
                        string diss = INS_Disassemble(branch);

                        printf("diss = %s\n", diss.c_str());
                        printf("REG = %s\n", REG_StringShort(reg).c_str());
                        */

                        INS_InsertCall(branch, IPOINT_BEFORE, (AFUNPTR)ComplexityBranchCountREG,
                                        IARG_ADDRINT, INS_Address(ins),
                                        IARG_CONST_CONTEXT,
                                        IARG_PTR, new string(INS_Disassemble(branch)), 
                                        IARG_REG_CONST_REFERENCE, REG_RAX,
                                        IARG_REG_REFERENCE, REG_RAX,
                                        IARG_END);
                                            
                    } else if (INS_OperandIsImmediate(branch, 1)) {

                        int value = INS_OperandImmediate(branch, 1);
                        
                        INS_InsertCall(branch, IPOINT_BEFORE, (AFUNPTR)ComplexityBranchCountImmediate, 
                                        IARG_ADDRINT, INS_Address(ins), 
                                        IARG_UINT64, value, 
                                        IARG_END);
                    
                    }
                }
            }
        }


        // Condition to insert a call (call instruction or ret instruction)
        if (INS_IsCall(ins)) {

            if (INS_IsDirectControlFlow(ins)) {

                ADDRINT addrTarget = INS_DirectControlFlowTargetAddress(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_ADDRINT, addrTarget, IARG_BOOL, true, IARG_END);

            } else {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_ADDRINT, 0, IARG_BOOL, true, IARG_END);
            }
            
        } else if (INS_IsRet(ins)) {

            if (INS_IsDirectControlFlow(ins)) {

                ADDRINT addrTarget = INS_DirectControlFlowTargetAddress(ins);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_ADDRINT, addrTarget, IARG_BOOL, false, IARG_END);
                
            } else {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_ADDRINT, 0, IARG_BOOL, false, IARG_END);
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
                    << " => operand: 0x" << it->second._complexity << "\n";

            if (it->second._branch == 1) {
                depthbranchcount++;
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
                << "Number of all conditional branches = " << branchcount << "\n";

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