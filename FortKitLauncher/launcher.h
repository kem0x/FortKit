#pragma once
#include "framework.h"

namespace Launcher
{
	static HANDLE Start()
	{
		const std::string datDir = R"(\Epic\UnrealEngineLauncher\LauncherInstalled.dat)";

		std::string fnPath;

		char programData[MAX_PATH];

		json LauncherInstalled;

		SHGetFolderPathA(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, programData);

		std::string fullDatDir = programData + datDir;

		std::ifstream datStream(fullDatDir);
		if (!datStream.is_open())
		{
			printfc(FOREGROUND_RED, "[x] Couldn't find fortnite installation!.\n");
			system("pause");
			exit(1);
		}

		datStream >> LauncherInstalled;
		{
			json installationList = LauncherInstalled["InstallationList"];

			for (int i = 0; i < installationList.size(); i++)
			{
				if (installationList[i]["AppName"] == "Fortnite")
				{
					fnPath = installationList[i]["InstallLocation"];
				}
			}

			if (fnPath.empty())
			{
				printfc(FOREGROUND_RED, "[x] Couldn't find fortnite installation!.\n");
				system("pause");
				exit(1);
			}

			fnPath += R"(\FortniteGame\Binaries\Win64\FortniteClient-Win64-Shipping.exe)";

			auto gameHandle = Util::CreateProcessE(fnPath.c_str(), (char*)" -epicapp=Fortnite -epicenv=Prod -epiclocale=en-us -epicportal -noeac -nobe -fromfl=none");

			if (!gameHandle)
			{
				printfc(FOREGROUND_RED, "[x] Couldn't launch fortnite!.\n");
				system("pause");
				exit(1);
			}

			return gameHandle;
		}
	}
}
