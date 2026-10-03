modded class SCR_PlayerNamesFilterCache
{
	//------------------------------------------------------------------------------------------------
	override string GetPlayerDisplayName(int playerId)
	{
		string displayName = super.GetPlayerDisplayName(playerId);

		if (!SRZ_LocalViewerSeesRealNames())
			return displayName;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return displayName;

		string gamertag = pm.GetPlayerName(playerId);
		if (gamertag.IsEmpty() || gamertag == displayName)
			return displayName;

		if (displayName.IsEmpty())
			return gamertag;

		return string.Format("%1 (%2)", gamertag, displayName);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_LocalViewerSeesRealNames()
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
}