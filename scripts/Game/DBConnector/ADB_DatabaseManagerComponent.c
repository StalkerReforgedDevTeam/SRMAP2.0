class ADB_DatabaseConfigEntry : JsonApiStruct
{
    string configName;
	string domain;
    string uriStub;
    string apiKey;

    void ADB_DatabaseConfigEntry()
    {
        RegV("configName");
        RegV("domain");
        RegV("uriStub");
        RegV("apiKey");
    }
}

class ADB_DatabaseConfig : JsonApiStruct
{
    ref array<ref ADB_DatabaseConfigEntry> entries;

    void ADB_DatabaseConfig()
    {
        entries = new array<ref ADB_DatabaseConfigEntry>();
        RegV("entries");
    }
}

class ADB_SupabaseSelectArrayResponse<Class T>
{
	static bool Parse(string response, out array<ref T> o)
	{
		SCR_JsonLoadContext json = new SCR_JsonLoadContext();
		bool ok = json.ImportFromString(response);
		if (!ok)
			return false;
		
		json.ReadValue("", o);
		
		return true;
	}
}

[ComponentEditorProps(category: "ADB/Components/Gamemode", description: "A required gamemode component that allows creation of connectors specified through file configs")]
class ADB_DatabaseManagerComponentClass : SCR_BaseGameModeComponentClass {}

/**
 * Main component that simply allows hand out of connectors that are specified through config files. Doesn't actually
 * manage or own any of the connectors. Callees are expected to manage the lifetime of the connectors themselves.
 *
 * Sample config:
 *   {"entries":[{"configName":"default","domain":"domain","uriStub":"my_app","apiKey":"secret"}]}
 */
class ADB_DatabaseManagerComponent : SCR_BaseGameModeComponent
{
	private ref ADB_DatabaseConfig m_config;
	
	static ADB_DatabaseManagerComponent Get()
	{
		ChimeraGame game = GetGame();
		if (!game)
			return null;
		
		BaseGameMode gamemode = game.GetGameMode();
		if (!gamemode)
			return null;
		
		ADB_DatabaseManagerComponent dbManager = ADB_DatabaseManagerComponent.Cast(gamemode.FindComponent(ADB_DatabaseManagerComponent));
		
		return dbManager;
	}
	
	void ADB_DatabaseManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		// Load the config from the disk and keep it in memory
		m_config = new ADB_DatabaseConfig();
		
		bool checkForConfig = m_config.LoadFromFile("$profile:DatabaseConfig.json");
		if (!checkForConfig)
		{
			ADB_Logger.Error("No config available");
			return;
		}
		
		foreach (auto entry : m_config.entries)
		{
			ADB_Logger.Info("Loaded config " + entry.configName);
		}
	}
	
	/**
	 * Creates a new connector (but not connected, because it's just a wrapper for RESTful APIs) where `dbConfigName` is the name of the
	 * key in the config.
	 */
	bool CreateDatabaseConnector(string dbConfigName, out ADB_Supabase supabase)
	{
		supabase = null;
		
		ref ADB_DatabaseConfigEntry foundEntry = null;
		foreach (ADB_DatabaseConfigEntry entry : m_config.entries)
		{
			if (entry.configName == dbConfigName)
				foundEntry = entry;
		}
		
		if (!foundEntry)
			return false;
		
		supabase = new ADB_Supabase(foundEntry.domain, foundEntry.uriStub, foundEntry.apiKey);
		
		return true;
	}
}