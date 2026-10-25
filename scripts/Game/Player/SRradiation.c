enum ARMST_FilterDrainTier
{
	LOW,
	MEDIUM,
	HIGH
}

modded class ARMST_RADIATIONSEntity
{
	[Attribute("100", UIWidgets.Slider, "Flat radiation level for this zone (no distance falloff)", "0 1000 1", category: "RADIATION ZONE")]
	protected float m_fZoneRadiationLevel;

	[Attribute("0.05", UIWidgets.EditBox, "Radiation stat gained per tick, per point of unprotected deficit (zone level minus player protection)", category: "RADIATION ZONE")]
	protected float m_fDeficitDoseScale;

	[Attribute("5.0", UIWidgets.EditBox, "Seconds between radiation/mask-wear ticks per player, independent of how often the engine calls OnActivate", category: "RADIATION ZONE")]
	protected float m_fZoneTickInterval;

	[Attribute("1", UIWidgets.ComboBox, "Gas mask condition drain tier for this zone", "", ParamEnumArray.FromEnum(ARMST_FilterDrainTier), category: "GAS MASK WEAR")]
	protected ARMST_FilterDrainTier m_eFilterDrainTier;

	[Attribute("0.0463", UIWidgets.EditBox, "Mask condition lost per tick - LOW tier (full mask lasts ~3 hours at the default 5s tick interval)", category: "GAS MASK WEAR")]
	protected float m_fFilterDrainLow;

	[Attribute("0.0926", UIWidgets.EditBox, "Mask condition lost per tick - MEDIUM tier (full mask lasts ~1.5 hours at the default 5s tick interval)", category: "GAS MASK WEAR")]
	protected float m_fFilterDrainMedium;

	[Attribute("0.2778", UIWidgets.EditBox, "Mask condition lost per tick - HIGH tier (full mask lasts ~30 minutes at the default 5s tick interval)", category: "GAS MASK WEAR")]
	protected float m_fFilterDrainHigh;

	protected ref map<IEntity, bool> m_mActiveOccupants = new map<IEntity, bool>();

	// How many zones each player is standing in, so leaving one overlapping zone doesn't zero the dosimeter
	protected static ref map<IEntity, int> s_mSRZ_ZoneCount = new map<IEntity, int>();
	protected ref set<IEntity> m_aTickPending = new set<IEntity>();

	override void OnActivate(IEntity ent)
	{
		if (!ent)
			return;

		if (!Replication.IsServer())
			return;

		if (!IsAlive(ent))
			return;

		SCR_ChimeraCharacter owner2 = SCR_ChimeraCharacter.Cast(ent);
		if (!owner2)
			return;

		CharacterControllerComponent contr = owner2.GetCharacterController();
		if (!contr)
			return;

		if (contr.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		if (!EntityUtils.IsPlayer(ent))
			return;

		if (m_mActiveOccupants.Contains(ent))
			return;

		m_mActiveOccupants.Insert(ent, true);
		s_mSRZ_ZoneCount.Set(ent, s_mSRZ_ZoneCount.Get(ent) + 1);

		if (m_aTickPending.Contains(ent))
			return;

		ApplyZoneTick(ent);
		QueueZoneTick(ent);
	}

	override void OnDeactivate(IEntity ent)
	{
		if (!ent)
			return;

		if (m_mActiveOccupants.Contains(ent))
		{
			m_mActiveOccupants.Remove(ent);
			int count = s_mSRZ_ZoneCount.Get(ent) - 1;
			if (count > 0)
			{
				s_mSRZ_ZoneCount.Set(ent, count);
				return;
			}

			s_mSRZ_ZoneCount.Remove(ent);
		}

		super.OnDeactivate(ent);
	}

	protected void QueueZoneTick(IEntity ent)
	{
		m_aTickPending.Insert(ent);
		GetGame().GetCallqueue().CallLater(RunZoneTick, m_fZoneTickInterval * 1000, false, ent);
	}

	protected void RunZoneTick(IEntity ent)
	{
		m_aTickPending.RemoveItem(ent);

		if (!ent)
			return;

		if (!m_mActiveOccupants.Contains(ent))
			return;

		if (!IsAlive(ent))
		{
			m_mActiveOccupants.Remove(ent);
			s_mSRZ_ZoneCount.Remove(ent);
			return;
		}

		ApplyZoneTick(ent);
		QueueZoneTick(ent);
	}

	protected void ApplyZoneTick(IEntity ent)
	{
		if (!IsAlive(ent))
			return;

		ARMST_ITEMS_STATS_COMPONENTS statComponent = ARMST_ITEMS_STATS_COMPONENTS.Cast(ent.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!statComponent)
			return;

		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;

		// The dosimeter reads the zone itself. It compares that against the player's protection on its own.
		statsComponent.ArmstRadiactiveLevelSet(m_fZoneRadiationLevel);

		if (statsComponent.GetFactionKey() == ARMST_FACTION_LABEL.FACTION_MUTANTS)
			return;

		float protection = statComponent.GetAllRadiactive(ent);

		float deficit = m_fZoneRadiationLevel - protection;
		if (deficit < 0)
			deficit = 0;

		if (deficit > 0)
			statsComponent.ArmstPlayerStatSetRadio(deficit * m_fDeficitDoseScale);

		if (m_fZoneRadiationLevel > 0)
			DrainGasmaskCondition(ent);
	}

	protected float GetFilterDrainAmount()
	{
		if (m_eFilterDrainTier == ARMST_FilterDrainTier.LOW)
			return m_fFilterDrainLow;

		if (m_eFilterDrainTier == ARMST_FilterDrainTier.HIGH)
			return m_fFilterDrainHigh;

		return m_fFilterDrainMedium;
	}

	protected void DrainGasmaskCondition(IEntity ent)
	{
		if (!ent)
			return;

		if (!EntityUtils.IsPlayer(ent))
			return;

		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(ent.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return;

		IEntity mask = loadout.GetClothFromArea(ARMST_FaceArea);
		if (!mask)
			return;

		ARMST_ITEMS_STATS_COMPONENTS maskStats = ARMST_ITEMS_STATS_COMPONENTS.Cast(mask.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!maskStats)
			return;

		if (maskStats.GetProtectionRadiactiveLevel() <= 0)
			return;

		maskStats.DecreaseCondition(GetFilterDrainAmount());
	}
}