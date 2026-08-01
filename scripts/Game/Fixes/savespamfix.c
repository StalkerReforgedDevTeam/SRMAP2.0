modded class EPF_ArmstPlayerStatsComponentSaveData
{
    //------------------------------------------------------------------------------------------------
    override bool Equals(notnull EPF_ComponentSaveData other)
    {
        EPF_ArmstPlayerStatsComponentSaveData otherData = EPF_ArmstPlayerStatsComponentSaveData.Cast(other);
        if (!otherData)
            return false;
        
        return float.AlmostEqual(m_fToxic, otherData.m_fToxic) &&
               float.AlmostEqual(m_fRadiactive, otherData.m_fRadiactive) &&
               float.AlmostEqual(m_fPsy, otherData.m_fPsy) &&
               float.AlmostEqual(m_fWater, otherData.m_fWater) &&
               float.AlmostEqual(m_fEat, otherData.m_fEat) &&
               m_sPlayerName == otherData.m_sPlayerName &&
               m_sPlayerBiography == otherData.m_sPlayerBiography &&
               m_sPlayerHead == otherData.m_sPlayerHead &&
               m_FactionKey == otherData.m_FactionKey &&
               float.AlmostEqual(m_sPlayerMoney, otherData.m_sPlayerMoney) &&
               vector.DistanceSq(m_ShelterIn, otherData.m_ShelterIn) < 0.0001;
    }
}