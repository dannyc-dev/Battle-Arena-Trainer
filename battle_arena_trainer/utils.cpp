#include "utils.h";

DWORD GetProcessId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

MODULEENTRY32* GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	MODULEENTRY32* pmodBaseAddr = NULL;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					pmodBaseAddr = &modEntry;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return pmodBaseAddr;
}

MODULEENTRY32 GetModule(DWORD dwProcID, const wchar_t* moduleName)
{
	MODULEENTRY32 modEntry = { 0 };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 currentModule = { 0 };

		currentModule.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &currentModule))
		{
			do
			{
				if (!wcscmp(currentModule.szModule, moduleName))
				{
					modEntry = currentModule;
					break;
				}
			} while (Module32Next(hSnapshot, &currentModule));
		}
	}
	CloseHandle(hSnapshot);
	return modEntry;
}

void welcome_banner(std::string banner_message) 
{
	std::string author = "By: dannyc_dev";
	std::string banner_case = "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	std::string banner_message_f = "\t" + banner_message + "\t\n";
	std::string banner_message_a = "\t  " + author + "\t\n";
	std::string banner = banner_case + banner_message_f + banner_message_a + banner_case;
	std::cout << banner << std::endl;
}

void clear_screen() 
{
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
		system("cls");
	#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__) || (__APPLE__)
		std::system("clear");
	#endif
}

std::vector<int> read_player_stats(HANDLE hProcess, std::vector<uintptr_t> playerAddrTable) {
	std::vector<int> result = { };
	int playerStat = NULL;
	for (int i = 0; i < playerAddrTable.size(); i++) {
		ReadProcessMemory(hProcess, (BYTE*)playerAddrTable[i], &playerStat, sizeof(playerStat), nullptr);
		result.push_back(playerStat);
	}
	return result;
}

void write_player_stats(HANDLE hProcess, std::vector<uintptr_t> playerAddrTable, std::vector<int> targetStats) {
	for (int i = 0; i < playerAddrTable.size(); i++) {
		WriteProcessMemory(hProcess, (BYTE*)playerAddrTable[i], &targetStats[i], sizeof(targetStats[i]), nullptr);
	}
}

std::vector<uintptr_t> create_player_structure(uintptr_t basePlayerAddr) {
	// Creates vector of player addr for processing - { HP, base health, armor }
	std::vector<uintptr_t> playerStruct = { basePlayerAddr + 0x8, basePlayerAddr, basePlayerAddr + 0x4 };
	return playerStruct;
}

void print_process_details(const wchar_t* target_process, DWORD procId, uintptr_t modBaseAddr) 
{
	// Print Process Details
	std::cout << "Process Details: " << std::endl;
	std::wcout << "Target Process: " << target_process << std::endl;
	std::cout << "Process Id: " << procId << std::endl;

	// Print Module Base Addr
	std::cout << "Module Base Addr: " << "0x" << std::hex << modBaseAddr << std::endl;

}


void show_menu() 
{
	/*
	Options:
		- Show game details: view details about game and players
		- Attack: Attack opponent but risk damage
		- Heal: Heal Player
		- Reset Game: Resets the game to original state
		- Debug Data: Shows debug data for game
		- Surrender: End Game
	*/
	std::cout << "\nTrainer Menu:" << std::endl;
	std::cout << "0 - Show Process Details" << std::endl;
	std::cout << "1 - Change Player Stats" << std::endl;
	std::cout << "2 - Change Enemy Stats" << std::endl;
	std::cout << "3 - Quit" << std::endl;
}

int get_user_option() 
{
	int i_user_option;
	std::cout << "Enter Option: ";

	std::string user_option;

	getline(std::cin >> std::ws, user_option);

	try {
		i_user_option = std::stoi(user_option);
	}
	catch (...) {
		return -1;
	}
	return i_user_option;
}

int get_user_input() 
{
	show_menu();
	int user_option = get_user_option();
	return user_option;
}

void print_player_health(uintptr_t hp, uintptr_t health, uintptr_t armor) 
{
	std::cout << "\n- Player Details - " << std::endl;
	std::cout << "Player Total HP: " << std::dec << hp << std::endl;
	std::cout << "Player Health: " << std::dec << health << std::endl;
	std::cout << "Player Armor: " << std::dec << armor << std::endl;   
}

void print_enemy_health(uintptr_t hp, uintptr_t health, uintptr_t armor)
{
	std::cout << "\n- Enemy Details - " << std::endl;
	std::cout << "Enemy Total HP: " << std::dec << hp << std::endl;
	std::cout << "Enemy Health: " << std::dec << health << std::endl;
	std::cout << "Enemy Armor: " << std::dec << armor << std::endl;
}

std::vector<int> get_user_inputs()
{
	std::vector<int> user_inputs = { 0, 0, 0 };
	int size = user_inputs.size();
	std::string title = "Enter Target Inputs";
	banner(title);
	for (size_t i = 0; i < size; ++i)
	{   
		int i_user_option;
		std::string user_option;
		if (i == 0)
		{
			std::cout << "\nEnter HP: ";
		}
		else if (i == 1)
		{
			std::cout << "\nEnter Health: ";
		}
		else if (i == 2) {
			std::cout << "\nEnter Armor: ";
		}
		getline(std::cin >> std::ws, user_option);
		i_user_option = std::stoi(user_option);

		user_inputs[i] = i_user_option;
		
	}
	return user_inputs;
}

void banner(std::string banner_message) 
{
	std::string banner_case = "=-=-=-=-=-=-=-=-=-=-=\n";
	std::string banner_message_f = "" + banner_message + "\t\n";
	std::string banner = banner_case + banner_message_f + banner_case;
	std::cout << banner << std::endl;
}