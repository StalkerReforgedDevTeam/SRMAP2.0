modded class SCR_CharacterControllerComponent
{
	override void WeaponHandler(int playerId)
	{
	    if (!Replication.IsServer()) return;
	
	    SCR_PlayerController scrPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if(!scrPlayerController)
			return;
	    IEntity user = scrPlayerController.GetMainEntity();
		
	    BaseGameMode gameMode = BaseGameMode.Cast(GetGame().GetGameMode());
	    if (gameMode)
	        {
		        // Получаем компонент глобальных настроек ARMST_EDITOR_GLOBAL_SETTINGS
		        ARMST_EDITOR_GLOBAL_SETTINGS Settings = ARMST_EDITOR_GLOBAL_SETTINGS.Cast(gameMode.FindComponent(ARMST_EDITOR_GLOBAL_SETTINGS));
		        if (Settings)
		        {
					conditionDamageModifier = Settings.m_WeaponDamageConditionLevel;
				}
			}
			else 
			{
				return;	
			}

		
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager) {  return; }

		BaseWeaponComponent weaponManager2 = weaponManager.GetCurrentWeapon();
		if (!weaponManager2) {  return; }
		
		IEntity wpn =  weaponManager2.GetOwner();
			if(wpn)
			{
			            ARMST_ITEMS_STATS_COMPONENTS m_ArmstStat_wpn = ARMST_ITEMS_STATS_COMPONENTS.Cast(wpn.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			            if (m_ArmstStat_wpn)
							{
			        		 		m_ArmstStat_wpn.DecreaseCondition(conditionDamageModifier);
							}
						ARMST_SHOTGUN_COMPONENTS m_shotgun = ARMST_SHOTGUN_COMPONENTS.Cast(wpn.FindComponent(ARMST_SHOTGUN_COMPONENTS));
						if(m_shotgun)
							{
				                BaseMagazineComponent currentMagazine = weaponManager2.GetCurrentMagazine();
				                 if (currentMagazine)
				                     {
				                                IEntity magazineEntity = currentMagazine.GetOwner();
				                                ResourceName resourceName = magazineEntity.GetPrefabData().GetPrefabName();
												int Capacity = m_shotgun.m_MaxMagazineAmmo;
										        int maxCapacity = Capacity;
										        if (maxCapacity <= 0)
										            return;
													
										        // Текущее количество патронов в магазине
										        int currentAmmo = currentMagazine.GetAmmoCount();
										        if (currentAmmo < maxCapacity)
										            return; // Всё в порядке
									
										        // === ОБРЕЗАЕМ МАГАЗИН ===
										        int excessAmmo = currentAmmo - maxCapacity;
										
										        // Устанавливаем максимум в текущий магазин
										        currentMagazine.SetAmmoCount(maxCapacity);
										        		Resource resource = Resource.Load(resourceName);
										        if (!resource)
										            return;
										
							                    vector transform[4];
							                    SCR_TerrainHelper.GetTerrainBasis(user.GetOrigin(), transform, GetGame().GetWorld(), false, new TraceParam());
							
							                    m_aOriginalTransform = transform;
							
							                    EntitySpawnParams params = new EntitySpawnParams();
							                    params.Transform = m_aOriginalTransform;
							                    params.TransformMode = ETransformMode.WORLD;
					
												SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
												if (!inventory)
										            return;

										        IEntity newMagazine = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);

										        if (!newMagazine)
										            return;
					
										        MagazineComponent newMag = MagazineComponent.Cast(newMagazine.FindComponent(MagazineComponent));
										        if (newMag)
										        {
										            newMag.SetAmmoCount(excessAmmo);
										        }

					
												if(excessAmmo <= 0)
												{
													SCR_EntityHelper.DeleteEntityAndChildren(newMagazine);
													inventory.TryDeleteItem(newMagazine);
												}
										        
												float ammo_last =  newMag.GetAmmoCount();
												if(ammo_last < 1)
												{
														inventory.TryDeleteItem(newMagazine);
														return;
												}
					
				                                private bool IsAddedToInventory = inventory.TryInsertItem(newMagazine);
										        // Кладём в инвентарь
										        if (!IsAddedToInventory)
										        {
										            // Если не влез — падает рядом с игроком
										            vector pos = user.GetOrigin();
										            SCR_EntityHelper.SnapToGround(newMagazine);
										        }
													
									}
							}
			}
		
	}
	
}
