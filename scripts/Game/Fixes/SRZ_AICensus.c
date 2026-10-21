// Every 10 minutes the server logs how many AI are alive and what they are, so the biggest sources can be tuned.
class SRZ_AICensus
{
	protected static const float INTERVAL_MS = 600000;
	protected static const int TOP_COUNT = 20;

	//------------------------------------------------------------------------------------------------
	static void Start()
	{
		if (!Replication.IsServer())
			return;

		GetGame().GetCallqueue().Remove(Run);
		GetGame().GetCallqueue().CallLater(Run, INTERVAL_MS, true);
	}

	//------------------------------------------------------------------------------------------------
	static void Run()
	{
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return;

		array<AIAgent> agents = {};
		aiWorld.GetAIAgents(agents);

		map<string, int> counts = new map<string, int>();
		int groups, characters, dead;

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			if (SCR_AIGroup.Cast(agent))
			{
				groups++;
				continue;
			}

			IEntity ent = agent.GetControlledEntity();
			if (!ent)
				continue;

			characters++;

			ChimeraCharacter character = ChimeraCharacter.Cast(ent);
			if (character && character.GetCharacterController() && character.GetCharacterController().GetLifeState() == ECharacterLifeState.DEAD)
				dead++;

			string name = "unknown";
			if (ent.GetPrefabData())
			{
				name = ent.GetPrefabData().GetPrefabName();
				int brace = name.IndexOf("}");
				if (brace >= 0)
					name = name.Substring(brace + 1, name.Length() - brace - 1);
			}

			counts.Set(name, counts.Get(name) + 1);
		}

		int players = GetGame().GetPlayerManager().GetPlayerCount();
		Print(string.Format("[SRZ AI] Players %1 | AI characters %2 (dead %3) | groups %4 | dead mutant props waiting %5", players, characters, dead, groups, ARMST_USER_LOOT_BODY.s_iSRZ_CorpsesAlive), LogLevel.NORMAL);

		for (int i = 0; i < TOP_COUNT && !counts.IsEmpty(); i++)
		{
			string bestName;
			int bestCount = -1;
			for (int j = 0; j < counts.Count(); j++)
			{
				if (counts.GetElement(j) > bestCount)
				{
					bestCount = counts.GetElement(j);
					bestName = counts.GetKey(j);
				}
			}

			Print(string.Format("[SRZ AI]   %1 x %2", bestCount, bestName), LogLevel.NORMAL);
			counts.Remove(bestName);
		}
	}
}

modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();
		SRZ_AICensus.Start();
	}
}
