// Every knockout ragdolls the player and lasts 30 to 60 seconds. They can't wake early, and are woken once the time is up.
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
}

class SRZ_Unconscious
{
	protected static const float MIN_TIME = 30;
	protected static const float MAX_TIME = 60;
	protected static const float TICK = 1;

	protected static ref map<IEntity, float> s_mWakeAt = new map<IEntity, float>();
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

		array<IEntity> gone = {};
		foreach (IEntity ent, float wakeAt : s_mWakeAt)
		{
			if (!ent)
				gone.Insert(ent);
		}

		foreach (IEntity ent : gone)
		{
			s_mWakeAt.Remove(ent);
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

		if (controller.IsDead() || !controller.IsUnconscious())
		{
			s_mWakeAt.Remove(character);
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

		HitZone resilience = damage.GetResilienceHitZone();

		// Hold them under until the time is up
		if (now < s_mWakeAt.Get(character))
		{
			if (resilience)
				resilience.SetHealth(0);

			return;
		}

		s_mWakeAt.Remove(character);
		if (resilience)
			resilience.SetHealthScaled(1);

		controller.SetUnconscious(false);
		damage.UpdateConsciousness();
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
