class SRZ_FXRange
{
	static const float DEFAULT_RANGE_M = 150;

	static bool IsHeadless()
	{
		if (System.IsConsoleApp())
			return true;

		return RplSession.Mode() == RplMode.Dedicated;
	}

	static bool CameraPosition(out vector pos)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		vector mat[4];
		world.GetCurrentCamera(mat);
		pos = mat[3];
		return true;
	}

	static bool InRange(vector pos, float rangeM = DEFAULT_RANGE_M)
	{
		if (IsHeadless())
			return false;

		vector camera;
		if (!CameraPosition(camera))
			return false;

		return vector.DistanceSq(camera, pos) <= rangeM * rangeM;
	}

	static ParticleEffectEntity Particle(ResourceName effect, vector pos, float rangeM = DEFAULT_RANGE_M)
	{
		if (effect.IsEmpty() || !InRange(pos, rangeM))
			return null;

		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;
		params.DeleteWhenStopped = true;
		return ParticleEffectEntity.SpawnParticleEffect(effect, params);
	}

	static AudioHandle Sound(ResourceName project, string eventName, vector pos, float rangeM = DEFAULT_RANGE_M)
	{
		if (project.IsEmpty() || eventName.IsEmpty() || !InRange(pos, rangeM))
			return AudioHandle.Invalid;

		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = pos;
		return AudioSystem.PlayEvent(project, eventName, mat, new array<string>(), new array<float>());
	}

	static void StopSound(AudioHandle handle)
	{
		if (handle != AudioHandle.Invalid)
			AudioSystem.TerminateSound(handle);
	}
}
