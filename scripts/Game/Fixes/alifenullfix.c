// scripts/Game/Fixes/ARMST_ALIFE_SPAWNGROUPS_Fix.c

modded class ARMST_ALIFE_SPAWNGROUPS : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	override void AlifeSpawn()
	{
		if (!m_IsSpawnerActive || !ALife.m_AlifeMonstersEnable)
			return;

		playersNearby = false;
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_PlayerDetectionRadius, CheckForPlayers);
		if (!playersNearby)
			return;

		playersNearby = false;

		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_CheckPlayerRadius, CheckForPlayers);
		if (playersNearby)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		m_TimeMgr = world.GetTimeAndWeatherManager();
		m_TimeMgr.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);

		if (m_fMonsterNight)
		{
			if (m_iHours > 23 || m_iHours < 6)
			{
				array<ref ARMST_PLAYER_START_CONF_DATA> allItemsNight = new array<ref ARMST_PLAYER_START_CONF_DATA>();
				foreach (ResourceName configResource : m_SpawnGroupsNight)
				{
					if (configResource.IsEmpty())
						continue;

					Resource resource = BaseContainerTools.LoadContainer(configResource);
					if (!resource || !resource.IsValid())
						continue;

					BaseContainer container = resource.GetResource().ToBaseContainer();
					if (!container)
						continue;

					ARMST_PLAYER_START_CONF_Config lootConfig = ARMST_PLAYER_START_CONF_Config.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
					if (lootConfig)
					{
						if (lootConfig.m_WikiData && lootConfig.m_WikiData.Count() > 0)
						{
							foreach (ARMST_PLAYER_START_CONF_DATA lootItem : lootConfig.m_WikiData)
							{
								if (!lootItem.m_PrefabTrader.IsEmpty())
									allItemsNight.Insert(lootItem);
							}
						}
					}
				}

				if (allItemsNight.Count() > 0)
				{
					int randomItemIndexNight = Math.RandomInt(0, allItemsNight.Count());
					ARMST_PLAYER_START_CONF_DATA selectedItemNight = allItemsNight[randomItemIndexNight];

					Resource lootResourceNight = Resource.Load(selectedItemNight.m_PrefabTrader);
					if (lootResourceNight.IsValid())
						SpawnGroup(lootResourceNight);
				}

				return;
			}
		}

		array<ref ARMST_PLAYER_START_CONF_DATA> allItems = new array<ref ARMST_PLAYER_START_CONF_DATA>();
		foreach (ResourceName configResource : m_SpawnGroups)
		{
			if (configResource.IsEmpty())
				continue;

			Resource resource = BaseContainerTools.LoadContainer(configResource);
			if (!resource || !resource.IsValid())
				continue;

			BaseContainer container = resource.GetResource().ToBaseContainer();
			if (!container)
				continue;

			ARMST_PLAYER_START_CONF_Config lootConfig = ARMST_PLAYER_START_CONF_Config.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			if (lootConfig)
			{
				if (lootConfig.m_WikiData && lootConfig.m_WikiData.Count() > 0)
				{
					foreach (ARMST_PLAYER_START_CONF_DATA lootItem : lootConfig.m_WikiData)
					{
						if (!lootItem.m_PrefabTrader.IsEmpty())
							allItems.Insert(lootItem);
					}
				}
			}
		}

		if (allItems.Count() > 0)
		{
			int randomItemIndex = Math.RandomInt(0, allItems.Count());
			ARMST_PLAYER_START_CONF_DATA selectedItem = allItems[randomItemIndex];

			Resource lootResource = Resource.Load(selectedItem.m_PrefabTrader);
			if (lootResource.IsValid())
				SpawnGroup(lootResource);
		}
	}
}