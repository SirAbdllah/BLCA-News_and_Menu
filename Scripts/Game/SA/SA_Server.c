class SA_Server
{
 static ref SA_Server Instance;
 static const string ROOT = "$profile:BLCAAdminSuite/";
 protected SCR_BaseGameMode m_Mode;
 protected ref SA_Config m_Config;
 protected ref map<string, ref SA_Staff> m_Staff = new map<string, ref SA_Staff>();
 protected ref map<string, ref SA_Ban> m_Bans = new map<string, ref SA_Ban>();
 protected ref map<int, ref SA_Player> m_Players = new map<int, ref SA_Player>();
 protected ref map<int, string> m_Sessions = new map<int, string>();
 protected ref map<int, SCR_EditorModeEntity> m_OpenGM = new map<int, SCR_EditorModeEntity>();
 protected string m_RunId;
 protected int m_ConnectionSequence;
 protected ref map<int, int> m_Requests = new map<int, int>();
 protected ref map<int, string> m_StateSent = new map<int, string>();
 protected ref map<int, ref SA_BanWait> m_BanNotices = new map<int, ref SA_BanWait>();
 protected int m_BanToken;
 protected ref array<ref SA_Event> m_Audit = {};
 protected ref SA_Status m_Status = new SA_Status();
 protected ref SA_CommandBatch m_Commands;
 protected int m_CommandRevision;
 protected int m_Ticks;
 protected int m_Revision;
 protected bool m_Ready;
 protected bool m_ChangingRoles;

 void SA_Server(SCR_BaseGameMode mode) { m_Mode = mode; }
 void Start()
 {
  if (Instance) Instance.Stop();
  Instance = this;
  m_RunId = string.Format("%1-%2-%3", System.GetUnixTime(), System.GetTickCount(), Math.RandomInt(1, 1000000000));
  m_ConnectionSequence = 0;
  FileIO.MakeDirectory(ROOT);
  FileIO.MakeDirectory(ROOT + "audit");
  m_Status.lastCommand = ReadMarker("command_cursor.txt");
  Refresh(true);
  m_Mode.GetOnPlayerAuditSuccess().Insert(OnAuditSuccess);
  m_Mode.GetOnPlayerDisconnected().Insert(OnDisconnected);
  m_Mode.GetOnPlayerRoleChange().Insert(OnRoleChanged);
  GetGame().GetCallqueue().CallLater(Tick, 1000, true);
  LogRun("server.started");
 }
 void Stop()
 {
  if (Instance != this) return;
  if (GetGame()) GetGame().GetCallqueue().Remove(Tick);
  if (m_Mode)
  {
   m_Mode.GetOnPlayerAuditSuccess().Remove(OnAuditSuccess);
   m_Mode.GetOnPlayerDisconnected().Remove(OnDisconnected);
   m_Mode.GetOnPlayerRoleChange().Remove(OnRoleChanged);
  }
  array<int> active = {};
  foreach (int playerId, string uuid : m_Sessions) active.Insert(playerId);
  foreach (int playerId : active) EndAdminAudit(playerId, "server stopped");
  array<int> editors = {};
  foreach (int editorId, SCR_EditorModeEntity editor : m_OpenGM) editors.Insert(editorId);
  foreach (int editorId : editors) EndGMAudit(editorId, "server stopped");
  DrainAudit();
  LogRun("server.stop");
  DrainAudit();
  Instance = null;
 }
 protected int ReadMarker(string file)
 {
  FileHandle handle = FileIO.OpenFile(ROOT + file, FileMode.READ);
  if (!handle) return 0;
  string line; handle.ReadLine(line); handle.Close();
  return line.ToInt();
 }
 protected void WriteMarker(string file, int value)
 {
  FileHandle handle = FileIO.OpenFile(ROOT + file, FileMode.WRITE);
  if (handle) { handle.WriteLine(value.ToString()); handle.Close(); }
 }
 void Refresh(bool force = false)
 {
  int revision = ReadMarker("config.rev");
  if (!force && revision == m_Revision) return;
  SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  SA_Config next = new SA_Config();
  bool valid = load.LoadFromFile(ROOT + "config.json") && load.ReadValue("", next);
  if (!valid || next.schema != 1 || next.revision != revision || next.staff.Count() > 5000 || next.bans.Count() > 20000)
  {
   m_Ready = false;
   m_Status.error = "Configuration unavailable or invalid. Open the Windows manager to initialize it.";
   m_Revision = revision;
   return;
  }
  map<string, ref SA_Staff> staff = new map<string, ref SA_Staff>();
  foreach (SA_Staff grant : next.staff)
  {
   if (!grant || grant.uuid.Length() != 36 || staff.Contains(grant.uuid)) { m_Ready = false; m_Revision = revision; m_Status.error = "Invalid or duplicate staff UUID"; return; }
   staff.Set(grant.uuid, grant);
  }
  m_Config = next;
  m_Config.maxGameMasters = Math.ClampInt(m_Config.maxGameMasters, 1, 3);
  m_Staff = staff;
  m_Bans.Clear();
  foreach (SA_Ban entry : m_Config.bans) if (entry && entry.Active(System.GetUnixTime())) m_Bans.Set(entry.uuid, entry);
  m_Revision = revision; m_Ready = true; m_Status.error = "";
  Log("permissions.reloaded", "SERVER", "", "revision=" + revision);
  array<int> active = {}; foreach (int sessionId, string sessionUuid : m_Sessions) active.Insert(sessionId);
  int occupied;
  foreach (int id : active)
  {
   SA_Staff current = Grant(id);
   if (!current || !current.enabled || !Active(id)) Logout(id, "Permission removed or access denied");
   else if (current.gm && !current.bypassLimit && occupied >= m_Config.maxGameMasters) Logout(id, "Game Master slot limit updated");
   else { if (current.gm && !current.bypassLimit) occupied++; SetGM(id, current.gm); SendState(id, "Permissions updated"); }
  }
 }
 string PlayerUuid(int playerId)
 {
  SA_Player player;
  if (m_Players.Find(playerId, player)) return player.uuid;
  BackendApi api = GetGame().GetBackendApi();
  if (!api) return "";
  return api.GetPlayerIdentityId(playerId);
 }
 SA_Staff Grant(int playerId)
 {
  if (!m_Ready) return null;
  SA_Staff grant;
  m_Staff.Find(PlayerUuid(playerId), grant);
  if (!grant || !grant.enabled) return null;
  return grant;
 }
 bool Active(int playerId)
 {
  SA_Ban entry;
  if (m_Bans.Find(PlayerUuid(playerId), entry) && entry.Active(System.GetUnixTime())) return false;
  return m_Ready && m_Sessions.Contains(playerId) && Grant(playerId);
 }
 bool Can(int playerId, string permission)
 {
  Refresh();
  if (!Active(playerId)) return false;
  SA_Staff grant = Grant(playerId);
  switch (permission)
  {
   case "gm": return grant.gm;
   case "news": return grant.news;
   case "players": return grant.players;
   case "kick": return grant.kick;
   case "ban": return grant.ban;
  }
  return false;
 }
 bool RateAllowed(int playerId)
 {
  int previous; bool known = m_Requests.Find(playerId, previous);
  int now = System.GetTickCount();
  if (known && now - previous >= 0 && now - previous < 500) { m_Status.deniedRequests++; return false; }
  m_Requests.Set(playerId, now);
  return true;
 }
 int ActiveGMCount(bool regularOnly = false)
 {
  int count;
  foreach (int id, string uuid : m_Sessions)
  { SA_Staff grant = Grant(id); if (grant && grant.gm && (!regularOnly || !grant.bypassLimit)) count++; }
  return count;
 }
 void Login(int playerId)
 {
  Refresh();
  SA_Staff grant = Grant(playerId);
  if (!grant) { SendState(playerId, "You are not assigned to server staff."); return; }
  if (m_Sessions.Contains(playerId)) { SendState(playerId, "Already logged in"); return; }
  SA_Ban banned;
  if (m_Bans.Find(PlayerUuid(playerId), banned) && banned.Active(System.GetUnixTime())) { CheckBan(playerId); return; }
  if (grant.gm && !grant.bypassLimit && ActiveGMCount(true) >= m_Config.maxGameMasters)
  { SendState(playerId, "All Game Master slots are occupied."); return; }
  m_Sessions.Set(playerId, grant.uuid);
  if (grant.gm && !SetGM(playerId, true))
  { m_Sessions.Remove(playerId); SendState(playerId, "Game Master system is not ready. Try again shortly."); return; }
  SendState(playerId, "Admin session activated");
  LogConnection("admin.login", playerId, grant.uuid, "", grant.rank, AuditStaffRank(grant));
 }
 void Logout(int playerId, string reason)
 {
  EndAdminAudit(playerId, reason);
  SetGM(playerId, false);
  SendState(playerId, reason);
 }
 bool SetGM(int playerId, bool enabled)
 {
  SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
  if (!core) return !enabled;
  SCR_EditorManagerEntity editor = core.GetEditorManager(playerId);
  if (!editor) return !enabled;
  if (enabled) editor.AddEditorModes(EEditorModeAccess.BASE, EEditorMode.EDIT, false);
  else
  {
   EndGMAudit(playerId, "Game Master role removed");
   editor.RemoveEditorModes(EEditorModeAccess.BASE, EEditorMode.EDIT);
   editor.RemoveEditorModes(EEditorModeAccess.ROLE, EEditorMode.EDIT);
  }
  PlayerManager pm = GetGame().GetPlayerManager();
  if (pm)
  {
   m_ChangingRoles = true;
   if (enabled) pm.GivePlayerRole(playerId, EPlayerRole.GAME_MASTER);
   else pm.ClearPlayerRole(playerId, EPlayerRole.GAME_MASTER);
   m_ChangingRoles = false;
  }
  return true;
 }
 void SendState(int playerId, string message)
 {
  SA_Staff grant = Grant(playerId);
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
  if (!pc) return;
  string signature = m_Revision.ToString() + ":" + Active(playerId).ToString() + ":" + (grant != null).ToString();
  string previous; m_StateSent.Find(playerId, previous);
  if (signature == previous && message.IsEmpty()) return;
  m_StateSent.Set(playerId, signature);
  pc.SA_SendState(grant != null, Active(playerId), grant, message);
 }
 void OnAuditSuccess(int playerId)
 {
  PlayerManager pm = GetGame().GetPlayerManager();
  string uuid;
  if (GetGame().GetBackendApi()) uuid = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
  if (uuid.IsEmpty()) return;
  SA_Player previous;
  m_Players.Find(playerId, previous);
  SA_Player record = new SA_Player();
  record.uuid = uuid; record.playerId = playerId; record.name = pm.GetPlayerName(playerId);
  record.joined = System.GetUnixTime();
  record.platform = typename.EnumToString(PlatformKind, pm.GetPlatformKind(playerId));
  PlayerController pc = pm.GetPlayerController(playerId);
  if (pc) record.rplIdentity = pc.GetRplIdentity().ToString();
  bool sameConnection = previous && previous.uuid == uuid && previous.rplIdentity == record.rplIdentity;
  if (previous)
  {
   string reason = "connection replaced";
   if (sameConnection) reason = "connection re-audited";
   EndAdminAudit(playerId, reason);
   EndGMAudit(playerId, reason);
   if (!sameConnection) LogConnection("player.leave", playerId, "SERVER", previous.uuid, reason);
  }
  m_Players.Set(playerId, record);
  if (sameConnection)
  {
   record.joined = previous.joined;
   record.sessionId = previous.sessionId;
   record.staffRank = previous.staffRank;
  }
  else
  {
   m_ConnectionSequence++;
   record.sessionId = m_RunId + ":" + m_ConnectionSequence;
   record.staffRank = AuditStaffRank(Grant(playerId));
  }
  m_Sessions.Remove(playerId);
  SetGM(playerId, false);
  if (!sameConnection) LogConnection("player.join", playerId, "", uuid, "");
  CheckBan(playerId);
  SendState(playerId, "Open Admin Panel to activate your staff session.");
 }
 void OnDisconnected(int playerId, KickCauseCode cause, int timeout)
 {
  EndAdminAudit(playerId, "disconnected");
  EndGMAudit(playerId, "disconnected");
  LogConnection("player.leave", playerId, "SERVER", PlayerUuid(playerId), cause.ToString());
  m_Players.Remove(playerId); m_Requests.Remove(playerId); m_StateSent.Remove(playerId); m_BanNotices.Remove(playerId);
 }
 void OnRoleChanged(int playerId, EPlayerRole role)
 {
  if (m_ChangingRoles) return;
  if ((role & EPlayerRole.GAME_MASTER) != 0 && !Can(playerId, "gm")) SetGM(playerId, false);
 }
 bool IsIdentityBanned(string identity)
 {
  SA_Ban entry;
  return m_Bans.Find(identity, entry) && entry.Active(System.GetUnixTime());
 }
 void CheckBan(int playerId)
 {
  SA_Ban entry;
  SA_BanWait waiting;
  m_BanNotices.Find(playerId, waiting);
  if (!m_Bans.Find(PlayerUuid(playerId), entry) || !entry.Active(System.GetUnixTime()))
  {
   if (waiting)
   {
    m_BanNotices.Remove(playerId);
    CancelBanNotice(playerId, waiting.token);
   }
   return;
  }
  // Verify the live connection before showing a notice or kicking a reused player ID.
  if (!BanConnectionMatches(playerId, entry.uuid)) return;
  int now = BanClock();
  if (!waiting || waiting.uuid != entry.uuid)
  {
   waiting = new SA_BanWait(); waiting.Begin(++m_BanToken, entry.uuid, now);
   m_BanNotices.Set(playerId, waiting);
   EndAdminAudit(playerId, "ban enforced"); SetGM(playerId, false);
   Log("ban.blocked", entry.actor, entry.uuid, entry.reason + " | " + entry.expiresLabel);
  }
  // The server owns this deadline. Closing the UI or withholding its RPC cannot cancel it.
  if (waiting.ShouldKick(now))
  {
   waiting.MarkKick(now);
   Log("ban.kick", entry.actor, entry.uuid, "attempt=" + waiting.kickAttempts);
   KickBannedPlayer(playerId);
   return;
  }
  if (!waiting.Due(now) && !waiting.noticeSent && DeliverBanNotice(playerId, waiting, entry)) waiting.NoticeSent(now);
 }
 protected int BanClock() { return System.GetTickCount(); }
 protected void CancelBanNotice(int playerId, int token)
 {
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
  if (pc) pc.SA_SendBanCancel(token);
 }
 protected bool BanConnectionMatches(int playerId, string uuid)
 {
  PlayerManager pm = GetGame().GetPlayerManager();
  BackendApi api = GetGame().GetBackendApi();
  return pm && pm.IsPlayerConnected(playerId) && api && api.GetPlayerIdentityId(playerId) == uuid;
 }
 protected bool DeliverBanNotice(int playerId, SA_BanWait waiting, SA_Ban entry)
 {
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
  if (!pc) return false;
  SA_BanNoticeData notice = new SA_BanNoticeData(); notice.token = waiting.token; notice.reason = entry.reason; notice.expiry = "حظر دائم";
  if (entry.expires > 0) notice.expiry = entry.expiresLabel;
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", notice);
  pc.SA_SendBanNotice(save.ExportToString());
  return true;
 }
 void BanNoticeReceived(int playerId, int token)
 {
  SA_BanWait waiting;
  if (!m_BanNotices.Find(playerId, waiting) || waiting.uuid != PlayerUuid(playerId)) return;
  waiting.Accept(token, BanClock());
 }
 protected void KickBannedPlayer(int playerId)
 {
  GetGame().GetPlayerManager().KickPlayer(playerId, PlayerManagerKickReason.KICK, 0);
 }
 void Log(string action, string actor, string target, string detail)
 {
  SA_Event auditEvent = new SA_Event(); auditEvent.timestamp = System.GetUnixTime(); auditEvent.action = action; auditEvent.actor = actor; auditEvent.target = target; auditEvent.detail = detail;
  Enqueue(auditEvent);
 }
 protected string AuditStaffRank(SA_Staff grant)
 {
  if (!grant || !grant.enabled) return "";
  if (grant.rank.IsEmpty()) return "Staff";
  return grant.rank;
 }
 protected void LogRun(string action)
 {
  SA_Event auditEvent = new SA_Event();
  auditEvent.timestamp = System.GetUnixTime();
  auditEvent.action = action;
  auditEvent.actor = "SERVER";
  auditEvent.sessionId = m_RunId;
  Enqueue(auditEvent);
 }
 protected void LogConnection(string action, int playerId, string actor, string target, string detail, string staffRank = "")
 {
  SA_Event auditEvent = new SA_Event();
  auditEvent.timestamp = System.GetUnixTime();
  auditEvent.action = action;
  auditEvent.actor = actor;
  auditEvent.target = target;
  auditEvent.detail = detail;
  auditEvent.staffRank = staffRank;
  SA_Player player;
  if (m_Players.Find(playerId, player))
  {
   auditEvent.sessionId = player.sessionId;
   if (auditEvent.staffRank.IsEmpty()) auditEvent.staffRank = player.staffRank;
   auditEvent.name = player.name;
   auditEvent.platform = player.platform;
   auditEvent.rplIdentity = player.rplIdentity;
  }
  Enqueue(auditEvent);
 }
 protected void EndAdminAudit(int playerId, string reason)
 {
  string uuid;
  if (!m_Sessions.Find(playerId, uuid)) return;
  m_Sessions.Remove(playerId);
  LogConnection("admin.logout", playerId, uuid, "", reason);
 }
 void EditorModeAudit(SCR_EditorModeEntity mode, bool opened)
 {
  if (!Replication.IsServer() || !mode || mode.GetModeType() != EEditorMode.EDIT) return;
  SCR_EditorManagerEntity editor = mode.GetManager();
  if (!editor) return;
  int playerId = editor.GetPlayerID();
  if (playerId <= 0) return;
  SCR_EditorModeEntity current;
  m_OpenGM.Find(playerId, current);
  if (!opened)
  {
   if (current == mode) EndGMAudit(playerId, "editor deactivated");
   return;
  }
  if (current == mode || !m_Players.Contains(playerId) || !Active(playerId)) return;
  SA_Staff grant = Grant(playerId);
  if (!grant || !grant.gm) return;
  if (current) EndGMAudit(playerId, "editor changed");
  m_OpenGM.Set(playerId, mode);
  LogConnection("gm.open", playerId, grant.uuid, "", "editor activated", AuditStaffRank(grant));
 }
 protected void EndGMAudit(int playerId, string reason)
 {
  if (!m_OpenGM.Contains(playerId)) return;
  m_OpenGM.Remove(playerId);
  LogConnection("gm.close", playerId, PlayerUuid(playerId), "", reason);
 }
 protected void Enqueue(SA_Event auditEvent)
 {
  if (m_Audit.Count() >= 2048) { m_Status.droppedAuditEvents++; return; }
  m_Audit.Insert(auditEvent);
 }
 protected void Flush()
 {
  if (m_Audit.IsEmpty()) return;
  int day = System.GetUnixTime() / 86400;
  FileHandle file = FileIO.OpenFile(ROOT + "audit/audit-" + day + ".ndjson", FileMode.APPEND);
  if (!file) return;
  int batch = Math.Min(m_Audit.Count(), 256);
  for (int i = 0; i < batch; i++)
  {
   SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", m_Audit[0]);
   string json = save.ExportToString(); json.Replace("\r", ""); json.Replace("\n", "");
   file.WriteLine(json); m_Audit.RemoveOrdered(0);
  }
  file.Close();
 }
 protected void DrainAudit()
 {
  while (!m_Audit.IsEmpty())
  {
   int pending = m_Audit.Count();
   Flush();
   if (m_Audit.Count() == pending) return;
  }
 }
 protected void WriteStatus()
 {
  m_Status.updated = System.GetUnixTime(); m_Status.configRevision = m_Revision;
  m_Status.ready = m_Ready; m_Status.activeGameMasters = ActiveGMCount();
  m_Status.occupiedSlots = ActiveGMCount(true);
  if (m_Config) m_Status.maxGameMasters = m_Config.maxGameMasters;
  m_Status.players.Clear();
  foreach (int id, SA_Player player : m_Players)
  {
   player.active = Active(id); SA_Staff grant = Grant(id);
   player.rank = ""; if (grant) player.rank = grant.rank;
   player.faction = BN_NewsAudience.PlayerFaction(GetGame().GetPlayerManager().GetPlayerController(id));
   m_Status.players.Insert(player);
  }
  SA_WeatherBridge.ReadStatus(m_Status);
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", m_Status); save.SaveToFile(ROOT + "status.json");
 }
 protected void RunCommands()
 {
  int revision = ReadMarker("commands.rev");
  if (revision != m_CommandRevision)
  {
   SA_CommandBatch batch = new SA_CommandBatch(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
   if (load.LoadFromFile(ROOT + "commands.json") && load.ReadValue("", batch) && batch.commands.Count() <= 32)
   { m_Commands = batch; m_CommandRevision = revision; }
  }
  if (!m_Commands) return;
  int performed;
  foreach (SA_Command command : m_Commands.commands)
  {
   if (!command || command.id <= m_Status.lastCommand || performed >= 4) continue;
   m_Status.lastCommand = command.id; WriteMarker("command_cursor.txt", command.id); // at-most-once acceptance across restart
   SA_Result result = new SA_Result(); result.id = command.id; result.timestamp = System.GetUnixTime();
   if (command.expires <= result.timestamp || command.created > result.timestamp + 30) result.message = "Command expired or clock mismatch";
   else if (command.kind == "news")
   {
    if (command.duration >= 15 && command.duration <= 180 && command.text.Length() <= 2400 && !command.text.IsEmpty() && (command.audience == "all" || command.audience == "us" || command.audience == "ussr"))
    { result.ok = m_Mode.BN_Publish("Server Admin says: " + command.text, "SERVER", command.duration, command.audience); result.message = "Server news processed"; }
    else result.message = "Invalid news message or duration";
   }
   else if (command.kind == "weather")
   {
    result.ok = SA_WeatherBridge.Apply(command.text, result.message);
   }
   else if (command.kind == "logout" || command.kind == "kick")
   {
    int targetId;
    foreach (int id, SA_Player player : m_Players)
    {
     if (player.uuid != command.target || !GetGame().GetPlayerManager().IsPlayerConnected(id)) continue;
     if (command.kind == "logout" && !m_Sessions.Contains(id)) continue;
     targetId = id;
     break;
    }
    if (targetId > 0)
    {
     if (command.kind == "logout") Logout(targetId, "Session closed by Windows manager");
     else GetGame().GetPlayerManager().KickPlayer(targetId, PlayerManagerKickReason.KICK, 0);
     result.ok = true;
    }
    if (result.ok) result.message = "Player action processed";
    else result.message = "Player is no longer connected";
   }
   else result.message = "Unknown command";
   Log("manager." + command.kind, command.actor, command.target, command.text + " | " + result.message);
   if (result.ok)
   {
    string actor = command.actor;
    if (!actor.StartsWith("WINDOWS:")) actor = "WINDOWS:" + actor;
    Log("manager." + command.kind + ".applied", actor, command.target, command.text + " | " + result.message);
   }
   else if (command.kind == "weather")
   {
    string actor = command.actor;
    if (!actor.StartsWith("WINDOWS:")) actor = "WINDOWS:" + actor;
    Log("manager.weather.rejected", actor, "", result.message);
   }
   m_Status.results.Insert(result); if (m_Status.results.Count() > 64) m_Status.results.RemoveOrdered(0);
   performed++;
  }
 }
 void Tick()
 {
  if (!GetGame() || GetGame().GetGameMode() != m_Mode) { Stop(); return; }
  int started = System.GetTickCount(); m_Ticks++;
  Refresh(); RunCommands();
  // Check every connected player, including a late/missed audit callback. A snapshot
  // is safe if a native kick immediately triggers OnDisconnected and changes the maps.
  array<int> connected = {};
  GetGame().GetPlayerManager().GetPlayers(connected);
  foreach (int connectedId : connected) CheckBan(connectedId);
  array<int> active = {}; foreach (int sessionId, string sessionUuid : m_Sessions) active.Insert(sessionId);
  foreach (int id : active) if (!Grant(id)) Logout(id, "Permission revoked");
  // Low-frequency reconciliation covers role changes and late player-controller creation.
  if (Math.Mod(m_Ticks, 2) == 0)
  {
   foreach (int id, SA_Player player : m_Players)
   {
    SA_Staff cachedGrant = Grant(id);
    if ((!Active(id) || !cachedGrant || !cachedGrant.gm) && (GetGame().GetPlayerManager().GetPlayerRoles(id) & EPlayerRole.GAME_MASTER) != 0) SetGM(id, false);
    SendState(id, "");
   }
   WriteStatus();
  }
  if (Math.Mod(m_Ticks, 5) == 0) Flush();
  m_Status.lastTickMs = System.GetTickCount() - started;
  m_Status.maxTickMs = Math.Max(m_Status.maxTickMs, m_Status.lastTickMs);
 }
 void PlayerList(int actorId)
 {
  if (!Can(actorId, "players")) { SendState(actorId, "Player list permission denied"); return; }
  SA_Status snapshot = new SA_Status();
  foreach (int id, SA_Player player : m_Players) snapshot.players.Insert(player);
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", snapshot);
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(actorId));
  if (pc) pc.SA_SendPlayers(save.ExportToString());
 }
 void ModerationTarget(int actorId, int targetId, string action)
 {
  if (!RateAllowed(actorId) || !Can(actorId, "gm") || !Can(actorId, "players") || !Can(actorId, action)) { SendState(actorId, "Game Master moderation permission denied"); return; }
  if (action != "kick" && action != "ban") return;
  SA_Player target;
  if (!m_Players.Find(targetId, target)) { SendState(actorId, "Player is no longer connected"); return; }
  SA_ModerationTarget payload = new SA_ModerationTarget(); payload.action = action; payload.player = target;
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", payload);
  SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(actorId));
  if (pc) pc.SA_SendModerationTarget(save.ExportToString());
 }
 void PlayerAction(int actorId, string action, string target, string reason, int minutes)
 {
  if (!RateAllowed(actorId) || !Can(actorId, "players") || !Can(actorId, action)) { SendState(actorId, "Player action permission denied"); return; }
  if (!SA_ModerationRules.ValidReason(reason) || target.Length() != 36) { SendState(actorId, "Enter a reason of 1-500 characters and select a player"); return; }
  SA_Player selected;
  foreach (int id, SA_Player player : m_Players) if (player.uuid == target) { selected = player; break; }
  if (!selected) { SendState(actorId, "Player is no longer connected"); return; }
  if (action == "kick")
  {
   Log("admin.kick", PlayerUuid(actorId), target, reason);
   GetGame().GetPlayerManager().KickPlayer(selected.playerId, PlayerManagerKickReason.KICK, 0);
   SendState(actorId, "Player kicked"); return;
  }
  if (action != "ban" || minutes < 0 || minutes > 43200) return;
  if (ReadMarker("manager-heartbeat.txt") < System.GetUnixTime() - 12) { SendState(actorId, "Start the Windows manager or background agent to persist this ban"); return; }
  SA_BanRequests batch = new SA_BanRequests(); SCR_JsonLoadContext load = new SCR_JsonLoadContext();
  load.LoadFromFile(ROOT + "ban-requests.json"); load.ReadValue("", batch);
  int cursor = ReadMarker("ban-cursor.txt");
  for (int i = batch.requests.Count() - 1; i >= 0; i--) if (batch.requests[i].id <= cursor || batch.requests[i].deadline <= System.GetUnixTime()) batch.requests.RemoveOrdered(i);
  if (batch.requests.Count() >= 32) { SendState(actorId, "Ban request queue is full"); return; }
  SA_BanRequest request = new SA_BanRequest(); request.id = ReadMarker("ban-sequence.txt") + 1;
  request.created = System.GetUnixTime(); request.deadline = request.created + 120; request.actor = PlayerUuid(actorId);
  request.uuid = target; request.name = selected.name; request.reason = reason;
  if (minutes > 0) request.expires = request.created + minutes * 60;
  WriteMarker("ban-sequence.txt", request.id); batch.requests.Insert(request);
  SCR_JsonSaveContext save = new SCR_JsonSaveContext(); save.WriteValue("", batch); save.SaveToFile(ROOT + "ban-requests.json");
  Log("admin.ban.request", request.actor, target, "request=" + request.id + " " + reason);
  SendState(actorId, "Ban request submitted to the Windows manager; check the audit result");
 }
}

modded class SCR_EditorModeEntity
{
 override void ActivateModeServer()
 {
  super.ActivateModeServer();
  if (SA_Server.Instance) SA_Server.Instance.EditorModeAudit(this, true);
 }
 override void DeactivateModeServer()
 {
  if (SA_Server.Instance) SA_Server.Instance.EditorModeAudit(this, false);
  super.DeactivateModeServer();
 }
}

modded class ArmaReforgerScripted
{
 override void OnGameEnd()
 {
  if (SA_Server.Instance) SA_Server.Instance.Stop();
  super.OnGameEnd();
 }
}

modded class SCR_BaseGameMode
{
 protected ref SA_Server m_SA_Server;
 override void EOnInit(IEntity owner)
 {
  super.EOnInit(owner);
  if (GetGame().InPlayMode() && Replication.IsServer()) { m_SA_Server = new SA_Server(this); m_SA_Server.Start(); }
 }
}
modded class SCR_EditorManagerEntity
{
 override void AddEditorModes(EEditorModeAccess access, EEditorMode modes, bool isInit = false)
 {
  if (Replication.IsServer() && (modes & EEditorMode.EDIT) != 0)
  {
   if (!SA_Server.Instance || !SA_Server.Instance.Can(GetPlayerID(), "gm")) modes = modes & ~EEditorMode.EDIT;
  }
  super.AddEditorModes(access, modes, isInit);
 }
 override void SetEditorModes(EEditorModeAccess access, EEditorMode modes, bool isInit = false)
 {
  if (Replication.IsServer() && (modes & EEditorMode.EDIT) != 0)
  {
   if (!SA_Server.Instance || !SA_Server.Instance.Can(GetPlayerID(), "gm")) modes = modes & ~EEditorMode.EDIT;
  }
  super.SetEditorModes(access, modes, isInit);
 }
}
