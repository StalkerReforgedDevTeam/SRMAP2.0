[BaseContainerProps()]
modded class ARMST_TRADER_UI : ChimeraMenuBase
{
	protected ref array<ref ResourceName> m_RTTZActiveTraderConfigs;
	protected string m_RTTZActiveTraderCategory;
	protected bool m_RTTZTraderCategoryReloadPending;
	protected bool m_RTTZRefreshScheduled;
	protected bool m_RTTZTradeCallbackReceived;
	protected ResourceName m_RTTZPendingTradePrefab;
	protected bool m_RTTZPendingTradeWasSell;
	protected bool m_RTTZPendingTradeWasBuy;

	override void Init(IEntity User, IEntity TRADER)
	{
		super.Init(User, TRADER);

		if (EditCountBuy)
			EditCountBuy.AddHandler(this);
		if (EditCountSell)
			EditCountSell.AddHandler(this);
	}

	override void LoadTraderCategory(array<ref ResourceName> traderConfigs, string categoryName)
	{
		m_RTTZActiveTraderConfigs = traderConfigs;
		m_RTTZActiveTraderCategory = categoryName;
		super.LoadTraderCategory(traderConfigs, categoryName);
	}

	override void SelectItemInGrid(ResourceName prefabName, bool isPlayerItem)
	{
		super.SelectItemInGrid(prefabName, isPlayerItem);
		RTTZ_UpdateTradePrice(isPlayerItem);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == Button_Buy || w == Button_Sell)
			m_RTTZTradeCallbackReceived = false;

		return super.OnClick(w, x, y, button);
	}

	override void UpdateInv(ResourceName m_PrefabTrader)
	{
		m_RTTZPendingTradePrefab = m_SelectedPrefab;
		m_RTTZPendingTradeWasSell = m_bIsPlayerItemSelected && !m_SelectedPrefab.IsEmpty();
		m_RTTZPendingTradeWasBuy = !m_bIsPlayerItemSelected && !m_SelectedPrefab.IsEmpty();

		UpdatePdaUI();

		if (TextInv0)
			TextInv0.SetText(GetInventoryCount(m_PrefabTrader).ToString());
		if (EditCountSell)
			EditCountSell.SetText("1");
		if (EditCountBuy)
			EditCountBuy.SetText("1");

		if (!m_RTTZPendingTradeWasSell && !m_RTTZPendingTradeWasBuy)
		{
			m_SelectedPrefab = ResourceName.Empty;
			m_bIsPlayerItemSelected = false;
		}
		else
		{
			RTTZ_UpdateTradePrice(m_RTTZPendingTradeWasSell);
		}

		if (m_RTTZTradeCallbackReceived)
		{
			m_RTTZTradeCallbackReceived = false;
			return;
		}

		if (!m_RTTZTradeCallbackReceived)
			RTTZ_ScheduleInventoryRefresh(250);
	}

	void RTTZ_ScheduleInventoryRefresh(int delay)
	{
		if (m_RTTZRefreshScheduled)
			return;

		m_RTTZRefreshScheduled = true;
		GetGame().GetCallqueue().CallLater(RTTZ_RefreshInventory, delay, false);
	}

	void RTTZ_RefreshInventory()
	{
		m_RTTZRefreshScheduled = false;

		if (!m_wRoot)
			return;

		LoadPlayerInventory();
		if (m_TradersStockSystem)
			UpdateTraderGridStockDisplay();

		if (m_RTTZPendingTradeWasSell && !m_RTTZPendingTradePrefab.IsEmpty())
		{
			int remainingCount = GetInventoryCount(m_RTTZPendingTradePrefab);
			if (TextInv0)
				TextInv0.SetText(remainingCount.ToString());

			if (remainingCount <= 0)
			{
				m_SelectedPrefab = ResourceName.Empty;
				m_bIsPlayerItemSelected = false;
				if (Button_Sell)
					Button_Sell.SetEnabled(false);
			}
		}
		else if (m_RTTZPendingTradeWasBuy && !m_RTTZPendingTradePrefab.IsEmpty()
			&& GetTraderStock(m_RTTZPendingTradePrefab) == 0)
		{
			m_SelectedPrefab = ResourceName.Empty;
			m_bIsPlayerItemSelected = false;
			if (Button_Buy)
				Button_Buy.SetEnabled(false);
		}

		if (m_bIsPlayerItemSelected && !m_SelectedPrefab.IsEmpty())
			RTTZ_RestoreSelectedPlayerButton();
		else if (m_RTTZPendingTradeWasBuy && !m_SelectedPrefab.IsEmpty())
			RTTZ_RestoreSelectedTraderButton();
	}

	void RTTZ_RestoreSelectedPlayerButton()
	{
		for (int gridIdx = 0; gridIdx < m_aItemMaps_Player.Count(); gridIdx++)
		{
			ref map<Widget, ResourceName> itemMap = m_aItemMaps_Player[gridIdx];
			if (!itemMap)
				continue;

			foreach (Widget button, ResourceName prefabName : itemMap)
			{
				if (prefabName == m_SelectedPrefab)
				{
					GetGame().GetWorkspace().SetFocusedWidget(button);
					return;
				}
			}
		}
	}

	void RTTZ_RestoreSelectedTraderButton()
	{
		for (int gridIdx = 0; gridIdx < m_aItemMaps_Trader.Count(); gridIdx++)
		{
			ref map<Widget, ResourceName> itemMap = m_aItemMaps_Trader[gridIdx];
			if (!itemMap)
				continue;

			foreach (Widget button, ResourceName prefabName : itemMap)
			{
				if (prefabName == m_SelectedPrefab)
				{
					GetGame().GetWorkspace().SetFocusedWidget(button);
					return;
				}
			}
		}
	}

	void RTTZ_TradeCompleted(ResourceName prefabName)
	{
		if (m_RTTZRefreshScheduled)
		{
			GetGame().GetCallqueue().Remove(RTTZ_RefreshInventory);
			m_RTTZRefreshScheduled = false;
		}

		m_RTTZTradeCallbackReceived = true;
		RTTZ_ScheduleInventoryRefresh(100);
	}

	override void UpdateTraderGridStockDisplay()
	{
		super.UpdateTraderGridStockDisplay();

		if (!m_TradersStockSystem || m_RTTZTraderCategoryReloadPending)
			return;

		bool hasDepletedItem = false;
		for (int gridIdx = 0; gridIdx < m_aItemMaps_Trader.Count(); gridIdx++)
		{
			ref map<Widget, ResourceName> itemMap = m_aItemMaps_Trader[gridIdx];
			if (!itemMap)
				continue;

			foreach (Widget button, ResourceName prefabName : itemMap)
			{
				if (GetTraderStock(prefabName) == 0)
				{
					if (!m_bIsPlayerItemSelected && prefabName == m_SelectedPrefab)
					{
						m_SelectedPrefab = ResourceName.Empty;
						if (Button_Buy)
							Button_Buy.SetEnabled(false);
					}

					hasDepletedItem = true;
					break;
				}
			}

			if (hasDepletedItem)
				break;
		}

		if (hasDepletedItem && m_RTTZActiveTraderConfigs && !m_RTTZActiveTraderConfigs.IsEmpty())
		{
			m_RTTZTraderCategoryReloadPending = true;
			GetGame().GetCallqueue().CallLater(RTTZ_ReloadActiveTraderCategory, 0, false);
		}
	}

	void RTTZ_ReloadActiveTraderCategory()
	{
		m_RTTZTraderCategoryReloadPending = false;
		if (m_RTTZActiveTraderConfigs && !m_RTTZActiveTraderConfigs.IsEmpty())
			LoadTraderCategory(m_RTTZActiveTraderConfigs, m_RTTZActiveTraderCategory);
	}

	override bool OnChange(Widget w, bool finished)
	{
		if (w == EditCountBuy || w == EditCountSell)
		{
			RTTZ_UpdateTradePrice(w == EditCountSell);
			return true;
		}

		return false;
	}

	void RTTZ_UpdateTradePrice(bool selling)
	{
		if (m_SelectedPrefab.IsEmpty())
			return;

		ARMST_TRADER_CONF_DATA traderItem = FindTraderItemData(m_SelectedPrefab);
		if (!traderItem)
			return;

		EditBoxWidget countEdit;
		if (selling)
			countEdit = EditCountSell;
		else
			countEdit = EditCountBuy;
		int count = 1;
		if (countEdit && !countEdit.GetText().IsEmpty())
		{
			count = countEdit.GetText().ToInt();
			if (count <= 0)
				count = 1;
		}

		float unitPrice = traderItem.m_fFixedPrice;
		if (unitPrice <= 0)
			unitPrice = Helpers.GetPrefabBuyPrice(traderItem.m_PrefabTrader);

		if (traderItem.m_bEnablePercent)
		{
			if (selling)
				unitPrice = unitPrice * (1.0 - (traderComponent.m_fSellPercent / 100.0));
			else
				unitPrice = unitPrice * (1.0 + (traderComponent.m_fBuyPercent / 100.0));
		}

		int totalPrice = (int)(unitPrice * count);
		if (selling && TextPriceSell)
			TextPriceSell.SetText(totalPrice.ToString());
		else if (!selling && TextPriceBuy)
			TextPriceBuy.SetText(totalPrice.ToString());
	}
}