// Component that manages RP names - stores ONLY the RP name in ARMST (not username)
class SRZ_RPNameCharacterComponentClass : ScriptComponentClass
{
}

class SRZ_RPNameCharacterComponent : ScriptComponent
{
	protected int m_PlayerId = -1;
	protected bool m_HasInitialized = false;
	protected bool m_HasShownWelcome = false;
	
	// --------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!Replication.IsServer())
			return;
		
		GetGame().GetCallqueue().CallLater(Initialize, 100, false, owner);
	}
	
	// --------------------------------------------------------------------------------------------
	protected void Initialize(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_HasInitialized)
			return;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
		{
			GetGame().GetCallqueue().CallLater(Initialize, 500, false, owner);
			return;
		}
		
		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
			if (pc && pc.GetMainEntity() == owner)
			{
				m_PlayerId = playerId;
				m_HasInitialized = true;
				
				Print(string.Format("[SRZ_RP] Character component initialized for player %1", playerId), LogLevel.NORMAL);
				
				CheckOrGenerateName();
				return;
			}
		}
		
		GetGame().GetCallqueue().CallLater(Initialize, 500, false, owner);
	}
	
	// --------------------------------------------------------------------------------------------
	protected void CheckOrGenerateName()
	{
		if (!Replication.IsServer())
			return;
		
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (!profileMgr)
		{
			Print("[SRZ_RP] Profile manager not available", LogLevel.ERROR);
			return;
		}
		
		// Check if player has a saved name in their profile
		string savedName = profileMgr.GetNameForPlayer(m_PlayerId);
		
		Print(string.Format("[SRZ_RP] Checking profile for player %1 - Saved name: '%2'", 
			m_PlayerId, savedName), LogLevel.NORMAL);
		
		if (savedName && !savedName.IsEmpty())
		{
			// Player has a saved name - apply it
			Print(string.Format("[SRZ_RP] Player %1 has saved name: %2", m_PlayerId, savedName), LogLevel.NORMAL);
			ApplyNameToCharacter(savedName);
			ShowWelcomeMessage(savedName, true);
		}
		else
		{
			// No saved name - generate new one
			Print(string.Format("[SRZ_RP] No saved name for player %1, generating new", m_PlayerId), LogLevel.NORMAL);
			GenerateAndApplyName();
		}
	}
	
	// --------------------------------------------------------------------------------------------
	protected void GenerateAndApplyName()
	{
		if (!Replication.IsServer())
			return;
		
		if (m_PlayerId <= 0)
			return;
		
		SRZ_RPNameManager mgr = SRZ_RPNameManager.GetInstance();
		if (!mgr)
		{
			Print("[SRZ_RP] Name manager not available", LogLevel.ERROR);
			return;
		}
		
		string newName = mgr.GenerateRandomName(m_PlayerId);
		Print(string.Format("[SRZ_RP] Generated new name for player %1: %2", m_PlayerId, newName), LogLevel.NORMAL);
		
		// Store in profile manager
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (profileMgr)
		{
			profileMgr.SetNameForPlayer(m_PlayerId, newName);
		}
		
		// Apply to current character
		ApplyNameToCharacter(newName);
		
		ShowWelcomeMessage(newName, false);
	}
	
	// --------------------------------------------------------------------------------------------
	protected void ApplyNameToCharacter(string rpName)
	{
		if (!Replication.IsServer())
			return;
		
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		// Set ONLY the RP name in ARMST (not username + rpname)
		// The nametag mod will handle adding the username for GM display
		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(
			owner.FindComponent(ARMST_PLAYER_STATS_COMPONENT)
		);
		
		if (stats)
		{
			stats.ArmstPlayerSetName(rpName);
			Print(string.Format("[SRZ_RP] Set ARMST name to: '%1' for player %2", rpName, m_PlayerId), LogLevel.NORMAL);
			
			// Verify
			string verifyName = stats.ArmstArmstPlayerGetName();
			Print(string.Format("[SRZ_RP] Verified ARMST name: '%1'", verifyName), LogLevel.NORMAL);
		}
	}
	
	// --------------------------------------------------------------------------------------------
	protected void ShowWelcomeMessage(string rpName, bool isReturning)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_HasShownWelcome || m_PlayerId <= 0 || !rpName || rpName.IsEmpty())
			return;
		
		m_HasShownWelcome = true;
		
		if (isReturning)
		{
			SRZ_RPNet.SendToPlayer(m_PlayerId, string.Format("Welcome back, %1.", rpName));
		}
		else
		{
			SRZ_RPNet.SendToPlayer(m_PlayerId, string.Format("You awaken in the Zone as %1.", rpName));
		}
	}
	
	// --------------------------------------------------------------------------------------------
	void UpdateRPName(string newName)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_PlayerId <= 0)
			return;
		
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (profileMgr)
		{
			profileMgr.SetNameForPlayer(m_PlayerId, newName);
		}
		
		ApplyNameToCharacter(newName);
		
		Print(string.Format("[SRZ_RP] Updated name to '%1' for player %2", newName, m_PlayerId), LogLevel.NORMAL);
	}
	
	// --------------------------------------------------------------------------------------------
	string GetCurrentRPName()
	{
		SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
		if (!profileMgr)
			return "";
		
		return profileMgr.GetNameForPlayer(m_PlayerId);
	}
	
	// --------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_PlayerId;
	}
}