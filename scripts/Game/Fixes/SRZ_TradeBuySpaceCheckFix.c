// scripts/Game/Fixes/SRZ_TraderBuyCapFix.c
//
// Full replacement of ArmstTraderBuy on the server side:
// 1. Refuses the purchase outright if buyCount exceeds SRZ_MAX_TRADER_BUY_COUNT
//    (no charge, no spawn).
// 2. Spawns + tries to insert each item, tracking every successful insert.
//    If ANY item fails to fit, deletes everything already inserted and
//    refunds nothing — no partial fills, no overcharge.
// 3. Only deducts money AFTER every single item is confirmed in the inventory.
//
// This does NOT call super.ArmstTraderBuy() — the original (locked) implementation
// has no space check at all, so delegating to it would silently reintroduce the bug.

modded class SCR_PlayerController : PlayerController
{
	protected const int SRZ_MAX_TRADER_BUY_COUNT = 20;

	//------------------------------------------------------------------------------------------------
	override void ArmstTraderBuy(IEntity m_User, ResourceName m_PrefabTrader, int buyCount, float totalCost)
	{
		if (!Replication.IsServer())
		{
			Print("[ARMST_TRADE] Ошибка: ArmstTraderBuy должен вызываться только на сервере", LogLevel.ERROR);
			return;
		}

		if (buyCount > SRZ_MAX_TRADER_BUY_COUNT)
		{
			Print(string.Format("[ARMST_TRADE] Запрошено %1, максимум за раз %2. Покупка отклонена.", buyCount, SRZ_MAX_TRADER_BUY_COUNT), LogLevel.WARNING);
			return;
		}

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(m_User.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventory)
		{
			Print("[ARMST_TRADE] Ошибка: Не удалось найти компонент инвентаря", LogLevel.ERROR);
			return;
		}

		ARMST_PLAYER_STATS_COMPONENT currencyComp = ARMST_PLAYER_STATS_COMPONENT.Cast(m_User.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!currencyComp)
		{
			Print("[ARMST_TRADE] Ошибка: Не удалось найти компонент валюты", LogLevel.ERROR);
			return;
		}

		if (currencyComp.GetValue() < totalCost)
		{
			Print("[ARMST_TRADE] Ошибка: Недостаточно денег для покупки", LogLevel.WARNING);
			return;
		}

		Resource entityResource = Resource.Load(m_PrefabTrader);
		if (!entityResource || !entityResource.IsValid())
		{
			Print("[ARMST_TRADE] Ошибка: Не удалось загрузить ресурс предмета " + m_PrefabTrader, LogLevel.ERROR);
			return;
		}

		vector transform[4];
		SCR_TerrainHelper.GetTerrainBasis(m_User.GetOrigin(), transform, GetGame().GetWorld(), false, new TraceParam());

		array<IEntity> insertedItems = new array<IEntity>();
		bool allItemsFit = true;

		for (int i = 0; i < buyCount; i++)
		{
			vector m_aOriginalTransform[4];
			m_aOriginalTransform = transform;
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = m_aOriginalTransform;
			params.TransformMode = ETransformMode.WORLD;

			IEntity itemEntity = GetGame().SpawnEntityPrefab(entityResource, GetGame().GetWorld(), params);
			if (!itemEntity)
			{
				Print("[ARMST_TRADE] Ошибка: Не удалось создать предмет", LogLevel.ERROR);
				allItemsFit = false;
				break;
			}

			if (!inventory.TryInsertItem(itemEntity))
			{
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				allItemsFit = false;
				break;
			}

			insertedItems.Insert(itemEntity);
		}

		if (!allItemsFit)
		{
			foreach (IEntity inserted : insertedItems)
			{
				if (inserted)
					SCR_EntityHelper.DeleteEntityAndChildren(inserted);
			}

			Print(string.Format("[ARMST_TRADE] Недостаточно места в инвентаре для %1 x%2, покупка отменена.", m_PrefabTrader, buyCount), LogLevel.WARNING);
			return;
		}

		currencyComp.ModifyValue(totalCost, false);

		string playerGuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(m_User);
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_User);
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		ARMST_PurchaseLogger.LogPurchase(playerGuid, playerName, m_PrefabTrader, buyCount, totalCost);
	}
}