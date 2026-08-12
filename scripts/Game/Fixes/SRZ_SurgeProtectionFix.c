modded class Helpers {

	override static bool IsPlayerIndoors(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;

		vector playerPos = playerEntity.GetOrigin();
		ChimeraCharacter character = ChimeraCharacter.Cast(playerEntity);
		if (character)
			playerPos = character.EyePosition();

		BaseWorld world = playerEntity.GetWorld();
		if (!world)
			return false;

		// Must have roof AND sufficient walls
		if (!CheckRoof(world, playerPos))
			return false;
		if (!CheckWalls(world, playerPos))
			return false;

		return true;
	}

	/// <summary>Internal: Check for roof above position</summary>
	override static bool CheckRoof(BaseWorld world, vector startPos)
	{
		const float ROOF_CHECK_DISTANCE = 8.0;
		
		TraceParam param = new TraceParam();
		param.Start = startPos;
		param.End = startPos + vector.Up * ROOF_CHECK_DISTANCE;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = null;
		param.LayerMask = EPhysicsLayerPresets.Projectile;

		float traceDistance = world.TraceMove(param, null);
		return traceDistance < 1.0;
	}

	/// <summary>Internal: Check for walls around position</summary>
	override static bool CheckWalls(BaseWorld world, vector startPos)
	{
		const float WALL_CHECK_DISTANCE = 8.0;
		const int MIN_WALL_HITS = 6;
		
		array<vector> directions = {
			vector.Forward,
			vector.Forward * -1,
			vector.Right,
			vector.Right * -1,
			vector.Right * vector.Forward,
			vector.Right * -vector.Forward,
			-vector.Right * vector.Forward,
			-vector.Right * -vector.Forward,
		};

		int hitCount = 0;
		foreach (vector dir : directions)
		{
			TraceParam param = new TraceParam();
			param.Start = startPos;
			param.End = startPos + dir * WALL_CHECK_DISTANCE;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.Exclude = null;
			param.LayerMask = EPhysicsLayerPresets.Projectile;

			float traceDistance = world.TraceMove(param, null);
			if (traceDistance < 1.0)
				hitCount++;
		}
		return hitCount >= MIN_WALL_HITS;
	}


}