// scripts/Game/Fixes/SRZ_LootRandomSpawnClearFix.c
//
// Fixes: PerformAction() rebuilds m_Tiers from config every call but never
// clears it first, so every use of this loot action on a repeatedly-interacted
// object appends a duplicate copy of every tier on top of the last. Grows
// without bound for the entity's lifetime and slows DetermineLootPool() more
// each time.

modded class ARMST_LootRandomSpawn : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_sConfigs)
			return;

		// FIX: clear before rebuilding, otherwise this keeps growing forever.
		m_Tiers.Clear();

		foreach (ResourceName m_ResourceName : m_sConfigs)
		{
			if (!m_ResourceName)
				continue;
			Resource m_Resource = BaseContainerTools.LoadContainer(m_ResourceName);
			BaseContainer m_Container = m_Resource.GetResource().ToBaseContainer();

			ARMST_TieredLootConfig m_Config = ARMST_TieredLootConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(m_Container));
			if (!m_Config)
				continue;

			array<ref ARMST_TierData> m_TierData = new array<ref ARMST_TierData>;

			foreach (ARMST_TierData item : m_Config.m_TierData)
			{
				if (!item.m_sPrefab)
					continue;
				m_TierData.Insert(item);
			}

			if (!m_TierData)
				continue;

			ARMST_Tier m_Tier = new ARMST_Tier;
			m_Tier.m_sName = FilePath.StripExtension(FilePath.StripPath(m_ResourceName));
			m_Tier.m_TierData = m_TierData;
			m_Tiers.Insert(m_Tier);
		}

		DetermineLootPool();
		ARMST_TierData m_Tier = GetItemToSpawn();
		if (!m_Tier)
			return;

		Resource m_Resource = Resource.Load(m_Tier.m_sPrefab);
		EntitySpawnParams params();
		m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.800;
		params.Parent = pOwnerEntity;
		m_Attack_Timer_bool = false;

		IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
		SCR_EntityHelper.SnapToGround(newEnt);

		GetGame().GetCallqueue().CallLater(Armst_open_action, (m_fRespawnDelay * 1000), false);
	}
}