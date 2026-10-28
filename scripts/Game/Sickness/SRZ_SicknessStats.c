// Per player sickness values, the DayZ PlayerStats equivalent.
// Everything is a number that moves. Saved to $profile:SRZ_Sickness/<uid>.txt, cleared on death.
class SRZ_SicknessStats
{
	IEntity m_Char;
	string m_sUid;

	// Environment
	float m_fTemperature;		// -100 to 100, 0 is comfortable
	float m_fWetness;			// 0 to 100
	float m_fHeatBuff;			// seconds left where temperature can't drop
	float m_fInsulation;		// last computed, 0 to 100, for the HUD and logs
	float m_fTarget;			// last environment target, for logs

	// Agents
	float m_fColdChain;			// 0 to 300: cold at 100, influenza at 200, pneumonia at 300
	float m_fSalmonella;		// 0 to 100
	float m_fIncubation;		// seconds until a swallowed dose of salmonella starts
	float m_fResistance;		// seconds where the cold chain can't rise (after a stage drop)
	float m_fImmunityBoost;		// seconds of forced immunity 100 (multivitamin)
	float m_fCodeine;			// seconds of codeine

	// Medicine pushes, applied per tick while their time runs
	float m_fTetraLeft;			// seconds of tetracycline left
	float m_fCharcoalLeft;		// seconds of charcoal left

	// Bookkeeping
	float m_fNextSave;
	float m_fNextSneeze;
	float m_fNextCough;
	float m_fNextVomit;
	float m_fNextMessage;
	int m_iLastColdStage = -1;
	int m_iLastTempBand = 0;
	int m_iLastSalmStage = -1;
	bool m_bDirty;

	//------------------------------------------------------------------------------------------------
	bool IsClean()
	{
		return Math.AbsFloat(m_fTemperature) < 1 && m_fWetness < 1 && m_fHeatBuff <= 0 && m_fColdChain < 1 && m_fSalmonella < 1
			&& m_fIncubation <= 0 && m_fResistance <= 0 && m_fImmunityBoost <= 0 && m_fCodeine <= 0 && m_fTetraLeft <= 0 && m_fCharcoalLeft <= 0;
	}

	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_fTemperature = 0;
		m_fWetness = 0;
		m_fHeatBuff = 0;
		m_fColdChain = 0;
		m_fSalmonella = 0;
		m_fIncubation = 0;
		m_fResistance = 0;
		m_fImmunityBoost = 0;
		m_fCodeine = 0;
		m_fTetraLeft = 0;
		m_fCharcoalLeft = 0;
		m_iLastColdStage = -1;
		m_iLastTempBand = 0;
		m_iLastSalmStage = -1;
	}

	//------------------------------------------------------------------------------------------------
	string Serialize()
	{
		string line = "1";
		line += "|" + m_fTemperature.ToString();
		line += "|" + m_fWetness.ToString();
		line += "|" + m_fHeatBuff.ToString();
		line += "|" + m_fColdChain.ToString();
		line += "|" + m_fSalmonella.ToString();
		line += "|" + m_fIncubation.ToString();
		line += "|" + m_fResistance.ToString();
		line += "|" + m_fImmunityBoost.ToString();
		line += "|" + m_fCodeine.ToString();
		line += "|" + m_fTetraLeft.ToString();
		line += "|" + m_fCharcoalLeft.ToString();
		return line;
	}

	//------------------------------------------------------------------------------------------------
	bool Deserialize(string line)
	{
		array<string> parts = {};
		line.Split("|", parts, false);
		if (parts.Count() < 12 || parts[0] != "1")
			return false;

		m_fTemperature = parts[1].ToFloat();
		m_fWetness = parts[2].ToFloat();
		m_fHeatBuff = parts[3].ToFloat();
		m_fColdChain = parts[4].ToFloat();
		m_fSalmonella = parts[5].ToFloat();
		m_fIncubation = parts[6].ToFloat();
		m_fResistance = parts[7].ToFloat();
		m_fImmunityBoost = parts[8].ToFloat();
		m_fCodeine = parts[9].ToFloat();
		m_fTetraLeft = parts[10].ToFloat();
		m_fCharcoalLeft = parts[11].ToFloat();
		return true;
	}
}
