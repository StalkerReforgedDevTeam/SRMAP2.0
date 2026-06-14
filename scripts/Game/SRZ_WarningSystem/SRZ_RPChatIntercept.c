// Intercept chat messages to check for commands
// This file handles BOTH RP name commands and warn commands
// NOTE: Only have ONE file that mods SCR_ChatComponent - delete any duplicates!

modded class SCR_ChatComponent
{
    //------------------------------------------------------------------------------------------------
    override void OnNewMessage(string msg, int channelId, int senderId)
    {
        // Check if this is a command (starts with .)
        if (msg.StartsWith("."))
        {
            PlayerManager pm = GetGame().GetPlayerManager();
            if (pm)
            {
                SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(senderId));
                if (pc)
                {
                    string lower = msg;
                    lower.ToLower();
                    
                    // Route warn command to warn system
                    if (lower.StartsWith(".warn "))
                    {
                        pc.SRZ_SendWarnCommand(msg);
                        return;
                    }
                    
                    // Route rtimer command to restart timer system
                    if (lower.StartsWith(".rtimer"))
                    {
                        pc.SRZ_SendRTimerCommand(msg);
                        return;
                    }
                    
                    // Route surge command
                    if (lower == ".surge")
                    {
                        pc.SRZ_SendSurgeCommand(msg);
                        return;
                    }
                    
                    // Route all other . commands to RP name system
                    // (.myname, .setmyname, .setname, .clearname, .names, .namehelp)
                    pc.SRZ_SendRPCommand(msg);
                    return;
                }
            }
        }
        
        // Not a command, display normally
        super.OnNewMessage(msg, channelId, senderId);
    }
}