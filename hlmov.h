namespace HLMovement {
	static std::string GetSpeechPath(const std::string& file) {
		return "data/sound/hl/" + file;
	}

	std::vector<NyaAudio::NyaSound> aSoundCache;
	void PlayGameSound(const std::string& path, float volume) {
		auto sound = NyaAudio::LoadFile(GetSpeechPath(path).c_str());
		if (!sound) return;
		NyaAudio::SetVolume(sound, volume);
		NyaAudio::Play(sound);
		aSoundCache.push_back(sound);
	}

	std::string lastConsoleMsg;

	bool bEnabled = false;
	bool bCanLongJump = false;
	bool bAutoBhop = true;
	bool bABH = false;
	bool bABHMixed = true;

	float cl_bob = 0.01;
	float cl_bobcycle = 0.8;
	float cl_bobup = 0.5;

	float cl_forwardspeed = 400;
	float cl_sidespeed = 400;
	float cl_upspeed = 320;
	float cl_movespeedkey = 0.3;

	float sv_gravity = 800;  			// Gravity for map
	float sv_stopspeed = 100;			// Deceleration when not moving
	float sv_maxspeed = 320; 			// Max allowed speed
	float sv_noclipspeed = 320; 		// Max allowed speed
	float sv_accelerate = 10;			// Acceleration factor
	float sv_airaccelerate = 10;		// Same for when in open air
	float sv_wateraccelerate = 10;		// Same for when in water
	float sv_friction = 4;
	float sv_edgefriction = 2;			// Extra friction near dropofs
	float sv_waterfriction = 1;			// Less in water
	//float sv_entgravity = 1.0;  		// 1.0
	float sv_bounce = 1.0;      		// Wall bounce value. 1.0
	float sv_stepsize = 18;
	float sv_maxvelocity = 2000; 		// maximum server velocity.
	//bool mp_footsteps = true;			// Play footstep sounds
	float sv_rollangle = 2;
	float sv_rollspeed = 200;

	inline double UnitsToMeters(double f) {
		return f * 0.0254;
	}

	inline double MetersToUnits(double f) {
		return f / 0.0254;
	}

	double DotProduct(const NyaVec3Double& x, const NyaVec3Double& y) {
		return ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2]);
	}
	void CrossProduct (const NyaVec3Double v1, const NyaVec3Double v2, NyaVec3Double cross)
	{
		cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
		cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
		cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
	}
	void VectorFill(NyaVec3Double& a, float b) {
		(a)[0]=(b);
		(a)[1]=(b);
		(a)[2]=(b);
	}
	float VectorAvg(const NyaVec3Double& a) {
		return ( ( (a)[0] + (a)[1] + (a)[2] ) / 3 );
	}
	void VectorSubtract(const NyaVec3Double& a, const NyaVec3Double& b, NyaVec3Double& c){
		(c)[0]=(a)[0]-(b)[0];
		(c)[1]=(a)[1]-(b)[1];
		(c)[2]=(a)[2]-(b)[2];
	}
	void VectorAdd(const NyaVec3Double& a, const NyaVec3Double& b, NyaVec3Double& c) {
		(c)[0]=(a)[0]+(b)[0];
		(c)[1]=(a)[1]+(b)[1];
		(c)[2]=(a)[2]+(b)[2];
	}
	void VectorCopy(const NyaVec3Double& a, NyaVec3Double& b) {
		(b)[0]=(a)[0];
		(b)[1]=(a)[1];
		(b)[2]=(a)[2];
	}
	void VectorScale(const NyaVec3Double& a, float b, NyaVec3Double& c) {
		(c)[0]=(b)*(a)[0];
		(c)[1]=(b)*(a)[1];
		(c)[2]=(b)*(a)[2];
	}
	float VectorNormalize (NyaVec3Double& v) {
		float	length, ilength;

		length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
		length = sqrt (length);

		if (length)
		{
			ilength = 1/length;
			v[0] *= ilength;
			v[1] *= ilength;
			v[2] *= ilength;
		}

		return length;

	}
	inline void VectorClear(NyaVec3Double& a) {
		a[0]=0.0;
		a[1]=0.0;
		a[2]=0.0;
	}
	inline void VectorMA(const NyaVec3Double &start, float scale, const NyaVec3Double &direction, NyaVec3Double &dest)
	{
		dest.x = start.x + scale * direction.x;
		dest.y = start.y + scale * direction.y;
		dest.z = start.z + scale * direction.z;
	}

	NyaVec3Double vec3_origin = {0,0,0};

	// default hullmins
	static const NyaVec3Double pm_hullmins[4] =
	{
			{ -16, -16, -36 },
			{ -16, -16, -18 },
			{   0,   0,   0 },
			{ -32, -32, -32 },
	};

	// defualt hullmaxs
	static const NyaVec3Double pm_hullmaxs[4] =
	{
			{  16,  16,  36 },
			{  16,  16,  18 },
			{   0,   0,   0 },
			{  32,  32,  32 },
	};

	// pitch/yaw/roll for flatout cam
	const int PITCH = 1;
	const int YAW = 0;
	const int ROLL = 2;

	//const int SIDE = 0;
	const int FORWARD = 2;
	const int UP = 1;

	const int MAX_PHYSENTS = 600; 				// Must have room for all entities in the world.
	const int MAX_MOVEENTS = 64;
	const int MAX_CLIP_PLANES = 5;

	const float TIME_TO_DUCK 				= 0.4;
	const int VEC_HULL_MIN					= -36;
	const int VEC_HULL_MAX					= 36;
	const int VEC_VIEW						= 28;
	const int VEC_DUCK_HULL_MIN				= -18;
	const int VEC_DUCK_HULL_MAX				= 18;
	const int VEC_DUCK_VIEW					= 12;
	const int PM_DEAD_VIEWHEIGHT			= -8;
	const int PLAYER_FATAL_FALL_SPEED		= 1024;	// approx 60 feet
	const int PLAYER_MAX_SAFE_FALL_SPEED	= 580;	// approx 20 feet
	const float DAMAGE_FOR_FALL_SPEED		= (float)100 / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED); // damage per unit per second.
	const int PLAYER_MIN_BOUNCE_SPEED		= 200;
	const int PLAYER_FALL_PUNCH_THRESHHOLD	= 350;	// won't punch player's screen/make scrape noise unless player falling at least this fast.
	const int PLAYER_LONGJUMP_SPEED			= 350;	// how fast we longjump
	const float PLAYER_DUCKING_MULTIPLIER 	= 0.333;
	const float	STOP_EPSILON				= 0.1;

	// edict->movetype values
	const int MOVETYPE_NONE			= 0;		// never moves
	const int MOVETYPE_ANGLENOCLIP	= 1;
	const int MOVETYPE_ANGLECLIP	= 2;
	const int MOVETYPE_WALK			= 3;		// Player only - moving on the ground
	const int MOVETYPE_STEP			= 4;		// gravity, special edge handling -- monsters use this
	const int MOVETYPE_FLY			= 5;		// No gravity, but still collides with stuff
	const int MOVETYPE_TOSS			= 6;		// gravity/collisions
	const int MOVETYPE_PUSH			= 7;		// no clip to world, push and crush
	const int MOVETYPE_NOCLIP		= 8;		// No gravity, no collisions, still do velocity/avelocity
	const int MOVETYPE_FLYMISSILE	= 9;		// extra size to monsters
	const int MOVETYPE_BOUNCE		= 10;		// Just like Toss, but reflect velocity when contacting surfaces
	const int MOVETYPE_BOUNCEMISSILE= 11;		// bounce w/o gravity
	const int MOVETYPE_FOLLOW		= 12;		// track movement of aiment
	const int MOVETYPE_PUSHSTEP		= 13;		// BSP model that needs physics/world collisions (uses nearest hull for world collision)

	// edict->flags
	const int FL_FLY				= (1<<0);	// Changes the SV_Movestep() behavior to not need to be on ground
	const int FL_SWIM				= (1<<1);	// Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
	const int FL_CONVEYOR			= (1<<2);
	const int FL_CLIENT				= (1<<3);
	const int FL_INWATER			= (1<<4);
	const int FL_MONSTER			= (1<<5);
	const int FL_GODMODE			= (1<<6);
	const int FL_NOTARGET			= (1<<7);
	const int FL_SKIPLOCALHOST		= (1<<8);	// Don't send entity to local host, it's predicting this entity itself
	const int FL_ONGROUND			= (1<<9);	// At rest / on the ground
	const int FL_PARTIALGROUND		= (1<<10);	// not all corners are valid
	const int FL_WATERJUMP			= (1<<11);	// player jumping out of water
	const int FL_FROZEN				= (1<<12);	// Player is frozen for 3rd person camera
	const int FL_FAKECLIENT			= (1<<13);	// JAC: fake client, simulated server side; don't send network messages to them
	const int FL_DUCKING			= (1<<14);	// Player flag -- Player is fully crouched
	const int FL_FLOAT				= (1<<15);	// Apply floating force to this entity when in water
	const int FL_GRAPHED			= (1<<16);	// worldgraph has this ent listed as something that blocks a connection

	const int FL_PROXY				= (1<<20);	// This is a spectator proxy
	const int FL_ALWAYSTHINK		= (1<<21);	// Brush model flag -- call think every frame regardless of nextthink - ltime (for constantly changing velocity/path)
	const int FL_BASEVELOCITY		= (1<<22);	// Base velocity has been applied this frame (used to convert base velocity into momentum)
	const int FL_MONSTERCLIP		= (1<<23);	// Only collide in with monsters who have FL_MONSTERCLIP set
	const int FL_ONTRAIN			= (1<<24);	// Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
	const int FL_WORLDBRUSH			= (1<<25);	// Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
	const int FL_SPECTATOR      	= (1<<26);	// This client is a spectator, don't run touch functions, etc.
	const int FL_CUSTOMENTITY		= (1<<29);	// This is a custom entity
	const int FL_KILLME				= (1<<30);	// This entity is marked for death -- This allows the engine to kill ents at the appropriate time
	const int FL_DORMANT			= (1<<31);	// Entity is dormant, no updates to client

	const int IN_ATTACK				= (1 << 0);
	const int IN_JUMP				= (1 << 1);
	const int IN_DUCK				= (1 << 2);
	const int IN_FORWARD			= (1 << 3);
	const int IN_BACK				= (1 << 4);
	const int IN_USE				= (1 << 5);
	const int IN_CANCEL				= (1 << 6);
	const int IN_LEFT				= (1 << 7);
	const int IN_RIGHT				= (1 << 8);
	const int IN_MOVELEFT			= (1 << 9);
	const int IN_MOVERIGHT 			= (1 << 10);
	const int IN_ATTACK2			= (1 << 11);
	const int IN_RUN      			= (1 << 12);
	const int IN_RELOAD				= (1 << 13);
	const int IN_ALT1				= (1 << 14);
	const int IN_SCORE				= (1 << 15);	// Used by client.dll for when scoreboard is held down

	// texture types
	const int CHAR_TEX_CONCRETE		= 'C';
	const int CHAR_TEX_METAL		= 'M';
	const int CHAR_TEX_DIRT			= 'D';
	const int CHAR_TEX_VENT			= 'V';
	const int CHAR_TEX_GRATE		= 'G';
	const int CHAR_TEX_TILE			= 'T';
	const int CHAR_TEX_SLOSH		= 'S';
	const int CHAR_TEX_WOOD			= 'W';
	const int CHAR_TEX_COMPUTER		= 'P';
	const int CHAR_TEX_GLASS		= 'Y';
	const int CHAR_TEX_FLESH		= 'F';
	const int CHAR_TEX_WADE			= 'Z';
	const int CHAR_TEX_LADDER		= 'L';

	const int CONTENTS_LAVA			= 0;
	const int CONTENTS_SLIME		= 1;
	const int CONTENTS_WATER		= 2;
	const int CONTENTS_EMPTY		= 3;

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

	struct physent_t {

	};

	struct pmplane_t {
		NyaVec3Double	normal;
		float	dist;
	};

	struct pmtrace_t {
		bool		allsolid;	    	// if true, plane is not valid
		bool		startsolid;	    	// if true, the initial point was in a solid area
		bool		inopen, inwater;	// End point is in empty space or in water
		float		fraction;			// time completed, 1.0 = didn't hit anything
		NyaVec3Double		endpos;				// final position
		pmplane_t	plane;		    	// surface normal at impact
		int			ent;				// entity at impact

		// added for point to point
		double distFromOrigin;
		int surfaceId;

		void Default() {
			allsolid = false;	    				// if true, plane is not valid
			startsolid = false;	    			// if true, the initial point was in a solid area
			inopen = true;
			inwater = false;						// End point is in empty space or in water
			fraction = 1.0f;						// time completed, 1.0 = didn't hit anything
			endpos = {0,0,0};							// final position
			plane.normal = {0,0,0};  	// surface normal at impact
			plane.dist = 9999;
			ent = -1;								// entity at impact
			distFromOrigin = 0;
			surfaceId = 0;
		}
	};

	struct movevars_s
	{
		float	gravity;  			// Gravity for map
		float	stopspeed;			// Deceleration when not moving
		float	maxspeed; 			// Max allowed speed
		float	accelerate;     	// Acceleration factor
		float	airaccelerate;  	// Same for when in open air
		float	wateraccelerate;	// Same for when in water
		float	friction;
		float   edgefriction;		// Extra friction near dropofs
		float	waterfriction;		// Less in water
		//float	entgravity;  		// 1.0
		float   bounce;      		// Wall bounce value. 1.0
		float   stepsize;    		// sv_stepsize;
		float   maxvelocity; 		// maximum server velocity.
		//bool	footsteps;			// Play footstep sounds
		float	rollangle;
		float	rollspeed;
	} *movevars = new movevars_s;

	typedef struct usercmd_s
	{
		uint32_t msec;				// Duration in ms of command
		NyaVec3Double	viewangles;			// Command view angles.

		// intended velocities
		float	forwardmove;		// Forward velocity.
		float	sidemove;			// Sideways velocity.
		float	upmove;				// Upward velocity.
		unsigned short buttons;		// Attack buttons
	} usercmd_t;

	struct playermove_s {
		float			frametime;			// Duration of this frame
		NyaVec3Double			forward, right, up;	// Vectors for angles

		// player state
		NyaVec3Double			origin;				// Movement origin.
		NyaVec3Double			angles;				// Movement view angles.
		NyaVec3Double			oldangles;			// Angles before movement view angles were looked at.
		NyaVec3Double			velocity;			// Current movement direction.
		NyaVec3Double			movedir;			// For waterjumping, a forced forward velocity so we can fly over lip of ledge.
		NyaVec3Double			basevelocity;		// Velocity of the conveyor we are standing, e.g.

		// For ducking/dead
		NyaVec3Double			view_ofs;			// Our eye position.
		float			flDuckTime;			// Time we started duck
		bool			bInDuck;			// In process of ducking or ducked already?

		// For walking/falling
		int				flTimeStepSound;	// Next time we can play a step sound
		int				iStepLeft;

		float			flFallVelocity;
		NyaVec3Double			punchangle;

		float			flSwimTime;

		int				flags;				// FL_ONGROUND, FL_DUCKING, etc.
		int				usehull;			// 0 = regular player hull, 1 = ducked player hull, 2 = point hull
		float			gravity;			// Our current gravity and friction.
		float			friction;
		int				oldbuttons;			// Buttons last usercmd
		float			waterjumptime;		// Amount of time left in jumping out of water cycle.
		bool			dead;				// Are we a dead player?
		int				deadflag;
		int				movetype;			// Our movement type, NOCLIP, WALK, FLY

		int				onground;
		int				waterlevel;
		int				watertype;
		int				oldwaterlevel;

		int				chtexturetype;

		float			maxspeed;
		float			clientmaxspeed;		// Player specific maxspeed

		// world state
		// Number of entities to clip against.
		int				numphysent;
		physent_t		physents[MAX_PHYSENTS];
		// Number of momvement entities (ladders)
		int				nummoveent;
		// just a list of ladders
		physent_t		moveents[MAX_MOVEENTS];

		// Trace results for objects we collided with.
		int				numtouch;
		pmtrace_t		touchindex[MAX_PHYSENTS];

		usercmd_t cmd;

		NyaVec3Double player_mins[ 4 ];
		NyaVec3Double player_maxs[ 4 ];
	} *pmove = new playermove_s;

	CNyaTimer gTimer;

	void AngleVectors(const NyaVec3Double& angles, NyaVec3Double& fwd, NyaVec3Double& right, NyaVec3Double& up) {
		auto anglesRad = angles * (std::numbers::pi / 180.0);

		auto mat = NyaMat4x4();
		mat.Rotate(NyaVec3(-anglesRad[1], anglesRad[2], anglesRad[0]));

		// this is disgusting.
		fwd.x = (*(NyaVec3*)&mat[FORWARD*4]).x;
		fwd.y = (*(NyaVec3*)&mat[FORWARD*4]).y;
		fwd.z = (*(NyaVec3*)&mat[FORWARD*4]).z;
		right.x = mat.x.x;
		right.y = mat.x.y;
		right.z = mat.x.z;
		up.x = (*(NyaVec3*)&mat[UP*4]).x;
		up.y = (*(NyaVec3*)&mat[UP*4]).y;
		up.z = (*(NyaVec3*)&mat[UP*4]).z;
	}

	void PM_DropPunchAngle(NyaVec3Double& punchangle) {
		float	len;

		len = VectorNormalize (punchangle);
		len -= (10.0 + len * 0.5) * pmove->frametime;
		len = std::max(len, 0.0f);
		VectorScale(punchangle, len, punchangle);
	}

	void PM_PlayWaterSounds() {
		// Did we enter or leave water?
		if  ( ( pmove->oldwaterlevel == 0 && pmove->waterlevel != 0 ) ||
			  ( pmove->oldwaterlevel != 0 && pmove->waterlevel == 0 ) )
		{
			switch ( rand() % 4 )
			{
				case 0:
					PlayGameSound( "player/pl_wade1.wav", 1 );
					break;
				case 1:
					PlayGameSound( "player/pl_wade2.wav", 1 );
					break;
				case 2:
					PlayGameSound( "player/pl_wade3.wav", 1 );
					break;
				case 3:
					PlayGameSound( "player/pl_wade4.wav", 1 );
					break;
			}
		}
	}

	float V_CalcRoll(NyaVec3Double angles, NyaVec3Double velocity, float rollangle, float rollspeed) {
		float   sign;
		float   side;
		float   value;
		NyaVec3Double  forward, right, up;

		AngleVectors (angles, forward, right, up);

		side = DotProduct (velocity, right);
		sign = side < 0 ? -1 : 1;
		side = fabs(side);

		value = rollangle;
		if (side < rollspeed)
		{
			side = side * value / rollspeed;
		}
		else
		{
			side = value;
		}

		return side * sign;
	}

	float V_CalcBob() {
		static	double	bobtime;
		static float	bob;
		float	cycle;
		static float	lasttime;
		NyaVec3Double	vel;


		if ( pmove->onground == -1 ||
			pmove->cmd.msec == lasttime )
		{
			// just use old value
			return bob;
		}

		lasttime = pmove->cmd.msec;

		bobtime += pmove->frametime;
		cycle = bobtime - (int)( bobtime / cl_bobcycle ) * cl_bobcycle;
		cycle /= cl_bobcycle;

		if ( cycle < cl_bobup )
		{
			cycle = M_PI * cycle / cl_bobup;
		}
		else
		{
			cycle = M_PI + M_PI * ( cycle - cl_bobup )/( 1.0 - cl_bobup );
		}

		// bob is proportional to simulated velocity in the xy plane
		// (don't count Z, or jumping messes it up)
		VectorCopy( pmove->velocity, vel );
		vel[UP] = 0;

		bob = sqrt( vel[0] * vel[0] + vel[FORWARD] * vel[FORWARD] ) * cl_bob;
		bob = bob * 0.3 + bob * 0.7 * std::sin(cycle);
		bob = std::min( bob, 4.0f );
		bob = std::max( bob, -7.0f );
		return bob;
	}

	void PM_CheckParamters() {
		float spd;
		float maxspeed;
		NyaVec3Double	v_angle;

		spd = ( pmove->cmd.forwardmove * pmove->cmd.forwardmove ) +
			  ( pmove->cmd.sidemove * pmove->cmd.sidemove ) +
			  ( pmove->cmd.upmove * pmove->cmd.upmove );
		spd = sqrt( spd );

		maxspeed = pmove->clientmaxspeed; //atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "maxspd" ) );
		if ( maxspeed != 0.0 )
		{
			pmove->maxspeed = std::min( maxspeed, pmove->maxspeed );
		}

		// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
		//
		// JoshA: Moved this to CheckParamters rather than working on the velocity,
		// as otherwise it affects every integration step incorrectly.
		if ( ( pmove->onground != -1 ) && ( pmove->cmd.buttons & IN_USE) )
		{
			pmove->maxspeed *= 1.0f / 3.0f;
		}

		if ( ( spd != 0.0 ) &&
			 ( spd > pmove->maxspeed ) )
		{
			float fRatio = pmove->maxspeed / spd;
			pmove->cmd.forwardmove *= fRatio;
			pmove->cmd.sidemove    *= fRatio;
			pmove->cmd.upmove      *= fRatio;
		}

		if ( pmove->flags & FL_FROZEN ||
			 pmove->flags & FL_ONTRAIN ||
			 pmove->dead )
		{
			pmove->cmd.forwardmove = 0;
			pmove->cmd.sidemove    = 0;
			pmove->cmd.upmove      = 0;
		}


		PM_DropPunchAngle( pmove->punchangle );

		// Take angles from command.
		if ( !pmove->dead )
		{
			VectorCopy ( pmove->cmd.viewangles, v_angle );

			auto punch = pmove->punchangle;
			punch[PITCH] /= 3.0; // 1/3 pitch from xash3d

			VectorAdd( v_angle, punch, v_angle );

			// Set up view angles.
			// using V_CalcRoll here instead of PM_CalcRoll
			pmove->angles[ROLL]  =	v_angle[ROLL] + V_CalcRoll (v_angle, pmove->velocity, movevars->rollangle, movevars->rollspeed);
			pmove->angles[PITCH] =	v_angle[PITCH];
			pmove->angles[YAW]   =	v_angle[YAW];
		}
		else
		{
			VectorCopy( pmove->oldangles, pmove->angles );
		}

		// Set dead player view_offset
		if ( pmove->dead )
		{
			pmove->view_ofs[UP] = PM_DEAD_VIEWHEIGHT;
		}

		// Adjust client view angles to match values used on server.
		if (pmove->angles[YAW] > 180.0f)
		{
			pmove->angles[YAW] -= 360.0f;
		}
	}

	void PM_ReduceTimers()
	{
		if ( pmove->flTimeStepSound > 0 )
		{
			pmove->flTimeStepSound -= pmove->cmd.msec;
			if ( pmove->flTimeStepSound < 0 )
			{
				pmove->flTimeStepSound = 0;
			}
		}
		if ( pmove->flDuckTime > 0 )
		{
			pmove->flDuckTime -= pmove->cmd.msec;
			if ( pmove->flDuckTime < 0 )
			{
				pmove->flDuckTime = 0;
			}
		}
		if ( pmove->flSwimTime > 0 )
		{
			pmove->flSwimTime -= pmove->cmd.msec;
			if ( pmove->flSwimTime < 0 )
			{
				pmove->flSwimTime = 0;
			}
		}
	}

	int GetPointContents(NyaVec3Double point) {
		if (pEnvironment && pEnvironment->bWaterPlane && point[UP] <= 0) {
			return CONTENTS_WATER;
		}

		return CONTENTS_EMPTY;
	}

	pmtrace_t PointRaytrace(NyaVec3Double origin, NyaVec3Double end) {
		pmtrace_t trace;
		trace.allsolid = false;	    				// if true, plane is not valid
		trace.startsolid = false;	    			// if true, the initial point was in a solid area
		trace.inopen = true;
		trace.inwater = false;						// End point is in empty space or in water
		trace.fraction = 1.0f;						// time completed, 1.0 = didn't hit anything
		trace.endpos = end;							// final position
		trace.plane.normal = {0,0,0};  	// surface normal at impact
		trace.plane.dist = 9999;
		trace.ent = -1;								// entity at impact
		trace.distFromOrigin = (origin - end).length();
		trace.surfaceId = 0;

		if (pGameFlow->nGameState == GAME_STATE_RACE && pmove->movetype != MOVETYPE_NOCLIP) {
			auto originUnits = origin;

			for (int i = 0; i < 3; i++) {
				origin[i] = UnitsToMeters(origin[i]);
				end[i] = UnitsToMeters(end[i]);
			}

			tLineOfSightIn prop;

			NyaVec3Double dir = end - origin;
			auto dist = dir.length();
			dir.Normalize();

			auto originf = NyaVec3(origin.x, origin.y, origin.z);
			auto endf = NyaVec3(end.x, end.y, end.z);
			auto dirf = NyaVec3(dir.x, dir.y, dir.z);

			prop.fMaxDistance = dist;
			tLineOfSightOut out;
			if (CheckLineOfSight(&prop, pGameFlow->pHost->pUnkForLOS, &originf, &dirf, &out)) {
				trace.plane.dist = MetersToUnits(out.fHitDistance);
				trace.plane.normal.x = out.vHitNormal.x;
				trace.plane.normal.y = out.vHitNormal.y;
				trace.plane.normal.z = out.vHitNormal.z;
				trace.fraction = out.fHitDistance / dist;
				if (trace.fraction > 1.0f) trace.fraction = 1.0f;
				trace.endpos = origin + (dir * out.fHitDistance);
				trace.ent = 0; // dummy entity
				trace.surfaceId = out.nSurfaceId;

				for (int i = 0; i < 3; i++) {
					trace.endpos[i] = MetersToUnits(trace.endpos[i]);
				}

				trace.distFromOrigin = (trace.endpos - originUnits).length();
			}
		}

		return trace;
	}

	int nColDensity = 2;

	// if you have a bbox trace function, implement this yourself
	/*pmtrace_t PM_PlayerTraceInternal(NyaVec3Double origin, NyaVec3Double end, NyaVec3Double offset1, NyaVec3Double offset2) {
		// bweh?
		//origin = end;

		// offset relative to hull
		offset1 *= pmove->player_maxs[pmove->usehull];
		offset2 *= pmove->player_maxs[pmove->usehull];
		auto minDistForSolid = offset2.length();

		pmtrace_t trace;
		trace.allsolid = false;	    				// if true, plane is not valid
		trace.startsolid = false;	    			// if true, the initial point was in a solid area
		trace.inopen = true;
		trace.inwater = false;						// End point is in empty space or in water
		trace.fraction = 1.0f;						// time completed, 1.0 = didn't hit anything
		trace.endpos = end;							// final position
		trace.plane.normal = {0,0,0};  	// surface normal at impact
		trace.plane.dist = 9999;
		trace.ent = -1;								// entity at impact
		trace.distFromOrigin = (origin - end).length();

		origin += offset1;
		end += offset2;

		if (pGameFlow->nGameState == GAME_STATE_RACE && pmove->movetype != MOVETYPE_NOCLIP) {
			auto originUnits = origin;

			for (int i = 0; i < 3; i++) {
				origin[i] = UnitsToMeters(origin[i]);
				end[i] = UnitsToMeters(end[i]);
			}

			tLineOfSightProperties prop;
			prop.InitAsCameraLOS();

			NyaVec3Double dir = end - origin;
			auto dist = dir.length();
			dir.Normalize();

			auto originf = NyaVec3(origin.x, origin.y, origin.z);
			auto endf = NyaVec3(end.x, end.y, end.z);
			auto dirf = NyaVec3(dir.x, dir.y, dir.z);

			prop.fMaxDistance = dist;
			tLineOfSightOut out;
			if (CheckLineOfSight(&prop, pGameFlow->pHost->pUnkForLOS, &originf, &dirf, &out)) {
				trace.plane.dist = MetersToUnits(out.fHitDistance);
				trace.plane.normal.x = out.vHitNormal.x;
				trace.plane.normal.y = out.vHitNormal.y;
				trace.plane.normal.z = out.vHitNormal.z;
				trace.fraction = out.fHitDistance / dist;
				if (trace.fraction > 1.0f) trace.fraction = 1.0f;
				trace.endpos = origin + (dir * out.fHitDistance);
				trace.ent = 0; // dummy entity

				auto offset3 = offset1;
				offset3[0] = std::lerp(offset1[0], offset2[0], trace.fraction);
				offset3[FORWARD] = std::lerp(offset1[FORWARD], offset2[FORWARD], trace.fraction);
				offset3[UP] = offset2[UP];

				for (int i = 0; i < 3; i++) {
					trace.endpos[i] = MetersToUnits(trace.endpos[i]) - offset3[i];
				}

				trace.distFromOrigin = (trace.endpos - originUnits).length();

				//if (trace.plane.dist < minDistForSolid * 0.5) {
				//	trace.allsolid = true;
				//	trace.startsolid = true;
				//}
			}
		}

		return trace;
	}

	// todo this is awful
	pmtrace_t PM_PlayerTrace(NyaVec3Double origin, NyaVec3Double end) {
		std::vector<pmtrace_t> traces;

		auto trace = PM_PlayerTraceInternal(origin, end, {0,0,0}, {0,0,0});
		if (trace.ent != -1) traces.push_back(trace);
		trace = PM_PlayerTraceInternal(origin, end, {0, 0, 0}, {0, -1, 0});
		if (trace.ent != -1) traces.push_back(trace);
		trace = PM_PlayerTraceInternal(origin, end, {0, 0, 0}, {0, 1, 0});
		if (trace.ent != -1) traces.push_back(trace);

		for (int x = -nColDensity; x <= nColDensity; x++) {
			auto posX = (double)x / nColDensity;
			for (int y = -nColDensity; y <= nColDensity; y++) {
				auto posY = (double)y / nColDensity;
				for (int z = -nColDensity; z <= nColDensity; z++) {
					auto posZ = (double)z / nColDensity;

					// checks we already did before
					if (x == 0 && y == 0 && z == 0) continue;
					if (x == 0 && y == 1 && z == 0) continue;
					if (x == 0 && y == -1 && z == 0) continue;

					trace = PM_PlayerTraceInternal(origin, end, { 0, 0, 0 }, {posX, posY, posZ});
					//trace = PM_PlayerTraceInternal(origin, end, { posX, posY, posZ}, {posX, posY, posZ});
					if (trace.ent != -1) traces.push_back(trace);
				}
			}
		}

		// grab closest hit
		for (auto& tr : traces) {
			if (trace.ent == -1 || (tr.distFromOrigin < trace.distFromOrigin && tr.ent != -1)) trace = tr;
		}
		return trace;
	}

	// trace down with offset on both start and end for stair checks
	pmtrace_t PM_PlayerTraceDown(NyaVec3Double origin, NyaVec3Double end) {
		std::vector<pmtrace_t> traces;

		auto trace = PM_PlayerTraceInternal(origin, end, {0, 0, 0}, {0, -1, 0});
		if (trace.ent != -1) traces.push_back(trace);

		for (int x = -nColDensity; x <= nColDensity; x++) {
			auto posX = (double)x / nColDensity;
			for (int z = -nColDensity; z <= nColDensity; z++) {
				auto posZ = (double)z / nColDensity;

				// checks we already did before
				if (x == 0 && z == 0) continue;

				trace = PM_PlayerTraceInternal(origin, end, {posX, 0, posZ}, {posX, -1, posZ});
				if (trace.ent != -1) traces.push_back(trace);
			}
		}

		// grab closest hit
		for (auto& tr : traces) {
			if (trace.ent == -1 || (tr.distFromOrigin < trace.distFromOrigin && tr.ent != -1)) trace = tr;
		}
		return trace;
	}*/

	// for ground movement
	pmtrace_t GetTopFloorForBBox(NyaVec3Double origin) {
		std::vector<pmtrace_t> traces;

		for (int x = -nColDensity; x <= nColDensity; x++) {
			auto posX = (double)x / nColDensity;
			for (int z = -nColDensity; z <= nColDensity; z++) {
				auto posZ = (double)z / nColDensity;

				auto bbox = pmove->player_maxs[pmove->usehull];

				// do a raycast from all sides downwards

				auto start = origin;
				auto end = origin;
				end[0] += posX * bbox.x;
				end[FORWARD] += posZ * bbox[FORWARD];
				end[UP] -= bbox[UP];

				auto trace = PointRaytrace(start, end);
				if (trace.ent != -1) {
					trace.endpos[UP] += bbox[UP];
					traces.push_back(trace);
				}
			}
		}

		auto out = pmtrace_t();
		out.Default();
		out.endpos = {-999999,-999999,-999999};
		for (auto& tr : traces) {
			if (tr.ent != -1 && tr.endpos[UP] > out.endpos[UP]) out = tr;
		}
		return out;
	}

	// for unducking
	pmtrace_t GetBottomCeilingForBBox(NyaVec3Double origin) {
		std::vector<pmtrace_t> traces;

		for (int x = -nColDensity; x <= nColDensity; x++) {
			auto posX = (double)x / nColDensity;
			for (int z = -nColDensity; z <= nColDensity; z++) {
				auto posZ = (double)z / nColDensity;

				auto bbox = pmove->player_maxs[pmove->usehull];

				// do a raycast from all sides upwards

				// start from the very bottom
				auto start = origin;
				start[UP] -= bbox[UP];

				auto end = origin;
				end[0] += posX * bbox.x;
				end[FORWARD] += posZ * bbox[FORWARD];
				end[UP] += bbox[UP];

				auto trace = PointRaytrace(start, end);
				if (trace.ent != -1) {
					trace.endpos[UP] -= bbox[UP];
					traces.push_back(trace);
				}
			}
		}

		auto out = pmtrace_t();
		out.Default();
		out.endpos = {999999,999999,999999};
		for (auto& tr : traces) {
			if (tr.ent != -1 && tr.endpos[UP] < out.endpos[UP]) out = tr;
		}
		return out;
	}

	// for general collisions
	// returns the center as endpoint
	pmtrace_t GetClosestBBoxIntersection(NyaVec3Double origPos, NyaVec3Double targetPos) {
		auto distanceTraveled = (origPos - targetPos).length();

		std::vector<pmtrace_t> traces;

		for (int x = -nColDensity; x <= nColDensity; x++) {
			auto posX = (double)x / nColDensity;
			for (int y = -nColDensity; y <= nColDensity; y++) {
				auto posY = (double)y / nColDensity;
				for (int z = -nColDensity; z <= nColDensity; z++) {
					auto posZ = (double)z / nColDensity;

					auto bbox = pmove->player_maxs[pmove->usehull];

					// cast from the origin outwards
					auto start = targetPos;
					auto offset = targetPos;
					offset.x = posX * bbox.x;
					offset.y = posY * bbox.y;
					offset.z = posZ * bbox.z;

					auto end = targetPos + offset;

					auto trace = PointRaytrace(start, end);
					if (trace.ent != -1) {
						trace.endpos -= offset;
						trace.fraction = (origPos - trace.endpos).length() / distanceTraveled;
						traces.push_back(trace);
					}
				}
			}
		}

		auto out = pmtrace_t();
		out.Default();
		out.endpos = targetPos;
		for (auto& tr : traces) {
			if (tr.ent != -1 && tr.fraction < out.fraction) out = tr;
		}
		return out;
	}

	bool PM_CheckWater()
	{
		NyaVec3Double 	point;
		int		cont;
		int		truecont;
		float     height;
		float		heightover2;

		// Pick a spot just above the players feet.
		point[0] = pmove->origin[0] + (pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0]) * 0.5;
		point[FORWARD] = pmove->origin[FORWARD] + (pmove->player_mins[pmove->usehull][FORWARD] + pmove->player_maxs[pmove->usehull][FORWARD]) * 0.5;
		point[UP] = pmove->origin[UP] + pmove->player_mins[pmove->usehull][UP] + 1;

		// Assume that we are not in water at all.
		pmove->waterlevel = 0;
		pmove->watertype = CONTENTS_EMPTY;

		// Grab point contents.
		cont = GetPointContents(point);
		// Are we under water? (not solid and not empty?)
		if (cont <= CONTENTS_WATER)
		{
			// Set water type
			pmove->watertype = cont;

			// We are at least at level one
			pmove->waterlevel = 1;

			height = (pmove->player_mins[pmove->usehull][UP] + pmove->player_maxs[pmove->usehull][UP]);
			heightover2 = height * 0.5;

			// Now check a point that is at the player hull midpoint.
			point[UP] = pmove->origin[UP] + heightover2;
			cont = GetPointContents(point);
			// If that point is also under water...
			if (cont <= CONTENTS_WATER)
			{
				// Set a higher water level.
				pmove->waterlevel = 2;

				// Now check the eye position.  (view_ofs is relative to the origin)
				point[UP] = pmove->origin[UP] + pmove->view_ofs[UP];

				cont = GetPointContents(point);
				if (cont <= CONTENTS_WATER)
					pmove->waterlevel = 3;  // In over our eyes
			}

			// Adjust velocity based on water current, if any.
			/*if ( ( truecont <= CONTENTS_CURRENT_0 ) &&
				 ( truecont >= CONTENTS_CURRENT_DOWN ) )
			{
				// The deeper we are, the stronger the current.
				static vec3_t current_table[] =
						{
								{1, 0, 0}, {0, 1, 0}, {-1, 0, 0},
								{0, -1, 0}, {0, 0, 1}, {0, 0, -1}
						};

				VectorMA (pmove->basevelocity, 50.0*pmove->waterlevel, current_table[CONTENTS_CURRENT_0 - truecont], pmove->basevelocity);
			}*/
		}

		return pmove->waterlevel > 1;
	}

	float fLastPlaneNormal;
	void PM_CatagorizePosition()
	{
		NyaVec3Double		point;
		pmtrace_t		tr;

		// if the player hull point one unit down is solid, the player
		// is on ground

		// see if standing on something solid

		// Doing this before we move may introduce a potential latency in water detection, but
		// doing it after can get us stuck on the bottom in water if the amount we move up
		// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
		// this several times per frame, so we really need to avoid sticking to the bottom of
		// water on each call, and the converse case will correct itself if called twice.
		PM_CheckWater();

		point[0] = pmove->origin[0];
		point[FORWARD] = pmove->origin[FORWARD];
		point[UP] = pmove->origin[UP] - 2;

		if (pmove->velocity[UP] > 180)   // Shooting up really fast.  Definitely not on ground.
		{
			pmove->onground = -1;
		}
		else
		{
			// Try and move down.
#ifdef NYA_HL_COL_FALLBACK
			tr = GetTopFloorForBBox(point);
#else
			tr = PM_PlayerTraceDown(pmove->origin, point);
#endif

			fLastPlaneNormal = tr.plane.normal[UP];
			// If we hit a steep plane, we are not on ground
			if ( tr.plane.normal[UP] < 0.7)
				pmove->onground = -1;	// too steep
			else
				pmove->onground = tr.ent;  // Otherwise, point to index of ent under us.

			// If we are on something...
			if (pmove->onground != -1)
			{
				pmove->chtexturetype = GetSurfaceTextureFromID(tr.surfaceId);

				// Then we are not in water jump sequence
				pmove->waterjumptime = 0;
				// If we could make the move, drop us down that 1 pixel
				if (pmove->waterlevel < 2/* && !tr.startsolid && !tr.allsolid*/) {
#ifdef NYA_HL_COL_FALLBACK
					// hacked here to only use the endpos Y
					pmove->origin[UP] = tr.endpos[UP];
#else
					VectorCopy (tr.endpos, pmove->origin);
#endif
				}
			}

			// todo
			// Standing on an entity other than the world
			//if (tr.ent > 0)   // So signal that we are touching something.
			//{
			//	PM_AddToTouched(tr, pmove->velocity);
			//}
		}
	}

	void PM_PlayStepSound( int step, float fvol ) {
		static int iSkipStep = 0;
		NyaVec3Double hvel;

		pmove->iStepLeft = !pmove->iStepLeft;

		int irand = (rand() % 2) + (pmove->iStepLeft * 2);

		VectorCopy(pmove->velocity, hvel);
		hvel[UP] = 0.0;

		// irand - 0,1 for right foot, 2,3 for left foot
		// used to alternate left and right foot

		switch (step)
		{
			default:
			case CHAR_TEX_CONCRETE:
				switch (irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_step1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_step3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_step2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_step4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_METAL:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_metal1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_metal3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_metal2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_metal4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_DIRT:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_dirt1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_dirt3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_dirt2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_dirt4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_VENT:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_duct1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_duct3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_duct2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_duct4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_GRATE:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_grate1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_grate3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_grate2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_grate4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_TILE:
				if ( !(rand() % 5) )
					irand = 4;

				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_tile1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_tile3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_tile2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_tile4.wav", fvol );	break;
					case 4: PlayGameSound("player/pl_tile5.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_SLOSH:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_slosh1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_slosh3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_slosh2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_slosh4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_WADE:
				if ( iSkipStep == 0 )
				{
					iSkipStep++;
					break;
				}

				if ( iSkipStep++ == 3 )
				{
					iSkipStep = 0;
				}

				switch (irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_wade1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_wade2.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_wade3.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_wade4.wav", fvol );	break;
				}
				break;
			case CHAR_TEX_LADDER:
				switch(irand)
				{
					// right foot
					case 0:	PlayGameSound("player/pl_ladder1.wav", fvol );	break;
					case 1:	PlayGameSound("player/pl_ladder3.wav", fvol );	break;
						// left foot
					case 2:	PlayGameSound("player/pl_ladder2.wav", fvol );	break;
					case 3:	PlayGameSound("player/pl_ladder4.wav", fvol );	break;
				}
				break;
		}
	}

	void PM_UpdateStepSound()
	{
		int	fWalking;
		float fvol;
		NyaVec3Double knee;
		NyaVec3Double feet;
		NyaVec3Double center;
		float height;
		float speed;
		float velrun;
		float velwalk;
		float flduck;
		int	fLadder;
		int step;

		if ( pmove->flTimeStepSound > 0 )
			return;

		if ( pmove->flags & FL_FROZEN )
			return;

		// moved to walkmove
		//PM_CatagorizeTextureType();

		speed = pmove->velocity.length();

		// determine if we are on a ladder
		fLadder = ( pmove->movetype == MOVETYPE_FLY );// IsOnLadder();

		// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!
		if ( ( pmove->flags & FL_DUCKING) || fLadder )
		{
			velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
			velrun = 80;		// UNDONE: Move walking to server
			flduck = 100;
		}
		else
		{
			velwalk = 120;
			velrun = 210;
			flduck = 0;
		}

		// If we're on a ladder or on the ground, and we're moving fast enough,
		//  play step sound.  Also, if pmove->flTimeStepSound is zero, get the new
		//  sound right away - we just started moving in new level.
		if ( (fLadder || ( pmove->onground != -1 ) ) &&
			 ( pmove->velocity.length() > 0.0 ) &&
			 ( speed >= velwalk || !pmove->flTimeStepSound ) )
		{
			fWalking = speed < velrun;

			VectorCopy( pmove->origin, center );
			VectorCopy( pmove->origin, knee );
			VectorCopy( pmove->origin, feet );

			height = pmove->player_maxs[ pmove->usehull ][ UP ] - pmove->player_mins[ pmove->usehull ][ UP ];

			knee[UP] = pmove->origin[UP] - 0.3 * height;
			feet[UP] = pmove->origin[UP] - 0.5 * height;

			if (GetPointContents(knee) == CONTENTS_WATER)
			{
				fvol = 0.65;
				pmove->flTimeStepSound = 600;
			}
			else if (GetPointContents(feet) == CONTENTS_WATER)
			{
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
			}
			else
			{
				step = pmove->chtexturetype;

				switch ( pmove->chtexturetype )
				{
					default:
					case CHAR_TEX_CONCRETE:
						fvol = fWalking ? 0.2 : 0.5;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_METAL:
						fvol = fWalking ? 0.2 : 0.5;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_DIRT:
						fvol = fWalking ? 0.25 : 0.55;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_VENT:
						fvol = fWalking ? 0.4 : 0.7;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_GRATE:
						fvol = fWalking ? 0.2 : 0.5;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_TILE:
						fvol = fWalking ? 0.2 : 0.5;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;

					case CHAR_TEX_SLOSH:
						fvol = fWalking ? 0.2 : 0.5;
						pmove->flTimeStepSound = fWalking ? 400 : 300;
						break;
				}
			}

			pmove->flTimeStepSound += flduck; // slower step time if ducking

			// play the sound
			// 35% volume if ducking
			if ( pmove->flags & FL_DUCKING )
			{
				fvol *= 0.35;
			}

			PM_PlayStepSound( step, fvol );
		}
	}

	void PM_UnDuck()
	{
		int i;
		pmtrace_t trace;
		NyaVec3Double newOrigin;

		VectorCopy( pmove->origin, newOrigin );

		if ( pmove->onground != -1 )
		{
			for ( i = 0; i < 3; i++ )
			{
				newOrigin[i] += ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
			}
		}

#ifdef NYA_HL_COL_FALLBACK
		trace = GetBottomCeilingForBBox(newOrigin);
		trace.startsolid = trace.ent != -1;
#else
		trace = PM_PlayerTrace(newOrigin, newOrigin);
#endif

		if ( !trace.startsolid )
		{
			pmove->usehull = 0;

			// Oh, no, changing hulls stuck us into something, try unsticking downward first.
#ifdef NYA_HL_COL_FALLBACK
			trace = GetBottomCeilingForBBox(newOrigin);
			trace.startsolid = trace.ent != -1;
#else
			trace = PM_PlayerTrace(newOrigin, newOrigin);
#endif
			if ( trace.startsolid )
			{
				// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
				lastConsoleMsg = ( "unstick got stuck\n" );
				pmove->usehull = 1;
				return;
			}

			pmove->flags &= ~FL_DUCKING;
			pmove->bInDuck  = false;
			pmove->view_ofs[UP] = VEC_VIEW;
			pmove->flDuckTime = 0;

			VectorCopy( newOrigin, pmove->origin );

			// Recatagorize position since ducking can change origin
			PM_CatagorizePosition();
		}
	}

	float PM_SplineFraction( float value, float scale )
	{
		float valueSquared;

		value = scale * value;
		valueSquared = value * value;

		// Nice little ease-in, ease-out spline-like curve
		return 3 * valueSquared - 2 * valueSquared * value;
	}

	void PM_Duck()
	{
		int i;
		float time;
		float duckFraction;

		int buttonsChanged	= (pmove->oldbuttons ^ pmove->cmd.buttons);	// These buttons have changed this frame
		int nButtonPressed	=  buttonsChanged & pmove->cmd.buttons;		// The changed ones still down are "pressed"

		int duckchange		= buttonsChanged & IN_DUCK ? 1 : 0;
		int duckpressed		= nButtonPressed & IN_DUCK ? 1 : 0;

		if (pmove->cmd.buttons & IN_DUCK)
		{
			pmove->oldbuttons |= IN_DUCK;
		}
		else
		{
			pmove->oldbuttons &= ~IN_DUCK;
		}

		// Prevent ducking if the iuser3 variable is set
		if (pmove->dead)
		{
			// Try to unduck
			if (pmove->flags & FL_DUCKING)
			{
				PM_UnDuck();
			}
			return;
		}

		if ( pmove->flags & FL_DUCKING )
		{
			pmove->cmd.forwardmove *= PLAYER_DUCKING_MULTIPLIER;
			pmove->cmd.sidemove    *= PLAYER_DUCKING_MULTIPLIER;
			pmove->cmd.upmove      *= PLAYER_DUCKING_MULTIPLIER;
		}

		if ( ( pmove->cmd.buttons & IN_DUCK ) || ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
		{
			if ( pmove->cmd.buttons & IN_DUCK )
			{
				if ( (nButtonPressed & IN_DUCK ) && !( pmove->flags & FL_DUCKING ) )
				{
					// Use 1 second so super long jump will work
					pmove->flDuckTime = 1000;
					pmove->bInDuck    = true;
				}

				time = std::max( 0.0, ( 1.0 - (float)pmove->flDuckTime / 1000.0 ) );

				if ( pmove->bInDuck )
				{
					// Finish ducking immediately if duck time is over or not on ground
					if ( ( (float)pmove->flDuckTime / 1000.0 <= ( 1.0 - TIME_TO_DUCK ) ) ||
						 ( pmove->onground == -1 ) )
					{
						pmove->usehull = 1;
						pmove->view_ofs[UP] = VEC_DUCK_VIEW;
						pmove->flags |= FL_DUCKING;
						pmove->bInDuck = false;

						// HACKHACK - Fudge for collision bug - no time to fix this properly
						if ( pmove->onground != -1 )
						{
							for ( i = 0; i < 3; i++ )
							{
								pmove->origin[i] -= ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
							}

							// todo
							// See if we are stuck?
							//PM_FixPlayerCrouchStuck( STUCK_MOVEUP );

							// Recatagorize position since ducking can change origin
							PM_CatagorizePosition();
						}
					}
					else
					{
						float fMore = (VEC_DUCK_HULL_MIN - VEC_HULL_MIN);

						// Calc parametric time
						duckFraction = PM_SplineFraction( time, (1.0/TIME_TO_DUCK) );
						pmove->view_ofs[UP] = ((VEC_DUCK_VIEW - fMore) * duckFraction) + (VEC_VIEW * (1-duckFraction));
					}
				}
			}
			else
			{
				// Try to unduck
				PM_UnDuck();
			}
		}
	}

	void PM_NoClip()
	{
		int			i;
		NyaVec3Double		wishvel;
		float		fmove, smove;

		// Copy movement amounts
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		VectorNormalize ( pmove->forward );
		VectorNormalize ( pmove->right );

		for (i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		{
			wishvel[i] = pmove->forward[i]*fmove + pmove->right[i]*smove;
		}
		wishvel[UP] += pmove->cmd.upmove;

		VectorMA (pmove->origin, pmove->frametime, wishvel, pmove->origin);

		// Zero out the velocity so that we don't accumulate a huge downward velocity from
		//  gravity, etc.
		VectorClear( pmove->velocity );
	}

	bool PM_InWater() {
		return pmove->waterlevel > 1;
	}

	// empty for now, might not be required
	void PM_CheckVelocity() {
		//
		// bound velocity
		//
		for (int i=0 ; i<3 ; i++)
		{
			// Bound it.
			if (pmove->velocity[i] > movevars->maxvelocity)
			{
				lastConsoleMsg = ("PM  Got a velocity too high on" + std::to_string(i));
				pmove->velocity[i] = movevars->maxvelocity;
			}
			else if (pmove->velocity[i] < -movevars->maxvelocity)
			{
				lastConsoleMsg = ("PM  Got a velocity too low on" + std::to_string(i));
				pmove->velocity[i] = -movevars->maxvelocity;
			}
		}
	}

	void PM_FixupGravityVelocity ()
	{
		float	ent_gravity;

		if ( pmove->waterjumptime )
			return;

		if (pmove->gravity)
			ent_gravity = pmove->gravity;
		else
			ent_gravity = 1.0;

		// Get the correct velocity for the end of the dt
		pmove->velocity[UP] -= (ent_gravity * movevars->gravity * pmove->frametime * 0.5 );

		PM_CheckVelocity();
	}

	void PM_AddCorrectGravity ()
	{
		float	ent_gravity;

		if ( pmove->waterjumptime )
			return;

		if (pmove->gravity)
			ent_gravity = pmove->gravity;
		else
			ent_gravity = 1.0;

		// Add gravity so they'll be in the correct position during movement
		// yes, this 0.5 looks wrong, but it's not.
		pmove->velocity[UP] -= (ent_gravity * movevars->gravity * 0.5 * pmove->frametime );
		pmove->velocity[UP] += pmove->basevelocity[UP] * pmove->frametime;
		pmove->basevelocity[UP] = 0;

		PM_CheckVelocity();
	}

	void PM_Friction() {
		NyaVec3Double vel;
		float	speed, newspeed, control;
		float	friction;
		float	drop;
		NyaVec3Double newvel;

		// If we are in water jump cycle, don't apply friction
		if (pmove->waterjumptime)
			return;

		// Get velocity
		vel = pmove->velocity;

		// Calculate speed
		speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1] + vel[2]*vel[2]);

		// If too slow, return
		if (speed < 0.1f) {
			return;
		}

		drop = 0;

		// apply ground friction
		if (pmove->onground != -1) { // On an entity that is the ground
			NyaVec3Double start, stop;
			pmtrace_t trace;

			start[0] = stop[0] = pmove->origin[0] + vel[0]/speed*16;
			start[FORWARD] = stop[FORWARD] = pmove->origin[FORWARD] + vel[FORWARD]/speed*16;
			start[UP] = pmove->origin[UP] + pmove->player_mins[pmove->usehull][UP];
			stop[UP] = start[UP] - 34;

			// todo - edgefriction is not implemented for the fallback
#ifdef NYA_HL_COL_FALLBACK
			friction = movevars->friction;
#else
			trace = PM_PlayerTrace (start, stop );

			if (trace.fraction == 1.0f)
				friction = movevars->friction*movevars->edgefriction;
			else
				friction = movevars->friction;
#endif

			friction *= pmove->friction;  // player friction?

			// Bleed off some speed, but if we have less than the bleed
			//  threshhold, bleed the theshold amount.
			control = (speed < movevars->stopspeed) ?
					  movevars->stopspeed : speed;
			// Add the amount to t'he drop amount.
			drop += control*friction*pmove->frametime;
		}

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;

		// Determine proportion of old speed we are using.
		newspeed /= speed;

		// Adjust velocity according to proportion.
		newvel[0] = vel[0] * newspeed;
		newvel[1] = vel[1] * newspeed;
		newvel[2] = vel[2] * newspeed;

		VectorCopy( newvel, pmove->velocity );
	}

	void PM_AirAccelerate(NyaVec3Double wishdir, float wishspeed, float accel) {
		int			i;
		float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;

		if (pmove->dead)
			return;
		if (pmove->waterjumptime)
			return;

		// Cap speed
		//wishspd = VectorNormalize (pmove->wishveloc);

		if (wishspd > 30)
			wishspd = 30;
		// Determine veer amount
		currentspeed = DotProduct (pmove->velocity, wishdir);
		// See how much to add
		addspeed = wishspd - currentspeed;
		// If not adding any, done.
		if (addspeed <= 0)
			return;
		// Determine acceleration speed after acceleration

		accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
		// Cap it
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		// Adjust pmove vel.
		for (i=0 ; i<3 ; i++)
		{
			pmove->velocity[i] += accelspeed*wishdir[i];
		}
	}

	int PM_ClipVelocity (const NyaVec3Double& in, const NyaVec3Double& normal, NyaVec3Double& out, float overbounce)
	{
		float	backoff;
		float	change;
		float angle;
		int		i, blocked;

		angle = normal[UP];

		blocked = 0x00;            // Assume unblocked.
		if (angle > 0)      // If the plane that is blocking us has a positive z component, then assume it's a floor.
			blocked |= 0x01;		//
		if (!angle)         // If the plane has no Z, it is vertical (wall/step)
			blocked |= 0x02;		//

		// Determine how far along plane to slide based on incoming direction.
		// Scale by overbounce factor.
		backoff = DotProduct (in, normal) * overbounce;

		for (i=0 ; i<3 ; i++)
		{
			change = normal[i]*backoff;
			out[i] = in[i] - change;
			// If out velocity is too small, zero it out.
			if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
				out[i] = 0;
		}

		// Return blocking flags.
		return blocked;
	}

	int PM_FlyMove()
	{
		int			bumpcount, numbumps;
		NyaVec3Double		dir;
		float		d;
		int			numplanes;
		NyaVec3Double		planes[MAX_CLIP_PLANES];
		NyaVec3Double		primal_velocity, original_velocity;
		NyaVec3Double      new_velocity;
		int			i, j;
		pmtrace_t	trace;
		NyaVec3Double		end;
		float		time_left, allFraction;
		int			blocked;

		//numbumps  = 4;           // Bump up to four times
		numbumps  = 1;           // Bump up to four times

		blocked   = 0;           // Assume not blocked
		numplanes = 0;           //  and not sliding along any planes
		VectorCopy (pmove->velocity, original_velocity);  // Store original velocity
		VectorCopy (pmove->velocity, primal_velocity);

		allFraction = 0;
		time_left = pmove->frametime;   // Total time for this movement operation.

		for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
		{
			if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
				break;

			// Assume we can move all the way from the current origin to the
			//  end point.
			for (i=0 ; i<3 ; i++)
				end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

			// See if we can make it from origin to end point.
#ifdef NYA_HL_COL_FALLBACK
			trace = GetClosestBBoxIntersection(pmove->origin, end);
#else
			trace = PM_PlayerTrace(pmove->origin, end);
#endif

			allFraction += trace.fraction;
			// If we started in a solid object, or we were in solid space
			//  the whole way, zero out our velocity and return that we
			//  are blocked by floor and wall.
			if (trace.allsolid)
			{	// entity is trapped in another solid
				VectorCopy (vec3_origin, pmove->velocity);
				lastConsoleMsg = ("Trapped 4\n");
				return 4;
			}

			// If we moved some portion of the total distance, then
			//  copy the end position into the pmove->origin and
			//  zero the plane counter.
			if (trace.fraction > 0)
			{	// actually covered some distance
				VectorCopy (trace.endpos, pmove->origin);
				VectorCopy (pmove->velocity, original_velocity);
				numplanes = 0;
			}

			// If we covered the entire distance, we are done
			//  and can return.
			if (trace.fraction == 1.0f)
				break;		// moved the entire distance

			//if (!trace.ent)
			//	Sys_Error ("PM_PlayerTrace: !trace.ent");

			// todo
			// Save entity that blocked us (since fraction was < 1.0)
			//  for contact
			// Add it if it's not already in the list!!!
			//PM_AddToTouched(trace, pmove->velocity);

			// If the plane we hit has a high z component in the normal, then
			//  it's probably a floor
			if (trace.plane.normal[UP] > 0.7)
			{
				blocked |= 1;		// floor
			}
			// If the plane has a zero z component in the normal, then it's a
			//  step or wall
			if (!trace.plane.normal[UP])
			{
				blocked |= 2;		// step / wall
				lastConsoleMsg = ("Blocked by " + std::to_string(trace.ent));
			}

			// Reduce amount of pmove->frametime left by total time left * fraction
			//  that we covered.
			time_left -= time_left * trace.fraction;

			// Did we run out of planes to clip against?
			if (numplanes >= MAX_CLIP_PLANES)
			{	// this shouldn't really happen
				//  Stop our movement if so.
				VectorCopy (vec3_origin, pmove->velocity);
				lastConsoleMsg = ("Too many planes 4\n");

				break;
			}

			// Set up next clipping plane
			VectorCopy (trace.plane.normal, planes[numplanes]);
			numplanes++;
			//

			// modify original_velocity so it parallels all of the clip planes
			//
			// relfect player velocity
			// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
			//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
			if (	numplanes == 1 &&
					pmove->movetype == MOVETYPE_WALK &&
					((pmove->onground == -1) || (pmove->friction != 1)) )
			{
				for ( i = 0; i < numplanes; i++ )
				{
					if ( planes[i][UP] > 0.7  )
					{// floor or slope
						PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
						VectorCopy( new_velocity, original_velocity );
					}
					else
						PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1.0 + movevars->bounce * (1-pmove->friction) );
				}

				VectorCopy( new_velocity, pmove->velocity );
				VectorCopy( new_velocity, original_velocity );
			}
			else
			{
				for (i=0 ; i<numplanes ; i++)
				{
					PM_ClipVelocity (
							original_velocity,
							planes[i],
							pmove->velocity,
							1);
					for (j=0 ; j<numplanes ; j++)
						if (j != i)
						{
							// Are we now moving against this plane?
							if (DotProduct (pmove->velocity, planes[j]) < 0)
								break;	// not ok
						}
					if (j == numplanes)  // Didn't have to clip, so we're ok
						break;
				}

				// Did we go all the way through plane set
				if (i != numplanes)
				{	// go along this plane
					// pmove->velocity is set in clipping call, no need to set again.
					;
				}
				else
				{	// go along the crease
					if (numplanes != 2)
					{
						lastConsoleMsg = ("clip velocity, numplanes == " + std::to_string(numplanes));
						VectorCopy (vec3_origin, pmove->velocity);
						//Con_DPrintf("Trapped 4\n");

						break;
					}
					CrossProduct (planes[0], planes[1], dir);
					d = DotProduct (dir, pmove->velocity);
					VectorScale (dir, d, pmove->velocity );
				}

				//
				// if original velocity is against the original velocity, stop dead
				// to avoid tiny occilations in sloping corners
				//
				if (DotProduct (pmove->velocity, primal_velocity) <= 0)
				{
					lastConsoleMsg = ("Back\n");
					VectorCopy (vec3_origin, pmove->velocity);
					break;
				}
			}
		}

		if ( allFraction == 0 )
		{
			VectorCopy (vec3_origin, pmove->velocity);
			lastConsoleMsg = ( "Don't stick\n" );
		}

		return blocked;
	}

	void PM_WaterMove()
	{
		int		i;
		NyaVec3Double	wishvel;
		float	wishspeed;
		NyaVec3Double	wishdir;
		NyaVec3Double	start, dest;
		NyaVec3Double  temp;
		pmtrace_t	trace;

		float speed, newspeed, addspeed, accelspeed;

		//
		// user intentions
		//
		for (i=0 ; i<3 ; i++)
			wishvel[i] = pmove->forward[i]*pmove->cmd.forwardmove + pmove->right[i]*pmove->cmd.sidemove;

		// Sinking after no other movement occurs
		if (!pmove->cmd.forwardmove && !pmove->cmd.sidemove && !pmove->cmd.upmove)
			wishvel[UP] -= 60;		// drift towards bottom
		else  // Go straight up by upmove amount.
			wishvel[UP] += pmove->cmd.upmove;

		// Copy it over and determine speed
		VectorCopy (wishvel, wishdir);
		wishspeed = VectorNormalize(wishdir);

		// Cap speed.
		if (wishspeed > pmove->maxspeed)
		{
			VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
			wishspeed = pmove->maxspeed;
		}
		// Slow us down a bit.
		wishspeed *= 0.8;

		VectorAdd (pmove->velocity, pmove->basevelocity, pmove->velocity);
		// Water friction
		VectorCopy(pmove->velocity, temp);
		speed = VectorNormalize(temp);
		if (speed)
		{
			newspeed = speed - pmove->frametime * speed * movevars->friction * pmove->friction;

			if (newspeed < 0)
				newspeed = 0;
			VectorScale (pmove->velocity, newspeed/speed, pmove->velocity);
		}
		else
			newspeed = 0;

		//
		// water acceleration
		//
		if (wishspeed < 0.1f) {
			return;
		}

		addspeed = wishspeed - newspeed;
		if (addspeed > 0)
		{

			VectorNormalize(wishvel);
			accelspeed = movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
			if (accelspeed > addspeed)
				accelspeed = addspeed;

			for (i = 0; i < 3; i++)
				pmove->velocity[i] += accelspeed * wishvel[i];
		}

#ifndef NYA_HL_COL_FALLBACK
		// Now move
		// assume it is a stair or a slope, so press down from stepheight above
		VectorMA (pmove->origin, pmove->frametime, pmove->velocity, dest);
		VectorCopy (dest, start);
		start[UP] += movevars->stepsize + 1;

		trace = PM_PlayerTrace(start, dest);

		if (!trace.startsolid && !trace.allsolid)
		{	// walked up the step, so just keep result and exit
			VectorCopy (trace.endpos, pmove->origin);
			return;
		}
#endif

		// Try moving straight along out normal path.
		PM_FlyMove();
	}

	void PM_AirMove()
	{
		NyaVec3Double		wishvel;
		float		fmove, smove;
		NyaVec3Double		wishdir;
		float		wishspeed;

		// Copy movement amounts
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		// Zero out z components of movement vectors
		pmove->forward[UP] = 0;
		pmove->right[UP]   = 0;
		// Renormalize
		VectorNormalize (pmove->forward);
		VectorNormalize (pmove->right);

		// Determine x and y parts of velocity
		wishvel[0] = pmove->forward[0]*fmove + pmove->right[0]*smove;
		wishvel[FORWARD] = pmove->forward[FORWARD]*fmove + pmove->right[FORWARD]*smove;
		// Zero out z part of velocity
		wishvel[UP] = 0;

		// Determine maginitude of speed of move
		VectorCopy (wishvel, wishdir);
		wishspeed = VectorNormalize(wishdir);

		// Clamp to server defined max speed
		if (wishspeed > pmove->maxspeed)
		{
			VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
			wishspeed = pmove->maxspeed;
		}

		PM_AirAccelerate (wishdir, wishspeed, movevars->airaccelerate);

		// Add in any base velocity to the current velocity.
		VectorAdd (pmove->velocity, pmove->basevelocity, pmove->velocity );

		PM_FlyMove();
	}

	void PM_Accelerate(NyaVec3Double wishdir, float wishspeed, float accel)
	{
		int			i;
		float		addspeed, accelspeed, currentspeed;

		// Dead player's don't accelerate
		if (pmove->dead)
			return;

		// If waterjumping, don't accelerate
		if (pmove->waterjumptime)
			return;

		// See if we are changing direction a bit
		currentspeed = DotProduct (pmove->velocity, wishdir);

		// Reduce wishspeed by the amount of veer.
		addspeed = wishspeed - currentspeed;

		// If not going to add any speed, done.
		if (addspeed <= 0)
			return;

		// Determine amount of accleration.
		accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;

		// Cap at addspeed
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		// Adjust velocity.
		for (i=0 ; i<3 ; i++) {
			pmove->velocity[i] += accelspeed * wishdir[i];
		}
	}

	void PM_WalkMove() {
		int			clip;
		int			oldonground;

		NyaVec3Double		wishvel;
		float       spd;
		float		fmove, smove;
		NyaVec3Double		wishdir;
		float		wishspeed;

		NyaVec3Double dest, start;
		NyaVec3Double original, originalvel;
		NyaVec3Double down, downvel;
		float downdist, updist;

		pmtrace_t trace;

		// Copy movement amounts
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		// Zero out z components of movement vectors
		pmove->forward[UP] = 0;
		pmove->right[UP]   = 0;

		VectorNormalize (pmove->forward);  // Normalize remainder of vectors.
		VectorNormalize (pmove->right);    //

		wishvel[0] = pmove->forward[0]*fmove + pmove->right[0]*smove; // Determine x and y parts of velocity
		wishvel[FORWARD] = pmove->forward[FORWARD]*fmove + pmove->right[FORWARD]*smove;
		wishvel[UP] = 0;             // Zero out z part of velocity

		VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
		wishspeed = VectorNormalize(wishdir);

		//
		// Clamp to server defined max speed
		//
		if (wishspeed > pmove->maxspeed)
		{
			VectorScale (wishvel, pmove->maxspeed/wishspeed, wishvel);
			wishspeed = pmove->maxspeed;
		}

		// Set pmove velocity
		pmove->velocity[UP] = 0;
		PM_Accelerate (wishdir, wishspeed, movevars->accelerate);
		pmove->velocity[UP] = 0;

		// Add in any base velocity to the current velocity.
		VectorAdd (pmove->velocity, pmove->basevelocity, pmove->velocity );

		spd = pmove->velocity.length();

		if (spd < 1.0f)
		{
			VectorClear(pmove->velocity);
			lastConsoleMsg = "clearing small speed";
			return;
		}

		// If we are not moving, do nothing
		//if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
		//	return;

		oldonground = pmove->onground;

		// first try just moving to the destination
		dest[0] = pmove->origin[0] + pmove->velocity[0]*pmove->frametime;
		dest[FORWARD] = pmove->origin[FORWARD] + pmove->velocity[FORWARD]*pmove->frametime;
		dest[UP] = pmove->origin[UP];

		// first try moving directly to the next spot
		VectorCopy (dest, start);

#ifdef NYA_HL_COL_FALLBACK
		trace = GetClosestBBoxIntersection(pmove->origin, dest);
#else
		trace = PM_PlayerTrace(pmove->origin, dest);
#endif

		// If we made it all the way, then copy trace end
		//  as new player position.
		if (trace.fraction == 1)
		{
			VectorCopy (trace.endpos, pmove->origin);
			return;
		}

		if (oldonground == -1 &&   // Don't walk up stairs if not on ground.
			pmove->waterlevel  == 0)
			return;

		if (pmove->waterjumptime)         // If we are jumping out of water, don't do anything more.
			return;

		// Try sliding forward both on ground and up 16 pixels
		//  take the move that goes farthest
		VectorCopy (pmove->origin, original);       // Save out original pos &
		VectorCopy (pmove->velocity, originalvel);  //  velocity.

		// Slide move
		clip = PM_FlyMove ();

		// Copy the results out
		VectorCopy (pmove->origin  , down);
		VectorCopy (pmove->velocity, downvel);

		// Reset original values.
		VectorCopy (original, pmove->origin);

		VectorCopy (originalvel, pmove->velocity);

		// Start out up one stair height
		VectorCopy (pmove->origin, dest);
		dest[UP] += movevars->stepsize;

#ifdef NYA_HL_COL_FALLBACK
		trace = GetClosestBBoxIntersection(pmove->origin, dest);
#else
		trace = PM_PlayerTrace(pmove->origin, dest);
#endif

		// If we started okay and made it part of the way at least,
		//  copy the results to the movement start position and then
		//  run another move try.
		if (!trace.startsolid && !trace.allsolid)
		{
			VectorCopy (trace.endpos, pmove->origin);
		}

		// slide move the rest of the way.
		clip = PM_FlyMove ();

		// Now try going back down from the end point
		//  press down the stepheight
		VectorCopy (pmove->origin, dest);
		dest[UP] -= movevars->stepsize;

#ifdef NYA_HL_COL_FALLBACK
		trace = GetClosestBBoxIntersection(pmove->origin, dest);
#else
		trace = PM_PlayerTrace(pmove->origin, dest);
#endif

		// If we are not on the ground any more then
		//  use the original movement attempt
		if ( trace.plane.normal[UP] < 0.7)
			goto usedown;

		// If the trace ended up in empty space, copy the end
		//  over to the origin.
		if (!trace.startsolid && !trace.allsolid)
		{
			VectorCopy (trace.endpos, pmove->origin);
		}
		// Copy this origion to up.
		VectorCopy (pmove->origin, pmove->up);

		// decide which one went farther
		downdist = (down[0] - original[0])*(down[0] - original[0])
				   + (down[FORWARD] - original[FORWARD])*(down[FORWARD] - original[FORWARD]);
		updist   = (pmove->up[0]   - original[0])*(pmove->up[0]   - original[0])
				   + (pmove->up[FORWARD]   - original[FORWARD])*(pmove->up[FORWARD]   - original[FORWARD]);

		if (downdist > updist)
		{
			usedown:
			VectorCopy (down   , pmove->origin);
			VectorCopy (downvel, pmove->velocity);
		} else // copy z value from slide move
			pmove->velocity[UP] = downvel[UP];

	}

	void PM_Jump()
	{
		if (pmove->dead)
		{
			pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
			return;
		}

		// See if we are waterjumping.  If so, decrement count and return.
		if ( pmove->waterjumptime )
		{
			pmove->waterjumptime -= pmove->cmd.msec;
			if (pmove->waterjumptime < 0)
			{
				pmove->waterjumptime = 0;
			}
			return;
		}

		// If we are in the water most of the way...
		if (pmove->waterlevel >= 2)
		{	// swimming, not jumping
			pmove->onground = -1;

			if (pmove->watertype == CONTENTS_WATER)    // We move up a certain amount
				pmove->velocity[UP] = 100;
			else if (pmove->watertype == CONTENTS_SLIME)
				pmove->velocity[UP] = 80;
			else  // LAVA
				pmove->velocity[UP] = 50;

			// play swiming sound
			if (pmove->flSwimTime <= 0)
			{
				// Don't play sound again for 1 second
				pmove->flSwimTime = 1000;

				switch ( rand() % 4 )
				{
					case 0:
						PlayGameSound( "player/pl_wade1.wav", 1 );
						break;
					case 1:
						PlayGameSound( "player/pl_wade2.wav", 1 );
						break;
					case 2:
						PlayGameSound( "player/pl_wade3.wav", 1 );
						break;
					case 3:
						PlayGameSound( "player/pl_wade4.wav", 1 );
						break;
				}
			}

			return;
		}

		// No more effect
		if (pmove->onground == -1) {
			pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
			return;		// in air, so no effect
		}

		if ( !bAutoBhop && pmove->oldbuttons & IN_JUMP )
			return;		// don't pogo stick

		// In the air now.
		pmove->onground = -1;

		//PM_PreventMegaBunnyJumping();

		PM_PlayStepSound( pmove->chtexturetype, 1.0 );

		// See if user can super long jump?
		bool cansuperjump = bCanLongJump;

		// Acclerate upward
		// If we are ducking...
		if ( ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
		{
			// Adjust for super long jump module
			// UNDONE -- note this should be based on forward angles, not current velocity.
			if ( cansuperjump &&
				 ( pmove->cmd.buttons & IN_DUCK ) &&
				 ( pmove->flDuckTime > 0 ) &&
				 pmove->velocity.length() > 50 )
			{
				pmove->punchangle[PITCH] = -5;

				pmove->velocity[0] = pmove->forward[0] * PLAYER_LONGJUMP_SPEED * 1.6;
				pmove->velocity[FORWARD] = pmove->forward[FORWARD] * PLAYER_LONGJUMP_SPEED * 1.6;
				pmove->velocity[UP] = sqrt(2 * 800 * 56.0);
			}
			else
			{
				pmove->velocity[UP] = sqrt(2 * 800 * 45.0);
			}
		}
		else
		{
			pmove->velocity[UP] = sqrt(2 * 800 * 45.0);
		}

		if (bABH || bABHMixed) {
			NyaVec3Double fwd, right, up;
			AngleVectors(pmove->angles, fwd, right, up);
			fwd[UP] = 0;
			VectorNormalize(fwd);

			// hack for hl1 to have more responsive duck ABH
			bool isSprinting = (pmove->cmd.buttons & IN_RUN) == 0;
			if (pmove->bInDuck) isSprinting = false;

			auto vel2D = pmove->velocity;
			vel2D[UP] = 0;
			auto velLength2D = vel2D.length();

			// We give a certain percentage of the current forward movement as a bonus to the jump speed.  That bonus is clipped
			// to not accumulate over time.
			float flSpeedBoostPerc = (!isSprinting && !pmove->bInDuck) ? 0.5f : 0.1f;
			float flSpeedAddition = std::abs(pmove->cmd.forwardmove * flSpeedBoostPerc);
			float flMaxSpeed = pmove->maxspeed + (pmove->maxspeed * flSpeedBoostPerc);
			float flNewSpeed = (flSpeedAddition + velLength2D);

			// If we're over the maximum, we want to only boost as much as will get us to the goal speed
			if (flNewSpeed > flMaxSpeed) {
				flSpeedAddition -= flNewSpeed - flMaxSpeed;
			}

			if (pmove->cmd.forwardmove < 0.0f)
				flSpeedAddition *= -1.0f;

			// Add it on
			auto endVel = pmove->velocity;
			VectorAdd((fwd * flSpeedAddition), pmove->velocity, endVel);
			if (!bABHMixed || endVel.length() > pmove->velocity.length()) {
				pmove->velocity = endVel;
			}
		}

		// Decay it for simulation
		PM_FixupGravityVelocity();

		// Flag that we jumped.
		pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
	}

	void PM_CheckFalling()
	{
		if ( pmove->onground != -1 &&
			 !pmove->dead &&
			 pmove->flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
		{
			float fvol = 0.5;

			if ( pmove->waterlevel > 0 )
			{

			}
			else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
			{
				// NOTE:  In the original game dll , there were no breaks after these cases, causing the first one to
				// cascade into the second
				//switch ( RandomLong(0,1) )
				//{
				//case 0:
				//PlayGameSound( "player/pl_fallpain2.wav", 1 );
				//break;
				//case 1:
				PlayGameSound( "player/pl_fallpain3.wav", 1 );
				//	break;
				//}
				fvol = 1.0;
			}
			else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2 )
			{
				fvol = 0.85;
			}
			else if ( pmove->flFallVelocity < PLAYER_MIN_BOUNCE_SPEED )
			{
				fvol = 0;
			}

			if ( fvol > 0.0 )
			{
				// Play landing step right away
				pmove->flTimeStepSound = 0;

				PM_UpdateStepSound();

				// play step sound for current texture
				PM_PlayStepSound(  pmove->chtexturetype, fvol );

				// Knock the screen around a little bit, temporary effect
				pmove->punchangle[ ROLL ] = pmove->flFallVelocity * 0.013;	// punch z axis

				if ( pmove->punchangle[ PITCH ] > 8 )
				{
					pmove->punchangle[ PITCH ] = 8;
				}
			}
		}

		if ( pmove->onground != -1 )
		{
			pmove->flFallVelocity = 0;
		}
	}

	void PM_WaterJump()
	{
		if (pmove->waterjumptime > 10000)
		{
			pmove->waterjumptime = 10000;
		}

		if (!pmove->waterjumptime)
			return;

		pmove->waterjumptime -= pmove->cmd.msec;
		if ( pmove->waterjumptime < 0 ||
			 !pmove->waterlevel )
		{
			pmove->waterjumptime = 0;
			pmove->flags &= ~FL_WATERJUMP;
		}

		pmove->velocity[0] = pmove->movedir[0];
		pmove->velocity[FORWARD] = pmove->movedir[FORWARD];
	}

	void PM_PlayerMove()
	{
		physent_t *pLadder = NULL;

		// Adjust speeds etc.
		PM_CheckParamters();

		// Assume we don't touch anything
		pmove->numtouch = 0;

		// # of msec to apply movement
		pmove->frametime = gTimer.fDeltaTime;
		pmove->cmd.msec = pmove->frametime * 1000;

		PM_ReduceTimers();

		// Convert view angles to vectors
		AngleVectors (pmove->angles, pmove->forward, pmove->right, pmove->up);

		// Always try and unstick us unless we are in NOCLIP mode
		/*if ( pmove->movetype != MOVETYPE_NOCLIP && pmove->movetype != MOVETYPE_NONE )
		{
			if ( PM_CheckStuck() )
			{
				//Let the user try to duck to get unstuck
				PM_Duck();

				if ( PM_CheckStuck() )
				{
					return;  // Can't move, we're stuck
				}
			}
		}*/

		// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
		PM_CatagorizePosition();

		// Store off the starting water level
		pmove->oldwaterlevel = pmove->waterlevel;

		// If we are not on ground, store off how fast we are moving down
		if ( pmove->onground == -1 )
		{
			pmove->flFallVelocity = -pmove->velocity[UP];
		}

		// todo
		//g_onladder = 0;
		//// Don't run ladder code if dead or on a train
		//if ( !pmove->dead && !(pmove->flags & FL_ONTRAIN) )
		//{
		//	pLadder = PM_Ladder();
		//	if ( pLadder )
		//	{
		//		g_onladder = 1;
		//	}
		//}

		PM_UpdateStepSound();

		PM_Duck();

		// todo
		// Don't run ladder code if dead or on a train
		//if ( !pmove->dead && !(pmove->flags & FL_ONTRAIN) )
		//{
		//	if ( pLadder )
		//	{
		//		PM_LadderMove( pLadder );
		//	}
		//	else if ( pmove->movetype != MOVETYPE_WALK &&
		//			  pmove->movetype != MOVETYPE_NOCLIP )
		//	{
		//		// Clear ladder stuff unless player is noclipping
		//		//  it will be set immediately again next frame if necessary
		//		pmove->movetype = MOVETYPE_WALK;
		//	}
		//}

		// Handle movement
		switch ( pmove->movetype ) {
			default:
				break;

			case MOVETYPE_NONE:
				break;

			case MOVETYPE_NOCLIP:
				PM_NoClip();
				break;

			//case MOVETYPE_TOSS:
			//case MOVETYPE_BOUNCE:
			//	PM_Physics_Toss();
			//	break;

			case MOVETYPE_FLY:
				PM_CheckWater();

				// Was jump button pressed?
				// If so, set velocity to 270 away from ladder.  This is currently wrong.
				// Also, set MOVE_TYPE to walk, too.
				if ( pmove->cmd.buttons & IN_JUMP )
				{
					if ( !pLadder )
					{
						PM_Jump ();
					}
				}
				else
				{
					pmove->oldbuttons &= ~IN_JUMP;
				}

				// Perform the move accounting for any base velocity.
				VectorAdd (pmove->velocity, pmove->basevelocity, pmove->velocity);
				PM_FlyMove ();
				VectorSubtract (pmove->velocity, pmove->basevelocity, pmove->velocity);
				break;

			case MOVETYPE_WALK:
				if ( !PM_InWater() )
				{
					PM_AddCorrectGravity();
				}

				// If we are leaping out of the water, just update the counters.
				if (pmove->waterjumptime)
				{
					PM_WaterJump();
					PM_FlyMove();
					// Make sure waterlevel is set correctly
					PM_CheckWater();
					return;
				}

				// If we are swimming in the water, see if we are nudging against a place we can jump up out
				//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
				if (pmove->waterlevel >= 2)
				{
					// todo
					//if ( pmove->waterlevel == 2 )
					//{
					//	PM_CheckWaterJump();
					//}

					// If we are falling again, then we must not trying to jump out of water any more.
					if ( pmove->velocity[UP] < 0 && pmove->waterjumptime )
					{
						pmove->waterjumptime = 0;
					}

					// Was jump button pressed?
					if (pmove->cmd.buttons & IN_JUMP)
					{
						PM_Jump();
					}
					else
					{
						pmove->oldbuttons &= ~IN_JUMP;
					}

					// Perform regular water movement
					PM_WaterMove();

					VectorSubtract (pmove->velocity, pmove->basevelocity, pmove->velocity);

					// Get a final position
					PM_CatagorizePosition();
				}
				else
				// Not underwater
				{
					// Was jump button pressed?
					if ( pmove->cmd.buttons & IN_JUMP )
					{
						if ( !pLadder )
						{
							PM_Jump();
						}
					}
					else
					{
						pmove->oldbuttons &= ~IN_JUMP;
					}

					// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
					//  we don't slow when standing still, relative to the conveyor.
					if ( pmove->onground != -1 )
					{
						pmove->velocity[UP] = 0.0;
						PM_Friction();
					}

					// Make sure velocity is valid.
					PM_CheckVelocity();

					// Are we on ground now
					if ( pmove->onground != -1 )
					{
						PM_WalkMove();
					}
					else
					{
						PM_AirMove();  // Take into account movement when in air.
					}

					// Set final flags.
					PM_CatagorizePosition();

					// Now pull the base velocity back out.
					// Base velocity is set if you are on a moving object, like
					//  a conveyor (or maybe another monster?)
					VectorSubtract (pmove->velocity, pmove->basevelocity, pmove->velocity );

					// Make sure velocity is valid.
					PM_CheckVelocity();

					// Add any remaining gravitational component.
					if ( !PM_InWater() )
					{
						PM_FixupGravityVelocity();
					}

					// If we are on ground, no downward velocity.
					if ( pmove->onground != -1 )
					{
						pmove->velocity[UP] = 0;
					}

					// See if we landed on the ground with enough force to play
					//  a landing sound.
					PM_CheckFalling();
				}

				// Did we enter or leave the water?
				PM_PlayWaterSounds();
				break;
		}
	}

	void PM_Move() {
		gTimer.Process();

		PM_PlayerMove();

		if (pmove->onground != -1)
		{
			pmove->flags |= FL_ONGROUND;
		}
		else
		{
			pmove->flags &= ~FL_ONGROUND;
		}
	}

	void SetupMoveParams() {
		movevars->gravity = sv_gravity;  			// Gravity for map
		movevars->stopspeed = sv_stopspeed;			// Deceleration when not moving
		movevars->maxspeed = sv_maxspeed; 			// Max allowed speed
		movevars->accelerate = sv_accelerate;			// Acceleration factor
		movevars->airaccelerate = sv_airaccelerate;		// Same for when in open air
		movevars->wateraccelerate = sv_wateraccelerate;		// Same for when in water
		movevars->friction = sv_friction;
		movevars->edgefriction = sv_edgefriction;
		movevars->waterfriction = sv_waterfriction;		// Less in water
		movevars->bounce = sv_bounce;      		// Wall bounce value. 1.0
		movevars->stepsize = sv_stepsize;    		// sv_stepsize;
		movevars->maxvelocity = sv_maxvelocity; 		// maximum server velocity.
		movevars->rollangle = sv_rollangle;
		movevars->rollspeed = sv_rollspeed;

		// todo train velocity
		pmove->basevelocity = {0,0,0};

		pmove->gravity = 1;
		pmove->friction = 1;
		pmove->cmd.viewangles.x = FO2Cam::vAngle.x / (std::numbers::pi / 180.0);
		pmove->cmd.viewangles.y = FO2Cam::vAngle.y / (std::numbers::pi / 180.0);
		pmove->cmd.viewangles.z = FO2Cam::vAngle.z / (std::numbers::pi / 180.0);
		pmove->clientmaxspeed = movevars->maxspeed;
		if (pmove->movetype == MOVETYPE_NOCLIP) pmove->clientmaxspeed = sv_noclipspeed;
		pmove->maxspeed = pmove->clientmaxspeed; // not sure what the difference is here? todo?

		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.buttons = 0;

		if (IsKeyPressed('A')) pmove->cmd.sidemove -= cl_sidespeed;
		if (IsKeyPressed('D')) pmove->cmd.sidemove += cl_sidespeed;
		if (IsKeyPressed('W')) pmove->cmd.forwardmove += cl_forwardspeed;
		if (IsKeyPressed('S')) pmove->cmd.forwardmove -= cl_forwardspeed;
		if (IsKeyPressed(VK_SPACE)) pmove->cmd.buttons |= IN_JUMP;
		if (IsKeyPressed(VK_LCONTROL)) pmove->cmd.buttons |= IN_DUCK;
		if (IsKeyPressed(VK_LSHIFT)) {
			pmove->cmd.buttons |= IN_RUN;
			pmove->cmd.forwardmove *= cl_movespeedkey;
			pmove->cmd.sidemove *= cl_movespeedkey;
		}
	}

	void ApplyMoveParams() {
		auto pos = pmove->origin + pmove->view_ofs;
		pos[UP] += V_CalcBob();
		FO2Cam::vPos.x = UnitsToMeters(pos.x);
		FO2Cam::vPos.y = UnitsToMeters(pos.y);
		FO2Cam::vPos.z = UnitsToMeters(pos.z);
		FO2Cam::vAngleView.x = pmove->angles.x * (std::numbers::pi / 180.0);
		FO2Cam::vAngleView.y = pmove->angles.y * (std::numbers::pi / 180.0);
		FO2Cam::vAngleView.z = pmove->angles.z * (std::numbers::pi / 180.0);
	}

	void Reset() {
		pmove->origin = {0,0,0};
		if (auto ply = GetPlayer(0)) {
			pmove->origin.x = MetersToUnits(ply->pCar->GetMatrix()->p.x);
			pmove->origin.y = MetersToUnits(ply->pCar->GetMatrix()->p.y + 1);
			pmove->origin.z = MetersToUnits(ply->pCar->GetMatrix()->p.z);
		}
		pmove->velocity = {0,0,0};
		pmove->onground = -1;
		pmove->movetype = MOVETYPE_WALK;
		pmove->usehull = 0;
		pmove->view_ofs[0] = 0;
		pmove->view_ofs[FORWARD] = 0;
		pmove->view_ofs[UP] = VEC_VIEW;
		for (int i = 0; i < 4; i++) {
			pmove->player_mins[i][0] = pm_hullmins[i][0];
			pmove->player_mins[i][FORWARD] = pm_hullmins[i][1];
			pmove->player_mins[i][UP] = pm_hullmins[i][2];
			pmove->player_maxs[i][0] = pm_hullmaxs[i][0];
			pmove->player_maxs[i][FORWARD] = pm_hullmaxs[i][1];
			pmove->player_maxs[i][UP] = pm_hullmaxs[i][2];
		}
	}

	void ToggleNoclip() {
		pmove->movetype = pmove->movetype == MOVETYPE_NOCLIP ? MOVETYPE_WALK : MOVETYPE_NOCLIP;
	}

	void Process() {
		SetupMoveParams();
		PM_Move();
		ApplyMoveParams();
	}

	void ProcessMenu() {
		if (DrawMenuOption(std::format("Active - {}", bEnabled), "")) {
			bEnabled = !bEnabled;
			FO2Cam::nLastGameState = -1;
		}

		if (DrawMenuOption(std::format("Field of View - {}", FO2Cam::fFOV), "")) {
			ValueEditorMenu(FO2Cam::fFOV);
		}

		if (DrawMenuOption(std::format("Sensitivity - {}", FO2Cam::fMouseRotateSpeed), "")) {
			ValueEditorMenu(FO2Cam::fMouseRotateSpeed);
		}

		ValueEditorMenu(bAutoBhop, "Auto Bhop");
		ValueEditorMenu(bABH, "ABH");
		ValueEditorMenu(bABHMixed, "Mixed ABH");
		ValueEditorMenu(bCanLongJump, "Long Jump Module");
		ValueEditorMenu(nColDensity, "Collision Density");

		if (DrawMenuOption("Parameters", "")) {
			ChloeMenuLib::BeginMenu();

			ValueEditorMenu(sv_gravity, "sv_gravity");
			ValueEditorMenu(sv_stopspeed, "sv_stopspeed");
			ValueEditorMenu(sv_maxspeed, "sv_maxspeed");
			ValueEditorMenu(sv_noclipspeed, "sv_noclipspeed");
			ValueEditorMenu(sv_accelerate, "sv_accelerate");
			ValueEditorMenu(sv_airaccelerate, "sv_airaccelerate");
			ValueEditorMenu(sv_wateraccelerate, "sv_wateraccelerate");
			ValueEditorMenu(sv_friction, "sv_friction");
			ValueEditorMenu(sv_edgefriction, "sv_edgefriction");
			ValueEditorMenu(sv_waterfriction, "sv_waterfriction");
			//ValueEditorMenu(sv_entgravity, "sv_entgravity");
			ValueEditorMenu(sv_bounce, "sv_bounce");
			ValueEditorMenu(sv_stepsize, "sv_stepsize");
			ValueEditorMenu(sv_maxvelocity, "sv_maxvelocity");
			//ValueEditorMenu(mp_footsteps, "mp_footsteps");
			ValueEditorMenu(sv_rollangle, "sv_rollangle");
			ValueEditorMenu(sv_rollspeed, "sv_rollspeed");
			ValueEditorMenu(cl_forwardspeed, "cl_forwardspeed");
			ValueEditorMenu(cl_sidespeed, "cl_sidespeed");
			ValueEditorMenu(cl_upspeed, "cl_upspeed");
			ValueEditorMenu(cl_movespeedkey, "cl_movespeedkey");
			ValueEditorMenu(cl_bob, "cl_bob");
			ValueEditorMenu(cl_bobcycle, "cl_bobcycle");
			ValueEditorMenu(cl_bobup, "cl_bobup");
			if (DrawMenuOption(std::format("sv_gravity - {}", sv_gravity), "")) {
				ValueEditorMenu(sv_gravity);
			}

			ChloeMenuLib::EndMenu();
		}

		DrawMenuOption(std::format("Velocity - {:.2f} {:.2f} {:.2f}", pmove->velocity[0], pmove->velocity[1], pmove->velocity[2]), "", true);
		DrawMenuOption(std::format("Punch Angle - {:.2f} {:.2f} {:.2f}", pmove->punchangle[0], pmove->punchangle[1], pmove->punchangle[2]), "", true);
		DrawMenuOption(std::format("View Angle - {:.2f} {:.2f} {:.2f}", pmove->angles[0], pmove->angles[1], pmove->angles[2]), "", true);
		DrawMenuOption(std::format("Last Plane Normal - {:.2f}", fLastPlaneNormal), "", true);
		DrawMenuOption(std::format("On Ground - {}", pmove->onground), "", true);
		DrawMenuOption(std::format("{}", lastConsoleMsg), "", true);
	}
}