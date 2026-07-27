// Merged SCR_PlayerController mod - handles both RPC messages and commands
modded class SCR_PlayerController
{
	// ==================== MESSAGE RPC (from SRZ_RPNet) ====================
	
	// Server calls this to send a message to this specific client
	void SRZ_SendChatMessage(string message)
	{
		if (!Replication.IsServer())
			return;

		Print(string.Format("[SRZ_RP SERVER] Sending message to player: %1", message), LogLevel.NORMAL);
		Rpc(SRZ_RpcAsk_ShowMessage, message);
	}
	
	// Server calls this to send a message directly to chat (for .names command)
	void SRZ_SendToChatOnly(string message)
	{
		if (!Replication.IsServer())
			return;

		Print(string.Format("[SRZ_RP SERVER] Sending to chat: %1", message), LogLevel.NORMAL);
		Rpc(SRZ_RpcAsk_ShowInChat, message);
	}

	// Client receives and displays the message
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SRZ_RpcAsk_ShowMessage(string message)
	{
		Print(string.Format("[SRZ_RP CLIENT RPC] Received message: %1", message), LogLevel.NORMAL);
		
		// Use notification handler if available
		SRZ_RPNotificationHandler handler = SRZ_RPNotificationHandler.GetInstance();
		if (handler)
		{
			handler.QueueMessage(message);
		}
		else
		{
			// Fallback to direct display
			Print("[SRZ_RP CLIENT] Notification handler not found, using fallback", LogLevel.WARNING);
			GetGame().GetCallqueue().CallLater(ShowNotificationFallback, 50, false, message);
		}
	}
	
	// Client receives and displays message in chat only
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SRZ_RpcAsk_ShowInChat(string message)
	{
		Print(string.Format("[SRZ_RP CLIENT RPC] Received chat message: %1", message), LogLevel.NORMAL);
		
		// Send directly to chat using the chat component
		SCR_ChatComponent chatComp = SCR_ChatComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ChatComponent));
		if (chatComp)
		{
			chatComp.ShowMessage(message);
		}
		else
		{
			Print(string.Format("[SRZ_RP] Chat component not available: %1", message), LogLevel.NORMAL);
		}
	}
	
	// Fallback notification display if handler not available
	protected void ShowNotificationFallback(string message)
	{
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		if (popup)
		{
			popup.PopupMsg(message, 5.0);
			return;
		}
		
		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();
		if (hintMgr)
		{
			hintMgr.ShowCustomHint(message, "RP Name", 5);
			return;
		}
		
		Print(string.Format("[SRZ_RP] >>> %1 <<<", message), LogLevel.NORMAL);
	}
	
	// ==================== ARMST NAME SYNC ====================
	
	// Pushes a name set through the SRZ RP name system into ARMST_PLAYER_STATS_COMPONENT,
	// since the PDA and other UI screens read m_statistik_player_name, not the SRZ component.
	protected void SRZ_SyncNameToArmstStats(int playerId, string newName)
	{
		if (!Replication.IsServer())
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		IEntity targetEntity = pm.GetPlayerControlledEntity(playerId);
		if (!targetEntity)
			return;

		ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(targetEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComp)
		{
			Print(string.Format("[SRZ_RP] ARMST_PLAYER_STATS_COMPONENT not found for player ID %1, name sync skipped", playerId), LogLevel.WARNING);
			return;
		}

		statsComp.ArmstPlayerSetName(newName);
	}
	
	// ==================== COMMAND RPC (from SRZ_RPServerCommands) ====================
	
	// Client sends command to server
	void SRZ_SendRPCommand(string command)
	{
		if (Replication.IsServer())
		{
			// Already on server, process directly
			SRZ_ProcessRPCommand(command);
		}
		else
		{
			// Send to server
			Rpc(SRZ_RpcAsk_ProcessRPCommand, command);
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_ProcessRPCommand(string command)
	{
		SRZ_ProcessRPCommand(command);
	}
	
	protected void SRZ_ProcessRPCommand(string text)
	{
		if (!Replication.IsServer())
			return;
		
		int playerId = GetPlayerId();
		
		// Get player info
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;
		
		if (playerId <= 0)
			return;
		
		// Check admin status
		bool isAdmin = pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR);
		
		// Get name manager
		SRZ_RPNameManager mgr = SRZ_RPNameManager.GetInstance();
		if (!mgr)
			return;
		
		// Trim and lowercase for command matching
		string trimmed = text;
		trimmed.Trim();
		string lower = trimmed;
		lower.ToLower();
		
		// Check if this is one of our commands
		bool isOurCommand =
			lower == ".myname" ||
			lower == ".namehelp" ||
			lower.StartsWith(".setname ") ||
			lower.StartsWith(".clearname ") ||
			lower.StartsWith(".setmyname ") ||
			lower == ".names";
		
		if (!isOurCommand)
			return;
		
		// .namehelp command
		if (lower == ".namehelp")
		{
			SRZ_RPNet.SendToPlayer(playerId, "=== SRZ RP Name Commands ===");
			SRZ_RPNet.SendToPlayer(playerId, ".myname - View your RP name");
			SRZ_RPNet.SendToPlayer(playerId, ".setmyname <n> - Set your own RP name (1-2 words)");
			SRZ_RPNet.SendToPlayer(playerId, ".namehelp - Show this help");
			
			if (isAdmin)
			{
				SRZ_RPNet.SendToPlayer(playerId, "--- Admin Only ---");
				SRZ_RPNet.SendToPlayer(playerId, ".setname <player> <n> - Set player's RP name");
				SRZ_RPNet.SendToPlayer(playerId, ".clearname <player> - Regenerate player's name");
				SRZ_RPNet.SendToPlayer(playerId, ".names - List all online players (shows in chat)");
			}
			return;
		}

		// .myname command
		if (lower == ".myname")
		{
			IEntity entity = GetMainEntity();
			if (!entity)
			{
				SRZ_RPNet.SendToPlayer(playerId, "[SRZ_RP] You need to be spawned.");
				return;
			}
			
			SRZ_RPNameCharacterComponent nameComp = SRZ_RPNameCharacterComponent.Cast(
				entity.FindComponent(SRZ_RPNameCharacterComponent)
			);
			
			if (!nameComp)
			{
				SRZ_RPNet.SendToPlayer(playerId, "[SRZ_RP] Name component not found.");
				return;
			}
			
			string rpName = nameComp.GetCurrentRPName();
			if (rpName.IsEmpty())
				rpName = "<not set>";
			
			SRZ_RPNet.SendToPlayer(playerId, string.Format("Your RP name: %1", rpName));
			return;
		}

		// .setmyname command
		if (lower.StartsWith(".setmyname "))
		{
			string newName = trimmed.Substring(11, trimmed.Length() - 11);
			newName.Trim();

			if (newName.IsEmpty())
			{
				SRZ_RPNet.SendToPlayer(playerId, "Usage: .setmyname <n>");
				SRZ_RPNet.SendToPlayer(playerId, "Examples: .setmyname Ghost OR .setmyname Viktor Reznov");
				return;
			}

			string validationError = "";
			if (!mgr.ValidatePlayerName(newName, validationError))
			{
				SRZ_RPNet.SendToPlayer(playerId, string.Format("[SRZ_RP] %1", validationError));
				return;
			}

			mgr.ForceApplyToCharacter(playerId, newName);
			SRZ_SyncNameToArmstStats(playerId, newName);
			SRZ_RPNet.SendToPlayer(playerId, string.Format("Your RP name: %1", newName));
			return;
		}

		// Admin commands
		if (!isAdmin)
		{
			SRZ_RPNet.SendToPlayer(playerId, "[SRZ_RP] Admin access required.");
			return;
		}

		// .names (admin only, displays in chat)
		if (lower == ".names")
		{
			array<int> playerIds = new array<int>();
			pm.GetPlayers(playerIds);
			
			SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
			
			// Send header to chat
			SRZ_RPNet.SendToChatOnly(playerId, string.Format("=== Online Players (%1) ===", playerIds.Count()));
			
			// Send each player entry to chat
			foreach (int pid : playerIds)
			{
				string playerName = pm.GetPlayerName(pid);
				
				// Get name from profile manager
				string rpName = "";
				if (profileMgr)
					rpName = profileMgr.GetNameForPlayer(pid);
				
				if (rpName.IsEmpty())
					rpName = "<not set>";
				
				SRZ_RPNet.SendToChatOnly(playerId, string.Format("%1 -> %2", playerName, rpName));
			}
			
			SRZ_RPNet.SendToChatOnly(playerId, "=== End of List ===");
			return;
		}

		// .clearname
		if (lower.StartsWith(".clearname "))
		{
			string targetName = trimmed.Substring(11, trimmed.Length() - 11);
			targetName.Trim();

			if (targetName.IsEmpty())
			{
				SRZ_RPNet.SendToPlayer(playerId, "Usage: .clearname <player>");
				return;
			}

			int targetId = FindPlayerByName(pm, targetName);
			if (targetId <= 0)
			{
				SRZ_RPNet.SendToPlayer(playerId, string.Format("Player not found: %1", targetName));
				return;
			}

			string newName = mgr.GenerateRandomName(targetId);
			mgr.ForceApplyToCharacter(targetId, newName);
			SRZ_SyncNameToArmstStats(targetId, newName);

			SRZ_RPNet.SendToPlayer(playerId, string.Format("[Admin] Regenerated: %1 -> %2", pm.GetPlayerName(targetId), newName));
			SRZ_RPNet.SendToPlayer(targetId, string.Format("Your RP name: %1", newName));
			return;
		}

		// .setname
		if (lower.StartsWith(".setname "))
		{
			string args = trimmed.Substring(9, trimmed.Length() - 9);
			args.Trim();
			
			int spaceIdx = args.IndexOf(" ");
			if (spaceIdx < 0)
			{
				SRZ_RPNet.SendToPlayer(playerId, "Usage: .setname <player> <n>");
				return;
			}

			string targetName = args.Substring(0, spaceIdx);
			string newName = args.Substring(spaceIdx + 1, args.Length() - spaceIdx - 1);
			newName.Trim();

			string validationError = "";
			if (!mgr.ValidatePlayerName(newName, validationError))
			{
				SRZ_RPNet.SendToPlayer(playerId, string.Format("[SRZ_RP] %1", validationError));
				return;
			}

			int targetId = FindPlayerByName(pm, targetName);
			if (targetId <= 0)
			{
				SRZ_RPNet.SendToPlayer(playerId, string.Format("Player not found: %1", targetName));
				return;
			}

			mgr.ForceApplyToCharacter(targetId, newName);
			SRZ_SyncNameToArmstStats(targetId, newName);
			SRZ_RPNet.SendToPlayer(playerId, string.Format("[Admin] Set %1's name: %2", pm.GetPlayerName(targetId), newName));
			SRZ_RPNet.SendToPlayer(targetId, string.Format("Your RP name: %1", newName));
			return;
		}
	}
	
	protected int FindPlayerByName(PlayerManager pm, string searchName)
	{
		if (!pm) return -1;
		
		searchName.ToLower();
		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);
		
		foreach (int pid : playerIds)
		{
			string playerName = pm.GetPlayerName(pid);
			string lowerName = playerName;
			lowerName.ToLower();
			
			if (lowerName == searchName || lowerName.Contains(searchName))
				return pid;
		}
		
		return -1;
	}

	// ==================== DICE ROLL + GAMBLE COMMANDS ====================
	// Step 1: ".diceroll" - rolls 1-100 and shows the player their current number
	// Step 2: ".gamble <amount> <higher|lower>" - rolls 1-100 again for flavor and
	// display only. The actual win/loss is a fixed 10% chance regardless of the
	// higher/lower call or the roll values. Must .diceroll again before gambling
	// a second time (roll is locked/consumed on use).

	protected float m_fSRZ_GambleCooldownUntil = 0;
	protected int m_iSRZ_CurrentDiceRoll = -1; // -1 = no roll yet

	protected ref array<string> m_aSRZ_GambleWinMessages = {
		"Lucky roll! You cleaned house.",
		"The dice favored you this time.",
		"Nice bet - you called it right.",
		"Winner! The house pays out.",
		"You read the dice perfectly."
	};

	protected ref array<string> m_aSRZ_GambleLoseMessages = {
		"Tough luck - the dice didn't cooperate.",
		"The house wins this round.",
		"Not this time, better luck next roll.",
		"You called it wrong. Dice giveth, dice taketh.",
		"Close, but no payout this round."
	};

	void SRZ_SendDiceRollCommand(string msg)
	{
		if (Replication.IsServer())
		{
			SRZ_ProcessDiceRollCommand(msg);
		}
		else
		{
			Rpc(SRZ_RpcAsk_ProcessDiceRollCommand, msg);
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_ProcessDiceRollCommand(string msg)
	{
		SRZ_ProcessDiceRollCommand(msg);
	}

	// Prevents spamming .diceroll to fish for a favorable number before committing
	// to .gamble. Once a roll is active, it's locked permanently - the player must
	// .gamble that exact roll before they're allowed to roll again. No timeout.
	protected void SRZ_ProcessDiceRollCommand(string msg)
	{
		if (!Replication.IsServer())
			return;

		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		// Already have a live, unspent roll - refuse to reroll until it's used.
		if (m_iSRZ_CurrentDiceRoll != -1)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("You already rolled %1. Type .gamble with an amount and higher or lower to use it.", m_iSRZ_CurrentDiceRoll));
			return;
		}

		int total = Math.RandomInt(1, 101); // 1-100

		m_iSRZ_CurrentDiceRoll = total;

		SRZ_RPNet.SendToPlayer(playerId, string.Format("You rolled %1. Now type .gamble with a amount and higher or lower", total));
	}

	void SRZ_SendGambleCommand(string msg)
	{
		if (Replication.IsServer())
		{
			SRZ_ProcessGambleCommand(msg);
		}
		else
		{
			Rpc(SRZ_RpcAsk_ProcessGambleCommand, msg);
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_ProcessGambleCommand(string msg)
	{
		SRZ_ProcessGambleCommand(msg);
	}

	protected void SRZ_ProcessGambleCommand(string msg)
	{
		if (!Replication.IsServer())
			return;

		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		array<string> parts = {};
		msg.Split(" ", parts, true);

		if (parts.Count() < 3)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Usage: .gamble <amount> <higher|lower>");
			return;
		}

		int betAmount = parts[1].ToInt();
		string prediction = parts[2];
		prediction.ToLower();

		if (prediction != "higher" && prediction != "lower")
		{
			SRZ_RPNet.SendToPlayer(playerId, "Usage: .gamble <amount> <higher|lower>");
			return;
		}

		if (betAmount <= 0)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Bet amount must be a positive number.");
			return;
		}

		if (m_iSRZ_CurrentDiceRoll < 0)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Roll first with .diceroll before you gamble.");
			return;
		}

		float now = GetGame().GetWorld().GetWorldTime();
		if (now < m_fSRZ_GambleCooldownUntil)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Slow down - wait before gambling again.");
			return;
		}

		IEntity playerEntity = GetMainEntity();
		if (!playerEntity)
		{
			SRZ_RPNet.SendToPlayer(playerId, "[SRZ] You need to be spawned.");
			return;
		}

		ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(playerEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComp)
			return;

		int currentMoney = statsComp.GetValue();
		if (currentMoney < betAmount)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("You need %1 roubles to place this bet.", betAmount));
			return;
		}

		m_fSRZ_GambleCooldownUntil = now + 180000; // 3 minute cooldown, ms

		int firstRoll = m_iSRZ_CurrentDiceRoll;

		m_iSRZ_CurrentDiceRoll = -1; // must .diceroll again before next gamble

		string predictionName;
		if (prediction == "higher")
			predictionName = "Higher";
		else
			predictionName = "Lower";

		// Fixed 10% win rate, but the second roll is generated to actually match
		// the outcome - a win always shows a genuinely correct roll, a loss
		// always shows a genuinely incorrect one.
		bool won = Math.RandomFloat(0, 100) < 10;
		int secondRoll;

		if (prediction == "higher")
		{
			if (won)
			{
				if (firstRoll >= 100)
					won = false; // no number is higher than 100, can't win this roll
				else
					secondRoll = Math.RandomInt(firstRoll + 1, 101); // firstRoll+1 .. 100
			}
			if (!won)
				secondRoll = Math.RandomInt(1, firstRoll + 1); // 1 .. firstRoll (tie counts as loss)
		}
		else // "lower"
		{
			if (won)
			{
				if (firstRoll <= 1)
					won = false; // no number is lower than 1, can't win this roll
				else
					secondRoll = Math.RandomInt(1, firstRoll); // 1 .. firstRoll-1
			}
			if (!won)
				secondRoll = Math.RandomInt(firstRoll, 101); // firstRoll .. 100 (tie counts as loss)
		}

		string flavor;
		string message;

		if (won)
		{
			flavor = m_aSRZ_GambleWinMessages.GetRandomElement();
			statsComp.SetValue(currentMoney + betAmount);
			message = string.Format("Rolled %1, then %2 - you bet %3. %4 (+%5 roubles)", firstRoll, secondRoll, predictionName, flavor, betAmount);
		}
		else
		{
			flavor = m_aSRZ_GambleLoseMessages.GetRandomElement();
			statsComp.SetValue(currentMoney - betAmount);
			SRZ_ApplyGambleLossPenalty(playerEntity, statsComp);
			message = string.Format("Rolled %1, then %2 - you bet %3. %4 (-%5 roubles)", firstRoll, secondRoll, predictionName, flavor, betAmount);
		}

		SRZ_RPNet.SendToPlayer(playerId, message);
	}

	// Applies -20 health and -20 psy on a lost gamble. Health floors at 1 so a
	// loss can't kill the player outright, same floor pattern as the vitals system.
	protected void SRZ_ApplyGambleLossPenalty(IEntity playerEntity, ARMST_PLAYER_STATS_COMPONENT statsComp)
	{
		// Health
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(playerEntity.FindComponent(DamageManagerComponent));
		if (damageManager)
		{
			HitZone defaultHitZone = damageManager.GetDefaultHitZone();
			if (defaultHitZone)
			{
				float currentHealth = defaultHitZone.GetHealth();
				float newHealth = currentHealth - 20;
				if (newHealth < 1)
					newHealth = 1;

				defaultHitZone.SetHealth(newHealth);
			}
		}

		// Psy
		int currentPsy = statsComp.ArmstPlayerStatGetPsy();
		int newPsy = currentPsy - 20;
		if (newPsy < 0)
			newPsy = 0;

		statsComp.ArmstPlayerStatSetPsyDirect(newPsy);
	}

	// ==================== PAYTRANSFER COMMAND ====================
	// Usage: ".paytransfer <player> <amount>"  e.g. ".paytransfer Vasya 5000"
	// Transfers roubles from the sender to a target player, both must be
	// currently spawned in. No limits, no cooldown.

	void SRZ_SendPayTransferCommand(string msg)
	{
		if (Replication.IsServer())
		{
			SRZ_ProcessPayTransferCommand(msg);
		}
		else
		{
			Rpc(SRZ_RpcAsk_ProcessPayTransferCommand, msg);
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_ProcessPayTransferCommand(string msg)
	{
		SRZ_ProcessPayTransferCommand(msg);
	}

	protected void SRZ_ProcessPayTransferCommand(string msg)
	{
		if (!Replication.IsServer())
			return;

		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		array<string> parts = {};
		msg.Split(" ", parts, true);

		if (parts.Count() < 3)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Usage: .paytransfer <player> <amount>");
			return;
		}

		string targetName = parts[1];
		int amount = parts[2].ToInt();

		if (amount <= 0)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Transfer amount must be a positive number.");
			return;
		}

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		int targetId = FindPlayerByName(pm, targetName);
		if (targetId <= 0)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("Player not found: %1", targetName));
			return;
		}

		if (targetId == playerId)
		{
			SRZ_RPNet.SendToPlayer(playerId, "You can't send money to yourself.");
			return;
		}

		IEntity senderEntity = GetMainEntity();
		if (!senderEntity)
		{
			SRZ_RPNet.SendToPlayer(playerId, "[SRZ] You need to be spawned.");
			return;
		}

		IEntity targetEntity = pm.GetPlayerControlledEntity(targetId);
		if (!targetEntity)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("%1 is not currently spawned in.", targetName));
			return;
		}

		ARMST_PLAYER_STATS_COMPONENT senderStats = ARMST_PLAYER_STATS_COMPONENT.Cast(senderEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		ARMST_PLAYER_STATS_COMPONENT targetStats = ARMST_PLAYER_STATS_COMPONENT.Cast(targetEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!senderStats || !targetStats)
			return;

		int senderMoney = senderStats.GetValue();
		if (senderMoney < amount)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("You need %1 roubles to send this. You only have %2.", amount, senderMoney));
			return;
		}

		int targetMoney = targetStats.GetValue();

		senderStats.SetValue(senderMoney - amount);
		targetStats.SetValue(targetMoney + amount);

		SRZ_RPNet.SendToPlayer(playerId, string.Format("Sent %1 roubles to %2. New balance: %3", amount, pm.GetPlayerName(targetId), senderMoney - amount));
		SRZ_RPNet.SendToPlayer(targetId, string.Format("You received %1 roubles from %2. New balance: %3", amount, pm.GetPlayerName(playerId), targetMoney + amount));
	}
}

// Helper class for network operations
class SRZ_RPNet
{
	// Server sends a message to a specific client (notification)
	static void SendToPlayer(int playerId, string message)
	{
		if (!Replication.IsServer())
		{
			Print("[SRZ_RP] SendToPlayer called on client - this should not happen!", LogLevel.WARNING);
			return;
		}

		if (!message || message.IsEmpty())
		{
			Print("[SRZ_RP] Empty message attempted to send", LogLevel.WARNING);
			return;
		}

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
		{
			Print("[SRZ_RP] PlayerManager not found", LogLevel.ERROR);
			return;
		}

		SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
		if (!pc)
		{
			Print(string.Format("[SRZ_RP] PlayerController not found for player ID: %1", playerId), LogLevel.ERROR);
			return;
		}

		Print(string.Format("[SRZ_RP] Sending to player %1: %2", playerId, message), LogLevel.NORMAL);
		
		// Call our custom method that handles the RPC
		pc.SRZ_SendChatMessage(message);
	}
	
	// Server sends a message to a specific client (chat only, no notification)
	static void SendToChatOnly(int playerId, string message)
	{
		if (!Replication.IsServer())
		{
			Print("[SRZ_RP] SendToChatOnly called on client - this should not happen!", LogLevel.WARNING);
			return;
		}

		if (!message || message.IsEmpty())
		{
			Print("[SRZ_RP] Empty message attempted to send", LogLevel.WARNING);
			return;
		}

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
		{
			Print("[SRZ_RP] PlayerManager not found", LogLevel.ERROR);
			return;
		}

		SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
		if (!pc)
		{
			Print(string.Format("[SRZ_RP] PlayerController not found for player ID: %1", playerId), LogLevel.ERROR);
			return;
		}

		Print(string.Format("[SRZ_RP] Sending to chat for player %1: %2", playerId, message), LogLevel.NORMAL);
		
		// Call chat-only method
		pc.SRZ_SendToChatOnly(message);
	}

	// Broadcasts a message to all players
	static void BroadcastToAll(string message)
	{
		if (!Replication.IsServer())
			return;

		if (!message || message.IsEmpty())
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);

		Print(string.Format("[SRZ_RP] Broadcasting to %1 players: %2", playerIds.Count(), message), LogLevel.NORMAL);

		foreach (int pid : playerIds)
		{
			SendToPlayer(pid, message);
		}
	}
}