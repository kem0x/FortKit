#include "consts.h"
#include "dumper.h"
#include "framework.h"
#include "memory.h"
#include "memcury.h"
#include "exceptions.h"

static void Main(HMODULE hModule)
{
    Util::SetupConsole();

    Memcury::Safety::SetExceptionMode<Memcury::Safety::ExceptionMode::CatchDllExceptionsOnly>();

    auto Start = std::chrono::steady_clock::now();

    GObjects = Memcury::Scanner::FindPattern(Patterns::GObjects)
                   .RelativeOffset(3)
                   .GetAs<decltype(GObjects)>();

    FNameToString = Memcury::Scanner::FindStringRef(StringRefs::FNameToString)
                        .ScanFor({ Memcury::ASM::CALL }, false)
                        .RelativeOffset(1)
                        .GetAs<decltype(FNameToString)>();

    auto End = std::chrono::steady_clock::now();

    printf("[=] Init Time: %.02f ms\n", (End - Start).count() / 1000000.);

    Dumper::Dump();
    Dumper::GenerateUsmap();

    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        __try
        {
            Main(hModule);
        }
        __except (HandlerForCallStack(GetExceptionInformation()))
        {
            MessageBoxA(nullptr, "Main function crashed!", "Error", MB_ICONERROR | MB_OK);
        }
        break;
    }
    default:
        break;
    }
    return TRUE;
}
