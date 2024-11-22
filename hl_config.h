// custom config
namespace HLMovement {
	bool bEnabled = true;
	bool bCanLongJump = false;
	bool bAutoBhop = true;
	bool bABH = false;
	bool bABHMixed = false;
	bool bBhopCap = false;
	float fSoundVolume = 1;

	// game integration config
	bool bConvertUnits = true; // do conversions from units to meters when handling game funcs

	// rotation order
	int PITCH = 1;
	int YAW = 0;
	int ROLL = 2;

	// 2 & 1 for Y-up, 1 & 2 for Z-up
	int FORWARD = 2;
	int UP = 1;
}