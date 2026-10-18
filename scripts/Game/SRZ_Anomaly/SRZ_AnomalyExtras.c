modded class ARMST_DamagingTriggerEntity
{
	[Attribute("1", UIWidgets.Slider, "Speed cap while inside, share of full speed (1 = no slowdown). Keep Update Period at 1 or lower", "0.1 1 0.05", category: "SRZ")]
	float m_fSRZ_SlowLimit;

	[Attribute("0", UIWidgets.CheckBox, "Only attacks people who sprint through it, walking is safe. Keep Update Period at 0.5 or lower", category: "SRZ")]
	bool m_bSRZ_SprintOnly;

	[Attribute("4.8", UIWidgets.Slider, "Ground speed in m/s that counts as sprinting", "2 8 0.1", category: "SRZ")]
	float m_fSRZ_SprintSpeed;

	[Attribute("0", UIWidgets.Slider, "Bleeding wounds opened on each hit", "0 4 1", category: "SRZ")]
	int m_iSRZ_BleedWounds;

	protected static const float SRZ_SLOW_HOLD_S = 1.5;

	override void OnActivate(IEntity ent)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (character && Replication.IsServer() && m_fSRZ_SlowLimit < 0.999 && IsAlive(ent))
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controller)
				controller.SRZ_AskSlow(m_fSRZ_SlowLimit, SRZ_SLOW_HOLD_S);
		}

		if (character && m_bSRZ_SprintOnly && !SRZ_IsHurrying(character))
			return;

		super.OnActivate(ent);
	}

	override void ArmstDamageStart(IEntity ent)
	{
		super.ArmstDamageStart(ent);
		if (m_iSRZ_BleedWounds > 0 && Replication.IsServer())
			SRZ_OpenWounds(ent);
	}

	protected bool SRZ_IsHurrying(ChimeraCharacter character)
	{
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;

		if (controller.IsSprinting())
			return true;

		return controller.GetMovementSpeed() > m_fSRZ_SprintSpeed;
	}

	protected void SRZ_OpenWounds(IEntity ent)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageManager || damageManager.GetState() == EDamageState.DESTROYED)
			return;

		array<HitZone> zones = {};
		damageManager.GetPhysicalHitZones(zones);
		if (zones.IsEmpty())
			return;

		for (int i = 0; i < m_iSRZ_BleedWounds; i++)
		{
			SCR_CharacterHitZone zone = SCR_CharacterHitZone.Cast(zones.GetRandomElement());
			if (zone)
				damageManager.AddBleedingEffectOnHitZone(zone);
		}
	}
}

class SRZ_SlowKey
{
}

class SRZ_SlowEntry
{
	SCR_ChimeraCharacter m_Character;
	float m_fLimit;
	float m_fUntil;
}

class SRZ_SlowHold
{
	protected static const int TICK_MS = 250;

	protected static ref SRZ_SlowKey s_Key = new SRZ_SlowKey();
	protected static ref array<ref SRZ_SlowEntry> s_aEntries = {};
	protected static bool s_bRunning;

	static void Hold(IEntity character, float limit, float seconds)
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(character);
		if (!chimera)
			return;

		float until = Now() + seconds;
		SRZ_SlowEntry entry;
		foreach (SRZ_SlowEntry e : s_aEntries)
		{
			if (e.m_Character == chimera)
			{
				entry = e;
				break;
			}
		}

		if (!entry)
		{
			entry = new SRZ_SlowEntry();
			entry.m_Character = chimera;
			s_aEntries.Insert(entry);
		}

		entry.m_fLimit = Math.Clamp(limit, 0.05, 1);
		entry.m_fUntil = until;
		chimera.SetSpeedLimit(s_Key, entry.m_fLimit, false);

		if (s_bRunning)
			return;

		s_bRunning = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK_MS, true);
	}

	protected static float Now()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected static void Tick()
	{
		float now = Now();
		for (int i = s_aEntries.Count() - 1; i >= 0; i--)
		{
			SRZ_SlowEntry e = s_aEntries[i];
			if (!e.m_Character || e.m_Character.IsDeleted())
			{
				s_aEntries.Remove(i);
				continue;
			}

			if (now < e.m_fUntil)
				continue;

			e.m_Character.SetSpeedLimit(s_Key, 1, false);
			s_aEntries.Remove(i);
		}

		if (!s_aEntries.IsEmpty())
			return;

		GetGame().GetCallqueue().Remove(Tick);
		s_bRunning = false;
	}
}

modded class SCR_CharacterControllerComponent
{
	void SRZ_AskSlow(float limit, float seconds)
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;

		SRZ_SlowHold.Hold(character, limit, seconds);
		RplComponent rpl = character.GetRplComponent();
		if (rpl && !rpl.IsOwner())
			Rpc(RpcDo_SRZ_Slow, limit, seconds);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZ_Slow(float limit, float seconds)
	{
		SRZ_SlowHold.Hold(GetCharacter(), limit, seconds);
	}
}
