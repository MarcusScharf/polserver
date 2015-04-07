#include "ExceptionParser.h"
#include "LogSink.h"

#include "plib/systemstate.h"
#include "plib/polver.h"

#include <cstring>
#include <signal.h>

#ifndef _WIN32
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

#define MAX_STACK_TRACE_DEPTH          200
#define MAX_STACK_TRACE_STEP_LENGTH    256

namespace Pol{ namespace Clib{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

void GetSignalDescription(int pSignal, string &pSignalName, string &pSignalDescription)
{
    switch(pSignal)
    {
        case 1:
            pSignalName = "SIGHUP";
            pSignalDescription = "hangup detected on controlling terminal or death of controlling process";
            break;
        case 2:
            pSignalName = "SIGINT";
            pSignalDescription = "interrupt from keyboard";
            break;
        case 3:
            pSignalName = "SIGQUIT";
            pSignalDescription = "quit from keyboard";
            break;
        case 4:
            pSignalName = "SIGILL";
            pSignalDescription = "illegal Instruction";
            break;
        case 6:
            pSignalName = "SIGABRT";
            pSignalDescription = "abort signal from abort()";
            break;
        case 8:
            pSignalName = "SIGFPE";
            pSignalDescription = "floating point exception";
            break;
        case 9:
            pSignalName = "SIGKILL";
            pSignalDescription = "kill signal";
            break;
        case 10:
            pSignalName = "SIGBUS";
            pSignalDescription = "bus error";
            break;
        case 11:
            pSignalName = "SIGSEGV";
            pSignalDescription = "invalid memory reference";
            break;
        case 12:
            pSignalName = "SIGSYS";
            pSignalDescription = "bad argument to system call";
            break;
        case 13:
            pSignalName = "SIGPIPE";
            pSignalDescription = "broken pipe: write to pipe with no readers";
            break;
        case 14:
            pSignalName = "SIGALRM";
            pSignalDescription = "timer signal from alarm()";
            break;
        case 15:
            pSignalName = "SIGTERM";
            pSignalDescription = "termination signal";
            break;
        case 18:
            pSignalName = "SIGCONT";
            pSignalDescription = "continue signal from tty";
            break;
        case 19:
            pSignalName = "SIGSTOP";
            pSignalDescription = "stop signal from tty";
            break;
        case 20:
            pSignalName = "SIGTSTP";
            pSignalDescription = "stop signal from user (keyboard)";
            break;
        case 16:
        case 30:
            pSignalName = "SIGUSR1";
            pSignalDescription = "user-defined signal 1";
            break;
        case 17:
        case 31:
            pSignalName = "SIGUSR2";
            pSignalDescription = "user-defined signal 2";
            break;
        default:
            pSignalName = "unsupported signal";
            pSignalDescription = "unsupported signal occurred";
            break;
    }
}

void LogExceptionSignal(int pSignal)
{
    string tSignalName;
    string tSignalDescription;

    GetSignalDescription(pSignal, tSignalName, tSignalDescription);
    printf("Signal \"%s\"(%d: %s) detected.\n", tSignalName.c_str(), pSignal, tSignalDescription.c_str());
}

std::string getCompilerVersion()
{
	#ifndef _WIN32
		char result[256];
		sprintf(result, "gcc %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	#else
		std::string result;
		switch(_MSC_VER)
		{
		case 1800:
			result = "MSVC++ 12.0 (Visual Studio 2013)";
			break;
		case 1700:
			result = "MSVC++ 11.0 (Visual Studio 2012)";
			break;
		case 1600:
			result = "MSVC++ 10.0 (Visual Studio 2010)";
			break;
		case 1500:
			result = "MSVC++ 9.0 (Visual Studio 2008)";
			break;
		case 1400:
			result = "MSVC++ 8.0 (Visual Studio 2005)";
			break;
		case 1310:
			result = "MSVC++ 7.1 (Visual Studio 2003)";
			break;
		case 1300:
			result = "MSVC++ 7.0";
			break;
		case 1200:
			result = "MSVC++ 6.0";
			break;
		case 1100:
			result = "MSVC++ 5.0";
			break;
		default:
			if(_MSC_VER > 1800)
				result = "MSVC++ newer than version 12.0"
			else if (_MSC_VER < 1100)
				result = "MSVC++ older than version 5.0"
			else
				result = "MSVC++ (some unsupported version)";
			break;
		}
	#endif

	return result;
}

void HandleExceptionSignal(int pSignal)
{
    switch(pSignal)
    {
        case SIGILL:
        case SIGFPE:
        case SIGSEGV:
        case SIGTERM:
        case SIGABRT:
            {
                printf("########################################################################################\n");
                printf("POL will exit now. Please post the following on http://forums.polserver.com/tracker.php.\n");
                string tStackTrace = ExceptionParser::GetTrace();
                printf("Executable: %s\n", Pol::Plib::systemstate.executable.c_str());
                printf("Start time: %s\n", Pol::Plib::systemstate.getStartTime().c_str());
                printf("Current time: %s\n", Pol::Clib::Logging::LogSink::GetTimeStamp().c_str());
                printf("\n");
                printf("Stack trace:\n%s", tStackTrace.c_str());
                printf("\n");
				printf("Compiler: %s", getCompilerVersion().c_str());
                printf("Compile time: %s\n", Pol::compiletime);
                printf("Build target: %s\n", Pol::polbuildtag);
                printf("Build revision: %s\n", Pol::polverstr);
				#ifndef _WIN32
					printf("GNU C library (compile time): %d.%d\n", __GLIBC__, __GLIBC_MINOR__);
				#endif
                printf("\n");
                printf("########################################################################################\n");
                exit(1);
            }
            break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

ExceptionParser::ExceptionParser()
{
}

ExceptionParser::~ExceptionParser()
{
}

///////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
string ExceptionParser::GetTrace()
{
    string tResult;

    void *tStackTrace[MAX_STACK_TRACE_DEPTH];
    int tStackTraceSize;
    char **tStackTraceList;
    int tStackTraceStep = 0;
    char *tStringBuf = (char*)malloc(MAX_STACK_TRACE_STEP_LENGTH);

    tStackTraceSize = backtrace(tStackTrace, MAX_STACK_TRACE_DEPTH);
    tStackTraceList = backtrace_symbols(tStackTrace, tStackTraceSize);

    size_t tFuncNameSize = 256;
    char* tFuncnName = (char*)malloc(tFuncNameSize);

    // iterate over all entries and do the demangling
    for ( int i = 0; i < tStackTraceSize; i++ )
    {
        // get the pointers to the name, offset and end of offset
        char *tBeginFuncName = 0;
        char *tBeginFuncOffset = 0;
        char *tEndFuncOffset = 0;
        char *tBeginBinaryName = tStackTraceList[i];
        char *tBeginBinaryOffset = 0;
        char *tEndBinaryOffset = 0;
        for (char *tEntryPointer = tStackTraceList[i]; *tEntryPointer; ++tEntryPointer)
        {
            if (*tEntryPointer == '(')
            {
                tBeginFuncName = tEntryPointer;
            }else if (*tEntryPointer == '+')
            {
                tBeginFuncOffset = tEntryPointer;
            }else if (*tEntryPointer == ')' && tBeginFuncOffset)
            {
                tEndFuncOffset = tEntryPointer;
            }else if (*tEntryPointer == '[')
            {
                tBeginBinaryOffset = tEntryPointer;
            }else if (*tEntryPointer == ']' && tBeginBinaryOffset)
            {
                tEndBinaryOffset = tEntryPointer;
                break;
            }
        }

        // set the default value for the output line
        sprintf(tStringBuf, "\n");

        // get the detailed values for the output line
        if (tBeginFuncName && tBeginFuncOffset && tEndFuncOffset && tBeginFuncName < tBeginFuncOffset)
        {
            // terminate the C strings
            *tBeginFuncName++ = '\0';
            *tBeginFuncOffset++ = '\0';
            *tEndFuncOffset = '\0';
            *tBeginBinaryOffset++ = '\0';
            *tEndBinaryOffset = '\0';

            int tRes;
            tFuncnName = abi::__cxa_demangle(tBeginFuncName, tFuncnName, &tFuncNameSize, &tRes);
            unsigned int tBinaryOffset = strtoul(tBeginBinaryOffset, NULL, 16);
            if (tRes == 0)
            {
                if(tBeginBinaryName && strlen(tBeginBinaryName))
                    sprintf(tStringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", tStackTraceStep, tBinaryOffset, tFuncnName, tBeginFuncOffset, tBeginBinaryName);
                else
                    sprintf(tStringBuf, "#%02d 0x%016x in %s from %s\n", tStackTraceStep, tBinaryOffset, tFuncnName, tBeginFuncOffset);
                tStackTraceStep++;
            }else{
                if(tBeginBinaryName && strlen(tBeginBinaryName))
                    sprintf(tStringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", tStackTraceStep, tBinaryOffset, tBeginFuncName, tBeginFuncOffset, tBeginBinaryName);
                else
                    sprintf(tStringBuf, "#%02d 0x%016x in %s:[%s]\n", tStackTraceStep, tBinaryOffset, tBeginFuncName, tBeginFuncOffset);
                tStackTraceStep++;
            }
        }else{
            sprintf(tStringBuf, "#%02d %s\n", tStackTraceStep, tStackTraceList[i]);
            tStackTraceStep++;
        }

        // append the line to the result
        tResult += string(tStringBuf);
    }

    // memory cleanup
    free(tFuncnName);
    free(tStackTraceList);

    return tResult;
}

static void HandleSignalLinux(int pSignal, siginfo_t *pSignalInfo, void *pArg)
{
    LogExceptionSignal(pSignal);
    if (pSignalInfo != NULL)
    {
        if(pSignal == SIGSEGV)
        {
            if(pSignalInfo->si_addr != NULL)
                printf("Segmentation fault detected - faulty memory reference at location: %p\n", pSignalInfo->si_addr);
            else
                printf("Segmentation fault detected - null pointer reference\n");
        }
        if (pSignalInfo->si_errno != 0)
            printf("This signal occurred because \"%s\"(%d)\n", strerror(pSignalInfo->si_errno), pSignalInfo->si_errno);
        if (pSignalInfo->si_code != 0)
            printf("Signal code is %d\n", pSignalInfo->si_code);
    }
    HandleExceptionSignal(pSignal);
}

void ExceptionParser::InitGlobalExceptionCatching()
{
    // set handler
    struct sigaction tSigAction;
    memset(&tSigAction, 0, sizeof(tSigAction));
    sigemptyset(&tSigAction.sa_mask);
    tSigAction.sa_sigaction = HandleSignalLinux;
    tSigAction.sa_flags   = SA_SIGINFO; // Invoke signal-catching function with three arguments instead of one
    sigaction(SIGINT, &tSigAction, NULL);
    sigaction(SIGTERM, &tSigAction, NULL);
    sigaction(SIGSEGV, &tSigAction, NULL);

    // set handler stack
    stack_t tStack;
    tStack.ss_sp = malloc(SIGSTKSZ);
    if (tStack.ss_sp == NULL)
    {
        printf("Could not allocate signal handler stack\n");
        exit(1);
    }
    tStack.ss_size = SIGSTKSZ;
    tStack.ss_flags = 0;
    if (sigaltstack(&tStack, NULL) == -1)
    {
        printf("Could not set signal handler stack\n");
        exit(1);
    }
}
#else // _WIN32
string ExceptionParser::GetTrace()
{
    string tResult;

    return tResult;
}

void ExceptionParser::InitGlobalExceptionCatching()
{

}
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////

}} // namespaces
