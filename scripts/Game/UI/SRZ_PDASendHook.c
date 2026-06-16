modded class ARMST_PLAYER_STATS_COMPONENT
{
    //------------------------------------------------------------------------------------------------
    // Personal PDA messages
    override void AddMessage(string sender, string text)
    {
        super.AddMessage(sender, text);
        
        if (!Replication.IsServer())
            return;
            
        SRZ_PDALoggerComponent.LogPDAMessage(sender, text, "Personal");
    }

    //------------------------------------------------------------------------------------------------
    // Faction PDA messages
    override void AddMessageFaction(string sender, string text)
    {
        super.AddMessageFaction(sender, text);
        
        if (!Replication.IsServer())
            return;
            
        SRZ_PDALoggerComponent.LogPDAMessage(sender, text, "Faction");
    }
}