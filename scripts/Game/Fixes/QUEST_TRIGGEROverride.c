modded class ARMST_QUEST_TRIGGER
{
	override void OnActivate(IEntity ent)
	{
		if (!ent)
			return;

		if (!GetGame() || !GetGame().InPlayMode())
			return;

		// Safety check: player only
		if (!EntityUtils.IsPlayer(ent))
			return;

		// Optional delay to prevent join crash
		GetGame().GetCallqueue().CallLater(FixedActivate, 50, false, ent);
	}

	void FixedActivate(IEntity ent)
	{
		if (!ent)
			return;

		// CALL ORIGINAL LOGIC
		super.OnActivate(ent);
	}
}