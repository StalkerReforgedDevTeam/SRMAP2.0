modded class SCR_PlayerController : PlayerController
{
	override void ArmstTraderBuy(IEntity m_User, ResourceName m_PrefabTrader, int buyCount, float totalCost)
	{
		super.ArmstTraderBuy(m_User, m_PrefabTrader, buyCount, totalCost);
		Rpc(RTTZ_RpcTraderTransactionCompleted, m_PrefabTrader);
	}

	override void ArmstTraderSell(IEntity m_User, ResourceName m_PrefabTrader, int sellCount, float totalRevenue)
	{
		super.ArmstTraderSell(m_User, m_PrefabTrader, sellCount, totalRevenue);
		Rpc(RTTZ_RpcTraderTransactionCompleted, m_PrefabTrader);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RTTZ_RpcTraderTransactionCompleted(ResourceName prefabTrader)
	{
		MenuBase menu = GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.TraderMenus);
		if (!menu)
			return;

		ARMST_TRADER_UI traderUI = ARMST_TRADER_UI.Cast(menu);
		if (traderUI)
			traderUI.RTTZ_TradeCompleted(prefabTrader);
	}
}