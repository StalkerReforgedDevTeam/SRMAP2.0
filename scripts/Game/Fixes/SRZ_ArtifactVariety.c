// Artifact variety: fields don't keep giving the same artifact.
// A field remembers its last picks, the server remembers the last picks everywhere,
// and some spawns are "wild" finds from any anomaly type of a fitting tier.
modded class ARMST_TRIGGER_SPAWN
{
	protected static const float SRZ_WILD_CHANCE = 0.3;
	protected static const int SRZ_FIELD_MEMORY = 4;
	protected static const int SRZ_GLOBAL_MEMORY = 10;

	protected static ref array<ResourceName> s_aSRZ_GlobalRecent = {};
	protected ref array<ResourceName> m_aSRZ_FieldRecent = {};

	//------------------------------------------------------------------------------------------------
	override protected ResourceName GetRandomPrefab()
	{
		if (!m_ArtsPrefab || m_ArtsPrefab.IsEmpty())
			return super.GetRandomPrefab();

		ResourceName pick;
		for (int attempt = 0; attempt < 8; attempt++)
		{
			pick = ResourceName.Empty;
			if (Math.RandomFloat01() < SRZ_WILD_CHANCE)
				pick = SRZ_PickWild();

			if (pick.IsEmpty())
				pick = m_ArtsPrefab.GetRandomElement();

			if (!m_aSRZ_FieldRecent.Contains(pick) && !s_aSRZ_GlobalRecent.Contains(pick))
				break;
		}

		SRZ_Remember(m_aSRZ_FieldRecent, pick, SRZ_FIELD_MEMORY);
		SRZ_Remember(s_aSRZ_GlobalRecent, pick, SRZ_GLOBAL_MEMORY);
		return pick;
	}

	//------------------------------------------------------------------------------------------------
	// Any anomaly type, weighted by how rare that artifact should be in a field of this tier
	protected ResourceName SRZ_PickWild()
	{
		int tier = 1;
		array<ResourceName> anomalies = {groupPrefab, m_ObjectPrefab, m_ObjectPrefab2, m_ObjectPrefab3};
		foreach (ResourceName anomaly : anomalies)
		{
			if (!SRZ_NewArtifacts.CategoryOf(anomaly).IsEmpty())
				tier = Math.Max(tier, SRZ_NewArtifacts.TierOf(anomaly));
		}

		if (m_iSRZ_FieldTier > 0)
			tier = m_iSRZ_FieldTier;

		array<ref SRZ_ArtEntry> arts = SRZ_NewArtifacts.GetArts();
		int total;
		foreach (SRZ_ArtEntry art : arts)
		{
			total += SRZ_NewArtifacts.Copies(tier, (int)art.m_fValue);
		}

		if (total <= 0)
			return ResourceName.Empty;

		int roll = Math.RandomInt(0, total);
		foreach (SRZ_ArtEntry candidate : arts)
		{
			roll -= SRZ_NewArtifacts.Copies(tier, (int)candidate.m_fValue);
			if (roll < 0)
				return candidate.m_sPrefab;
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SRZ_Remember(array<ResourceName> list, ResourceName pick, int size)
	{
		if (pick.IsEmpty())
			return;

		list.Insert(pick);
		while (list.Count() > size)
			list.RemoveOrdered(0);
	}
}
