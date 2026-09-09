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

	[Attribute("1", UIWidgets.ComboBox, "Filter drain tier for this zone", "", ParamEnumArray.FromEnum(ARMST_FilterDrainTier), category: "FILTER")]
	protected ARMST_FilterDrainTier m_eFilterDrainTier;

	[Attribute("0.25", UIWidgets.EditBox, "Filter condition lost per tick - LOW tier", category: "FILTER")]
	protected float m_fFilterDrainLow;

	[Attribute("0.5", UIWidgets.EditBox, "Filter condition lost per tick - MEDIUM tier", category: "FILTER")]
	protected float m_fFilterDrainMedium;

	[Attribute("1.0", UIWidgets.EditBox, "Filter condition lost per tick - HIGH tier", category: "FILTER")]
	protected float m_fFilterDrainHigh;

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

		ARMST_ITEMS_STATS_COMPONENTS statComponent = ARMST_ITEMS_STATS_COMPONENTS.Cast(ent.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!statComponent)
			return;

		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;

		float protection = statComponent.GetAllRadiactive(ent);

		statsComponent.ArmstRadiactiveLevelSet(m_fZoneRadiationLevel);

		float deficit = m_fZoneRadiationLevel - protection;
		if (deficit > 0)
			statsComponent.ArmstPlayerStatSetRadio(deficit * m_fDeficitDoseScale);

		DrainGasmaskFilter(ent);
	}

	protected float GetFilterDrainAmount()
	{
		if (m_eFilterDrainTier == ARMST_FilterDrainTier.LOW)
			return m_fFilterDrainLow;

		if (m_eFilterDrainTier == ARMST_FilterDrainTier.HIGH)
			return m_fFilterDrainHigh;

		return m_fFilterDrainMedium;
	}

	protected void DrainGasmaskFilter(IEntity ent)
	{
		if (!ent)
			return;

		if (!Replication.IsServer())
			return;

		if (!IsAlive(ent))
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