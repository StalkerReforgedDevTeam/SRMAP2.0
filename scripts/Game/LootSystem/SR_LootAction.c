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
			m_LootComponent.CreateLoot(); 

		OpenInventoryForUser(pUserEntity, pOwnerEntity);
	} 

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

};
