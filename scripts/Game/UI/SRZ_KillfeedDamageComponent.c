class SRZ_KillfeedDamageComponentClass : ScriptComponentClass {}

class SRZ_KillfeedDamageComponent : ScriptComponent
{
    protected int m_PlayerId = -1;

    //------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (!Replication.IsServer())
            return;

        SetEventMask(owner, EntityEvent.INIT);
    }

    //------------------------------------------------------------------------------------------------
    override void EOnInit(IEntity owner)
    {
        if (!Replication.IsServer())
            return;

        GetGame().GetCallqueue().CallLater(ResolvePlayerId, 500, false);

        SCR_CharacterDamageManagerComponent dmgMgr = SCR_CharacterDamageManagerComponent.Cast(
            owner.FindComponent(SCR_CharacterDamageManagerComponent)
        );
        if (dmgMgr)
            dmgMgr.GetOnDamage().Insert(OnDamageReceived);
    }

    //------------------------------------------------------------------------------------------------
    protected void ResolvePlayerId()
    {
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        m_PlayerId = pm.GetPlayerIdFromControlledEntity(GetOwner());
    }

    //------------------------------------------------------------------------------------------------
    protected string GetWeaponName(IEntity killerEntity)
    {
        if (!killerEntity)
            return "";

        BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(
            killerEntity.FindComponent(BaseWeaponManagerComponent)
        );
        if (!weaponManager)
            return "";

        BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
        if (!currentWeapon)
            return "";

        IEntity heldItem = currentWeapon.GetOwner();
        if (!heldItem)
            return "";

        EntityPrefabData prefabData = heldItem.GetPrefabData();
        if (prefabData)
        {
            string prefabName = FilePath.StripExtension(FilePath.StripPath(prefabData.GetPrefabName()));
            prefabName.Replace("_", " ");
            return prefabName;
        }

        return "";
    }

    //------------------------------------------------------------------------------------------------
    protected void OnDamageReceived(BaseDamageContext damageContext)
    {
        if (!Replication.IsServer())
            return;

        if (m_PlayerId <= 0)
            ResolvePlayerId();

        if (m_PlayerId <= 0)
            return;

        if (!damageContext)
            return;

        if (damageContext.damageValue <= 0)
            return;

        // Hit body part
        string hitPart = "Body";
        bool isHeadshot = false;
        if (damageContext.struckHitZone)
        {
            string zoneName = damageContext.struckHitZone.GetName();
            zoneName.ToLower();

            if (zoneName.Contains("head"))
            {
                hitPart = "Head";
                isHeadshot = true;
            }
            else if (zoneName.Contains("chest") || zoneName.Contains("torso"))
                hitPart = "Chest";
            else if (zoneName.Contains("arm") || zoneName.Contains("hand") || zoneName.Contains("forearm"))
                hitPart = "Arm";
            else if (zoneName.Contains("leg") || zoneName.Contains("foot") || zoneName.Contains("thigh") || zoneName.Contains("calf"))
                hitPart = "Leg";
            else if (zoneName.Contains("stomach") || zoneName.Contains("abdomen") || zoneName.Contains("hips"))
                hitPart = "Stomach";
            else if (zoneName.Contains("neck"))
                hitPart = "Neck";
        }

        // Killer name and weapon
        string killerName = "Unknown";
        string weaponName = "";
        IEntity killerEntity = null;

        if (damageContext.instigator)
            killerEntity = damageContext.instigator.GetInstigatorEntity();

        if (killerEntity)
        {
            PlayerManager pm = GetGame().GetPlayerManager();
            if (pm)
            {
                int killerId = pm.GetPlayerIdFromControlledEntity(killerEntity);
                if (killerId > 0)
                {
                    killerName = pm.GetPlayerName(killerId);
                }
                else
                {
                    string entName = killerEntity.GetName();
                    if (!entName.IsEmpty())
                    {
                        killerName = entName;
                    }
                    else
                    {
                        EntityPrefabData prefabData = killerEntity.GetPrefabData();
                        if (prefabData)
                        {
                            string prefabName = FilePath.StripExtension(FilePath.StripPath(prefabData.GetPrefabName()));
                            prefabName.Replace("armst_monster_", "");
                            prefabName.Replace("armst_mutant_", "");
                            prefabName.Replace("armst_anomaly_", "");
                            prefabName.Replace("armst_", "");
                            prefabName.Replace("_", " ");
                            killerName = prefabName;
                        }
                    }
                }

                weaponName = GetWeaponName(killerEntity);
            }
        }

        // Use damage source (weapon entity) for weapon name if available
        if (weaponName.IsEmpty() && damageContext.damageSource)
        {
            EntityPrefabData srcPrefab = damageContext.damageSource.GetPrefabData();
            if (srcPrefab)
            {
                string srcName = FilePath.StripExtension(FilePath.StripPath(srcPrefab.GetPrefabName()));
                srcName.Replace("_", " ");
                weaponName = srcName;
            }
        }

        EDamageType dmgType = damageContext.damageType;

        SRZ_KillfeedDamageTracker.RecordHit(
            m_PlayerId,
            killerName,
            weaponName,
            hitPart,
            Math.Round(damageContext.damageValue),
            dmgType,
            isHeadshot
        );
    }
}