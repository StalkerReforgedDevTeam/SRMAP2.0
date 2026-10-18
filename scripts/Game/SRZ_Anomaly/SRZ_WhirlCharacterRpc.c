modded class SCR_CharacterControllerComponent
{
	void SRZ_AskWhirlPull(vector centre, float seconds, float speed)
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;

		RplComponent rpl = character.GetRplComponent();
		if (!rpl || rpl.IsOwner())
		{
			SRZ_WhirlPull.Begin(character, centre, seconds, speed);
			return;
		}

		Rpc(RpcDo_SRZ_WhirlPull, centre, seconds, speed);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZ_WhirlPull(vector centre, float seconds, float speed)
	{
		SRZ_WhirlPull.Begin(GetCharacter(), centre, seconds, speed);
	}

	void SRZ_BroadcastWhirlSpin(vector centre, float height, float radius, float speed, float gain, float seconds, float dir)
	{
		SRZ_WhirlPull.End(GetCharacter());
		SRZ_WhirlBody.Spin(GetCharacter(), centre, height, radius, speed, gain, seconds, dir);
		Rpc(RpcDo_SRZ_WhirlSpin, centre, height, radius, speed, gain, seconds, dir);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SRZ_WhirlSpin(vector centre, float height, float radius, float speed, float gain, float seconds, float dir)
	{
		SRZ_WhirlPull.End(GetCharacter());
		SRZ_WhirlBody.Spin(GetCharacter(), centre, height, radius, speed, gain, seconds, dir);
	}

	void SRZ_BroadcastWhirlFling(vector velocity)
	{
		SRZ_WhirlBody.Fling(GetCharacter(), velocity);
		Rpc(RpcDo_SRZ_WhirlFling, velocity);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SRZ_WhirlFling(vector velocity)
	{
		SRZ_WhirlBody.Fling(GetCharacter(), velocity);
	}
}
