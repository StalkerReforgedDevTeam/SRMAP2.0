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

// Saves restoring items into a packed jacket must not be refused, or they would spill on the floor
class SRZ_StorageLoadGuard
{
	static int s_iLoading;
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

		if (SRZ_IsNested())
			return false;

		return super.CanStoreItem(item, slotID);
	}

	//------------------------------------------------------------------------------------------------
	// A jacket, vest or bag that is itself packed inside another storage takes nothing. Worn gear and anything
	// lying in the world is fine. Artifact containers are the one exception so a packed container still holds artifacts.
	protected bool SRZ_IsNested()
	{
		if (SRZ_StorageLoadGuard.s_iLoading > 0 || ARMST_ARTEFACT_CONTAINER.Cast(this))
			return false;

		IEntity owner = GetOwner();
		if (!owner)
			return false;

		InventoryItemComponent itemComp = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;

		InventoryStorageSlot parentSlot = itemComp.GetParentSlot();
		if (!parentSlot)
			return false;

		BaseInventoryStorageComponent parentStorage = parentSlot.GetStorage();
		if (!parentStorage)
			return false;

		IEntity holder = parentStorage.GetOwner();
		if (!holder || ChimeraCharacter.Cast(holder))
			return false;

		return true;
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

// Crates and storages that live in the world can't be picked up from the vicinity list
modded class SCR_PickUpItemAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (SRZ_WorldContainer.Is(GetOwner()))
			return false;

		return super.CanBeShownScript(user);
	}
}
