[BaseContainerProps()]
class ADB_DoorUserAction : SCR_DoorUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		// Always respect base door rules (distance, locked state, etc.)
		if (!super.CanBePerformedScript(user))
			return false;
		
		if (!user)
			return false;
		
		// So, I did try to encorporate the editable attribute list into this action, but the editor kept crashing.
		// This is why we settle for a separate component. It is what it is.
		ADB_AttributeListComponent attrListComponent = ADB_AttributeListComponent.Cast(GetOwner().FindComponent(ADB_AttributeListComponent));
		if (!attrListComponent)
			return false;
		
		if (!attrListComponent.stringAttributes)
			return false;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		// Get the controller of the player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		IEntity controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return false;
		
		ADB_PlayerAttrComponent playerAttrComponent = ADB_PlayerAttrComponent.Cast(controller.FindComponent(ADB_PlayerAttrComponent));
		if (playerAttrComponent)
		{
			foreach (auto attr : attrListComponent.stringAttributes)
			{
				if (!playerAttrComponent.DoesAttrHaveValue(attr.key, attr.value))
					return false;
			}
		}
		
		return true;
	}
}