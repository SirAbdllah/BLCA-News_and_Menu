// Server selects recipients before sending. Opposing clients never receive the news payload.
modded class SCR_PlayerController
{
 void BN_DeliverNews(string text, string senderFaction, int duration, bool withSound)
 {
  if (!Replication.IsServer()) return;
  Rpc(BN_ReceiveOwnedNews, text, senderFaction, duration, withSound);
  if (GetGame().GetPlayerController() == this) BN_ReceiveOwnedNews(text, senderFaction, duration, withSound);
 }
 [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
 protected void BN_ReceiveOwnedNews(string text, string senderFaction, int duration, bool withSound)
 {
  if (!GetGame().GetWorkspace()) return;
  if (duration <= 0) BN_LocalNews.Stop();
  else BN_LocalNews.Show(text, senderFaction, duration, withSound);
 }
}
