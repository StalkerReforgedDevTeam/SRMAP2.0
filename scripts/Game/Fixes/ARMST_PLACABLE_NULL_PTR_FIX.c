modded class SCR_ItemPlacementComponent : ScriptComponent
{
    //------------------------------------------------------------------------------------------------
    protected void RestorePlayerControls()
    {
        ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
        if (!character)
            return;

        SCR_CharacterControllerComponent ctrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
        if (!ctrl)
            return;

        ctrl.SetDisableWeaponControls(false);
        ctrl.SetDisableMovementControls(false);
    }

    //------------------------------------------------------------------------------------------------
    override void AskPlaceItem(RplId placeableId, RplId targetId, int nodeId, bool isBeingAttachedToEntity)
    {
        super.AskPlaceItem(placeableId, targetId, nodeId, isBeingAttachedToEntity);
        RestorePlayerControls();
        DisablePreview();
    }

    //------------------------------------------------------------------------------------------------
    override protected void OnPlacingEnded(IEntity item, bool successful, ItemUseParameters animParams)
    {
        super.OnPlacingEnded(item, successful, animParams);
        RestorePlayerControls();
    }

    //------------------------------------------------------------------------------------------------
    protected override void DisablePreview()
    {
        GetGame().GetInputManager().RemoveActionListener("CharacterFire", EActionTrigger.DOWN, StartPlaceItem);
        super.DisablePreview();
        RestorePlayerControls();
    }

    //------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------
    protected override void EOnFrame(IEntity owner, float timeSlice)
    {
        SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(GetOwner().FindComponent(SCR_CompartmentAccessComponent));
        if (!compartmentAccess)
        {
            DisablePreview();
            return;
        }

        if (compartmentAccess.IsGettingIn())
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