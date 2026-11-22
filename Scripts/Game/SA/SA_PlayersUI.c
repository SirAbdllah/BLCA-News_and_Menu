class SA_PlayersHandler : ScriptedWidgetEventHandler
{
 override bool OnClick(Widget w, int x, int y, int button)
 {
  while (w && w.GetName() != "SA_Players")
  {
   if (SA_PlayersUI.Click(w.GetName())) return true;
   w = w.GetParent();
  }
  return false;
 }
}
class SA_PlayersUI
{
 protected static Widget s_Root;
 protected static ref SA_Status s_Data;
 protected static ref SA_PlayersHandler s_Handler;
 protected static ref array<ref BLCA_DialogButtonVisual> s_ActionStyles = {};
 protected static ref SA_NoticeRenderer s_Renderer;
 protected static int s_Page;
 protected static string s_Selected;
 protected static bool s_Awaiting;
 static void Hide()
 {
  SA_Client.onStateChanged.Remove(OnStateChanged);
  if (s_Root) s_Root.RemoveFromHierarchy();
  s_ActionStyles.Clear();
  s_Root = null; s_Data = null; s_Handler = null; s_Renderer = null;
  s_Page = 0; s_Selected = ""; s_Awaiting = false;
 }
 static bool HasAllControls(Widget root)
 {
  if (!root) return false;
  for (int i = 0; i < 8; i++)
   if (!root.FindAnyWidget("Row" + i) || !root.FindAnyWidget("Label" + i)) return false;
  return root.FindAnyWidget("Previous") && root.FindAnyWidget("Next") && root.FindAnyWidget("Kick") && root.FindAnyWidget("Ban") && root.FindAnyWidget("Close") && TextWidget.Cast(root.FindAnyWidget("Page")) && TextWidget.Cast(root.FindAnyWidget("Selected")) && TextWidget.Cast(root.FindAnyWidget("Status")) && EditBoxWidget.Cast(root.FindAnyWidget("Reason")) && EditBoxWidget.Cast(root.FindAnyWidget("Minutes"));
 }
 static void Show(string json)
 {
  if (!SA_Client.active || !SA_Client.players) return;
  SA_Status data = new SA_Status(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  if (!load.ImportFromString(json) || !load.ReadValue("", data) || !data.players) return;
  Hide(); s_Data = data;
  s_Root = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14993}UI/Layouts/SA/Players.layout");
  if (!HasAllControls(s_Root))
  {
   Print("[BLCA Admin] Players layout is incomplete. Update all addon files.", LogLevel.ERROR);
   Hide(); return;
  }
  s_Root.SetZOrder(2000); s_Handler = new SA_PlayersHandler(); s_Root.AddHandler(s_Handler);
  BLCA_PauseDialogStyle.ApplyCustomPanel(s_Root);
  array<string> actions = {"Kick", "Ban", "Close"};
  foreach (string action : actions)
   s_ActionStyles.Insert(BLCA_PauseDialogStyle.ApplyCustomButton(s_Root, action));
  BLCA_PauseDialogStyle.Tint(s_Root, "ReasonUnderline", BLCA_PausePalette.Outline());
  BLCA_PauseDialogStyle.Tint(s_Root, "MinutesUnderline", BLCA_PausePalette.Outline());
  EditBoxWidget reason = EditBoxWidget.Cast(s_Root.FindAnyWidget("Reason"));
  EditBoxWidget minutes = EditBoxWidget.Cast(s_Root.FindAnyWidget("Minutes"));
  reason.SetText(""); reason.SetTextColor(0xFFF0EDE5);
  minutes.SetText("60"); minutes.SetTextColor(0xFFF0EDE5);
  s_Renderer = new SA_NoticeRenderer();
  SA_Client.onStateChanged.Insert(OnStateChanged);
  Draw();
  if (!s_Data.players.IsEmpty()) GetGame().GetWorkspace().SetFocusedWidget(s_Root.FindAnyWidget("Row0"));
 }
 protected static void OnStateChanged()
 {
  if (!SA_Client.active || !SA_Client.players) { Hide(); return; }
  if (!s_Root) return;
  Draw();
  if (s_Awaiting && !SA_Client.message.IsEmpty())
  {
   Status(SA_Client.message); s_Awaiting = false;
  }
 }
 protected static void Status(string text)
 {
  if (!s_Root) return;
  TextWidget label = TextWidget.Cast(s_Root.FindAnyWidget("Status"));
  if (label) label.SetText(text);
 }
 protected static void Enable(string name, bool enabled)
 {
  Widget item = s_Root.FindAnyWidget(name);
  if (!item) return;
  item.SetEnabled(enabled);
  if (name == "Kick" || name == "Ban") return;
  if (enabled) item.SetOpacity(1); else item.SetOpacity(0.4);
 }
 protected static void Draw()
 {
  if (!s_Root || !s_Data || !s_Renderer) return;
  for (int row = 0; row < 8; row++)
  {
   int index = s_Page * 8 + row;
   Widget button = s_Root.FindAnyWidget("Row" + row);
   Widget label = s_Root.FindAnyWidget("Label" + row);
   if (!button || !label) continue;
   button.SetVisible(index < s_Data.players.Count());
   Widget child = label.GetChildren();
   while (child) { Widget next = child.GetSibling(); child.RemoveFromHierarchy(); child = next; }
   if (index < s_Data.players.Count())
   {
    SA_Player player = s_Data.players[index];
    if (!player) continue;
    s_Renderer.DrawSmall(label, player.playerId.ToString() + " | " + player.name + " | " + player.platform);
    Widget background = s_Root.FindAnyWidget("Row" + row + "BG");
    if (background)
    {
     if (player.uuid == s_Selected) background.SetColor(BLCA_PausePalette.Selected());
     else background.SetColor(BLCA_PausePalette.Button());
    }
   }
  }
  bool selected = !s_Selected.IsEmpty();
  Enable("Kick", selected && SA_Client.active && SA_Client.kick);
  Enable("Ban", selected && SA_Client.active && SA_Client.ban);
  Enable("Previous", s_Page > 0);
  Enable("Next", (s_Page + 1) * 8 < s_Data.players.Count());
  int pages = Math.Max(1, (s_Data.players.Count() + 7) / 8);
  TextWidget pageLabel = TextWidget.Cast(s_Root.FindAnyWidget("Page"));
  if (pageLabel) pageLabel.SetText("PAGE " + (s_Page + 1) + " / " + pages + " | " + s_Data.players.Count() + " PLAYERS");
  TextWidget chosen = TextWidget.Cast(s_Root.FindAnyWidget("Selected"));
  if (chosen)
  {
   if (selected) chosen.SetText("Selected UUID: " + s_Selected);
   else chosen.SetText("Select a player from the list above.");
  }
 }
 static bool Click(string name)
 {
  if (!s_Root || !s_Data) return false;
  if (name == "Close") { Hide(); return true; }
  if (name == "Next") { if ((s_Page + 1) * 8 < s_Data.players.Count()) { s_Page++; s_Selected = ""; } Draw(); return true; }
  if (name == "Previous") { if (s_Page > 0) { s_Page--; s_Selected = ""; } Draw(); return true; }
  if (name == "Reason" || name == "Minutes")
  {
   EditBoxWidget input = EditBoxWidget.Cast(s_Root.FindAnyWidget(name));
   if (input) input.ActivateWriteMode();
   return true;
  }
  for (int row = 0; row < 8; row++)
  {
   if (name != "Row" + row) continue;
   int index = s_Page * 8 + row;
   if (index < s_Data.players.Count() && s_Data.players[index]) s_Selected = s_Data.players[index].uuid;
   Status("Enter the reason, then choose KICK or BAN."); Draw(); return true;
  }
  if (name == "Kick" || name == "Ban")
  {
   if (s_Selected.IsEmpty()) { Status("Select a player first."); return true; }
   if (!SA_Client.active || !SA_Client.players || (name == "Kick" && !SA_Client.kick) || (name == "Ban" && !SA_Client.ban))
   { Status("Permission denied."); return true; }
   EditBoxWidget reasonInput = EditBoxWidget.Cast(s_Root.FindAnyWidget("Reason"));
   EditBoxWidget durationInput = EditBoxWidget.Cast(s_Root.FindAnyWidget("Minutes"));
   if (!reasonInput || !durationInput) return true;
   string reason = reasonInput.GetText(); int minutes;
   if (!SA_ModerationRules.ValidReason(reason)) { Status("Enter a reason of 1-500 characters."); return true; }
   if (name == "Ban" && !SA_ModerationRules.BanMinutes(durationInput.GetText(), minutes))
   { Status("Ban minutes must be 0-43200. 0 = permanent."); return true; }
   SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
   if (pc)
   {
    s_Awaiting = true; Status("Sending request...");
    if (name == "Kick") pc.SA_Action("kick", s_Selected, reason, 0);
    else pc.SA_Action("ban", s_Selected, reason, minutes);
   }
   return true;
  }
  return false;
 }
}
