#include "Address.h"

//GetAddress
uint32_t ADDRESS::GetAddress(const wchar_t* processName, std::vector<uint32_t> offsets) //Returns the bytes that were read
{
	std::vector<uint32_t> otherOffsets(offsets.begin() + 1, offsets.end()); //Gets all offsets after the first
	processId = GetProcId(processName);											//Get pID
	moduleBase = GetModuleBaseAddress(processId, processName);					//Get module base address
	processHandle = 0;															//Get process handle
	processHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);			//Open process
	pointerBase = moduleBase + offsets[0];										//Calculate offsets
	address = FindDMAAddy(processHandle, pointerBase, otherOffsets);			//Get address

	return address;
}

//ReadBytes
std::vector<byte> ADDRESS::ReadBytes(uint32_t numBytes)
{
	if (numBytes)
	{
		currentValues.resize(numBytes);
		if (GetAddress(processName, offsets))
			ReadProcessMemory(processHandle, (BYTE*)address, currentValues.data(), currentValues.size(), nullptr);
	}
	return currentValues;
}

//ReadFloat
float ADDRESS::ReadFloat() {
	float f = 0.0f;
	if (GetAddress(processName, offsets))
		ReadProcessMemory(processHandle, (BYTE*)address, &f, sizeof(f), nullptr);
	return f;
}

//WriteFloat
float ADDRESS::WriteFloat(float f) {
	if (GetAddress(processName, offsets) && f)
		WriteProcessMemory(processHandle, (BYTE*)address, &f, sizeof(f), nullptr);
	return f;
}

// WriteBytes
std::vector<byte> ADDRESS::WriteBytes(std::vector<byte> modifiedValues) //Returns the bytes that were written
{
	currentValues = modifiedValues;
	if (!WriteProcessMemory(processHandle, (BYTE*)address, currentValues.data(), currentValues.size(), nullptr))
		currentValues = {};
	return currentValues;
}

// RestoreBytes
std::vector<byte> ADDRESS::RestoreBytes() //Returns the bytes that were written
{
	if (!restoreValues.empty())
	{
		currentValues = restoreValues;
		WriteBytes(restoreValues);
	}
	return currentValues;
}

// ToggleBytes
void ADDRESS::toggleBytes()
{
	currentValues = ReadBytes(bytesToWrite.size());					// Read current value
	if (restoreValues.empty() && currentValues == expectedBytes)	// If restore values aren't set
		restoreValues = currentValues;                              // Set restore values
	{
		if (toggleFlag)                                             // If already toggled on
			WriteBytes(restoreValues);                              // Toggle off
		else
			WriteBytes(bytesToWrite);                               // Toggle on
		toggleFlag = !toggleFlag;                                   // Set flag
	}
}