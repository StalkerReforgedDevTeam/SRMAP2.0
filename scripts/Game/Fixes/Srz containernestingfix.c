// SRZ_ContainerNestingFix.c
// scripts/Game/Fixes/SRZ_ContainerNestingFix.c
//
// Fixes: inserting a container item into a container that is itself already stored
// inside another container (depth-2 nesting) crashes the server when that nested
// storage is opened in the UI. Example: beril A worn, beril B placed inside A (fine,
// depth 1) - inserting beril C into B (now depth 2, since B is inside A) crashes.
//
// Rule: a container item may be inserted into a destination container ONLY IF that
// destination container is not itself already stored inside another container.
// The player/AI character's own body is excluded from counting as a "container" -
// being worn directly on a character is depth-0, not depth-1, otherwise every item
// worn directly on the player would incorrectly count as already nested.
//
// Depth-1 nesting (one container inside another) is always allowed. Depth-2 nesting
// is blocked. This only triggers when the ITEM being inserted is itself a container,
// so normal gear (jackets, mags, meds, etc.) into a worn vest/backpack is unaffected.

class SRZ_ContainerNestingHelpers
{
	static bool IsContainerItem(IEntity entity)
	{
		if (!entity)
			return false;

		if (entity.FindComponent(SCR_UniversalInventoryStorageComponent))
			return true;

		if (entity.FindComponent(UniversalInventoryStorageComponent))
			return true;

		if (entity.FindComponent(ClothNodeStorageComponent))
			return true;

		return false;
	}
}

modded class SCR_UniversalInventoryStorageComponent
{
	// Returns true if 'entity' is currently stored inside another container's storage
	// (i.e. its parent slot's owner is itself a container, and NOT the player/AI
	// character's own body).
	protected bool SRZ_IsInsideAContainer(IEntity entity)
	{
		if (!entity)
			return false;

		InventoryItemComponent itemComp = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;

		InventoryStorageSlot parentSlot = itemComp.GetParentSlot();
		if (!parentSlot)
			return false; // not stored inside anything - root/ground/main inventory

		IEntity parentOwner = parentSlot.GetStorage().GetOwner(); // VERIFY this accessor chain in Workbench before deploying
		if (!parentOwner)
			return false;

		// The character's own body/main inventory is not a "container" for nesting-depth
		// purposes - being worn directly on a player/AI is depth-0, not depth-1.
		if (EntityUtils.IsPlayer(parentOwner) || SCR_ChimeraCharacter.Cast(parentOwner))
			return false;

		return SRZ_ContainerNestingHelpers.IsContainerItem(parentOwner);
	}

	protected bool SRZ_WouldNestContainer(IEntity item)
	{
		if (!item)
			return false;

		if (!SRZ_ContainerNestingHelpers.IsContainerItem(item))
			return false; // only care about containers being inserted

		IEntity destOwner = GetOwner();
		if (!destOwner)
			return false;

		if (!SRZ_ContainerNestingHelpers.IsContainerItem(destOwner))
			return false; // destination isn't a container - normal storage, allow

		// destOwner IS a container. If destOwner is ALSO already inside another
		// container (not counting the player's own body), inserting another
		// container into it creates depth-2 nesting - this is what crashes. Block it.
		return SRZ_IsInsideAContainer(destOwner);
	}

	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (SRZ_WouldNestContainer(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}

	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		if (SRZ_WouldNestContainer(nextItem))
			return false;

		return super.CanReplaceItem(nextItem, slotID);
	}
}