#pragma once
#include "framework.h"
#include "resource.h"

void AttachToProcess(HWND hWnd);
void UpdateStatusText(HWND hWnd, LPCSTR text, std::vector<byte> color = {});
void SavePosition(SAVEDATA& saveObject);
void LoadPosition(SAVEDATA& saveObject);
void OnEditButtonClick(HWND hwnd);
//bool IsKeyDown(HOTKEY hotkey, bool toggle = true);
//DWORD WINAPI threadUpdate(LPVOID lpParameter);
void Update();

float SmoothClamp(float value, float targetMin, float targetMax, float smoothFactor);





void SaveHotkeysToFile(const std::vector<HOTKEY>& hotkeys, const std::string& filePath);
bool LoadHotkeysFromFile(std::vector<HOTKEY>& hotkeys, const std::string& filePath);