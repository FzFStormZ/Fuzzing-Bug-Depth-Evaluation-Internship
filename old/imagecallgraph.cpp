//
// This tool prints a trace of image load, image limits of main executable and the number of conditional branches
//

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stack>
using std::ofstream;
using std::string;
using std::endl;
using std::stack;


KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "imagecallgraph.out", "specify file name");

ofstream TraceFile;

class COUNTER
{
  public:
    UINT64 _branch;
    UINT64 _taken;

    COUNTER() : _branch(0), _taken(0)  {}

};

std::map<ADDRINT, COUNTER> counterBranches;

// The running count of branches is kept here
//static UINT64 uniqbranchcount = 0;
//static UINT64 branchcount = 0;

// Call graph utilities
//static std::stack<INS> callstack;
static UINT64 callcount = 0;

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
VOID CallCount(ADDRINT addr, BOOL IsCall)
{
    if(CheckBounds(addr)) {
        if (IsCall) {
            callcount++;

        } else {
            callcount--;
        }
    }
}

VOID Instruction(INS ins, VOID *v) 
{

    if (INS_IsCall(ins)) {

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_BOOL, true, IARG_END);
        
    } else if (INS_IsRet(ins)) {

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CallCount, IARG_INST_PTR, IARG_BOOL, false, IARG_END);
    }


    /*
    if (INS_IsCall(ins) || INS_IsRet(ins)) {
        if (!callstack.empty()) {
            if (INS_IsCall(callstack.top())) {

            // Insert a call to BranchCount before every branch
            //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)callCount, IARG_INST_PTR, IARG_END);
            callstack.push(ins);

            } else {

                if (!callstack.empty()) {
                    // remove last call because RET instruction
                    //INS_Delete(callstack.top());
                    callstack.pop();
                }
                
            }

        } else {
            callstack.push(ins);
        }
        
    }
    */
}

// This function is called when the application exits
// It closes the output file.
VOID Fini(INT32 code, VOID *v)
{

    TraceFile << "\n" << "Call depth = " << callcount << "\n";

    if (TraceFile.is_open()) { TraceFile.close(); }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of image load, image limits of main executable and the number of conditional branches\n"
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