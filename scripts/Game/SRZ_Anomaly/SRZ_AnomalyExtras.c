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

	[Attribute("0", UIWidgets.Slider, "A bolt makes it discharge and go quiet for this many seconds (0 = off)", "0 30 0.5", category: "SRZ")]
	float m_fSRZ_BoltDisarmSeconds;

	[Attribute("0", UIWidgets.Slider, "Extra condition lost by worn boots, pants and suit on each hit", "0 20 0.5", category: "SRZ")]
	float m_fSRZ_GearWear;

	[Attribute("5", UIWidgets.Slider, "Condition lost by every piece of worn gear when it hits you", "0 50 0.5", category: "SRZ")]
	float m_fSRZ_HitGearWear;

	[Attribute("3", UIWidgets.Slider, "Seconds before the same person loses gear condition to this anomaly again", "0 30 0.5", category: "SRZ")]
	float m_fSRZ_GearWearCooldown;

	protected static const float SRZ_SLOW_HOLD_S = 1.5;
	protected static ref array<typename> s_aSRZ_WearAreas = {LoadoutBootsArea, LoadoutPantsArea, LoadoutArmoredVestSlotArea};
	protected static ref array<typename> s_aSRZ_AllAreas = {LoadoutHeadCoverArea, ARMST_FaceArea, ZEL_FaceArea, ZEL_HeadArea, LoadoutJacketArea, LoadoutPantsArea, LoadoutArmoredVestSlotArea, LoadoutVestArea, LoadoutBootsArea, LoadoutHandwearSlotArea};

	protected float m_fSRZ_DisarmedUntil;
	protected ref array<IEntity> m_aSRZ_WearWho = {};
	protected ref array<float> m_aSRZ_WearNext = {};
	protected ref array<IEntity> m_aSRZ_DisarmBolts = {};
	protected ref array<IEntity> m_aSRZ_SpeedWho = {};
	protected ref array<vector> m_aSRZ_SpeedPos = {};
	protected ref array<float> m_aSRZ_SpeedTime = {};

	override void OnActivate(IEntity ent)
	{
		if (ent && m_fSRZ_BoltDisarmSeconds > 0 && m_fBoltAction && ent.FindComponent(GrenadeMoveComponent))
			SRZ_Disarm(ent);

		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (character && SRZ_ExtrasNow() < m_fSRZ_DisarmedUntil)
			return;

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

		if (!Replication.IsServer() || !EntityUtils.IsPlayer(ent))
			return;

		if (SRZ_RuinIfDead(ent))
			return;

		if (!SRZ_WearReady(ent))
			return;

		if (m_fSRZ_HitGearWear > 0)
			SRZ_WearGear(ent, s_aSRZ_AllAreas, m_fSRZ_HitGearWear);

		if (m_fSRZ_GearWear > 0)
			SRZ_WearGear(ent, s_aSRZ_WearAreas, m_fSRZ_GearWear);
	}

	protected float SRZ_ExtrasNow()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected void SRZ_Disarm(IEntity bolt)
	{
		for (int i = m_aSRZ_DisarmBolts.Count() - 1; i >= 0; i--)
		{
			if (!m_aSRZ_DisarmBolts[i] || m_aSRZ_DisarmBolts[i].IsDeleted())
				m_aSRZ_DisarmBolts.Remove(i);
		}

		if (m_aSRZ_DisarmBolts.Contains(bolt))
			return;

		m_aSRZ_DisarmBolts.Insert(bolt);
		m_fSRZ_DisarmedUntil = SRZ_ExtrasNow() + m_fSRZ_BoltDisarmSeconds;
		SpawnAttackEffects();
	}

	protected bool SRZ_WearReady(IEntity ent)
	{
		float now = SRZ_ExtrasNow();
		for (int i = m_aSRZ_WearWho.Count() - 1; i >= 0; i--)
		{
			if (!m_aSRZ_WearWho[i] || m_aSRZ_WearWho[i].IsDeleted() || now > m_aSRZ_WearNext[i] + 60)
			{
				m_aSRZ_WearWho.Remove(i);
				m_aSRZ_WearNext.Remove(i);
				continue;
			}

			if (m_aSRZ_WearWho[i] != ent)
				continue;

			if (now < m_aSRZ_WearNext[i])
				return false;

			m_aSRZ_WearNext[i] = now + m_fSRZ_GearWearCooldown;
			return true;
		}

		m_aSRZ_WearWho.Insert(ent);
		m_aSRZ_WearNext.Insert(now + m_fSRZ_GearWearCooldown);
		return true;
	}

	protected bool SRZ_RuinIfDead(IEntity ent)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));
		return damageManager && damageManager.GetState() == EDamageState.DESTROYED;
	}

	protected void SRZ_WearGear(IEntity ent, array<typename> areas, float amount)
	{
		EquipedLoadoutStorageComponent storage = EquipedLoadoutStorageComponent.Cast(ent.FindComponent(EquipedLoadoutStorageComponent));
		if (!storage)
			return;

		foreach (typename area : areas)
		{
			IEntity item = storage.GetClothFromArea(area);
			if (!item)
				continue;

			ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			if (stats)
				stats.DecreaseCondition(amount);
		}
	}

	// The server doesn't get a player's sprint state or speed reliably, so measure how far they moved since the last check
	protected bool SRZ_IsHurrying(ChimeraCharacter character)
	{
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (controller && (controller.IsSprinting() || controller.GetMovementSpeed() > m_fSRZ_SprintSpeed))
			return true;

		float now = SRZ_ExtrasNow();
		vector pos = character.GetOrigin();
		for (int i = m_aSRZ_SpeedWho.Count() - 1; i >= 0; i--)
		{
			if (!m_aSRZ_SpeedWho[i] || m_aSRZ_SpeedWho[i].IsDeleted() || now - m_aSRZ_SpeedTime[i] > 5)
			{
				m_aSRZ_SpeedWho.Remove(i);
				m_aSRZ_SpeedPos.Remove(i);
				m_aSRZ_SpeedTime.Remove(i);
				continue;
			}

			if (m_aSRZ_SpeedWho[i] != character)
				continue;

			float dt = now - m_aSRZ_SpeedTime[i];
			vector last = m_aSRZ_SpeedPos[i];
			m_aSRZ_SpeedPos[i] = pos;
			m_aSRZ_SpeedTime[i] = now;
			if (dt < 0.05)
				return false;

			return vector.DistanceXZ(pos, last) / dt > m_fSRZ_SprintSpeed;
		}

		m_aSRZ_SpeedWho.Insert(character);
		m_aSRZ_SpeedPos.Insert(pos);
		m_aSRZ_SpeedTime.Insert(now);
		return false;
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
