// PDA chat goes out under the RP name, set by the server so nobody can send under another name and admins never leak their real name.
// No anonymous messages. English prefixes. The quest log shows the running contract.
class SRZ_ContractLogEntry : Managed
{
	string m_sTitle;
	string m_sDesc;
	vector m_vPos;
}

class SRZ_PDA
{
	static const int MODE_ALL = 0;
	static const int MODE_FACTION = 1;
	static const int MODE_PM = 2;
	static const int MAX_LENGTH = 500;

	static ref SRZ_ContractLogEntry s_Contract;

	//------------------------------------------------------------------------------------------------
	static string SenderName(int playerId)
	{
		SRZ_RPNameProfileManager profiles = SRZ_RPNameProfileManager.GetInstance();
		if (profiles)
		{
			string rpName = profiles.GetNameForPlayer(playerId);
			if (!rpName.IsEmpty())
				return rpName;
		}

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (player)
		{
			ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (stats && !stats.ArmstArmstPlayerGetName().IsEmpty())
				return stats.ArmstArmstPlayerGetName();
		}

		return "Unknown stalker";
	}

	//------------------------------------------------------------------------------------------------
	static string FactionName(ARMST_FACTION_LABEL faction)
	{
		switch (faction)
		{
			case ARMST_FACTION_LABEL.FACTION_STALKER: return "Loners";
			case ARMST_FACTION_LABEL.FACTION_BANDIT: return "Bandits";
			case ARMST_FACTION_LABEL.FACTION_ARMY: return "UKM";
			case ARMST_FACTION_LABEL.FACTION_MUTANTS: return "Monolith";
			case ARMST_FACTION_LABEL.FACTION_MERCENARIES: return "Mercs";
			case ARMST_FACTION_LABEL.FACTION_KB: return "Freedom";
			case ARMST_FACTION_LABEL.FACTION_STORM: return "Duty";
		}

		return "faction";
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_PlayerController
{
	protected float m_fSRZNextPDA;
	protected float m_fSRZNextLog;

	//------------------------------------------------------------------------------------------------
	void SRZ_SendPDA(int mode, string text, string targetGuid, string targetName)
	{
		if (Replication.IsServer())
		{
			SRZ_DoSendPDA(mode, text, targetGuid, targetName);
			return;
		}

		Rpc(RpcAsk_SRZSendPDA, mode, text, targetGuid, targetName);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZSendPDA(int mode, string text, string targetGuid, string targetName)
	{
		SRZ_DoSendPDA(mode, text, targetGuid, targetName);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoSendPDA(int mode, string text, string targetGuid, string targetName)
	{
		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		float now = GetGame().GetWorld().GetWorldTime();
		if (now < m_fSRZNextPDA)
			return;

		m_fSRZNextPDA = now + 4000;

		text.TrimInPlace();
		if (text.IsEmpty())
			return;

		if (text.Length() > SRZ_PDA.MAX_LENGTH)
			text = text.Substring(0, SRZ_PDA.MAX_LENGTH);

		ARMST_EDITOR_GLOBAL_SETTINGS pda = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!pda)
			return;

		string name = SRZ_PDA.SenderName(playerId);
		string realName = GetGame().GetPlayerManager().GetPlayerName(playerId) + " (" + name + ")";
		string guid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);

		if (mode == SRZ_PDA.MODE_FACTION)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!player)
				return;

			ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (!stats)
				return;

			ARMST_FACTION_LABEL faction = stats.GetFactionKey();
			if (!faction)
				return;

			pda.HandleMessageFromClientToFaction("To " + SRZ_PDA.FactionName(faction) + " from: " + name, text, faction, true);
			ARMST_PDA_MESSAGE_LOGGER.LogPDAmessenger("TO FACTION", realName, guid, text);
			return;
		}

		if (mode == SRZ_PDA.MODE_PM)
		{
			if (targetGuid.IsEmpty() || targetGuid == guid)
				return;

			pda.HandleMessageFromClientToClient("PM from: " + name, text, targetGuid, true);
			ARMST_PDA_MESSAGE_LOGGER.LogPDAmessenger("PM", realName, guid, text, targetName, targetGuid);
			return;
		}

		pda.HandleMessageFromClient("All from: " + name, text, true);
		ARMST_PDA_MESSAGE_LOGGER.LogPDAmessenger("TO ALL", realName, guid, text);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_RequestContractLog()
	{
		if (Replication.IsServer())
		{
			SRZ_DoContractLog();
			return;
		}

		Rpc(RpcAsk_SRZContractLog);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZContractLog()
	{
		SRZ_DoContractLog();
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoContractLog()
	{
		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		float now = GetGame().GetWorld().GetWorldTime();
		if (now < m_fSRZNextLog)
			return;

		m_fSRZNextLog = now + 1000;

		string title, desc;
		vector pos;
		bool has = SRZ_Contracts.GetLogEntry(playerId, title, desc, pos);

		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			SRZ_ReceiveContractLog(has, title, desc, pos);
			return;
		}

		Rpc(RpcDo_SRZContractLog, has, title, desc, pos);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZContractLog(bool has, string title, string desc, vector pos)
	{
		SRZ_ReceiveContractLog(has, title, desc, pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ReceiveContractLog(bool has, string title, string desc, vector pos)
	{
		if (has)
		{
			SRZ_ContractLogEntry entry = new SRZ_ContractLogEntry();
			entry.m_sTitle = title;
			entry.m_sDesc = desc;
			entry.m_vPos = pos;
			SRZ_PDA.s_Contract = entry;
		}
		else
		{
			SRZ_PDA.s_Contract = null;
		}

		ARMST_PDA_UI.SRZ_OnContractLog();
	}
}

//------------------------------------------------------------------------------------------------
modded class ARMST_PDA_UI
{
	protected static ARMST_PDA_UI s_SRZOpen;
	protected bool m_bSRZWorkView;
	protected bool m_bSRZRefreshing;
	protected float m_fSRZSendBlockedUntil;
	protected ref SRZ_ContractLogEntry m_SRZShown;

	//------------------------------------------------------------------------------------------------
	override protected void InitializeWidgets()
	{
		super.InitializeWidgets();

		if (m_CheckboxAnonim)
		{
			m_CheckboxAnonim.SetChecked(false);
			m_CheckboxAnonim.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ToggleAnonim()
	{
		if (m_CheckboxAnonim)
			m_CheckboxAnonim.SetChecked(false);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleSendMessage()
	{
		if (!m_StatsComponent || !m_EditBoxMessage)
			return;

		if (!m_StatsComponent.ArmstPlayerPdaStat())
		{
			SCR_PlayerController.ShowNotificationPDA(m_User, "#armst_pda_system", "No connection... Wait...", 5);
			return;
		}

		float now = GetGame().GetWorld().GetWorldTime();
		if (now < m_fSRZSendBlockedUntil)
		{
			SCR_PlayerController.ShowNotificationPDA(m_User, "#armst_pda_system", "Timeout... Wait...", 5);
			return;
		}

		string messageText = m_EditBoxMessage.GetText();
		if (messageText.IsEmpty())
		{
			SCR_PlayerController.ShowNotificationPDA(m_User, "#armst_pda_system", "Message is empty!");
			return;
		}

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		m_fSRZSendBlockedUntil = now + 5000;

		if (m_CheckboxFaction && m_CheckboxFaction.IsChecked())
			controller.SRZ_SendPDA(SRZ_PDA.MODE_FACTION, messageText, "", "");
		else if (SelectPlayer != "")
			controller.SRZ_SendPDA(SRZ_PDA.MODE_PM, messageText, SelectPlayer, SelectPlayerName);
		else
			controller.SRZ_SendPDA(SRZ_PDA.MODE_ALL, messageText, "", "");

		m_EditBoxMessage.SetText("");
		ShowMessage();
	}

	//------------------------------------------------------------------------------------------------
	override void LoadQuests()
	{
		super.LoadQuests();

		m_bSRZWorkView = true;
		s_SRZOpen = this;

		m_SRZShown = SRZ_PDA.s_Contract;
		if (m_ListboxQuest && m_SRZShown)
			m_ListboxQuest.AddItem(m_SRZShown.m_sTitle, m_SRZShown, 0, -1);

		if (m_bSRZRefreshing)
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.SRZ_RequestContractLog();
	}

	//------------------------------------------------------------------------------------------------
	override void LoadQuestsCompleted()
	{
		super.LoadQuestsCompleted();
		m_bSRZWorkView = false;
	}

	//------------------------------------------------------------------------------------------------
	static void SRZ_OnContractLog()
	{
		if (!s_SRZOpen || !s_SRZOpen.m_bSRZWorkView || !s_SRZOpen.m_User)
			return;

		s_SRZOpen.m_bSRZRefreshing = true;
		s_SRZOpen.LoadQuests();
		s_SRZOpen.m_bSRZRefreshing = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnItemSelected(Widget w, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_ListboxQuest && m_ListboxQuest)
		{
			int selected = m_ListboxQuest.GetSelectedRow();
			if (selected >= 0)
			{
				SRZ_ContractLogEntry entry = SRZ_ContractLogEntry.Cast(m_ListboxQuest.GetItemData(selected, 0));
				if (entry)
				{
					if (m_TextQuestName) m_TextQuestName.SetText(entry.m_sTitle);
					if (m_TextQuestDesc) m_TextQuestDesc.SetText(entry.m_sDesc);
					if (m_TextQuestStatus) m_TextQuestStatus.SetText("#armst_uiunfo_pda_quests_status_work");
					m_QuestPosX = entry.m_vPos[0];
					m_QuestPosY = entry.m_vPos[2];
					return true;
				}
			}
		}

		return super.OnItemSelected(w, row, column, oldRow, oldColumn);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (s_SRZOpen == this)
			s_SRZOpen = null;

		super.OnMenuClose();
	}
}
