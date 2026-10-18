class SRZ_WhirlSpinEntry
{
	ChimeraCharacter m_Character;
	vector m_vCentre;
	float m_fHeight;
	float m_fRadius;
	float m_fSpeed;
	float m_fGain;
	float m_fDir;
	float m_fStart;
	float m_fUntil;
	float m_fNextShake;
	float m_fNextLog;
	float m_fNextPush;
	vector m_vLastBody;
	float m_fLastBodyTime = -1;
}

class SRZ_WhirlFlingEntry
{
	ChimeraCharacter m_Character;
	vector m_vVelocity;
	int m_iTries;
}

class SRZ_WhirlBody
{
	protected static const float LIFT_S = 1.0;
	protected static const float RADIAL_GAIN = 2.5;
	protected static const float HEIGHT_GAIN = 3.0;
	protected static const float SHAKE_EVERY_S = 0.45;
	protected static const float PUSH_EVERY_S = 0.1;
	protected static const float PUSH_LIFE_S = 0.15;
	protected static const float PUSH_RADIUS = 2.0;
	protected static const float PUSH_FORCE_SCALE = 6.0;
	protected static const float PUSH_FORCE_MAX = 400;
	protected static const float FLING_FORCE_PER_MPS = 20;
	protected static const float FLING_LIFE_S = 0.25;
	protected static const int FLING_TRIES = 20;
	static const bool DEBUG = false;

	protected static ref array<ref SRZ_WhirlSpinEntry> s_aSpins = {};
	protected static ref array<ref SRZ_WhirlFlingEntry> s_aFlings = {};
	protected static float s_fLastTick = -1;
	protected static bool s_bRunning;

	protected static ref array<string> s_aBodyBones = {"Spine3", "Spine2", "Spine1", "Hips", "Pelvis", "Head"};

	static void Spin(ChimeraCharacter character, vector centre, float height, float radius, float speed, float gain, float seconds, float dir)
	{
		if (!character)
			return;

		Stop(character);
		SRZ_WhirlSpinEntry entry = new SRZ_WhirlSpinEntry();
		entry.m_Character = character;
		entry.m_vCentre = centre;
		entry.m_fHeight = height;
		entry.m_fRadius = radius;
		entry.m_fSpeed = speed;
		entry.m_fGain = gain;
		entry.m_fDir = dir;
		entry.m_fStart = Now();
		entry.m_fUntil = entry.m_fStart + seconds;
		s_aSpins.Insert(entry);
		if (DEBUG)
			Print(string.Format("[SRZ Whirl] spin start on %1 for %2 s, server %3", character, seconds, Replication.IsServer()), LogLevel.NORMAL);

		EnsureRunning();
	}

	static void Fling(ChimeraCharacter character, vector velocity)
	{
		if (!character)
			return;

		Stop(character);
		SRZ_WhirlFlingEntry entry = new SRZ_WhirlFlingEntry();
		entry.m_Character = character;
		entry.m_vVelocity = velocity;
		entry.m_iTries = FLING_TRIES;
		s_aFlings.Insert(entry);
		EnsureRunning();
	}

	static void Stop(ChimeraCharacter character)
	{
		for (int i = s_aSpins.Count() - 1; i >= 0; i--)
		{
			if (s_aSpins[i].m_Character == character)
				s_aSpins.Remove(i);
		}
	}

	static vector BodyPosition(IEntity character)
	{
		if (!character)
			return vector.Zero;

		Animation anim = character.GetAnimation();
		if (anim)
		{
			foreach (string bone : s_aBodyBones)
			{
				TNodeId id = anim.GetBoneIndex(bone);
				if (id < 0)
					continue;

				vector mat[4];
				anim.GetBoneMatrix(id, mat);
				return character.CoordToParent(mat[3]);
			}
		}

		return character.GetOrigin() + Vector(0, 1, 0);
	}

	protected static float Now()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected static void EnsureRunning()
	{
		if (s_bRunning)
			return;

		s_bRunning = true;
		s_fLastTick = -1;
		GetGame().GetCallqueue().CallLater(Tick, 0, true);
	}

	protected static CharacterAnimationComponent RagdollAnim(ChimeraCharacter character)
	{
		CharacterAnimationComponent anim = CharacterAnimationComponent.Cast(character.FindComponent(CharacterAnimationComponent));
		if (!anim || !anim.IsRagdollActive())
			return null;

		return anim;
	}

	protected static void Push(ChimeraCharacter character, CharacterAnimationComponent anim, vector worldDir, float force, float life)
	{
		if (force <= 0 || worldDir.LengthSq() < 0.0001)
			return;

		vector localDir = character.VectorToLocal(worldDir.Normalized());
		anim.AddRagdollEffectorDamage(Vector(0, 1, 0), localDir, force, PUSH_RADIUS, life);
	}

	protected static void Tick()
	{
		float now = Now();
		float dt = 0;
		if (s_fLastTick >= 0)
			dt = Math.Clamp(now - s_fLastTick, 0, 0.1);

		s_fLastTick = now;
		IEntity local = SCR_PlayerController.GetLocalControlledEntity();

		for (int i = s_aSpins.Count() - 1; i >= 0; i--)
		{
			SRZ_WhirlSpinEntry e = s_aSpins[i];
			if (!e.m_Character || e.m_Character.IsDeleted() || now > e.m_fUntil)
			{
				s_aSpins.Remove(i);
				continue;
			}

			CharacterAnimationComponent anim = RagdollAnim(e.m_Character);
			vector body = BodyPosition(e.m_Character);
			vector vel = vector.Zero;
			if (e.m_fLastBodyTime >= 0 && now - e.m_fLastBodyTime > 0.001)
				vel = (body - e.m_vLastBody) * (1 / (now - e.m_fLastBodyTime));

			if (now < e.m_fNextPush)
				continue;

			e.m_fNextPush = now + PUSH_EVERY_S;
			e.m_vLastBody = body;
			e.m_fLastBodyTime = now;
			if (!anim)
				continue;

			float t = Math.Clamp((now - e.m_fStart) / LIFT_S, 0, 1);
			float lift = t * t * (3 - 2 * t);
			vector centre = e.m_vCentre + Vector(0, e.m_fHeight * lift, 0);
			vector flat = body - centre;
			flat[1] = 0;
			float dist = flat.Length();
			vector away = Vector(1, 0, 0);
			if (dist > 0.05)
				away = flat * (1 / dist);

			vector tangent = Vector(-away[2], 0, away[0]) * e.m_fDir;
			vector want = tangent * (e.m_fSpeed * lift) + away * ((e.m_fRadius - dist) * RADIAL_GAIN) + Vector(0, (centre[1] - body[1]) * HEIGHT_GAIN, 0);
			vector error = want - vel;
			float force = Math.Clamp(error.Length() * e.m_fGain * PUSH_FORCE_SCALE, 0, PUSH_FORCE_MAX);
			Push(e.m_Character, anim, error, force, PUSH_LIFE_S);

			if (DEBUG && now >= e.m_fNextLog)
			{
				e.m_fNextLog = now + 1;
				Print(string.Format("[SRZ Whirl] tick: body %1, height above centre %2, speed %3, force %4", body, body[1] - e.m_vCentre[1], vel.Length(), force), LogLevel.NORMAL);
			}

			if (e.m_Character == local && now >= e.m_fNextShake)
			{
				e.m_fNextShake = now + SHAKE_EVERY_S;
				SCR_CameraShakeManagerComponent.AddCameraShake(0.4, 1.2, 0.05, 0.2, 0.4);
			}
		}

		for (int j = s_aFlings.Count() - 1; j >= 0; j--)
		{
			SRZ_WhirlFlingEntry f = s_aFlings[j];
			if (!f.m_Character || f.m_Character.IsDeleted() || f.m_iTries <= 0)
			{
				s_aFlings.Remove(j);
				continue;
			}

			f.m_iTries--;
			CharacterAnimationComponent flungAnim = RagdollAnim(f.m_Character);
			if (DEBUG)
				Print(string.Format("[SRZ Whirl] fling try, ragdoll active %1, tries left %2", flungAnim != null, f.m_iTries), LogLevel.NORMAL);

			if (!flungAnim)
				continue;

			Push(f.m_Character, flungAnim, f.m_vVelocity, f.m_vVelocity.Length() * FLING_FORCE_PER_MPS, FLING_LIFE_S);
			s_aFlings.Remove(j);
		}

		if (!s_aSpins.IsEmpty() || !s_aFlings.IsEmpty())
			return;

		GetGame().GetCallqueue().Remove(Tick);
		s_bRunning = false;
	}
}
