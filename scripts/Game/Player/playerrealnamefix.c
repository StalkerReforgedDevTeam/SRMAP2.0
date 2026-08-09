// SRZ_GMInspectRealGamertag.c
// Makes the Game Master inspect panel show the real gamertag alongside the RP
// name, instead of only the RP name, so admins can identify/ban players.
//
// Format shown: "Gamertag (RPName)"
//
// NOTE: This overrides SCR_EditableCharacterComponent.GetDisplayName(), which is
// the display-name path used by GM's editable-entity inspection. If the GM
// inspect panel still shows only the RP name after deploying this, the panel
// may be reading from a different method entirely (e.g. a GM-specific
// inspector class, or PlayerManager.GetPlayerDisplayName() called directly) -
// in that case this file needs to be re-targeted at the real call site.

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