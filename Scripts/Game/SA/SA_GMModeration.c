class SA_ModerationRules
{
 static bool ValidReason(string reason)
 {
  array<string> chars = {}; BN_Arabic.Split(reason, chars);
  if (chars.Count() < 1 || chars.Count() > 500) return false;
  reason.Replace(" ", ""); reason.Replace("\r", ""); reason.Replace("\n", ""); reason.Replace("\t", "");
  return !reason.IsEmpty();
 }
 static bool CanLocal(string action)
 {
  if (!SA_Client.active || !SA_Client.players) return false;
  if (action == "kick") return SA_Client.kick;
  if (action == "ban") return SA_Client.ban;
  return action == "players";
 }
 static bool BanMinutes(string text, out int minutes)
 {
  if (!BN_NewsCommand.IsNumber(text) || text.Length() > 5) return false;
  minutes = text.ToInt(); return minutes >= 0 && minutes <= 43200;
 }
}
class SA_GMModerationDialog : SCR_ConfigurableDialogUi
{
 protected ref SA_ModerationTarget m_Target;
 protected ref SA_NoticeRenderer m_Renderer;
 static void Open(string json)
 {
  SA_ModerationTarget target = new SA_ModerationTarget(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  if (!load.ImportFromString(json) || !load.ReadValue("", target) || !target.player || !SA_ModerationRules.CanLocal(target.action)) return;
  SA_GMModerationDialog dialog = new SA_GMModerationDialog(); dialog.m_Target = target;
  SCR_ConfigurableDialogUi.CreateFromPreset("{64A9DC260EE14995}Configs/Editor/SA/ModerationDialog.conf", "blca_moderation", dialog);
 }
 override protected void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
 {
  if (!m_Target || !m_Target.player) { Close(); return; }
  if (m_Target.action == "ban") SetTitle("BLCA | BAN PLAYER"); else SetTitle("BLCA | KICK PLAYER");
  m_Renderer = new SA_NoticeRenderer(); m_Renderer.DrawSmall(GetRootWidget().FindAnyWidget("TargetName"), m_Target.player.name);
  TextWidget.Cast(GetRootWidget().FindAnyWidget("TargetUuid")).SetText(m_Target.player.uuid);
  GetRootWidget().FindAnyWidget("Duration").SetVisible(m_Target.action == "ban");
  EditBoxWidget reason = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Reason"));
  EditBoxWidget minutes = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Minutes"));
  if (reason) { reason.SetText(""); reason.SetTextColor(0xFFF0EDE5); }
  if (minutes) { minutes.SetText("60"); minutes.SetTextColor(0xFFF0EDE5); }
  SCR_InputButtonComponent confirm = FindButton("confirm");
  if (confirm) { if (m_Target.action == "ban") confirm.SetLabel("BAN PLAYER"); else confirm.SetLabel("KICK PLAYER"); }
  GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget().FindAnyWidget("Reason"));
 }
 override protected void OnConfirm()
 {
  if (!m_Target || !SA_ModerationRules.CanLocal(m_Target.action)) { Close(); return; }
  string reason = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Reason")).GetText(); int minutes;
  TextWidget error = TextWidget.Cast(GetRootWidget().FindAnyWidget("Error"));
  if (!SA_ModerationRules.ValidReason(reason)) { error.SetText("Enter a reason of 1-500 characters."); return; }
  if (m_Target.action == "ban" && !SA_ModerationRules.BanMinutes(EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Minutes")).GetText(), minutes))
  { error.SetText("Ban duration must be 0-43200 minutes. 0 = permanent."); return; }
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
  if (pc) pc.SA_Action(m_Target.action, m_Target.player.uuid, reason, minutes);
  Close();
 }
 override void OnMenuUpdate(float tDelta)
 {
  if (m_Target && !SA_ModerationRules.CanLocal(m_Target.action)) Close();
 }
}
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SA_GMPlayerContextAction : SCR_BaseContextAction
{
 [Attribute("kick")] protected string m_sModerationAction;
 override bool IsServer() { return false; }
 static int PlayerFromEntity(SCR_EditableEntityComponent entity)
 {
  if (!entity || !GetGame() || !GetGame().GetPlayerManager()) return 0;
  SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
  if (delegate) return delegate.GetPlayerID();
  int id = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity.GetOwner());
  if (id <= 0) id = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(entity.GetOwner());
  return id;
 }
 protected int Target(SCR_EditableEntityComponent hoveredEntity, set<SCR_EditableEntityComponent> selectedEntities)
 {
  if (selectedEntities.Count() > 1) return 0;
  if (selectedEntities.Count() == 1) return PlayerFromEntity(selectedEntities[0]);
  return PlayerFromEntity(hoveredEntity);
 }
 override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
 {
  SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
  return editor && !editor.IsLimited() && SA_ModerationRules.CanLocal(m_sModerationAction) && Target(hoveredEntity, selectedEntities) > 0;
 }
 override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
 { return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags); }
 override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
 {
  if (!CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags)) return;
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
  if (pc) pc.SA_RequestModeration(m_sModerationAction, Target(hoveredEntity, selectedEntities));
 }
}
// Replace the native variants, which bypass suite flags and do not collect a reason/date.
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
modded class SCR_KickPlayerContextAction
{
 override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags) { return false; }
 override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags) { return false; }
 override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1) {}
}
[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of editor context actions", icon: "WBData/ComponentEditorProps/componentEditor.png")]
modded class SCR_ContextActionsEditorComponentClass
{
 protected ref SCR_EditorActionList m_SA_ModerationActions;
 void SCR_ContextActionsEditorComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
 {
  Resource resource = BaseContainerTools.LoadContainer("{64A9DC260EE14994}Configs/Editor/SA/ModerationActions.conf");
  if (resource && resource.IsValid()) m_SA_ModerationActions = SCR_EditorActionList.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
  if (m_SA_ModerationActions) foreach (SCR_BaseEditorAction action : m_SA_ModerationActions.m_Actions) if (action && action.IsEnabled()) m_ActionsSorted.Insert(action);
 }
}
