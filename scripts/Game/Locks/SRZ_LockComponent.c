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

	[RplProp()]
	protected bool m_bUnlocked;

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
		return m_aKeys && !m_aKeys.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	string GetKeyName()
	{
		if (!HasKeys())
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

		if (m_aKeys.Contains(held.GetPrefabData().GetPrefabName()))
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
