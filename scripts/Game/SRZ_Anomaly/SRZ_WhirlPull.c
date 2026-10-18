class SRZ_WhirlPullEntry
{
	ChimeraCharacter m_Character;
	vector m_vCentre;
	float m_fSpeed;
	float m_fUntil;
	bool m_bLocked;
}

class SRZ_WhirlPull
{
	protected static const float STOP_M = 0.4;

	protected static ref array<ref SRZ_WhirlPullEntry> s_aPulls = {};
	protected static bool s_bRunning;

	static void Begin(ChimeraCharacter character, vector centre, float seconds, float speed)
	{
		if (!character || character.IsInVehicle())
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller || controller.GetLifeState() != ECharacterLifeState.ALIVE)
			return;

		End(character);
		SRZ_WhirlPullEntry entry = new SRZ_WhirlPullEntry();
		entry.m_Character = character;
		entry.m_vCentre = centre;
		entry.m_fSpeed = speed;
		entry.m_fUntil = Now() + seconds;
		if (!controller.GetDisableMovementControls())
		{
			controller.SetDisableMovementControls(true);
			entry.m_bLocked = true;
		}

		s_aPulls.Insert(entry);
		if (s_bRunning)
			return;

		s_bRunning = true;
		GetGame().GetCallqueue().CallLater(Tick, 0, true);
	}

	static void End(ChimeraCharacter character)
	{
		for (int i = s_aPulls.Count() - 1; i >= 0; i--)
		{
			if (s_aPulls[i].m_Character != character)
				continue;

			Release(s_aPulls[i]);
			s_aPulls.Remove(i);
		}
	}

	protected static float Now()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected static void Release(SRZ_WhirlPullEntry entry)
	{
		if (!entry.m_Character)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(entry.m_Character.GetCharacterController());
		if (!controller)
			return;

		controller.SetMovement(0, vector.Zero);
		controller.SetMovementDirWorld(vector.Zero);
		if (entry.m_bLocked)
			controller.SetDisableMovementControls(false);
	}

	protected static void Tick()
	{
		float now = Now();
		for (int i = s_aPulls.Count() - 1; i >= 0; i--)
		{
			SRZ_WhirlPullEntry e = s_aPulls[i];
			SCR_CharacterControllerComponent controller;
			if (e.m_Character && !e.m_Character.IsDeleted())
				controller = SCR_CharacterControllerComponent.Cast(e.m_Character.GetCharacterController());

			bool done = !controller || now > e.m_fUntil || controller.GetLifeState() != ECharacterLifeState.ALIVE;
			if (!done)
			{
				CharacterAnimationComponent anim = CharacterAnimationComponent.Cast(e.m_Character.FindComponent(CharacterAnimationComponent));
				done = anim && anim.IsRagdollActive();
			}

			if (done)
			{
				Release(e);
				s_aPulls.Remove(i);
				continue;
			}

			vector flat = e.m_vCentre - e.m_Character.GetOrigin();
			flat[1] = 0;
			float dist = flat.Length();
			if (dist <= STOP_M)
			{
				controller.SetMovement(0, vector.Zero);
				continue;
			}

			controller.SetMovementDirWorld(flat * (1 / dist));
			controller.SetMovement(e.m_fSpeed, vector.Forward);
		}

		if (!s_aPulls.IsEmpty())
			return;

		GetGame().GetCallqueue().Remove(Tick);
		s_bRunning = false;
	}
}
