// Crates and storages that sit in the world (faction storages, lock and key crates, ARMST search crates,
// arsenal boxes, anything with the ARMST Search action)
class SRZ_WorldContainer
{
	static bool Is(IEntity ent)
	{
		if (!ent)
			return false;

		if (ent.FindComponent(SRZ_FactionStorageComponent) || ent.FindComponent(SRZ_LockComponent) || ent.FindComponent(SRZ_FixedLootComponent) || ent.FindComponent(ARMST_TieredLootSpawnerComponent) || ent.FindComponent(SCR_ArsenalComponent))
			return true;

		ActionsManagerComponent mgr = ActionsManagerComponent.Cast(ent.FindComponent(ActionsManagerComponent));
		if (!mgr)
			return false;

		array<BaseUserAction> actions = {};
		mgr.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			if (ARMST_OpenStorageAction.Cast(action))
				return true;
		}

		return false;
	}
}

// Only hidden from the nearby list for players who aren't allowed in: wrong faction, or still locked.
// Everyone else still sees and loots it normally.
modded class SCR_UniversalInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override protected bool ShouldHideInVicinity()
	{
		IEntity owner = GetOwner();
		if (owner && !owner.GetParent())
		{
			SRZ_LockComponent lockComp = SRZ_LockComponent.Get(owner);
			if (lockComp && lockComp.IsLocked())
				return true;

			SRZ_FactionStorageComponent factionStorage = SRZ_FactionStorageComponent.Get(owner);
			IEntity local = SCR_PlayerController.GetLocalControlledEntity();
			if (factionStorage && local && !factionStorage.CanUse(local))
				return true;
		}

		// Not super: the base version hides every storage, which emptied every crate's loot window
		return false;
	}

	//------------------------------------------------------------------------------------------------
	// A crate can't be dragged into a backpack, vest or pocket. Only checked on storages a character is wearing,
	// so loot going into crates and stashes is never touched.
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (SRZ_OnCharacter() && SRZ_WorldContainer.Is(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_OnCharacter()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		IEntity root = owner.GetRootParent();
		if (!root || root == owner)
			return false;

		return ChimeraCharacter.Cast(root) != null;
	}
}

modded class SCR_CharacterInventoryStorageComponent
{
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (SRZ_WorldContainer.Is(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}
}
