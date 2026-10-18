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

	protected ref array<IEntity> m_aSRZ_Anomalies = {};
	protected ref array<IEntity> m_aSRZ_Artifacts = {};

	// Spawning is done here instead of in ARMST so picked up artifacts are not tracked and deleted with the field
	override void OnActivate(IEntity ent)
	{
		if (!Replication.IsServer())
			return;

		if (!m_bSRZ_ArtListFiltered)
			SRZ_FilterArtList();

		if (!m_aSRZ_Anomalies.IsEmpty())
			return;

		if (!IsAlive(ent) || !EntityUtils.IsPlayer(ent))
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character || character.GetCharacterController().GetLifeState() == ECharacterLifeState.DEAD)
			return;

		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
		{
			Print("SRZ_ArtifactFix: could not load anomaly prefab " + groupPrefab, LogLevel.ERROR);
			return;
		}

		vector playerPos = ent.GetOrigin();
		for (int i = 0; i < m_SpawnCount; i++)
		{
			vector spawnPos = SRZ_RandomPosition(playerPos);
			if (spawnPos == vector.Zero)
				continue;

			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform[3] = spawnPos;
			params.TransformMode = ETransformMode.WORLD;

			IEntity anomaly = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
			if (!anomaly)
				continue;

			SCR_EntityHelper.SnapToGround(anomaly, null, 10.0, Vector(0, 0, 0), false);
			m_aSRZ_Anomalies.Insert(anomaly);

			AIControlComponent control = AIControlComponent.Cast(anomaly.FindComponent(AIControlComponent));
			if (control)
				control.ActivateAI();

			if (Math.RandomFloat(1, 99) < m_SpawnChanceArtefacts)
				SRZ_SpawnArtifactNear(anomaly);
		}

		SetUpdateRate(30);
	}

	override void OnDeactivate(IEntity ent)
	{
		if (Replication.IsServer() && SRZ_HasPlayerInside(ent))
			return;

		if (Replication.IsServer())
		{
			foreach (IEntity anomaly : m_aSRZ_Anomalies)
			{
				if (anomaly)
					SCR_EntityHelper.DeleteEntityAndChildren(anomaly);
			}

			foreach (IEntity artifact : m_aSRZ_Artifacts)
			{
				if (artifact && !SRZ_IsCarried(artifact))
					SCR_EntityHelper.DeleteEntityAndChildren(artifact);
			}

			m_aSRZ_Anomalies.Clear();
			m_aSRZ_Artifacts.Clear();
		}

		super.OnDeactivate(ent);
	}

	protected bool SRZ_IsCarried(IEntity item)
	{
		if (item.GetParent())
			return true;

		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		return inventoryItem && inventoryItem.GetParentSlot();
	}

	protected vector SRZ_RandomPosition(vector playerPos)
	{
		float radius = GetSphereRadius();
		vector centre = GetOrigin();
		for (int attempt = 0; attempt < 50; attempt++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(0, radius) * (1.0 - m_CenterBias);
			vector pos = centre + Vector(Math.Cos(angle) * dist, 0, Math.Sin(angle) * dist);

			if (vector.Distance(playerPos, pos) < m_MinDistanceToPlayer)
				continue;

			bool tooClose = false;
			foreach (IEntity other : m_aSRZ_Anomalies)
			{
				if (other && vector.Distance(other.GetOrigin(), pos) < m_MinDistanceBetweenAnomalies)
				{
					tooClose = true;
					break;
				}
			}

			if (tooClose)
				continue;

			float surface = GetSurfaceHeight(pos);
			if (ChimeraWorldUtils.TryGetWaterSurfaceSimple(GetWorld(), Vector(pos[0], surface - 0.5, pos[2])))
				continue;

			return Vector(pos[0], surface + 0.5, pos[2]);
		}

		return vector.Zero;
	}

	protected void SRZ_SpawnArtifactNear(IEntity anomaly)
	{
		ResourceName prefab = GetRandomPrefab();
		if (prefab == ResourceName.Empty)
			return;

		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid())
			return;

		vector anomalyPos = anomaly.GetOrigin();
		float x = anomalyPos[0] + Math.RandomFloatInclusive(-5, 5);
		float z = anomalyPos[2] + Math.RandomFloatInclusive(-5, 5);
		float surface = GetSurfaceHeight(Vector(x, anomalyPos[1], z));

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = Vector(x, surface + 0.5, z);
		params.TransformMode = ETransformMode.WORLD;

		IEntity artifact = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		if (!artifact)
			return;

		SCR_EntityHelper.SnapToGround(artifact, null, 10.0, Vector(0, 0, 0), false);
		ARMST_ARTEFACT_COMPONENTS art = ARMST_ARTEFACT_COMPONENTS.Cast(artifact.FindComponent(ARMST_ARTEFACT_COMPONENTS));
		if (art)
			art.EnableLight();

		m_aSRZ_Artifacts.Insert(artifact);
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
