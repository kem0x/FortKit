#pragma once
#include "../FortKitLauncher/util.h"

class IBinaryWriter
{
public:

    virtual __forceinline void WriteString(std::string String) = 0;
    virtual __forceinline void Write(void* Input, size_t Size) = 0;
    virtual __forceinline void Seek(int Pos, int Origin = SEEK_CUR) = 0;
    virtual uint32_t Size() = 0;
};

class StreamWriter : IBinaryWriter
{
    std::stringstream m_Stream;

public:

    __forceinline std::stringstream& GetBuffer()
    {
        return m_Stream;
    }

    __forceinline void WriteString(std::string String) override
    {
        m_Stream.write(String.c_str(), String.size());
    }

    __forceinline void Write(void* Input, size_t Size) override
    {
        m_Stream.write((char*)Input, Size);
    }

    __forceinline void Seek(int Pos, int Origin = SEEK_CUR) override
    {
        m_Stream.seekp(Pos, Origin);
    }

    uint32_t Size() override
    {
        auto pos = m_Stream.tellp();
        this->Seek(0, SEEK_END);
        auto ret = m_Stream.tellp();
        this->Seek(pos, SEEK_SET);

        return ret;
    }

    template <typename T>
    __forceinline void Write(T Input)
    {
        Write(&Input, sizeof(T));
    }
};

class FileWriter : IBinaryWriter
{
    FILE* m_File;

public:
    FileWriter(const char* FileName)
    {
        fopen_s(&m_File, FileName, "wb");
    }

    ~FileWriter()
    {
        std::fclose(m_File);
    }

    __forceinline void WriteString(std::string String) override
    {
        std::fwrite(String.c_str(), String.length(), 1, m_File);
    }

    __forceinline void Write(void* Input, size_t Size) override
    {
        std::fwrite(Input, Size, 1, m_File);
    }

    __forceinline void Seek(int Pos, int Origin = SEEK_CUR) override
    {
        std::fseek(m_File, Pos, Origin);
    }

    uint32_t Size() override
    {
        auto pos = std::ftell(m_File);
        std::fseek(m_File, 0, SEEK_END);
        auto ret = std::ftell(m_File);
        std::fseek(m_File, pos, SEEK_SET);
        return ret;
    }

    template <typename T>
    __forceinline void Write(T Input)
    {
        Write(&Input, sizeof(T));
    }
};
