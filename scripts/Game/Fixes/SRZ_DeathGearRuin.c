class SRZ_DeathGearRuin
{
	static const float RUIN_CHANCE = 70;

	protected static ref array<ref SRZ_DeathGearRuin> s_aWatchers = {};
	protected static ref array<typename> s_aAreas = {LoadoutHeadCoverArea, ARMST_FaceArea, ZEL_FaceArea, ZEL_HeadArea, LoadoutJacketArea, LoadoutPantsArea, LoadoutArmoredVestSlotArea, LoadoutVestArea, LoadoutBootsArea, LoadoutHandwearSlotArea};

	protected IEntity m_Character;

	static void Watch(IEntity character)
	{
		if (!character || !Replication.IsServer())
			return;

		for (int i = s_aWatchers.Count() - 1; i >= 0; i--)
		{
			if (!s_aWatchers[i].m_Character || s_aWatchers[i].m_Character.IsDeleted())
			{
				s_aWatchers.Remove(i);
				continue;
			}

			if (s_aWatchers[i].m_Character == character)
				return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!controller)
			return;

		SRZ_DeathGearRuin watcher = new SRZ_DeathGearRuin();
		watcher.m_Character = character;
		controller.m_OnLifeStateChanged.Insert(watcher.OnLifeStateChanged);
		s_aWatchers.Insert(watcher);
	}

	void OnLifeStateChanged(ECharacterLifeState previousState, ECharacterLifeState newState)
	{
		if (newState != ECharacterLifeState.DEAD || previousState == ECharacterLifeState.DEAD)
			return;

		if (!m_Character || !Replication.IsServer())
			return;

		EquipedLoadoutStorageComponent storage = EquipedLoadoutStorageComponent.Cast(m_Character.FindComponent(EquipedLoadoutStorageComponent));
		if (!storage)
			return;

		foreach (typename area : s_aAreas)
		{
			IEntity item = storage.GetClothFromArea(area);
			if (!item || Math.RandomFloat(0, 100) >= RUIN_CHANCE)
				continue;

			ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			if (stats)
				stats.SetConditionLevel(0);
		}
	}
}

modded class SCR_PlayerController
{
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
		SRZ_DeathGearRuin.Watch(to);
	}
}
