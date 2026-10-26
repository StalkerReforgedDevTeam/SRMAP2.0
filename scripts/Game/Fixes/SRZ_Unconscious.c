// Every knockout ragdolls the player and lasts 30 to 60 seconds. They can't wake early, and are woken once the time is up.
// Waking tops blood up to a safe level and gives a short grace so they don't drop straight back down,
// which used to start a fresh 30 to 60 second knockout every time (stuck down forever).
modded class SCR_ChimeraCharacter
{
	void SRZ_SendRagdoll()
	{
		Rpc(RpcDo_SRZRagdoll);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SRZRagdoll()
	{
		SRZ_Unconscious.Ragdoll(this);
	}

	void SRZ_SendSelfRagdoll()
	{
		if (Replication.IsServer())
			SRZ_Unconscious.MarkSelfRagdoll(this);
		else
			Rpc(RpcAsk_SRZSelfRagdoll);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SRZSelfRagdoll()
	{
		SRZ_Unconscious.MarkSelfRagdoll(this);
	}
}

// Ragdolling on purpose (ARMST ragdoll key) shouldn't count as a knockout
modded class SCR_CharacterControllerComponent
{
	override void ARMST_Ragdoll()
	{
		super.ARMST_Ragdoll();

		IEntity controlledEnt = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEnt || Helpers.CheckSurrenderState(controlledEnt))
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controlledEnt);
		if (character)
			character.SRZ_SendSelfRagdoll();
	}
}

class SRZ_Unconscious
{
	protected static const float MIN_TIME = 30;
	protected static const float MAX_TIME = 60;
	protected static const float TICK = 1;
	protected static const float WAKE_GRACE = 20;
	protected static const float WAKE_BLOOD = 0.65;
	protected static const float SELF_RAGDOLL_TIME = 5;
	protected static const float SELF_RAGDOLL_BLOOD = 0.6;

	protected static ref map<IEntity, float> s_mWakeAt = new map<IEntity, float>();
	protected static ref map<IEntity, float> s_mGraceUntil = new map<IEntity, float>();
	protected static ref map<IEntity, float> s_mSelfRagdollUntil = new map<IEntity, float>();
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static void Start()
	{
		if (s_bTicking || !Replication.IsServer())
			return;

		s_bTicking = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	// True for a short time after waking, so the engine can't knock them straight back out
	static bool InWakeGrace(IEntity character)
	{
		if (!character || !s_mGraceUntil.Contains(character) || !GetGame().GetWorld())
			return false;

		return GetGame().GetWorld().GetWorldTime() / 1000 < s_mGraceUntil.Get(character);
	}

	//------------------------------------------------------------------------------------------------
	static void MarkSelfRagdoll(IEntity character)
	{
		if (character && GetGame().GetWorld())
			s_mSelfRagdollUntil.Set(character, GetGame().GetWorld().GetWorldTime() / 1000 + SELF_RAGDOLL_TIME);
	}

	//------------------------------------------------------------------------------------------------
	// Just threw themselves on the ground and aren't actually hurt
	static bool InSelfRagdoll(IEntity character, SCR_CharacterDamageManagerComponent damage)
	{
		if (!character || !damage || !s_mSelfRagdollUntil.Contains(character) || !GetGame().GetWorld())
			return false;

		if (GetGame().GetWorld().GetWorldTime() / 1000 >= s_mSelfRagdollUntil.Get(character))
			return false;

		HitZone blood = damage.GetBloodHitZone();
		return !blood || blood.GetHealthScaled() >= SELF_RAGDOLL_BLOOD;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(id));
			if (character)
				Check(character, now);
		}

		Clean(s_mWakeAt);
		Clean(s_mGraceUntil);
		Clean(s_mSelfRagdollUntil);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Clean(map<IEntity, float> entries)
	{
		array<IEntity> gone = {};
		foreach (IEntity ent, float t : entries)
		{
			if (!ent)
				gone.Insert(ent);
		}

		foreach (IEntity ent : gone)
		{
			entries.Remove(ent);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void Ragdoll(IEntity character)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(character);
		if (chimera && chimera.GetCharacterController())
			chimera.GetCharacterController().Ragdoll();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Check(ChimeraCharacter character, float now)
	{
		CharacterControllerComponent controller = character.GetCharacterController();
		SCR_CharacterDamageManagerComponent damage = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!controller || !damage)
			return;

		if (controller.IsDead())
		{
			s_mWakeAt.Remove(character);
			s_mGraceUntil.Remove(character);
			return;
		}

		// Self ragdoll: undo any knock from hitting the ground, no knockout timer
		if (!s_mWakeAt.Contains(character) && InSelfRagdoll(character, damage))
		{
			HitZone resilience = damage.GetResilienceHitZone();
			if (resilience)
				resilience.SetHealthScaled(1);

			if (controller.IsUnconscious())
			{
				damage.UpdateConsciousness();
				if (controller.IsUnconscious())
					controller.SetUnconscious(false);
			}

			return;
		}

		// Up early (a hit animation or anything else stood them up): the clock keeps running instead of rerolling
		if (!controller.IsUnconscious())
		{
			if (s_mWakeAt.Contains(character) && now >= s_mWakeAt.Get(character))
			{
				s_mWakeAt.Remove(character);
				s_mGraceUntil.Set(character, now + WAKE_GRACE);
			}

			return;
		}

		// Still down during the grace window: keep waking them instead of starting a new knockout
		if (InWakeGrace(character))
		{
			Wake(character, controller, damage);
			return;
		}

		if (!s_mWakeAt.Contains(character))
		{
			float duration = Math.RandomFloat(MIN_TIME, MAX_TIME);
			s_mWakeAt.Insert(character, now + duration);
			Print("[SRZ Unconscious] Knocked out for " + (int)duration + "s");

			Ragdoll(character);
			SCR_ChimeraCharacter scrChar = SCR_ChimeraCharacter.Cast(character);
			if (scrChar && scrChar != SCR_PlayerController.GetLocalControlledEntity())
				scrChar.SRZ_SendRagdoll();
		}

		// Hold them under until the time is up
		if (now < s_mWakeAt.Get(character))
		{
			HitZone resilience = damage.GetResilienceHitZone();
			if (resilience)
				resilience.SetHealth(0);

			return;
		}

		s_mWakeAt.Remove(character);
		s_mGraceUntil.Set(character, now + WAKE_GRACE);
		Print("[SRZ Unconscious] Waking up");
		Wake(character, controller, damage);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Wake(ChimeraCharacter character, CharacterControllerComponent controller, SCR_CharacterDamageManagerComponent damage)
	{
		HitZone resilience = damage.GetResilienceHitZone();
		if (resilience)
			resilience.SetHealthScaled(1);

		HitZone blood = damage.GetBloodHitZone();
		if (blood && blood.GetHealthScaled() < WAKE_BLOOD)
			blood.SetHealthScaled(WAKE_BLOOD);

		// Let the engine wake them first so it plays the normal get up, only force it if they stay down
		damage.UpdateConsciousness();
		if (controller.IsUnconscious())
			controller.SetUnconscious(false);
	}
}

modded class SCR_CharacterDamageManagerComponent
{
	override protected bool ShouldBeUnconscious()
	{
		if (SRZ_Unconscious.InWakeGrace(GetOwner()) || SRZ_Unconscious.InSelfRagdoll(GetOwner(), this))
			return false;

		return super.ShouldBeUnconscious();
	}

	// Bleeding runs at half the vanilla rate
	override float GetBleedingScale()
	{
		return super.GetBleedingScale() * 0.5;
	}
}

// Mutant hit animations (the "Falling" flinch) stood an unconscious player up, which reset the knockout every hit
modded class ARMST_PLAYER_ANIMATION
{
	protected bool SRZ_IsDown()
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(GetOwner());
		if (!chimera)
			return false;

		CharacterControllerComponent controller = chimera.GetCharacterController();
		return controller && (controller.IsUnconscious() || controller.IsDead());
	}

	override void StartAnimationsPlayerByName(string animName)
	{
		if (SRZ_IsDown())
			return;

		super.StartAnimationsPlayerByName(animName);
	}
}

modded class ARMST_PLAYER_STATS_COMPONENT
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (GetGame().InPlayMode() && !SCR_Global.IsEditMode())
			SRZ_Unconscious.Start();
	}
}
