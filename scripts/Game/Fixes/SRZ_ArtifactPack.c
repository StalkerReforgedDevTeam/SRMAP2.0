// SRZ artifact pack: our artifacts join the anomaly fields by anomaly type and tier,
// stay hidden until a detector finds them, and mutants can drop the new parts when cut.

class SRZ_ArtEntry
{
	ResourceName m_sPrefab;
	string m_sKey;
	float m_fValue;

	void SRZ_ArtEntry(ResourceName prefab, string key, float value)
	{
		m_sPrefab = prefab;
		m_sKey = key;
		m_fValue = value;
	}
}

class SRZ_NewArtifacts
{
	protected static ref array<ref SRZ_ArtEntry> s_aArts;
	protected static ref array<ref SRZ_ArtEntry> s_aParts;
	protected static ref map<string, ResourceName> s_mSwap;

	protected static void Build()
	{
		s_aArts = {};
		s_aParts = {};
		s_mSwap = new map<string, ResourceName>();
		AddArt("{5109FE8DA93EB69E}Prefabs/Items/SRZ_Artifacts/srz_art_jellyfish_drop.et", "gravi", 1);
		AddArt("{9E3C4F75603F0B91}Prefabs/Items/SRZ_Artifacts/srz_art_wrenched_drop.et", "gravi", 1);
		AddArt("{10188910D844062E}Prefabs/Items/SRZ_Artifacts/srz_art_bloodstone_drop.et", "gravi", 1);
		AddArt("{855AB3DFED11482C}Prefabs/Items/SRZ_Artifacts/srz_art_empty_drop.et", "gravi", 1);
		AddArt("{32C674352CB78DA6}Prefabs/Items/SRZ_Artifacts/srz_art_ring_drop.et", "gravi", 1);
		AddArt("{1F92DA013133D7D8}Prefabs/Items/SRZ_Artifacts/srz_art_gravi_drop.et", "gravi", 2);
		AddArt("{48AEAE2C1B01226D}Prefabs/Items/SRZ_Artifacts/srz_art_spring_drop.et", "gravi", 2);
		AddArt("{A02E7A194B1A18C7}Prefabs/Items/SRZ_Artifacts/srz_art_balloon_drop.et", "gravi", 2);
		AddArt("{7F2CD584EC5E4E56}Prefabs/Items/SRZ_Artifacts/srz_art_fullempty_drop.et", "gravi", 3);
		AddArt("{DDF65A77F528CAD8}Prefabs/Items/SRZ_Artifacts/srz_art_shell_drop.et", "electro", 1);
		AddArt("{68DCD034A55BC4E7}Prefabs/Items/SRZ_Artifacts/srz_art_thorn_drop.et", "electro", 1);
		AddArt("{82F336AEF8685AA8}Prefabs/Items/SRZ_Artifacts/srz_art_battery_drop.et", "electro", 2);
		AddArt("{988E97547AD6D4CE}Prefabs/Items/SRZ_Artifacts/srz_art_ice_drop.et", "electro", 2);
		AddArt("{C60ED4857C9D70DC}Prefabs/Items/SRZ_Artifacts/srz_art_bracelet_drop.et", "electro", 2);
		AddArt("{6CD314BA9A641B82}Prefabs/Items/SRZ_Artifacts/srz_art_snowflake_drop.et", "electro", 3);
		AddArt("{F30B29F44A9F6614}Prefabs/Items/SRZ_Artifacts/srz_art_crystalthorn_drop.et", "fire", 1);
		AddArt("{18CFAB76B7844B57}Prefabs/Items/SRZ_Artifacts/srz_art_firefly_drop.et", "fire", 2);
		AddArt("{8EE740E2EE72C38B}Prefabs/Items/SRZ_Artifacts/srz_art_emberheart_drop.et", "fire", 2);
		AddArt("{272981249BDB283F}Prefabs/Items/SRZ_Artifacts/srz_art_mamasbeads_drop.et", "fire", 3);
		AddArt("{11FD6DBFFECD04B1}Prefabs/Items/SRZ_Artifacts/srz_art_flame_drop.et", "fire", 3);
		AddArt("{4B65495DA1CFC149}Prefabs/Items/SRZ_Artifacts/srz_art_geode_drop.et", "toxic", 1);
		AddArt("{9D4C5C28EFB41D67}Prefabs/Items/SRZ_Artifacts/srz_art_meatchunk_drop.et", "toxic", 1);
		AddArt("{08A19A43A9C9730B}Prefabs/Items/SRZ_Artifacts/srz_art_acidsponge_drop.et", "toxic", 2);
		AddArt("{F6596E18A1C6D3CE}Prefabs/Items/SRZ_Artifacts/srz_art_toadseye_drop.et", "toxic", 2);
		AddArt("{2004BBFD0C1B943D}Prefabs/Items/SRZ_Artifacts/srz_art_pellicle_drop.et", "toxic", 2);
		AddArt("{5ADA987B1B9AAFA8}Prefabs/Items/SRZ_Artifacts/srz_art_glasspearl_drop.et", "toxic", 3);
		AddArt("{AD0F27344B00DB75}Prefabs/Items/SRZ_Artifacts/srz_art_kolobok_drop.et", "toxic", 3);
		AddArt("{FC84B69543D774A3}Prefabs/Items/SRZ_Artifacts/srz_art_deathlamp_drop.et", "toxic", 3);
		AddArt("{878E586387B6970C}Prefabs/Items/SRZ_Artifacts/srz_art_lobstereyes_drop.et", "psy", 1);
		AddArt("{B098F5017A0DF728}Prefabs/Items/SRZ_Artifacts/srz_art_urchin_drop.et", "psy", 2);
		AddArt("{CEF165CCDD76B63D}Prefabs/Items/SRZ_Artifacts/srz_art_nightlantern_drop.et", "psy", 2);
		AddArt("{43DDC8B4FEB7B84D}Prefabs/Items/SRZ_Artifacts/srz_art_cocoon_drop.et", "psy", 2);
		AddArt("{EAC2EA47C6721B86}Prefabs/Items/SRZ_Artifacts/srz_art_eye_drop.et", "psy", 3);
		AddArt("{143C7BCC3325D724}Prefabs/Items/SRZ_Artifacts/srz_art_phantomknot_drop.et", "psy", 3);
		AddArt("{F75FD354BA3364F5}Prefabs/Items/SRZ_Artifacts/srz_art_compass_drop.et", "psy", 4);
		AddArt("{1F3A673D5769098D}Prefabs/Items/SRZ_Artifacts/srz_art_soul_drop.et", "psy", 4);
		AddArt("{54BC3045466027A6}Prefabs/Items/SRZ_Artifacts/srz_art_monolithshard_drop.et", "psy", 4);
		AddArt("{3A2F2E3610EE55F2}Prefabs/Items/SRZ_Artifacts/srz_art_sparkler_drop.et", "electro", 1);
		AddArt("{87F0289143EB422A}Prefabs/Items/SRZ_Artifacts/srz_art_droplets_drop.et", "fire", 1);
		AddArt("{FDC468B65A3063B5}Prefabs/Items/SRZ_Artifacts/srz_art_stoneflower_drop.et", "gravi", 1);
		AddArt("{DDABF1BF89A0FCF3}Prefabs/Items/SRZ_Artifacts/srz_art_slime_drop.et", "toxic", 1);
		AddArt("{52FAB1AB64EFC40C}Prefabs/Items/SRZ_Artifacts/srz_art_flash_drop.et", "electro", 2);
		AddArt("{36EDCD48C4B859D6}Prefabs/Items/SRZ_Artifacts/srz_art_fireball_drop.et", "fire", 2);
		AddArt("{3AC34C529604795C}Prefabs/Items/SRZ_Artifacts/srz_art_nightstar_drop.et", "gravi", 2);
		AddArt("{66F331DC0A211784}Prefabs/Items/SRZ_Artifacts/srz_art_slug_drop.et", "toxic", 2);
		AddArt("{541ECC8687DE8816}Prefabs/Items/SRZ_Artifacts/srz_art_moonlight_drop.et", "electro", 3);
		AddArt("{9A323D17EB333099}Prefabs/Items/SRZ_Artifacts/srz_art_crystal_drop.et", "fire", 3);
		AddArt("{873178EBA4D8AED3}Prefabs/Items/SRZ_Artifacts/srz_art_goldenfish_drop.et", "gravi", 3);
		AddArt("{359C074E5561706F}Prefabs/Items/SRZ_Artifacts/srz_art_mica_drop.et", "toxic", 3);
		AddArt("{4DBCC76C628D16D7}Prefabs/Items/SRZ_Artifacts/srz_art_bubble_drop.et", "toxic", 2);
		s_mSwap.Set("{00DDB16A5A775F47}", "{3A2F2E3610EE55F2}Prefabs/Items/SRZ_Artifacts/srz_art_sparkler_drop.et");
		s_mSwap.Set("{EABCF33CC349E7C3}", "{87F0289143EB422A}Prefabs/Items/SRZ_Artifacts/srz_art_droplets_drop.et");
		s_mSwap.Set("{CCD21D34E9CB21BF}", "{FDC468B65A3063B5}Prefabs/Items/SRZ_Artifacts/srz_art_stoneflower_drop.et");
		s_mSwap.Set("{5BA31E48F7933CA9}", "{DDABF1BF89A0FCF3}Prefabs/Items/SRZ_Artifacts/srz_art_slime_drop.et");
		s_mSwap.Set("{2F3C09BBF8CCA364}", "{4DBCC76C628D16D7}Prefabs/Items/SRZ_Artifacts/srz_art_bubble_drop.et");
		AddPart("{8411311F3EDA391C}Prefabs/Items/SRZ_MutantParts/srz_mut_boar_tusk.et", "boar", 0.35);
		AddPart("{9BD1A5C2548B603C}Prefabs/Items/SRZ_MutantParts/srz_mut_boar_hoof.et", "boar", 0.25);
		AddPart("{0ECAB89F23D3561C}Prefabs/Items/SRZ_MutantParts/srz_mut_wolf_tail.et", "wolf", 0.35);
		AddPart("{B100723494877258}Prefabs/Items/SRZ_MutantParts/srz_mut_wolf_paw.et", "wolf", 0.25);
		AddPart("{3B9B76EBB8C8B188}Prefabs/Items/SRZ_MutantParts/srz_mut_rat_tail.et", "_rat", 0.35);
		AddPart("{92F4BD350157A99D}Prefabs/Items/SRZ_MutantParts/srz_mut_rat_skull.et", "_rat", 0.2);
		AddPart("{FE695B61EDF41EE1}Prefabs/Items/SRZ_MutantParts/srz_mut_spider_fang.et", "spider", 0.35);
		AddPart("{D53097AF9E8F47E3}Prefabs/Items/SRZ_MutantParts/srz_mut_spider_venomgland.et", "spider", 0.2);
		AddPart("{0C3E6C233FC5EE7C}Prefabs/Items/SRZ_MutantParts/srz_mut_zombie_hand.et", "zombie", 0.35);
		AddPart("{BB08228EC8500941}Prefabs/Items/SRZ_MutantParts/srz_mut_zombie_heart.et", "zombie", 0.15);
		AddPart("{8E0742E815CE3686}Prefabs/Items/SRZ_MutantParts/srz_mut_nightstalker_tentacles.et", "nighstalker", 0.3);
		AddPart("{80EA9CB114C5F51E}Prefabs/Items/SRZ_MutantParts/srz_mut_nightstalker_eye.et", "nighstalker", 0.15);
		AddPart("{289B05D9FA6C2CDC}Prefabs/Items/SRZ_MutantParts/srz_mut_erb_claw.et", "_erb", 0.3);
		AddPart("{441178E1AEDBCE55}Prefabs/Items/SRZ_MutantParts/srz_mut_erb_spine.et", "_erb", 0.15);
		AddPart("{F2EC22B88172522A}Prefabs/Items/SRZ_MutantParts/srz_mut_busher_root.et", "busher", 0.3);
		AddPart("{5F3D9468A90F3711}Prefabs/Items/SRZ_MutantParts/srz_mut_busher_seedpod.et", "busher", 0.15);
		AddPart("{EA11B8AAC556F3AC}Prefabs/Items/SRZ_MutantParts/srz_mut_psydestroyer_brain.et", "psy_destroyer", 0.2);
		AddPart("{2C4A2E6B6718A50C}Prefabs/Items/SRZ_MutantParts/srz_mut_psydestroyer_hand.et", "psy_destroyer", 0.3);
		AddPart("{314065EBBEA96F64}Prefabs/Items/SRZ_MutantParts/srz_mut_shaman_eye.et", "shaman", 0.2);
		AddPart("{C54E9BCB79C641D1}Prefabs/Items/SRZ_MutantParts/srz_mut_shaman_charm.et", "shaman", 0.25);
	}

	protected static void AddArt(ResourceName prefab, string category, int tier)
	{
		s_aArts.Insert(new SRZ_ArtEntry(prefab, category, tier));
	}

	protected static void AddPart(ResourceName prefab, string mutant, float chance)
	{
		s_aParts.Insert(new SRZ_ArtEntry(prefab, mutant, chance));
	}

	static array<ref SRZ_ArtEntry> GetArts()
	{
		if (!s_aArts)
			Build();
		return s_aArts;
	}

	static array<ref SRZ_ArtEntry> GetParts()
	{
		if (!s_aParts)
			Build();
		return s_aParts;
	}

	static ResourceName GetSwap(ResourceName armstPrefab)
	{
		if (!s_mSwap)
			Build();

		string key = armstPrefab;
		if (key.Length() < 18)
			return ResourceName.Empty;

		ResourceName swap;
		if (s_mSwap.Find(key.Substring(0, 18), swap))
			return swap;
		return ResourceName.Empty;
	}

	// copies of each artifact added to a field; better artifacts are rarer and only show up in harder fields
	static int Copies(int fieldTier, int artTier)
	{
		if (fieldTier <= 1)
		{
			if (artTier == 1) return 4;
			if (artTier == 2) return 1;
			return 0;
		}

		if (fieldTier == 2)
		{
			if (artTier == 1) return 3;
			if (artTier == 2) return 2;
			if (artTier == 3) return 1;
			return 0;
		}

		if (artTier == 1) return 2;
		if (artTier == 2) return 3;
		if (artTier == 3) return 2;
		return 1;
	}

	static string Lower(string text)
	{
		string s = text;
		s.ToLower();
		return s;
	}

	static string CategoryOf(string anomalyPrefab)
	{
		string p = Lower(anomalyPrefab);
		if (p.IsEmpty())
			return "";
		if (p.Contains("electro"))
			return "electro";
		if (p.Contains("jarka") || p.Contains("puziriks"))
			return "fire";
		if (p.Contains("studen") || p.Contains("toxic") || p.Contains("moshkara"))
			return "toxic";
		if (p.Contains("gravy") || p.Contains("trampoline") || p.Contains("grinder") || p.Contains("whirligig") || p.Contains("vortex"))
			return "gravi";
		if (p.Contains("/eye/") || p.Contains("ghost") || p.Contains("miracle") || p.Contains("teleport"))
			return "psy";
		return "";
	}

	static int TierOf(string anomalyPrefab)
	{
		string p = Lower(anomalyPrefab);
		if (p.Contains("_t3") || p.Contains("tier3"))
			return 3;
		if (p.Contains("/eye/") || p.Contains("ghost") || p.Contains("miracle") || p.Contains("teleport"))
			return 3;
		if (p.Contains("_t2") || p.Contains("tier2") || p.Contains("grinder") || p.Contains("whirligig") || p.Contains("puziriks"))
			return 2;
		return 1;
	}

	static bool IsPackDrop(IEntity ent)
	{
		if (!ent || !ent.GetPrefabData())
			return false;

		string path = ent.GetPrefabData().GetPrefabName();
		return path.Contains("SRZ_Artifacts/") && path.Contains("_drop");
	}

	static string PrefabKey(IEntity ent)
	{
		if (!ent || !ent.GetPrefabData())
			return "";
		return Lower(ent.GetPrefabData().GetPrefabName());
	}
}

modded class ARMST_TRIGGER_SPAWN
{
	[Attribute("0", UIWidgets.ComboBox, "Artifact tier for this field. Auto guesses from the anomaly names", "", { ParamEnum("Auto", "0"), ParamEnum("Tier 1", "1"), ParamEnum("Tier 2", "2"), ParamEnum("Tier 3", "3") }, category: "SRZ Artifacts")]
	int m_iSRZ_FieldTier;

	protected bool m_bSRZ_PackAdded;

	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		if (SCR_Global.IsEditMode() || !GetGame().InPlayMode() || !Replication.IsServer())
			return;

		SRZ_AddPackArtifacts();
	}

	protected void SRZ_AddPackArtifacts()
	{
		if (m_bSRZ_PackAdded || !m_ArtsPrefab || m_ArtsPrefab.IsEmpty())
			return;

		m_bSRZ_PackAdded = true;

		for (int i = 0; i < m_ArtsPrefab.Count(); i++)
		{
			ResourceName swap = SRZ_NewArtifacts.GetSwap(m_ArtsPrefab[i]);
			if (swap != ResourceName.Empty)
				m_ArtsPrefab.Set(i, swap);
		}

		string category;
		int tier = 1;
		array<ResourceName> anomalies = {groupPrefab, m_ObjectPrefab, m_ObjectPrefab2, m_ObjectPrefab3};
		foreach (ResourceName anomaly : anomalies)
		{
			string cat = SRZ_NewArtifacts.CategoryOf(anomaly);
			if (cat.IsEmpty())
				continue;

			if (category.IsEmpty())
				category = cat;
			tier = Math.Max(tier, SRZ_NewArtifacts.TierOf(anomaly));
		}

		if (m_iSRZ_FieldTier > 0)
			tier = m_iSRZ_FieldTier;

		if (category.IsEmpty())
		{
			Print(string.Format("[SRZ_ArtifactPack] field at %1 (%2): no matching anomaly type, pack artifacts not added", GetOrigin(), groupPrefab), LogLevel.WARNING);
			return;
		}

		int before = m_ArtsPrefab.Count();

		foreach (SRZ_ArtEntry art : SRZ_NewArtifacts.GetArts())
		{
			if (art.m_sKey != category)
				continue;

			int copies = SRZ_NewArtifacts.Copies(tier, (int)art.m_fValue);
			for (int c = 0; c < copies; c++)
				m_ArtsPrefab.Insert(art.m_sPrefab);
		}

		Print(string.Format("[SRZ_ArtifactPack] field at %1: %2 tier %3, added %4 pack artifacts", GetOrigin(), category, tier, m_ArtsPrefab.Count() - before));
	}

	override void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);

		if (Replication.IsServer() && EntityUtils.IsPlayer(ent))
			Print(string.Format("[SRZ_ArtifactPack] field at %1 activated by player, anomaly %2, count %3", GetOrigin(), groupPrefab, m_SpawnCount));
	}
}

// Our models have no alpha fade, so the hidden state is the entity's visibility instead
modded class ARMST_ARTEFACT_COMPONENTS
{
	override void EnableLight()
	{
		super.EnableLight();
		SRZ_SetShown(false);
	}

	override void DisableLight()
	{
		super.DisableLight();
		SRZ_SetShown(true);
	}

	protected void SRZ_SetShown(bool shown)
	{
		IEntity owner = GetOwner();
		if (!SRZ_NewArtifacts.IsPackDrop(owner))
			return;

		if (shown)
			owner.SetFlags(EntityFlags.VISIBLE, false);
		else
			owner.ClearFlags(EntityFlags.VISIBLE, false);
	}
}

modded class ARMST_USER_LOOT_BODY
{
	// One cut gives one part. Sometimes that part is one of the new pack parts instead of the ARMST one.
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_ForMutants || !pOwnerEntity)
		{
			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		// Only the server spawns the part, so clients never get a local copy of it
		if (!Replication.IsServer())
		{
			m_lock_ent = false;
			return;
		}

		ResourceName pick = SRZ_PickPackPart(pOwnerEntity);
		if (pick.IsEmpty())
		{
			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		ResourceName original = m_PrefabToSpawn;
		m_PrefabToSpawn = pick;
		super.PerformAction(pOwnerEntity, pUserEntity);
		m_PrefabToSpawn = original;
	}

	protected ResourceName SRZ_PickPackPart(IEntity body)
	{
		IEntity root = body.GetRootParent();
		string key = SRZ_NewArtifacts.PrefabKey(root) + " " + SRZ_NewArtifacts.Lower(m_PrefabToSpawn);

		array<SRZ_ArtEntry> matches = {};
		foreach (SRZ_ArtEntry part : SRZ_NewArtifacts.GetParts())
		{
			if (key.Contains(part.m_sKey))
				matches.Insert(part);
		}

		while (!matches.IsEmpty())
		{
			int i = Math.RandomInt(0, matches.Count());
			SRZ_ArtEntry candidate = matches[i];
			if (Math.RandomFloat01() <= candidate.m_fValue)
				return candidate.m_sPrefab;

			matches.Remove(i);
		}

		return ResourceName.Empty;
	}
}
