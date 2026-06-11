  [ComponentEditorProps()]
modded class ARMST_QUEST_TRIGGER : SCR_BaseTriggerEntity
{
    //------------------------------------------------------------------------------------------------
    override void OnActivate(IEntity ent)
    {
        if (!ent)
            return;
        
        // Проверка: это игрок
        if (!EntityUtils.IsPlayer(ent))
            return;
        
        
        // Проверка: игрок жив
        SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
        
        if (!character)
            return;
        
        CharacterControllerComponent charController = character.GetCharacterController();
        
        if (!charController)
            return;
       
        if (!charController.GetLifeState() != ECharacterLifeState.ALIVE)
            return;
        
        if(m_iStageStart > 0)
        {
                HandleQuestTrigger(ent);
        }
        else
        {
                HandleQuestTriggerFirst(ent);
        }
        

    }


};