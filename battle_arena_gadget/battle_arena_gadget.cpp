#include "utils.h";
#include "pattern.h"

int main()
{   
    std::string welcome = "Battle Arena Trainer!";
    welcome_banner(welcome);
    const wchar_t * target_process = L"battle_arena.exe";
    int playerHealth, playerArmor, playerHp;
    int enemyHealth, enemyArmor, enemyHp;

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
    std::uintptr_t playerArmorAddr = calculatedPlayerAddr + 0x4;
    std::uintptr_t playerHpAddr = playerArmorAddr + 0x4;
    std::cout << "\n[*]Calculated Player Address: " << "0x" << std::hex << calculatedPlayerAddr << std::endl;
    ReadProcessMemory(hProcess, (BYTE*)calculatedPlayerAddr, &playerHealth, sizeof(playerHealth), nullptr);
    ReadProcessMemory(hProcess, (BYTE*)playerArmorAddr, &playerArmor, sizeof(playerArmor), nullptr);
    ReadProcessMemory(hProcess, (BYTE*)playerHpAddr, &playerHp, sizeof(playerHp), nullptr);
    print_player_health(playerHp, playerHealth, playerArmor);

    // Enemy Addr
    std::uintptr_t calculatedEnemyAddr = calculatedPlayerAddr + 0x10;
    std::uintptr_t enemyArmorAddr = calculatedEnemyAddr + 0x4;
    std::uintptr_t enemyHpAddr = enemyArmorAddr + 0x4;
    std::cout << "\n[*]Calculated Enemy Address: " << "0x" << std::hex << calculatedEnemyAddr << std::endl;
    ReadProcessMemory(hProcess, (BYTE*)calculatedEnemyAddr, &enemyHealth, sizeof(enemyHealth), nullptr);
    ReadProcessMemory(hProcess, (BYTE*)enemyArmorAddr, &enemyArmor, sizeof(enemyArmor), nullptr);
    ReadProcessMemory(hProcess, (BYTE*)enemyHpAddr, &enemyHp, sizeof(enemyHp), nullptr);
    print_enemy_health(enemyHp, enemyHealth, enemyArmor);

    int user_opt;
    // menu loop
    do {
        user_opt = get_user_input();
        switch (user_opt) {
        case 0: {
            clear_screen();
            welcome_banner(welcome);
            print_process_details(target_process, procId, modBaseAddr);

            // Read player memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedPlayerAddr, &playerHealth, sizeof(playerHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerArmorAddr, &playerArmor, sizeof(playerArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerHpAddr, &playerHp, sizeof(playerHp), nullptr);

            // Read enemy memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedEnemyAddr, &enemyHealth, sizeof(enemyHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyArmorAddr, &enemyArmor, sizeof(enemyArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyHpAddr, &enemyHp, sizeof(enemyHp), nullptr);

            std::cout << "\nGame Details: " << std::endl;
            print_player_health(playerHp, playerHealth, playerArmor);
            print_enemy_health(enemyHp, enemyHealth, enemyArmor);

            break;
        }
        case 1: {
            clear_screen();
            std::vector<int> user_inputs = get_user_inputs();
            // Boosting Health
            int targetHealth = user_inputs[1];
            WriteProcessMemory(hProcess, (BYTE*)calculatedPlayerAddr, &targetHealth, sizeof(targetHealth), nullptr);
            // Boosting Armor
            int targetArmor = user_inputs[2];
            WriteProcessMemory(hProcess, (BYTE*)playerArmorAddr, &targetArmor, sizeof(targetArmor), nullptr);
            // Boosting Total HP
            int targetHp = user_inputs[0];
            WriteProcessMemory(hProcess, (BYTE*)playerHpAddr, &targetHp, sizeof(targetHp), nullptr);

            // Read player memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedPlayerAddr, &playerHealth, sizeof(playerHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerArmorAddr, &playerArmor, sizeof(playerArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerHpAddr, &playerHp, sizeof(playerHp), nullptr);

            // Read enemy memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedEnemyAddr, &enemyHealth, sizeof(enemyHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyArmorAddr, &enemyArmor, sizeof(enemyArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyHpAddr, &enemyHp, sizeof(enemyHp), nullptr);

            std::cout << "\nCurrent Game Stats: " << std::endl;
            print_player_health(playerHp, playerHealth, playerArmor);
            print_enemy_health(enemyHp, enemyHealth, enemyArmor);

            break;
        }
        case 2: {
            clear_screen();
            std::vector<int> user_inputs = get_user_inputs();
            // Changing Health
            int targetHealth = user_inputs[1];
            WriteProcessMemory(hProcess, (BYTE*)calculatedEnemyAddr, &targetHealth, sizeof(targetHealth), nullptr);
            // Changing Armor
            int targetArmor = user_inputs[2];
            WriteProcessMemory(hProcess, (BYTE*)enemyArmorAddr, &targetArmor, sizeof(targetArmor), nullptr);
            // Changing Total HP
            int targetHp = user_inputs[0];
            WriteProcessMemory(hProcess, (BYTE*)enemyHpAddr, &targetHp, sizeof(targetHp), nullptr);

            // Read player memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedPlayerAddr, &playerHealth, sizeof(playerHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerArmorAddr, &playerArmor, sizeof(playerArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)playerHpAddr, &playerHp, sizeof(playerHp), nullptr);

            // Read enemy memory
            ReadProcessMemory(hProcess, (BYTE*)calculatedEnemyAddr, &enemyHealth, sizeof(enemyHealth), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyArmorAddr, &enemyArmor, sizeof(enemyArmor), nullptr);
            ReadProcessMemory(hProcess, (BYTE*)enemyHpAddr, &enemyHp, sizeof(enemyHp), nullptr);

            std::cout << "\nCurrent Game Stats: " << std::endl;
            print_player_health(playerHp, playerHealth, playerArmor);
            print_enemy_health(enemyHp, enemyHealth, enemyArmor);

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