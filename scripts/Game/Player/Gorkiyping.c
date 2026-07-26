[EntityEditorProps(category: "GameScripted/Triggers", description: "Trigger area that sends a Discord webhook ping when a non-mutant player enters.")]
class SRZ_DiscordPingTriggerEntityClass : ScriptedGameTriggerEntityClass {}

class SRZ_DiscordPingTriggerEntity : ScriptedGameTriggerEntity
{
	[Attribute("gorkiy", UIWidgets.EditBox, "Area name used in the ping message", category: "Discord")]
	protected string m_sAreaName;

	protected RplComponent m_RplComponent;
	protected ref set<int> m_aPingedPlayerIds = new set<int>();

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnActivate(IEntity ent)
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

		// already pinged for this stay in the zone, ignore repeat activations
		if (m_aPingedPlayerIds.Contains(playerId))
			return;

		string playerName = pm.GetPlayerName(playerId);
		string rpName = playerName;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
		{
			int armstFactionId = stats.GetFactionKey();
			if (armstFactionId == 4) // FACTION_MUTANT
				return;
		}

		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (profileMgr)
		{
			string storedName = profileMgr.GetNameForPlayer(playerId);
			if (!storedName.IsEmpty())
				rpName = storedName;
		}

		m_aPingedPlayerIds.Insert(playerId);
		SendDiscordWebhook(rpName);
	}

	//------------------------------------------------------------------------------------------------
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
		if (playerId > 0)
			m_aPingedPlayerIds.RemoveItem(playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		if (!m_RplComponent)
			m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));

		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	protected void SendDiscordWebhook(string playerName)
	{
		RestApi api = GetGame().GetRestApi();
		if (!api)
			return;

		RestContext ctx = api.GetContext("https://discord.com");
		if (!ctx)
			return;

		string webhookUrl = "https://discord.com/api/webhooks/1528464951079997630/4UNq15Y_42YKHUucJbyXsvfXzwELZvLOLRMZ14naEt_YSuPKNJ4K_M2roPKuAkjPpm8U";
		string pathAndToken = "";
		int apiPathIndex = webhookUrl.IndexOf("/api/webhooks/");
		if (apiPathIndex != -1)
			pathAndToken = webhookUrl.Substring(apiPathIndex, webhookUrl.Length() - apiPathIndex);

		if (pathAndToken.IsEmpty())
			return;

		ctx.SetHeaders("Content-Type,application/json");

		string content = BuildMonolithMessage(playerName);
		content.Replace("\"", "");

		string body = "{ \"content\": \"" + content + "\" }";

		ctx.POST_now(pathAndToken, body);
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildMonolithMessage(string rpName)
	{
		ref array<string> templates = {
			"<@&1517712151752474784> the Monolith sees intruders in %1. %2 has entered sacred ground.",
			"<@&1517712151752474784> heresy detected in %1. %2 walks where the faithful alone should tread.",
			"<@&1517712151752474784> the signal grows restless. %2 has trespassed into %1.",
			"<@&1517712151752474784> unbelievers in %1. %2 will answer to the Monolith.",
			"<@&1517712151752474784> the Zone whispers of an intruder. %2 has entered %1.",
			"<@&1517712151752474784> %2 defies the will of the Monolith, entering %1 uninvited.",
			"<@&1517712151752474784> chuds in %1. %2 has been spotted."
		};

		int index = Math.RandomInt(0, templates.Count());
		string chosen = templates[index];

		return string.Format(chosen, m_sAreaName, rpName);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_DiscordPingTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	void ~SRZ_DiscordPingTriggerEntity() {}
}