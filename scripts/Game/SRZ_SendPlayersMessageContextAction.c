// Relatively simple context menu ability to send messages to players that will show up on their UI
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SRZ_SendPlayersMessageContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute()]
	protected string m_message;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!hoveredEntity)
			return false;
		
		GenericEntity owner = hoveredEntity.GetOwner();
		if (!SCR_ChimeraCharacter.Cast(owner))
			return false;
		
		// Only able to do on players
		if (!EntityUtils.IsPlayer(owner))
			return false;
		
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!InitPerform()) 
			return;
		
		// Grab the local player controller so we can send an RPC via it
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
		{
			return;
		}
		
		// Iterate over all the selected entities, get their player IDs and then send that off with the message to the server
		array<int> playerIdsSelected = {};		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			if (CanBePerformed(entity, cursorWorldPosition, flags))
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity.GetOwner());
				if (!character)
					continue;
				
				int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
				playerIdsSelected.Insert(playerId);
			}
		}
		
		pc.SRZ_ShowMessageToPlayers(playerIdsSelected, m_message);
	}
}