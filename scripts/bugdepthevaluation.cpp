//
// This tool prints some metrics such as bug depth (function and conditional branches)
//

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
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

    COUNTER_BRANCH() : _branch(0), _taken(0)  {}

};

class COUNTER_CALL
{
  public:
    int _call;
    int _ret;

    COUNTER_CALL() : _call(0)  {}

};


// variable for conditional branches metric
std::map<ADDRINT, COUNTER_BRANCH> branchesCounter;
static int uniqbranchcount = 0;
static int branchcount = 0;

// variable for call graph metric
std::map<ADDRINT, COUNTER_CALL> callCounter;
static int uniqcallcount = 1; // 1 because of the main function
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
    if(CheckBounds(addr)) {

        branchesCounter[addr]._branch++;
        if (taken)
	        branchesCounter[addr]._taken++; 
    }
}

// This function is called before every call instruction is executed
VOID CallCount(ADDRINT addr, BOOL IsCall)
{
    if(CheckBounds(addr)) {
        if (IsCall) {
            callCounter[addr]._call++;

        } else {
            callCounter[addr]._ret--;
        }
    }
}

VOID Instruction(INS ins, VOID *v)
{
    // Condition to insert a call (conditional branch and in the "main" executable)
	if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {

   		// Insert a call to BranchCount before every branch
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchCount, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_END);
	}

    // Condition to insert a call (call instruction or ret instruction)
    if (INS_IsCall(ins)) {

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_BOOL, true, IARG_END);
        
    } else if (INS_IsRet(ins)) {

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_BOOL, false, IARG_END);
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
                    << " => taken count: "  << it->second._taken << "\n";

            if (it->second._branch == 1) {
                uniqbranchcount++;
            }

            branchcount += it->second._branch;
        }
    }

    for (std::map<ADDRINT, COUNTER_CALL>::iterator it=callCounter.begin(); it!=callCounter.end(); ++it) {
        TraceFile << "\n" << "call/ret address: 0x" << it->first
                << " => call count: " << it->second._call
                << " => ret count: "  << it->second._ret;

        uniqcallcount += it->second._call + it->second._ret;

        callcount += it->second._call;
    }

    TraceFile << "\n" << "\n" 
                << "Number of unique conditional branches = " << uniqbranchcount << " (depth)" << "\n"
                << "Number of all conditional branches = " << branchcount << "\n";

    TraceFile   << "Number of unique call instruction (call and not ret) = " << uniqcallcount << " (depth)" << "\n"
                << "Number of all call instruction = " << callcount << "\n" << endl;

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