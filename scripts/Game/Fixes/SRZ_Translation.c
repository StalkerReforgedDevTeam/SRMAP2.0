modded class VLST_RadioVolumeScrollAction : SCR_AdjustSignalAction
{

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		float value = m_fTargetValue;

		int percentage = Math.Round(value * 100);
		outName = "Volume: " + percentage.ToString() + "%";
		return true;
	}

}
