#pragma once
#include <Windows.h>
#include <TlHelp32.h>

char* PatternScanModule(HANDLE hProcess, uintptr_t begin, uintptr_t end, const char* pattern, const char* mask);
char* PatternScan(const char* pattern, const char* mask, char* begin, intptr_t size);