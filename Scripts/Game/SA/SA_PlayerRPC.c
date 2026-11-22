modded class SCR_PlayerController
{
 void SA_Request(string action)
 {
  if (Replication.IsServer()) SA_RequestServer(action);
  else Rpc(SA_RequestServer, action);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Server)]
 protected void SA_RequestServer(string action)
 {
  SA_Server server = SA_Server.Instance;
  if (!server || !server.RateAllowed(GetPlayerId())) return;
  // Identity comes from this owned controller, never from an argument sent by the client.
  if (action == "login") server.Login(GetPlayerId());
  else if (action == "logout") server.Logout(GetPlayerId(), "Signed out from Admin Panel");
  else if (action == "state") server.SendState(GetPlayerId(), "");
  else if (action == "players") server.PlayerList(GetPlayerId());
 }
 void SA_Action(string action, string uuid, string reason, int minutes)
 {
  if (Replication.IsServer()) SA_ActionServer(action, uuid, reason, minutes);
  else Rpc(SA_ActionServer, action, uuid, reason, minutes);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Server)]
 protected void SA_ActionServer(string action, string uuid, string reason, int minutes)
 {
  if (SA_Server.Instance) SA_Server.Instance.PlayerAction(GetPlayerId(), action, uuid, reason, minutes);
 }
 void SA_RequestModeration(string action, int targetId)
 {
  if (Replication.IsServer()) SA_RequestModerationServer(action, targetId);
  else Rpc(SA_RequestModerationServer, action, targetId);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Server)]
 protected void SA_RequestModerationServer(string action, int targetId)
 {
  if (SA_Server.Instance) SA_Server.Instance.ModerationTarget(GetPlayerId(), targetId, action);
 }
 void SA_SendModerationTarget(string json)
 {
  Rpc(SA_ReceiveModerationTarget, json);
  if (GetGame().GetPlayerController() == this) SA_ReceiveModerationTarget(json);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void SA_ReceiveModerationTarget(string json) { SA_GMModerationDialog.Open(json); }
 void SA_SendPlayers(string json)
 {
  Rpc(SA_ReceivePlayers, json);
  if (GetGame().GetPlayerController() == this) SA_ReceivePlayers(json);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void SA_ReceivePlayers(string json) { SA_PlayersUI.Show(json); }
 void SA_SendState(bool eligible, bool active, SA_Staff grant, string message)
 {
  bool news, players, kick, ban; string rank;
  if (grant) { news = grant.news; players = grant.players; kick = grant.kick; ban = grant.ban; rank = grant.rank; }
  Rpc(SA_ReceiveState, eligible, active, news, players, kick, ban, rank, message);
  if (GetGame().GetPlayerController() == this) SA_ReceiveState(eligible, active, news, players, kick, ban, rank, message);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void SA_ReceiveState(bool eligible, bool active, bool news, bool players, bool kick, bool ban, string rank, string message)
 {
  bool changed = SA_Client.eligible != eligible || SA_Client.active != active || SA_Client.news != news || SA_Client.players != players || SA_Client.kick != kick || SA_Client.ban != ban || SA_Client.rank != rank;
  if (!message.IsEmpty() && SA_Client.message != message) changed = true;
  SA_Client.eligible = eligible; SA_Client.active = active; SA_Client.news = news;
  SA_Client.players = players; SA_Client.kick = kick; SA_Client.ban = ban; SA_Client.rank = rank;
  if (!message.IsEmpty()) SA_Client.message = message;
  if (changed || !message.IsEmpty()) SA_Client.onStateChanged.Invoke();
 }
 void SA_SendBanNotice(string json)
 {
  Rpc(SA_ReceiveBanNotice, json);
  if (GetGame().GetPlayerController() == this) SA_ReceiveBanNotice(json);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void SA_ReceiveBanNotice(string json)
 {
  SA_BanNoticeData notice = new SA_BanNoticeData(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  if (!load.ImportFromString(json) || !load.ReadValue("", notice) || notice.token <= 0) return;
  if (SA_Client.banToken == notice.token) return;
  SA_Client.banToken = notice.token;
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", notice); save.SaveToFile("$profile:BLCAAdminSuite_LastBan.json");
  if (SA_Notice.ShowBan(notice, true))
  {
   if (Replication.IsServer()) SA_BanNoticeReady(notice.token);
   else Rpc(SA_BanNoticeReady, notice.token);
  }
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Server)]
 protected void SA_BanNoticeReady(int token) { if (SA_Server.Instance) SA_Server.Instance.BanNoticeReceived(GetPlayerId(), token); }
 void SA_SendBanCancel(int token)
 {
  Rpc(SA_ReceiveBanCancel, token);
  if (GetGame().GetPlayerController() == this) SA_ReceiveBanCancel(token);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void SA_ReceiveBanCancel(int token)
 {
  if (SA_Client.banToken != token) return;
  SA_Notice.Hide(); SA_Client.banToken = 0; SA_Client.banNotice = "";
  FileIO.DeleteFile("$profile:BLCAAdminSuite_LastBan.json");
  FileIO.DeleteFile("$profile:BLCAAdminSuite_LastBan.txt");
 }
}

modded class PauseMenuUI
{
 protected ref SA_PausePanel m_SA_Panel;
 protected string m_SA_LastMessage;
 override void OnMenuOpen()
 {
  super.OnMenuOpen();
  SA_HideFeedback();
  m_SA_Panel = new SA_PausePanel(); m_SA_Panel.Bind(m_wRoot);
  SCR_ButtonTextComponent button = SCR_ButtonTextComponent.GetButtonText("FieldManual", m_wRoot);
  if (button) { button.m_OnClicked.Clear(); button.m_OnClicked.Insert(SA_TogglePanel); }
  SCR_ButtonTextComponent players = SCR_ButtonTextComponent.GetButtonText("Players", m_wRoot);
  if (players) { players.m_OnClicked.Clear(); players.m_OnClicked.Insert(SA_OpenPlayers); }
  SA_Client.onStateChanged.Insert(SA_UpdatePanel);
  SA_UpdatePanel();
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
  if (pc) pc.SA_Request("state");
 }
 override void OnMenuShow() { super.OnMenuShow(); SA_HideFeedback(); SA_UpdatePanel(); }
 override void OnMenuFocusGained()
 {
  super.OnMenuFocusGained(); SA_HideFeedback(); SA_UpdatePanel();
 }
 override void OnMenuClose()
 {
  SA_Client.onStateChanged.Remove(SA_UpdatePanel);
  m_SA_Panel = null; m_SA_LastMessage = "";
  SA_PlayersUI.Hide();
  super.OnMenuClose();
 }
 protected void SA_UpdatePanel()
 {
  if (!m_wRoot || !m_SA_Panel) return;
  if (!SA_Client.active || !SA_Client.players) SA_PlayersUI.Hide();
  m_SA_Panel.Apply(SA_Client.eligible, SA_Client.active, SA_Client.players);
  if (m_wVersion && !SA_Client.message.IsEmpty() && SA_Client.message != m_SA_LastMessage)
  { m_wVersion.SetText(SA_Client.message); m_SA_LastMessage = SA_Client.message; }
 }
 protected void SA_TogglePanel()
 {
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
  if (!pc) return;
  if (SA_Client.active) pc.SA_Request("logout");
  else pc.SA_Request("login");
 }
 protected void SA_OpenPlayers()
 {
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
  if (pc && SA_Client.active && SA_Client.players) pc.SA_Request("players");
 }
 protected void SA_HideFeedback()
 {
  if (!m_wRoot) return;
  SCR_ButtonTextComponent button = SCR_ButtonTextComponent.GetButtonText("Feedback", m_wRoot);
  if (!button) return;
  button.m_OnClicked.Clear();
  button.SetEnabled(false, false);
  button.SetVisible(false, false);
 }
}
