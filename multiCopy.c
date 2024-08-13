#include <windows.h>
#include <stdio.h>
#include <string.h>

#define MAX_TEXT_LENGTH 1024

BOOL ctrlPressed = FALSE;
BOOL bPressed = FALSE;
BOOL mPressed = FALSE;
char clipboardTexts[10][MAX_TEXT_LENGTH] = {0};

void SimulateCopy()
{
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event('C', 0, 0, 0);
    keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

void SimulatePaste()
{
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event('V', 0, 0, 0);
    keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

void SaveClipboardText(int index)
{
    if (!OpenClipboard(NULL)) return;
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) return;
    char *pszText = (char*)GlobalLock(hData);
    if (pszText == NULL) return;
    strncpy(clipboardTexts[index], pszText, MAX_TEXT_LENGTH - 1);
    clipboardTexts[index][MAX_TEXT_LENGTH - 1] = '\0';
    GlobalUnlock(hData);
    CloseClipboard();
}

void PasteClipboardText(int index)
{
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, strlen(clipboardTexts[index]) + 1);
        if (hGlob != NULL)
        {
            memcpy(hGlob, clipboardTexts[index], strlen(clipboardTexts[index]) + 1);
            SetClipboardData(CF_TEXT, hGlob);
        }
        CloseClipboard();
        Sleep(100); // waiting for the clipboard data to be set
        SimulatePaste();
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            if (GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_RCONTROL) & 0x8000) ctrlPressed = TRUE;
            if (pKeyBoard->vkCode == 'B') bPressed = TRUE;
            if (pKeyBoard->vkCode == 'M') mPressed = TRUE;

            if (ctrlPressed && bPressed && pKeyBoard->vkCode >= '1' && pKeyBoard->vkCode <= '9')
            {
                int index = pKeyBoard->vkCode - '1';
                SimulateCopy();
                Sleep(100);
                SaveClipboardText(index);
                return 1;
            }

            if (ctrlPressed && mPressed && pKeyBoard->vkCode >= '1' && pKeyBoard->vkCode <= '9')
            {
                int index = pKeyBoard->vkCode - '1';
                PasteClipboardText(index);
                return 1;
            }
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
        {
            if (pKeyBoard->vkCode == VK_CONTROL || pKeyBoard->vkCode == VK_RCONTROL) ctrlPressed = FALSE;
            if (pKeyBoard->vkCode == 'B') bPressed = FALSE;
            if (pKeyBoard->vkCode == 'M') mPressed = FALSE;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hHook == NULL)
    {
        printf("Error: Failed to install hook! Make sure you compiled the program with -lgdi32\n");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
