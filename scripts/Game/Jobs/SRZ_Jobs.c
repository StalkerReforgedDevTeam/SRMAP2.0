// Quartermaster jobs, only for DUTY_QM and FREEDOM_QM. Every other NPC keeps normal ARMST missions.
// The config's Missions list is a pool. Each day the board shows one job per pay tier, the same for everyone.
// The turn in runs only on the server: faction, distance, today's board, cooldown and items are checked, items removed, then paid.
// Cooldowns are per player per job and saved to disk.
class SRZ_Jobs
{
	static const string SAVE_DIR = "$profile:SRZ_Jobs";
	static const string SAVE_FILE = "$profile:SRZ_Jobs/cooldowns.txt";
	static const float MAX_DISTANCE = 6;

	protected static ref map<string, int> s_mCooldowns;

	//------------------------------------------------------------------------------------------------
	static ARMST_DIALOGS_CONFIG LoadConfig(ResourceName configName)
	{
		if (configName.IsEmpty())
			return null;

		Resource resource = Resource.Load(configName);
		if (!resource || !resource.IsValid())
			return null;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		if (!container)
			return null;

		return ARMST_DIALOGS_CONFIG.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsJobBoard(ARMST_DIALOGS_CONFIG config)
	{
		return config && (config.m_Actor == "DUTY_QM" || config.m_Actor == "FREEDOM_QM");
	}

	//------------------------------------------------------------------------------------------------
	static int Today()
	{
		return System.GetUnixTime() / 86400;
	}

	//------------------------------------------------------------------------------------------------
	static int Tier(float reward)
	{
		if (reward <= 15000)
			return 0;
		if (reward <= 35000)
			return 1;
		if (reward <= 70000)
			return 2;
		return 3;
	}

	//------------------------------------------------------------------------------------------------
	// Pool indices on the board for a given day, one per tier, small to large.
	static array<int> Board(ARMST_DIALOGS_CONFIG config, int day)
	{
		array<int> board = {};
		if (!config || !config.m_MissionsList)
			return board;

		for (int tier = 0; tier < 4; tier++)
		{
			array<int> inTier = {};
			foreach (int i, ARMST_MISSIONS_CONFIG mission : config.m_MissionsList)
			{
				if (mission && !mission.m_sNameMission.IsEmpty() && Tier(mission.m_RewardMoney) == tier)
					inTier.Insert(i);
			}

			if (!inTier.IsEmpty())
				board.Insert(inTier[(day + tier) % inTier.Count()]);
		}

		return board;
	}

	//------------------------------------------------------------------------------------------------
	static string Describe(ARMST_MISSIONS_CONFIG mission)
	{
		string text = mission.m_sNameMission;
		if (!mission.m_sDescMission.IsEmpty())
			text += "\n   " + mission.m_sDescMission;

		map<ResourceName, int> counts = new map<ResourceName, int>();
		if (mission.m_RequestPrefab)
		{
			foreach (ResourceName prefab : mission.m_RequestPrefab)
			{
				if (prefab.IsEmpty())
					continue;
				int count;
				counts.Find(prefab, count);
				counts.Set(prefab, count + 1);
			}
		}

		foreach (ResourceName prefab, int count : counts)
			text += "\n   Bring: " + Helpers.GetPrefabDisplayName(prefab) + " x" + count.ToString();

		text += "\n   Pay: " + ((int)mission.m_RewardMoney).ToString() + " RUB";
		return text;
	}

	//------------------------------------------------------------------------------------------------
	// 10k and 25k jobs: 8 hours. 50k: 12 hours. Above that: 24 hours.
	static int CooldownSeconds(float reward)
	{
		if (reward <= 25000)
			return 8 * 3600;
		if (reward <= 50000)
			return 12 * 3600;
		return 24 * 3600;
	}

	//------------------------------------------------------------------------------------------------
	static int GetReadyAt(string key)
	{
		Load();
		int until;
		if (s_mCooldowns.Find(key, until))
			return until;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static void SetReadyAt(string key, int until)
	{
		Load();
		s_mCooldowns.Set(key, until);
		Save();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Load()
	{
		if (s_mCooldowns)
			return;

		s_mCooldowns = new map<string, int>();
		if (!FileIO.FileExists(SAVE_FILE))
			return;

		FileHandle file = FileIO.OpenFile(SAVE_FILE, FileMode.READ);
		if (!file)
			return;

		string line;
		while (file.ReadLine(line) >= 0)
		{
			array<string> parts = {};
			line.Split("=", parts, true);
			if (parts.Count() == 2)
				s_mCooldowns.Set(parts[0], parts[1].ToInt());
		}
		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Save()
	{
		FileIO.MakeDirectory(SAVE_DIR);
		FileHandle file = FileIO.OpenFile(SAVE_FILE, FileMode.WRITE);
		if (!file)
			return;

		int now = System.GetUnixTime();
		foreach (string key, int until : s_mCooldowns)
		{
			if (until > now)
				file.WriteLine(key + "=" + until.ToString());
		}
		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	static string FormatWait(int seconds)
	{
		int hours = seconds / 3600;
		int minutes = (seconds % 3600) / 60;
		return hours.ToString() + "h " + minutes.ToString() + "m";
	}
}

modded class ARMST_DIALOGS_COMPONENT
{
	static ARMST_DIALOGS_COMPONENT s_SRZPendingJobDialog;

	//------------------------------------------------------------------------------------------------
	override void Armst_dialog_Missions(out string PlayerText, out string BotText)
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		if (!SRZ_Jobs.IsJobBoard(config))
		{
			super.Armst_dialog_Missions(PlayerText, BotText);
			return;
		}

		PlayerText = "#armst_dialogs_missions";
		BotText = "Here's what I need today. The list changes every day.\n";

		array<int> board = SRZ_Jobs.Board(config, SRZ_Jobs.Today());
		foreach (int n, int poolIndex : board)
			BotText += "\n" + (n + 1).ToString() + ". " + SRZ_Jobs.Describe(config.m_MissionsList[poolIndex]) + "\n";

		if (m_DialogUI)
			m_DialogUI.UpdateMissionButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void ButtonMissionByIndex(IEntity playerEntity, int missionIndex)
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		if (!SRZ_Jobs.IsJobBoard(config))
		{
			super.ButtonMissionByIndex(playerEntity, missionIndex);
			return;
		}

		array<int> board = SRZ_Jobs.Board(config, SRZ_Jobs.Today());
		if (missionIndex < 0 || missionIndex >= board.Count())
			return;

		missionIndex = board[missionIndex];

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!controller || !rpl)
			return;

		s_SRZPendingJobDialog = this;
		if (m_DialogUI)
			m_DialogUI.SetTextDialogue("...");

		controller.SRZ_RequestJob(rpl.Id(), missionIndex);
	}
}

modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	void SRZ_RequestJob(RplId npcId, int missionIndex)
	{
		if (Replication.IsServer())
		{
			SRZ_DoJob(npcId, missionIndex);
			return;
		}

		Rpc(RpcAsk_SRZJob, npcId, missionIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZJob(RplId npcId, int missionIndex)
	{
		SRZ_DoJob(npcId, missionIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZJobResult(string text)
	{
		SRZ_ShowJobResult(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ShowJobResult(string text)
	{
		ARMST_DIALOGS_COMPONENT dialog = ARMST_DIALOGS_COMPONENT.s_SRZPendingJobDialog;
		if (dialog && dialog.m_DialogUI)
			dialog.m_DialogUI.SetTextDialogue(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_JobReply(string text)
	{
		if (GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
		{
			SRZ_ShowJobResult(text);
			return;
		}

		Rpc(RpcDo_SRZJobResult, text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoJob(RplId npcId, int missionIndex)
	{
		IEntity player = GetMainEntity();
		IEntity npc = SCR_EntityHelper.RplIdToEntity(npcId);
		if (!player || !npc)
			return;

		if (vector.Distance(player.GetOrigin(), npc.GetOrigin()) > SRZ_Jobs.MAX_DISTANCE)
			return;

		ARMST_DIALOGS_COMPONENT dialog = ARMST_DIALOGS_COMPONENT.Cast(npc.FindComponent(ARMST_DIALOGS_COMPONENT));
		if (!dialog)
			return;

		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(dialog.m_DialogConfig);
		if (!SRZ_Jobs.IsJobBoard(config) || !config.m_MissionsList || missionIndex < 0 || missionIndex >= config.m_MissionsList.Count())
			return;

		int today = SRZ_Jobs.Today();
		if (!SRZ_Jobs.Board(config, today).Contains(missionIndex) && !SRZ_Jobs.Board(config, today - 1).Contains(missionIndex))
		{
			SRZ_JobReply("The board changed. Ask me what I need again.");
			return;
		}

		ARMST_MISSIONS_CONFIG mission = config.m_MissionsList[missionIndex];
		if (!mission)
			return;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats || stats.GetFactionKey() != ARMST_FACTION_LABEL.FACTION_STALKER)
		{
			SRZ_JobReply("I only hand out work to loners.");
			return;
		}

		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(GetPlayerId());
		if (uid.IsEmpty())
			return;

		string key = uid + "|" + config.m_Actor + "|" + mission.m_sNameMission;
		int now = System.GetUnixTime();
		int readyAt = SRZ_Jobs.GetReadyAt(key);
		if (readyAt > now)
		{
			SRZ_JobReply("You already did that one. Come back in " + SRZ_Jobs.FormatWait(readyAt - now) + ".");
			return;
		}

		map<ResourceName, int> required = new map<ResourceName, int>();
		if (mission.m_RequestPrefab)
		{
			foreach (ResourceName prefab : mission.m_RequestPrefab)
			{
				if (prefab.IsEmpty())
					continue;
				int count;
				required.Find(prefab, count);
				required.Set(prefab, count + 1);
			}
		}

		SCR_InventoryStorageManagerComponent storage = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storage)
			return;

		array<IEntity> items = {};
		storage.GetItems(items);

		string missing;
		foreach (ResourceName prefab, int need : required)
		{
			int have = 0;
			foreach (IEntity item : items)
			{
				if (item && item.GetPrefabData() && item.GetPrefabData().GetPrefabName() == prefab)
					have++;
			}

			if (have < need)
				missing += "\n" + Helpers.GetPrefabDisplayName(prefab) + " (" + have.ToString() + "/" + need.ToString() + ")";
		}

		if (!missing.IsEmpty())
		{
			SRZ_JobReply("You're short." + missing);
			return;
		}

		SRZ_Jobs.SetReadyAt(key, now + SRZ_Jobs.CooldownSeconds(mission.m_RewardMoney));

		foreach (ResourceName prefab, int need : required)
		{
			int removed = FindAndRemoveItemsInInventory(prefab, items, need);
			if (removed < need)
			{
				Print("[SRZ_Jobs] " + uid + " " + mission.m_sNameMission + " removed " + removed.ToString() + " of " + need.ToString() + " " + prefab, LogLevel.WARNING);
				SRZ_JobReply("Something went wrong with the hand in. Tell staff.");
				return;
			}
		}

		if (mission.m_RewardMoney > 0)
			stats.ModifyValue(mission.m_RewardMoney, true);

		Print("[SRZ_Jobs] " + uid + " finished " + mission.m_sNameMission + " for " + mission.m_RewardMoney.ToString(), LogLevel.NORMAL);

		string done = mission.m_sDescFinishMission;
		if (done.IsEmpty())
			done = "Good work.";
		SRZ_JobReply(done + "\n\nPaid " + ((int)mission.m_RewardMoney).ToString() + " RUB.");
	}
}

modded class ARMST_DIALOGS_UI
{
	//------------------------------------------------------------------------------------------------
	override void UpdateMissionButtons()
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		if (!SRZ_Jobs.IsJobBoard(config))
		{
			super.UpdateMissionButtons();
			return;
		}

		m_CurrentMissions.Clear();
		m_IsInMissionMode = true;
		ResetButtons();

		array<int> board = SRZ_Jobs.Board(config, SRZ_Jobs.Today());
		foreach (int n, int poolIndex : board)
		{
			ARMST_MISSIONS_CONFIG mission = config.m_MissionsList[poolIndex];
			m_CurrentMissions.Insert(mission);

			string buttonText = "#armst_dialogs_mission: " + mission.m_sNameMission;
			switch (n)
			{
				case 0: if (Talk_1_Button_Text) Talk_1_Button_Text.SetText(buttonText); break;
				case 1: if (Talk_2_Button_Text) Talk_2_Button_Text.SetText(buttonText); break;
				case 2: if (Talk_3_Button_Text) Talk_3_Button_Text.SetText(buttonText); break;
				case 3: if (Talk_4_Button_Text) Talk_4_Button_Text.SetText(buttonText); break;
			}
		}

		if (Talk_5_Button_Text)
			Talk_5_Button_Text.SetText("#armst_dialogs_mission_not_interested");
	}
}
