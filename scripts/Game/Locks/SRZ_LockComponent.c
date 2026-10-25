// Locks a door or container until a key, keycard or lockpick opens it. Not saved, so everything locks again on restart.
[ComponentEditorProps(category: "SRZ", description: "Lock for doors and containers")]
class SRZ_LockComponentClass : ScriptComponentClass
{
}

class SRZ_LockComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Items that unlock this, any one works (keys or keycards)", "et")]
	protected ref array<ResourceName> m_aKeys;

	[Attribute("0", UIWidgets.CheckBox, "Use up the key when unlocking (keycards)")]
	protected bool m_bConsumeKey;

	[Attribute("0", UIWidgets.CheckBox, "Can be opened with a lockpick")]
	protected bool m_bAllowLockpick;

	[Attribute("0", UIWidgets.CheckBox, "Also opens with the tier key that matches this crate's loot tier")]
	protected bool m_bTierKey;

	[Attribute("{072BBEF024816BBD}Prefabs/Items/Keys/Tier/SRZ_Key_T1.et", UIWidgets.ResourcePickerThumbnail, "Tier 1 key", "et")]
	protected ResourceName m_sTierKey1;

	[Attribute("{3B7FFD4B0D298673}Prefabs/Items/Keys/Tier/SRZ_Key_T2.et", UIWidgets.ResourcePickerThumbnail, "Tier 2 key", "et")]
	protected ResourceName m_sTierKey2;

	[Attribute("{0E8018E7EC40F2F5}Prefabs/Items/Keys/Tier/SRZ_Key_T3.et", UIWidgets.ResourcePickerThumbnail, "Tier 3 key", "et")]
	protected ResourceName m_sTierKey3;

	[RplProp(onRplName: "OnUnlockedRpl")]
	protected bool m_bUnlocked;

	static const ResourceName OPEN_SOUND = "{2E3DFF1F19AE3C36}Sounds/SRZ/Lockpick/SRZ_LockpickOpen.acp";

	protected int m_iInitTick;
	protected ResourceName m_sTierKey;
	protected bool m_bTierKeyResolved;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_iInitTick = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	// Skips the first seconds so players joining late don't hear every open crate
	protected void OnUnlockedRpl()
	{
		if (!m_bUnlocked)
			return;

		bool live = System.GetTickCount() - m_iInitTick > 5000;
		if (live)
			PlayOpenSound();

		OpenLid(live);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenLid(bool animate)
	{
		SRZ_CrateLidComponent lid = SRZ_CrateLidComponent.Cast(GetOwner().FindComponent(SRZ_CrateLidComponent));
		if (lid)
			lid.Open(animate);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayOpenSound()
	{
		SRZ_FXRange.Sound(OPEN_SOUND, "SOUND_SRZ_LOCKPICK_OPEN", GetOwner().GetOrigin(), 50);
	}

	//------------------------------------------------------------------------------------------------
	static SRZ_LockComponent Get(IEntity ent)
	{
		if (!ent)
			return null;

		return SRZ_LockComponent.Cast(ent.FindComponent(SRZ_LockComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsLocked()
	{
		return !m_bUnlocked;
	}

	bool AllowsLockpick()
	{
		return m_bAllowLockpick;
	}

	bool HasKeys()
	{
		return (m_aKeys && !m_aKeys.IsEmpty()) || !GetTierKey().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	// Highest tier on the crate's ARMST loot spawner picks the key
	ResourceName GetTierKey()
	{
		if (m_bTierKeyResolved)
			return m_sTierKey;

		m_bTierKeyResolved = true;
		if (!m_bTierKey || !GetOwner())
			return m_sTierKey;

		int tier = 1;
		ARMST_TieredLootSpawnerComponent tierComp = ARMST_TieredLootSpawnerComponent.Cast(GetOwner().FindComponent(ARMST_TieredLootSpawnerComponent));
		if (tierComp && tierComp.m_LootTiersSpawn)
		{
			foreach (string t : tierComp.m_LootTiersSpawn)
			{
				if (t == "TIER_2" && tier < 2)
					tier = 2;
				else if (t == "TIER_3" || t == "TIER_4" || t == "TIER_5")
					tier = 3;
			}
		}

		if (tier == 3)
			m_sTierKey = m_sTierKey3;
		else if (tier == 2)
			m_sTierKey = m_sTierKey2;
		else
			m_sTierKey = m_sTierKey1;

		return m_sTierKey;
	}

	//------------------------------------------------------------------------------------------------
	string GetKeyName()
	{
		ResourceName tierKey = GetTierKey();
		if (!tierKey.IsEmpty())
			return Helpers.GetPrefabDisplayName(tierKey);

		if (!m_aKeys || m_aKeys.IsEmpty())
			return "";

		return Helpers.GetPrefabDisplayName(m_aKeys[0]);
	}

	//------------------------------------------------------------------------------------------------
	IEntity FindKey(IEntity user)
	{
		if (!HasKeys())
			return null;

		IEntity held = GetHeldItem(user);
		if (!held || !held.GetPrefabData())
			return null;

		ResourceName heldPrefab = held.GetPrefabData().GetPrefabName();
		if (m_aKeys && m_aKeys.Contains(heldPrefab))
			return held;

		ResourceName tierKey = GetTierKey();
		if (!tierKey.IsEmpty() && heldPrefab == tierKey)
			return held;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity FindLockpick(IEntity user)
	{
		IEntity held = GetHeldItem(user);
		if (held && held.FindComponent(SRZ_LockpickComponent))
			return held;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// Keys and lockpicks have to be held, same check ARMST uses for items in hand
	static IEntity GetHeldItem(IEntity user)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return null;

		CharacterControllerComponent ctrl = character.GetCharacterController();
		if (!ctrl)
			return null;

		return ctrl.GetAttachedGadgetAtLeftHandSlot();
	}

	//------------------------------------------------------------------------------------------------
	// Server only
	bool UnlockWithKey(IEntity user)
	{
		if (!Replication.IsServer() || m_bUnlocked)
			return false;

		IEntity key = FindKey(user);
		if (!key)
			return false;

		if (m_bConsumeKey)
			SCR_EntityHelper.DeleteEntityAndChildren(key);

		SetUnlocked();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Server only
	bool UnlockWithLockpick(IEntity user)
	{
		if (!Replication.IsServer() || m_bUnlocked || !m_bAllowLockpick)
			return false;

		IEntity pick = FindLockpick(user);
		if (!pick)
			return false;

		SRZ_LockpickComponent pickComp = SRZ_LockpickComponent.Cast(pick.FindComponent(SRZ_LockpickComponent));
		if (pickComp)
			pickComp.UseOnce(pick);

		SetUnlocked();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetUnlocked()
	{
		m_bUnlocked = true;
		Replication.BumpMe();
		PlayOpenSound();
		OpenLid(true);
	}
}

// Marks an item as a lockpick. Uses are stored as the item's condition so they save with the item.
[ComponentEditorProps(category: "SRZ", description: "Lockpick")]
class SRZ_LockpickComponentClass : ScriptComponentClass
{
}

class SRZ_LockpickComponent : ScriptComponent
{
	[Attribute("3", UIWidgets.EditBox, "Uses before it breaks")]
	protected int m_iUses;

	//------------------------------------------------------------------------------------------------
	void UseOnce(IEntity pick)
	{
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(pick.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats || m_iUses <= 1)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(pick);
			return;
		}

		stats.DecreaseCondition(100 / m_iUses + 1);
		if (stats.GetConditionLevel() <= 1)
			SCR_EntityHelper.DeleteEntityAndChildren(pick);
	}
}

class SRZ_LockpickPredicate : InventorySearchPredicate
{
	void SRZ_LockpickPredicate()
	{
		QueryComponentTypes.Insert(SRZ_LockpickComponent);
	}

	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return SRZ_LockpickComponent.Cast(queriedComponents[0]);
	}
}
