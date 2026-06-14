modded class ARMST_ARTEFACT_PROPERTIES : ScriptComponent
{
    //------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        if (!GetGame().InPlayMode())
            return;

        if (!owner)
            return;

        LoadParamsFromDatabase();
        GenerateRandomProperties();
    }

    //------------------------------------------------------------------------------------------------
    override void GenerateRandomProperties()
    {
        if (m_bPropertiesToxicLevelEnable)
            m_fPropertiesToxicLevel = GenerateRoundedValue(m_fPropertiesToxicLevelMin, m_fPropertiesToxicLevelMax);
        else
            m_fPropertiesToxicLevel = 0.0;

        if (m_bPropertiesRadiactiveLevelEnable)
            m_fPropertiesRadiactiveLevel = GenerateRoundedValue(m_fPropertiesRadiactiveLevelMin, m_fPropertiesRadiactiveLevelMax);
        else
            m_fPropertiesRadiactiveLevel = 0.0;

        if (m_bPropertiesPsyLevelEnable)
            m_fPropertiesPsyLevel = GenerateRoundedValue(m_fPropertiesPsyLevelMin, m_fPropertiesPsyLevelMax);
        else
            m_fPropertiesPsyLevel = 0.0;

        if (m_bPropertiesPhysicalsLevelEnable)
            m_fPropertiesPhysicalsLevel = GenerateRoundedValue(m_fPropertiesPhysicalsLevelMin, m_fPropertiesPhysicalsLevelMax);
        else
            m_fPropertiesPhysicalsLevel = 0.0;

        if (m_bPropertiesThermoLevelEnable)
            m_fPropertiesThermoLevel = GenerateRoundedValue(m_fPropertiesThermoLevelMin, m_fPropertiesThermoLevelMax);
        else
            m_fPropertiesThermoLevel = 0.0;

        if (m_bPropertiesElectroLevelEnable)
            m_fPropertiesElectroLevel = GenerateRoundedValue(m_fPropertiesElectroLevelMin, m_fPropertiesElectroLevelMax);
        else
            m_fPropertiesElectroLevel = 0.0;

        if (m_bPropertiesHealthLevelEnable)
            m_fPropertiesHealthLevel = GenerateRoundedValue(m_fPropertiesHealthLevelMin, m_fPropertiesHealthLevelMax);
        else
            m_fPropertiesHealthLevel = 0.0;

        if (m_bPropertiesBloodLevelEnable)
            m_fPropertiesBloodLevel = GenerateRoundedValue(m_fPropertiesBloodLevelMin, m_fPropertiesBloodLevelMax);
        else
            m_fPropertiesBloodLevel = 0.0;

        if (m_bPropertiesStaminaLevelEnable)
            m_fPropertiesStaminaLevel = GenerateRoundedValue(m_fPropertiesStaminaLevelMin, m_fPropertiesStaminaLevelMax);
        else
            m_fPropertiesStaminaLevel = 0.0;

        if (m_bPropertiesWaterLevelEnable)
            m_fPropertiesWaterLevel = GenerateRoundedValue(m_fPropertiesWaterLevelMin, m_fPropertiesWaterLevelMax);
        else
            m_fPropertiesWaterLevel = 0.0;

        if (m_bPropertiesEatLevelEnable)
            m_fPropertiesEatLevel = GenerateRoundedValue(m_fPropertiesEatLevelMin, m_fPropertiesEatLevelMax);
        else
            m_fPropertiesEatLevel = 0.0;

        ResourceName prefab = GetOwner().GetPrefabData().GetPrefabName();
        ARMST_ArtefactLogger.LogGeneratedArtefact(prefab);
    }
}