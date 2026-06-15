modded class ARMST_TRIGGER_SPAWN : SCR_BaseTriggerEntity
{
    override protected void SpawnArtifactNearAnomaly(IEntity anomalyEntity)
    {
        if (!Replication.IsServer())
            return;

        if (!anomalyEntity)
            return;

        ResourceName artifactPrefab = GetRandomPrefab();
        if (artifactPrefab == ResourceName.Empty)
            return;

        Resource resource = Resource.Load(artifactPrefab);
        if (!resource || !resource.IsValid())
            return;

        vector anomalyPos = anomalyEntity.GetOrigin();
        float newX = anomalyPos[0] + Math.RandomFloatInclusive(-5, 5);
        float newZ = anomalyPos[2] + Math.RandomFloatInclusive(-5, 5);

        // Use existing GetSurfaceHeight from base class
        float surfaceHeight = GetSurfaceHeight(Vector(newX, anomalyPos[1], newZ));
        vector spawnPosition = Vector(newX, surfaceHeight + 0.5, newZ);

        EntitySpawnParams params = new EntitySpawnParams();
        params.Transform[3] = spawnPosition;
        params.TransformMode = ETransformMode.WORLD;

        IEntity newArtifact = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
        if (!newArtifact)
        {
            Print("ARMST_TRIGGER_SPAWN: Failed to spawn artifact.", LogLevel.ERROR);
            return;
        }

        // Artifact starts hidden — detector reveals it within 2m
        newArtifact.SetFlags(EntityFlags.VISIBLE, false);

        // Still snap to ground for accurate placement
        SCR_EntityHelper.SnapToGround(newArtifact, null, 10.0, Vector(0, 0, 0), false);

        if (m_SpawnedObjects)
            m_SpawnedObjects.Insert(newArtifact);

        Print("ARMST_TRIGGER_SPAWN: Artifact spawned hidden at: " + spawnPosition, LogLevel.NORMAL);
    }
}