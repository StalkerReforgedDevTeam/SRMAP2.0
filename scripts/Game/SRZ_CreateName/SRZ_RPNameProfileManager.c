// Profile-based RP name persistence - stores per-player profile in a simple text file
// This is more reliable than entity-based persistence for player data

class SRZ_RPNameProfileManager : Managed
{
	protected static ref SRZ_RPNameProfileManager s_Instance;
	protected ref map<string, string> m_PlayerNames = new map<string, string>(); // PlayerUID -> Name
	
	//------------------------------------------------------------------------------------------------
	static SRZ_RPNameProfileManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new SRZ_RPNameProfileManager();
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void SRZ_RPNameProfileManager()
	{
		if (Replication.IsServer())
		{
			Print("[SRZ_RP Profile] Profile manager initialized", LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Get player's UID (Steam ID or BI ID)
	protected string GetPlayerUID(int playerId)
	{
		BackendApi api = GetGame().GetBackendApi();
		if (!api)
			return "";
		
		return api.GetPlayerIdentityId(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	// Get name for a player (by player ID)
	string GetNameForPlayer(int playerId)
	{
		string uid = GetPlayerUID(playerId);
		if (uid.IsEmpty())
			return "";
		
		string name;
		if (m_PlayerNames.Find(uid, name))
		{
			Print(string.Format("[SRZ_RP Profile] Found saved name for player %1 (UID: %2): %3", playerId, uid, name), LogLevel.NORMAL);
			return name;
		}
		
		return "";
	}
	
	//------------------------------------------------------------------------------------------------
	// Set name for a player (by player ID)
	void SetNameForPlayer(int playerId, string name)
	{
		string uid = GetPlayerUID(playerId);
		if (uid.IsEmpty())
		{
			Print(string.Format("[SRZ_RP Profile] Cannot save name - no UID for player %1", playerId), LogLevel.ERROR);
			return;
		}
		
		m_PlayerNames.Set(uid, name);
		Print(string.Format("[SRZ_RP Profile] Saved name for player %1 (UID: %2): %3", playerId, uid, name), LogLevel.NORMAL);
		
		// Trigger save
		SaveToFile();
	}
	
	//------------------------------------------------------------------------------------------------
	// Save to text file (simple format: UID|Name per line)
	protected void SaveToFile()
	{
		if (!Replication.IsServer())
			return;
		
		string filePath = "$profile:SRZ_RPNames.txt";
		FileHandle file = FileIO.OpenFile(filePath, FileMode.WRITE);
		
		if (!file)
		{
			Print(string.Format("[SRZ_RP Profile] Failed to open file for writing: %1", filePath), LogLevel.ERROR);
			return;
		}
		
		// Write each player name as "UID|Name"
		for (int i = 0; i < m_PlayerNames.Count(); i++)
		{
			string uid = m_PlayerNames.GetKey(i);
			string name = m_PlayerNames.GetElement(i);
			string line = string.Format("%1|%2", uid, name);
			file.WriteLine(line);
		}
		
		file.Close();
		
		Print(string.Format("[SRZ_RP Profile] Saved %1 player names to %2", m_PlayerNames.Count(), filePath), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	// Load from text file
	void LoadFromFile()
	{
		if (!Replication.IsServer())
			return;
		
		string filePath = "$profile:SRZ_RPNames.txt";
		FileHandle file = FileIO.OpenFile(filePath, FileMode.READ);
		
		if (!file)
		{
			Print("[SRZ_RP Profile] No saved names file found, starting fresh", LogLevel.NORMAL);
			return;
		}
		
		m_PlayerNames.Clear();
		
		// Read each line
		string line;
		while (file.ReadLine(line) > 0)
		{
			line.Trim();
			if (line.IsEmpty())
				continue;
			
			// Parse "UID|Name" format
			int separatorIndex = line.IndexOf("|");
			if (separatorIndex < 0)
				continue;
			
			string uid = line.Substring(0, separatorIndex);
			string name = line.Substring(separatorIndex + 1, line.Length() - separatorIndex - 1);
			
			uid.Trim();
			name.Trim();
			
			if (!uid.IsEmpty() && !name.IsEmpty())
			{
				m_PlayerNames.Set(uid, name);
			}
		}
		
		file.Close();
		
		Print(string.Format("[SRZ_RP Profile] Loaded %1 player names from %2", m_PlayerNames.Count(), filePath), LogLevel.NORMAL);
	}
}

// Auto-initialize on server start
modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();
		
		if (Replication.IsServer())
		{
			// Load saved names
			SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
			profileMgr.LoadFromFile();
			
			Print("[SRZ_RP] Profile manager initialized and loaded", LogLevel.NORMAL);
		}
	}
}