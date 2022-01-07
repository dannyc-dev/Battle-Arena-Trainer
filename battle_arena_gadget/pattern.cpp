#include "pattern.h"

char* PatternScanModule(HANDLE hProcess, uintptr_t begin, uintptr_t end, const char* pattern, const char* mask)
{
	uintptr_t currentChunk = begin;
	SIZE_T bytesRead;

	while (currentChunk < end)
	{
		// Chunk
		char buffer[4096];

		// R\W protections
		DWORD oldmemprotect;
		VirtualProtectEx(hProcess, (void*)currentChunk, sizeof(buffer), PAGE_EXECUTE_READWRITE, &oldmemprotect);
		ReadProcessMemory(hProcess, (void*)currentChunk, &buffer, sizeof(buffer), &bytesRead);
		VirtualProtectEx(hProcess, (void*)currentChunk, sizeof(buffer), oldmemprotect, &oldmemprotect);

		if (bytesRead == 0)
		{
			// End
			return nullptr;
		}

		char* internalAddr = PatternScan(pattern, mask, buffer, (intptr_t)bytesRead);

		if (internalAddr != nullptr)
		{
			// Calculate relative offset from buffer
			uintptr_t offsetFromBuffer = (uintptr_t)internalAddr - (uintptr_t)&buffer;
			return (char*)(currentChunk + offsetFromBuffer);
		}
		else
		{
			// Go to next chunk
			currentChunk = currentChunk + bytesRead;
		}
	}
	return nullptr;
}

// Basic Byte Comparison from https://guidedhacking.com/threads/external-internal-pattern-scanning-guide.14112/
char* PatternScan(const char* pattern, const char* mask, char* begin, intptr_t size)
{
	intptr_t patternLen = strlen(mask);

	for (int i = 0; i < size; i++)
	{
		bool found = true;
		for (int j = 0; j < patternLen; j++)
		{
			if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return (begin + i);
		}
	}
	return nullptr;
}
