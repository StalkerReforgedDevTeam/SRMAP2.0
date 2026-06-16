const string SRZ_KILLFEED_LOG_PREFIX = "[SRZ_Killfeed]";
class SRZ_KillfeedConfigManager
{
    static const string CONFIG_DIR_PATH  = "$profile:/SR_Killfeed/";
    static const string CONFIG_FILE_NAME = "sr_killfeed_config.json";
    static const string CONFIG_FILE_PATH = CONFIG_DIR_PATH + CONFIG_FILE_NAME;
    static const string README_FILE_NAME = "SR_Killfeed_README.txt";
    static const string README_FILE_PATH = CONFIG_DIR_PATH + README_FILE_NAME;

    private static bool s_bDefaultsEnsured = false;
    private static ref map<string, string> s_mConfigCache;
    private static bool s_bCacheLoaded = false;

    //------------------------------------------------------------------------------------------------
    static void EnsureDefaults()
    {
        if (!Replication.IsServer())
            return;

        if (s_bDefaultsEnsured)
            return;

        s_bDefaultsEnsured = true;
        Print(string.Format("%1 Ensuring SR_Killfeed config & README...", SRZ_KILLFEED_LOG_PREFIX), LogLevel.NORMAL);

        if (!EnsureDirectoryExists(CONFIG_DIR_PATH))
            return;

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
                s_mConfigCache.Insert(key, defVal);
                configWasChanged = true;
            }
        }

        if (configWasChanged)
        {
            if (!SaveConfigCache())
                Print(string.Format("%1 ERROR: Failed to save updated SR_Killfeed config!", SRZ_KILLFEED_LOG_PREFIX), LogLevel.ERROR);
        }

        EnsureReadmeFileExists(configWasChanged);
        Print(string.Format("%1 SR_Killfeed config/README check complete.", SRZ_KILLFEED_LOG_PREFIX), LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    private static map<string, string> DefineDefaults()
    {
        map<string, string> defaults = new map<string, string>();
        defaults.Insert("m_bEnableKillfeedWebhook", "true");
        defaults.Insert("m_sKillfeedWebhookURL", "");
        defaults.Insert("m_sPDALogWebhookURL", "");
        return defaults;
    }

    //------------------------------------------------------------------------------------------------
    private static bool EnsureDirectoryExists(string dirPath)
    {
        if (!FileIO.FileExists(dirPath))
        {
            if (!FileIO.MakeDirectory(dirPath))
            {
                Print(string.Format("%1 ERROR: FileIO.MakeDirectory failed for %2", SRZ_KILLFEED_LOG_PREFIX, dirPath), LogLevel.ERROR);
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
            Print(string.Format("%1 Config file not found (%2). Using empty cache.", SRZ_KILLFEED_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.WARNING);
            return;
        }

        SCR_JsonLoadContext jsonContext = new SCR_JsonLoadContext();
        if (!jsonContext.LoadFromFile(CONFIG_FILE_PATH))
        {
            Print(string.Format("%1 ERROR: Failed to parse config file %2.", SRZ_KILLFEED_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.ERROR);
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

        Print(string.Format("%1 Loaded %2 keys from %3.", SRZ_KILLFEED_LOG_PREFIX, loadedCount, CONFIG_FILE_PATH), LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    private static bool SaveConfigCache()
    {
        if (!s_mConfigCache)
            return false;

        ContainerSerializationSaveContext writer();
        PrettyJsonSaveContainer jsonContainer = new PrettyJsonSaveContainer();
        jsonContainer.SetMaxDecimalPlaces(3);
        writer.SetContainer(jsonContainer);

        foreach (string key, string val : s_mConfigCache)
            writer.WriteValue(key, val);

        if (!jsonContainer.SaveToFile(CONFIG_FILE_PATH))
        {
            Print(string.Format("%1 ERROR: Failed to save SR_Killfeed config to %2", SRZ_KILLFEED_LOG_PREFIX, CONFIG_FILE_PATH), LogLevel.ERROR);
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
        if (FileIO.FileExists(README_FILE_PATH) && !forceRegenerate)
            return;

        FileHandle fh = FileIO.OpenFile(README_FILE_PATH, FileMode.WRITE);
        if (!fh)
            return;

        fh.Write("--- SR Killfeed Configuration ---\n\n");
        fh.Write("Config file: sr_killfeed_config.json (same directory as this README).\n\n");
        fh.Write("Keys:\n");
        fh.Write("  m_bEnableKillfeedWebhook : \"true\" or \"false\" - master enable for Discord killfeed webhooks.\n");
        fh.Write("  m_sKillfeedWebhookURL    : Full Discord webhook URL used for killfeed messages.\n");
        fh.Write("  m_sPDALogWebhookURL      : Full Discord webhook URL used for PDA message logs.\n\n");
        fh.Write("This file and the JSON are auto-generated by the SR_Killfeed mod on first server run.\n");
        fh.Close();
    }
}