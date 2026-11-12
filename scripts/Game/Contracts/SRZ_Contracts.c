// Contracts: Hold the Line, Alpha Hunt, Recovery and Artifact Commission. Taken at the faction vendors by their own members.
// Hunt, Recovery and Commission end with a walk back to the vendor who gave the job. Nothing is carried, so nothing can be stolen, sold or duped.
// The server runs everything: timer, waves, area checks, pay and cooldowns. The client only asks to start.
// Contract enemies are tracked by entity, give no parts and are removed when the contract ends. Wild mutants are never touched.
class SRZ_Contract
{
	string m_sUid;
	int m_iPlayerId;
	SRZ_ContractSite m_Site;
	ARMST_FACTION_LABEL m_eFaction;
	int m_iStarted;
	float m_fHeld;
	float m_fOutside;
	bool m_bArrived;
	bool m_bWarned;
	int m_iWavesDone;
	int m_iEndAt;
	ref array<IEntity> m_aGroups = {};
	ref array<IEntity> m_aWaypoints = {};
	ref array<IEntity> m_aBodies = {};
	ref array<ResourceName> m_aQueue = {};

	int m_iStage;
	IEntity m_Vendor;
	vector m_vVendorPos;
	int m_iArrivedAt;
	int m_iReturnAt;
	IEntity m_Alpha;
	bool m_bAlphaSpawned;
	IEntity m_Crate;
	float m_fSearch;
	bool m_bSearching;
	vector m_vTarget;
	int m_iHintBand = -1;
}

class SRZ_Contracts
{
	static const float TICK = 2;
	static const float MAX_DISTANCE = 6;
	static const int TRAVEL_LIMIT = 900;
	static const float OUTSIDE_LIMIT = 60;
	static const int COOLDOWN = 8 * 3600;
	static const int FAIL_COOLDOWN = 2 * 3600;
	static const int MAX_AI_SITE = 24;
	static const int MAX_AI_TOTAL = 60;
	static const int START_ROOM = 4;
	static const int BOARD_SIZE = 4;
	static const int WORK_LIMIT = 900;
	static const int RETURN_LIMIT = 1200;
	static const float SEARCH_RANGE = 2.5;
	static const float FOUND_RANGE = 2;
	static const int STAGE_WORK = 0;
	static const int STAGE_RETURN = 1;

	static bool s_bEnabled = true;
	protected static ref array<SRZ_ContractSite> s_aSites = {};
	protected static ref array<SRZ_ContractSpawnMarker> s_aMarkers = {};
	protected static ref map<string, ref SRZ_Contract> s_mActive = new map<string, ref SRZ_Contract>();
	protected static ref array<ref SRZ_Contract> s_aEnding = {};
	protected static ref array<IEntity> s_aTrash = {};
	protected static ref array<float> s_aTrashAt = {};
	protected static ref set<IEntity> s_ContractBodies = new set<IEntity>();

	protected static const ResourceName WAYPOINT = "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";

	protected static const ResourceName CARRIER = "{5A5A3C0000000A12}Prefabs/SRZ/Contracts/SRZ_ContractMutantGroup.et";
	protected static const ResourceName CARRIER_FALLBACK = "{1DBDD336741DF40E}Prefabs/Monsters/Groups/Group_Zombie_4.et";

	//------------------------------------------------------------------------------------------------
	// Used when a site has no wave setup. One of these per wave.
	protected static void DefaultWave(array<ResourceName> wave)
	{
		int pick = Math.RandomInt(0, 3);
		SRZ_EContractEnemy enemy = SRZ_EContractEnemy.ZOMBIE;
		int count = 4;
		if (pick == 1)
		{
			enemy = SRZ_EContractEnemy.WOLF;
			count = 3;
		}
		else if (pick == 2)
		{
			enemy = SRZ_EContractEnemy.BOAR;
			count = 3;
		}

		for (int i = 0; i < count; i++)
			wave.Insert(EnemyPrefab(enemy));
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName EnemyPrefab(SRZ_EContractEnemy enemy)
	{
		switch (enemy)
		{
			case SRZ_EContractEnemy.ZOMBIE: return "{D77F7DAF37BD0FD3}Prefabs/Monsters/Characters/Zombie/armst_monster_zombie.et";
			case SRZ_EContractEnemy.ZOMBIE_RANDOM: return "{44149F05FA2F11BF}Prefabs/Monsters/Characters/Zombie/armst_monster_zombie_random.et";
			case SRZ_EContractEnemy.ZOMBIE_PISTOL: return "{86488E3C7539BE7F}Prefabs/Monsters/Characters/Zombie/armst_monster_zombie_pistol.et";
			case SRZ_EContractEnemy.TOXIC: return "{0BA7489D81ADFA27}Prefabs/Monsters/Characters/Zombie/Toxic/armst_monster_toxic.et";
			case SRZ_EContractEnemy.TOXIC_2: return "{780BBB1FEE4674E4}Prefabs/Monsters/Characters/Zombie/Toxic/armst_monster_toxic_2.et";
			case SRZ_EContractEnemy.WOLF: return "{B6EFC6079B4EC75E}Prefabs/Monsters/Characters/wolf/armst_monster_wolf.et";
			case SRZ_EContractEnemy.BOAR: return "{4334F5D29ADDCE55}Prefabs/Monsters/Characters/boar/armst_monster_boar.et";
			case SRZ_EContractEnemy.RAT: return "{1972B20F5989520C}Prefabs/Monsters/Characters/rat/armst_monster_rat.et";
			case SRZ_EContractEnemy.NIGHTSTALKER: return "{2DF464EFB80C1E54}Prefabs/Monsters/Characters/Nightstalker/armst_monster_nighstalker_invisible.et";
			case SRZ_EContractEnemy.ERB: return "{0BA8571DF450D353}Prefabs/Monsters/Characters/Erb/armst_monster_erb.et";
			case SRZ_EContractEnemy.SPIDER: return "{AFA2592DFF1A89C9}Prefabs/Monsters/Characters/spider/armst_monster_spider.et";
			case SRZ_EContractEnemy.SPIDER_STALKER: return "{DCF905F769F99DDD}Prefabs/Monsters/Characters/Zombie/armst_monster_zombie_spider.et";
			case SRZ_EContractEnemy.GHOST: return "{03796631F9018045}Prefabs/Monsters/Characters/Ghost/armst_monster_ghost.et";
			case SRZ_EContractEnemy.PSY_DESTROYER: return "{2246776F50904DF1}Prefabs/Monsters/Characters/Psy_destroyer/armst_monster_psy_destroyer.et";
			case SRZ_EContractEnemy.ELECTRO_ELEMENTAL: return "{02CB07CF03B77AAF}Prefabs/Monsters/Characters/Elementals/Electro/armst_electro_elemental.et";
			case SRZ_EContractEnemy.FIRE_ELEMENTAL: return "{514C84C6A41B1E46}Prefabs/Monsters/Characters/Elementals/Fire/armst_fire_elemental.et";
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	// About 1 in 5 recovery crates roll from the tier 3 table.
	static ResourceName CratePrefab(SRZ_EContractLoot loot)
	{
		bool t3 = Math.RandomFloat01() < 0.2;
		switch (loot)
		{
			case SRZ_EContractLoot.GEAR:
				if (t3) return "{5A5A3C0000000B04}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Gear_T3.et";
				return "{5A5A3C0000000B03}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Gear_T2.et";
			case SRZ_EContractLoot.MEDICAL:
				if (t3) return "{5A5A3C0000000B06}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Medical_T3.et";
				return "{5A5A3C0000000B05}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Medical_T2.et";
			case SRZ_EContractLoot.UNIQUE:
				if (t3) return "{5A5A3C0000000B08}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Unique_T3.et";
				return "{5A5A3C0000000B07}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Unique_T2.et";
		}

		if (t3) return "{5A5A3C0000000B02}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Weapons_T3.et";
		return "{5A5A3C0000000B01}Prefabs/SRZ/Contracts/SRZ_ContractCrate_Weapons_T2.et";
	}

	//------------------------------------------------------------------------------------------------
	static string EnemyName(SRZ_EContractEnemy enemy)
	{
		string name = typename.EnumToString(SRZ_EContractEnemy, enemy);
		name.Replace("_", " ");
		name.ToLower();
		return name;
	}

	//------------------------------------------------------------------------------------------------
	static string TypeName(SRZ_EContractType type)
	{
		switch (type)
		{
			case SRZ_EContractType.HUNT: return "Alpha Hunt";
			case SRZ_EContractType.RECOVERY: return "Recovery";
			case SRZ_EContractType.ANOMALY: return "Artifact Commission";
		}

		return "Hold the Line";
	}

	//------------------------------------------------------------------------------------------------
	static string ButtonPrefix(SRZ_EContractType type)
	{
		switch (type)
		{
			case SRZ_EContractType.HUNT: return "Hunt: ";
			case SRZ_EContractType.RECOVERY: return "Recover: ";
			case SRZ_EContractType.ANOMALY: return "Find: ";
		}

		return "Hold: ";
	}

	//------------------------------------------------------------------------------------------------
	// Called by the mutant damage hook. 1 for anything that is not a contract alpha.
	static float AlphaToughness(IEntity mutant)
	{
		if (!mutant)
			return 1;

		foreach (string uid, SRZ_Contract contract : s_mActive)
		{
			if (contract.m_Alpha == mutant && contract.m_Site)
				return Math.Clamp(contract.m_Site.m_fAlphaToughness, 1, 15);
		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	// Vendor actor to the faction it serves. FACTION_ALL means not a contract vendor.
	static ARMST_FACTION_LABEL VendorFaction(string actor)
	{
		if (actor == "DUTY_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_STORM;
		if (actor == "MONOLITH_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_MUTANTS;
		if (actor == "UKM_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_ARMY;
		if (actor == "FREEDOM_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_KB;
		if (actor == "MERC_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_MERCENARIES;
		if (actor == "BANDIT_VENDOR")
			return ARMST_FACTION_LABEL.FACTION_BANDIT;

		return ARMST_FACTION_LABEL.FACTION_ALL;
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterSite(SRZ_ContractSite site)
	{
		if (!site || s_aSites.Contains(site))
			return;

		s_aSites.Insert(site);

		if (Replication.IsServer())
		{
			GetGame().GetCallqueue().Remove(Tick);
			GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void UnregisterSite(SRZ_ContractSite site)
	{
		if (s_aSites)
			s_aSites.RemoveItem(site);
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterMarker(SRZ_ContractSpawnMarker marker)
	{
		if (marker && !s_aMarkers.Contains(marker))
			s_aMarkers.Insert(marker);
	}

	//------------------------------------------------------------------------------------------------
	static void UnregisterMarker(SRZ_ContractSpawnMarker marker)
	{
		if (s_aMarkers)
			s_aMarkers.RemoveItem(marker);
	}

	//------------------------------------------------------------------------------------------------
	static string Grid(vector pos)
	{
		return Pad3((int)(pos[0] / 100)) + "-" + Pad3((int)(pos[2] / 100));
	}

	//------------------------------------------------------------------------------------------------
	protected static string Pad3(int value)
	{
		string text = value.ToString();
		while (text.Length() < 3)
			text = "0" + text;

		return text;
	}

	//------------------------------------------------------------------------------------------------
	static string FormatWait(int seconds)
	{
		int hours = seconds / 3600;
		int minutes = (seconds % 3600) / 60;
		return hours.ToString() + "h " + minutes.ToString() + "m";
	}

	//------------------------------------------------------------------------------------------------
	// Same list on client and server: sites for this faction, sorted, rotating daily.
	static array<SRZ_ContractSite> Board(ARMST_FACTION_LABEL faction)
	{
		array<SRZ_ContractSite> eligible = {};
		array<string> keys = {};
		foreach (SRZ_ContractSite site : s_aSites)
		{
			if (site && site.IsFor(faction))
				keys.Insert(site.GetKey());
		}

		keys.Sort();
		foreach (string key : keys)
			eligible.Insert(FindSite(key));

		if (eligible.Count() <= BOARD_SIZE)
			return eligible;

		array<SRZ_ContractSite> board = {};
		int offset = (System.GetUnixTime() / 86400) % eligible.Count();
		for (int i = 0; i < BOARD_SIZE; i++)
			board.Insert(eligible[(offset + i) % eligible.Count()]);

		return board;
	}

	//------------------------------------------------------------------------------------------------
	static SRZ_ContractSite FindSite(string key)
	{
		foreach (SRZ_ContractSite site : s_aSites)
		{
			if (site && site.GetKey() == key)
				return site;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static string Describe(SRZ_ContractSite site)
	{
		string where = site.GetDisplayName() + " (grid " + Grid(site.GetOrigin()) + ")";
		string what;
		switch (site.m_eType)
		{
			case SRZ_EContractType.HUNT:
				what = "\n   A big " + EnemyName(site.m_eAlpha) + " and its pack. Kill it, then come back to me.";
				break;
			case SRZ_EContractType.RECOVERY:
				what = "\n   Find our crate there, take what we need and come back. Keep whatever else is in it.";
				break;
			case SRZ_EContractType.ANOMALY:
				what = "\n   There's something in the field there. Find the exact spot and come back.";
				break;
			default:
				what = "\n   Hold it for " + ((int)site.m_fHoldMinutes).ToString() + " minutes against " + site.GetWaveCount().ToString() + " waves.";
				break;
		}

		return TypeName(site.m_eType) + ": " + where + what + "\n   Pay: " + site.m_iPay.ToString() + " RUB";
	}

	//------------------------------------------------------------------------------------------------
	static bool IsContractBody(IEntity entity)
	{
		return entity && s_ContractBodies.Contains(entity);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsSiteBusy(SRZ_ContractSite site)
	{
		foreach (string uid, SRZ_Contract contract : s_mActive)
		{
			if (contract.m_Site == site)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static SRZ_Contract GetActive(string uid)
	{
		return s_mActive.Get(uid);
	}

	//------------------------------------------------------------------------------------------------
	static string CooldownKey(string uid)
	{
		return "contract|" + uid + "|HOLD";
	}

	//------------------------------------------------------------------------------------------------
	static void Notify(int playerId, string text)
	{
		if (playerId > 0)
			SRZ_RPNet.SendToPlayer(playerId, text);
	}

	//------------------------------------------------------------------------------------------------
	static void Log(string text)
	{
		Print("[SRZ_Contracts] " + text, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Server. Every check happens here, the client only sent the site key.
	static string TryStart(int playerId, IEntity player, IEntity npc, string siteKey)
	{
		if (!s_bEnabled)
			return "Contracts are closed right now.";

		if (!player || !npc || vector.Distance(player.GetOrigin(), npc.GetOrigin()) > MAX_DISTANCE)
			return string.Empty;

		ARMST_DIALOGS_COMPONENT dialog = ARMST_DIALOGS_COMPONENT.Cast(npc.FindComponent(ARMST_DIALOGS_COMPONENT));
		if (!dialog)
			return string.Empty;

		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(dialog.m_DialogConfig);
		if (!config)
			return string.Empty;

		ARMST_FACTION_LABEL faction = VendorFaction(config.m_Actor);
		if (faction == ARMST_FACTION_LABEL.FACTION_ALL)
			return string.Empty;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats || stats.GetFactionKey() != faction)
			return "I only give contracts to our own people.";

		SRZ_ContractSite site = FindSite(siteKey);
		if (!site || !site.IsFor(faction) || !Board(faction).Contains(site))
			return "The board changed. Ask me again.";

		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (uid.IsEmpty())
			return string.Empty;

		SRZ_Contract running = s_mActive.Get(uid);
		if (running)
			return "You're already on a contract at " + running.m_Site.GetDisplayName() + ". Finish it or type .contract drop";

		int now = System.GetUnixTime();
		int readyAt = SRZ_Jobs.GetReadyAt(CooldownKey(uid));
		if (readyAt > now && !SRZ_IgnoreCooldown())
			return "You did one not long ago. Come back in " + FormatWait(readyAt - now) + ".";

		if (IsSiteBusy(site))
			return "Someone is already working that site. Pick another one.";

		if (TotalAlive() + START_ROOM > MAX_AI_TOTAL)
			return "Too much going on out there right now. Try again in a bit.";

		SRZ_Contract contract = new SRZ_Contract();
		contract.m_sUid = uid;
		contract.m_iPlayerId = playerId;
		contract.m_Site = site;
		contract.m_eFaction = faction;
		contract.m_iStarted = now;
		contract.m_Vendor = npc;
		contract.m_vVendorPos = npc.GetOrigin();
		s_mActive.Set(uid, contract);

		SRZ_Jobs.SetReadyAt(CooldownKey(uid), now + COOLDOWN);
		Log(uid + " started " + TypeName(site.m_eType) + " at " + site.GetDisplayName() + " (" + site.GetKey() + ")");

		string go = "Get to " + site.GetDisplayName() + ", grid " + Grid(site.GetOrigin()) + ". You have 15 minutes to get there.";
		switch (site.m_eType)
		{
			case SRZ_EContractType.HUNT:
				return go + "\nIt shows up when you get close. Anyone can help kill it, but only you get paid. Come back to me after.";
			case SRZ_EContractType.RECOVERY:
				return go + "\nSearch our crate, then come back to me. Die on the way and it's lost. Type .contract to check on it.";
			case SRZ_EContractType.ANOMALY:
				return go + "\nYou'll pick up the signal inside the field. Find the spot, then come back to me. Type .contract to check on it.";
		}

		return go + "\nThe clock starts when you're there. Hold it for " + ((int)site.m_fHoldMinutes).ToString() + " minutes. Leave for more than a minute and it's over."
			+ "\nType .contract to check on it.";
	}

	//------------------------------------------------------------------------------------------------
	// Testing in Workbench skips the cooldown. The live server never compiles this in.
	static bool SRZ_IgnoreCooldown()
	{
#ifdef WORKBENCH
		return true;
#else
		return false;
#endif
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		if (!Replication.IsServer())
			return;

		array<string> finished = {};
		array<bool> results = {};
		array<string> reasons = {};

		foreach (string uid, SRZ_Contract contract : s_mActive)
		{
			string reason;
			int state = Step(contract, reason);
			if (state != 0)
			{
				finished.Insert(uid);
				results.Insert(state > 0);
				reasons.Insert(reason);
			}
		}

		foreach (int f, string doneUid : finished)
			End(doneUid, results[f], reasons[f]);

		float now = System.GetUnixTime();
		for (int e = s_aEnding.Count() - 1; e >= 0; e--)
		{
			if (s_aEnding[e].m_iEndAt <= now)
			{
				Cleanup(s_aEnding[e]);
				s_aEnding.Remove(e);
			}
		}

		for (int t = s_aTrash.Count() - 1; t >= 0; t--)
		{
			if (s_aTrashAt[t] <= now)
			{
				if (s_aTrash[t])
					SCR_EntityHelper.DeleteEntityAndChildren(s_aTrash[t]);

				s_aTrash.Remove(t);
				s_aTrashAt.Remove(t);
			}
		}

		if (s_ContractBodies.Count() > 0)
		{
			array<IEntity> stale = {};
			foreach (IEntity body : s_ContractBodies)
			{
				if (!body)
					stale.Insert(body);
			}

			foreach (IEntity staleBody : stale)
				s_ContractBodies.RemoveItem(staleBody);
		}
	}

	//------------------------------------------------------------------------------------------------
	// 0 keep going, 1 done, -1 failed.
	protected static int Step(SRZ_Contract contract, out string reason)
	{
		SRZ_ContractSite site = contract.m_Site;
		if (!site)
		{
			reason = "The site is gone.";
			return -1;
		}

		if (SCR_PlayerIdentityUtils.GetPlayerIdentityId(contract.m_iPlayerId) != contract.m_sUid)
		{
			reason = "disconnected";
			return -1;
		}

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(contract.m_iPlayerId);
		ChimeraCharacter character = ChimeraCharacter.Cast(player);
		if (!character || !character.GetCharacterController() || character.GetCharacterController().GetLifeState() == ECharacterLifeState.DEAD)
		{
			reason = "You died. The contract is off.";
			return -1;
		}

		TrackMembers(contract);

		bool awake = character.GetCharacterController().GetLifeState() == ECharacterLifeState.ALIVE;

		if (contract.m_iStage == STAGE_RETURN)
			return StepReturn(contract, player, awake, reason);

		switch (site.m_eType)
		{
			case SRZ_EContractType.HUNT: return StepHunt(contract, player, awake, reason);
			case SRZ_EContractType.RECOVERY: return StepRecovery(contract, player, awake, reason);
			case SRZ_EContractType.ANOMALY: return StepAnomaly(contract, player, awake, reason);
		}

		return StepHold(contract, player, awake, reason);
	}

	//------------------------------------------------------------------------------------------------
	protected static int StepHold(SRZ_Contract contract, IEntity player, bool awake, out string reason)
	{
		SRZ_ContractSite site = contract.m_Site;
		bool inside = awake && Flat(player.GetOrigin(), site.GetOrigin()) <= site.m_fRadius;

		if (inside)
		{
			if (!contract.m_bArrived)
			{
				contract.m_bArrived = true;
				contract.m_iArrivedAt = System.GetUnixTime();
				Notify(contract.m_iPlayerId, "You're at " + site.GetDisplayName() + ". Hold this spot for " + ((int)site.m_fHoldMinutes).ToString() + " minutes. They're coming.");
			}
			else if (contract.m_bWarned)
			{
				Notify(contract.m_iPlayerId, "Back on the spot. The clock is running again.");
			}

			contract.m_fHeld += TICK;
			contract.m_fOutside = 0;
			contract.m_bWarned = false;
		}
		else if (contract.m_bArrived)
		{
			contract.m_fOutside += TICK;
			if (!contract.m_bWarned)
			{
				contract.m_bWarned = true;
				Notify(contract.m_iPlayerId, "You left the area. Get back within a minute or the contract is lost.");
			}

			if (contract.m_fOutside >= OUTSIDE_LIMIT)
			{
				reason = "You left " + site.GetDisplayName() + " too long. The contract is lost.";
				return -1;
			}
		}
		else if (System.GetUnixTime() - contract.m_iStarted > TRAVEL_LIMIT)
		{
			reason = "You never made it to " + site.GetDisplayName() + ". The contract is lost.";
			return -1;
		}

		if (!contract.m_bArrived)
			return 0;

		float duration = site.GetHoldSeconds();
		int waves = site.GetWaveCount();
		int due = 1 + (int)(contract.m_fHeld / site.GetWaveInterval());
		if (due > waves)
			due = waves;

		if (contract.m_iWavesDone < due)
			SpawnWave(contract);

		DrainQueue(contract);

		if (contract.m_fHeld >= duration)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// Shared by Hunt, Recovery and Commission. True once the player has reached the site.
	protected static bool CheckArrival(SRZ_Contract contract, IEntity player, bool awake, string arriveText, out int failed, out string reason)
	{
		failed = 0;
		if (contract.m_bArrived)
			return true;

		SRZ_ContractSite site = contract.m_Site;
		if (awake && Flat(player.GetOrigin(), site.GetOrigin()) <= site.m_fRadius)
		{
			contract.m_bArrived = true;
			contract.m_iArrivedAt = System.GetUnixTime();
			Notify(contract.m_iPlayerId, arriveText);
			return true;
		}

		if (System.GetUnixTime() - contract.m_iStarted > TRAVEL_LIMIT)
		{
			reason = "You never made it to " + site.GetDisplayName() + ". The contract is lost.";
			failed = -1;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool OutOfTime(SRZ_Contract contract, string text, out string reason)
	{
		if (System.GetUnixTime() - contract.m_iArrivedAt <= WORK_LIMIT)
			return false;

		reason = text;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Waves keep coming on the site's interval while the job is being worked.
	protected static void KeepWavesComing(SRZ_Contract contract)
	{
		int due = 1 + (int)((System.GetUnixTime() - contract.m_iArrivedAt) / contract.m_Site.GetWaveInterval());
		if (contract.m_iWavesDone < due)
			SpawnWave(contract);

		DrainQueue(contract);
	}

	//------------------------------------------------------------------------------------------------
	protected static int StepHunt(SRZ_Contract contract, IEntity player, bool awake, out string reason)
	{
		SRZ_ContractSite site = contract.m_Site;
		int failed;
		if (!CheckArrival(contract, player, awake, "Something big is moving out here. Get ready.", failed, reason))
			return failed;

		if (!contract.m_bAlphaSpawned)
		{
			if (OutOfTime(contract, "Nothing turned up. The contract is cancelled.", reason))
				return -1;

			if (!SpawnAlpha(contract))
				return 0;

			SpawnWave(contract);
		}

		DrainQueue(contract);

		if (!contract.m_Alpha)
		{
			reason = "The " + EnemyName(site.m_eAlpha) + " got away. The contract is lost.";
			return -1;
		}

		if (!IsAlive(contract.m_Alpha))
		{
			StartReturn(contract, "The " + EnemyName(site.m_eAlpha) + " is down. Get back to the vendor to get paid. Die on the way and it's lost.");
			return 0;
		}

		if (OutOfTime(contract, "The " + EnemyName(site.m_eAlpha) + " got away. The contract is lost.", reason))
			return -1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static int StepRecovery(SRZ_Contract contract, IEntity player, bool awake, out string reason)
	{
		SRZ_ContractSite site = contract.m_Site;
		int failed;
		if (!CheckArrival(contract, player, awake, "You're at " + site.GetDisplayName() + ". Find our crate and search it. Stay close to it.", failed, reason))
			return failed;

		if (!contract.m_Crate)
		{
			if (contract.m_fSearch > 0 || contract.m_bSearching)
			{
				reason = "The crate is gone. The contract is lost.";
				return -1;
			}

			if (!SpawnCrate(contract))
			{
				reason = "Couldn't find a spot for the crate. The contract is cancelled.";
				return -1;
			}
		}

		KeepWavesComing(contract);

		bool close = awake && vector.Distance(player.GetOrigin(), contract.m_Crate.GetOrigin()) <= SEARCH_RANGE;
		if (close)
		{
			if (!contract.m_bSearching)
			{
				contract.m_bSearching = true;
				Notify(contract.m_iPlayerId, "Searching the crate. Stay next to it.");
			}

			contract.m_fSearch += TICK;
			if (contract.m_fSearch >= Math.Max(site.m_fSearchSeconds, 5))
			{
				StartReturn(contract, "Got it. Get back to the vendor to get paid. Anything else in the crate is yours to take. Die on the way and it's lost.");
				return 0;
			}
		}
		else if (contract.m_bSearching)
		{
			contract.m_bSearching = false;
			Notify(contract.m_iPlayerId, "You stepped away from the crate. Get back to it to keep searching.");
		}

		if (OutOfTime(contract, "Took too long at " + site.GetDisplayName() + ". The contract is lost.", reason))
			return -1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static int StepAnomaly(SRZ_Contract contract, IEntity player, bool awake, out string reason)
	{
		SRZ_ContractSite site = contract.m_Site;
		int failed;
		if (!contract.m_bArrived)
		{
			if (!CheckArrival(contract, player, awake, "You've got the signal. It's somewhere in this field. Watch your step.", failed, reason))
				return failed;

			if (!PickTarget(contract))
			{
				reason = "Lost the signal. The contract is cancelled.";
				return -1;
			}
		}

		KeepWavesComing(contract);

		if (awake)
		{
			vector here = player.GetOrigin();
			vector target = contract.m_vTarget;
			float dist = vector.Distance(Vector(here[0], 0, here[2]), Vector(target[0], 0, target[2]));
			if (dist <= FOUND_RANGE && Math.AbsFloat(here[1] - target[1]) < 3)
			{
				StartReturn(contract, "Found it. Get back to the vendor to get paid. Die on the way and it's lost.");
				return 0;
			}

			int band = 0;
			if (dist <= 6)
				band = 3;
			else if (dist <= 15)
				band = 2;
			else if (dist <= site.m_fRadius * 1.5)
				band = 1;

			if (band != contract.m_iHintBand)
			{
				contract.m_iHintBand = band;
				switch (band)
				{
					case 3: Notify(contract.m_iPlayerId, "Signal is strong. It's right around here."); break;
					case 2: Notify(contract.m_iPlayerId, "Signal is getting stronger."); break;
					case 1: Notify(contract.m_iPlayerId, "Faint signal."); break;
					default: Notify(contract.m_iPlayerId, "Lost the signal. Go back into the field."); break;
				}
			}
		}

		if (OutOfTime(contract, "Took too long in the field. The contract is lost.", reason))
			return -1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static void StartReturn(SRZ_Contract contract, string text)
	{
		contract.m_iStage = STAGE_RETURN;
		contract.m_iReturnAt = System.GetUnixTime();
		contract.m_aQueue.Clear();
		Notify(contract.m_iPlayerId, text);
		Log(contract.m_sUid + " " + TypeName(contract.m_Site.m_eType) + " at " + contract.m_Site.GetDisplayName() + " done, heading back");
	}

	//------------------------------------------------------------------------------------------------
	protected static int StepReturn(SRZ_Contract contract, IEntity player, bool awake, out string reason)
	{
		if (System.GetUnixTime() - contract.m_iReturnAt > RETURN_LIMIT)
		{
			reason = "You never made it back. The contract is lost.";
			return -1;
		}

		vector vendorPos = contract.m_vVendorPos;
		if (contract.m_Vendor)
			vendorPos = contract.m_Vendor.GetOrigin();

		if (awake && vector.Distance(player.GetOrigin(), vendorPos) <= MAX_DISTANCE + 2)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SpawnAlpha(SRZ_Contract contract)
	{
		SRZ_ContractSite site = contract.m_Site;
		vector pos;
		if (!SpawnPoint(site, pos))
			return false;

		array<IEntity> spawned = {};
		SpawnPack(contract, EnemyPrefab(site.m_eAlpha), 1, pos, spawned);
		if (spawned.IsEmpty())
			return false;

		contract.m_Alpha = spawned[0];
		contract.m_bAlphaSpawned = true;
		Log(contract.m_sUid + " alpha " + EnemyName(site.m_eAlpha) + " spawned at " + site.GetDisplayName());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SpawnCrate(SRZ_Contract contract)
	{
		SRZ_ContractSite site = contract.m_Site;
		Resource resource = Resource.Load(CratePrefab(site.m_eLoot));
		if (!resource || !resource.IsValid())
			return false;

		vector center = site.GetOrigin();
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = Vector(center[0], GroundAt(center[0], center[2]), center[2]);

		contract.m_Crate = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		return contract.m_Crate != null;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool PickTarget(SRZ_Contract contract)
	{
		SRZ_ContractSite site = contract.m_Site;
		vector center = site.GetOrigin();
		BaseWorld world = GetGame().GetWorld();
		for (int attempt = 0; attempt < 20; attempt++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(0, site.m_fRadius * 0.8);
			float x = center[0] + Math.Cos(angle) * dist;
			float z = center[2] + Math.Sin(angle) * dist;
			float y = GroundAt(x, z);

			if (ChimeraWorldUtils.TryGetWaterSurfaceSimple(world, Vector(x, y - 0.5, z)))
				continue;

			contract.m_vTarget = Vector(x, y, z);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// Ground distance. Site height doesn't matter.
	static float Flat(vector a, vector b)
	{
		return vector.Distance(Vector(a[0], 0, a[2]), Vector(b[0], 0, b[2]));
	}

	//------------------------------------------------------------------------------------------------
	// Terrain, or a rock or floor right at terrain level. Never a roof or a treetop.
	protected static float GroundAt(float x, float z)
	{
		BaseWorld world = GetGame().GetWorld();
		float ground = world.GetSurfaceY(x, z);
		TraceParam trace = new TraceParam();
		trace.Start = Vector(x, ground + 2, z);
		trace.End = Vector(x, ground - 2, z);
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerPresets.Projectile;

		float hit = world.TraceMove(trace, null);
		if (hit < 1)
			ground = trace.Start[1] + (trace.End[1] - trace.Start[1]) * hit;

		return ground;
	}

	//------------------------------------------------------------------------------------------------
	protected static void End(string uid, bool success, string reason)
	{
		SRZ_Contract contract = s_mActive.Get(uid);
		if (!contract)
			return;

		s_mActive.Remove(uid);

		string siteName = "?";
		if (contract.m_Site)
			siteName = contract.m_Site.GetDisplayName();

		if (success)
		{
			int pay = contract.m_Site.m_iPay;
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(contract.m_iPlayerId);
			ARMST_PLAYER_STATS_COMPONENT stats;
			if (player)
				stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));

			if (stats && pay > 0)
			{
				stats.ModifyValue(pay, true);
				Notify(contract.m_iPlayerId, "Contract done. Paid " + pay.ToString() + " RUB.");
				GiveKey(contract, player);
				Log(uid + " finished " + TypeName(contract.m_Site.m_eType) + " at " + siteName + " for " + pay.ToString());
			}
			else
			{
				Log(uid + " finished " + TypeName(contract.m_Site.m_eType) + " at " + siteName + " but could not be paid");
			}

			contract.m_iEndAt = System.GetUnixTime() + 30;
			s_aEnding.Insert(contract);
			return;
		}

		SRZ_Jobs.SetReadyAt(CooldownKey(uid), System.GetUnixTime() + FAIL_COOLDOWN);
		if (reason != "disconnected")
			Notify(contract.m_iPlayerId, reason);

		string typeName = "contract";
		if (contract.m_Site)
			typeName = TypeName(contract.m_Site.m_eType);

		Log(uid + " failed " + typeName + " at " + siteName + ": " + reason);
		Cleanup(contract);
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName KeyPrefab(int tier)
	{
		switch (tier)
		{
			case 2: return "{3B7FFD4B0D298673}Prefabs/Items/Keys/Tier/SRZ_Key_T2.et";
			case 3: return "{0E8018E7EC40F2F5}Prefabs/Items/Keys/Tier/SRZ_Key_T3.et";
		}

		return "{072BBEF024816BBD}Prefabs/Items/Keys/Tier/SRZ_Key_T1.et";
	}

	//------------------------------------------------------------------------------------------------
	// Rolled once, only on a paid finish. Goes in the bag, or at the player's feet if the bag is full.
	protected static void GiveKey(SRZ_Contract contract, IEntity player)
	{
		SRZ_ContractSite site = contract.m_Site;
		if (!site || !player || Math.RandomFloat01() >= site.m_fKeyChance)
			return;

		int tier = Math.ClampInt(site.m_iKeyTier, 1, 3);
		if (tier < 3 && Math.RandomFloat01() < site.m_fKeyUpgradeChance)
			tier++;

		ResourceName key = KeyPrefab(tier);
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		bool stored = inventory && inventory.TrySpawnPrefabToStorage(key);
		if (!stored)
		{
			Resource resource = Resource.Load(key);
			if (!resource || !resource.IsValid())
				return;

			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = player.GetOrigin() + Vector(0, 0.3, 0);
			if (!GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params))
				return;
		}

		string where = "It's in your bag.";
		if (!stored)
			where = "Your bag was full, it's at your feet.";

		Notify(contract.m_iPlayerId, "They threw in a tier " + tier.ToString() + " crate key. " + where);
		Log(contract.m_sUid + " got a tier " + tier.ToString() + " key from " + TypeName(site.m_eType) + " at " + site.GetDisplayName());
	}

	//------------------------------------------------------------------------------------------------
	static void Drop(int playerId)
	{
		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!s_mActive.Get(uid))
		{
			Notify(playerId, "You don't have a contract.");
			return;
		}

		End(uid, false, "You dropped the contract. You can take a new one in 2 hours.");
	}

	//------------------------------------------------------------------------------------------------
	protected static string StatusText(SRZ_Contract contract)
	{
		SRZ_ContractSite site = contract.m_Site;
		if (contract.m_iStage == STAGE_RETURN)
			return "Done there. Get back to the vendor who gave you this. " + FormatWait(TimeLeft(RETURN_LIMIT, contract.m_iReturnAt)) + " left.";

		if (!contract.m_bArrived)
			return "Not there yet. " + FormatWait(TimeLeft(TRAVEL_LIMIT, contract.m_iStarted)) + " left to get there.";

		if (site.m_eType == SRZ_EContractType.HOLD)
			return "Held " + ((int)(contract.m_fHeld / 60)).ToString() + " of " + ((int)site.m_fHoldMinutes).ToString() + " minutes, wave " + contract.m_iWavesDone.ToString() + " of " + site.GetWaveCount().ToString() + ".";

		string text;
		switch (site.m_eType)
		{
			case SRZ_EContractType.HUNT: text = "The " + EnemyName(site.m_eAlpha) + " is still out there. "; break;
			case SRZ_EContractType.RECOVERY: text = "Search the crate. "; break;
			case SRZ_EContractType.ANOMALY: text = "Follow the signal. "; break;
		}

		return text + FormatWait(TimeLeft(WORK_LIMIT, contract.m_iArrivedAt)) + " left.";
	}

	//------------------------------------------------------------------------------------------------
	// For the PDA quest log. Only what Status already tells the player, plus the pay.
	static bool GetLogEntry(int playerId, out string title, out string desc, out vector pos)
	{
		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SRZ_Contract contract = s_mActive.Get(uid);
		if (!contract || !contract.m_Site)
			return false;

		SRZ_ContractSite site = contract.m_Site;
		title = "Contract: " + TypeName(site.m_eType) + " at " + site.GetDisplayName();

		desc = site.GetDisplayName() + ", grid " + Grid(site.GetOrigin()) + ".\n\n" + StatusText(contract);
		desc += "\n\nPay: " + site.m_iPay.ToString() + " RUB";
		if (site.m_fKeyChance > 0)
			desc += ", with a chance of a tier " + Math.ClampInt(site.m_iKeyTier, 1, 3).ToString() + " crate key";

		desc += ".";

		pos = site.GetOrigin();
		if (contract.m_iStage == STAGE_RETURN && contract.m_vVendorPos != vector.Zero)
			pos = contract.m_vVendorPos;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Status(int playerId)
	{
		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SRZ_Contract contract = s_mActive.Get(uid);
		if (contract && contract.m_Site)
		{
			SRZ_ContractSite site = contract.m_Site;
			Notify(playerId, TypeName(site.m_eType) + " at " + site.GetDisplayName() + ", grid " + Grid(site.GetOrigin()) + ". " + StatusText(contract));
			return;
		}

		int wait = SRZ_Jobs.GetReadyAt(CooldownKey(uid)) - System.GetUnixTime();
		if (wait > 0)
			Notify(playerId, "No contract. You can take a new one in " + FormatWait(wait) + ".");
		else
			Notify(playerId, "No contract. Ask your faction vendor for one.");
	}

	//------------------------------------------------------------------------------------------------
	protected static int TimeLeft(int limit, int since)
	{
		int left = limit - (System.GetUnixTime() - since);
		if (left < 0)
			return 0;

		return left;
	}

	//------------------------------------------------------------------------------------------------
	static void SetEnabled(bool enabled)
	{
		s_bEnabled = enabled;
		Log("contracts switched " + enabled.ToString());
		if (enabled)
			return;

		array<string> uids = {};
		foreach (string uid, SRZ_Contract contract : s_mActive)
			uids.Insert(uid);

		foreach (string cancelUid : uids)
			End(cancelUid, false, "Staff closed contracts. Yours was cancelled.");
	}

	//------------------------------------------------------------------------------------------------
	// Only what the contract spawned belongs to it. Anything else that ends up in its groups is removed.
	protected static void TrackMembers(SRZ_Contract contract)
	{
		foreach (IEntity groupEntity : contract.m_aGroups)
			StripExtras(SCR_AIGroup.Cast(groupEntity));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsAlive(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character || !character.GetCharacterController())
			return false;

		return character.GetCharacterController().GetLifeState() != ECharacterLifeState.DEAD;
	}

	//------------------------------------------------------------------------------------------------
	protected static int AliveCount(SRZ_Contract contract)
	{
		int count;
		foreach (IEntity body : contract.m_aBodies)
		{
			if (IsAlive(body))
				count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	protected static int TotalAlive()
	{
		int count;
		foreach (string uid, SRZ_Contract contract : s_mActive)
			count += AliveCount(contract);

		return count;
	}

	//------------------------------------------------------------------------------------------------
	static int SRZ_SpawnRoom(SRZ_Contract contract)
	{
		return Math.Min(MAX_AI_SITE - AliveCount(contract), MAX_AI_TOTAL - TotalAlive());
	}

	//------------------------------------------------------------------------------------------------
	protected static bool CanSpawn(SRZ_Contract contract)
	{
		return SRZ_SpawnRoom(contract) > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PlayersInArea(SRZ_Contract contract)
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);

		int count;
		float range = contract.m_Site.m_fRadius * 1.5;
		foreach (int playerId : players)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (player && IsAlive(player) && Flat(player.GetOrigin(), contract.m_Site.GetOrigin()) <= range)
				count++;
		}

		if (count < 1)
			return 1;

		return count;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SpawnWave(SRZ_Contract contract)
	{
		SRZ_ContractSite site = contract.m_Site;
		contract.m_iWavesDone++;

		array<ResourceName> wave = {};
		SRZ_ContractWave setup;
		if (site.m_aWaveSetup && !site.m_aWaveSetup.IsEmpty())
			setup = site.m_aWaveSetup[(contract.m_iWavesDone - 1) % site.m_aWaveSetup.Count()];

		if (setup && setup.m_aSpawns)
		{
			foreach (SRZ_ContractWaveSpawn spawn : setup.m_aSpawns)
			{
				if (!spawn)
					continue;

				ResourceName spawnPrefab = EnemyPrefab(spawn.m_eEnemy);
				if (spawnPrefab.IsEmpty())
					continue;

				int amount = Math.ClampInt(spawn.m_iCount, 1, 12);
				for (int c = 0; c < amount; c++)
					wave.Insert(spawnPrefab);
			}
		}

		if (wave.IsEmpty())
			DefaultWave(wave);

		// +30% per extra player in the area: sometimes two more of something already in this wave
		if (Math.RandomFloat01() < (PlayersInArea(contract) - 1) * 0.3)
		{
			ResourceName extra = wave.GetRandomElement();
			wave.Insert(extra);
			wave.Insert(extra);
		}

		// Nobody can farm a pile-up by not killing anything. Past this the extra just never comes.
		foreach (ResourceName queued : wave)
		{
			if (contract.m_aQueue.Count() >= MAX_AI_SITE)
				break;

			contract.m_aQueue.Insert(queued);
		}

		DrainQueue(contract);

		if (contract.m_iWavesDone > 1)
		{
			if (site.m_eType == SRZ_EContractType.HOLD)
				Notify(contract.m_iPlayerId, "More coming. Wave " + contract.m_iWavesDone.ToString() + " of " + site.GetWaveCount().ToString() + ".");
			else
				Notify(contract.m_iPlayerId, "More coming.");
		}

		Log(contract.m_sUid + " wave " + contract.m_iWavesDone.ToString() + " at " + site.GetDisplayName() + ", " + wave.Count().ToString() + " mutant(s)");
	}

	//------------------------------------------------------------------------------------------------
	// Mutants wait in line while the site or the server is at its AI cap.
	// The same mutant in a row comes in together from one spot.
	protected static void DrainQueue(SRZ_Contract contract)
	{
		int room = SRZ_SpawnRoom(contract);
		while (!contract.m_aQueue.IsEmpty() && room > 0)
		{
			ResourceName prefab = contract.m_aQueue[0];
			int amount = 1;
			while (amount < contract.m_aQueue.Count() && amount < room && contract.m_aQueue[amount] == prefab)
				amount++;

			for (int r = 0; r < amount; r++)
				contract.m_aQueue.RemoveOrdered(0);

			room -= amount;

			vector pos;
			if (SpawnPoint(contract.m_Site, pos))
				SpawnPack(contract, prefab, amount, pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	// One empty group, then exactly the asked number of mutants put into it.
	protected static void SpawnPack(SRZ_Contract contract, ResourceName prefab, int amount, vector pos, array<IEntity> spawned = null)
	{
		Resource unitResource = Resource.Load(prefab);
		if (!unitResource || !unitResource.IsValid())
			return;

		SCR_AIGroup group = SpawnCarrier(pos);
		if (!group)
			return;

		contract.m_aGroups.Insert(group);

		BaseWorld world = GetGame().GetWorld();
		for (int i = 0; i < amount; i++)
		{
			float x = pos[0] + Math.RandomFloat(-2, 2);
			float z = pos[2] + Math.RandomFloat(-2, 2);
			float y = Math.Max(world.GetSurfaceY(x, z), pos[1] - 0.5) + 0.3;

			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = Vector(x, y, z);

			IEntity unit = GetGame().SpawnEntityPrefab(unitResource, world, params);
			if (!unit)
				continue;

			contract.m_aBodies.Insert(unit);
			s_ContractBodies.Insert(unit);
			if (spawned)
				spawned.Insert(unit);
			group.AddAIEntityToGroup(unit);
		}

		// If the group brought its own members, they go. Only ours stay.
		GetGame().GetCallqueue().CallLater(StripExtras, 1000, false, group);
		GetGame().GetCallqueue().CallLater(StripExtras, 4000, false, group);

		Resource waypointResource = Resource.Load(WAYPOINT);
		if (!waypointResource || !waypointResource.IsValid())
			return;

		EntitySpawnParams wpParams = new EntitySpawnParams();
		wpParams.TransformMode = ETransformMode.WORLD;
		wpParams.Transform[3] = contract.m_Site.GetOrigin();

		AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(waypointResource, world, wpParams));
		if (!waypoint)
			return;

		waypoint.SetCompletionRadius(contract.m_Site.m_fRadius);
		group.AddWaypoint(waypoint);
		contract.m_aWaypoints.Insert(waypoint);
	}

	//------------------------------------------------------------------------------------------------
	protected static SCR_AIGroup SpawnCarrier(vector pos)
	{
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;

		array<ResourceName> carriers = {CARRIER, CARRIER_FALLBACK};
		foreach (ResourceName carrier : carriers)
		{
			Resource resource = Resource.Load(carrier);
			if (!resource || !resource.IsValid())
				continue;

			IEntity groupEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
			SCR_AIGroup group = SCR_AIGroup.Cast(groupEntity);
			if (group)
				return group;

			if (groupEntity)
				SCR_EntityHelper.DeleteEntityAndChildren(groupEntity);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void StripExtras(SCR_AIGroup group)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity member = agent.GetControlledEntity();
			if (member && !s_ContractBodies.Contains(member))
				SCR_EntityHelper.DeleteEntityAndChildren(member);
		}
	}

	//------------------------------------------------------------------------------------------------
	// A marker near the site wins. Otherwise a random spot on dry ground just outside the area.
	protected static bool SpawnPoint(SRZ_ContractSite site, out vector pos)
	{
		vector center = site.GetOrigin();

		array<SRZ_ContractSpawnMarker> markers = {};
		foreach (SRZ_ContractSpawnMarker marker : s_aMarkers)
		{
			if (marker && vector.Distance(marker.GetOrigin(), center) <= site.m_fRadius + 150 && ClosestSite(marker.GetOrigin()) == site)
				markers.Insert(marker);
		}

		if (!markers.IsEmpty())
		{
			pos = markers.GetRandomElement().GetOrigin();
			return true;
		}

		BaseWorld world = GetGame().GetWorld();
		for (int attempt = 0; attempt < 20; attempt++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = site.m_fRadius + Math.RandomFloat(25, 45);
			float x = center[0] + Math.Cos(angle) * dist;
			float z = center[2] + Math.Sin(angle) * dist;

			float ground = world.GetSurfaceY(x, z);
			TraceParam trace = new TraceParam();
			trace.Start = Vector(x, ground + 2, z);
			trace.End = Vector(x, ground - 2, z);
			trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			trace.LayerMask = EPhysicsLayerPresets.Projectile;

			float hit = world.TraceMove(trace, null);
			if (hit < 1)
				ground = trace.Start[1] + (trace.End[1] - trace.Start[1]) * hit;

			if (ChimeraWorldUtils.TryGetWaterSurfaceSimple(world, Vector(x, ground - 0.5, z)))
				continue;

			pos = Vector(x, ground + 0.5, z);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static SRZ_ContractSite ClosestSite(vector pos)
	{
		SRZ_ContractSite best;
		float bestDist = float.MAX;
		foreach (SRZ_ContractSite site : s_aSites)
		{
			if (!site)
				continue;

			float dist = vector.DistanceSq(site.GetOrigin(), pos);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = site;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	// Living contract enemies and their groups go now, waypoints a moment later. Dead bodies stay for the normal corpse cleanup.
	protected static void Cleanup(SRZ_Contract contract)
	{
		TrackMembers(contract);

		foreach (IEntity body : contract.m_aBodies)
		{
			if (body && IsAlive(body))
				SCR_EntityHelper.DeleteEntityAndChildren(body);
		}

		foreach (IEntity groupEntity : contract.m_aGroups)
		{
			if (groupEntity)
				SCR_EntityHelper.DeleteEntityAndChildren(groupEntity);
		}

		float later = System.GetUnixTime() + 3;
		foreach (IEntity waypoint : contract.m_aWaypoints)
		{
			if (!waypoint)
				continue;

			s_aTrash.Insert(waypoint);
			s_aTrashAt.Insert(later);
		}

		if (contract.m_Crate)
			SCR_EntityHelper.DeleteEntityAndChildren(contract.m_Crate);

		contract.m_Crate = null;
		contract.m_aGroups.Clear();
		contract.m_aWaypoints.Clear();
		contract.m_aQueue.Clear();
	}
}

modded class ARMST_USER_LOOT_BODY
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (pOwnerEntity && SRZ_Contracts.IsContractBody(pOwnerEntity.GetRootParent()))
			return;

		super.PerformAction(pOwnerEntity, pUserEntity);
	}
}

modded class ARMST_DIALOGS_COMPONENT
{
	static ARMST_DIALOGS_COMPONENT s_SRZContractDialog;

	//------------------------------------------------------------------------------------------------
	override void Armst_dialog_Missions(out string PlayerText, out string BotText)
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		ARMST_FACTION_LABEL faction = ARMST_FACTION_LABEL.FACTION_ALL;
		if (config)
			faction = SRZ_Contracts.VendorFaction(config.m_Actor);

		if (faction == ARMST_FACTION_LABEL.FACTION_ALL)
		{
			super.Armst_dialog_Missions(PlayerText, BotText);
			return;
		}

		PlayerText = "#armst_dialogs_missions";

		array<SRZ_ContractSite> board = SRZ_Contracts.Board(faction);
		if (board.IsEmpty())
		{
			BotText = "Nothing on the board right now.";
		}
		else
		{
			BotText = "Contracts. One at a time, take what you can handle.\n";
			foreach (int n, SRZ_ContractSite site : board)
				BotText += "\n" + (n + 1).ToString() + ". " + SRZ_Contracts.Describe(site) + "\n";

		}

		if (m_DialogUI)
			m_DialogUI.UpdateMissionButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void ButtonMissionByIndex(IEntity playerEntity, int missionIndex)
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		ARMST_FACTION_LABEL faction = ARMST_FACTION_LABEL.FACTION_ALL;
		if (config)
			faction = SRZ_Contracts.VendorFaction(config.m_Actor);

		if (faction == ARMST_FACTION_LABEL.FACTION_ALL)
		{
			super.ButtonMissionByIndex(playerEntity, missionIndex);
			return;
		}

		array<SRZ_ContractSite> board = SRZ_Contracts.Board(faction);
		if (missionIndex < 0 || missionIndex >= board.Count())
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!controller || !rpl)
			return;

		s_SRZContractDialog = this;
		if (m_DialogUI)
			m_DialogUI.SetTextDialogue("...");

		controller.SRZ_RequestContract(rpl.Id(), board[missionIndex].GetKey());
	}
}

modded class ARMST_DIALOGS_UI
{
	//------------------------------------------------------------------------------------------------
	override void UpdateMissionButtons()
	{
		ARMST_DIALOGS_CONFIG config = SRZ_Jobs.LoadConfig(m_DialogConfig);
		ARMST_FACTION_LABEL faction = ARMST_FACTION_LABEL.FACTION_ALL;
		if (config)
			faction = SRZ_Contracts.VendorFaction(config.m_Actor);

		if (faction == ARMST_FACTION_LABEL.FACTION_ALL)
		{
			super.UpdateMissionButtons();
			return;
		}

		m_CurrentMissions.Clear();
		m_IsInMissionMode = true;
		ResetButtons();

		array<SRZ_ContractSite> board = SRZ_Contracts.Board(faction);
		foreach (int n, SRZ_ContractSite site : board)
		{
			string buttonText = SRZ_Contracts.ButtonPrefix(site.m_eType) + site.GetDisplayName();
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

modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	void SRZ_RequestContract(RplId npcId, string siteKey)
	{
		if (Replication.IsServer())
		{
			SRZ_DoContract(npcId, siteKey);
			return;
		}

		Rpc(RpcAsk_SRZContract, npcId, siteKey);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZContract(RplId npcId, string siteKey)
	{
		SRZ_DoContract(npcId, siteKey);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoContract(RplId npcId, string siteKey)
	{
		string reply = SRZ_Contracts.TryStart(GetPlayerId(), GetMainEntity(), SCR_EntityHelper.RplIdToEntity(npcId), siteKey);
		if (!reply.IsEmpty())
			SRZ_ContractReply(reply);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZContractResult(string text)
	{
		SRZ_ShowContractResult(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ShowContractResult(string text)
	{
		ARMST_DIALOGS_COMPONENT dialog = ARMST_DIALOGS_COMPONENT.s_SRZContractDialog;
		if (dialog && dialog.m_DialogUI)
			dialog.m_DialogUI.SetTextDialogue(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ContractReply(string text)
	{
		if (GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
		{
			SRZ_ShowContractResult(text);
			return;
		}

		Rpc(RpcDo_SRZContractResult, text);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_SendContractCommand(string command)
	{
		if (Replication.IsServer())
		{
			SRZ_DoContractCommand(command);
			return;
		}

		Rpc(RpcAsk_SRZContractCommand, command);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZContractCommand(string command)
	{
		SRZ_DoContractCommand(command);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoContractCommand(string command)
	{
		int playerId = GetPlayerId();
		if (playerId <= 0)
			return;

		string text = command;
		text.ToLower();
		text.TrimInPlace();

		if (text.StartsWith(".contracts"))
		{
			PlayerManager pm = GetGame().GetPlayerManager();
			if (!pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR) && !pm.HasPlayerRole(playerId, EPlayerRole.GAME_MASTER))
			{
				SRZ_RPNet.SendToPlayer(playerId, "GM or Admin access required.");
				return;
			}

			if (text == ".contracts off")
			{
				SRZ_Contracts.SetEnabled(false);
				SRZ_RPNet.SendToPlayer(playerId, "Contracts are off. Running ones were cancelled.");
			}
			else if (text == ".contracts on")
			{
				SRZ_Contracts.SetEnabled(true);
				SRZ_RPNet.SendToPlayer(playerId, "Contracts are on.");
			}
			else
			{
				SRZ_RPNet.SendToPlayer(playerId, "Use .contracts on or .contracts off");
			}
			return;
		}

		if (text == ".contract drop")
		{
			SRZ_Contracts.Drop(playerId);
			return;
		}

		SRZ_Contracts.Status(playerId);
	}
}
