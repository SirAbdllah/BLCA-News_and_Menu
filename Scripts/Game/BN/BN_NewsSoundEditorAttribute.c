[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class BN_NewsSoundEditorAttribute : SCR_BaseEditorAttribute
{
 override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
 {
  SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(item);
  if (!mode) return null;
  return SCR_BaseEditorAttributeVar.CreateBool(mode.BN_GetNewsSound());
 }
 override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
 {
  SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(item);
  if (mode && var && Replication.IsServer()) mode.BN_SetNewsSound(var.GetBool());
 }
}
modded class SCR_AttributesManagerEditorComponentClass
{
 protected ref SCR_BaseEditorAttribute m_BN_NewsSoundAttribute;
 void SCR_AttributesManagerEditorComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
 {
  Resource attribute = BaseContainerTools.LoadContainer("{57C3F6639C3C4201}Configs/Editor/BN/NewsSound.conf");
  if (attribute && attribute.IsValid())
   m_BN_NewsSoundAttribute = SCR_BaseEditorAttribute.Cast(BaseContainerTools.CreateInstanceFromContainer(attribute.GetResource().ToBaseContainer()));
  if (m_BN_NewsSoundAttribute) m_aAttributes.Insert(m_BN_NewsSoundAttribute);
 }
}
