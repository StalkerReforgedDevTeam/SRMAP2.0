// Placed chem light burns out after m_fBurnSeconds. In the last part of the burn it flickers, then the entity is deleted.
// Server only. The placed time is saved through EPF so a restart does not reset it.
[ComponentEditorProps(category: "SRZ", description: "Chem light burn timer")]
class SRZ_ChemLightBurnComponentClass : ScriptComponentClass
{
}

class SRZ_ChemLightBurnComponent : ScriptComponent
{
	[Attribute("600", UIWidgets.Slider, "Seconds it burns after being placed", "30 3600 10")]
	float m_fBurnSeconds;

	[Attribute("0.15", UIWidgets.Slider, "Last part of the burn where it flickers, as a fraction", "0 0.9 0.05")]
	float m_fFlickerFraction;

	float m_fPlacedAt = -1;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame() || !GetGame().InPlayMode())
			return;
		if (!Replication.IsServer())
			return;
		GetGame().GetCallqueue().CallLater(Tick, 4000, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(Tick);
			GetGame().GetCallqueue().Remove(ShowAgain);
		}
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	protected void Tick()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		if (m_fPlacedAt < 0)
			m_fPlacedAt = Now();
		float left = m_fBurnSeconds - (Now() - m_fPlacedAt);
		if (left <= 0)
		{
			GetGame().GetCallqueue().Remove(Tick);
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
			return;
		}
		if (left > m_fBurnSeconds * m_fFlickerFraction)
			return;
		Rpc(RpcDo_Blink, true);
		RpcDo_Blink(true);
		GetGame().GetCallqueue().CallLater(ShowAgain, 250 + Math.RandomInt(0, 400), false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowAgain()
	{
		Rpc(RpcDo_Blink, false);
		RpcDo_Blink(false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Blink(bool hide)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		if (hide)
			owner.ClearFlags(EntityFlags.VISIBLE, true);
		else
			owner.SetFlags(EntityFlags.VISIBLE, true);
	}
}

// Persist the placed time so a restart does not reset the burn.
[EPF_ComponentSaveDataType(SRZ_ChemLightBurnComponent), BaseContainerProps()]
class SRZ_ChemLightBurnSaveDataClass : EPF_ComponentSaveDataClass
{
}

[EDF_DbName()]
class SRZ_ChemLightBurnSaveData : EPF_ComponentSaveData
{
	float m_fSecondsLeft;

	override EPF_EReadResult ReadFrom(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		SRZ_ChemLightBurnComponent c = SRZ_ChemLightBurnComponent.Cast(component);
		if (!c)
			return EPF_EReadResult.ERROR;
		float now = GetGame().GetWorld().GetWorldTime() * 0.001;
		if (c.m_fPlacedAt < 0)
			m_fSecondsLeft = c.m_fBurnSeconds;
		else
			m_fSecondsLeft = c.m_fBurnSeconds - (now - c.m_fPlacedAt);
		return EPF_EReadResult.OK;
	}

	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		SRZ_ChemLightBurnComponent c = SRZ_ChemLightBurnComponent.Cast(component);
		if (!c)
			return EPF_EApplyResult.ERROR;
		float now = GetGame().GetWorld().GetWorldTime() * 0.001;
		c.m_fPlacedAt = now - (c.m_fBurnSeconds - m_fSecondsLeft);
		return EPF_EApplyResult.OK;
	}
}
