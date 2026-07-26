// Hooks ARMST_EDITOR_GLOBAL_SETTINGS.SaveMessage() - the actual method that
// fires for messages sent through HandleMessageFromClient() / HandleMessageFromClientDeath().
// Confirmed via server log:
//   [ARMST PDA] Server: Processing message from client: ...
//   [ARMST PDA] Saved message from ...: .... Total saved messages: N
// Neither of those log lines come from AddMessage/AddMessageFaction on
// ARMST_PLAYER_STATS_COMPONENT - they come from SaveMessage() on this class.
// That existing AddMessage/AddMessageFaction hook stays valid for direct
// player-to-player and faction PDA messages (those route through
// HandleMessageFromClientToClient / HandleMessageFromClientToFaction instead).
modded class ARMST_EDITOR_GLOBAL_SETTINGS
{
	//------------------------------------------------------------------------------------------------
	protected override void SaveMessage(string senderName, string message)
	{
		super.SaveMessage(senderName, message);

		if (!Replication.IsServer())
			return;

		SRZ_PDALoggerComponent.LogPDAMessage(senderName, message, "System");
	}
}