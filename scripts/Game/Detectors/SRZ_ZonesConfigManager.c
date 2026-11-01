// Scripts/Game/SRZ_ZonesConfigManager.c
// Simple JSON config manager for SR Zones (Discord alerts etc.)

const string SRZ_ZONES_LOG_PREFIX = "[SR Zones]";

class SRZ_ZonesConfigManager
{
	// --- Paths / filenames ---
	static const string CONFIG_DIR_PATH   = "$profile:/SR_Zones/";
	static const string CONFIG_FILE_NAME  = "sr_zones_config.json";
	static const string CONFIG_FILE_PATH  = CONFIG_DIR_PATH + CONFIG_FILE_NAME;
	static const string README_FILE_NAME  = "SR_Zones_README.txt";
	static const string README_FILE_PATH  = CONFIG_DIR_PATH + README_FILE_NAME;

	// --- State ---
	private static bool s_bDefaultsEnsured = false;
	private static ref map<string, string> s_mConfigCache;
	private static bool s_bCacheLoaded = false;

	//------------------------------------------------------------------------------------------------
	// Called once on server to ensure directory, JSON + README exist and contain default keys.
	// Call this from your GameMode / zone entity (server only).
	//------------------------------------------------------------------------------------------------
	static void EnsureDefaults()
	{
		// Only care about server
		if (!Replication.IsServer())
			return;

		if (s_bDefaultsEnsured)
			return;
		s_bDefaultsEnsured = true;

		Print(string.Format("%1 Ensuring SR_Zones config & README...", SRZ_ZONES_LOG_PREFIX), LogLevel.NORMAL);

		if (!EnsureDirectoryExists(CONFIG_DIR_PATH))
			return;

		// Make sure we at least tried to load once
		if (!s_bCacheLoaded)
			LoadConfigCache();

		map<string, string> defaultValues = DefineDefaults();

		bool configWasChanged = false;

		if (!s_mConfigCache)
			s_mConfigCache = new map<string, string>();

		foreach (string key, string defVal : defaultValues)
		{
			if (!s_mConfigCache.Contains(key))
			{
				Print(string.Format("%1 Adding missing config key '%2' with default '%3'",
					SRZ_ZONES_LOG_PREFIX, key, defVal), LogLevel.NORMAL);
				s_mConfigCache.Insert(key, defVal);
				configWasChanged = true;
			}
		}

		if (configWasChanged)
		{
			if (!SaveConfigCache())
				Print(string.Format("%1 ERROR: Failed to save updated SR_Zones config!", SRZ_ZONES_LOG_PREFIX), LogLevel.ERROR);
		}

		EnsureReadmeFileExists(configWasChanged);

		Print(string.Format("%1 SR_Zones config/README check complete.", SRZ_ZONES_LOG_PREFIX), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Default values – single source of truth for keys
	//------------------------------------------------------------------------------------------------
	private static map<string, string> DefineDefaults()
	{
		map<string, string> defaults = new map<string, string>();

		// Whether zone alerts are active at all
		defaults.Insert("m_bEnableZoneAlerts", "true");
		// ARMST_FACTION_LABEL enum id for mutants (ignored by the alert system).
		// NOTE: this must match the key name read in SRZ_DiscordZoneEntity.OnActivate().
		defaults.Insert("m_sMutantsFactionId", "4");
		// Discord webhook URL for zone alerts – server owner sets this
		defaults.Insert("m_sZoneAlertWebhookURL", "");

		return defaults;
	}

	//------------------------------------------------------------------------------------------------
	private static bool EnsureDirectoryExists(string dirPath)
	{
		if (!FileIO.FileExists(dirPath))
		{
			Print(string.Format("%1 Creating directory: %2", SRZ_ZONES_LOG_PREFIX, dirPath), LogLevel.NORMAL);
			if (!FileIO.MakeDirectory(dirPath))
			{
				Print(string.Format("%1 ERROR: FileIO.MakeDirectory failed for %2", SRZ_ZONES_LOG_PREFIX, dirPath), LogLevel.ERROR);
				return false;
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	private static void LoadConfigCache()
	{
		s_mConfigCache = new map<string, string>();
		s_bCacheLoaded = true;

		if (!FileIO.FileExists(CONFIG_FILE_PATH))
		{
			Print(string.Format("%1 Config file not found (%2). Using empty cache.",
				SRZ_ZONES_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.WARNING);
			return;
		}

		SCR_JsonLoadContext jsonContext = new SCR_JsonLoadContext();
		if (!jsonContext.LoadFromFile(CONFIG_FILE_PATH))
		{
			Print(string.Format("%1 ERROR: Failed to parse config file %2. Using empty cache.",
				SRZ_ZONES_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.ERROR);
			return;
		}

		map<string, string> defaults = DefineDefaults();
		string valueFromFile;
		int loadedCount = 0;

		foreach (string key, auto _ : defaults)
		{
			if (jsonContext.ReadValue(key, valueFromFile))
			{
				s_mConfigCache.Insert(key, valueFromFile);
				loadedCount++;
			}
		}

		Print(string.Format("%1 Loaded %2 keys from %3 into SR_Zones config cache.",
			SRZ_ZONES_LOG_PREFIX, loadedCount, CONFIG_FILE_PATH), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	private static bool SaveConfigCache()
	{
		if (!s_mConfigCache)
		{
			Print(string.Format("%1 ERROR: Cannot save config cache, it's null.", SRZ_ZONES_LOG_PREFIX), LogLevel.ERROR);
			return false;
		}

		ContainerSerializationSaveContext writer();
		PrettyJsonSaveContainer jsonContainer = new PrettyJsonSaveContainer();
		jsonContainer.SetMaxDecimalPlaces(3);
		writer.SetContainer(jsonContainer);

		int keysWritten = 0;
		foreach (string key, string val : s_mConfigCache)
		{
			writer.WriteValue(key, val);
			keysWritten++;
		}

		Print(string.Format("%1 Saving SR_Zones config (%2 keys) to %3",
			SRZ_ZONES_LOG_PREFIX, keysWritten, CONFIG_FILE_PATH), LogLevel.NORMAL);

		if (!jsonContainer.SaveToFile(CONFIG_FILE_PATH))
		{
			Print(string.Format("%1 ERROR: Failed to save SR_Zones config to %2",
				SRZ_ZONES_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string GetStringValue(string keyName, string defaultValue = "")
	{
		if (!s_bCacheLoaded)
			LoadConfigCache();

		if (!s_mConfigCache)
			return defaultValue;

		string v;
		if (s_mConfigCache.Find(keyName, v))
			return v;

		return defaultValue;
	}

	//------------------------------------------------------------------------------------------------
	static bool GetBoolValue(string keyName, bool defaultValue = false)
	{
		string s = GetStringValue(keyName, "");
		if (s == "")
			return defaultValue;

		s.ToLower();
		return (s == "true" || s == "1");
	}

	//------------------------------------------------------------------------------------------------
	private static void EnsureReadmeFileExists(bool forceRegenerate = false)
	{
		if (!FileIO.FileExists(README_FILE_PATH) && !forceRegenerate)
		{
			// fall through to create
		}
		else if (!forceRegenerate)
		{
			return; // already exists and nothing changed
		}

		FileHandle fh = FileIO.OpenFile(README_FILE_PATH, FileMode.WRITE);
		if (!fh)
		{
			Print(string.Format("%1 ERROR: Failed to open README file %2 for writing.",
				SRZ_ZONES_LOG_PREFIX, README_FILE_PATH), LogLevel.ERROR);
			return;
		}

		fh.Write("--- SR Zones Configuration ---\n\n");
		fh.Write("Config file: sr_zones_config.json (same directory as this README).\n\n");
		fh.Write("Keys:\n");
		fh.Write("  m_bEnableZoneAlerts       : \"true\" or \"false\" - master enable for Discord alerts.\n");
		fh.Write("  m_sMutantsFactionId       : ARMST faction id (as shown by GetFactionKey()) that is ignored by alerts.\n");
		fh.Write("  m_sZoneAlertWebhookURL    : Full Discord webhook URL used for zone alerts.\n\n");
		fh.Write("This file and the JSON are auto-generated by the SR_Zones mod on first server run.\n");

		fh.Close();
	}
}