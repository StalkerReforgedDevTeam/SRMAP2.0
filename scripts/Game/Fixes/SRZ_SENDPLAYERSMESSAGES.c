[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SRZ_SendPlayersMessageContextAction : SCR_HealEntitiesContextAction
{
	[Attribute("", UIWidgets.EditBox, "Message to send to selected player")]
	string m_message;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;

		return selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;

		IEntity owner = selectedEntity.GetOwner();
		if (!owner)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		int playerId = pm.GetPlayerIdFromControlledEntity(owner);
		if (playerId <= 0)
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
		if (!pc)
			return;

		// Always route to the target player via RPC — works whether hosted or dedicated,
		// since SRZ_RPNet.SendToPlayer and SRZ_SendWarning both check Replication.IsServer() internally.
		SRZ_RPNet.SendToPlayer(playerId, m_message);
		pc.SRZ_SendWarning("Admin", m_message);
	}
}