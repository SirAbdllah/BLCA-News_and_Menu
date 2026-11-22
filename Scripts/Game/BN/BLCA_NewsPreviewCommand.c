// Local preview only: this command never sends an RPC or publishes to other players.
class BLCA_NewsPreviewCommand : BN_NewsCommand
{
 protected string m_BLCA_PreviewFaction = "US";
 protected string m_BLCA_PreviewMessage;
 protected int m_BLCA_PreviewDuration = 20;
 override string GetKeyword() { return "newstest"; }
 override bool IsServerSide() { return false; }
 override int RequiredChatPermission() { return 0; }
 override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
 {
  m_BLCA_PreviewFaction = "US";
  m_BLCA_PreviewMessage = "عاجل: تحديث ميداني من قوات USSR | BLCA 120";
  m_BLCA_PreviewDuration = 20;
  int start = 1;
  if (argv.Count() > 1)
  {
   string key = argv[1]; key.ToLower();
   if (key == "stop") { BN_LocalNews.Stop(); return new ScrServerCmdResult("Preview fading out.", EServerCmdResultType.OK); }
   if (key == "ussr") { m_BLCA_PreviewFaction = "USSR"; start = 2; }
   if (key == "server") { m_BLCA_PreviewFaction = "SERVER"; start = 2; }
   if (key == "army" || key == "us") start = 2;
   if (argv.Count() > start && BN_NewsCommand.IsNumber(argv[start]))
   { m_BLCA_PreviewDuration = Math.ClampInt(argv[start].ToInt(), 15, 180); start++; }
   if (argv.Count() > start)
   {
    m_BLCA_PreviewMessage = "";
    for (int i = start; i < argv.Count(); i++)
    { if (i > start) m_BLCA_PreviewMessage += " "; m_BLCA_PreviewMessage += argv[i]; }
   }
  }
  GetGame().GetCallqueue().Remove(BLCA_ShowPreview);
  GetGame().GetCallqueue().CallLater(BLCA_ShowPreview, 200, false);
  return new ScrServerCmdResult("Opening local news preview...", EServerCmdResultType.OK);
 }
 protected void BLCA_ShowPreview()
 {
  SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
  bool soundEnabled = mode && mode.BN_GetNewsSound();
  bool shown = BN_LocalNews.Show(m_BLCA_PreviewMessage, m_BLCA_PreviewFaction, m_BLCA_PreviewDuration, soundEnabled);
  PrintFormat("[BLCA News] Local preview shown=%1", shown);
 }
}
