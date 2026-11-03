// Armor rework damage hooks. Chest and head hits on stalkers are normalised to the Armor Rework Chart:
// the struck zone takes exactly the chart share per hit for the vest or helmet tier the character is wearing,
// whether the round went through the armor or was stopped by it.
// Limbs, hips and neck stay vanilla. Mutants are untouched. Tables live in SRZ_ArmorTuning.
modded class SCR_CharacterHitZone
{
	//------------------------------------------------------------------------------------------------
	float SRZ_BaseEffective(notnull BaseDamageContext damageContext, bool isDOT)
	{
		return super.ComputeEffectiveDamage(damageContext, isDOT);
	}

	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		float effective = super.ComputeEffectiveDamage(damageContext, isDOT);

		if (isDOT || damageContext.damageType != EDamageType.KINETIC || effective <= 0 || !SRZ_ArmorTuning.IsBullet(damageContext.damageSource))
			return effective;

		SCR_CharacterHitZone struck = SCR_CharacterHitZone.Cast(damageContext.struckHitZone);
		if (!struck)
			return effective;

		ChimeraCharacter character = ChimeraCharacter.Cast(damageContext.hitEntity);
		if (!character || Bacon_622120A5448725E3_InfectedCharacter.Cast(character))
			return effective;

		SRZ_Caliber cal = SRZ_ArmorTuning.CaliberOf(damageContext.damageSource);

		string zone = struck.GetName();
		float target;
		if (SRZ_ArmorTuning.IsVestZone(zone))
			target = SRZ_ArmorTuning.ChestDamage(cal, SRZ_ArmorTuning.WornVestTier(character));
		else if (zone == "Head")
			target = SRZ_ArmorTuning.HeadDamage(cal, SRZ_ArmorTuning.WornHelmetTier(character));
		else if (SRZ_ArmorTuning.IsLimbZone(zone))
			return effective * SRZ_ArmorTuning.LIMB_SCALE;
		else
			return effective;

		if (struck == this)
			return target;

		// The health pool takes a fixed share of the chart number, whatever the engine worked out on the way through
		if (GetName() == "Health")
			return target * HEALTH_SHARE;

		float base = struck.SRZ_BaseEffective(damageContext, isDOT);
		if (base <= 0.01)
			return effective;

		return effective * (target / base);
	}

	static const float HEALTH_SHARE = 0.9;
}

modded class SCR_CharacterResilienceHitZone
{
	//------------------------------------------------------------------------------------------------
	// Resilience lost per chest hit is set so the knockout lands at KO_FRACTION of the kill count
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		float effective = super.ComputeEffectiveDamage(damageContext, isDOT);

		if (isDOT || damageContext.damageType != EDamageType.KINETIC || effective <= 0 || !damageContext.struckHitZone || !SRZ_ArmorTuning.IsBullet(damageContext.damageSource))
			return effective;

		if (!SRZ_ArmorTuning.IsVestZone(damageContext.struckHitZone.GetName()))
			return effective;

		ChimeraCharacter character = ChimeraCharacter.Cast(damageContext.hitEntity);
		if (!character || Bacon_622120A5448725E3_InfectedCharacter.Cast(character))
			return effective;

		SRZ_Caliber cal = SRZ_ArmorTuning.CaliberOf(damageContext.damageSource);
		return SRZ_ArmorTuning.ResilienceDamage(cal, SRZ_ArmorTuning.WornVestTier(character));
	}
}

modded class SCR_ArmorDamageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	// Vanilla hijacks a round the armor stopped and only drains resilience by the pass scale.
	// Instead the character takes a chest or head hit, which flows through SCR_CharacterHitZone above
	// and comes out at the chart number for the worn tier.
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.KINETIC || !Replication.IsServer() || !SRZ_ArmorTuning.IsBullet(damageContext.damageSource))
			return super.HijackDamageHandling(damageContext);

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner().GetParent());
		if (!character || Bacon_622120A5448725E3_InfectedCharacter.Cast(character))
			return super.HijackDamageHandling(damageContext);

		SCR_CharacterDamageManagerComponent charDM = SCR_CharacterDamageManagerComponent.Cast(character.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!charDM)
			return super.HijackDamageHandling(damageContext);

		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(character.FindComponent(EquipedLoadoutStorageComponent));
		bool isHelmet = loadout && loadout.GetClothFromArea(LoadoutHeadCoverArea) == GetOwner();

		string zoneName = "Chest";
		if (isHelmet)
			zoneName = "Head";

		HitZone zone = charDM.GetHitZoneByName(zoneName);
		if (!zone)
			return super.HijackDamageHandling(damageContext);

		SRZ_Caliber cal = SRZ_ArmorTuning.CaliberOf(damageContext.damageSource);
		float dmg;
		if (isHelmet)
			dmg = SRZ_ArmorTuning.HeadDamage(cal, SRZ_ArmorTuning.WornHelmetTier(character));
		else
			dmg = SRZ_ArmorTuning.ChestDamage(cal, SRZ_ArmorTuning.WornVestTier(character));

		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = damageContext.hitPosition;
		hitPosDirNorm[1] = damageContext.hitDirection;
		hitPosDirNorm[2] = damageContext.hitNormal;

		SCR_DamageContext context = new SCR_DamageContext(EDamageType.KINETIC, dmg, hitPosDirNorm, character, zone, damageContext.instigator, null, -1, -1);
		context.damageSource = damageContext.damageSource;
		context.damageEffect = new SCR_MeleeDamageEffect();
		charDM.HandleDamage(context);

		return true;
	}
}
