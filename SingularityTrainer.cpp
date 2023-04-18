// SingularityTrainer.cpp : Defines the entry point for the application.

#include "SingularityTrainer.h"
#include <iostream>
#include "ControlsManager.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                    // current instance
WCHAR szTitle[MAX_LOADSTRING];                      // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];                // the main window class name

LPCSTR statusText = "Initializing...";              // Process status display in the main window
int statusColor = 0;                                // Display color for status text
bool attached = 0;                                  // Display color for status text
bool flying = 0;

const wchar_t* processName = L"Singularity.exe";    // Process name of address to modify
DWORD lastPID = 0;                                  // Used to recognize changes to the process
DWORD pID = 0;                                      // Used to compare pID to lastPID

ADDRESS hoverToggle(processName, { 0x60FCFD },{ 0xF3, 0x0F, 0x11, 0x46, 0x5C },{ 0x90, 0x90, 0x90, 0x90, 0x90 }); // process, offset, expectedBytes, bytesToWrite
ADDRESS healthToggle(processName, { 0x8EDC9 },{ 0xF3, 0x0F, 0x11, 0x07 },{ 0x90, 0x90, 0x90, 0x90}); //This affects enemies too, might need new instruction // process, offset, expectedBytes, bytesToWrite
//ADDRESS coalescedHash(processName, { 0x186B3E4, 0x268, 0x94 },
//    { 0xFF, 0xA3, 0x26, 0xE7, 0x39, 0x21, 0x29, 0xE7, 0xD1, 0x1B, 0x09, 0x0A, 0x83, 0x1B, 0xF7, 0x2D, 0xB9, 0x54, 0x7F, 0x28 },
//    { 0x49, 0xD4, 0x5B, 0x6E, 0xD2, 0x99, 0x01, 0xB1, 0xF7, 0xCB, 0x4B, 0xDC, 0xC8, 0x56, 0x97, 0xFB, 0x63, 0xF4, 0x27, 0xD9 }); // process, offset, expectedBytes, bytesToWrite
ADDRESS playerX(processName, { 0x191A0E8, 0x30, 0x54 }); // process, offset
ADDRESS playerY(processName, { 0x191A0E8, 0x30, 0x58 }); // process, offset
ADDRESS playerZ(processName, { 0x191A0E8, 0x30, 0x5C }); // process, offset
ADDRESS playerPitch(processName, { 0x192B1EC, 0xF80, 0x430, 0xC3C }); // process, offset
ADDRESS playerYaw(processName, { 0x191A0E8, 0x30, 0x1EC, 0x64 }); // process, offset
ADDRESS PlayerVelocityX(processName, { 0x191A0E8, 0x30, 0x128 }); // process, offset
ADDRESS PlayerVelocityY(processName, { 0x191A0E8, 0x30, 0x12C }); // process, offset
ADDRESS PlayerVelocityZ(processName, { 0x191A0E8, 0x30, 0x130 }); // process, offset

SAVEDATA savePos1;
SAVEDATA savePos2;
SAVEDATA savePos3;
SAVEDATA savePos4;

int rewindSpeed = 10;
int rewindState = 0;
std::vector<SAVEDATA> rewindHistory = {};

HWND listBox1Handle;
HWND listBox2Handle;
int listBoxIndex = 0;
HWND descriptionHandle;

HOTKEY godHotkey(L"God Toggle", { VK_F9 }, L"Enables/Disables invulnerability. Also makes enemies invulnerable");
HOTKEY flyToggleHotkey(L"Fly Toggle", { VK_F10 }, L"Enables/Disables fly mode");
HOTKEY hoverHotkey(L"Hover Toggle", { VK_F11 }, L"Enables/Disables hover mode");
HOTKEY jumpHotkey(L"Fly", { VK_SPACE }, L"Fly up while in fly mode");
HOTKEY runHotkey(L"Fly Speed", { VK_SHIFT }, L"Hold key to move faster while in fly mode");
HOTKEY rewindHotkey(L"Rewind", { VK_BACK }, L"Rewind player's position through past movement history");
HOTKEY save1(L"Save 1", { VK_CONTROL, 0x31 }, L"Saves the player's location in position 1");
HOTKEY load1(L"Load 1", { VK_SHIFT, 0x31 }, L"Teleports the player back to position 1");
HOTKEY save2(L"Save 2", { VK_CONTROL, 0x32 }, L"Saves the player's location in position 2");
HOTKEY load2(L"Load 2", { VK_SHIFT, 0x32 }, L"Teleports the player back to position 2");
HOTKEY save3(L"Save 3", { VK_CONTROL, 0x33 }, L"Saves the player's location in position 3");
HOTKEY load3(L"Load 3", { VK_SHIFT, 0x33 }, L"Teleports the player back to position 3");
HOTKEY save4(L"Save 4", { VK_CONTROL, 0x34 }, L"Saves the player's location in position 4");
HOTKEY load4(L"Load 4", { VK_SHIFT, 0x34 }, L"Teleports the player back to position 4");
std::vector<HOTKEY*> hotkeysList = { &godHotkey, &flyToggleHotkey, &hoverHotkey, &jumpHotkey, &runHotkey, &rewindHotkey, &save1, &load1, &save2, &load2, &save3, &load3, &save4, &load4 };

std::vector<byte> currentValues = {};               // Most recent values read from address

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Settings(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SINGULARITYTRAINER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // In your main function or window procedure:
    //HANDLE hThread = CreateThread(NULL, 0, threadUpdate, NULL, 0, NULL);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SINGULARITYTRAINER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SINGULARITYTRAINER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SINGULARITYTRAINER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SINGULARITYTRAINER));

    return RegisterClassExW(&wcex);
}

//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 130, nullptr, nullptr, hInstance, nullptr);
   //RegisterHotKey(hWnd, SavePosHotkey, 0, VK_F5);
   //RegisterHotKey(hWnd, LoadPosHotkey, 0, VK_F9);
   //RegisterHotKey(hWnd, HealthHotkey, 0, VK_F10);
   //RegisterHotKey(hWnd, FlyToggleHotkey, 0, VK_F11);
   //RegisterHotKey(hWnd, HoverHotkey, 0, VK_F7);
   //RegisterHotKey(hWnd, RewindHotkey, 0, VK_BACK);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //Timer starts short, then gets longer after first run
   SetTimer(hWnd, IDT_PROCESSTIMER, 100, (TIMERPROC)NULL); //0.1 second timer
   SetTimer(hWnd, IDT_UPDATETIMER, 100, (TIMERPROC)NULL); //0.1 second timer

   return TRUE;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, Settings);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDT_PROCESSTIMER:
                {
                    AttachToProcess(hWnd);
                    if(attached)
                        SetTimer(hWnd, IDT_PROCESSTIMER, 2000, (TIMERPROC)NULL); //2 second timer
                    else
                        SetTimer(hWnd, IDT_PROCESSTIMER, 20, (TIMERPROC)NULL); //0.01 second timer
                }
                break;
            case IDT_UPDATETIMER:
                {
                    Update();
                    SetTimer(hWnd, IDT_UPDATETIMER, 10, (TIMERPROC)NULL); //0.01 second timer
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_HOTKEY:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        //case SavePosHotkey:
        //{
        //    SavePosition(savePos1);
        //    //MessageBox(NULL, L"Save hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        //case LoadPosHotkey:
        //{
        //    LoadPosition(savePos1);
        //    //MessageBox(NULL, L"Load hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        //case HealthHotkey:
        //{
        //    healthToggle.toggleBytes();
        //    //MessageBox(NULL, L"Fly hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        //case FlyToggleHotkey:
        //{
        //    flying = !flying;

        //    //MessageBox(NULL, L"Fly hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        //case HoverHotkey:
        //{
        //    hoverToggle.toggleBytes();
        //    //MessageBox(NULL, L"Hover hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        //case RewindHotkey:
        //{
        //    LoadPosition(savePos2);
        //    //MessageBox(NULL, L"Rewind hotkey pressed!", L"MyApp", MB_OK);
        //}
        //break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rect;
            HFONT font = CreateFontA(25, 0, 0, 0, FW_HEAVY, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SelectObject(hdc, font);
            SetBkColor(hdc, RGB(255, 255, 255));

            SetTextColor(hdc, 0);
            SetRect(&rect, 10, 5, 280, 25);
            DrawTextA(hdc, "Status:", -1, &rect, DT_LEFT | DT_TOP);

            SetTextColor(hdc, statusColor);
            SetRect(&rect, 10, 30, 280, 60);
            DrawTextA(hdc, statusText, -1, &rect, DT_LEFT | DT_TOP);
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //Restore modified address to original value
        hoverToggle.RestoreBytes();
        healthToggle.RestoreBytes();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for settings box.
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:

        // Get element handles
        listBox1Handle = GetDlgItem(hDlg, IDC_LIST1);
        listBox2Handle = GetDlgItem(hDlg, IDC_LIST2);
        descriptionHandle = GetDlgItem(hDlg, IDC_DESCRIPTIONLABEL);

        //HOTKEY hotkey = hotkeysList[listBoxIndex]; //--------------
        // 
        //HOTKEY* hotkeyPtr = hotkeysList[listBoxIndex];
        //HOTKEY& hotkey = *hotkeyPtr;

        // Add elements to the listboxes from vector
        for (auto& hotkey : hotkeysList) {
            SendMessage(listBox1Handle, LB_ADDSTRING, 0, (LPARAM)hotkey->label.c_str());
            SendMessage(listBox2Handle, LB_ADDSTRING, 0, (LPARAM)hotkey->HotkeyToString(hotkey->hotkey, L" + ").c_str());
        //for (auto& hotkey : hotkeysList) {
        //    SendMessage(listBox1Handle, LB_ADDSTRING, 0, (LPARAM)hotkey.label.c_str());
        //    SendMessage(listBox2Handle, LB_ADDSTRING, 0, (LPARAM)hotkey.HotkeyToString(hotkey.hotkey, L" + ").c_str());

            ////LPARAM test1 = hotkey&;
            //HOTKEY* hotkey = hotkey;
            //LPARAM lParam = reinterpret_cast<LPARAM>(hotkey);
            ////SendMessage(listBox2Handle, LB_SETITEMDATA, 0, (LPARAM)0xFF);
            ////SendMessage(listBox2Handle, LB_SETITEMDATA, 0, (LPARAM)hotkey&);
            //SendMessage(listBox2Handle, LB_SETITEMDATA, 0, (LPARAM)hotkey);
        }

        SendMessage(listBox1Handle, LB_SETCURSEL, listBoxIndex, 0); //Set listbox 1 selection
        SendMessage(listBox2Handle, LB_SETCURSEL, listBoxIndex, 0); //Set listbox 2 selection

        //SendMessage(descriptionHandle, WM_SETTEXT, 0, (LPARAM)L"New Label Text");
        SendMessage(descriptionHandle, WM_SETTEXT, 0, (LPARAM)hotkeysList[listBoxIndex]->description.c_str());

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        // When one listbox element is selected, select the other to match
        else if (LOWORD(wParam) == IDC_LIST1 && HIWORD(wParam) == LBN_SELCHANGE)
        {
            listBoxIndex = SendMessage(listBox1Handle, LB_GETCURSEL, 0, 0); //Get listbox1 index
            SendMessage(listBox2Handle, LB_SETCURSEL, listBoxIndex, 0); //Set listbox 2 selection
            // Handle the selection change in listBox1
            // ...
            SendMessage(descriptionHandle, WM_SETTEXT, 0, (LPARAM)hotkeysList[listBoxIndex]->description.c_str());
        }
        else if (LOWORD(wParam) == IDC_LIST2 && HIWORD(wParam) == LBN_SELCHANGE)
        {
            listBoxIndex = SendMessage(listBox2Handle, LB_GETCURSEL, 0, 0); //Get listbox2 index
            SendMessage(listBox1Handle, LB_SETCURSEL, listBoxIndex, 0); //Set listbox 1 selection
            // Handle the selection change in listBox2
            // ...
            SendMessage(descriptionHandle, WM_SETTEXT, 0, (LPARAM)hotkeysList[listBoxIndex]->description.c_str());
        }
        else if ((HIWORD(wParam) == BN_CLICKED) && (lParam != 0))
        {
            switch (LOWORD(wParam))
            {
            case IDC_EDITBUTTON:
                OnEditButtonClick(hDlg);
                break;
            //case ID_BTN2:
            //    Function_B2();
            //    break;
            }
        }

        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
HWND statusTextHWND = 0;
void AttachToProcess(HWND hWnd)
{
    //ToDo: If the address can't be found or if the expected bytes don't match, perform an AoB scan instead
    statusTextHWND = hWnd;
    //if process is open
    pID = GetProcId(processName);
    if (pID && (pID != lastPID)) //Check if process is open and changed
    {
        //currentValues = restoreValues = flyToggle.ReadBytes(flyToggle.bytesToWrite.size());
        currentValues = healthToggle.ReadBytes(healthToggle.bytesToWrite.size());
        if (currentValues == healthToggle.expectedBytes) //If bytes match what we expect, it's probably the right address
        {
            //flyToggle.WriteBytes(flyToggle.bytesToWrite);
            UpdateStatusText(hWnd, "Process attached", { 0,100,0 });
        }
        //currentValues = coalescedHash.ReadBytes(coalescedHash.bytesToWrite.size());
        //if (currentValues == coalescedHash.expectedBytes) //If bytes match what we expect, it's probably the right address
        //{
        //    coalescedHash.toggleBytes();
        //    attached = 1;
        //    UpdateStatusText(hWnd, "Process attached", { 0,100,0 });
        //}
        else
        {
            UpdateStatusText(hWnd, "Failed to find memory address", { 255, 0, 0 });
        }
        lastPID = pID;
    }
    else
    {
        if (!pID)
            UpdateStatusText(hWnd, "Unable to locate process...", { 150, 0, 0 });
    }
}

// Updates process status text
void UpdateStatusText(HWND hWnd, LPCSTR text, std::vector<byte> color)
{
    if (color.size() == 3)
        statusColor = RGB(color[0], color[1], color[2]);
    statusText = text;
    InvalidateRect(hWnd, NULL, TRUE);
}

void SavePosition(SAVEDATA& saveObject)
{
    //MessageBox(NULL, L"Save hotkey pressed!", L"MyApp", MB_OK);
    saveObject.x = playerX.ReadFloat();
    saveObject.y = playerY.ReadFloat();
    saveObject.z = playerZ.ReadFloat();
    //saveObject.pitch = playerPitch.ReadFloat(); //Disabled because pitch seems to be broken somehow?
    saveObject.yaw = playerYaw.ReadFloat();
}

void LoadPosition(SAVEDATA& saveObject)
{
    //MessageBox(NULL, L"Load hotkey pressed!", L"MyApp", MB_OK);
    playerX.WriteFloat(saveObject.x);
    playerY.WriteFloat(saveObject.y);
    playerZ.WriteFloat(saveObject.z);
    //playerPitch.WriteFloat(saveObject.pitch); //Disabled because pitch seems to be broken somehow?
    playerYaw.WriteFloat(saveObject.yaw);
}

//Clamps values smoothly. Lower smoothFactor is more abrupt
float SmoothClamp(float value, float targetMin = 100.0f, float targetMax = 100.0f, float smoothFactor = 0.5f)
{
    float valueClamped = std::clamp(value, targetMin, targetMax);
    float difference = value - valueClamped;
    difference = difference * smoothFactor;
    value = valueClamped + difference;

    return value;
}


void OnEditButtonClick(HWND hwnd) {

    // Get Edit Button handle
    HWND editButton = GetDlgItem(hwnd, IDC_EDITBUTTON);
    listBoxIndex = SendMessage(listBox1Handle, LB_GETCURSEL, 0, 0);
    //SendMessage(listBox2Handle, LB_GETITEMDATA, listBoxIndex, 0);
    //HOTKEY hotkey = hotkeysList[listBoxIndex];//--------------------------------------------------
    //HOTKEY& hotkey = hotkeysList[listBoxIndex];
    //LPARAM itemData = SendMessage(listBox2Handle, LB_GETITEMDATA, listBoxIndex, 0);
    //HOTKEY* hotkey = reinterpret_cast<HOTKEY*>(itemData);

    SetWindowText(editButton, L"Press any key");
    
    // Listen for hotkey inputs
    MSG msg;
    wchar_t keyName[0xFF];
    std::vector<short> keyList = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            // if WM_KEYDOWN is esc, break loop
            if (msg.wParam == VK_ESCAPE)
                break;

            // Get the name of the key that was pressed
            GetKeyNameText((LONG)msg.lParam, keyName, 0xFF);
            //hotkey.hotkey = { 0xFF };

            // Add key to a temporary hotkey vector
            keyList.push_back((short)msg.wParam);

            // Change the label to the name of the key
            SetWindowText(editButton, keyName);
        }
        if (msg.message == WM_KEYUP) {

            // Remove duplicates from keyList
            std::sort(keyList.begin(), keyList.end());
            keyList.erase(unique(keyList.begin(), keyList.end()), keyList.end());
            //hotkey.hotkey = keyList;
            hotkeysList[listBoxIndex]->hotkey = keyList;

            // Update the listbox with new hotkey
            SendMessage(listBox2Handle, LB_DELETESTRING, listBoxIndex, 0); //Delete selected string
            SendMessage(listBox2Handle, LB_INSERTSTRING, listBoxIndex, (LPARAM)hotkeysList[listBoxIndex]->HotkeyToString(hotkeysList[listBoxIndex]->hotkey, L" + ").c_str()); //Replace deleted string
            SendMessage(listBox2Handle, LB_SETCURSEL, listBoxIndex, 0); //Set listbox 2 selection

            //Need to check if a matches or is included in another keybind, and if so, highlight the shorter one red

            break; // Exit the message loop
        }
    }
    SetWindowText(editButton, L"Edit Hotkey");
}
//std::wstring HOTKEY::GetKeyName(short keyCode)
//{
//    WCHAR keyName[0xFF] = { 0 };
//    int result = GetKeyNameText(MapVirtualKey(keyCode, MAPVK_VK_TO_VSC) << 16, keyName, 0xFF);
//    if (result == 0)
//        return L"None"; // Failed to get key name
//    else
//        return keyName;
//}

//void OnEditButtonClick(HWND hwnd) {
//    // Get a handle to the edit button control
//    HWND editButton = GetDlgItem(hwnd, IDC_EDITBUTTON);
//
//    // Change the label to "Press any key"
//    SetWindowText(editButton, L"Press any key");
//
//    // Wait for a key to be pressed
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        if (msg.message == WM_KEYDOWN) {
//            // Get the name of the key that was pressed
//            wchar_t keyName[256];
//            GetKeyNameText((LONG)msg.lParam, keyName, 256);
//
//            // Change the label to the name of the key
//            SetWindowText(editButton, keyName);
//
//            break; // Exit the message loop
//        }
//    }
//}


//----------------------------------------------------------------
//bool IsKeyDown(HOTKEY hotkey, bool toggle)
//{
//    bool wasDown = hotkey.wasDown;
//    std::vector<short> keyList = hotkey.hotkey;
//    if (keyList.size() <= 0) return false;
//
//    // Loop through each key and return if any aren't pressed
//    for (const auto& vkCode : keyList) {
//        if (!(GetAsyncKeyState(vkCode) & 0x8000)) {
//            // Key is up
//            // Return false as soon as one key is up
//            if (hotkey.label == L"Load")
//                UpdateStatusText(statusTextHWND, "Hotkey isn't active", { 255, 50, 50 });
//            hotkey.wasDown = false;
//            return false;
//        }
//    }
//
//    //hotkey.wasDown = hotkey.wasDown; // Flag for future checks
//
//    // if hotkey was down and toggle is on, return false anyway
//    //if (hotkey.wasDown && toggle) {
//    if (wasDown && toggle) {
//        if (hotkey.label == L"Load")
//            UpdateStatusText(statusTextHWND, "Hotkey is active but toggled", { 0, 50, 255 });
//        return false;
//    }
//
//    if (hotkey.label == L"Load")
//        UpdateStatusText(statusTextHWND, "Hotkey is active", { 50, 255, 50 });
//    // Loop finished, all keys are down
//    hotkey.wasDown = true; // Flag for future checks
//    return true; // Return true if all keys are pressed
//}
//----------------------------------------------------------------

//DWORD WINAPI threadUpdate(LPVOID lpParameter) {
//    while (true) {
//        Update();
//        Sleep(10); // Need to set this as a variable that can be changed in the settings
//    }
//    return 0;
//}

// Runs continually on WM_TIMER
void Update()
{
    if (save1.IsKeyDown())
        SavePosition(savePos1);
    if (load1.IsKeyDown())
        LoadPosition(savePos1);
    if (save2.IsKeyDown())
        SavePosition(savePos2);
    if (load2.IsKeyDown())
        LoadPosition(savePos2);
    if (save3.IsKeyDown())
        SavePosition(savePos3);
    if (load3.IsKeyDown())
        LoadPosition(savePos3);
    if (save4.IsKeyDown())
        SavePosition(savePos4);
    if (load4.IsKeyDown())
        LoadPosition(savePos4);

    if (godHotkey.IsKeyDown(true))
        healthToggle.toggleBytes();

    if (hoverHotkey.IsKeyDown(true))
        hoverToggle.toggleBytes();

    if (flyToggleHotkey.IsKeyDown(true))
        flying = !flying;

    

    //If rewinding, load next position
    if (rewindHotkey.IsKeyDown(false))
    {
        //UpdateStatusText(hWnd, "Rewinding", { 0,100,0 });
        //MessageBox(NULL, L"Rewind hotkey pressed!", L"MyApp", MB_OK);
        if (rewindHistory.size() > 1) {
            SAVEDATA rewind = { 0 };
            rewind = rewindHistory.back();
            rewindHistory.pop_back();
            LoadPosition(rewind);
        }
    }
    //Else, not rewinding, save current position in rewind vector
    else {
        if (rewindState == rewindSpeed) {
            SAVEDATA rewind = { 0 };
            SavePosition(rewind);
            rewindHistory.push_back(rewind);
            if (rewindHistory.size() > 10000) {
                rewindHistory.erase(rewindHistory.begin()); //if rewind history gets too long
            }
            rewindState = 0;
        }
        else {
            rewindState++;
        }
    }

    //Flying
    if (flying)
    {
        //if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        //if (IsKeyDown(jumpHotkey, true))
        //if (IsKeyDown(jumpHotkey))
        if (jumpHotkey.IsKeyDown(false))
        {
            //if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            //if (IsKeyDown(runHotkey))
            if (runHotkey.IsKeyDown(false))
            {
                PlayerVelocityZ.WriteFloat(1000.0f);
            }
            else
                PlayerVelocityZ.WriteFloat(300.0f);

            //flyToggle.toggleBytes();
        }

        //if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        //if (IsKeyDown(runHotkey))
        if (runHotkey.IsKeyDown(false))
        {
            PlayerVelocityX.WriteFloat(PlayerVelocityX.ReadFloat() * 1.2f);
            PlayerVelocityY.WriteFloat(PlayerVelocityY.ReadFloat() * 1.2f);
        }
        else
        {
            PlayerVelocityX.WriteFloat(SmoothClamp(PlayerVelocityX.ReadFloat(), -500.0f, 500.0f, 0.8f));
            PlayerVelocityY.WriteFloat(SmoothClamp(PlayerVelocityY.ReadFloat(), -500.0f, 500.0f, 0.8f));
        }
    }
}
