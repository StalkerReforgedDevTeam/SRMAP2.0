modded class ARMST_RANDOM_LOOTS_POINT
{
	override void Setup()
	{
		if (!m_ListsPrefab || m_ListsPrefab.IsEmpty())
		{
			if (GetOwner())
				Print(string.Format("[SRZ Loot] loot point with no prefabs at %1, skipped", GetOwner().GetOrigin()), LogLevel.WARNING);

			return;
		}

		super.Setup();
	}
}
