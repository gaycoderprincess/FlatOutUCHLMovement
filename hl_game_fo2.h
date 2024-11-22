// HL to FO2 integration
namespace HLMovement {
	bool bTeleportCar = false;
	bool bCarGodmode = false;

	struct tFO2MaterialMatchup {
		int surfaceId;
		int footstepId;
	};
	tFO2MaterialMatchup aSurfaces[] = {
			{ 1, CHAR_TEX_CONCRETE }, // NoCollision
			{ 2, CHAR_TEX_CONCRETE }, // Tarmac (Road)
			{ 3, CHAR_TEX_CONCRETE }, // Tarmac Mark (Road)
			{ 4, CHAR_TEX_CONCRETE }, // Asphalt (Road)
			{ 5, CHAR_TEX_CONCRETE }, // Asphalt Mark (Road)
			{ 6, CHAR_TEX_CONCRETE }, // Cement Mark (Road)
			{ 7, CHAR_TEX_CONCRETE }, // Cement Mark (Road)
			{ 8, CHAR_TEX_DIRT }, // Hard (Road)
			{ 9, CHAR_TEX_DIRT }, // Hard Mark (Road)
			{ 10, CHAR_TEX_DIRT }, // Medium (Road)
			{ 11, CHAR_TEX_DIRT }, // Medium Mark (Road)
			{ 12, CHAR_TEX_DIRT }, // Soft (Road)
			{ 13, CHAR_TEX_DIRT }, // Soft Mark (Road)
			{ 14, CHAR_TEX_DIRT }, // Derby Gravel (Road)
			{ 15, CHAR_TEX_CONCRETE }, // Derby Tarmac (Road)
			{ 16, CHAR_TEX_DIRT }, // Snow (Road)
			{ 17, CHAR_TEX_DIRT }, // Snow Mark (Road)
			{ 18, CHAR_TEX_DIRT }, // Dirt (Road)
			{ 19, CHAR_TEX_DIRT }, // Dirt Mark (Road)
			{ 20, CHAR_TEX_METAL }, // Bridge Metal (Road)
			{ 21, CHAR_TEX_WOOD }, // Bridge Wooden (Road)
			{ 22, CHAR_TEX_CONCRETE }, // Curb (Terrain)
			{ 23, CHAR_TEX_DIRT }, // Bank Sand (terrain)
			{ 24, CHAR_TEX_DIRT }, // Grass (terrain)
			{ 25, CHAR_TEX_DIRT }, // Forest (terrain)
			{ 26, CHAR_TEX_DIRT }, // Sand (terrain)
			{ 27, CHAR_TEX_CONCRETE }, // Rock (terrain)
			{ 28, CHAR_TEX_DIRT }, // Mould (terrain)
			{ 29, CHAR_TEX_DIRT }, // Snow  (terrain)
			{ 30, CHAR_TEX_DIRT }, // Field  (terrain)
			{ 31, CHAR_TEX_SLOSH }, // Wet  (terrain)
			{ 32, CHAR_TEX_CONCRETE }, // Concrete (Object)
			{ 33, CHAR_TEX_CONCRETE }, // Rock (Object)
			{ 34, CHAR_TEX_METAL }, // Metal (Object)
			{ 35, CHAR_TEX_WOOD }, // Wood (Object)
			{ 36, CHAR_TEX_WOOD }, // Tree (Object)
			{ 37, CHAR_TEX_CONCRETE }, // Bush
			{ 38, CHAR_TEX_CONCRETE }, // Rubber (Object)
			{ 39, CHAR_TEX_SLOSH }, // Water (water)
			{ 40, CHAR_TEX_SLOSH }, // River (water)
			{ 41, CHAR_TEX_SLOSH }, // Puddle (water)
			{ 42, CHAR_TEX_CONCRETE }, // No Camera Col
			{ 43, CHAR_TEX_CONCRETE }, // Camera only col
			{ 44, CHAR_TEX_CONCRETE }, // Reset
			{ 45, CHAR_TEX_CONCRETE }, // Stunt Conveyer
			{ 46, CHAR_TEX_CONCRETE }, // Stunt Bouncer
			{ 47, CHAR_TEX_CONCRETE }, // Stunt Curling
			{ 48, CHAR_TEX_CONCRETE }, // Stunt Bowling
			{ 49, CHAR_TEX_CONCRETE }, // Stunt Tarmac
			{ 50, CHAR_TEX_CONCRETE }, // Oil (Road)
	};
	int GetSurfaceTextureFromID(int id) {
		for (auto& surf : aSurfaces) {
			if (surf.surfaceId == id + 1) return surf.footstepId;
		}
		return CHAR_TEX_CONCRETE;
	}

	bool GetGamePlayerDead() {
		if (auto ply = GetPlayer(0)) {
			return ply->nIsWrecked;
		}
		return false;
	}

	void GetGamePlayerPosition(NyaVec3Double* out) {
		*out = {0,0,0};
		if (auto ply = GetPlayer(0)) {
			out->x = ply->pCar->GetMatrix()->p.x;
			out->y = ply->pCar->GetMatrix()->p.y + 1;
			out->z = ply->pCar->GetMatrix()->p.z;
		}
	}

	void GetGamePlayerViewAngle(NyaVec3Double* out) {
		out->x = FO2Cam::vAngle.x / (std::numbers::pi / 180.0);
		out->y = FO2Cam::vAngle.y / (std::numbers::pi / 180.0);
		out->z = FO2Cam::vAngle.z / (std::numbers::pi / 180.0);
	}

	void SetGamePlayerPosition(const NyaVec3Double* in, const NyaVec3Double* inVelocity) {
		if (!bTeleportCar) return;

		if (auto ply = GetPlayer(0)) {
			ply->pCar->GetMatrix()->p.x = in->x;
			ply->pCar->GetMatrix()->p.y = in->y - 1;
			ply->pCar->GetMatrix()->p.z = in->z;
			ply->pCar->GetVelocity()->x = inVelocity->x;
			ply->pCar->GetVelocity()->y = inVelocity->y - 1;
			ply->pCar->GetVelocity()->z = inVelocity->z;
			*ply->pCar->GetAngVelocity() = {0,0,0};
			if (bCarGodmode) {
				ply->pCar->fDamage = 0;
			}
		}
	}

	void SetGamePlayerViewPosition(const NyaVec3Double* in) {
		FO2Cam::vPos.x = in->x;
		FO2Cam::vPos.y = in->y;
		FO2Cam::vPos.z = in->z;
	}

	void SetGamePlayerViewAngle(const NyaVec3Double* in) {
		FO2Cam::vAngleView.x = in->x * (std::numbers::pi / 180.0);
		FO2Cam::vAngleView.y = in->y * (std::numbers::pi / 180.0);
		FO2Cam::vAngleView.z = in->z * (std::numbers::pi / 180.0);

		if (bTeleportCar) {
			if (auto ply = GetPlayer(0)) {
				auto mat = ply->pCar->GetMatrix();
				auto oldPos = mat->p;
				mat->SetIdentity();
				mat->Rotate({-FO2Cam::vAngleView[1], FO2Cam::vAngleView[2], FO2Cam::vAngleView[0]});
				mat->p = oldPos;
				FO2MatrixToQuat(&mat->x.x, &ply->pCar->qQuaternion[0]);
			}
		}
	}

	int GetPointContentsGame(const NyaVec3Double* point) {
		if (pEnvironment && pEnvironment->bWaterPlane && (*point)[UP] <= 0) {
			return CONTENTS_WATER;
		}

		return CONTENTS_EMPTY;
	}

	pmtrace_t* PointRaytraceGame(const NyaVec3Double* _origin, const NyaVec3Double* _end) {
		auto origin = *_origin;
		auto end = *_end;

		static pmtrace_t trace;
		trace.allsolid = false;
		trace.startsolid = false;
		trace.inopen = true;
		trace.inwater = false;
		trace.fraction = 1.0f;
		trace.endpos = end;
		trace.plane.normal = {0,0,0};
		trace.plane.dist = 9999;
		trace.ent = -1;
		trace.surfaceId = 0;

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
				trace.plane.normal.x = out.vHitNormal.x;
				trace.plane.normal.y = out.vHitNormal.y;
				trace.plane.normal.z = out.vHitNormal.z;
				trace.fraction = out.fHitDistance / dist;
				if (trace.fraction > 1.0f) trace.fraction = 1.0f;
				trace.endpos = origin + (dir * out.fHitDistance);
				trace.ent = 0; // dummy entity
				trace.surfaceId = GetSurfaceTextureFromID(out.nSurfaceId);
			}
		}
		return &trace;
	}
}