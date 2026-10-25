// Use a key or keycard on a locked door or container
class SRZ_UnlockAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		return lockComp && lockComp.IsLocked() && lockComp.HasKeys();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (!lockComp || !lockComp.IsLocked())
			return false;

		if (!lockComp.FindKey(user))
		{
			SetCannotPerformReason("Locked");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(pOwnerEntity);
		if (lockComp)
			lockComp.UnlockWithKey(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.HasKeys())
			outName = "Unlock (" + lockComp.GetKeyName() + ")";
		else
			outName = "Unlock";

		return true;
	}
}

// Pick a lock. The 15 second hold is the action's Duration in the prefab.
class SRZ_PickLockAction : ScriptedUserAction
{
	static const ResourceName PICK_SOUND = "{A2D45F6B2FC200AC}Sounds/SRZ/Lockpick/SRZ_Lockpick.acp";
	static const int PICK_SOUND_LENGTH_MS = 2400;

	protected AudioHandle m_hPickSound = AudioHandle.Invalid;
	protected int m_iPickLoopsLeft;

	//------------------------------------------------------------------------------------------------
	override protected void OnActionStart(IEntity pUserEntity)
	{
		StopPickSound();

		float duration = GetActionDuration();
		if (duration <= 0)
			duration = 15;

		m_iPickLoopsLeft = Math.Ceil(duration * 1000 / PICK_SOUND_LENGTH_MS);
		PlayPickSound();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		StopPickSound();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayPickSound()
	{
		if (m_iPickLoopsLeft <= 0 || !GetOwner())
			return;

		m_iPickLoopsLeft--;
		m_hPickSound = SRZ_FXRange.Sound(PICK_SOUND, "SOUND_SRZ_LOCKPICK", GetOwner().GetOrigin(), 50);
		GetGame().GetCallqueue().CallLater(PlayPickSound, PICK_SOUND_LENGTH_MS, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void StopPickSound()
	{
		m_iPickLoopsLeft = 0;
		GetGame().GetCallqueue().Remove(PlayPickSound);
		SRZ_FXRange.StopSound(m_hPickSound);
		m_hPickSound = AudioHandle.Invalid;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		return lockComp && lockComp.IsLocked() && lockComp.AllowsLockpick();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!SRZ_LockComponent.FindLockpick(user))
		{
			SetCannotPerformReason("Hold a lockpick in your hand");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		StopPickSound();

		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(pOwnerEntity);
		if (lockComp)
			lockComp.UnlockWithLockpick(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Pick lock";
		return true;
	}
}

// Doors with a lock can't be opened until unlocked
modded class SCR_DoorUserAction : DoorUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.IsLocked())
		{
			SetCannotPerformReason("Locked");
			return false;
		}

		return super.CanBePerformedScript(user);
	}
}

// Loot crates with a lock hide their search action until unlocked
modded class ARMST_OpenStorageAction : ScriptedUserAction
{
	protected bool m_bSRZ_Searched;

	// Locked crates fill once on the server, then Search just reopens them so nothing is lost after closing
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!SRZ_LockComponent.Get(pOwnerEntity))
		{
			// Already searched and waiting on the respawn timer, so just open it again
			if (!m_lock_ent)
			{
				OpenInventoryForUser(pUserEntity, pOwnerEntity);
				return;
			}

			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		if (Replication.IsServer() && !m_bSRZ_Searched)
		{
			m_bSRZ_Searched = true;
			BuildValidLootItemsFromDatabase();
			TryToSpawnLoot();
		}

		OpenInventoryForUser(pUserEntity, pOwnerEntity);
	}

	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.IsLocked())
			return false;

		// ARMST hides Search after the first open, which left no way back in to loot the rest.
		// Keep it while something is left inside, hide it once looted empty until respawn.
		if (!m_lock_ent && !lockComp && SRZ_IsEmpty())
			return false;

		return CanBePerformedScript(user);
	}

	protected bool SRZ_IsEmpty()
	{
		InventoryStorageManagerComponent storage = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
		if (!storage)
			return true;

		array<IEntity> items = {};
		storage.GetItems(items);
		return items.IsEmpty();
	}
}

// Hide vanilla Open on locked crates, and on any ARMST loot container, where it would show the box before Search fills it
modded class SCR_OpenStorageAction
{
	protected int m_iSRZ_HasArmstSearch = -1;

	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.IsLocked())
			return false;

		if (SRZ_HasArmstSearch())
			return false;

		return super.CanBeShownScript(user);
	}

	protected bool SRZ_HasArmstSearch()
	{
		if (m_iSRZ_HasArmstSearch < 0)
		{
			m_iSRZ_HasArmstSearch = 0;
			ActionsManagerComponent mgr = ActionsManagerComponent.Cast(GetOwner().FindComponent(ActionsManagerComponent));
			if (mgr)
			{
				array<BaseUserAction> actions = {};
				mgr.GetActionsList(actions);
				foreach (BaseUserAction action : actions)
				{
					if (ARMST_OpenStorageAction.Cast(action))
					{
						m_iSRZ_HasArmstSearch = 1;
						break;
					}
				}
			}
		}

		return m_iSRZ_HasArmstSearch == 1;
	}
}
