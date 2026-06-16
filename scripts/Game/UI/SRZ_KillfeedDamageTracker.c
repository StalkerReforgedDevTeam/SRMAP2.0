class SRZ_DamageEntry
{
    string killerName;
    string weaponName;
    string damagePart;
    float damageAmount;
    EDamageType damageType;
    bool isHeadshot;
    int timestamp;

    void SRZ_DamageEntry(string killer, string weapon, string part,
                         float amount, EDamageType type, bool headshot)
    {
        killerName   = killer;
        weaponName   = weapon;
        damagePart   = part;
        damageAmount = amount;
        damageType   = type;
        isHeadshot   = headshot;
        timestamp    = System.GetTickCount();
    }
}

class SRZ_KillfeedDamageTracker
{
    static const int MAX_ENTRIES = 5;
    static const int MAX_AGE_MS  = 30000;

    protected static ref map<int, ref array<ref SRZ_DamageEntry>> s_mPlayerLogs =
        new map<int, ref array<ref SRZ_DamageEntry>>();

    //------------------------------------------------------------------------------------------------
    static void RecordHit(int victimPlayerId, string killerName, string weaponName,
                          string hitPart, float damage, EDamageType dmgType, bool isHeadshot)
    {
        if (!s_mPlayerLogs.Contains(victimPlayerId))
            s_mPlayerLogs.Insert(victimPlayerId, new array<ref SRZ_DamageEntry>());

        array<ref SRZ_DamageEntry> log = s_mPlayerLogs.Get(victimPlayerId);

        int now = System.GetTickCount();
        for (int i = log.Count() - 1; i >= 0; i--)
        {
            if ((now - log[i].timestamp) > MAX_AGE_MS)
                log.Remove(i);
        }

        while (log.Count() >= MAX_ENTRIES)
            log.RemoveOrdered(0);

        log.Insert(new SRZ_DamageEntry(killerName, weaponName, hitPart, damage, dmgType, isHeadshot));
    }

    //------------------------------------------------------------------------------------------------
    static array<ref SRZ_DamageEntry> GetAndClearLog(int victimPlayerId)
    {
        if (!s_mPlayerLogs.Contains(victimPlayerId))
            return new array<ref SRZ_DamageEntry>();

        array<ref SRZ_DamageEntry> log = s_mPlayerLogs.Get(victimPlayerId);
        s_mPlayerLogs.Remove(victimPlayerId);
        return log;
    }
}