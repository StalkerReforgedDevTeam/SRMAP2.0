// The Vortex was only placed as an option on one field, so it almost never showed up.
// Tier 2 and 3 gravity fields and whirligig fields now roll a chance to become a Vortex instead.
modded class ARMST_TRIGGER_SPAWN
{
	protected static const ResourceName SRZ_VORTEX = "{F7182983E3890A96}Prefabs/Anomals/Vortex/SRZ_VORTEX.et";
	protected static const float SRZ_VORTEX_CHANCE = 0.25;

	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		if (SCR_Global.IsEditMode() || !GetGame().InPlayMode() || !Replication.IsServer() || IsDeleted())
			return;

		if (groupPrefab.IsEmpty())
			return;

		string path = groupPrefab;
		path.ToLower();
		bool gravity = path.Contains("gravy_tier2") || path.Contains("gravy_tier3") || path.Contains("whirligig");
		if (gravity && Math.RandomFloat01() < SRZ_VORTEX_CHANCE)
			groupPrefab = SRZ_VORTEX;
	}
}
