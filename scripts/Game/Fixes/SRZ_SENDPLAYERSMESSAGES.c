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

        SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

        // Hosted/editor mode — show directly on local client
        if (gameMode && gameMode.IsHosted())
        {
            // PDA notification
            SCR_PlayerController.ShowNotificationPDA(null, "Admin", m_message);

            // Center screen popup
            SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
            if (popup)
                popup.PopupMsg(m_message, 10.0);

            return;
        }

        // Dedicated server — send via RPC to target player
        SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(playerId));
        if (!pc)
            return;

        // PDA message
        SRZ_RPNet.SendToPlayer(playerId, m_message);

        // Center screen warning popup via RPC
        pc.SRZ_SendWarning("Admin", m_message);
    }
}