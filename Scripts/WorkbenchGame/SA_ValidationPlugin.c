[WorkbenchPluginAttribute(name: "Validate BLCA News_and_Menu", wbModules: {"ResourceManager"})]
class SA_ValidationPlugin : WorkbenchPlugin
{
 override void Run()
 {
  SA_PolicyProbe probe = new SA_PolicyProbe(null); probe.Seed();
  PrintFormat("SA_STAFF_START_INACTIVE=%1", probe.Grant(1) && !probe.Active(1) && !probe.Can(1, "gm"));
  probe.Login(1); probe.Login(2); probe.Login(3); probe.Login(4);
  PrintFormat("SA_THREE_GM_LIMIT=%1", probe.Active(1) && probe.Active(2) && probe.Active(3) && !probe.Active(4) && probe.ActiveGMCount(true) == 3);
  probe.Login(5); probe.Login(6);
  PrintFormat("SA_BYPASS_EXCLUDES_ORDINARY_SLOTS=%1", probe.Active(5) && probe.Active(6) && probe.ActiveGMCount() == 4 && probe.ActiveGMCount(true) == 3);
  PrintFormat("SA_NEWS_SEPARATE_PERMISSION=%1", !probe.Can(1, "news") && probe.Can(2, "news") && probe.Can(6, "news") && !probe.Can(6, "gm"));
  probe.RemoveNews(6); probe.Revoke(2);
  PrintFormat("SA_PERMISSION_REVOKED_AT_ACTION=%1", !probe.Can(6, "news") && !probe.Can(2, "gm") && !probe.Active(2));
  probe.Logout(1, "test"); probe.Login(4);
  PrintFormat("SA_LOGOUT_REVOKES_GM_AND_FREES_SLOT=%1", !probe.Active(1) && !probe.gmState.Get(1) && probe.Active(4));
  SA_Ban ban = new SA_Ban(); ban.enabled = true; ban.expires = 0;
  PrintFormat("SA_PERMANENT_BAN=%1", ban.Active(100));
  ban.expires = 101;
  PrintFormat("SA_TIMED_BAN_BOUNDARY=%1", ban.Active(100) && !ban.Active(101));
  ban.enabled = false;
  PrintFormat("SA_LIFTED_BAN=%1", !ban.Active(100));
  SA_Config config = new SA_Config(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  bool loaded = load.LoadFromFile("$profile:BLCAAdminSuite/config.json") && load.ReadValue("", config);
  PrintFormat("SA_CSHARP_JSON_INTEROP=%1", loaded && config.schema == 1 && config.maxGameMasters == 3 && config.staff.Count() == 1 && config.staff[0].news && !config.staff[0].bypassLimit);
  Widget notice = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14992}UI/Layouts/SA/Notice.layout");
  PrintFormat("SA_BAN_NOTICE_LAYOUT=%1", notice && notice.FindAnyWidget("Close") && notice.FindAnyWidget("Line9"));
  if (notice) notice.RemoveFromHierarchy();
  Widget players = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14993}UI/Layouts/SA/Players.layout");
  PrintFormat("SA_PLAYERS_LAYOUT=%1", players && players.FindAnyWidget("Row7") && EditBoxWidget.Cast(players.FindAnyWidget("Reason")) && EditBoxWidget.Cast(players.FindAnyWidget("Minutes")));
  if (players) players.RemoveFromHierarchy();
  ValidatePlayersPanel();
  ValidateModeration();
  ValidateBanEnforcement();
  ValidatePausePanel();
  BN_BuildPlugin.ValidateResources();
  Print("SA_VALIDATION_FINISHED");
 }
 protected void ValidatePlayersPanel()
 {
  bool wasActive = SA_Client.active; bool wasPlayers = SA_Client.players; bool wasKick = SA_Client.kick; bool wasBan = SA_Client.ban;
  SA_Client.active = true; SA_Client.players = true; SA_Client.kick = true; SA_Client.ban = true;
  SA_Status data = new SA_Status();
  for (int i = 1; i <= 17; i++)
  {
   SA_Player item = new SA_Player(); item.playerId = i; item.uuid = "synthetic-test-player-" + i;
   item.name = "Player " + i; if (i == 2) item.name = "لاعب عربي";
   item.platform = "PC"; data.players.Insert(item);
  }
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", data); SA_PlayersUI.Show(save.ExportToString());
  Widget root = GetGame().GetWorkspace().FindAnyWidget("SA_Players");
  PrintFormat("SA_PANEL_ALL_CONTROLS_PRESENT=%1", SA_PlayersUI.HasAllControls(root));
  if (root)
  {
   root.Update(); Widget row = root.FindAnyWidget("Row1"); Widget label = root.FindAnyWidget("Label1");
   PrintFormat("SA_PANEL_BUTTON_SINGLE_CONTENT=%1", row.GetChildren() && !row.GetChildren().GetSibling() && label.GetParent().GetParent() == row);
   float bx, by, bw, bh, lx, ly, lw, lh; row.GetScreenPos(bx, by); row.GetScreenSize(bw, bh); label.GetScreenPos(lx, ly); label.GetScreenSize(lw, lh);
   PrintFormat("SA_PANEL_NAME_WITHIN_ROW=%1", bw > 0 && bh > 0 && lw > 0 && lh > 0 && lx >= bx && ly >= by && lx + lw <= bx + bw + 1 && ly + lh <= by + bh + 1);
   PrintFormat("SA_PANEL_ARABIC_NAME_DRAWN=%1", label.GetChildren() && label.GetChildren().GetParent() == label);
   PrintFormat("SA_PANEL_DURATION_INITIALIZED=%1", EditBoxWidget.Cast(root.FindAnyWidget("Minutes")).GetText() == "60");
   SA_PlayersUI.Click("Row1");
   PrintFormat("SA_PANEL_PLAYER_SELECTION=%1", TextWidget.Cast(root.FindAnyWidget("Selected")).GetText().Contains("synthetic-test-player-2") && root.FindAnyWidget("Ban").IsEnabled());
   SA_PlayersUI.Click("Ban");
   PrintFormat("SA_PANEL_EMPTY_REASON_BLOCKED=%1", TextWidget.Cast(root.FindAnyWidget("Status")).GetText().Contains("Enter a reason"));
   SA_PlayersUI.Click("Next");
   PrintFormat("SA_PANEL_PAGE_CHANGE_CLEARS_TARGET=%1", TextWidget.Cast(root.FindAnyWidget("Page")).GetText().Contains("PAGE 2") && !root.FindAnyWidget("Ban").IsEnabled());
   SA_Client.kick = false; SA_Client.onStateChanged.Invoke();
   SA_PlayersUI.Click("Row0");
   PrintFormat("SA_PANEL_LIVE_PERMISSION_REVOCATION=%1", !root.FindAnyWidget("Kick").IsEnabled() && root.FindAnyWidget("Ban").IsEnabled());
  }
  SA_PlayersUI.Hide();
  Widget before = GetGame().GetWorkspace().GetChildren(); SA_NoticeRenderer renderer = new SA_NoticeRenderer(); renderer.DrawSmall(null, "MUST NOT DRAW ABOVE THE PANEL");
  PrintFormat("SA_PANEL_NULL_PARENT_CANNOT_LEAK_TEXT=%1", before == GetGame().GetWorkspace().GetChildren());
  SA_Client.active = wasActive; SA_Client.players = wasPlayers; SA_Client.kick = wasKick; SA_Client.ban = wasBan;
 }
 protected void ValidateModeration()
 {
  SA_BanWait waiting = new SA_BanWait(); waiting.Begin(9, "test", 1000);
  PrintFormat("SA_BAN_FULL_TEN_SECONDS=%1", SA_BanWait.WAIT_MS == 10000 && SA_BanWait.DELIVERY_TIMEOUT_MS > SA_BanWait.WAIT_MS);
  PrintFormat("SA_BAN_ACK_WRONG_TOKEN_REJECTED=%1", !waiting.Accept(8, 1000) && !waiting.acknowledged);
  PrintFormat("SA_BAN_ACK_ONCE_ONLY=%1", waiting.Accept(9, 1000) && !waiting.Accept(9, 1000));
  PrintFormat("SA_BAN_DISCORD_TEXT=%1", SA_BanNoticeData.DISCORD_TEXT == "Please contact the server administration to appeal this ban.");
  Widget notice = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14992}UI/Layouts/SA/Notice.layout");
  PrintFormat("SA_BAN_REASON_EXPIRY_COUNTDOWN_SEPARATE=%1", notice && notice.FindAnyWidget("Intro0") && notice.FindAnyWidget("Line11") && notice.FindAnyWidget("Expiry1") && TextWidget.Cast(notice.FindAnyWidget("Countdown")));
  if (notice) notice.RemoveFromHierarchy();
  Resource actions = BaseContainerTools.LoadContainer("{64A9DC260EE14994}Configs/Editor/SA/ModerationActions.conf");
  SCR_EditorActionList list;
  if (actions && actions.IsValid()) list = SCR_EditorActionList.Cast(BaseContainerTools.CreateInstanceFromContainer(actions.GetResource().ToBaseContainer()));
  PrintFormat("SA_GM_KICK_BAN_ACTIONS_LOADED=%1", list && list.m_Actions.Count() == 2 && list.m_Actions[0].IsEnabled() && !list.m_Actions[0].IsServer() && list.m_Actions[1].IsEnabled() && !list.m_Actions[1].IsServer());
  PrintFormat("SA_GM_NATIVE_MODAL_PRESET=%1", SCR_ConfigurableDialogUi.IsPresetValid("{64A9DC260EE14995}Configs/Editor/SA/ModerationDialog.conf", "blca_moderation"));
  Widget form = GetGame().GetWorkspace().CreateWidgets("{64A9DC260EE14996}UI/Layouts/SA/Moderation.layout");
  PrintFormat("SA_GM_REASON_DURATION_FORM=%1", form && EditBoxWidget.Cast(form.FindAnyWidget("Reason")) && EditBoxWidget.Cast(form.FindAnyWidget("Minutes")) && form.FindAnyWidget("TargetUuid"));
  if (form) form.RemoveFromHierarchy();
  bool active = SA_Client.active; bool players = SA_Client.players; bool kick = SA_Client.kick; bool ban = SA_Client.ban;
  SA_Client.active = false; SA_Client.players = true; SA_Client.kick = true; SA_Client.ban = true;
  PrintFormat("SA_GM_MODERATION_REQUIRES_LOGIN=%1", !SA_ModerationRules.CanLocal("kick") && !SA_ModerationRules.CanLocal("ban"));
  SA_Client.active = true; SA_Client.ban = false;
  PrintFormat("SA_GM_KICK_BAN_SEPARATE_PERMISSIONS=%1", SA_ModerationRules.CanLocal("kick") && !SA_ModerationRules.CanLocal("ban"));
  SA_Client.active = active; SA_Client.players = players; SA_Client.kick = kick; SA_Client.ban = ban;
  int minutes;
  PrintFormat("SA_GM_BAN_DURATION_BOUNDARIES=%1", SA_ModerationRules.BanMinutes("0", minutes) && SA_ModerationRules.BanMinutes("43200", minutes) && !SA_ModerationRules.BanMinutes("43201", minutes) && !SA_ModerationRules.BanMinutes("abc", minutes));
  PrintFormat("SA_GM_REASON_VALIDATION=%1", SA_ModerationRules.ValidReason("سبب عربي") && !SA_ModerationRules.ValidReason(" \t "));
 }
 protected void ValidateBanEnforcement()
 {
  SA_BanEnforcementProbe probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.CheckBan(42);
  int token = probe.Pending().token;
  SA_BanNoticeData dismissed = new SA_BanNoticeData(); dismissed.token = token; dismissed.reason = "اختبار إغلاق الإشعار"; dismissed.expiry = "حظر دائم";
  SA_Notice.ShowBan(dismissed, true); SA_Notice.Hide();
  PrintFormat("SA_BAN_DISMISSING_NOTICE_CANNOT_CANCEL_SERVER_WAIT=%1", probe.Pending() && probe.Pending().token == token);
  probe.clock = 2000; probe.BanNoticeReceived(42, token);
  probe.clock = 10999; probe.CheckBan(42);
  PrintFormat("SA_BAN_SERVER_WAIT_BEFORE_KICK=%1", probe.kicks == 0 && probe.Pending().deadline == 11000);
  probe.clock = 11000; probe.CheckBan(42);
  PrintFormat("SA_BAN_SERVER_KICKS_WITHOUT_UI_CALLBACK=%1", probe.kicks == 1 && probe.notices == 1);
  probe.clock = 12000; probe.CheckBan(42);
  PrintFormat("SA_BAN_RETRY_RATE_LIMITED=%1", probe.kicks == 1);
  probe.clock = 13000; probe.CheckBan(42);
  PrintFormat("SA_BAN_RETRIES_WHILE_CONNECTED=%1", probe.kicks == 2);
  probe.Unban(); probe.clock = 16000; probe.CheckBan(42);
  PrintFormat("SA_BAN_UNBAN_STOPS_RETRIES=%1", probe.kicks == 2 && !probe.Pending() && probe.cancellations == 1);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.CheckBan(42);
  token = probe.Pending().token; probe.clock = 6000; probe.BanNoticeReceived(42, token);
  PrintFormat("SA_BAN_LATE_ACK_CANNOT_EXTEND_DEADLINE=%1", probe.Pending().deadline == 11000);
  probe.clock = 10000; probe.BanNoticeReceived(42, token); probe.CheckBan(42);
  PrintFormat("SA_BAN_DUPLICATE_ACK_CANNOT_RENEW_WAIT=%1", probe.kicks == 0 && probe.Pending().deadline == 11000);
  probe.clock = 11000; probe.CheckBan(42);
  PrintFormat("SA_BAN_HARD_DEADLINE_ENFORCED=%1", probe.kicks == 1);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.controllerReady = false; probe.CheckBan(42);
  probe.clock = 6000; probe.controllerReady = true; probe.CheckBan(42);
  PrintFormat("SA_BAN_LATE_CONTROLLER_DELIVERY_RETRIED=%1", probe.notices == 1 && probe.Pending().deadline == 16000);
  probe.clock = 16000; probe.CheckBan(42);
  PrintFormat("SA_BAN_LATE_CONTROLLER_KICK_ENFORCED=%1", probe.kicks == 1);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.controllerReady = false; probe.CheckBan(42);
  probe.clock = 20000; probe.controllerReady = true; probe.CheckBan(42); probe.BanNoticeReceived(42, probe.Pending().token);
  PrintFormat("SA_BAN_DELIVERY_CANNOT_EXTEND_HARD_LIMIT=%1", probe.Pending().deadline == 21000);
  probe.clock = 21000; probe.CheckBan(42);
  PrintFormat("SA_BAN_DELAYED_DELIVERY_HARD_LIMIT_KICKS=%1", probe.kicks == 1);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.controllerReady = false; probe.CheckBan(42);
  probe.clock = 21000; probe.CheckBan(42);
  PrintFormat("SA_BAN_NO_CONTROLLER_STILL_KICKS=%1", probe.notices == 0 && probe.kicks == 1);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.CheckBan(42);
  probe.BanNoticeReceived(42, probe.Pending().token + 1); probe.clock = 11000; probe.CheckBan(42);
  PrintFormat("SA_BAN_NO_VALID_ACK_STILL_KICKS=%1", probe.kicks == 1 && !probe.Pending().acknowledged);
  probe.BanNoticeReceived(42, probe.Pending().token);
  PrintFormat("SA_BAN_EXPIRED_DEADLINE_REJECTS_ACK=%1", !probe.Pending().acknowledged);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.CheckBan(42);
  token = probe.Pending().token; probe.Disconnect(); probe.clock = 5000; probe.CheckBan(42);
  probe.BanNoticeReceived(42, token);
  PrintFormat("SA_BAN_RECONNECT_REJECTS_OLD_TOKEN=%1", probe.Pending().token != token && !probe.Pending().acknowledged);
  probe.liveUuid = "12345678-1234-4321-8123-000000000099"; probe.clock = 25000; probe.CheckBan(42);
  PrintFormat("SA_BAN_REUSED_PLAYER_ID_NOT_KICKED=%1", probe.kicks == 0);

  probe = new SA_BanEnforcementProbe(null); probe.SeedBan(); probe.CheckBan(42); probe.Expire();
  probe.clock = 25000; probe.CheckBan(42);
  PrintFormat("SA_BAN_EXPIRED_BAN_NOT_KICKED=%1", probe.kicks == 0 && !probe.Pending());
 }
 protected void ValidatePausePanel()
 {
  Widget root = GetGame().GetWorkspace().CreateWidgets("UI/layouts/Menus/PauseMenu/PauseMenu.layout");
  if (!root) { Print("SA_PAUSE_NATIVE_LAYOUT=0"); return; }
  SCR_ButtonTextComponent admin = SCR_ButtonTextComponent.GetButtonText("FieldManual", root);
  SCR_ButtonTextComponent list = SCR_ButtonTextComponent.GetButtonText("Players", root);
  PrintFormat("SA_PAUSE_NATIVE_LAYOUT=%1", admin && list);
  SA_PausePanel panel = new SA_PausePanel(); panel.Bind(root);
  panel.Apply(true, false, true);
  PrintFormat("SA_PAUSE_ELIGIBLE_INACTIVE=%1", admin.IsVisible() && admin.IsEnabled() && !list.IsVisible());
  root.Update(); float x, y, afterX, afterY; root.FindAnyWidget("Logo").GetScreenPos(x, y);
  for (int i = 0; i < 100; i++) panel.Apply(true, false, true);
  root.Update(); root.FindAnyWidget("Logo").GetScreenPos(afterX, afterY);
  PrintFormat("SA_PAUSE_IDENTICAL_STATE_NO_REFLOW=%1", panel.GetApplyCount() == 1 && x == afterX && y == afterY);
  panel.Apply(true, true, true);
  PrintFormat("SA_PAUSE_LOGIN_UPDATES_ONCE=%1", panel.GetApplyCount() == 2 && admin.IsVisible() && list.IsVisible() && list.IsEnabled());
  for (int n = 0; n < 100; n++) panel.Apply(true, true, true);
  PrintFormat("SA_PAUSE_ACTIVE_STATE_NO_REPLAY=%1", panel.GetApplyCount() == 2);
  panel.Apply(true, true, false);
  PrintFormat("SA_PAUSE_PLAYER_PERMISSION_REVOKED=%1", admin.IsVisible() && !list.IsVisible());
  panel.Apply(false, false, false);
  PrintFormat("SA_PAUSE_STAFF_PERMISSION_REVOKED=%1", !admin.IsVisible() && !admin.IsEnabled() && !list.IsVisible());
  root.RemoveFromHierarchy();
 }
 override void RunCommandline() { Run(); Workbench.Exit(0); }
}
