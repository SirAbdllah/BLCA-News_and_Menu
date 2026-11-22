#ifdef WORKBENCH
// Workbench-only probe. Exercises the real session methods without granting native player roles.
class SA_PolicyProbe : SA_Server
{
 ref map<int, bool> gmState = new map<int, bool>();
 override void Refresh(bool force = false) {}
 override void SendState(int playerId, string message) {}
 override bool SetGM(int playerId, bool enabled) { gmState.Set(playerId, enabled); return true; }
 override string PlayerUuid(int playerId) { return "12345678-1234-4321-8123-00000000000" + playerId; }
 void Seed()
 {
  m_Config = new SA_Config(); m_Config.maxGameMasters = 3; m_Ready = true;
  for (int id = 1; id <= 6; id++)
  {
   SA_Staff staff = new SA_Staff(); staff.uuid = PlayerUuid(id); staff.news = id == 2 || id == 6;
   staff.bypassLimit = id == 5; staff.gm = id != 6; m_Staff.Set(staff.uuid, staff);
  }
 }
 void Revoke(int id) { m_Staff.Get(PlayerUuid(id)).enabled = false; }
 void RemoveNews(int id) { m_Staff.Get(PlayerUuid(id)).news = false; }
}
// Exercises the production ban reconciliation without disconnecting native players.
class SA_BanEnforcementProbe : SA_Server
{
 int clock = 1000;
 int kicks;
 int notices;
 int cancellations;
 bool controllerReady = true;
 string liveUuid = "12345678-1234-4321-8123-000000000042";
 protected string m_CachedUuid = "12345678-1234-4321-8123-000000000042";
 override void Refresh(bool force = false) {}
 override string PlayerUuid(int playerId) { return m_CachedUuid; }
 override bool SetGM(int playerId, bool enabled) { return true; }
 override protected int BanClock() { return clock; }
 override protected bool BanConnectionMatches(int playerId, string uuid) { return liveUuid == uuid; }
 override protected bool DeliverBanNotice(int playerId, SA_BanWait waiting, SA_Ban entry)
 {
  if (!controllerReady) return false;
  notices++; return true;
 }
 override protected void CancelBanNotice(int playerId, int token) { cancellations++; }
 override protected void KickBannedPlayer(int playerId) { kicks++; }
 void SeedBan()
 {
  m_Ready = true; m_Config = new SA_Config();
  SA_Ban entry = new SA_Ban(); entry.uuid = m_CachedUuid; entry.reason = "اختبار الحظر";
  m_Bans.Set(entry.uuid, entry);
 }
 SA_BanWait Pending() { return m_BanNotices.Get(42); }
 void Unban() { m_Bans.Remove(m_CachedUuid); }
 void Expire() { m_Bans.Get(m_CachedUuid).expires = System.GetUnixTime(); }
 void Disconnect() { m_BanNotices.Remove(42); }
}
#endif
