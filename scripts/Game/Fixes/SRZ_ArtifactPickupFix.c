// Picking up a field artifact ran on every machine, so each client spawned its own local copy with
// its own random stats. That fake copy showed the wrong artifact and got removed when the inventory synced.
// Now only the server spawns it, and the real artifact replicates to the player.

modded class ARMST_USER_PUT_ARTEFACT
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer() || !pOwnerEntity || !pUserEntity)
			return;

		Resource resource = Resource.Load(m_PrefabToSpawn);
		if (!resource || !resource.IsValid())
			return;

		vector transform[4];
		SCR_TerrainHelper.GetTerrainBasis(pUserEntity.GetOrigin(), transform, GetGame().GetWorld(), false, new TraceParam());

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		params.TransformMode = ETransformMode.WORLD;

		IEntity spawnedObject = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		if (!spawnedObject)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);

		ARMST_ARTEFACT_PROPERTIES artStats = ARMST_ARTEFACT_PROPERTIES.Cast(spawnedObject.FindComponent(ARMST_ARTEFACT_PROPERTIES));
		if (artStats)
			artStats.GenerateRandomProperties();

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventoryManager)
			inventoryManager.TryInsertItem(spawnedObject);
	}
}
