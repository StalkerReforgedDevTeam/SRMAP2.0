class RTTZ_DeletePlayerConfirmDialog : SCR_ConfigurableDialogUi
{
	protected SCR_EditableEntityComponent m_Entity;

	void SetEntity(SCR_EditableEntityComponent entity)
	{
		m_Entity = entity;
	}

	override protected void OnConfirm()
	{
		if (m_Entity)
			m_Entity.RTTZ_RequestDelete();

		super.OnConfirm();
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
modded class SCR_DeleteSelectedContextAction
{
	protected bool RTTZ_IsPlayerEntity(SCR_EditableEntityComponent selectedEntity)
	{
		if (!selectedEntity)
			return false;

		set<SCR_EditableEntityComponent> children = new set<SCR_EditableEntityComponent>();
		selectedEntity.GetChildren(children);
		children.Insert(selectedEntity);

		foreach (SCR_EditableEntityComponent child : children)
		{
			if (!child)
				continue;

			IEntity owner = child.GetOwner();
			if (!owner)
				continue;

			SCR_PossessingManagerComponent possessionManager = SCR_PossessingManagerComponent.GetInstance();
			if (possessionManager && possessionManager.GetIdFromMainEntity(owner) != 0)
				return true;

			if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner) != 0)
				return true;
		}

		return false;
	}

	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;

		if (RTTZ_IsPlayerEntity(selectedEntity))
			return;

		super.Perform(selectedEntity, cursorWorldPosition);
	}

	override void PerformOwner(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;

		if (!RTTZ_IsPlayerEntity(selectedEntity))
			return;

		RTTZ_DeletePlayerConfirmDialog dialog = new RTTZ_DeletePlayerConfirmDialog();
		dialog.SetEntity(selectedEntity);
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "delete_player", dialog);
	}
}

modded class SCR_EditableEntityComponent
{
	void RTTZ_RequestDelete()
	{
		if (Replication.IsServer())
		{
			Delete(true, true);
			return;
		}

		Rpc(Rpc_RTTZ_DeleteEntity);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_RTTZ_DeleteEntity()
	{
		Delete(true, true);
	}
}