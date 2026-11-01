// Per item insulation, edited in Configs/SRZ_Sickness/SRZ_Insulation.conf. Our own config, nothing of ARMST's is touched.
// An item that isn't listed, or is listed at 0, falls back to the slot default plus half its fire protection.
[BaseContainerProps()]
class SRZ_InsulationEntry
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Clothing prefab", "et")]
	ResourceName m_sPrefab;

	[Attribute("0", UIWidgets.Slider, "Insulation 0 to 100, 0 = slot default", "0 100 1")]
	float m_fInsulation;

	[Attribute("false", UIWidgets.CheckBox, "Waterproof, halves rain wetness")]
	bool m_bWaterproof;
}

[BaseContainerProps(configRoot: true)]
class SRZ_InsulationConfig
{
	[Attribute("", UIWidgets.Object, "Items")]
	ref array<ref SRZ_InsulationEntry> m_aItems;

	protected static const ResourceName CONF = "{A0FE38841C1CDF22}Configs/SRZ_Sickness/SRZ_Insulation.conf";
	protected static ref map<string, ref SRZ_InsulationEntry> s_mByPrefab;

	//------------------------------------------------------------------------------------------------
	static SRZ_InsulationEntry Get(ResourceName prefab)
	{
		if (!s_mByPrefab)
			Load();

		return s_mByPrefab.Get(prefab);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Load()
	{
		s_mByPrefab = new map<string, ref SRZ_InsulationEntry>();

		Resource res = BaseContainerTools.LoadContainer(CONF);
		if (!res || !res.IsValid())
		{
			Print("[SRZ Sickness] Insulation conf missing, using slot defaults", LogLevel.WARNING);
			return;
		}

		SRZ_InsulationConfig cfg = SRZ_InsulationConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(res.GetResource().ToBaseContainer()));
		if (!cfg || !cfg.m_aItems)
			return;

		foreach (SRZ_InsulationEntry e : cfg.m_aItems)
		{
			if (!e.m_sPrefab.IsEmpty())
				s_mByPrefab.Set(e.m_sPrefab, e);
		}

		Print("[SRZ Sickness] Insulation conf loaded, " + s_mByPrefab.Count().ToString() + " items");
	}
}
