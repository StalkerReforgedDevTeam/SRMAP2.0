modded enum InstigatorType
{
	INSTIGATOR_WORLD,
	INSTIGATOR_PLAYER_STATS
}

modded enum EDamageType
{
	Starvation,
	Thirst
}

[ComponentEditorProps(category: "ADB/Components/Player", description: "")]
class ADB_PlayerDeathRecorderComponentClass : ScriptComponentClass {}

class ADB_PlayerDeathRecorderComponent : ScriptComponent
{
    protected EDamageType m_lastDamageType = EDamageType.TRUE;
	protected InstigatorType m_instigatorType = InstigatorType.INSTIGATOR_NONE;
    protected IEntity m_lastInstigatorEntity = null;
	protected int m_instigatorPlayerId = -1;
	protected string m_lastInstigatorName = "";
    protected string m_lastInstigatorFaction = "";
	protected int m_lastInstigatorBackendId = -1;

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		if (!Replication.IsServer())
			return;
        
        SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
        if (characterController)
        {
			characterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
        }
    }

    void SetLastDamageInfo(EDamageType type, Instigator instigator, float damage)
    {
		m_lastDamageType = type;
		m_lastInstigatorEntity = instigator.GetInstigatorEntity();
		m_instigatorType = instigator.GetInstigatorType();
		m_instigatorPlayerId = instigator.GetInstigatorPlayerID();
		
		m_lastInstigatorName = "";
        m_lastInstigatorFaction = "";
		m_lastInstigatorBackendId = -1;
        
		// Pul data from instigator
        SCR_ChimeraCharacter instigatorCharacter = SCR_ChimeraCharacter.Cast(instigator.GetInstigatorEntity());
		if (instigatorCharacter)
		{
			SCR_CharacterIdentityComponent charIdentity = SCR_CharacterIdentityComponent.Cast(instigatorCharacter.FindComponent(SCR_CharacterIdentityComponent));   
			if (charIdentity)
			{
				m_lastInstigatorName = charIdentity.GetIdentity().GetName() + " " + charIdentity.GetIdentity().GetSurname();
			}
			
			SCR_EditableCharacterComponent editableChar = SCR_EditableCharacterComponent.Cast(instigatorCharacter.FindComponent(SCR_EditableCharacterComponent));
			if (editableChar)
			{
				SCR_UIInfo info = editableChar.GetInfo();
				if (info)
				{
					m_lastInstigatorName = info.GetName();
				}
			}
			
			FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(instigatorCharacter.FindComponent(FactionAffiliationComponent));
			if (factionComponent)
			{
				m_lastInstigatorFaction = factionComponent.GetAffiliatedFaction().GetFactionKey();
			}
			
			// Get the instigator's backend ID
			if (m_instigatorPlayerId > 0)
			{
				PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(m_instigatorPlayerId);
				if (playerController)
				{
					ADB_BackendInfoComponent backendInfoComponent = ADB_BackendInfoComponent.Cast(playerController.FindComponent(ADB_BackendInfoComponent));
					if (backendInfoComponent)
					{
						m_lastInstigatorBackendId = backendInfoComponent.GetBackendId();
					}
				}
			}
		}
    }

    void OnLifeStateChanged(ECharacterLifeState previousState, ECharacterLifeState newState)
    {
		if (!Replication.IsServer())
			return;
		
        bool playerDied = false;
        if (newState == ECharacterLifeState.DEAD && previousState != ECharacterLifeState.DEAD)
            playerDied = true;

        if (!playerDied)
            return;
		
		if (!EntityUtils.IsPlayer(GetOwner()))
			return;

        // Inform the death recorder about the death after gathering the necessary info
		ADB_PlayerDeathReporterComponent deathReporter = ADB_PlayerDeathReporterComponent.GetInstance();
		if (deathReporter)
		{
			deathReporter.ReportDeath(GetOwner(), m_lastDamageType, m_instigatorType, m_lastInstigatorEntity, m_instigatorPlayerId, m_lastInstigatorName, m_lastInstigatorFaction, m_lastInstigatorBackendId);
		}
    }
}

// Need to mod the damage manager to be able to forward info to the recorder
modded class SCR_CharacterDamageManagerComponent : SCR_ExtendedDamageManagerComponent
{
    override protected void OnDamage(notnull BaseDamageContext damageContext)
    {
        super.OnDamage(damageContext);
		
		if (!Replication.IsServer())
			return;

        // Save the context data to the death recorder
        if (damageContext.struckHitZone && damageContext.damageValue > 0)
        {
            IEntity owner = GetOwner();
            ADB_PlayerDeathRecorderComponent deathRecorderComponent = ADB_PlayerDeathRecorderComponent.Cast(owner.FindComponent(ADB_PlayerDeathRecorderComponent));
            if (!deathRecorderComponent)
                return;

            deathRecorderComponent.SetLastDamageInfo(damageContext.damageType, damageContext.instigator, damageContext.damageValue);
        }
    }
}