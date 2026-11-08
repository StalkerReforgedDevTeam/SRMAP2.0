// A character save with one item that fails to load never spawned. The failed load also deleted the save record,
// so every rejoin restored the same backup, failed again and left the player on the loading screen forever.
// Now the items that fail are dropped and the load is tried again. If it still fails the player gets a fresh character.
class SRZ_CharacterLoadGuard
{
	static bool s_bTracking;
	static bool s_bRootFailed;
	static ref set<string> s_aFailedIds;

	//------------------------------------------------------------------------------------------------
	static void Begin()
	{
		if (!s_aFailedIds)
			s_aFailedIds = new set<string>();

		s_aFailedIds.Clear();
		s_bRootFailed = false;
		s_bTracking = true;
	}

	//------------------------------------------------------------------------------------------------
	static void End()
	{
		s_bTracking = false;
	}

	//------------------------------------------------------------------------------------------------
	static void OnSpawnFailed(EPF_EntitySaveData saveData, bool isRoot)
	{
		if (!s_bTracking || !saveData)
			return;

		if (isRoot)
		{
			s_bRootFailed = true;
			return;
		}

		string id = saveData.GetId();
		if (!id.IsEmpty())
			s_aFailedIds.Insert(id);
	}

	//------------------------------------------------------------------------------------------------
	// Removes every saved item that failed to load, at any depth. Returns how many were removed.
	static int Prune(EPF_EntitySaveData saveData, string owner)
	{
		if (!saveData || !saveData.m_aComponents || !s_aFailedIds || s_aFailedIds.IsEmpty())
			return 0;

		int removed;
		foreach (EPF_PersistentComponentSaveData component : saveData.m_aComponents)
		{
			if (!component)
				continue;

			EPF_BaseInventoryStorageComponentSaveData storage = EPF_BaseInventoryStorageComponentSaveData.Cast(component.m_pData);
			if (!storage || !storage.m_aSlots)
				continue;

			for (int i = storage.m_aSlots.Count() - 1; i >= 0; i--)
			{
				EPF_PersistentInventoryStorageSlot slot = storage.m_aSlots[i];
				if (!slot || !slot.m_pEntity)
					continue;

				if (s_aFailedIds.Contains(slot.m_pEntity.GetId()))
				{
					Print(string.Format("[SRZ Save] Dropped item %1 (%2) from %3, it failed to load", slot.m_pEntity.GetId(), slot.m_pEntity.m_rPrefab, owner), LogLevel.WARNING);
					storage.m_aSlots.Remove(i);
					removed++;
					continue;
				}

				removed += Prune(slot.m_pEntity, owner);
			}
		}

		return removed;
	}
}

//------------------------------------------------------------------------------------------------
modded class EPF_PersistenceManager
{
	override IEntity SpawnWorldEntity(EPF_EntitySaveData saveData, bool isRoot = true)
	{
		IEntity entity = super.SpawnWorldEntity(saveData, isRoot);
		if (!entity)
			SRZ_CharacterLoadGuard.OnSpawnFailed(saveData, isRoot);

		return entity;
	}
}

//------------------------------------------------------------------------------------------------
modded class ARMST_BasicSpawnLogic: EPF_BasicSpawnLogic
{
	protected static const int SRZ_LOAD_ATTEMPTS = 3;

	//------------------------------------------------------------------------------------------------
	override protected void LoadCharacter(int playerId, string characterPersistenceId, EPF_CharacterSaveData saveData)
	{
		if (!saveData)
		{
			CreateCharacter(playerId, characterPersistenceId);
			return;
		}

		for (int attempt = 0; attempt < SRZ_LOAD_ATTEMPTS; attempt++)
		{
			SRZ_CharacterLoadGuard.Begin();
			super.LoadCharacter(playerId, characterPersistenceId, saveData);
			SRZ_CharacterLoadGuard.End();

			if (!SRZ_CharacterLoadGuard.s_bRootFailed)
				return;

			int removed = SRZ_CharacterLoadGuard.Prune(saveData, characterPersistenceId);
			Print(string.Format("[SRZ Save] Character %1 failed to load (try %2), dropped %3 broken item(s)", characterPersistenceId, attempt + 1, removed), LogLevel.WARNING);

			if (removed == 0)
				break;

			// The failed spawn marked the position as used, so the retry would put the character at the map origin.
			if (saveData.m_pTransformation)
				saveData.m_pTransformation.m_bApplied = false;
		}

		Print(string.Format("[SRZ Save] Character %1 could not be loaded, giving the player a fresh character so they are not stuck loading", characterPersistenceId), LogLevel.ERROR);
		CreateCharacter(playerId, characterPersistenceId);
	}
}
