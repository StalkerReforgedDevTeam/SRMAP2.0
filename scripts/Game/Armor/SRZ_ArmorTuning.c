// Armor rework tables. Every number here comes from the Armor Rework Chart.
// Chest tables are hits to kill from the chest: bare body, then tier 1 to 5.
// Head tables are hits to kill on the head: bare, light helmet, medium helmet, heavy helmet.
// Gear is matched by prefab name, lowercase substring, first match wins. Unknown gear falls back to the defaults below and prints a line in the log.
enum SRZ_Caliber
{
	UNKNOWN,
	PISTOL_9X18,
	PISTOL_9X19,
	PISTOL_762X25,
	PISTOL_45,
	CARBINE_30,
	KURZ_792,
	RIFLE_545,
	RIFLE_556,
	RIFLE_762X39,
	RIFLE_9X39_SP5,
	RIFLE_9X39_SP6,
	BATTLE_3006,
	BATTLE_762X51,
	SNIPER_762X54,
	SHOT_SLUG,
	SHOT_BUCK
}

class SRZ_ArmorTuning
{
	static const float BODY_POOL = 225;
	static const float HEAD_POOL = 102;
	static const float RESILIENCE = 180;
	// Knockout lands at this share of the chest kill count
	static const float KO_FRACTION = 0.75;
	// The engine knocks you out once resilience falls under this share of its maximum, not at zero
	static const float KO_THRESHOLD = 0.3;
	// Every chest column in the chart is multiplied by this. 1 = the chart as written, 1.3 = 30 percent more hits on every tier.
	static const float CHEST_HITS_SCALE = 1.3;
	// Arm and leg hits do this share of their vanilla damage. 1 = vanilla.
	static const float LIMB_SCALE = 0.4;
	static const int BUCK_PELLETS = 4;
	static const int DEFAULT_VEST_TIER = 2;
	static const int DEFAULT_HELMET_TIER = 2;
	static const bool LOG_UNKNOWN = true;

	// hits to kill, chest: bare, T1, T2, T3, T4 Beril, T5 RAD
	static ref array<int> CHEST_9X18 = {4, 6, 10, 16, 25, 40};
	static ref array<int> CHEST_9X19 = {4, 6, 12, 16, 25, 40};
	static ref array<int> CHEST_762X25 = {4, 6, 12, 16, 25, 40};
	static ref array<int> CHEST_45 = {4, 5, 11, 16, 22, 36};
	static ref array<int> CHEST_30CARBINE = {3, 5, 7, 17, 18, 28};
	static ref array<int> CHEST_792KURZ = {2, 3, 4, 15, 17, 22};
	static ref array<int> CHEST_545 = {2, 5, 8, 12, 17, 28};
	static ref array<int> CHEST_556 = {2, 5, 8, 13, 17, 28};
	static ref array<int> CHEST_762X39 = {2, 4, 7, 9, 13, 22};
	static ref array<int> CHEST_SP5 = {2, 5, 6, 12, 14, 22};
	static ref array<int> CHEST_SP6 = {2, 4, 6, 13, 15, 18};
	static ref array<int> CHEST_3006 = {1, 2, 7, 12, 15, 14};
	static ref array<int> CHEST_762X51 = {1, 2, 5, 10, 15, 15};
	static ref array<int> CHEST_762X54 = {1, 2, 5, 8, 16, 14};
	static ref array<int> CHEST_SLUG = {2, 3, 8, 9, 14, 20};
	static ref array<int> CHEST_BUCK = {2, 3, 5, 9, 16, 26};

	// hits to kill, head: bare, light, medium, heavy
	static ref array<int> HEAD_PISTOL = {2, 3, 5, 8};
	static ref array<int> HEAD_RIFLE = {1, 2, 3, 5};
	static ref array<int> HEAD_BATTLE = {1, 1, 2, 3};
	static ref array<int> HEAD_SLUG = {1, 2, 3, 4};
	static ref array<int> HEAD_BUCK = {1, 2, 3, 5};

	// vest and suit tiers, lowercase substrings of the prefab name, checked top to bottom
	static ref array<string> VEST_T5 = {"radsuit"};
	static ref array<string> VEST_T4 = {"beril"};
	static ref array<string> VEST_T3 = {"6b13", "6b45", "platecarrier", "platecharacter", "ryeksuit", "vestvariant", "mercheavyvest", "merc heavy vest", "exosuit", "mdl_exosuit"};
	static ref array<string> VEST_T2 = {"mercbest", "b05", "s26vest", "io7", "vst_eva", "suit_speca", "suit_science", "vest_pasgt", "dbsh", "seva", "psz9m", "banditvest", "dutyvest", "freedomvest", "monovest"};
	static ref array<string> VEST_T1 = {"jacket", "6b2", "6b3", "b03", "asg", "sunrise", "kirza", "psz9", "rhuno", "lifchik", "type56", "sru21p", "vest_naz", "vest_hunt", "zis", "stalker_modules", "sovietharness", "tracksuit"};

	// helmet tiers: 1 light, 2 medium, 3 heavy
	static ref array<string> HELMET_T3 = {"dutyberillhelm", "freedomberillhelm", "ukmberilhelm", "monoberillhelm", "militaryberillhelm", "altyn_full", "altyn_clear", "helmet_altyn_black", "helmet_altyn_green", "dh132a", "helmetmetal", "hereticalhelmet"};
	static ref array<string> HELMET_T2 = {"pasgt", "ssh68", "achhelmet", "tacticalhelmet", "exohelmet", "lonerberillhelm", "mercberilhelm", "hlmt_obol_", "bulat", "psyhea", "psychea", "militaryhelmet", "mercbrithelm", "helmet_seva", "tsh4"};
	static ref array<string> HELMET_T1 = {"ryekhelmet", "altyn_closed", "altyn_novis", "altyn_open", "kiver", "mich", "obol_l_", "helmet_cap"};

	//------------------------------------------------------------------------------------------------
	// Only fired rounds go through the chart. Mutant swipes, melee and anything else stay vanilla.
	static bool IsBullet(IEntity projectile)
	{
		if (!projectile || !projectile.GetPrefabData())
			return false;

		string n = projectile.GetPrefabData().GetPrefabName();
		n.ToLower();
		return n.Contains("ammo") || n.Contains("bullet") || n.Contains("pellet") || n.Contains("slug") || n.Contains("buck");
	}

	//------------------------------------------------------------------------------------------------
	static SRZ_Caliber CaliberOf(IEntity projectile)
	{
		if (!projectile || !projectile.GetPrefabData())
			return SRZ_Caliber.UNKNOWN;

		string n = projectile.GetPrefabData().GetPrefabName();
		n.ToLower();

		if (n.Contains("762x54") || n.Contains("127x55") || n.Contains("12x55") || n.Contains("8.6x70") || n.Contains("86x70") || n.Contains("ash12") || n.Contains("ash-12") || n.Contains("shak12") || n.Contains("338"))
			return SRZ_Caliber.SNIPER_762X54;
		if (n.Contains("762x51"))
			return SRZ_Caliber.BATTLE_762X51;
		if (n.Contains("762x63") || n.Contains("3006") || n.Contains("30-06") || n.Contains("garand"))
			return SRZ_Caliber.BATTLE_3006;
		if (n.Contains("762x39"))
			return SRZ_Caliber.RIFLE_762X39;
		if (n.Contains("762x33") || n.Contains("30carbine") || n.Contains("30_carbine") || n.Contains("carbine"))
			return SRZ_Caliber.CARBINE_30;
		if (n.Contains("762x25") || n.Contains("763x25") || n.Contains("tokarev") || n.Contains("tt33"))
			return SRZ_Caliber.PISTOL_762X25;
		if (n.Contains("792x33") || n.Contains("kurz") || n.Contains("stg"))
			return SRZ_Caliber.KURZ_792;
		if (n.Contains("9x39"))
		{
			if (n.Contains("sp6") || n.Contains("sp-6"))
				return SRZ_Caliber.RIFLE_9X39_SP6;

			return SRZ_Caliber.RIFLE_9X39_SP5;
		}
		if (n.Contains("545"))
			return SRZ_Caliber.RIFLE_545;
		if (n.Contains("556") || n.Contains("223"))
			return SRZ_Caliber.RIFLE_556;
		if (n.Contains("9x18") || n.Contains("makarov"))
			return SRZ_Caliber.PISTOL_9X18;
		if (n.Contains("9x19") || n.Contains("9mm") || n.Contains("parabellum"))
			return SRZ_Caliber.PISTOL_9X19;
		if (n.Contains("45acp") || n.Contains("45_acp") || n.Contains("_45") || n.Contains("45auto") || n.Contains("thompson") || n.Contains("1911"))
			return SRZ_Caliber.PISTOL_45;
		if (n.Contains("slug"))
			return SRZ_Caliber.SHOT_SLUG;
		if (n.Contains("buck") || n.Contains("pellet") || n.Contains("12ga") || n.Contains("12g"))
			return SRZ_Caliber.SHOT_BUCK;

		if (LOG_UNKNOWN && n.Contains("ammo"))
			Print("[SRZ Armor] unknown round " + n + ", treated as rifle");

		return SRZ_Caliber.UNKNOWN;
	}

	//------------------------------------------------------------------------------------------------
	static array<int> ChestTable(SRZ_Caliber cal)
	{
		switch (cal)
		{
			case SRZ_Caliber.PISTOL_9X18: return CHEST_9X18;
			case SRZ_Caliber.PISTOL_9X19: return CHEST_9X19;
			case SRZ_Caliber.PISTOL_762X25: return CHEST_762X25;
			case SRZ_Caliber.PISTOL_45: return CHEST_45;
			case SRZ_Caliber.CARBINE_30: return CHEST_30CARBINE;
			case SRZ_Caliber.KURZ_792: return CHEST_792KURZ;
			case SRZ_Caliber.RIFLE_545: return CHEST_545;
			case SRZ_Caliber.RIFLE_556: return CHEST_556;
			case SRZ_Caliber.RIFLE_762X39: return CHEST_762X39;
			case SRZ_Caliber.RIFLE_9X39_SP5: return CHEST_SP5;
			case SRZ_Caliber.RIFLE_9X39_SP6: return CHEST_SP6;
			case SRZ_Caliber.BATTLE_3006: return CHEST_3006;
			case SRZ_Caliber.BATTLE_762X51: return CHEST_762X51;
			case SRZ_Caliber.SNIPER_762X54: return CHEST_762X54;
			case SRZ_Caliber.SHOT_SLUG: return CHEST_SLUG;
			case SRZ_Caliber.SHOT_BUCK: return CHEST_BUCK;
		}

		return CHEST_545;
	}

	//------------------------------------------------------------------------------------------------
	static array<int> HeadTable(SRZ_Caliber cal)
	{
		switch (cal)
		{
			case SRZ_Caliber.PISTOL_9X18: return HEAD_PISTOL;
			case SRZ_Caliber.PISTOL_9X19: return HEAD_PISTOL;
			case SRZ_Caliber.PISTOL_762X25: return HEAD_PISTOL;
			case SRZ_Caliber.PISTOL_45: return HEAD_PISTOL;
			case SRZ_Caliber.CARBINE_30: return HEAD_PISTOL;
			case SRZ_Caliber.BATTLE_3006: return HEAD_BATTLE;
			case SRZ_Caliber.BATTLE_762X51: return HEAD_BATTLE;
			case SRZ_Caliber.SNIPER_762X54: return HEAD_BATTLE;
			case SRZ_Caliber.SHOT_SLUG: return HEAD_SLUG;
			case SRZ_Caliber.SHOT_BUCK: return HEAD_BUCK;
		}

		return HEAD_RIFLE;
	}

	//------------------------------------------------------------------------------------------------
	// Health taken from the chest per hit so the chart count kills. Buckshot is per pellet.
	static float ChestDamage(SRZ_Caliber cal, int tier)
	{
		array<int> table = ChestTable(cal);
		int hits = table[Math.Clamp(tier, 0, 5)];
		if (hits < 1)
			hits = 1;

		float dmg = BODY_POOL / (hits * CHEST_HITS_SCALE);
		if (cal == SRZ_Caliber.SHOT_BUCK)
			dmg = dmg / BUCK_PELLETS;

		return dmg;
	}

	//------------------------------------------------------------------------------------------------
	// Resilience taken per chest hit so the knockout lands at KO_FRACTION of the kill count
	static float ResilienceDamage(SRZ_Caliber cal, int tier)
	{
		array<int> table = ChestTable(cal);
		int hits = table[Math.Clamp(tier, 0, 5)];
		if (hits < 1)
			hits = 1;

		float dmg = RESILIENCE * (1 - KO_THRESHOLD) / (KO_FRACTION * hits * CHEST_HITS_SCALE);
		if (cal == SRZ_Caliber.SHOT_BUCK)
			dmg = dmg / BUCK_PELLETS;

		return dmg;
	}

	//------------------------------------------------------------------------------------------------
	static float HeadDamage(SRZ_Caliber cal, int helmetTier)
	{
		array<int> table = HeadTable(cal);
		int hits = table[Math.Clamp(helmetTier, 0, 3)];
		if (hits < 1)
			hits = 1;

		float dmg = HEAD_POOL / hits;
		if (cal == SRZ_Caliber.SHOT_BUCK)
			dmg = dmg / BUCK_PELLETS;

		return dmg;
	}

	//------------------------------------------------------------------------------------------------
	static int VestTierOf(IEntity item)
	{
		if (!item || !item.GetPrefabData())
			return 0;

		string n = item.GetPrefabData().GetPrefabName();
		int slash = n.LastIndexOf("/");
		if (slash >= 0)
			n = n.Substring(slash + 1, n.Length() - slash - 1);
		n.ToLower();

		if (Matches(n, VEST_T5)) return 5;
		if (Matches(n, VEST_T4)) return 4;
		if (Matches(n, VEST_T3)) return 3;
		if (Matches(n, VEST_T2)) return 2;
		if (Matches(n, VEST_T1)) return 1;

		if (!item.FindComponent(SCR_ArmorDamageManagerComponent))
			return 0;

		if (LOG_UNKNOWN)
			Print("[SRZ Armor] unknown vest " + n + ", treated as T" + DEFAULT_VEST_TIER);

		return DEFAULT_VEST_TIER;
	}

	//------------------------------------------------------------------------------------------------
	static int HelmetTierOf(IEntity item)
	{
		if (!item || !item.GetPrefabData())
			return 0;

		string n = item.GetPrefabData().GetPrefabName();
		int slash = n.LastIndexOf("/");
		if (slash >= 0)
			n = n.Substring(slash + 1, n.Length() - slash - 1);
		n.ToLower();

		if (Matches(n, HELMET_T3)) return 3;
		if (Matches(n, HELMET_T1)) return 1;
		if (Matches(n, HELMET_T2)) return 2;

		if (!item.FindComponent(SCR_ArmorDamageManagerComponent))
			return 0;

		if (LOG_UNKNOWN)
			Print("[SRZ Armor] unknown helmet " + n + ", treated as tier " + DEFAULT_HELMET_TIER);

		return DEFAULT_HELMET_TIER;
	}

	//------------------------------------------------------------------------------------------------
	// Highest tier between the armored vest slot and the jacket slot, so suits count too
	static int WornVestTier(IEntity character)
	{
		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(character.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return 0;

		int tier = VestTierOf(loadout.GetClothFromArea(LoadoutArmoredVestSlotArea));
		int jacket = VestTierOf(loadout.GetClothFromArea(LoadoutJacketArea));
		if (jacket > tier)
			tier = jacket;

		return tier;
	}

	//------------------------------------------------------------------------------------------------
	static int WornHelmetTier(IEntity character)
	{
		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(character.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return 0;

		return HelmetTierOf(loadout.GetClothFromArea(LoadoutHeadCoverArea));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsVestZone(string zone)
	{
		return zone == "Chest" || zone == "Abdomen";
	}

	//------------------------------------------------------------------------------------------------
	static bool IsLimbZone(string zone)
	{
		string z = zone;
		z.ToLower();
		return z.Contains("arm") || z.Contains("leg") || z.Contains("hand") || z.Contains("foot") || z.Contains("thigh") || z.Contains("calf") || z.Contains("shoulder") || z.Contains("knee") || z.Contains("elbow");
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Matches(string name, array<string> keys)
	{
		foreach (string key : keys)
		{
			if (name.Contains(key))
				return true;
		}

		return false;
	}
}
