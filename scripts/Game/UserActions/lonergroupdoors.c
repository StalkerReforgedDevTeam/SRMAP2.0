[ComponentEditorProps(category: "SRZ/Access", description: "Lock a door to a list of player GUIDs")]
class SRZ_DoorAccessComponentClass : ScriptComponentClass
{
}

class SRZ_DoorAccessComponent : ScriptComponent
{
	[Attribute("door", UIWidgets.EditBox, "Name shown in server logs, e.g. loner_base_north")]
	protected string m_sDoorId;

	[Attribute("0", UIWidgets.CheckBox, "Only players in the list below can open this door")]
	protected bool m_bLocked;

	[Attribute(desc: "Player GUIDs allowed to open this door")]
	protected ref array<string> m_aAllowedPlayers;

	[Attribute("1", UIWidgets.CheckBox, "Server admins can always open it")]
	protected bool m_bAllowAdmins;

	bool IsLocked()
	{
		return m_bLocked;
	}

	RplId GetDoorRplId()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return RplId.Invalid();
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl)
			return RplId.Invalid();
		return Replication.FindItemId(rpl);
	}

	bool IsInRange(IEntity user)
	{
		IEntity owner = GetOwner();
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!owner || !character || !character.GetCharacterController())
			return false;
		CharacterControllerComponent controller = character.GetCharacterController();
		if (controller.IsDead() || controller.IsUnconscious())
			return false;
		return vector.Distance(user.GetOrigin(), owner.GetOrigin()) <= 4.0;
	}

	bool ServerTryUse(SCR_PlayerController requester)
	{
		if (!Replication.IsServer() || !requester || !GetGame() || !GetGame().GetPlayerManager())
			return false;

		int playerId = requester.GetPlayerId();
		IEntity character = requester.GetControlledEntity();
		if (playerId <= 0 || GetGame().GetPlayerManager().GetPlayerController(playerId) != requester || !IsInRange(character))
			return false;

		if (m_bLocked && !IsAllowed(playerId))
			return false;

		BaseDoorComponent door = BaseDoorComponent.Cast(GetOwner().FindComponent(SlidingDoorComponent));
		if (!door)
			door = BaseDoorComponent.Cast(GetOwner().FindComponent(DoorComponent));
		if (!door)
			return false;

		door.UseDoorAction(character);
		return true;
	}

	protected bool IsAllowed(int playerId)
	{
		PlayerManager manager = GetGame().GetPlayerManager();
		if (m_bAllowAdmins && (manager.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR)
			|| manager.HasPlayerRole(playerId, EPlayerRole.SESSION_ADMINISTRATOR)))
			return true;

		string guid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!guid.IsEmpty() && m_aAllowedPlayers)
		{
			foreach (string allowed : m_aAllowedPlayers)
			{
				if (allowed.Trim() == guid)
					return true;
			}
		}

		PrintFormat("SRZ_DOOR_DENIED|door=%1|player=%2|name=%3|guid=%4",
			m_sDoorId, playerId, manager.GetPlayerName(playerId), guid);
		return false;
	}
}

modded class SCR_PlayerController
{
	protected int m_iSRZ_LastDoorRequestMs;

	void SRZ_RequestDoorUse(RplId doorId)
	{
		if (Replication.IsServer())
			SRZ_RpcAsk_DoorUse(doorId);
		else
			Rpc(SRZ_RpcAsk_DoorUse, doorId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_DoorUse(RplId doorId)
	{
		if (!Replication.IsServer() || doorId == RplId.Invalid() || !GetControlledEntity())
			return;

		int now = System.GetTickCount();
		if (m_iSRZ_LastDoorRequestMs != 0 && now - m_iSRZ_LastDoorRequestMs >= 0 && now - m_iSRZ_LastDoorRequestMs < 250)
			return;
		m_iSRZ_LastDoorRequestMs = now;

		RplComponent rpl = RplComponent.Cast(Replication.FindItem(doorId));
		if (!rpl || !rpl.GetEntity())
			return;
		SRZ_DoorAccessComponent access = SRZ_DoorAccessComponent.Cast(rpl.GetEntity().FindComponent(SRZ_DoorAccessComponent));
		if (!access || access.ServerTryUse(this))
			return;

		if (RplSession.Mode() == RplMode.None || (GetGame() && GetGame().GetPlayerController() == this))
			SRZ_ShowDoorLocked();
		else
			Rpc(SRZ_RpcDo_DoorLocked);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SRZ_RpcDo_DoorLocked()
	{
		SRZ_ShowDoorLocked();
	}

	protected void SRZ_ShowDoorLocked()
	{
		SCR_HintManagerComponent.ShowCustomHint("This door is locked.", "Locked", 3);
	}
}

modded class SCR_DoorUserAction
{
	protected SRZ_DoorAccessComponent SRZ_GetActiveLock(IEntity owner)
	{
		if (!owner)
			return null;
		SRZ_DoorAccessComponent access = SRZ_DoorAccessComponent.Cast(owner.FindComponent(SRZ_DoorAccessComponent));
		if (access && access.IsLocked())
			return access;
		return null;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SRZ_DoorAccessComponent access = SRZ_GetActiveLock(pOwnerEntity);
		if (!access)
		{
			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		if (!GetGame() || !access.IsInRange(pUserEntity))
			return;
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && controller.GetControlledEntity() == pUserEntity)
			controller.SRZ_RequestDoorUse(access.GetDoorRplId());
	}

	override bool HasLocalEffectOnlyScript()
	{
		if (SRZ_GetActiveLock(GetOwner()))
			return true;
		return super.HasLocalEffectOnlyScript();
	}
}