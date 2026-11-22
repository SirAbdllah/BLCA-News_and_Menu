class BN_NewsCommand : ScrServerCommand
{
 override string GetKeyword() { return "news"; }
 override bool IsServerSide() { return true; }
 override ref ScrServerCmdResult OnUpdate() { return new ScrServerCmdResult("", EServerCmdResultType.OK); }
 // The owned controller and active suite permissions are checked on the server.
 override int RequiredChatPermission() { return 0; }
 override int RequiredRCONPermission() { return ERCONPermissions.PERMISSIONS_ADMIN; }
 static bool IsNumber(string token)
 {
  if (token.IsEmpty()) return false;
  for (int n = 0; n < token.Length(); n++)
  {
   int digit = token.ToAscii(n);
   if (digit < 48 || digit > 57) return false;
  }
  return true;
 }
 static BN_NewsRequest ParseRequest(array<string> argv, out string error)
 {
  error = "";
  if (argv.Count() < 2) { error = "#news [us|ussr|all|Server] [15-180 seconds] <message>"; return null; }
  BN_NewsRequest request = new BN_NewsRequest();
  int start = 1;
  string selector = argv[1]; selector.ToLower();
  if (selector == "us" || selector == "ussr" || selector == "all" || selector == "server")
  {
   start = 2;
   if (selector == "server") { request.ServerBrand = true; request.Audience = "all"; }
   else request.Audience = selector;
  }
  if (start >= argv.Count()) { error = "Add the news text after the audience and optional duration."; return null; }
  if (IsNumber(argv[start]) && (start + 1 < argv.Count() || start == 2))
  {
   request.DisplaySeconds = argv[start].ToInt(); start++;
   if (request.DisplaySeconds < 15 || request.DisplaySeconds > 180) { error = "Duration must be 15-180 seconds."; return null; }
  }
  if (start >= argv.Count()) { error = "The news message is missing."; return null; }
  for (int i = start; i < argv.Count(); i++)
  {
   if (i > start) request.Message += " ";
   request.Message += argv[i];
  }
  array<string> chars = {}; BN_Arabic.Split(request.Message, chars);
  if (chars.Count() < 1 || chars.Count() > 600 || request.Message.Contains("\n") || request.Message.Contains("\r") || request.Message.Contains("\t"))
  { error = "Use a single line of 1-600 characters."; return null; }
  return request;
 }
 static bool ParseMessage(array<string> argv, out string message, out int duration, out string error)
 {
  BN_NewsRequest request = ParseRequest(argv, error);
  if (!request) return false;
  message = request.Message; duration = request.DisplaySeconds;
  return true;
 }
 override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
 {
  return new ScrServerCmdResult("", EServerCmdResultType.OK);
 }
 override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
 {
   return new ScrServerCmdResult("Use #news in game as an administrator or Game Master.", EServerCmdResultType.ERR);
 }
 override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
 {
  PlayerManager pm = GetGame().GetPlayerManager();
  if (!Replication.IsServer() || !pm || !SA_Server.Instance || !SA_Server.Instance.Can(playerId, "news"))
   return new ScrServerCmdResult("Activate Admin Panel and request the news permission from the server owner.", EServerCmdResultType.MISSING_PERMISSION);
  SCR_BaseGameMode mode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
  if (!mode) return new ScrServerCmdResult("Unsupported game mode.", EServerCmdResultType.ERR);
  if (argv.Count() == 2 && argv[1] == "stop")
  {
   mode.BN_Publish("", "", 0, "all");
   SA_Server.Instance.Log("news.stop", SA_Server.Instance.PlayerUuid(playerId), "all", "News stopped");
   return new ScrServerCmdResult("News is fading out.", EServerCmdResultType.OK);
  }
  string error;
  BN_NewsRequest request = ParseRequest(argv, error);
  if (!request) return new ScrServerCmdResult(error, EServerCmdResultType.PARAMETERS);
  string senderFaction = BN_NewsAudience.PlayerFaction(pm.GetPlayerController(playerId));
  string audience = request.Audience;
  if (audience == "self")
  {
   if (senderFaction.IsEmpty()) return new ScrServerCmdResult("Select a faction first, or use #news server <seconds> <message>.", EServerCmdResultType.PARAMETERS);
   audience = "faction:" + senderFaction;
  }
  if (request.ServerBrand || senderFaction.IsEmpty()) senderFaction = "SERVER";
  if (!mode.BN_Publish(request.Message, senderFaction, request.DisplaySeconds, audience))
   return new ScrServerCmdResult("Please wait 3 seconds between broadcasts.", EServerCmdResultType.BUSY);
  PrintFormat("[BLCA News] admin=%1 audience=%2 sender=%3 duration=%4 text=%5", playerId, audience, senderFaction, request.DisplaySeconds, request.Message);
  SA_Server.Instance.Log("news.send", SA_Server.Instance.PlayerUuid(playerId), audience, request.Message);
  return new ScrServerCmdResult("News sent to audience: " + audience, EServerCmdResultType.OK);
 }
}
