// Check injuries shows the injuries only, never who the casualty is.
modded class SCR_InspectCasualtyWidget
{
	override protected void GetCasualtyName(inout string sName, IEntity targetCharacter)
	{
		sName = "";
	}
}
