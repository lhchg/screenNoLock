#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include <thread>

//#define DEBUG


HMENU hPopupMenu;
HWND hWnd;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_USER + 1: 
		switch (lParam) {
		case WM_RBUTTONDOWN:
		{
			POINT pt;
			GetCursorPos(&pt);

			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
		}
		break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:
			DestroyWindow(hwnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}


void clickEvent() {
	LASTINPUTINFO lastInputInfo;
	lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

	DWORD lockTime = 0;
	DWORD remainingTime = 0;
	SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &lockTime, 0);

	int flag = 0;
	DWORD idleTime = 0;
	while (true) {

		if (GetLastInputInfo(&lastInputInfo)) {
			idleTime = GetTickCount64() - lastInputInfo.dwTime;
			remainingTime = lockTime * 1000 - idleTime;
			std::cout << "Remaining lock screen time: " << remainingTime / 1000 << " seconds" << std::endl;
		}
		else {
			std::cout << "Failed to retrieve last input info." << std::endl;
		}
		Sleep(1000);

		if (remainingTime / 1000 <= 60) {
			POINT pt;
			GetCursorPos(&pt);

			mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
			std::cout << "move !" << std::endl;
		}

	}
}

void moveEvent() {
	DWORD idleTime = 0;
	DWORD remainingTime = 0;

	LASTINPUTINFO lastInputInfo;
	lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

	DWORD lockTime = 0;
	SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &lockTime, 0);

	while (true) {
		if (GetLastInputInfo(&lastInputInfo)) {
			idleTime = GetTickCount64() - lastInputInfo.dwTime;
			remainingTime = lockTime * 1000 - idleTime;
			std::cout << "Remaining lock screen time: " << remainingTime / 1000 << " seconds" << std::endl;
		}
		else {
			std::cout << "Failed to retrieve last input info." << std::endl;
		}
		Sleep(1000);

		if (remainingTime / 1000 <= 60){
			INPUT inputs[1] = {};
			ZeroMemory(inputs, sizeof(inputs));

			MOUSEINPUT mouseInput;
			mouseInput.dx = -1;
			mouseInput.dy = 0;
			mouseInput.mouseData = 0;
			mouseInput.dwFlags = MOUSEEVENTF_MOVE;
			mouseInput.time = 0;
			mouseInput.dwExtraInfo = 0;

			INPUT input[1];
			input[0].type = INPUT_MOUSE;
			input[0].mi = mouseInput;

			UINT uSent = SendInput(1, input, sizeof(INPUT));
			if (uSent != ARRAYSIZE(inputs))
			{
				std::cout << "SendInput failed : " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
			}
			Sleep(100);

			mouseInput.dx = 1;
			input[0].mi = mouseInput;
			uSent = SendInput(1, input, sizeof(INPUT));
			if (uSent != ARRAYSIZE(inputs))
			{
				std::cout << "SendInput failed : " << HRESULT_FROM_WIN32(GetLastError()) << std::endl;
			}
			Sleep(100);
			std::cout << "move !" << std::endl;
		}
	}
}

int main() {
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = "ScreenNoLock";
	RegisterClass(&wc);

	HWND hwnd = CreateWindow(wc.lpszClassName, NULL, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, wc.hInstance, NULL);

	ShowWindow(hwnd, SW_HIDE);

	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	lstrcpy(nid.szTip, "ScreenNoLock");
	Shell_NotifyIcon(NIM_ADD, &nid);

	hPopupMenu = CreatePopupMenu();
	AppendMenu(hPopupMenu, MF_STRING, 1, "Exit");

	hWnd = hwnd;

	MSG msg;

	std::thread t(moveEvent);
	t.detach();

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Shell_NotifyIcon(NIM_DELETE, &nid);

	DestroyMenu(hPopupMenu);

	return 0;
}
