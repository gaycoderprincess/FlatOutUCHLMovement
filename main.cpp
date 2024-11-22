#include <windows.h>
#include <d3d9.h>
#include <format>
#include "toml++/toml.hpp"

#include "nya_dx9_hookbase.h"
#include "nya_commonmath.h"
#include "nya_commonhooklib.h"

#include "fouc.h"
#include "chloemenulib.h"

void ValueEditorMenu(float& value) {
	ChloeMenuLib::BeginMenu();

	static char inputString[1024] = {};
	ChloeMenuLib::AddTextInputToString(inputString, 1024, true);
	ChloeMenuLib::SetEnterHint("Apply");

	if (DrawMenuOption(inputString + (std::string)"...", "", false, false) && inputString[0]) {
		value = std::stof(inputString);
		memset(inputString,0,sizeof(inputString));
		ChloeMenuLib::BackOut();
	}

	ChloeMenuLib::EndMenu();
}

void ValueEditorMenu(int& value) {
	ChloeMenuLib::BeginMenu();

	static char inputString[1024] = {};
	ChloeMenuLib::AddTextInputToString(inputString, 1024, true);
	ChloeMenuLib::SetEnterHint("Apply");

	if (DrawMenuOption(inputString + (std::string)"...", "", false, false) && inputString[0]) {
		value = std::stoi(inputString);
		memset(inputString,0,sizeof(inputString));
		ChloeMenuLib::BackOut();
	}

	ChloeMenuLib::EndMenu();
}

void ValueEditorMenu(bool& value, const std::string& name) {
	if (DrawMenuOption(std::format("{} - {}", name, value), "")) {
		value = !value;
	}
}

void ValueEditorMenu(float& value, const std::string& name) {
	if (DrawMenuOption(std::format("{} - {}", name, value), "")) {
		ValueEditorMenu(value);
	}
}

void ValueEditorMenu(int& value, const std::string& name) {
	if (DrawMenuOption(std::format("{} - {}", name, value), "")) {
		ValueEditorMenu(value);
	}
}

#include "cam.h"
#include "hlmov.h"

void HookLoop() {}

void MenuLoop() {
	ChloeMenuLib::BeginMenu();
	HLMovement::ProcessMenu();
	ChloeMenuLib::EndMenu();
}

void UpdateD3DProperties() {
	g_pd3dDevice = pDeviceD3d->pD3DDevice;
	ghWnd = pDeviceD3d->hWnd;
	nResX = nGameResolutionX;
	nResY = nGameResolutionY;
}

void RunMovement(Camera* cam) {
	if (!cam) return;
	if (!HLMovement::bEnabled) return;

	if (FO2Cam::nLastGameState != pGameFlow->nRaceState) {
		HLMovement::Reset();
	}

	static CNyaTimer gTimer;
	gTimer.Process();
	HLMovement::Process(gTimer.fDeltaTime);

	FO2Cam::Process(cam);
}

void MainLoop() {
	UpdateD3DProperties();

	CNyaTimer gTimer;
	gTimer.Process();

	if (!pGameFlow) return;

	if (IsKeyJustPressed('V')) {
		HLMovement::ToggleNoclip();
	}

	CommonMain(false);
}

uintptr_t DrawHook_jmp = 0x4F49FF;
void __attribute__((naked)) DrawHook() {
	__asm__ (
		"mov eax, [ebx+0x1B4]\n\t"
		"push 0\n\t"
		"push eax\n\t"
		"jmp %0\n\t"
			:
			:  "m" (DrawHook_jmp)
	);
}

auto UpdateCameraHooked_call = (void(__thiscall*)(void*, float))0x4FAEA0;
void __fastcall UpdateCameraHooked(void* a1, void*, float a2) {
	UpdateCameraHooked_call(a1, a2);
	if (pCameraManager) RunMovement(pCameraManager->pCamera);
}

void MouseWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static bool bOnce = true;
	if (bOnce) {
		RAWINPUTDEVICE device;
		device.usUsagePage = 1;
		device.usUsage = 2;
		device.dwFlags = RIDEV_INPUTSINK;
		device.hwndTarget = hWnd;
		RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE));
		bOnce = false;
	}

	switch (msg) {
		case WM_MOUSEWHEEL:
			nMouseWheelState += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			break;
		case WM_INPUT: {
			if (HLMovement::bEnabled) {
				RAWINPUT raw;
				UINT size = sizeof(raw);
				GetRawInputData((HRAWINPUT) lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
				if (raw.header.dwType != RIM_TYPEMOUSE) return;
				FO2Cam::fMouse[0] += raw.data.mouse.lLastX;
				FO2Cam::fMouse[1] += raw.data.mouse.lLastY;
			}
		} break;
		default:
			break;
	}
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x24CEF7) {
				MessageBoxA(nullptr, aFOUCVersionFail, "nya?!~", MB_ICONERROR);
				exit(0);
				return TRUE;
			}

			ChloeMenuLib::RegisterMenu("Half-Life Movement - gaycoderprincess", MenuLoop);
			NyaFO2Hooks::PlaceD3DHooks();
			NyaFO2Hooks::aEndSceneFuncs.push_back(MainLoop);
			if (*(uint64_t*)0x4F49F6 == 0x006A000001B4838B) {
				NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4F49F6, &DrawHook);
			}
			NyaFO2Hooks::PlaceWndProcHook();
			NyaFO2Hooks::aWndProcFuncs.push_back(MouseWndProc);

			UpdateCameraHooked_call = (void(__thiscall*)(void*, float))(*(uintptr_t*)0x6EB7DC);
			NyaHookLib::Patch(0x6EB7DC, &UpdateCameraHooked);
		} break;
		default:
			break;
	}
	return TRUE;
}