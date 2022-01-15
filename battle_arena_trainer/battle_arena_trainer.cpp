#include "utils.h";
#include "pattern.h"

int main()
{   
	std::string welcome = "Battle Arena Trainer!";
	welcome_banner(welcome);
	const wchar_t * target_process = L"battle_arena.exe";

	DWORD procId = GetProcessId(target_process);
	MODULEENTRY32 procMod = GetModule(procId, target_process);

	if (procId == 0) {
		std::cout << "[Error] Process name/id not found.\nMake sure the target process is running and the process name is correct." << std::endl;
		return 0;
	}

	// Get Modules Base Addr
	MODULEENTRY32* pmoduleBase = GetModuleBaseAddress(procId, target_process);
	uintptr_t modBaseAddr = (uintptr_t)pmoduleBase->modBaseAddr;

	//Get Handle to Process
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	if (!hProcess) {
		std::cout << "[Error] Could not open handle to process." << std::endl;
		return 0;
	}

	// Print Process Details 
	print_process_details(target_process, procId, modBaseAddr);

	// Static calculated offsets and values - 0x0 health, 0x4 armor, 0x8 total hp
	//uintptr_t playerBase = modBaseAddr + 0x3C528;
	//ReadProcessMemory(hProcess, (BYTE*)playerBase, &playerHealth, sizeof(playerHealth), nullptr);

	// Pattern Scan
	// Player Pattern Scan
	const char* playerPattern = "\x48\x8B\x95\xE8\x07\x00\x00\x48\x8D\x0D\x68\x93\x01\x00";
	const char* playerMask = "xxx????xxx????";
	uintptr_t begin = (uintptr_t)procMod.modBaseAddr;
	uintptr_t end = begin + procMod.modBaseSize;
	
	// Pattern Base Addr
	char* patternBaseAddr = PatternScanModule(hProcess, begin, end, playerPattern, playerMask);
	// Add offset for mask bytes [xxx????xxx]???? -> 0xA
	std::uintptr_t playerBytesAddr = reinterpret_cast<uintptr_t>(patternBaseAddr) + 0xA;
	// Add offset until next instruction -> 0xE
	std::uintptr_t nextAddr = reinterpret_cast<uintptr_t>(patternBaseAddr) + 0xE;
	int offsetValue = 0;
	ReadProcessMemory(hProcess, (BYTE*)playerBytesAddr, &offsetValue, sizeof(offsetValue), nullptr);

	// Player Addr
	std::uintptr_t calculatedPlayerAddr = nextAddr + offsetValue;
	std::cout << "\n[*]Calculated Player Address: " << "0x" << std::hex << calculatedPlayerAddr << std::endl;
	std::vector<uintptr_t> playerStruct = create_player_structure(calculatedPlayerAddr);

	// Read Player stats
	std::vector<int> player_stats = read_player_stats(hProcess, playerStruct);
	print_player_health(player_stats);

	// Enemy Addr
	std::uintptr_t calculatedEnemyAddr = calculatedPlayerAddr + 0x10;
	std::vector<uintptr_t> enemyStruct = create_player_structure(calculatedEnemyAddr);
	std::vector<int> enemy_stats = read_player_stats(hProcess, enemyStruct);

	std::cout << "\n[*]Calculated Enemy Address: " << "0x" << std::hex << calculatedEnemyAddr << std::endl;
	print_enemy_health(enemy_stats);

	int user_opt = NULL;
	// menu loop
	do {
		user_opt = get_user_input();
		switch (user_opt) {
		case 0: {
			clear_screen();
			welcome_banner(welcome);
			print_process_details(target_process, procId, modBaseAddr);

			// Read player memory
			std::vector<int> player_stats = read_player_stats(hProcess, playerStruct);

			// Read enemy memory
			std::vector<int> enemy_stats = read_player_stats(hProcess, enemyStruct);

			std::cout << "\nGame Details: " << std::endl;
			print_player_health(player_stats);
			print_enemy_health(enemy_stats);

			break;
		}
		case 1: {
			clear_screen();
			std::vector<int> user_inputs = get_user_inputs();
			write_player_stats(hProcess, playerStruct, user_inputs);

			// Read player memory
			std::vector<int> player_stats = read_player_stats(hProcess, playerStruct);

			// Read enemy memory
			std::vector<int> enemy_stats = read_player_stats(hProcess, enemyStruct);

			std::cout << "\nCurrent Game Stats: " << std::endl;
			print_player_health(player_stats);
			print_enemy_health(enemy_stats);

			break;
		}
		case 2: {
			clear_screen();
			std::vector<int> user_inputs = get_user_inputs();
			write_player_stats(hProcess, enemyStruct, user_inputs);

			// Read player memory
			std::vector<int> player_stats = read_player_stats(hProcess, playerStruct);

			// Read enemy memory
			std::vector<int> enemy_stats = read_player_stats(hProcess, enemyStruct);

			std::cout << "\nCurrent Game Stats: " << std::endl;
			print_player_health(player_stats);
			print_enemy_health(enemy_stats);

			break;
		}
		case 3: {
			break;
		}
		default: {
			std::cout << user_opt << " is not a valid choice" << std::endl;
		}
		}
	} while (user_opt != 3);

}