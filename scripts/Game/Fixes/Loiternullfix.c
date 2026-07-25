// scripts/Game/Fixes/SRZ_LoiterCommandFix.c

modded class SCR_CharacterCommandHandlerComponent
{
    //------------------------------------------------------------------------------------------------
    protected override SCR_CharacterCommandLoiter GetLoiterCommand()
    {
        if (!m_CharacterAnimComp)
            return null;

        AnimPhysCommandScripted currentCmdScripted = m_CharacterAnimComp.GetCommandScripted();
        if (!currentCmdScripted)
            return null;

        return SCR_CharacterCommandLoiter.Cast(currentCmdScripted);
    }
};