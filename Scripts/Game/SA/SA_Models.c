// Private server configuration lives under $profile:BLCAAdminSuite, never in the addon.
class SA_Staff
{
 string uuid;
 string name;
 string rank = "Moderator";
 bool enabled = true;
 bool gm = true;
 bool news;
 bool bypassLimit;
 bool players = true;
 bool kick;
 bool ban;
}
class SA_Ban
{
 string uuid;
 string name;
 string reason;
 string actor;
 int created;
 int expires; // UTC epoch; zero means permanent.
 string expiresLabel;
 bool enabled = true;
 bool Active(int now) { return enabled && (expires == 0 || expires > now); }
}
class SA_BanNoticeData
{
 static const string DISCORD_TEXT = "Please contact the server administration to appeal this ban.";
 int token;
 string reason;
 string expiry;
}
class SA_BanWait
{
 static const int WAIT_MS = 10000;
 static const int DELIVERY_TIMEOUT_MS = 20000;
 static const int RETRY_MS = 2000;
 int token;
 string uuid;
 bool acknowledged;
 int deadline;
 int kickAttempts;
 int lastKick;
 bool noticeSent;
 void Begin(int nextToken, string identity, int now)
 {
  token = nextToken; uuid = identity; deadline = now + DELIVERY_TIMEOUT_MS;
 }
 bool Accept(int receivedToken, int now)
 {
  if (receivedToken != token || acknowledged || Due(now)) return false;
  // Client acknowledgement is telemetry only; it cannot control the deadline.
  acknowledged = true; return true;
 }
 void NoticeSent(int now)
 {
  noticeSent = true;
  if (deadline - now > WAIT_MS) deadline = now + WAIT_MS;
 }
 bool Due(int now) { return now - deadline >= 0; }
 bool ShouldKick(int now) { return Due(now) && (kickAttempts == 0 || now - lastKick >= RETRY_MS); }
 void MarkKick(int now) { lastKick = now; kickAttempts++; }
}
class SA_ModerationTarget
{
 string action;
 ref SA_Player player;
}
class SA_Config
{
 int schema = 1;
 int revision = 1;
 int maxGameMasters = 3;
 ref array<ref SA_Staff> staff = {};
 ref array<ref SA_Ban> bans = {};
}
class SA_Player
{
 int playerId;
 string uuid;
 string name;
 string platform;
 string rplIdentity;
 string faction;
 int joined;
 bool active;
 string rank;
 string sessionId;
 string staffRank;
}
class SA_Event
{
 int timestamp;
 string action;
 string actor;
 string target;
 string detail;
 string name;
 string platform;
 string rplIdentity;
 string sessionId;
 string staffRank;
}
class SA_Command
{
 int id;
 int created;
 int expires;
 string kind;
 string actor;
 string target;
 string text;
 string audience = "all";
 int duration = 20;
}
class SA_CommandBatch
{
 int revision;
 ref array<ref SA_Command> commands = {};
}
class SA_Result
{
 int id;
 int timestamp;
 bool ok;
 string message;
}
class SA_Status
{
 int schema = 1;
 int updated;
 int configRevision;
 int lastCommand;
 int activeGameMasters;
 int occupiedSlots;
 int maxGameMasters;
 int deniedRequests;
 int droppedAuditEvents;
 float lastTickMs;
 float maxTickMs;
 bool ready;
 string error;
 string weatherCurrent;
 bool weatherAutomatic;
 ref array<string> weatherPresets = {};
 ref array<ref SA_Player> players = {};
 ref array<ref SA_Result> results = {};
}
class SA_BanRequest
{
 int id;
 int created;
 int deadline;
 string actor;
 string uuid;
 string name;
 string reason;
 int expires;
}
class SA_BanRequests
{
 ref array<ref SA_BanRequest> requests = {};
}
class SA_Client
{
 static ref ScriptInvoker onStateChanged = new ScriptInvoker();
 static bool eligible;
 static bool active;
 static bool news;
 static bool players;
 static bool kick;
 static bool ban;
 static string rank;
 static string message;
 static string banNotice;
 static int banToken;
}
