// Swings the crate lid open once it's unlocked. Visual only, runs on each client.
// The closed crate is hidden and an open body plus a lid are spawned locally in its place.
[ComponentEditorProps(category: "SRZ", description: "Crate lid that opens when unlocked")]
class SRZ_CrateLidComponentClass : ScriptComponentClass
{
}

class SRZ_CrateLidComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Crate body without the lid", "et")]
	protected ResourceName m_sBodyPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lid on its own, origin on the hinge", "et")]
	protected ResourceName m_sLidPrefab;

	[Attribute("0 0.364 -0.23", UIWidgets.EditBox, "Hinge position on the crate")]
	protected vector m_vHinge;

	[Attribute("95", UIWidgets.Slider, "How far the lid opens in degrees", "0 180 1")]
	protected float m_fOpenAngle;

	[Attribute("1.6", UIWidgets.Slider, "Seconds to swing open", "0.1 5 0.1")]
	protected float m_fOpenTime;

	protected IEntity m_Body;
	protected IEntity m_Lid;
	protected bool m_bOpen;
	protected float m_fStartTime;
	protected vector m_vPivot;
	protected float m_fDir = 1;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner) || System.IsConsoleApp())
			return;

		GetGame().GetCallqueue().CallLater(CheckAlreadyOpen, 500, false);
	}

	//------------------------------------------------------------------------------------------------
	// Crates that streamed in or were opened before the player joined
	protected void CheckAlreadyOpen()
	{
		SRZ_LockComponent lockComp = SRZ_LockComponent.Get(GetOwner());
		if (lockComp && !lockComp.IsLocked())
			Open(false);
	}

	//------------------------------------------------------------------------------------------------
	void Open(bool animate)
	{
		if (m_bOpen || System.IsConsoleApp())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		Resource bodyRes = Resource.Load(m_sBodyPrefab);
		Resource lidRes = Resource.Load(m_sLidPrefab);
		if (!bodyRes.IsValid() || !lidRes.IsValid())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		owner.GetWorldTransform(params.Transform);

		m_Body = GetGame().SpawnEntityPrefabLocal(bodyRes, owner.GetWorld(), params);
		m_Lid = GetGame().SpawnEntityPrefabLocal(lidRes, owner.GetWorld(), params);
		if (!m_Body || !m_Lid)
		{
			DeleteSpawned();
			return;
		}

		m_bOpen = true;
		owner.ClearFlags(EntityFlags.VISIBLE, false);

		// The importer can flip the model's front axis, so read which way the lid actually extends
		vector mins, maxs;
		m_Lid.GetBounds(mins, maxs);
		m_vPivot = m_vHinge;
		if (mins[2] + maxs[2] < 0)
		{
			m_vPivot[2] = -m_vHinge[2];
			m_fDir = -1;
		}

		if (!animate)
		{
			SetAngle(m_fOpenAngle);
			return;
		}

		SetAngle(0);
		m_fStartTime = owner.GetWorld().GetWorldTime();
		GetGame().GetCallqueue().CallLater(Step, 15, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void Step()
	{
		IEntity owner = GetOwner();
		if (!owner || !m_Lid)
		{
			GetGame().GetCallqueue().Remove(Step);
			return;
		}

		float t = (owner.GetWorld().GetWorldTime() - m_fStartTime) / (m_fOpenTime * 1000);
		if (t >= 1)
		{
			SetAngle(m_fOpenAngle);
			GetGame().GetCallqueue().Remove(Step);
			return;
		}

		// eases in as it lifts and eases out as it settles back
		float ease = 0.5 - 0.5 * Math.Cos(t * Math.PI);
		SetAngle(m_fOpenAngle * ease);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetAngle(float degrees)
	{
		IEntity owner = GetOwner();
		if (!m_Lid || !owner)
			return;

		float s = Math.Sin(degrees * m_fDir * Math.DEG2RAD);
		float c = Math.Cos(degrees * m_fDir * Math.DEG2RAD);

		vector local[4];
		local[0] = Vector(1, 0, 0);
		local[1] = Vector(0, c, -s);
		local[2] = Vector(0, s, c);
		local[3] = m_vPivot;

		vector world[4];
		vector ownerMat[4];
		owner.GetWorldTransform(ownerMat);
		Math3D.MatrixMultiply4(ownerMat, local, world);
		m_Lid.SetWorldTransform(world);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteSpawned()
	{
		if (m_Body)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Body);

		if (m_Lid)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Lid);

		m_Body = null;
		m_Lid = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(Step);
			GetGame().GetCallqueue().Remove(CheckAlreadyOpen);
		}

		DeleteSpawned();
	}
}
