#ifndef BATTLE_ARENA_TRAINER_HELPER_H
#define BATTLE_ARENA_TRAINER_HELPER_H
#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>

DWORD GetProcessId(const wchar_t* procName);
MODULEENTRY32* GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
MODULEENTRY32 GetModule(DWORD dwProcID, const wchar_t* moduleName);
void welcome_banner(std::string banner_message);
void clear_screen();
void banner(std::string banner_message);
void print_process_details(const wchar_t* target_process, DWORD procId, uintptr_t modBaseAddr);
int get_user_input();
int get_user_option();
void show_menu();
void print_player_health(uintptr_t hp, uintptr_t health, uintptr_t armor);
void print_enemy_health(uintptr_t hp, uintptr_t health, uintptr_t armor);
std::vector<int> get_user_inputs();
std::vector<int> read_player_stats(HANDLE hProcess, std::vector<uintptr_t> playerAddrTable);
std::vector<uintptr_t> create_player_structure(uintptr_t basePlayerAddr);
void write_player_stats(HANDLE hProcess, std::vector<uintptr_t> playerAddrTable, std::vector<int> targetStats);

#endif

