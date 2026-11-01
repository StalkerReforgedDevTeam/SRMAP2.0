// SRZ_DiscordZoneEntity.c
// Trigger-based zone that sends a Discord webhook when a NON-excluded-faction
// ARMST player enters the trigger volume. Webhook URL and excluded faction id
// are read from the SR_Zones profile config.

[ComponentEditorProps(category: "Zone / Discord", description: "Discord alert when non-excluded ARMST players enter trigger")]
class SRZ_DiscordZoneEntityClass : ScriptedGameTriggerEntityClass {}

class SRZ_DiscordZoneEntity : ScriptedGameTriggerEntity
{
	// Name of this zone (for logging + Discord message)
	[Attribute(defvalue: "Zone A", uiwidget: UIWidgets.EditBox, desc: "Friendly zone name", category: "Zone")]
	string m_sZoneName;

	protected RplComponent m_RplComponent;
	protected ref set<int> m_aAlertedPlayerIds = new set<int>();

	// ---------------------------------------------------------------------
	// Init: cache RplComponent for IsProxy() checks, ensure config exists
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		PrintFormat("[SR_Zones] DiscordZone '%1' EOnInit (owner=%2)", m_sZoneName, owner);
	}

	// ---------------------------------------------------------------------
	// Trigger enter
	override protected void OnActivate(IEntity ent)
	{
		// Only execute on authority (server / non-proxy)
		if (IsProxy())
			return;

		// Only care about player-controlled characters
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		// Get the player ID; AI / bots will usually not have a valid one
		int playerId = pm.GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0)
		{
			// This is not a real player (AI etc.) – ignore
			return;
		}

		// Only fire once per player per zone-entry, not every frame they're inside
		if (m_aAlertedPlayerIds.Contains(playerId))
			return;

		string playerName = pm.GetPlayerName(playerId);

		// -----------------------------------------------------------------
		// ARMST faction via ARMST_PLAYER_STATS_COMPONENT
		int armstFactionId = -1;
		string factionDisplay = "<no faction>";

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
		{
			armstFactionId = stats.GetFactionKey();          // ARMST_FACTION_LABEL enum value
			factionDisplay = armstFactionId.ToString();      // e.g. "4", "9"
		}

		PrintFormat("[SR_Zones] OnActivate zone='%1' player='%2' id=%3 ARMST_factionId=%4",
			m_sZoneName, playerName, playerId, armstFactionId);

		// -----------------------------------------------------------------
		// Make sure the config JSON + README exist on the server (no-op on clients).
		// Called here rather than EOnInit because replication/server state isn't
		// reliably established yet during entity init in some preview/host flows.
		SRZ_ZonesConfigManager.EnsureDefaults();

		// Read config
		if (!SRZ_ZonesConfigManager.GetBoolValue("m_bEnableZoneAlerts", true))
		{
			Print("[SR_Zones] Zone alerts disabled in config (m_bEnableZoneAlerts=false), skipping.", LogLevel.DEBUG);
			return;
		}

		// NOTE: key name must match SRZ_ZonesConfigManager.DefineDefaults() exactly.
		string mutantsIdStr = SRZ_ZonesConfigManager.GetStringValue("m_sMutantsFactionId", "4");
		int excludedFactionId = mutantsIdStr.ToInt();

		string webhookUrl = SRZ_ZonesConfigManager.GetStringValue("m_sZoneAlertWebhookURL", "");
		if (webhookUrl.IsEmpty())
		{
			Print("[SR_Zones] m_sZoneAlertWebhookURL is empty in config, cannot send webhook.", LogLevel.WARNING);
			return;
		}

		// -----------------------------------------------------------------
		// Skip excluded faction (mutants)
		if (armstFactionId == excludedFactionId && armstFactionId != -1)
		{
			PrintFormat("[SR_Zones] Player '%1' in excluded ARMST factionId %2 (mutants), not sending alert.",
				playerName, armstFactionId);
			return;
		}

		// -----------------------------------------------------------------
		// Passed all filters, send webhook (mark alerted first so repeat
		// frames this same visit don't re-send)
		m_aAlertedPlayerIds.Insert(playerId);
		SendDiscordWebhook(webhookUrl, playerName, factionDisplay);
	}

	// ---------------------------------------------------------------------
	// Trigger leave: clear the alerted flag so re-entering the zone pings again
	override void OnDeactivate(IEntity ent)
	{
		if (IsProxy())
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		int playerId = pm.GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0)
			return;

		m_aAlertedPlayerIds.RemoveItem(playerId);
	}

	// ---------------------------------------------------------------------
	// Authority check based on RplComponent (same pattern as Zelik)
	bool IsProxy()
	{
		if (!m_RplComponent)
			m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));

		bool proxy = (m_RplComponent && m_RplComponent.IsProxy());
		return proxy;
	}

	// ---------------------------------------------------------------------
	protected void SendDiscordWebhook(string webhookUrl, string playerName, string factionDisplay)
	{
		RestApi api = GetGame().GetRestApi();
		if (!api)
		{
			Print("[SR_Zones] RestApi not available, cannot send webhook.", LogLevel.ERROR);
			return;
		}

		// Use the same pattern GM Tools uses: GetContext("https://discord.com")
		RestContext ctx = api.GetContext("https://discord.com");
		if (!ctx)
		{
			Print("[SR_Zones] Could not create RestContext for Discord.", LogLevel.ERROR);
			return;
		}

		// Extract /api/webhooks/... path from full URL
		string pathAndToken = "";
		int apiPathIndex = webhookUrl.IndexOf("/api/webhooks/");
		if (apiPathIndex != -1)
		{
			int length = webhookUrl.Length() - apiPathIndex;
			pathAndToken = webhookUrl.Substring(apiPathIndex, length);
		}

		if (pathAndToken.IsEmpty() || pathAndToken.Length() < 16)
		{
			Print("[SR_Zones] Invalid webhook URL format, cannot extract path.", LogLevel.ERROR);
			return;
		}

		ctx.SetHeaders("Content-Type,application/json");

		// Build message. Player names (esp. RP names set via .setmyname) can contain
		// quotes, backslashes, or newlines - escape them properly instead of just
		// stripping quotes, or the JSON payload can end up malformed.
		string content = string.Format(
			"Target **%1** (faction %2) entering **%3**. Destroy them.",
			EscapeForJson(playerName), factionDisplay, EscapeForJson(m_sZoneName)
		);

		string body = "{ \"content\": \"" + content + "\" }";

		Print("[SR_Zones] Sending Discord webhook body: " + body, LogLevel.NORMAL);

		// Same as GM Tools: synchronous POST_now; tiny hitch is expected
		ctx.POST_now(pathAndToken, body);
	}

	// ---------------------------------------------------------------------
	// Minimal JSON string escaping so player/zone names can't break the payload.
	protected string EscapeForJson(string input)
	{
		string result = input;
		result.Replace("\\", "\\\\");
		result.Replace("\"", "\\\"");
		result.Replace("\n", " ");
		result.Replace("\r", " ");
		result.Replace("\t", " ");
		return result;
	}

	// ---------------------------------------------------------------------
	void SRZ_DiscordZoneEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	void ~SRZ_DiscordZoneEntity() {}
}