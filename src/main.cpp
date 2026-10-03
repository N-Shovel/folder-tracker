// Folder Tracker: a floating bubble that shows which project folders are on GitHub,
// git only, or not using git at all.
#include <windows.h>
#include <objbase.h>

#include "Messages.h"
#include "core/Workspace.h"
#include "platform/Settings.h"
#include "ui/BubbleWindow.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Only one Folder Tracker at a time. Opening it again opens the running one's panel.
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"FolderTracker.SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND running = FindWindowW(kWindowClass, nullptr)) {
            DWORD processId = 0;
            GetWindowThreadProcessId(running, &processId);
            AllowSetForegroundWindow(processId);
            PostMessageW(running, WM_SHOW_PANEL, 0, 0);
        }
        return 0;
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int exitCode = 0;
    {
        Settings settings;
        Workspace workspace(settings);
        BubbleWindow window(settings, workspace);
        if (window.create(instance)) {
            workspace.loadSaved();
            MSG message;
            while (GetMessageW(&message, nullptr, 0, 0)) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
        } else {
            exitCode = 1;
        }
    }
    CoUninitialize();
    CloseHandle(singleInstance);
    return exitCode;
}
