// All typography uses the bundled OFL glyph atlas, independent of game language/fonts.
class BN_NewsHUD
{
 protected Widget m_Root;
 protected Widget m_Ticker;
 protected ref map<string,int> m_Indices = new map<string,int>();
 protected ref array<float> m_Widths = {};
 protected ref array<ResourceName> m_CellTextures = {};
 protected ref map<string,string> m_Forms = new map<string,string>();
 protected float m_Width;
 protected float m_X;
 protected float m_Remaining;
 protected bool m_RTL;
 protected float m_Age;
 protected float m_Speed;

 void BN_NewsHUD()
 {
  BN_GlyphData.Init(m_Indices, m_Widths, m_Forms);
  BN_GlyphResources.Init(m_CellTextures);
 }
 protected float MakeLine(Widget parent, string value, float scale)
 {
  // A null parent attaches glyphs to the whole workspace, leaking text above the UI.
  if (!parent) return 0;
  array<string> visual = {}; BN_Arabic.Visual(value, m_Forms, visual);
  float x;
  foreach (string c : visual)
  {
   int index;
   if (!m_Indices.Find(c, index)) index = m_Indices.Get("?");
   if (c != " ")
   {
    ImageWidget glyph = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets("{A1B4A620769B4B03}UI/Layouts/BN/Glyph.layout", parent));
    if (!glyph) continue;
    // Each character has a full texture: no custom UVs or atlas sampling at runtime.
    ResourceName cell = m_CellTextures[index];
    glyph.LoadImageTexture(0, cell);
    glyph.SetImage(0);
    glyph.SetVisible(true);
    glyph.SetEnabled(true);
    glyph.SetZOrder(20);
    FrameSlot.SetAnchor(glyph, 0, 0);
    FrameSlot.SetPos(glyph, x, 0);
    FrameSlot.SetSize(glyph, 64*scale, 96*scale);
   }
   x += m_Widths[index]*scale;
  }
  return x;
 }
 void Show(string message, string factionKey, int duration)
 {
  Hide();
  if (!GetGame().GetWorkspace()) return;
  m_Root = GetGame().GetWorkspace().CreateWidgets("{A1B4A620769B4B02}UI/Layouts/BN/News.layout");
  if (!m_Root) return;
  m_Root.SetZOrder(1000);
  ImageWidget portrait = ImageWidget.Cast(m_Root.FindAnyWidget("SenderPortrait"));
  if (portrait)
  {
   portrait.SetZOrder(10);
   ResourceName portraitTexture = BN_SenderPortrait.ForFaction(factionKey);
   Widget arabicTitle = m_Root.FindAnyWidget("ArabicTitle");
   if (arabicTitle) arabicTitle.SetVisible(portraitTexture.IsEmpty());
   if (portraitTexture.IsEmpty()) portrait.SetVisible(false);
   else portrait.SetVisible(portrait.LoadImageTexture(0, portraitTexture));
  }
  Widget clip = m_Root.FindAnyWidget("TickerClip");
  m_Ticker = m_Root.FindAnyWidget("TickerLine");
  if (!clip || !m_Ticker) { Hide(); return; }
  clip.SetFlags(WidgetFlags.CLIPCHILDREN);
  FrameSlot.SetAnchor(m_Ticker, 0, 0);
  m_Root.FindAnyWidget("TeamName").SetFlags(WidgetFlags.CLIPCHILDREN);
  m_Width = MakeLine(m_Ticker, message, 0.78);
  MakeLine(m_Root.FindAnyWidget("ArabicTitle"), "عاجل", 0.8);
  string teamName = "BLCA NEWS";
  FactionManager fm = GetGame().GetFactionManager();
  SCR_Faction faction;
  if (fm) faction = SCR_Faction.Cast(fm.GetFactionByKey(factionKey));
  ImageWidget logo = ImageWidget.Cast(m_Root.FindAnyWidget("TeamLogo"));
  if (logo && logo.LoadImageTexture(0, "{16F849822AA54A70}UI/Textures/BN/ServerLogo.edds") && logo.SetImage(0))
  {
   int logoWidth, logoHeight;
   logo.GetImageSize(0, logoWidth, logoHeight);
   if (logoWidth > 0 && logoHeight > 0)
   {
    float logoScale = Math.Min(100.0 / logoWidth, 80.0 / logoHeight);
    float displayWidth = logoWidth * logoScale;
    float displayHeight = logoHeight * logoScale;
    FrameSlot.SetSize(logo, displayWidth, displayHeight);
    FrameSlot.SetPos(logo, 20 + (100 - displayWidth) * 0.5, 15 + (80 - displayHeight) * 0.5);
   }
  }
  else if (logo) logo.SetVisible(false);
  string senderKey = factionKey; senderKey.ToUpper();
  if (senderKey == "USSR") teamName = "USSR";
  else if (senderKey != "SERVER" && faction)
  {
   teamName = faction.GetFactionName();
   // Translate the faction's localized display name on each client's language.
   teamName = WidgetManager.Translate(teamName);
  }
  MakeLine(m_Root.FindAnyWidget("TeamName"), teamName, 0.44);
  array<string> chars = {}; BN_Arabic.Split(message, chars);
  m_RTL = false;
  foreach (string c : chars) if (BN_Arabic.IsRTL(c, m_Forms)) m_RTL = true;
  // The leading character enters first; never start with the middle already in view.
  m_X = 1308;
  if (m_RTL) m_X = -m_Width - 8;
  m_Remaining = duration;
  m_Age = 0;
  m_Speed = Math.Max(115.0, (m_Width + 1316.0) / Math.Max(5.0, duration - 3.0));
  m_Root.SetOpacity(0);
  FrameSlot.SetPos(m_Ticker, m_X, -6);
  m_Root.Update();
  float screenW, screenH, panelW, panelH, panelX, panelY;
  m_Root.GetScreenSize(screenW, screenH);
  Widget panel = m_Root.FindAnyWidget("Broadcast");
  panel.GetScreenSize(panelW, panelH);
  panel.GetScreenPos(panelX, panelY);
  PrintFormat("[BLCA News] viewport=%1x%2 panel=%3x%4 position=%5,%6", screenW, screenH, panelW, panelH, panelX, panelY);
 }
 void Update(float timeSlice)
 {
  if (!m_Root) return;
  m_Remaining -= timeSlice;
  m_Age += timeSlice;
  if (m_Remaining <= 0) { Hide(); return; }
  // Arabic scrolls left-to-right, so the rightmost start of the sentence enters first.
  float speed = 0;
  if (m_Age >= 1.2) speed = m_Speed;
  if (m_RTL)
  {
   m_X += speed*timeSlice;
   if (m_X > 1308) m_X = -m_Width - 8;
  }
  else
  {
   m_X -= speed*timeSlice;
   if (m_X < -m_Width - 8) m_X = 1308;
  }
  FrameSlot.SetPos(m_Ticker, m_X, -6);
  float phase = Math.Min(Math.Clamp(m_Age / 1.2, 0, 1), Math.Clamp(m_Remaining / 1.1, 0, 1));
  float eased = phase * phase * (3.0 - 2.0 * phase);
  m_Root.SetOpacity(eased);
  float drop = 24.0 * (1.0 - eased);
  FrameSlot.SetPos(m_Root.FindAnyWidget("Broadcast"), -650, -240 + drop);
  FrameSlot.SetPos(m_Root.FindAnyWidget("SenderPortrait"), 425, -395 + drop);
 }
 void RequestHide() { m_Remaining = Math.Min(m_Remaining, 1.1); }
 float GetTickerSpeed() { return m_Speed; }
 float GetTextWidth() { return m_Width; }
 float GetTickerPosition() { return m_X; }
 void Hide()
 {
  if (m_Root) m_Root.RemoveFromHierarchy();
  m_Root = null; m_Ticker = null;
 }
 bool IsVisible() { return m_Root && m_Ticker; }
 bool HasVisibleBounds()
 {
  if (!IsVisible()) return false;
  m_Root.Update();
  float width, height;
  m_Root.FindAnyWidget("Broadcast").GetScreenSize(width, height);
  return width > 0 && height > 0;
 }
 bool HasUnclippedPortraitBounds()
 {
  if (!m_Root) return false;
  Widget portrait = m_Root.FindAnyWidget("SenderPortrait");
  if (!portrait || portrait.GetParent() != m_Root) return false;
  m_Root.Update();
  float width, height, px, py, bx, by;
  portrait.GetScreenSize(width, height);
  portrait.GetScreenPos(px, py);
  m_Root.FindAnyWidget("Broadcast").GetScreenPos(bx, by);
  return width > 0 && height > 0 && py < by;
 }
 void ~BN_NewsHUD() { Hide(); }
}


