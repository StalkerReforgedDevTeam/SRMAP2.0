modded class SCR_PlayerListMenu : SCR_SuperMenuBase
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		if (!SRZ_CanViewPlayerList())
			GetGame().GetCallqueue().CallLater(SRZ_ClosePlayerList, 1, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ClosePlayerList()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_CanViewPlayerList()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (pm && pm.HasPlayerRole(pc.GetPlayerId(), EPlayerRole.ADMINISTRATOR))
			return true;

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		return editorManager && !editorManager.IsLimited();
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateGameMasterIndicator(notnull SCR_PlayerListEntry entry, bool editorIslimited)
	{
		Widget gameMasterIndicator = entry.m_wRow.FindAnyWidget(m_sGameMasterIndicatorName);
		if (gameMasterIndicator)
			gameMasterIndicator.SetVisible(false);
	}
}