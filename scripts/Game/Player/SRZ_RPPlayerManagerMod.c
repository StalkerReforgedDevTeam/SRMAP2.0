modded class SCR_EditableCharacterComponent
{
    //------------------------------------------------------------------------------------------------
    override string GetDisplayName()
    {
        string baseName = super.GetDisplayName();

        int playerId = GetPlayerID();
        if (playerId <= 0)
            return baseName;

        SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
        if (!profileMgr)
            return baseName;

        string rpName = profileMgr.GetNameForPlayer(playerId);

        if (rpName.IsEmpty())
            return baseName;

        return string.Format("%1 (%2)", baseName, rpName);
    }
}