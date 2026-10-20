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
			SetCannotPerformReason("Hold the " + lockComp.GetKeyName() + " in your hand");
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
	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.IsLocked())
			return false;

		return super.CanBeShownScript(user);
	}
}

// Vanilla open storage on locked crates too
modded class SCR_OpenStorageAction
{
	override bool CanBeShownScript(IEntity user)
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && lockComp.IsLocked())
			return false;

		return super.CanBeShownScript(user);
	}
}
