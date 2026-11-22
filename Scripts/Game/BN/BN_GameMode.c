// Automatic integration: no scenario or prefab edits required for SCR_BaseGameMode scenarios.
modded class SCR_BaseGameMode
{
 protected int m_BN_NextPublishTick;
 [RplProp(onRplName: "BN_OnNewsAudioChanged")] protected bool m_BN_NewsSound = true;
 bool BN_GetNewsSound() { return m_BN_NewsSound; }
 void BN_SetNewsSound(bool enabled)
 {
  if (!Replication.IsServer()) return;
  m_BN_NewsSound = enabled;
  BN_OnNewsAudioChanged();
  Replication.BumpMe();
 }
 protected void BN_OnNewsAudioChanged() { if (!m_BN_NewsSound) BN_NewsAudio.Stop(); }

 bool BN_Publish(string text, string faction, int duration, string audience = "all")
 {
  if (!Replication.IsServer()) return false;
  if (duration > 0 && System.GetTickCount() < m_BN_NextPublishTick) return false;
  if (duration > 0) m_BN_NextPublishTick = System.GetTickCount() + 3000;
  PlayerManager players = GetGame().GetPlayerManager();
  if (!players) return false;
  array<int> ids = {}; players.GetPlayers(ids);
  int count;
  foreach (int id : ids)
  {
   SCR_PlayerController pc = SCR_PlayerController.Cast(players.GetPlayerController(id));
   if (!pc || !BN_NewsAudience.Matches(audience, BN_NewsAudience.PlayerFaction(pc))) continue;
   pc.BN_DeliverNews(text, faction, duration, m_BN_NewsSound);
   count++;
  }
  PrintFormat("[BLCA News] delivered=%1 audience=%2", count, audience);
  return true;
 }
}
