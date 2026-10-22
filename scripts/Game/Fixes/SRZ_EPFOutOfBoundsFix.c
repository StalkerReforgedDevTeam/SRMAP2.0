// A saved item that fell through the map (y around -150000) crashed the load with "Entity out of world bounds".
// Saved world entities with a position that far outside the map are skipped instead of spawned.
modded class EPF_PersistenceManager
{
	protected static const float SRZ_MIN_HEIGHT = -1000;
	protected static const float SRZ_MAX_HEIGHT = 20000;
	protected static const float SRZ_MAX_HORIZONTAL = 100000;

	//------------------------------------------------------------------------------------------------
	override IEntity SpawnWorldEntity(EPF_EntitySaveData saveData, bool isRoot = true)
	{
		if (saveData && saveData.m_pTransformation && !saveData.m_pTransformation.m_bApplied)
		{
			vector origin = saveData.m_pTransformation.m_vOrigin;
			if (!EPF_Const.IsUnset(origin) && SRZ_IsOutOfBounds(origin))
			{
				Print(string.Format("[SRZ Save] Skipped saved entity %1 (%2) at %3, outside the map", saveData.GetId(), saveData.m_rPrefab, origin), LogLevel.WARNING);
				return null;
			}
		}

		return super.SpawnWorldEntity(saveData, isRoot);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_IsOutOfBounds(vector origin)
	{
		if (origin[1] < SRZ_MIN_HEIGHT || origin[1] > SRZ_MAX_HEIGHT)
			return true;

		return Math.AbsFloat(origin[0]) > SRZ_MAX_HORIZONTAL || Math.AbsFloat(origin[2]) > SRZ_MAX_HORIZONTAL;
	}
}
