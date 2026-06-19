[BaseContainerProps()]
class SR_PlayerRankAttribute : SCR_BaseFloatValueHolderEditorAttribute // Inheriting here unlocks the config values array
{
	// Keep your metadata declaration for safe measures
	[Attribute("0", UIWidgets.ComboBox, "Select the player's custom STALKER rank.", enums: ParamEnumArray.FromEnum(SR_STALKER_RANK))]
	protected SR_STALKER_RANK m_eSelectedRank;

	//------------------------------------------------------------------------------------------------
	// ENGINE READ: Determines if the widget shows up and defines its initial selection state
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return null; 

		IEntity targetEntity = editableEntity.GetOwner();
		ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(targetEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		
		if (!statsComp)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetEntity);
			if (playerId > 0)
			{
				PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
				if (playerController)
					statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(playerController.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			}
		}

		if (!statsComp)
			return null;

		SR_STALKER_RANK currentRank = statsComp.SR_GetRank();
		
		// Tells the UI layout exactly which Integer index is currently selected
		return SCR_BaseEditorAttributeVar.CreateInt(currentRank); 
	}

	//------------------------------------------------------------------------------------------------
	// ENGINE WRITE: Fires on the server when the GM selects a rank and clicks "Apply"
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return;

		IEntity targetEntity = editableEntity.GetOwner();
		ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(targetEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		
		if (!statsComp)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetEntity);
			if (playerId > 0)
			{
				PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
				if (playerController)
					statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(playerController.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			}
		}

		if (!statsComp)
			return;

		SR_STALKER_RANK UIChosenRank = var.GetInt();
		statsComp.SR_SetRank(UIChosenRank);
	}
}
