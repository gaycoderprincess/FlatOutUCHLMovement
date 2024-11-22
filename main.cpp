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
	if (pGameFlow->nRaceState == RACE_STATE_RACING) {
		HLMovement::Process(gTimer.fDeltaTime);
		FO2Cam::Process(cam);
	}
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

void LoadConfig() {
	auto config = toml::parse_file("FlatOutUCHLMovement_gcp.toml");
	HLMovement::bEnabled = config["main"]["enabled"].value_or(HLMovement::bEnabled);
	HLMovement::bCanLongJump = config["main"]["longjump"].value_or(HLMovement::bCanLongJump);
	HLMovement::bAutoHop = config["main"]["autohop"].value_or(HLMovement::bAutoHop);
	HLMovement::bABH = config["main"]["abh"].value_or(HLMovement::bABH);
	HLMovement::bABHMixed = config["main"]["abh_mixed"].value_or(HLMovement::bABHMixed);
	HLMovement::bBhopCap = config["main"]["bhop_cap"].value_or(HLMovement::bBhopCap);
	HLMovement::bSmartVelocityCap = config["main"]["better_maxvelocity"].value_or(HLMovement::bSmartVelocityCap);
	HLMovement::bNoclipKey = config["main"]["noclip_toggle"].value_or(HLMovement::bNoclipKey);

	HLMovement::bTeleportCar = config["game"]["teleport_car"].value_or(HLMovement::bTeleportCar);
	HLMovement::bCarGodmode = config["game"]["car_godmode"].value_or(HLMovement::bCarGodmode);

	HLMovement::nPhysicsSteps = config["advanced"]["physics_steps"].value_or(HLMovement::nPhysicsSteps);
	HLMovement::nColDensity = config["advanced"]["collision_density"].value_or(HLMovement::nColDensity);

	FO2Cam::fFOV = config["cvars"]["fov_desired"].value_or(FO2Cam::fFOV);
	FO2Cam::fSensitivity = config["cvars"]["sensitivity"].value_or(FO2Cam::fSensitivity);
	HLMovement::fSoundVolume = config["cvars"]["volume"].value_or(HLMovement::fSoundVolume);
	HLMovement::cl_bob = config["cvars"]["cl_bob"].value_or(HLMovement::cl_bob);
	HLMovement::cl_bobcycle = config["cvars"]["cl_bobcycle"].value_or(HLMovement::cl_bobcycle);
	HLMovement::cl_bobup = config["cvars"]["cl_bobup"].value_or(HLMovement::cl_bobup);
	HLMovement::cl_forwardspeed = config["cvars"]["cl_forwardspeed"].value_or(HLMovement::cl_forwardspeed);
	HLMovement::cl_sidespeed = config["cvars"]["cl_sidespeed"].value_or(HLMovement::cl_sidespeed);
	HLMovement::cl_upspeed = config["cvars"]["cl_upspeed"].value_or(HLMovement::cl_upspeed);
	HLMovement::cl_movespeedkey = config["cvars"]["cl_movespeedkey"].value_or(HLMovement::cl_movespeedkey);
	HLMovement::sv_gravity = config["cvars"]["sv_gravity"].value_or(HLMovement::sv_gravity);
	HLMovement::sv_stopspeed = config["cvars"]["sv_stopspeed"].value_or(HLMovement::sv_stopspeed);
	HLMovement::sv_maxspeed = config["cvars"]["sv_maxspeed"].value_or(HLMovement::sv_maxspeed);
	HLMovement::sv_noclipspeed = config["cvars"]["sv_noclipspeed"].value_or(HLMovement::sv_noclipspeed);
	HLMovement::sv_accelerate = config["cvars"]["sv_accelerate"].value_or(HLMovement::sv_accelerate);
	HLMovement::sv_airaccelerate = config["cvars"]["sv_airaccelerate"].value_or(HLMovement::sv_airaccelerate);
	HLMovement::sv_wateraccelerate = config["cvars"]["sv_wateraccelerate"].value_or(HLMovement::sv_wateraccelerate);
	HLMovement::sv_friction = config["cvars"]["sv_friction"].value_or(HLMovement::sv_friction);
	HLMovement::sv_edgefriction = config["cvars"]["sv_edgefriction"].value_or(HLMovement::sv_edgefriction);
	HLMovement::sv_waterfriction = config["cvars"]["sv_waterfriction"].value_or(HLMovement::sv_waterfriction);
	//HLMovement::sv_entgravity = config["cvars"]["sv_entgravity"].value_or(HLMovement::sv_entgravity);
	HLMovement::sv_bounce = config["cvars"]["sv_bounce"].value_or(HLMovement::sv_bounce);
	HLMovement::sv_stepsize = config["cvars"]["sv_stepsize"].value_or(HLMovement::sv_stepsize);
	HLMovement::sv_maxvelocity = config["cvars"]["sv_maxvelocity"].value_or(HLMovement::sv_maxvelocity);
	//HLMovement::mp_footsteps = config["cvars"]["mp_footsteps"].value_or(HLMovement::mp_footsteps);
	HLMovement::sv_rollangle = config["cvars"]["sv_rollangle"].value_or(HLMovement::sv_rollangle);
	HLMovement::sv_rollspeed = config["cvars"]["sv_rollspeed"].value_or(HLMovement::sv_rollspeed);
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

			LoadConfig();
		} break;
		default:
			break;
	}
	return TRUE;
}