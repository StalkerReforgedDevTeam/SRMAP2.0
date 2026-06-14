modded class ARMST_ARTEFACT_COMPONENTS : SCR_GadgetComponent
{
    //------------------------------------------------------------------------------------------------
    override void EnableLight()
    {
        m_bLastLightState = true;
        if (m_EmissiveMaterial)
        {
            m_EmissiveMaterial.SetUserAlphaTestParam(255);
            m_EmissiveMaterial.SetEmissiveMultiplier(m_Emmisive);
        }
    }

    //------------------------------------------------------------------------------------------------
    override void DisableLight()
    {
        m_bLastLightState = false;
        float m_Time_Hidden2 = m_Time_Hidden * 1000;
        if (m_EmissiveMaterial)
        {
            m_EmissiveMaterial.SetUserAlphaTestParam(0);
            m_EmissiveMaterial.SetEmissiveMultiplier(0);
        }

        SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetOwner().GetWorld());
        if (soundManager)
            soundManager.CreateAndPlayAudioSource(arts, "SOUND_ART");

        GetGame().GetCallqueue().CallLater(EnableLight, m_Time_Hidden2, false);
    }

    //------------------------------------------------------------------------------------------------
    override void EOnInit(IEntity owner)
    {
        if (!GetGame().InPlayMode())
            return;

        arts = owner;
        m_EmissiveMaterial = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));

        if (m_fStartHidden)
            DisableLight();
        else
            EnableLight();
    }
}