modded class ARMST_TRIGGER_SPAWN : ARMST_TRIGGER_SPAWN
{
    private static const float DESPAWN_RANGE = 200;
    private static const int CHECK_INTERVAL_MS = 30000;
    private bool m_bCleanupScheduled = false;

    override void OnActivate(IEntity ent)
    {
        super.OnActivate(ent);

        if (!Replication.IsServer())
            return;

        if (m_SpawnedObjects.Count() > 0 && !m_bCleanupScheduled)
        {
            m_bCleanupScheduled = true;
            GetGame().GetCallqueue().CallLater(CheckPlayerProximity, CHECK_INTERVAL_MS, true);
        }
    }

    override void OnDeactivate(IEntity ent)
    {
        GetGame().GetCallqueue().Remove(CheckPlayerProximity);
        m_bCleanupScheduled = false;
        super.OnDeactivate(ent);
    }

    void CheckPlayerProximity()
    {
        if (!Replication.IsServer())
            return;

        if (m_SpawnedObjects.Count() == 0)
        {
            GetGame().GetCallqueue().Remove(CheckPlayerProximity);
            m_bCleanupScheduled = false;
            return;
        }

        array<int> playerIds = {};
        GetGame().GetPlayerManager().GetAllPlayers(playerIds);

        foreach (int playerId : playerIds)
        {
            IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
            if (!playerEntity)
                continue;

            SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(playerEntity);
            if (character && character.GetCharacterController().GetLifeState() == ECharacterLifeState.DEAD)
                continue;

            if (vector.Distance(playerEntity.GetOrigin(), m_TriggerCenter) <= DESPAWN_RANGE)
                return;
        }

        Print("ARMST_TRIGGER_SPAWN [override]: No players within 200m, despawning entities.");

        for (int i = 0; i < m_SpawnedObjects.Count(); i++)
        {
            IEntity entity = m_SpawnedObjects[i];
            if (entity)
                SCR_EntityHelper.DeleteEntityAndChildren(entity);
        }

        m_SpawnedObjects.Clear();
        m_Initialized = false;
        GetGame().GetCallqueue().Remove(CheckPlayerProximity);
        m_bCleanupScheduled = false;
    }
}