#pragma once

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
};
