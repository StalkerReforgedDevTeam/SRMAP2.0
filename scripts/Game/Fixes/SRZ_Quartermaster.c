// Faction quartermasters (actor names ending in _QM) only trade items that have a row for that exact actor.
// "ALL" rows are ignored so they cannot be used to dump loot. Only loners can open them.
modded class ARMST_TRADER_UI
{
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity User, IEntity TRADER)
	{
		super.Init(User, TRADER);

		if (!SRZ_IsQuartermaster() || !m_StatsComponent)
			return;

		if (m_StatsComponent.GetFactionKey() != ARMST_FACTION_LABEL.FACTION_STALKER)
			Close();
	}

	//------------------------------------------------------------------------------------------------
	override void AddItemToGrid(ResourceName prefabName, bool isPlayerGrid = false)
	{
		if (!isPlayerGrid && SRZ_IsQuartermaster() && !SRZ_QuartermasterHasRow(prefabName))
			return;

		super.AddItemToGrid(prefabName, isPlayerGrid);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsItemInTraderCategory(ResourceName prefabName, array<ARMST_TRADER_CATEGORY> traderCategories)
	{
		if (SRZ_IsQuartermaster() && !SRZ_QuartermasterCanBuyFromPlayer(prefabName))
			return false;

		return super.IsItemInTraderCategory(prefabName, traderCategories);
	}

	//------------------------------------------------------------------------------------------------
	override ARMST_TRADER_CONF_DATA FindTraderItemData(ResourceName prefabName)
	{
		if (SRZ_IsQuartermaster() && !SRZ_QuartermasterHasRow(prefabName))
			return null;

		return super.FindTraderItemData(prefabName);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleSellAction()
	{
		if (SRZ_IsQuartermaster() && !SRZ_QuartermasterCanBuyFromPlayer(m_SelectedPrefab))
		{
			if (TextMessage)
				TextMessage.SetText("The quartermaster doesn't buy that.");
			return;
		}

		super.HandleSellAction();
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_IsQuartermaster()
	{
		return traderComponent && traderComponent.m_Actor.Contains("_QM");
	}

	//------------------------------------------------------------------------------------------------
	protected ARMST_DATABASE_ITEM_TRADER SRZ_QuartermasterRow(ResourceName prefabName)
	{
		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!db)
			return null;

		ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(prefabName);
		if (!dbItem || !dbItem.m_ItemTrader)
			return null;

		foreach (ARMST_DATABASE_ITEM_TRADER traderInfo : dbItem.m_ItemTrader)
		{
			if (traderInfo.m_Actor == traderComponent.m_Actor)
				return traderInfo;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_QuartermasterHasRow(ResourceName prefabName)
	{
		return SRZ_QuartermasterRow(prefabName) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_QuartermasterCanBuyFromPlayer(ResourceName prefabName)
	{
		ARMST_DATABASE_ITEM_TRADER row = SRZ_QuartermasterRow(prefabName);
		return row && row.m_EnableSell;
	}
}
