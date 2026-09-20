modded class SCR_PlayerListMenu : SCR_SuperMenuBase
{
	override protected void UpdateGameMasterIndicator(notnull SCR_PlayerListEntry entry, bool editorIslimited)
	{
		Widget gameMasterIndicator = entry.m_wRow.FindAnyWidget(m_sGameMasterIndicatorName);
		if (gameMasterIndicator)
			gameMasterIndicator.SetVisible(false);
	}
}