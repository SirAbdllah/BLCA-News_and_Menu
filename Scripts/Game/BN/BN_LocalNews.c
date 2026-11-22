// Client UI owns its update loop; Game Master modes need not tick the base game-mode HUD.
class BN_LocalNews
{
 protected static ref BN_NewsHUD s_HUD;
 protected static int s_LastTick;
 static bool Show(string message, string faction, int duration, bool withSound = false)
 {
  if (!GetGame() || !GetGame().GetWorkspace()) return false;
  GetGame().GetCallqueue().Remove(Tick);
  if (!s_HUD) s_HUD = new BN_NewsHUD();
  s_HUD.Show(message, faction, duration);
  if (!s_HUD.IsVisible()) return false;
  s_LastTick = System.GetTickCount();
  GetGame().GetCallqueue().CallLater(Tick, 16, true);
  BN_NewsAudio.Stop();
  if (withSound) BN_NewsAudio.Play();
  return true;
 }
 static void Tick()
 {
  if (!s_HUD || !s_HUD.IsVisible()) { Stop(true); return; }
  int now = System.GetTickCount();
  float elapsed = Math.Clamp((now - s_LastTick) * 0.001, 0, 0.25);
  s_LastTick = now;
  s_HUD.Update(elapsed);
 }
 static void Stop(bool instant = false)
 {
  BN_NewsAudio.Stop();
  if (!instant && s_HUD && s_HUD.IsVisible()) { s_HUD.RequestHide(); return; }
  if (GetGame()) GetGame().GetCallqueue().Remove(Tick);
  if (s_HUD) s_HUD.Hide();
  s_HUD = null;
 }
}
