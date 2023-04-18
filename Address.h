#pragma once
#include "framework.h"

class ADDRESS
{
	public:
		//Constructor
		ADDRESS(const wchar_t* processName, std::vector<unsigned int> offsets, std::vector<byte> expectedBytes, std::vector<byte> bytesToWrite) :
			processName(processName), offsets(offsets), expectedBytes(expectedBytes), bytesToWrite(bytesToWrite)
		{
			// Constructor body here, if needed
		}

		//Constructor
		ADDRESS(const wchar_t* processName, std::vector<unsigned int> offsets) :
			processName(processName), offsets(offsets)
		{
			// Constructor body here, if needed
		}

		uint32_t			GetAddress(const wchar_t* processName, std::vector<uint32_t> offsets); //Returns address
		//uintptr_t			GetAddress(const wchar_t* processName, std::vector<unsigned int> offsets); //Returns address
		std::vector<byte>	ReadBytes(unsigned int numBytes = 1); //Returns the bytes that were read
		float				ReadFloat(); //Returns the value that was read
		float				WriteFloat(float f); //Returns the value that was written
		std::vector<byte>   WriteBytes(std::vector<byte> modifiedValues); //Returns the bytes that were written
		std::vector<byte>	RestoreBytes();
		void				toggleBytes();

	//private:
		//This works for now, but I need to eventually figure out which of these need to be private or public
		std::vector<uint32_t>		offsets = {}; //offsets
		DWORD						processId = 0; //process id
		uintptr_t					moduleBase = 0;//module base address
		HANDLE						processHandle = 0; //process handle
		uint32_t					pointerBase = 0; //pointer base
		uint32_t					address = 0; //Get address
		std::vector<byte>			currentValues = {}; //Addresses are read to and from here
		std::vector<byte>			modifiedValues = {}; //Modified values to copy into currentValues
		std::vector<byte>			restoreValues = {}; //Modified values to copy into currentValues

		std::vector<byte>			expectedBytes = {};
		std::vector<byte>			bytesToWrite = {};
		const wchar_t*				processName = 0;
		bool						toggleFlag = 0;
};