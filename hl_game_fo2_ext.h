// HL to FO2 integration
bool bTeleportCar = false;
bool bCarGodmode = false;
bool bShowVelocity = false;
bool bShowVelocity2D = false;

struct tFO2MaterialMatchup {
	int surfaceId;
	int footstepId;
};
tFO2MaterialMatchup aSurfaces[] = {
		{ 1, FreemanAPI::CHAR_TEX_CONCRETE }, // NoCollision
		{ 2, FreemanAPI::CHAR_TEX_CONCRETE }, // Tarmac (Road)
		{ 3, FreemanAPI::CHAR_TEX_CONCRETE }, // Tarmac Mark (Road)
		{ 4, FreemanAPI::CHAR_TEX_CONCRETE }, // Asphalt (Road)
		{ 5, FreemanAPI::CHAR_TEX_CONCRETE }, // Asphalt Mark (Road)
		{ 6, FreemanAPI::CHAR_TEX_CONCRETE }, // Cement Mark (Road)
		{ 7, FreemanAPI::CHAR_TEX_CONCRETE }, // Cement Mark (Road)
		{ 8, FreemanAPI::CHAR_TEX_GRAVEL }, // Hard (Road)
		{ 9, FreemanAPI::CHAR_TEX_GRAVEL }, // Hard Mark (Road)
		{ 10, FreemanAPI::CHAR_TEX_GRAVEL }, // Medium (Road)
		{ 11, FreemanAPI::CHAR_TEX_GRAVEL }, // Medium Mark (Road)
		{ 12, FreemanAPI::CHAR_TEX_GRAVEL }, // Soft (Road)
		{ 13, FreemanAPI::CHAR_TEX_GRAVEL }, // Soft Mark (Road)
		{ 14, FreemanAPI::CHAR_TEX_GRAVEL }, // Derby Gravel (Road)
		{ 15, FreemanAPI::CHAR_TEX_CONCRETE }, // Derby Tarmac (Road)
		{ 16, FreemanAPI::CHAR_TEX_SAND }, // Snow (Road)
		{ 17, FreemanAPI::CHAR_TEX_SAND }, // Snow Mark (Road)
		{ 18, FreemanAPI::CHAR_TEX_DIRT }, // Dirt (Road)
		{ 19, FreemanAPI::CHAR_TEX_DIRT }, // Dirt Mark (Road)
		{ 20, FreemanAPI::CHAR_TEX_METAL }, // Bridge Metal (Road)
		{ 21, FreemanAPI::CHAR_TEX_WOOD }, // Bridge Wooden (Road)
		{ 22, FreemanAPI::CHAR_TEX_CONCRETE }, // Curb (Terrain)
		{ 23, FreemanAPI::CHAR_TEX_SAND }, // Bank Sand (terrain)
		{ 24, FreemanAPI::CHAR_TEX_GRASS }, // Grass (terrain)
		{ 25, FreemanAPI::CHAR_TEX_GRASS }, // Forest (terrain)
		{ 26, FreemanAPI::CHAR_TEX_SAND }, // Sand (terrain)
		{ 27, FreemanAPI::CHAR_TEX_CONCRETE }, // Rock (terrain)
		{ 28, FreemanAPI::CHAR_TEX_GRASS }, // Mould (terrain)
		{ 29, FreemanAPI::CHAR_TEX_SAND }, // Snow  (terrain)
		{ 30, FreemanAPI::CHAR_TEX_GRASS }, // Field  (terrain)
		{ 31, FreemanAPI::CHAR_TEX_SLOSH }, // Wet  (terrain)
		{ 32, FreemanAPI::CHAR_TEX_CONCRETE }, // Concrete (Object)
		{ 33, FreemanAPI::CHAR_TEX_CONCRETE }, // Rock (Object)
		{ 34, FreemanAPI::CHAR_TEX_METAL }, // Metal (Object)
		{ 35, FreemanAPI::CHAR_TEX_WOOD }, // Wood (Object)
		{ 36, FreemanAPI::CHAR_TEX_WOOD }, // Tree (Object)
		{ 37, FreemanAPI::CHAR_TEX_CONCRETE }, // Bush
		{ 38, FreemanAPI::CHAR_TEX_CONCRETE }, // Rubber (Object)
		{ 39, FreemanAPI::CHAR_TEX_SLOSH }, // Water (water)
		{ 40, FreemanAPI::CHAR_TEX_SLOSH }, // River (water)
		{ 41, FreemanAPI::CHAR_TEX_SLOSH }, // Puddle (water)
		{ 42, FreemanAPI::CHAR_TEX_CONCRETE }, // No Camera Col
		{ 43, FreemanAPI::CHAR_TEX_CONCRETE }, // Camera only col
		{ 44, FreemanAPI::CHAR_TEX_CONCRETE }, // Reset
		{ 45, FreemanAPI::CHAR_TEX_CONCRETE }, // Stunt Conveyer
		{ 46, FreemanAPI::CHAR_TEX_CONCRETE }, // Stunt Bouncer
		{ 47, FreemanAPI::CHAR_TEX_CONCRETE }, // Stunt Curling
		{ 48, FreemanAPI::CHAR_TEX_CONCRETE }, // Stunt Bowling
		{ 49, FreemanAPI::CHAR_TEX_CONCRETE }, // Stunt Tarmac
		{ 50, FreemanAPI::CHAR_TEX_CONCRETE }, // Oil (Road)
};
int GetSurfaceTextureFromID(int id) {
	for (auto& surf : aSurfaces) {
		if (surf.surfaceId == id + 1) return surf.footstepId;
	}
	return FreemanAPI::CHAR_TEX_CONCRETE;
}

std::string GetSpeechPath(const std::string& file) {
	return "data/sound/hl/" + file;
}

std::vector<NyaAudio::NyaSound> aSoundCache;
void ProcessSoundCache() {
	int i = 0;
	for (auto& sound : aSoundCache) {
		if (NyaAudio::IsFinishedPlaying(sound)) {
			auto tmp = sound;
			NyaAudio::Delete(&tmp);
			aSoundCache.erase(aSoundCache.begin() + i);
			return ProcessSoundCache();
		}
		i++;
	}
}

float fSoundVolume = 1;
void PlayGameSound(const char* path, float volume) {
	NyaAudio::Init(pDeviceD3d->hWnd);
	ProcessSoundCache();

	auto sound = NyaAudio::LoadFile(GetSpeechPath(path).c_str());
	if (!sound) return;
	NyaAudio::SetVolume(sound, volume * fSoundVolume);
	NyaAudio::Play(sound);
	aSoundCache.push_back(sound);
}

bool GetGamePlayerDead() {
	if (auto ply = GetPlayer(0)) {
		return ply->nIsWrecked;
	}
	return false;
}

void GetGamePlayerPosition(double* out) {
	out[0] = out[1] = out[2] = 0;
	if (auto ply = GetPlayer(0)) {
		out[0] = ply->pCar->GetMatrix()->p.x;
		out[1] = ply->pCar->GetMatrix()->p.y + 1.5;
		out[2] = ply->pCar->GetMatrix()->p.z;
	}
}

void GetGamePlayerVelocity(double* out) {
	out[0] = out[1] = out[2] = 0;
	if (auto ply = GetPlayer(0)) {
		out[0] = ply->pCar->GetVelocity()->x;
		out[1] = ply->pCar->GetVelocity()->y;
		out[2] = ply->pCar->GetVelocity()->z;
	}
}

void GetGamePlayerViewAngle(double* out) {
	out[0] = FO2Cam::vAngle.x / (std::numbers::pi / 180.0);
	out[1] = FO2Cam::vAngle.y / (std::numbers::pi / 180.0);
	out[2] = FO2Cam::vAngle.z / (std::numbers::pi / 180.0);
}

void SetGamePlayerPosition(const double* in, const double* inVelocity) {
	if (!bTeleportCar) return;

	if (auto ply = GetPlayer(0)) {
		ply->pCar->GetMatrix()->p.x = in[0];
		ply->pCar->GetMatrix()->p.y = in[1] - 1.5;
		ply->pCar->GetMatrix()->p.z = in[2];
		ply->pCar->GetVelocity()->x = inVelocity[0];
		ply->pCar->GetVelocity()->y = inVelocity[1];
		ply->pCar->GetVelocity()->z = inVelocity[2];
		*ply->pCar->GetAngVelocity() = {0,0,0};
		if (bCarGodmode) {
			ply->pCar->fDamage = 0;
		}
	}
}

void SetGamePlayerViewPosition(const double* in) {
	FO2Cam::vPos.x = in[0];
	FO2Cam::vPos.y = in[1];
	FO2Cam::vPos.z = in[2];
}

void SetGamePlayerViewAngle(const double* in) {
	FO2Cam::vAngleView.x = in[0] * (std::numbers::pi / 180.0);
	FO2Cam::vAngleView.y = in[1] * (std::numbers::pi / 180.0);
	FO2Cam::vAngleView.z = in[2] * (std::numbers::pi / 180.0);

	if (bTeleportCar) {
		if (auto ply = GetPlayer(0)) {
			int PITCH, ROLL;
			FreemanAPI::GetRotateOrder(&PITCH, nullptr, &ROLL);

			auto angle = FO2Cam::vAngleView;
			angle[PITCH] = 0;
			angle[ROLL] = 0;
			auto mat = ply->pCar->GetMatrix();
			auto oldPos = mat->p;
			mat->SetIdentity();
			mat->Rotate({-angle[1], angle[2], angle[0]});
			mat->p = oldPos;
			FO2MatrixToQuat(&mat->x.x, &ply->pCar->qQuaternion[0]);
		}
	}
}

int GetPointContents(const double* point) {
	if (pEnvironment && pEnvironment->bWaterPlane && point[1] <= 0) {
		return FreemanAPI::CONTENTS_WATER;
	}

	return FreemanAPI::CONTENTS_EMPTY;
}

FreemanAPI::pmtrace_t* PointRaytrace(const double* _origin, const double* _end) {
	auto origin = *(NyaVec3Double*)_origin;
	auto end = *(NyaVec3Double*)_end;

	static FreemanAPI::pmtrace_t trace;
	trace.Default();
	trace.endpos[0] = end[0];
	trace.endpos[1] = end[1];
	trace.endpos[2] = end[2];

	if (pGameFlow->nGameState == GAME_STATE_RACE) {
		tLineOfSightIn prop;

		NyaVec3Double dir = end - origin;
		auto dist = dir.length();
		dir.Normalize();

		auto originf = NyaVec3(origin.x, origin.y, origin.z);
		auto dirf = NyaVec3(dir.x, dir.y, dir.z);

		prop.fMaxDistance = dist;
		prop.bGetClosestHit = true;

		tLineOfSightOut out;
		if (CheckLineOfSight(&prop, pGameFlow->pHost->pUnkForLOS, &originf, &dirf, &out)) {
			trace.plane.dist = out.fHitDistance;
			trace.plane.normal[0] = out.vHitNormal[0];
			trace.plane.normal[1] = out.vHitNormal[1];
			trace.plane.normal[2] = out.vHitNormal[2];
			trace.fraction = out.fHitDistance / dist;
			if (trace.fraction > 1.0f) trace.fraction = 1.0f;
			auto endpos = origin + (dir * out.fHitDistance);
			trace.endpos[0] = endpos[0];
			trace.endpos[1] = endpos[1];
			trace.endpos[2] = endpos[2];
			trace.ent = 0; // dummy entity
			trace.surfaceId = GetSurfaceTextureFromID(out.nSurfaceId);
		}
	}
	return &trace;
}

float GetGameMoveLeftRight() {
	float lr = 0;
	if (IsKeyPressed('A')) lr -= 1;
	if (IsKeyPressed('D')) lr += 1;
	return lr;
}

float GetGameMoveFwdBack() {
	float lr = 0;
	if (IsKeyPressed('W')) lr += 1;
	if (IsKeyPressed('S')) lr -= 1;
	return lr;
}

float GetGameMoveUpDown() {
	return 0;
}

bool GetGameMoveJump() {
	return IsKeyPressed(VK_SPACE);
}

bool GetGameMoveDuck() {
	return IsKeyPressed(VK_LCONTROL);
}

bool GetGameMoveRun() {
	return IsKeyPressed(VK_LSHIFT);
}

bool GetGameMoveUse() {
	return IsKeyPressed('E');
}

void RegisterHLMovement() {
	// params
	FreemanAPI::SetIsZUp(false);
	FreemanAPI::SetConvertUnits(true);
	FreemanAPI::SetRotateOrder(1, 0, 2);

	// game funcs
	FreemanAPI::Register_PlayGameSound(PlayGameSound);
	FreemanAPI::Register_GetGamePlayerDead(GetGamePlayerDead);
	FreemanAPI::Register_GetGamePlayerPosition(GetGamePlayerPosition);
	FreemanAPI::Register_GetGamePlayerVelocity(GetGamePlayerVelocity);
	FreemanAPI::Register_GetGamePlayerViewAngle(GetGamePlayerViewAngle);
	FreemanAPI::Register_SetGamePlayerPosition(SetGamePlayerPosition);
	FreemanAPI::Register_SetGamePlayerViewPosition(SetGamePlayerViewPosition);
	FreemanAPI::Register_SetGamePlayerViewAngle(SetGamePlayerViewAngle);
	FreemanAPI::Register_GetPointContents(GetPointContents);
	FreemanAPI::Register_PointRaytrace(PointRaytrace);
	FreemanAPI::Register_GetGameMoveLeftRight(GetGameMoveLeftRight);
	FreemanAPI::Register_GetGameMoveFwdBack(GetGameMoveFwdBack);
	FreemanAPI::Register_GetGameMoveUpDown(GetGameMoveUpDown);
	FreemanAPI::Register_GetGameMoveJump(GetGameMoveJump);
	FreemanAPI::Register_GetGameMoveDuck(GetGameMoveDuck);
	FreemanAPI::Register_GetGameMoveRun(GetGameMoveRun);
	FreemanAPI::Register_GetGameMoveUse(GetGameMoveUse);

	// custom vars
	FreemanAPI::SetConfigName("FlatOutUCHLMovement_gcp.toml");
	FreemanAPI::RegisterCustomBoolean("Teleport Car", "teleport_car", &bTeleportCar, 0);
	FreemanAPI::RegisterCustomBoolean("Car Godmode", "car_godmode", &bCarGodmode, 0);
	FreemanAPI::RegisterCustomBoolean("Show Velocity", "show_velocity", &bShowVelocity, 0);
	FreemanAPI::RegisterCustomBoolean("Show 2D Velocity", "show_velocity_2d", &bShowVelocity2D, 0);
	FreemanAPI::RegisterCustomFloat("fov", "fov", &FO2Cam::fFOV, 1);
	FreemanAPI::RegisterCustomFloat("sensitivity", "sensitivity", &FO2Cam::fSensitivity, 1);
	FreemanAPI::RegisterCustomFloat("volume", "volume", &fSoundVolume, 1);
	FreemanAPI::LoadConfig();
}