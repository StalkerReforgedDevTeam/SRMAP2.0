[EntityEditorProps(category: "GameScripted/Triggers", description: "Trigger area that sends a Discord webhook ping when a non-mutant player enters.")]
class SRZ_DiscordPingTriggerEntityClass : ScriptedGameTriggerEntityClass {}

class SRZ_DiscordPingTriggerEntity : ScriptedGameTriggerEntity
{
	[Attribute("gorkiy", UIWidgets.EditBox, "Area name used in the ping message", category: "Discord")]
	protected string m_sAreaName;

	[Attribute("30", UIWidgets.EditBox, "Seconds a player must be out of the area before entering counts as a new visit and pings again", category: "Discord")]
	protected int m_iReentryGapSec;

	protected RplComponent m_RplComponent;
	protected ref map<int, float> m_mLastSeenInside = new map<int, float>();

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

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		int playerId = pm.GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0)
			return;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
		{
			int armstFactionId = stats.GetFactionKey();
			if (armstFactionId == 4) // FACTION_MUTANT
				return;
		}

		float now = GetGame().GetWorld().GetWorldTime();
		float lastSeen;
		bool wasInside = m_mLastSeenInside.Find(playerId, lastSeen) && (now - lastSeen) < m_iReentryGapSec * 1000;
		m_mLastSeenInside.Set(playerId, now);

		if (wasInside)
			return;

		string rpName = SRZ_RPNameProfileManager.GetInstance().GetNameForPlayer(playerId);
		if (rpName.IsEmpty())
			rpName = "An unknown stalker";

		SendDiscordWebhook(rpName);
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		if (!m_RplComponent)
			m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));

		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	protected void SendDiscordWebhook(string rpName)
	{
		RestApi api = GetGame().GetRestApi();
		if (!api)
			return;

		RestContext ctx = api.GetContext("https://discord.com");
		if (!ctx)
			return;

		string webhookUrl = SRZ_KillfeedConfigManager.GetStringValue("m_sGorkiyWebhookURL", "");
		if (webhookUrl.IsEmpty())
			return;

		string pathAndToken = "";
		int apiPathIndex = webhookUrl.IndexOf("/api/webhooks/");
		if (apiPathIndex != -1)
			pathAndToken = webhookUrl.Substring(apiPathIndex, webhookUrl.Length() - apiPathIndex);

		if (pathAndToken.IsEmpty())
			return;

		ctx.SetHeaders("Content-Type,application/json");

		string content = BuildMonolithMessage(rpName);
		content.Replace("\\", "");
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