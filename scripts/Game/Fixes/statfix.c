modded class ARMST_ITEMS_STATS_COMPONENTS
{
	override float GetProtectionRadiactiveLevel()
	{
		float condition = GetConditionLevel();
		if (condition <= 0)
			return 0;

		float scaled = super.GetProtectionRadiactiveLevel();
		return scaled / (condition / 100);
	}
}