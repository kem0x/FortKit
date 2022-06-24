#pragma once
#include "../FortKitLauncher/util.h"

class FileWriter
{
    FILE* m_File;

public:
    FileWriter()
    {
    }

    FileWriter(const char* FileName)
    {
        fopen_s(&m_File, FileName, "wb");
    }

    ~FileWriter()
    {
        std::fclose(m_File);
    }

    void WriteString(std::string String)
    {
        std::fwrite(String.c_str(), String.length(), 1, m_File);
    }

    template <typename T>
    void Write(T Input)
    {
        std::fwrite(&Input, sizeof(T), 1, m_File);
    }

    void Seek(int Pos, int Origin = SEEK_CUR)
    {
        std::fseek(m_File, Pos, Origin);
    }

    uint32_t Size()
    {
        auto pos = std::ftell(m_File);
        std::fseek(m_File, 0, SEEK_END);
        auto ret = std::ftell(m_File);
        std::fseek(m_File, pos, SEEK_SET);
        return ret;
    }

    bool Compress(const std::string file_path, const std::string algorithm)
    {
        TCHAR n_path[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, n_path);

        const LPPROCESS_INFORMATION process_information = {nullptr};

        std::wstring args;
        args.append((wchar_t*)"-compression=");
        args.append((wchar_t*)algorithm.c_str());
        args.append((wchar_t*)" -path=");
        args.append(n_path);

        BOOL result = CreateProcessW(LPWSTR(file_path.c_str()), LPWSTR(args.c_str()), nullptr, nullptr, FALSE,
                                     NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, nullptr, nullptr, nullptr,
                                     process_information);

        if (result)
        {
            WaitForSingleObject(process_information->hProcess, INFINITE);

            LPDWORD exit_code;

            result = GetExitCodeProcess(process_information->hProcess, reinterpret_cast<LPDWORD>(&exit_code));

            CloseHandle(process_information->hProcess);
            CloseHandle(process_information->hThread);

            if (!result)
            {
                printfc(FOREGROUND_RED, "[x] Could not get exit code of compressor\n")
                return false;
            }
        }
        else
        {
            printfc(FOREGROUND_RED, "[x] An error occurred with the compressor\n")
            return false;
        }

        return true;
    }
};
