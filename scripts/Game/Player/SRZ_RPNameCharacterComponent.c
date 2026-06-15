class SRZ_RPNameCharacterComponentClass : ScriptComponentClass
{
}

class SRZ_RPNameCharacterComponent : ScriptComponent
{
    protected int m_PlayerId = -1;
    protected bool m_HasInitialized = false;
    protected bool m_HasShownWelcome = false;
    protected string m_CurrentRPName = "";

    //------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (!Replication.IsServer())
            return;

        GetGame().GetCallqueue().CallLater(Initialize, 100, false, owner);
    }

    //------------------------------------------------------------------------------------------------
    protected void Initialize(IEntity owner)
    {
        if (!Replication.IsServer())
            return;

        if (m_HasInitialized)
            return;

        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        m_PlayerId = pm.GetPlayerIdFromControlledEntity(owner);
        if (m_PlayerId <= 0)
            return;

        SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();
        if (!profileMgr)
            return;

        string savedName = profileMgr.GetNameForPlayer(m_PlayerId);

        if (savedName.IsEmpty())
        {
            SRZ_RPNameManager nameMgr = SRZ_RPNameManager.GetInstance();
            if (!nameMgr)
                return;

            string newName = nameMgr.GenerateRandomName(m_PlayerId);
            profileMgr.SetNameForPlayer(m_PlayerId, newName);
            m_CurrentRPName = newName;
        }
        else
        {
            m_CurrentRPName = savedName;
        }

        m_HasInitialized = true;

        if (!m_HasShownWelcome)
        {
            m_HasShownWelcome = true;
            SRZ_RPNet.SendToPlayer(m_PlayerId, string.Format("Your RP name: %1", m_CurrentRPName));
        }
    }

    //------------------------------------------------------------------------------------------------
    string GetCurrentRPName()
    {
        return m_CurrentRPName;
    }

    //------------------------------------------------------------------------------------------------
    void UpdateRPName(string newName)
    {
        m_CurrentRPName = newName;
    }
}