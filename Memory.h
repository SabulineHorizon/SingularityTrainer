#pragma once
#include "framework.h"

DWORD GetProcId(const wchar_t* procName);

uint32_t GetModuleBaseAddress(DWORD procID, const wchar_t* modName);

uint32_t FindDMAAddy(HANDLE hProc, uint32_t ptr, std::vector<uint32_t> offsets);