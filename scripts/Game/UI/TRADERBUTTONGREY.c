[BaseContainerProps()]
modded class ARMST_DIALOGS_UI : ChimeraMenuBase
{
	bool IsTraderButtonLocked(ARMST_DIALOGS_SCRIPTS script)
	{
		if (script != ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader)
			return false;

		if (!DialogComponent || !m_StatsComponent)
			return false;

		if (!DialogComponent.m_OnlyForFaction)
			return false;

		if (DialogComponent.m_FactionTrader == ARMST_FACTION_LABEL.FACTION_ALL)
			return false;

		return m_StatsComponent.GetFactionKey() != DialogComponent.m_FactionTrader;
	}

	void ApplyTraderLock(ButtonWidget button, bool locked)
	{
		if (!button)
			return;

		button.SetEnabled(!locked);
		if (locked)
			button.SetOpacity(0.4);
		else
			button.SetOpacity(1.0);
	}

	override void SetupDialogs()
	{
		super.SetupDialogs();

		ApplyTraderLock(Talk_1_Button, IsTraderButtonLocked(DialogComponent.m_sDialog_1_Scripts));
		ApplyTraderLock(Talk_2_Button, IsTraderButtonLocked(DialogComponent.m_sDialog_2_Scripts));
		ApplyTraderLock(Talk_3_Button, IsTraderButtonLocked(DialogComponent.m_sDialog_3_Scripts));
		ApplyTraderLock(Talk_4_Button, IsTraderButtonLocked(DialogComponent.m_sDialog_4_Scripts));
		ApplyTraderLock(Talk_5_Button, IsTraderButtonLocked(DialogComponent.m_sDialog_5_Scripts));
	}
}