#pragma once
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

void printStack(CONTEXT* ctx)
{
    STACKFRAME64 stack;
    memset(&stack, 0, sizeof(STACKFRAME64));

    auto process = GetCurrentProcess();
    auto thread = GetCurrentThread();

    SymInitialize(process, NULL, TRUE);

    bool result;
    DWORD64 displacement = 0;

    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] { 0 };
    char name[512] { 0 };
    char module[512] { 0 };

    PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)buffer;

    for (ULONG frame = 0;; frame++)
    {
        result = StackWalk64(
            IMAGE_FILE_MACHINE_AMD64,
            process,
            thread,
            &stack,
            ctx,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL);

        if (!result)
            break;

        symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbolInfo->MaxNameLen = MAX_SYM_NAME;
        SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, symbolInfo);

        HMODULE hModule = NULL;
        lstrcpyA(module, "");
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (const wchar_t*)(stack.AddrPC.Offset), &hModule);

        if (hModule != NULL)
            GetModuleFileNameA(hModule, module, 512);

        printf("[%lu] Name: %s - Address: %p  - Module: %s\n", frame, symbolInfo->Name, (void*)symbolInfo->Address, module);
    }
}

int HandlerForCallStack(_EXCEPTION_POINTERS* ex)
{
    printStack(ex->ContextRecord);

    return EXCEPTION_EXECUTE_HANDLER;
}