// ARMST_STATIC_ANIMATION.StopAnimations calls StopLoitering on characters that were already deleted
modded class SCR_CharacterControllerComponent
{
		override void StopLoitering(bool terminateFast, SCR_LoiterCustomAnimData customAnimData = null)
	{
		if (!GetCharacter())
			return;

				super.StopLoitering(terminateFast, customAnimData);
	}
}
