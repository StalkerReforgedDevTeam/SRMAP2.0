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
		ScheduleZoneTick(ent);
	}

	override void OnDeactivate(IEntity ent)
	{
		super.OnDeactivate(ent);

		if (!ent)
			return;

		m_mActiveOccupants.Remove(ent);
	}

	protected void ScheduleZoneTick(IEntity ent)
	{
		if (!ent)
			return;

		if (!m_mActiveOccupants.Contains(ent))
			return;

		ApplyZoneTick(ent);
		GetGame().GetCallqueue().CallLater(ScheduleZoneTick, m_fZoneTickInterval * 1000, false, ent);
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

		if (statsComponent.GetFactionKey() == ARMST_FACTION_LABEL.FACTION_MUTANTS)
		{
			statsComponent.ArmstRadiactiveLevelSet(0);
			return;
		}

		float protection = statComponent.GetAllRadiactive(ent);

		statsComponent.ArmstRadiactiveLevelSet(m_fZoneRadiationLevel);

		float deficit = m_fZoneRadiationLevel - protection;
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