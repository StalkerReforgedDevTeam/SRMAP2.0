modded class SCR_EditableCharacterComponent
{
	//------------------------------------------------------------------------------------------------
	override string GetDisplayName()
	{
		int playerId = GetPlayerID();
		if (playerId <= 0)
			return super.GetDisplayName();

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return super.GetDisplayName();

		// Pull the real Gamertag directly, bypassing the ARMST-modded
		// GetPlayerDisplayName (which returns the RP name instead of the Gamertag)
		string gamertag = pm.GetPlayerName(playerId);
		if (gamertag.IsEmpty())
			return super.GetDisplayName();

		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (!profileMgr)
			return gamertag;

		string rpName = profileMgr.GetNameForPlayer(playerId);
		if (rpName.IsEmpty())
			return gamertag;

		return string.Format("%1 (%2)", gamertag, rpName);
	}
}