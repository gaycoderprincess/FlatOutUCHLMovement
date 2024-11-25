#include <windows.h>
#include <d3d9.h>
#include <format>

#include "nya_dx9_hookbase.h"
#include "nya_commonmath.h"
#include "nya_commonhooklib.h"

#include "fouc.h"
#include "chloemenulib.h"
#include "freemanapi.h"

bool ShouldRunMovement() {
	if (!FreemanAPI::GetIsEnabled()) return false;
	if (!pGameFlow) return false;
	if (pGameFlow->nGameState != GAME_STATE_RACE) return false;
	if (pGameFlow->nRaceState != RACE_STATE_RACING) return false;
	auto ply = GetPlayer(0);
	if (!ply) return false;
	if (ply->pCar->nIsRagdolled) return false;
	return true;
}

#include "cam.h"
#include "hl_game_fo2_ext.h"

void MenuLoop() {
	ChloeMenuLib::BeginMenu();
	FreemanAPI::ProcessChloeMenu();
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
	if (!FreemanAPI::GetIsEnabled()) {
		FO2Cam::nLastGameState = -1;
		return;
	}
	FO2Cam::Process(cam);
}

void __fastcall ProcessPlayerCar(Player* pPlayer) {
	if (!ShouldRunMovement()) return;
	FreemanAPI::Process(0.01);
}

void UpdateCodePatches() {
	if (FreemanAPI::GetIsEnabled() && pGameFlow->nDerbyType == DERBY_NONE) {
		// disable ragdolling
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x427EAF, 0x427FCF);
	}
	else {
		NyaHookLib::Patch<uint64_t>(0x427EAF, 0x99390000011A840F);
	}

	// disable autoreset & resetmap
	NyaHookLib::Patch<uint8_t>(0x4D8460, FreemanAPI::GetIsEnabled() ? 0xEB : 0x77);
	NyaHookLib::Patch<uint8_t>(0x43D69E, FreemanAPI::GetIsEnabled() ? 0xEB : 0x75);
}

void HookLoop() {
	UpdateCodePatches();

	if (!ShouldRunMovement()) return;

	if (IsKeyJustPressed('V')) {
		FreemanAPI::ToggleNoclip();
	}

	tNyaStringData data;
	data.x = 0.5;
	data.y = 0.9;
	data.size = 0.04;
	data.XCenterAlign = true;

	if (bShowVelocity) {
		DrawString(data, std::format("{:.1f}", FreemanAPI::GetPlayerVelocity()));
		data.y += data.size;
	}
	if (bShowVelocity2D) {
		DrawString(data, std::format("{:.1f}", FreemanAPI::GetPlayerVelocity2D()));
		data.y += data.size;
	}

	CommonMain();
}

bool bDeviceJustReset = false;
void D3DHookMain() {
	if (!g_pd3dDevice) {
		UpdateD3DProperties();
		InitHookBase();
	}

	if (bDeviceJustReset) {
		ImGui_ImplDX9_CreateDeviceObjects();
		bDeviceJustReset = false;
	}
	HookBaseLoop();
}

void OnD3DReset() {
	if (g_pd3dDevice) {
		UpdateD3DProperties();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		bDeviceJustReset = true;
	}
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
			if (FreemanAPI::GetIsEnabled()) {
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

uintptr_t ProcessPlayerCarsASM_call = 0x478CF0;
void __attribute__((naked)) ProcessPlayerCarsASM() {
	__asm__ (
		"pushad\n\t"
		"mov ecx, esi\n\t"
		"call %1\n\t"
		"popad\n\t"
		"jmp %0\n\t"
			:
			:  "m" (ProcessPlayerCarsASM_call), "i" (ProcessPlayerCar)
	);
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
			NyaFO2Hooks::aEndSceneFuncs.push_back(D3DHookMain);
			NyaFO2Hooks::aD3DResetFuncs.push_back(OnD3DReset);
			if (*(uint64_t*)0x4F49F6 == 0x006A000001B4838B) {
				NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x4F49F6, &DrawHook);
			}
			NyaFO2Hooks::PlaceWndProcHook();
			NyaFO2Hooks::aWndProcFuncs.push_back(MouseWndProc);

			UpdateCameraHooked_call = (void(__thiscall*)(void*, float))(*(uintptr_t*)0x6EB7DC);
			NyaHookLib::Patch(0x6EB7DC, &UpdateCameraHooked);

			ProcessPlayerCarsASM_call = NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x47A010, &ProcessPlayerCarsASM);

			RegisterHLMovement();
		} break;
		default:
			break;
	}
	return TRUE;
}