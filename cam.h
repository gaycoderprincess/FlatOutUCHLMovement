namespace FO2Cam {
	int nLastGameState = -1;

	double fMouse[2] = {};
	NyaVec3 vPos = {0, 0, 0};
	NyaVec3 vAngle = {0, 0, 0}; // input angle
	NyaVec3 vAngleView = {0, 0, 0}; // output angle with HL offsets
	float fFOV = 90;
	float fSensitivity = 1;

	void SetRotation(Camera *cam) {
		auto mat = cam->GetMatrix();
		mat->SetIdentity();
		mat->Rotate({-vAngleView[1], vAngleView[2], vAngleView[0]});
		mat->p = vPos;
	}

	void DoMovement(Camera *cam) {
		auto mat = cam->GetMatrix();
		vAngle[0] += fMouse[0] * -fSensitivity * (std::numbers::pi / 180.0) * 0.05;
		vAngle[1] -= fMouse[1] * -fSensitivity * (std::numbers::pi / 180.0) * 0.05;

		float maxPitch = std::numbers::pi * 0.4999;
		if (vAngle[1] < -maxPitch) vAngle[1] = -maxPitch;
		if (vAngle[1] > maxPitch) vAngle[1] = maxPitch;

		mat->p = vPos;
	}

	void Process(Camera *cam) {
		if (!cam) return;
		if (nLastGameState != pGameFlow->nRaceState) {
			vAngle = {0, 0, 0};
			vAngleView = {0, 0, 0};
			if (auto ply = GetPlayer(0)) {
				auto fwd = ply->pCar->GetMatrix()->z;
				vAngle[0] = atan2(-fwd.x, fwd.z);
			}
			nLastGameState = pGameFlow->nRaceState;
		}

		SetRotation(cam);
		DoMovement(cam);
		SetRotation(cam);

		fMouse[0] = 0;
		fMouse[1] = 0;
		nMouseWheelState = 0;

		cam->fFOV = fFOV * (std::numbers::pi / 180.0);
	}
}