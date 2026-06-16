// SRZ_TimersForLootTiers.c
// Overrides ARMST_OpenStorageAction to apply random weapon conditions
// and use per-tier respawn timers from ARMST_TieredLootSpawnerComponent
modded class ARMST_OpenStorageAction : ScriptedUserAction
{
    //------------------------------------------------------------------------------------------------
    // Override Spawn to apply random weapon conditions to spawned weapons
    protected override void Spawn()
    {
        super.Spawn();

        // Apply random weapon conditions after super spawns items
        if (!m_StorageComponent)
            return;

        InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(
            GetOwner().FindComponent(InventoryStorageManagerComponent)
        );
        if (!storageManager)
            return;

        array<IEntity> items = new array<IEntity>();
        storageManager.GetItems(items);

        foreach (IEntity item : items)
        {
            if (!item)
                continue;

            WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
            if (weaponComp)
                GetGame().GetCallqueue().CallLater(ApplyRandomWeaponCondition, 100, false, item);
        }
    }

    //------------------------------------------------------------------------------------------------
    // Apply random condition to a spawned weapon (70-90 in 2-point increments)
    protected void ApplyRandomWeaponCondition(IEntity itemEntity)
    {
        if (!itemEntity)
            return;

        ARMST_ITEMS_STATS_COMPONENTS armstStats = ARMST_ITEMS_STATS_COMPONENTS.Cast(
            itemEntity.FindComponent(ARMST_ITEMS_STATS_COMPONENTS)
        );
        if (!armstStats)
            return;

        float condition = 70.0 + Math.RandomInt(0, 11) * 2.0;
        armstStats.SetConditionLevel(condition);
    }

    //------------------------------------------------------------------------------------------------
    // Override PerformAction to use tier-based respawn delay
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        BuildValidLootItemsFromDatabase();
        TryToSpawnLoot();
        m_lock_ent = false;

        if (!m_lock_ent)
        {
            if (m_bRespawnToggle)
            {
                float delay = GetTierRespawnDelay(pOwnerEntity);
                Print(string.Format("[SRZ_Loot] Container respawn delay: %1s", delay), LogLevel.NORMAL);
                GetGame().GetCallqueue().CallLater(unLockDisable, delay * 1000, false);
            }
        }

        SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
        if (gameMode.IsHosted())
        {
            int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
            IEntity localPlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerId);
            if (!localPlayerEntity)
                return;

            if (pUserEntity != localPlayerEntity)
                return;

            SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
            if (!inventoryManager)
                return;

            inventoryManager.SetStorageToOpen(pOwnerEntity);
            inventoryManager.OpenInventory();
            return;
        }

        OpenInventoryForUser(pUserEntity, pOwnerEntity);
    }

    //------------------------------------------------------------------------------------------------
    // Returns respawn delay based on loot tier from ARMST_TieredLootSpawnerComponent
    protected float GetTierRespawnDelay(IEntity ownerEntity)
    {
        // Get global settings for fallback timer
        float globalDelay = 520;
        BaseGameMode gameMode = BaseGameMode.Cast(GetGame().GetGameMode());
        if (gameMode)
        {
            ARMST_EDITOR_GLOBAL_SETTINGS settings = ARMST_EDITOR_GLOBAL_SETTINGS.Cast(gameMode.FindComponent(ARMST_EDITOR_GLOBAL_SETTINGS));
            if (settings)
                globalDelay = settings.m_ItemsRespawn;
        }

        if (!ownerEntity)
            return globalDelay;

        // Check tier from spawner component
        ARMST_TieredLootSpawnerComponent tierComp = ARMST_TieredLootSpawnerComponent.Cast(
            ownerEntity.FindComponent(ARMST_TieredLootSpawnerComponent)
        );
        if (!tierComp)
            return globalDelay;

        // Use tier string to determine delay
        if (tierComp.m_LootTiersSpawn && tierComp.m_LootTiersSpawn.Count() > 0)
        {
            string highestTier = "";
            foreach (string tier : tierComp.m_LootTiersSpawn)
            {
                string lowerTier = tier;
                lowerTier.ToLower();
                if (lowerTier.Contains("hard") || lowerTier.Contains("tier3") || lowerTier.Contains("3"))
                {
                    highestTier = "hard";
                    break;
                }
                else if (lowerTier.Contains("med") || lowerTier.Contains("tier2") || lowerTier.Contains("2"))
                {
                    if (highestTier != "hard")
                        highestTier = "medium";
                }
                else if (lowerTier.Contains("low") || lowerTier.Contains("tier1") || lowerTier.Contains("1"))
                {
                    if (highestTier.IsEmpty())
                        highestTier = "low";
                }
            }

            if (highestTier == "hard")   return 1600;
            if (highestTier == "medium") return 800;
            if (highestTier == "low")    return 520;
        }

        return globalDelay;
    }
}