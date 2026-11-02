// SRZ_HitLog v3 (worn armor). Hit logger for armor testing. One line per hit on any character in script.log:
// [SRZ Hit] victim | zone | type | damage | round | material | health | blood | resilience
// Armor hits log a [SRZ Armor] line with the passed damage and the armor pieces the victim is wearing.
// Server only. Set ENABLED to false (or delete the file) before this goes to the live server.
modded class SCR_CharacterDamageManagerComponent
{
	static const bool SRZ_HITLOG_ENABLED = true;

	//------------------------------------------------------------------------------------------------
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);

		if (!SRZ_HITLOG_ENABLED || !Replication.IsServer())
			return;

		if (damageContext.damageType != EDamageType.KINETIC && damageContext.damageType != EDamageType.MELEE)
			return;

		string zone = "?";
		if (damageContext.struckHitZone)
			zone = damageContext.struckHitZone.GetName();

		string round = "?";
		if (damageContext.damageSource && damageContext.damageSource.GetPrefabData())
			round = SRZ_Short(damageContext.damageSource.GetPrefabData().GetPrefabName());

		string material = "?";
		if (damageContext.material)
			material = SRZ_Short(damageContext.material.GetName());

		string line = "[SRZ Hit] " + SRZ_Victim();
		line += " | " + zone;
		line += " | " + typename.EnumToString(EDamageType, damageContext.damageType);
		line += " | dmg " + damageContext.damageValue.ToString(-1, 1);
		line += " | " + round;
		line += " | mat " + material;
		line += " | health " + SRZ_Health().ToString(-1, 1);
		line += " | blood " + SRZ_Zone("Blood").ToString(-1, 1);
		line += " | resilience " + SRZ_Zone("Resilience").ToString(-1, 1);
		line += " | cal " + typename.EnumToString(SRZ_Caliber, SRZ_ArmorTuning.CaliberOf(damageContext.damageSource));
		line += " | vest T" + SRZ_ArmorTuning.WornVestTier(GetOwner()) + " helmet " + SRZ_ArmorTuning.WornHelmetTier(GetOwner());
		PrintFormat("%1", line);
	}

	//------------------------------------------------------------------------------------------------
	override void ArmorHitEventDamage(EDamageType type, float damage, IEntity instigator)
	{
		if (SRZ_HITLOG_ENABLED && Replication.IsServer())
			PrintFormat("[SRZ Armor] %1 | armor passed %2 | %3 | wearing %4", SRZ_Victim(), damage.ToString(-1, 1), typename.EnumToString(EDamageType, type), SRZ_Worn());

		super.ArmorHitEventDamage(type, damage, instigator);
	}

	//------------------------------------------------------------------------------------------------
	protected string SRZ_Worn()
	{
		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(GetOwner().FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return "no loadout";

		array<IEntity> items = {};
		loadout.GetAll(items);

		string worn = "";
		foreach (IEntity item : items)
		{
			if (!item || !item.FindComponent(SCR_ArmorDamageManagerComponent))
				continue;

			if (worn != "")
				worn += ", ";

			worn += SRZ_Short(item.GetPrefabData().GetPrefabName());
		}

		if (worn == "")
			return "none";

		return worn;
	}

	//------------------------------------------------------------------------------------------------
	protected float SRZ_Health()
	{
		HitZone hz = GetDefaultHitZone();
		if (!hz)
			return -1;

		return hz.GetHealth();
	}

	//------------------------------------------------------------------------------------------------
	protected float SRZ_Zone(string name)
	{
		HitZone hz = GetHitZoneByName(name);
		if (!hz)
			return -1;

		return hz.GetHealth();
	}

	//------------------------------------------------------------------------------------------------
	protected string SRZ_Victim()
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner());
		if (playerId > 0)
			return GetGame().GetPlayerManager().GetPlayerName(playerId);

		if (GetOwner().GetPrefabData())
			return SRZ_Short(GetOwner().GetPrefabData().GetPrefabName());

		return "npc";
	}

	//------------------------------------------------------------------------------------------------
	protected static string SRZ_Short(string path)
	{
		int slash = path.LastIndexOf("/");
		if (slash >= 0)
			path = path.Substring(slash + 1, path.Length() - slash - 1);

		path.Replace(".et", "");
		path.Replace(".gamemat", "");
		return path;
	}
}
