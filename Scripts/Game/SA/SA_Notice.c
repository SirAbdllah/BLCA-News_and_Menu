class SA_NoticeRenderer : BN_NewsHUD
{
 void Draw(Widget parent, string text) { MakeLine(parent, text, 0.46); }
 void DrawSmall(Widget parent, string text) { MakeLine(parent, text, 0.34); }
 void DrawScale(Widget parent, string text, float scale) { MakeLine(parent, text, scale); }
 float Measure(string text, float scale = 0.46)
 {
  array<string> visual = {}; BN_Arabic.Visual(text, m_Forms, visual);
  float width;
  foreach (string character : visual) { int index; if (!m_Indices.Find(character, index)) index = m_Indices.Get("?"); width += m_Widths[index] * scale; }
  return width;
 }
}
class SA_NoticeHandler : ScriptedWidgetEventHandler
{
 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (w.GetName() == "Close") { SA_Notice.Hide(); return true; }
  return false;
 }
}
class SA_Notice
{
 protected static Widget s_Root;
 protected static ref SA_NoticeRenderer s_Renderer;
 protected static ref SA_NoticeHandler s_Handler;
 protected static int s_Deadline;
 protected static int s_LastSecond = -1;
 static void Hide()
 {
  if (GetGame()) GetGame().GetCallqueue().Remove(TickCountdown);
  if (s_Root) s_Root.RemoveFromHierarchy();
  s_Root = null; s_Renderer = null; s_Handler = null; s_Deadline = 0; s_LastSecond = -1;
 }
 protected static void DrawWrapped(string prefix, string text, int maxRows, float scale)
 {
  array<string> characters = {}; BN_Arabic.Split(text, characters);
  string line; int row;
  foreach (string character : characters)
  {
   if (s_Renderer.Measure(line + character, scale) > 940 && !line.IsEmpty())
   {
    if (row >= maxRows) return;
    s_Renderer.DrawScale(s_Root.FindAnyWidget(prefix + row), line, scale); row++; line = "";
   }
   line += character;
  }
  if (!line.IsEmpty() && row < maxRows) s_Renderer.DrawScale(s_Root.FindAnyWidget(prefix + row), line, scale);
 }
 static bool ShowBan(SA_BanNoticeData notice, bool countdown)
 {
  if (!GetGame() || !GetGame().GetWorkspace() || !notice) return false;
  Hide();
  s_Root = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14992}UI/Layouts/SA/Notice.layout");
  if (!s_Root) return false;
  s_Root.SetZOrder(3000); s_Renderer = new SA_NoticeRenderer();
  DrawWrapped("Intro", SA_BanNoticeData.DISCORD_TEXT, 2, 0.46);
  DrawWrapped("Line", notice.reason, 12, 0.42);
  DrawWrapped("Expiry", notice.expiry, 2, 0.46);
  Widget close = s_Root.FindAnyWidget("Close"); close.SetVisible(!countdown); close.SetEnabled(!countdown);
  s_Root.FindAnyWidget("Countdown").SetVisible(countdown);
  if (countdown)
  {
   s_Deadline = System.GetTickCount() + SA_BanWait.WAIT_MS;
   TickCountdown(); GetGame().GetCallqueue().CallLater(TickCountdown, 200, true);
  }
  else
  {
   s_Handler = new SA_NoticeHandler(); close.AddHandler(s_Handler);
   GetGame().GetWorkspace().SetFocusedWidget(close);
  }
  return true;
 }
 protected static void TickCountdown()
 {
  if (!s_Root) { if (GetGame()) GetGame().GetCallqueue().Remove(TickCountdown); return; }
  int left = Math.Max(0, s_Deadline - System.GetTickCount()); int seconds = (left + 999) / 1000;
  if (s_LastSecond == seconds) return;
  s_LastSecond = seconds;
  TextWidget label = TextWidget.Cast(s_Root.FindAnyWidget("Countdown"));
  if (seconds > 0) label.SetText("Disconnecting in " + seconds + " seconds...");
  else label.SetText("Waiting for server disconnect...");
 }
 static void ShowPendingBan()
 {
  Hide();
  SA_BanNoticeData notice = new SA_BanNoticeData(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  if (load.LoadFromFile("$profile:BLCAAdminSuite_LastBan.json") && load.ReadValue("", notice))
  {
   FileIO.DeleteFile("$profile:BLCAAdminSuite_LastBan.json");
   FileIO.DeleteFile("$profile:BLCAAdminSuite_LastBan.txt");
   SA_Client.banToken = 0; ShowBan(notice, false); return;
  }
  FileHandle file = FileIO.OpenFile("$profile:BLCAAdminSuite_LastBan.txt", FileMode.READ);
  string previous;
  if (file) { file.ReadLine(previous); file.Close(); FileIO.DeleteFile("$profile:BLCAAdminSuite_LastBan.txt"); }
  if (!previous.IsEmpty()) { notice.reason = previous; notice.expiry = "See the server ban details above"; ShowBan(notice, false); }
 }
}
modded class MainMenuUI
{
 override void OnMenuOpen()
 {
  super.OnMenuOpen();
  GetGame().GetCallqueue().CallLater(SA_Notice.ShowPendingBan, 1500, false);
 }
}
