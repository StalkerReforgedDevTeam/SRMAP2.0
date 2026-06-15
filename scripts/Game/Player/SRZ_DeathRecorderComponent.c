modded enum InstigatorType
{
    INSTIGATOR_WORLD,
    INSTIGATOR_PLAYER_STATS
}

modded enum EDamageType
{
    Starvation,
    Thirst
}

[ComponentEditorProps(category: "ADB/Components/Player", description: "")]
class ADB_PlayerDeathRecorderComponentClass : ScriptComponentClass {}

class ADB_PlayerDeathRecorderComponent : ScriptComponent
{
    protected EDamageType m_lastDamageType = EDamageType.TRUE;
    protected InstigatorType m_instigatorType = InstigatorType.INSTIGATOR_NONE;
    protected IEntity m_lastInstigatorEntity = null;
    protected int m_instigatorPlayerId = -1;
    protected string m_lastInstigatorName = "";
    protected string m_lastInstigatorFaction = "";
    protected int m_lastInstigatorBackendId = -1;

    //------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (!GetGame().InPlayMode())
            return;

        SetEventMask(owner, EntityEvent.INIT);
    }

    //------------------------------------------------------------------------------------------------
    // Call this when damage is received to record the instigator
    void RecordDamage(EDamageType damageType, IEntity instigator)
    {
        m_lastDamageType = damageType;
        m_lastInstigatorEntity = instigator;

        if (!instigator)
        {
            m_instigatorType = InstigatorType.INSTIGATOR_WORLD;
            m_instigatorPlayerId = -1;
            m_lastInstigatorName = "";
            m_lastInstigatorFaction = "";
            return;
        }

        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        int playerId = pm.GetPlayerIdFromControlledEntity(instigator);
        if (playerId > 0)
        {
            m_instigatorType = InstigatorType.INSTIGATOR_PLAYER;
            m_instigatorPlayerId = playerId;
            m_lastInstigatorName = pm.GetPlayerName(playerId);

            ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(instigator.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
            if (stats)
                m_lastInstigatorFaction = stats.GetFactionKey().ToString();
        }
        else
        {
            m_instigatorType = InstigatorType.INSTIGATOR_WORLD;
            m_instigatorPlayerId = -1;
            m_lastInstigatorName = "";
        }
    }

    //------------------------------------------------------------------------------------------------
    // Call this on player death to log it
    void OnPlayerDied(IEntity owner)
    {
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        int victimId = pm.GetPlayerIdFromControlledEntity(owner);
        string victimName = pm.GetPlayerName(victimId);

        ARMST_DEATH_LOGGER.LogDEATH(
            victimName,
            Helpers.GetPlayerGUID(victimId),
            m_lastInstigatorName,
            Helpers.GetPlayerGUID(m_instigatorPlayerId)
        );
    }

    //------------------------------------------------------------------------------------------------
    EDamageType GetLastDamageType() { return m_lastDamageType; }
    InstigatorType GetInstigatorType() { return m_instigatorType; }
    IEntity GetLastInstigatorEntity() { return m_lastInstigatorEntity; }
    int GetInstigatorPlayerId() { return m_instigatorPlayerId; }
    string GetLastInstigatorName() { return m_lastInstigatorName; }
    string GetLastInstigatorFaction() { return m_lastInstigatorFaction; }
}