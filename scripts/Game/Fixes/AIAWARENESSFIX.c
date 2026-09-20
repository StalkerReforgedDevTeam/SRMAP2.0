modded class Bacon_622120A5448725E3_AIGetShooter
{
	override protected bool ProcessEvent(AIDangerEvent dangerEvent, out IEntity target)
	{
		EAIDangerEventType eventType = dangerEvent.GetDangerType();

		if (eventType != EAIDangerEventType.Danger_DoorMovement &&
			eventType != EAIDangerEventType.Danger_WeaponFire &&
			eventType != EAIDangerEventType.BaconZombies_TargetSharing)
			return false;

		return super.ProcessEvent(dangerEvent, target);
	}
}