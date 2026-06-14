// Fix for loiter command derefing a null pointer without a check
modded class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent
{
	protected override SCR_CharacterCommandLoiter GetLoiterCommand()
	{
		// Check nullptr before attempting to call into super, as super has no early outs
		if (m_CharacterAnimComp == null)
			return null;
		
		return super.GetLoiterCommand();
	}
}