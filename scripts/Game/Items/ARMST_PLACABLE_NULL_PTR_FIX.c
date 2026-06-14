modded class SCR_ItemPlacementComponent : ScriptComponent
{
    override void AskPlaceItem(RplId placeableId, RplId targetId, int nodeId, bool isBeingAttachedToEntity)
    {
        Rpc(RPC_AskPlaceItem, placeableId, previewPos, previewAngles, m_Prefab);

        ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
        if (!controlledEntity)
            return;

        SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.GetCharacterController());
        if (!characterController)
            return;

        characterController.SetDisableWeaponControls(false);
        characterController.SetDisableMovementControls(false);

        DisablePreview();
    }

    override protected void OnPlacingEnded(IEntity item, bool successful, ItemUseParameters animParams)
    {
        super.OnPlacingEnded(item, successful, animParams);

        ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
        if (!controlledEntity)
            return;

        SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.GetCharacterController());
        if (!characterController)
            return;

        characterController.SetDisableWeaponControls(false);
        characterController.SetDisableMovementControls(false);
    }

    // DisablePreview is called in ALL cancellation paths — item put back in inventory,
    // player gets in vehicle, placed item becomes null, etc.
    // This is the correct place to restore controls.
    protected override void DisablePreview()
    {
        // Always remove the CharacterFire listener — prevents it consuming shots
        // when no item is being placed
        GetGame().GetInputManager().RemoveActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);

        super.DisablePreview();
		
		auto playerController = GetGame().GetPlayerController();
		if (playerController == null)
			return;
		
		auto controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity == null)
			return;

        ChimeraCharacter controlledCharacter = ChimeraCharacter.Cast(controlledEntity);
        if (controlledCharacter == null)
            return;

        SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledCharacter.GetCharacterController());
        if (!characterController)
            return;

        characterController.SetDisableWeaponControls(false);
        characterController.SetDisableMovementControls(false);
    }

    protected override void EnablePreview(IEntity weapon)
    {
        SCR_PlaceableItemComponent placeableItemComponent = SCR_PlaceableItemComponent.Cast(weapon.FindComponent(SCR_PlaceableItemComponent));
        if (!placeableItemComponent)
        {
            DisablePreview();
            return;
        }

        m_PlaceableItem = placeableItemComponent;

        EntityPrefabData prefabData = weapon.GetPrefabData();
        if (!prefabData)
            return;

        GetGame().GetInputManager().AddActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);

        SetEventMask(GetOwner(), EntityEvent.FRAME);
        if (!m_PreviewEntity)
            m_PreviewEntity = GetGame().SpawnEntity(GenericEntity, GetOwner().GetWorld());

        SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(weapon.FindComponent(SCR_PlaceableInventoryItemComponent));
        if (itemComponent)
        {
            m_Prefab = itemComponent.m_PrefabToSpawn;
        }
        else
        {
            SCR_MineInventoryItemComponent mineComponent = SCR_MineInventoryItemComponent.Cast(weapon.FindComponent(SCR_MineInventoryItemComponent));
            if (!mineComponent)
            {
                DisablePreview();
                return;
            }
            m_Prefab = mineComponent.m_PrefabToSpawn;
        }

        Resource resource2 = Resource.Load(m_Prefab);
        if (resource2)
        {
            ResourceName modelPath;
            string remapPath;

            bool foundModelPath = SCR_Global.GetModelAndRemapFromResource(m_Prefab, modelPath, remapPath);
            if (!foundModelPath)
                return;

            Resource resource = Resource.Load(modelPath);
            if (!resource.IsValid())
                return;

            MeshObject model = resource.GetResource().ToMeshObject();
            m_PreviewEntity.SetObject(model, "");
            return;
        }

        ResourceName modelPath;
        string remapPath;

        bool foundModelPath = SCR_Global.GetModelAndRemapFromResource(m_Prefab, modelPath, remapPath);
        if (!foundModelPath)
            return;

        Resource resource = Resource.Load(modelPath);
        if (!resource.IsValid())
            return;

        MeshObject model = resource.GetResource().ToMeshObject();
        m_PreviewEntity.SetObject(model, "");
    }

    protected override void EOnFrame(IEntity owner, float timeSlice)
    {
        if (!m_CompartmnetAccessComponent)
        {
            DisablePreview();
            return;
        }

        if (m_CompartmnetAccessComponent.IsGettingIn())
        {
            DisablePreview();
            return;
        }

        if (!m_PlacedItem)
        {
            DisablePreview();
            return;
        }

        if (!m_PlaceableItem)
        {
            DisablePreview();
            return;
        }

        CameraManager cameraManager = GetGame().GetCameraManager();
        if (!cameraManager)
            return;

        CameraBase currentCamera = cameraManager.CurrentCamera();
        if (!currentCamera)
            return;

        vector cameraMat[4];
        currentCamera.GetTransform(cameraMat);
        float maxPlacementDistance = m_PlaceableItem.GetMaxPlacementDistance();
        SCR_EPlacementType placementType = m_PlaceableItem.GetPlacementType();

        m_eCantPlaceReason = 0;
        switch (placementType)
        {
            case SCR_EPlacementType.XZ_FIXED:
                UseXZFixedPlacement(owner, maxPlacementDistance, cameraMat);
                break;

            case SCR_EPlacementType.XYZ:
                UseXYZPlacement(owner, maxPlacementDistance, cameraMat);
                break;
        }

        if (m_eCantPlaceReason == 0)
            SCR_Global.SetMaterial(m_PreviewEntity, m_sCanBuildMaterial);
        else if (m_eCantPlaceReason == ENotification.PLACEABLE_ITEM_CANT_PLACE_DISTANCE)
            SCR_Global.SetMaterial(m_PreviewEntity, m_sTransparentMaterial);
        else
            SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);

        previewAngles = m_PreviewEntity.GetAngles();
        previewPos = m_PreviewEntity.GetOrigin();
    }
}