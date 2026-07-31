// scripts/Game/Fixes/SRZ_TradeBuySpaceCheckFix.c
modded class ARMST_TRADE_BUY_ACTIONS
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Resource m_Resource = Resource.Load(m_PrefabToSpawn);
		EntitySpawnParams params();
		m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.800;
		params.Parent = pOwnerEntity;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
			return;

		ARMST_PLAYER_STATS_COMPONENT currencyComp = ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!currencyComp)
			return;

		int totalCurrency = currencyComp.GetValue();
		float buyPrice = Helpers.GetPrefabBuyPrice(m_PrefabToSpawn);
		if (totalCurrency < buyPrice)
		{
			Print("[ARMST_TRADE] Недостаточно денег для покупки!");
			return;
		}

		IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
		if (!newEnt)
		{
			Print("[ARMST_TRADE] Ошибка при создании предмета!");
			return;
		}

		bool inserted = inventoryManager.TryInsertItem(newEnt);

		if (!inserted)
		{
			Print("[ARMST_TRADE] Недостаточно места в инвентаре, покупка отменена.");
			SCR_EntityHelper.DeleteEntityAndChildren(newEnt); // VERIFY: exact deletion helper/API may differ in your build
			return;
		}

		if (currencyComp) 
		{
			currencyComp.ModifyValue(buyPrice, false);
			ShowPurchaseNotification(pUserEntity, buyPrice, totalCurrency - buyPrice);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ShowPurchaseNotification(IEntity player, float buyPrice, float updatedMoney)
	{
		string itemName4 = "#armst_player_cash";
		string message = string.Format("%2: %1 RUB.", updatedMoney, itemName4);
		string itemName3 = "#Armst_buy_done";
		string itemName = Helpers.GetPrefabDisplayName(m_PrefabToSpawn);
		string message2 = string.Format("%3 %1 за %2 RUB.", itemName, buyPrice, itemName3);
		if (Replication.IsServer())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode.IsHosted())
			{
				// SCR_PlayerController.ShowNotification(player, message, message2);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~ARMST_TRADE_BUY_ACTIONS()
	{
	}
};