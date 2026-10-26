// Map placed loudspeaker. Plays a sound event over and over for players in range, like the old Monolith call
[ComponentEditorProps(category: "SRZ", description: "Repeating broadcast for nearby players")]
class SRZ_BroadcastComponentClass : ScriptComponentClass
{
}

class SRZ_BroadcastComponent : ScriptComponent
{
	[Attribute("{D07E75B522CB17A0}Sounds/SRZ/Monolith/SRZ_MonolithCall.acp", UIWidgets.ResourceNamePicker, "Sound project", "acp")]
	protected ResourceName m_sProject;

	[Attribute("SOUND_SRZ_MONOLITH_CALL", desc: "Sound event inside the project")]
	protected string m_sEvent;

	[Attribute("45", desc: "Seconds from the start of one play to the next. Set it to the clip length for nonstop")]
	protected float m_fRepeat;

	[Attribute("400", desc: "Players further than this never hear it")]
	protected float m_fRange;

	protected float m_fNext;
	protected AudioHandle m_Handle = AudioHandle.Invalid;

	override void OnPostInit(IEntity owner)
	{
		if (System.IsConsoleApp() || !GetGame() || !GetGame().InPlayMode())
			return;

		GetGame().GetCallqueue().CallLater(Tick, 1000, true);
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Tick);

		Stop();
	}

	protected void Stop()
	{
		if (m_Handle != AudioHandle.Invalid)
			AudioSystem.TerminateSound(m_Handle);

		m_Handle = AudioHandle.Invalid;
	}

	protected void Tick()
	{
		IEntity owner = GetOwner();
		if (!owner || !owner.GetWorld())
			return;

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		if (vector.DistanceSq(player.GetOrigin(), owner.GetOrigin()) > m_fRange * m_fRange)
		{
			Stop();
			m_fNext = 0;
			return;
		}

		float now = owner.GetWorld().GetWorldTime() / 1000;
		if (now < m_fNext)
			return;

		vector mat[4];
		owner.GetWorldTransform(mat);
		m_Handle = AudioSystem.PlayEvent(m_sProject, m_sEvent, mat, new array<string>(), new array<float>());
		m_fNext = now + m_fRepeat;
	}
}
