// Fixes unchecked pointer/nullptr crash
[BaseContainerProps()]
modded class ARMST_Consumable : SCR_ConsumableEffectBase
{
//	override void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
//	{
//		if (Temp == null)
//			return;
//		
//		ItemUseComponent = ARMST_ItemUseComponent.Cast(Temp.FindComponent(ARMST_ItemUseComponent));
//		if (ItemUseComponent == null)
//			return;
//		
//		super.AddConsumableDamageEffects(char, instigator);
//	}
//	
//	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
//	{
//		if (Temp == null)
//			return;
//		
//		ItemUseComponent = ARMST_ItemUseComponent.Cast(Temp.FindComponent(ARMST_ItemUseComponent));
//		if (ItemUseComponent == null)
//			return;
//		
//		super.ApplyEffect(target, user, item, animParams);
//	}
	
}

