// The Telepach's psy aura built up far too fast and killed players in seconds.
// Psy damage coming from a Telepach is capped here. Raise or lower SRZ_TELEPACH_MAX_DPS to tune it (ARMST max is 100).
modded class SCR_PsyDamageEffect
{
	protected static const float SRZ_TELEPACH_MAX_DPS = 25;
	protected static int s_iSRZ_DebugPrints;
	protected int m_iSRZ_FromTelepach = -1;

	//------------------------------------------------------------------------------------------------
	override void RecalculateDPS(float timeSlice, notnull SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.RecalculateDPS(timeSlice, dmgManager);

		if (m_fLocalDPSValue <= SRZ_TELEPACH_MAX_DPS || !SRZ_FromTelepach())
			return;

		m_fLocalDPSValue = SRZ_TELEPACH_MAX_DPS;
		SetDPS(m_fLocalDPSValue);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_FromTelepach()
	{
		if (m_iSRZ_FromTelepach >= 0)
			return m_iSRZ_FromTelepach == 1;

		m_iSRZ_FromTelepach = 0;
		Instigator instigator = GetInstigator();
		if (!instigator)
			return false;

		IEntity source = instigator.GetInstigatorEntity();
		if (!source)
			return false;

		string path = SRZ_PrefabPath(source) + " " + SRZ_PrefabPath(source.GetRootParent());
		if (path.Contains("psy_destroyer"))
			m_iSRZ_FromTelepach = 1;

		if (s_iSRZ_DebugPrints < 20)
		{
			s_iSRZ_DebugPrints++;
			Print("[SRZ_TelepachFix] psy damage source: " + path + " capped: " + (m_iSRZ_FromTelepach == 1));
		}

		return m_iSRZ_FromTelepach == 1;
	}

	//------------------------------------------------------------------------------------------------
	protected string SRZ_PrefabPath(IEntity ent)
	{
		if (!ent || !ent.GetPrefabData())
			return "";

		string path = ent.GetPrefabData().GetPrefabName();
		path.ToLower();
		return path;
	}
}
