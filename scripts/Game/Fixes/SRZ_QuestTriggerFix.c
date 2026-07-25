// scripts/Game/Fixes/SRZ_QuestTriggerFix.c

modded class ARMST_QUEST_TRIGGER
{
    //------------------------------------------------------------------------------------------------
    override void OnActivate(IEntity ent)
    {
        if (!ent)
            return;

        // Проверка: это игрок
        if (!EntityUtils.IsPlayer(ent))
            return;

        // Проверка: это персонаж
        SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
        if (!character)
            return;

        // Проверка: контроллер персонажа существует (устраняет NULL pointer crash)
        CharacterControllerComponent charController = character.GetCharacterController();
        if (!charController)
            return;

        // Проверка: игрок жив
        if (charController.GetLifeState() != ECharacterLifeState.ALIVE)
            return;

        if (m_iStageStart > 0)
        {
            HandleQuestTrigger(ent);
        }
        else
        {
            HandleQuestTriggerFirst(ent);
        }
    }
};