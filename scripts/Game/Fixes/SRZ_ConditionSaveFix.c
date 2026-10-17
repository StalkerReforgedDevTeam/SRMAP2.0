// EPF only saves the components listed in each prefab's persistence config. Many prefabs (mod guns,
// custom gear) don't list the ARMST condition / artifact data, so it resets on relog or restart.
// This adds those entries at runtime for any item that has the ARMST components.

modded class EPF_EntitySaveData
{
	override EPF_EReadResult ReadFrom(IEntity entity, EPF_EntitySaveDataClass attributes)
	{
		SRZ_EnsureArmstSaveData(entity, attributes);
		return super.ReadFrom(entity, attributes);
	}

	override EPF_EApplyResult ApplyTo(IEntity entity, EPF_EntitySaveDataClass attributes)
	{
		SRZ_EnsureArmstSaveData(entity, attributes);
		return super.ApplyTo(entity, attributes);
	}

	protected static void SRZ_EnsureArmstSaveData(IEntity entity, EPF_EntitySaveDataClass attributes)
	{
		if (!entity || !attributes || ChimeraCharacter.Cast(entity))
			return;

		bool hasStats = entity.FindComponent(ARMST_ITEMS_STATS_COMPONENTS) != null;
		bool hasArt = entity.FindComponent(ARMST_ARTEFACT_PROPERTIES) != null;
		if (!hasStats && !hasArt)
			return;

		if (!attributes.m_aComponents)
			attributes.m_aComponents = {};

		bool statsListed, artListed;
		foreach (EPF_ComponentSaveDataClass entry : attributes.m_aComponents)
		{
			if (EPF_ARMST_ITEMS_STATS_COMPONENTS_SaveDataClass.Cast(entry))
				statsListed = true;
			else if (EPF_ArmstArtefactPropertiesComponentSaveDataClass.Cast(entry))
				artListed = true;
		}

		if (hasStats && !statsListed)
		{
			EPF_ARMST_ITEMS_STATS_COMPONENTS_SaveDataClass statsData = new EPF_ARMST_ITEMS_STATS_COMPONENTS_SaveDataClass();
			statsData.m_bEnabled = true;
			statsData.m_bTrimDefaults = attributes.m_bTrimDefaults;
			attributes.m_aComponents.Insert(statsData);
		}

		if (hasArt && !artListed)
		{
			EPF_ArmstArtefactPropertiesComponentSaveDataClass artData = new EPF_ArmstArtefactPropertiesComponentSaveDataClass();
			artData.m_bEnabled = true;
			artData.m_bTrimDefaults = attributes.m_bTrimDefaults;
			attributes.m_aComponents.Insert(artData);
		}
	}
}
