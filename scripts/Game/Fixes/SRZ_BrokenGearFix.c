// Condition-0 clothes, armor and guns can't be equipped, sold or repaired. Gas masks are exempt.
// Gear already worn when it hits 0 stays on until the player removes it.

class SRZ_BrokenGear
{
	protected static const ResourceName GASMASK_DB = "{3C6FE0CC05D636CB}Configs/DATABASE_ITEMS/CLOTHES/GASMASKS/GASMASKS.conf";
	protected static ref set<ResourceName> s_GasMasks;

	static bool IsBlocked(IEntity item)
	{
		if (!item)
			return false;

		// Only guns, clothes and armor
		if (!item.FindComponent(WeaponComponent) && !item.FindComponent(BaseLoadoutClothComponent))
			return false;

		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats || stats.GetConditionLevel() > 0)
			return false;

		return !IsGasMask(item);
	}

	static bool IsGasMask(IEntity item)
	{
		if (!s_GasMasks)
			LoadGasMasks();

		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return false;

		return s_GasMasks.Contains(prefabData.GetPrefabName());
	}

	protected static void LoadGasMasks()
	{
		s_GasMasks = new set<ResourceName>();

		Resource resource = BaseContainerTools.LoadContainer(GASMASK_DB);
		if (!resource || !resource.IsValid())
			return;

		ARMST_DATABASE_ITEMS list = ARMST_DATABASE_ITEMS.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (!list || !list.m_ItemsList)
			return;

		foreach (ARMST_DATABASE_ITEM entry : list.m_ItemsList)
		{
			if (!entry.m_Prefab.IsEmpty())
				s_GasMasks.Insert(entry.m_Prefab);

			if (!entry.m_ChildrensPrefabs)
				continue;

			foreach (ResourceName child : entry.m_ChildrensPrefabs)
			{
				if (!child.IsEmpty())
					s_GasMasks.Insert(child);
			}
		}
	}
}

// Grace window after the character spawns so saved worn gear can be put back on at login
class SRZ_BrokenGearGrace
{
	static const float GRACE_MS = 15000;

	static bool Active(inout float readyTime)
	{
		float now = GetGame().GetWorld().GetWorldTime();
		if (readyTime < 0)
			readyTime = now + GRACE_MS;

		return now < readyTime;
	}
}

// Clothes and armor slots
modded class SCR_CharacterInventoryStorageComponent
{
	protected float m_fSRZ_GraceUntil = -1;

	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (!SRZ_BrokenGearGrace.Active(m_fSRZ_GraceUntil) && SRZ_BrokenGear.IsBlocked(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}

	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		if (!SRZ_BrokenGearGrace.Active(m_fSRZ_GraceUntil) && SRZ_BrokenGear.IsBlocked(nextItem))
			return false;

		return super.CanReplaceItem(nextItem, slotID);
	}

	override bool CanEquipItem_Inventory(notnull IEntity item)
	{
		if (SRZ_BrokenGear.IsBlocked(item))
			return false;

		return super.CanEquipItem_Inventory(item);
	}
}

// Weapon slots / hands
modded class EquipedWeaponStorageComponent
{
	protected float m_fSRZ_GraceUntil = -1;

	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (!SRZ_BrokenGearGrace.Active(m_fSRZ_GraceUntil) && SRZ_BrokenGear.IsBlocked(item))
			return false;

		return super.CanStoreItem(item, slotID);
	}

	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		if (!SRZ_BrokenGearGrace.Active(m_fSRZ_GraceUntil) && SRZ_BrokenGear.IsBlocked(nextItem))
			return false;

		return super.CanReplaceItem(nextItem, slotID);
	}
}

// Hides broken gear from trader sell lists and marks it "can't trade" in the tooltip
modded class ARMST_ITEMS_STATS_COMPONENTS
{
	override bool GetNotTrade()
	{
		if (super.GetNotTrade())
			return true;

		return SRZ_BrokenGear.IsBlocked(GetOwner());
	}
}

// Save only the real "not tradeable" flag, not the broken-gear one
modded class EPF_ARMST_ITEMS_STATS_COMPONENTS_SaveData
{
	override EPF_EReadResult ReadFrom(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		EPF_EReadResult result = super.ReadFrom(owner, component, attributes);

		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(component);
		if (stats)
			m_NotTrade = stats.m_NotTrade;

		return result;
	}
}

modded class SCR_PlayerController
{
	// Server side sell: never take broken gear or anything worn/equipped (watches and binos showed up twice and
	// got deleted twice, which froze the server), and don't delete anything unless the full amount is available
	override void ArmstTraderSell(IEntity m_User, ResourceName m_PrefabTrader, int sellCount, float totalRevenue)
	{
		if (!Replication.IsServer() || !m_User)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(m_User.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventory)
			return;

		array<IEntity> items = {};
		if (!inventory.GetItems(items))
			return;

		array<IEntity> sellable = {};
		foreach (IEntity item : items)
		{
			if (sellable.Count() >= sellCount)
				break;

			if (!item || sellable.Contains(item))
				continue;

			if (!item.GetPrefabData() || item.GetPrefabData().GetPrefabName() != m_PrefabTrader)
				continue;

			if (SRZ_BrokenGear.IsBlocked(item) || SRZ_IsEquippedOn(m_User, item))
				continue;

			sellable.Insert(item);
		}

		if (sellable.Count() < sellCount)
		{
			Print("[SRZ_BrokenGear] Sell refused, not enough sellable " + m_PrefabTrader, LogLevel.WARNING);
			return;
		}

		foreach (IEntity item : sellable)
		{
			if (item)
				SCR_EntityHelper.DeleteEntityAndChildren(item);
		}

		ARMST_PLAYER_STATS_COMPONENT currencyComp = ARMST_PLAYER_STATS_COMPONENT.Cast(m_User.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (currencyComp)
			currencyComp.ModifyValue(totalRevenue, true);

		string playerGuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(m_User);
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_User);
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		ARMST_SellerLogger.LogSeller(playerGuid, playerName, m_PrefabTrader, sellCount, totalRevenue);
	}

	// Worn or equipped directly on the character (clothes, gadget slots, weapon slots), not in a bag or pocket
	protected bool SRZ_IsEquippedOn(IEntity user, IEntity item)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;

		InventoryStorageSlot slot = itemComp.GetParentSlot();
		if (!slot || !slot.GetStorage())
			return false;

		return slot.GetStorage().GetOwner() == user;
	}

	// Server side repair: pick a copy of the item that isn't broken, otherwise refuse without using the kit (kit handling matches ARMST)
	override void ArmstRepairItem(IEntity player, ResourceName itemPrefab, int expectedCondition, ResourceName RepairPrefab, int RepCondition)
	{
		if (!Replication.IsServer() || !player)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventory)
			return;

		array<IEntity> items = {};
		if (!inventory.GetItems(items))
			return;

		IEntity target;
		float bestDiff = float.MAX;
		foreach (IEntity item : items)
		{
			if (!item || !item.GetPrefabData() || item.GetPrefabData().GetPrefabName() != itemPrefab)
				continue;

			ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			if (!stats || SRZ_BrokenGear.IsBlocked(item))
				continue;

			float diff = Math.AbsFloat(stats.GetConditionLevel() - expectedCondition);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				target = item;
			}
		}

		if (!target)
		{
			Print("[SRZ_BrokenGear] Repair refused, no repairable " + itemPrefab, LogLevel.WARNING);
			return;
		}

		B_PrefabNamePredicate kitPred = new B_PrefabNamePredicate();
		kitPred.prefabName.Insert(RepairPrefab);

		array<IEntity> kits = {};
		if (inventory.FindItems(kits, kitPred) && !kits.IsEmpty())
			SCR_EntityHelper.DeleteEntityAndChildren(kits[0]);

		ARMST_ITEMS_STATS_COMPONENTS targetStats = ARMST_ITEMS_STATS_COMPONENTS.Cast(target.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		targetStats.SetConditionLevel(targetStats.GetConditionLevel() + RepCondition);
	}
}
