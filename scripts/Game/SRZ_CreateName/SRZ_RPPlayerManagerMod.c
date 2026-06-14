// Modify the editable character component to show RP names in GM
modded class SCR_EditableCharacterComponent
{
	//------------------------------------------------------------------------------------------------
	override string GetDisplayName()
	{
		// Get the default display name (player username)
		string baseName = super.GetDisplayName();
		
		// Try to get the player ID
		int playerId = GetPlayerID();
		if (playerId <= 0)
			return baseName;
		
		// Try to get RP name from profile manager
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (!profileMgr)
			return baseName;
		
		string rpName = profileMgr.GetNameForPlayer(playerId);
		
		// If no RP name set, just return base name
		if (rpName.IsEmpty())
			return baseName;
		
		// Return formatted as "Username (RP Name)"
		string displayName = string.Format("%1 (%2)", baseName, rpName);
		
		Print(string.Format("[SRZ_RP GM] Returning display name for player %1: '%2'", playerId, displayName), LogLevel.VERBOSE);
		
		return displayName;
	}
}