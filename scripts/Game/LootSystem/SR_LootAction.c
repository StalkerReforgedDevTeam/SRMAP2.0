// SR_LootAction.c
//
// Player-facing "open loot container" action. This is what actually
// triggers the loot roll (first open) or the respawn-timer restart
// (subsequent opens) via SR_LootRollComponent.OnPlayerInteracted() -
// see that file's header for the full reasoning.
//
// STRUCTURE MIRRORS your working ARMST_OpenStorageAction example closely
// (Init() caching the component ref, OpenInventoryForUser doing the actual
// UI opening) since that pattern is already proven in your project.
//
// ASSUMPTION I'm flagging explicitly: I could not independently confirm
// from documentation whether ScriptedUserAction.PerformAction() is
// guaranteed server-only by the action framework itself, or whether it
// runs on whichever machine triggered it with results replicated after.
// Your own reference example does NOT guard its Spawn() call with
// Replication.IsServer() inside PerformAction, which suggests it may
// already be server-authoritative by the framework - but I'd rather add
// the guard defensively (cheap, and OnPlayerInteracted() already has its
// own internal Replication.IsServer() check too, so this is belt-and-braces,
// not strictly required twice). If you find PerformAction is already
// server-only via the framework, the inner check in OnPlayerInteracted()
// alone is sufficient and this outer one is just harmless redundancy.

class SR_OpenLootAction : ScriptedUserAction
{
	protected SR_LootRollComponent m_LootComponent;

	//------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_LootComponent = SR_LootRollComponent.Cast(pOwnerEntity.FindComponent(SR_LootRollComponent));
	}

	//------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;
		if (m_LootComponent)
			m_LootComponent.RollLoot(); // first open = rolls loot; later opens = restarts respawn timer

		OpenInventoryForUser(pUserEntity, pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------
	// Mirrors your working reference's OpenInventoryForUser exactly -
	// local-player check, then hand off to the player's own inventory
	// manager to open the container's storage.
	protected void OpenInventoryForUser(IEntity userEntity, IEntity ownerEntity)
	{
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		IEntity localPlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerId);
		if (!localPlayerEntity)
			return;

		if (userEntity != localPlayerEntity)
			return;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(userEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
		{
			Print("[SR_OpenLootAction] Error: No inventory manager found.", LogLevel.ERROR);
			return;
		}

		inventoryManager.SetStorageToOpen(ownerEntity);
		inventoryManager.OpenInventory();
	}

	//------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_LootComponent)
			return false;
	
		return m_LootComponent.IsLootAvailable();
	}

	// TODO consider adding CanBeShownScript with a lock/cooldown flag if you
	// want the action itself hidden while, e.g., a respawn is mid-transition -
	// your reference example does exactly this with m_lock_ent. Not included
	// here since SR_LootRollComponent doesn't currently expose a "is rolling"
	// state for the action to query - easy to add if you want that polish.
};