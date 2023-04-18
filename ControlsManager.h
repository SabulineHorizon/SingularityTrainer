#pragma once
#include "framework.h"

// I'm currently setting this up to define hotkeys in memory only
// Later, when it's working, I'll read/write them to/from a hotkeys file

//typedef struct {
//	std::wstring saveLabel, loadLabel, flyLabel, hoverLabel, godLabel, rewindLabel; // label for settings list
//	std::vector<short> saveHotkey, loadHotkey, flyHotkey, hoverHotkey, godHotkey, rewindHotkey; // hotkey input
//} HOTKEYS;

//typedef struct {
//	std::wstring Label; // label for settings list
//	std::vector<short> Hotkey; // hotkey input
//	std::wstring Description; // text that describes usage of hotkey
//} HOTKEY;

class HOTKEY {
	public:
		std::wstring label = L""; // label for settings list
		std::vector<short> hotkey = { 0 }; // hotkey input
		std::wstring description = L""; // text that describes usage of hotkey

		bool wasDown = true;
		HOTKEY(std::wstring label, std::vector<short> hotkey, std::wstring description);
		std::wstring GetKeyName(short keyCode);
		std::wstring HotkeyToString(std::vector<short> hotkey, std::wstring padding = L" + ");
		std::wstring HotkeyToString(short hotkeyShort, std::wstring padding = L" + ");
		bool IsKeyDown(bool toggle = false);
};

typedef struct {
	float x, y, z, pitch, yaw;
} SAVEDATA;

void SetHotkeys(std::vector<HOTKEY> hotkeys);

// defineHotkeys()
//		if readFile(path)
//			foreach(hotkey)
//				if path contains hotkey
//					set hotkey to match path
//				else
//					set hotkey to default
//		else
//			createFile(path)

