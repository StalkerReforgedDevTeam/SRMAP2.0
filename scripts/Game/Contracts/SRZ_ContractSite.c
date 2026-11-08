enum SRZ_EContractEnemy
{
	ZOMBIE,
	ZOMBIE_RANDOM,
	ZOMBIE_PISTOL,
	TOXIC,
	TOXIC_2,
	WOLF,
	BOAR,
	RAT,
	NIGHTSTALKER,
	ERB,
	SPIDER,
	SPIDER_STALKER,
	GHOST,
	PSY_DESTROYER,
	ELECTRO_ELEMENTAL,
	FIRE_ELEMENTAL
}

enum SRZ_EContractLoot
{
	WEAPONS,
	GEAR,
	MEDICAL,
	UNIQUE
}

enum SRZ_EContractType
{
	HOLD,
	HUNT,
	RECOVERY,
	ANOMALY
}

// One enemy line in a wave: which mutant and exactly how many of it.
[BaseContainerProps()]
class SRZ_ContractWaveSpawn
{
	[Attribute("0", UIWidgets.ComboBox, "Mutant", "", ParamEnumArray.FromEnum(SRZ_EContractEnemy))]
	SRZ_EContractEnemy m_eEnemy;

	[Attribute("2", UIWidgets.Slider, "How many of this mutant", "1 12 1")]
	int m_iCount;
}

// One wave. Add a line per enemy type.
[BaseContainerProps()]
class SRZ_ContractWave
{
	[Attribute(desc: "What comes in this wave. Press + for each mutant")]
	ref array<ref SRZ_ContractWaveSpawn> m_aSpawns;
}

// A contract site. Drop the prefab in the world and it is live. Everything has a working default.
[EntityEditorProps(category: "SRZ/Contracts", description: "Contract site")]
class SRZ_ContractSiteClass : GenericEntityClass
{
}

class SRZ_ContractSite : GenericEntity
{
	[Attribute("0", UIWidgets.ComboBox, "Contract type", "", ParamEnumArray.FromEnum(SRZ_EContractType), category: "Contract")]
	SRZ_EContractType m_eType;

	[Attribute("", desc: "Name players see on the board, like Old Farm. Empty shows the grid", category: "Contract")]
	string m_sSiteName;

	[Attribute("0", UIWidgets.ComboBox, "Only this faction gets this site. FACTION_ALL means any faction that runs this contract type", "", ParamEnumArray.FromEnum(ARMST_FACTION_LABEL), category: "Contract")]
	ARMST_FACTION_LABEL m_eFaction;

	[Attribute("30", UIWidgets.Slider, "Area the player has to stay in (m)", "10 100 1", category: "Contract")]
	float m_fRadius;

	[Attribute("6", UIWidgets.Slider, "Minutes to hold", "2 20 1", category: "Contract")]
	float m_fHoldMinutes;

	[Attribute("60", UIWidgets.Slider, "Seconds between waves. Waves keep coming the whole hold", "20 300 5", category: "Contract")]
	float m_fWaveInterval;

	[Attribute(desc: "Press + for each wave, then + inside it for each mutant. When the list runs out it starts over from wave 1. Empty sends zombies, wolves or boars", category: "Contract")]
	ref array<ref SRZ_ContractWave> m_aWaveSetup;

	[Attribute("25000", UIWidgets.EditBox, "Pay in roubles", category: "Contract")]
	int m_iPay;

	[Attribute("0", UIWidgets.Slider, "Chance to also get a crate key when done, 0 to 1", "0 1 0.05", category: "Contract")]
	float m_fKeyChance;

	[Attribute("1", UIWidgets.Slider, "Which crate key: 1, 2 or 3", "1 3 1", category: "Contract")]
	int m_iKeyTier;

	[Attribute("0", UIWidgets.Slider, "Chance the key is one tier higher, 0 to 1", "0 1 0.05", category: "Contract")]
	float m_fKeyUpgradeChance;

	[Attribute("9", UIWidgets.ComboBox, "Hunt only: the alpha", "", ParamEnumArray.FromEnum(SRZ_EContractEnemy), category: "Hunt")]
	SRZ_EContractEnemy m_eAlpha;

	[Attribute("4", UIWidgets.Slider, "Hunt only: how many times tougher the alpha is", "1 15 0.5", category: "Hunt")]
	float m_fAlphaToughness;

	[Attribute("0", UIWidgets.ComboBox, "Recovery only: what the crate holds", "", ParamEnumArray.FromEnum(SRZ_EContractLoot), category: "Recovery")]
	SRZ_EContractLoot m_eLoot;

	[Attribute("15", UIWidgets.Slider, "Recovery only: seconds to search the crate", "5 60 1", category: "Recovery")]
	float m_fSearchSeconds;


	//------------------------------------------------------------------------------------------------
	void SRZ_ContractSite(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SRZ_ContractSite()
	{
		SRZ_Contracts.UnregisterSite(this);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode())
			return;

		SRZ_Contracts.RegisterSite(this);
	}

	//------------------------------------------------------------------------------------------------
	string GetKey()
	{
		vector pos = GetOrigin();
		return ((int)pos[0]).ToString() + "_" + ((int)pos[2]).ToString();
	}

	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		if (!m_sSiteName.IsEmpty())
			return m_sSiteName;

		return "Grid " + SRZ_Contracts.Grid(GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	bool IsFor(ARMST_FACTION_LABEL faction)
	{
		return m_eFaction == ARMST_FACTION_LABEL.FACTION_ALL || m_eFaction == faction;
	}

	//------------------------------------------------------------------------------------------------
	int GetWaveCount()
	{
		int waves = (int)Math.Ceil(GetHoldSeconds() / GetWaveInterval());
		if (waves < 1)
			return 1;

		return waves;
	}

	//------------------------------------------------------------------------------------------------
	float GetWaveInterval()
	{
		return Math.Max(m_fWaveInterval, 20);
	}

	//------------------------------------------------------------------------------------------------
	float GetHoldSeconds()
	{
		return Math.Max(m_fHoldMinutes, 1) * 60;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		Shape.CreateSphere(ARGB(255, 255, 140, 0), ShapeFlags.WIREFRAME | ShapeFlags.ONCE, GetOrigin(), m_fRadius);
		super._WB_AfterWorldUpdate(timeSlice);
	}
#endif
}

// Optional. Drop one near a site if the automatic spawn spots are bad there. The closest site uses it.
[EntityEditorProps(category: "SRZ/Contracts", description: "Contract spawn marker")]
class SRZ_ContractSpawnMarkerClass : GenericEntityClass
{
}

class SRZ_ContractSpawnMarker : GenericEntity
{
	//------------------------------------------------------------------------------------------------
	void SRZ_ContractSpawnMarker(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SRZ_ContractSpawnMarker()
	{
		SRZ_Contracts.UnregisterMarker(this);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode() || !Replication.IsServer())
			return;

		SRZ_Contracts.RegisterMarker(this);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.WIREFRAME | ShapeFlags.ONCE, GetOrigin(), 1.5);
		super._WB_AfterWorldUpdate(timeSlice);
	}
#endif
}
