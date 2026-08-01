// scripts/Game/Fixes/SRZ_TraderUIBuyCapFix.c
//
// Client-side companion to SRZ_TraderBuyCapFix.c: gives the player an immediate
// "max 20 per purchase" message when they type an oversized quantity, instead
// of the request silently dying once it reaches the (also capped) server function.
//
// SRZ_MAX_TRADER_BUY_COUNT here must be kept in sync by hand with the value in
// SRZ_TraderBuyCapFix.c — they're two independent constants since these are two
// separate patch files.

modded class ARMST_TRADER_UI
{
	protected const int SRZ_MAX_TRADER_BUY_COUNT = 20;

	//------------------------------------------------------------------------------------------------
	override void HandleBuyAction()
	{
		string buyCountText = EditCountBuy.GetText();
		int buyCount = 1;
		if (!buyCountText.IsEmpty())
		{
			buyCount = buyCountText.ToInt();
		}

		if (buyCount > SRZ_MAX_TRADER_BUY_COUNT)
		{
			Print("[ARMST TRADER] Ошибка: Превышен лимит покупки за раз.", LogLevel.WARNING);
			if (TextMessage)
				TextMessage.SetText("Максимум за одну покупку: " + SRZ_MAX_TRADER_BUY_COUNT.ToString());
			return;
		}

		super.HandleBuyAction();
	}
}