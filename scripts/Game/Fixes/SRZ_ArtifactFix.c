// Artifact hunting fixes: anomaly field spawning, detectors, artifact properties.

modded class ARMST_TRIGGER_SPAWN
{
	protected static const float SRZ_ARTIFACT_CHANCE = 12;
	protected bool m_bSRZ_ArtListFiltered;

	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		if (SCR_Global.IsEditMode() || !GetGame().InPlayMode() || !Replication.IsServer())
			return;

		m_SpawnChanceArtefacts = SRZ_ARTIFACT_CHANCE;
	}

	override void OnActivate(IEntity ent)
	{
		if (Replication.IsServer() && !m_bSRZ_ArtListFiltered)
			SRZ_FilterArtList();

		super.OnActivate(ent);
	}

	override void OnDeactivate(IEntity ent)
	{
		if (Replication.IsServer() && SRZ_HasPlayerInside(ent))
			return;

		super.OnDeactivate(ent);
	}

	// Drops artifacts that have no properties in the database (they spawn with nothing and can't be sold)
	protected void SRZ_FilterArtList()
	{
		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!db || !m_ArtsPrefab)
			return;

		m_bSRZ_ArtListFiltered = true;

		for (int i = m_ArtsPrefab.Count() - 1; i >= 0; i--)
		{
			string path = m_ArtsPrefab[i];
			if (path.Contains("_drop"))
				continue;

			ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(m_ArtsPrefab[i]);
			if (!dbItem || !dbItem.m_ItemArt || dbItem.m_ItemArt.IsEmpty())
				m_ArtsPrefab.Remove(i);
		}
	}

	protected bool SRZ_HasPlayerInside(IEntity leaving)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return false;

		array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		float radius = GetSphereRadius();
		vector center = GetOrigin();

		foreach (int playerId : playerIds)
		{
			IEntity player = playerManager.GetPlayerControlledEntity(playerId);
			if (!player || player == leaving)
				continue;

			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(player);
			if (!character || character.GetCharacterController().GetLifeState() == ECharacterLifeState.DEAD)
				continue;

			if (vector.Distance(player.GetOrigin(), center) <= radius)
				return true;
		}

		return false;
	}
}

modded class ARMST_DETECTOR_ARTS_COMPONENTS
{
	protected static const float SRZ_MIN_REVEAL_DISTANCE = 3;
	protected static const float SRZ_MIN_SEARCH_DISTANCE = 10;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_fDistance_Founded = Math.Max(m_fDistance_Founded, SRZ_MIN_REVEAL_DISTANCE);
		m_fDistance_Find_Art = Math.Max(m_fDistance_Find_Art, SRZ_MIN_SEARCH_DISTANCE);
	}

	// Any frequency, and ignore artifacts sitting in someone's inventory
	override protected bool ArmstArtDetectorObjectListDistance(IEntity ent)
	{
		if (!ent || ent.GetParent())
			return true;

		ARMST_ARTEFACT_COMPONENTS artefact = ARMST_ARTEFACT_COMPONENTS.Cast(ent.FindComponent(ARMST_ARTEFACT_COMPONENTS));
		if (!artefact)
			return true;

		if (m_DetectorType == ARMST_DETECTOR.DETECTOR_OLIMP)
			StartBlinking(vector.Distance(GetOwner().GetOrigin(), ent.GetOrigin()));

		ArmstArtFoundScanner(ent, artefact);
		return false;
	}
}

modded class ARMST_ARTEFACT_PROPERTIES
{
	protected bool m_bSRZ_Generated;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !GetGame().InPlayMode() || !Replication.IsServer())
			return;

		GetGame().GetCallqueue().CallLater(SRZ_EnsureGenerated, 5000, false);
	}

	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(SRZ_EnsureGenerated);
		super.OnDelete(owner);
	}

	override void GenerateRandomProperties()
	{
		super.GenerateRandomProperties();
		SRZ_MarkGenerated();
	}

	void SRZ_MarkGenerated()
	{
		m_bSRZ_Generated = true;
		if (Replication.IsServer())
			Replication.BumpMe();
	}

	bool SRZ_HasAnyValue()
	{
		return m_fPropertiesToxicLevel != 0 || m_fPropertiesRadiactiveLevel != 0 || m_fPropertiesPsyLevel != 0
			|| m_fPropertiesPhysicalsLevel != 0 || m_fPropertiesThermoLevel != 0 || m_fPropertiesElectroLevel != 0
			|| m_fPropertiesHealthLevel != 0 || m_fPropertiesBloodLevel != 0 || m_fPropertiesStaminaLevel != 0
			|| m_fPropertiesWaterLevel != 0 || m_fPropertiesEatLevel != 0;
	}

	// Artifacts from traders, loot, quests and direct field spawns never got rolled
	protected void SRZ_EnsureGenerated()
	{
		if (m_bSRZ_Generated || !GetOwner())
			return;

		if (SRZ_HasAnyValue())
		{
			SRZ_MarkGenerated();
			return;
		}

		LoadParamsFromDatabase();
		GenerateRandomProperties();
	}
}

modded class EPF_ArmstArtefactPropertiesComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		EPF_EApplyResult result = super.ApplyTo(owner, component, attributes);

		ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(component);
		if (props && props.SRZ_HasAnyValue())
			props.SRZ_MarkGenerated();

		return result;
	}
}
