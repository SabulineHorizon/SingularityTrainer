#include "ControlsManager.h"




//Writes the hotkeys to file
void SetHotkeys(std::vector<HOTKEY> hotkeys) {
	//std::string path;

	// defineHotkeys(std::vector hotkeysList)
	//		if readFile(path)
	//			foreach(hotkey)
	//				if path contains hotkey
	//					set hotkey to match path
	//				else
	//					set hotkey to default
	//		else
	//			createFile(path)
}

HOTKEY::HOTKEY(std::wstring label, std::vector<short> hotkey, std::wstring description)
	: label(label), hotkey(hotkey), description(description)
{
	// Constructor body
}

// Joins all buttons involved in a hotkey into a single wstring
std::wstring HOTKEY::HotkeyToString(std::vector<short> hotkey, std::wstring padding)
{
	std::wstring fullString;
	for (size_t i = 0; i < hotkey.size(); ++i) {
		fullString += GetKeyName(hotkey[i]);
		if (i != hotkey.size() - 1) {
			fullString += padding;
		}
	}
	return fullString;
}

// Overload for HotkeyToString to accept a single short
std::wstring HOTKEY::HotkeyToString(short hotkeyShort, std::wstring padding)
{
	// This code isn't especially efficient, but it makes copy/pasting code easier
	std::vector<short> hotkey = { hotkeyShort };

	std::wstring fullString;
	for (size_t i = 0; i < hotkey.size(); ++i) {
		fullString += GetKeyName(hotkey[i]);
		if (i != hotkey.size() - 1) {
			fullString += padding;
		}
	}
	return fullString;
}

// Called by HotkeyToString to get the text name of each key code
std::wstring HOTKEY::GetKeyName(short keyCode)
{
	wchar_t keyName[0xFF] = { 0 };
	UINT scanCode = MapVirtualKeyW(keyCode, MAPVK_VK_TO_VSC);

	// If the virtual-key code matches the code of an extended key
	switch (keyCode)
	{
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: case VK_RCONTROL: case VK_RMENU:
	case VK_LWIN: case VK_RWIN: case VK_APPS: case VK_PRIOR: case VK_NEXT: case VK_END:
	case VK_HOME: case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK: case VK_SNAPSHOT:
		scanCode |= KF_EXTENDED; // bitwise or adds the extended flag to make sure the correct key is recognized
	}

	// Write key name into the keyName buffer
	int result = GetKeyNameText((LONG)scanCode << 16, keyName, 0xFF);

	if (result == 0)
		return L"None"; // Failed to get key name
	else
		return keyName;
}

//bool HOTKEY::IsKeyDown(HOTKEY hotkey, bool toggle)
bool HOTKEY::IsKeyDown(bool toggle)
{
	std::vector<short> keyList = hotkey;
	if (keyList.size() <= 0) return false;

	// Loop through each key and return if any aren't pressed
	for (const auto& vkCode : keyList) {
		if (!(GetAsyncKeyState(vkCode) & 0x8000)) {
			// Key is up
			// Return false as soon as one key is up
			//if ((label == L"Jump") || (label == L"Save") || (label == L"Load"))
				//UpdateStatusText(statusTextHWND, "Hotkey isn't active", { 255, 50, 50 });
			wasDown = false;
			return false;
		}
	}

	//hotkey.wasDown = hotkey.wasDown; // Flag for future checks

	// if hotkey was down and toggle is on, return false anyway
	//if (hotkey.wasDown && toggle) {
	if (wasDown && toggle) {
		//if ((label == L"Jump") || (label == L"Save") || (label == L"Load"))
			//UpdateStatusText(statusTextHWND, "Hotkey is active but toggled", { 0, 50, 255 });
		return false;
	}

	//if ((label == L"Jump") || (label == L"Save") || (label == L"Load"))
		//UpdateStatusText(statusTextHWND, "Hotkey is active", { 50, 255, 50 });
	// Loop finished, all keys are down
	wasDown = true; // Flag for future checks
	return true; // Return true if all keys are pressed
}