

[BaseContainerProps()]
class ADB_TeleportUserAction : ScriptedUserAction
{
	[Attribute("Teleport", UIWidgets.EditBox, "Action name")]
    protected string m_sTeleportActionName;
	
	[Attribute("", UIWidgets.EditBox, "The map entity name for the location of where the player will teleport to")]
    protected string m_sTeleportTarget;
	
	[Attribute()]
	protected ref array<ref ADB_AttributeString> m_requiredAttributeStrings;
	
	void ADB_TeleportUserAction()
	{
		m_requiredAttributeStrings = new array<ref ADB_AttributeString>();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!user)
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
			foreach (auto attr : m_requiredAttributeStrings)
			{
				if (!playerAttrComponent.DoesAttrHaveValue(attr.key, attr.value))
					return false;
			}
		}
		
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		IEntity targetEntity = GetGame().FindEntity(m_sTeleportTarget);
        if (!targetEntity)
        {
            return;
        }
		
		vector targetPos = targetEntity.GetOrigin();
		pUserEntity.SetOrigin(targetPos);
	}
}