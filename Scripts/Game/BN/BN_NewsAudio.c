class BN_NewsAudio
{
 protected static AudioHandle s_Handle;
 static void Play()
 {
  Stop();
  s_Handle = AudioSystem.PlaySound("{A620F06D61B845D1}Sounds/BN/NewsIntro.wav");
 }
 static void Stop()
 {
  if (AudioSystem.IsSoundPlayed(s_Handle)) AudioSystem.TerminateSoundFadeOut(s_Handle, true, 0.7);
 }
 static bool IsPlaying() { return AudioSystem.IsSoundPlayed(s_Handle); }
}
