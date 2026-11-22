class BLCA_PausePalette
{
 static Color Panel()
 {
  Color color = Color.FromSRGBA(8, 21, 35, 255);
  color.SetA(0.87);
  return color;
 }

 static Color Button()
 {
  Color color = Color.FromSRGBA(17, 37, 55, 255);
  color.SetA(0.75);
  return color;
 }

 static Color Selected() { return Color.FromSRGBA(24, 60, 91, 255); }
 static Color Outline() { return Color.FromSRGBA(111, 195, 255, 255); }
 static Color Text() { return Color.FromSRGBA(239, 248, 255, 255); }
 static Color Hover() { return Color.FromSRGBA(30, 71, 106, 255); }
 static Color Pressed() { return Color.FromSRGBA(43, 94, 135, 255); }
 static Color Disabled() { return Color.FromSRGBA(32, 47, 61, 210); }
 static Color DisabledText() { return Color.FromSRGBA(148, 170, 189, 255); }
}

class BLCA_PauseRowStyle : ScriptedWidgetEventHandler
{
 protected Widget m_Row;
 protected Widget m_Fill;
 protected Widget m_Border;
 protected Widget m_Text;
 protected Widget m_Icon;
 protected Widget m_ActionIcon;
 protected bool m_Focused;
 protected bool m_MouseOver;
 protected bool m_MouseDown;
 protected bool m_Clicked;

 void Bind(SCR_ButtonTextComponent button)
 {
  m_Row = button.GetRootWidget();
  m_Fill = button.m_wBackground;
  m_Border = button.m_wBorder;
  m_Text = button.GetTextWidget();
  m_Icon = m_Row.FindAnyWidget("Image0");
  m_ActionIcon = m_Row.FindAnyWidget("ActionIcon");

  // The stock modular effects tint the gradient, text and icon and animate the fill opacity.
  SCR_ModularButtonComponent modular = SCR_ModularButtonComponent.FindComponent(m_Row);
  if (modular) modular.SetAllEffectsEnabled(false);
  button.m_bUseColorization = false;
  button.m_bUseTextColorization = false;
  button.m_bShowBackgroundOnFocus = false;
  button.m_bShowBorderOnFocus = false;
  button.m_bShowBorderOnHover = false;
  button.SetDisabledOpacity(1);

  HideDecoration("LineHighlight");
  HideScenarioFills();
  if (m_Fill)
  {
   AnimateWidget.StopAnimation(m_Fill, WidgetAnimationOpacity);
   m_Fill.SetOpacity(1);
   m_Fill.SetVisible(true);
   OverlaySlot.SetPadding(m_Fill, 13, 1, 19, 1);
  }
  if (m_Border)
  {
   AnimateWidget.StopAnimation(m_Border, WidgetAnimationOpacity);
   m_Border.SetColor(BLCA_PausePalette.Outline());
   OverlaySlot.SetPadding(m_Border, 13, 1, 19, 1);
  }
  if (m_Row.IsVisible() && !m_Row.IsEnabled())
  {
   AnimateWidget.StopAnimation(m_Row, WidgetAnimationOpacity);
   m_Row.SetOpacity(1);
  }

  m_Focused = GetGame().GetWorkspace().GetFocusedWidget() == m_Row;
  m_Row.AddHandler(this);
  Refresh();
 }

 protected void HideDecoration(string name)
 {
  Widget decoration = m_Row.FindAnyWidget(name);
  if (decoration) decoration.SetVisible(false);
 }

 protected void HideScenarioFills()
 {
  if (!m_Fill || !m_Fill.GetParent()) return;
  for (Widget decoration = m_Fill.GetParent().GetChildren(); decoration; decoration = decoration.GetSibling())
   if (decoration != m_Fill && ImageWidget.Cast(decoration) && decoration.GetName().EndsWith("PauseFill"))
    decoration.SetVisible(false);
 }

 void ReconcileDecorations()
 {
  HideScenarioFills();
  if (m_Fill) { m_Fill.SetVisible(true); m_Fill.SetOpacity(1); }
  Refresh();
 }

 protected void Refresh()
 {
  if (!m_Row) return;

  bool enabled = m_Row.IsEnabled();
  bool hovered = m_MouseOver && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
  Color fill = BLCA_PausePalette.Button();
  Color foreground = BLCA_PausePalette.Text();
  if (!enabled)
  {
   fill = BLCA_PausePalette.Disabled();
   foreground = BLCA_PausePalette.DisabledText();
  }
  else if (m_MouseDown || m_Clicked)
   fill = BLCA_PausePalette.Pressed();
  else if (hovered)
   fill = BLCA_PausePalette.Hover();
  else if (m_Focused)
   fill = BLCA_PausePalette.Selected();

  if (m_Fill) m_Fill.SetColor(fill);
  if (m_Border) m_Border.SetOpacity(enabled && (m_Focused || hovered));
  if (m_Text) m_Text.SetColor(foreground);
  if (m_Icon) m_Icon.SetColor(foreground);
  if (m_ActionIcon) m_ActionIcon.SetColor(foreground);
 }

 override bool OnFocus(Widget w, int x, int y) { m_Focused = true; Refresh(); return false; }
 override bool OnFocusLost(Widget w, int x, int y) { m_Focused = false; m_MouseDown = false; Refresh(); return false; }
 override bool OnMouseEnter(Widget w, int x, int y) { m_MouseOver = true; Refresh(); return false; }
 override bool OnMouseLeave(Widget w, Widget enterW, int x, int y) { m_MouseOver = false; m_MouseDown = false; Refresh(); return false; }
 override bool OnEnable(Widget w) { Refresh(); return false; }
 override bool OnDisable(Widget w) { m_MouseDown = false; Refresh(); return false; }
 override bool OnShow(Widget w)
 {
  if (w != m_Row) return false;
  m_Focused = GetGame().GetWorkspace().GetFocusedWidget() == m_Row;
  if (!m_Row.IsEnabled()) m_Row.SetOpacity(1);
  Refresh();
  return false;
 }

 override bool OnHide(Widget w)
 {
  if (w != m_Row) return false;
  m_Focused = false;
  m_MouseOver = false;
  m_MouseDown = false;
  m_Clicked = false;
  GetGame().GetCallqueue().Remove(EndClick);
  return false;
 }

 override bool OnMouseButtonDown(Widget w, int x, int y, int button)
 {
  if (button == 0 && m_Row && m_Row.IsEnabled()) { m_MouseDown = true; Refresh(); }
  return false;
 }

 override bool OnMouseButtonUp(Widget w, int x, int y, int button)
 {
  if (button == 0) { m_MouseDown = false; Refresh(); }
  return false;
 }

 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (button != 0 || !m_Row || !m_Row.IsEnabled()) return false;
  m_Clicked = true;
  Refresh();
  GetGame().GetCallqueue().Remove(EndClick);
  GetGame().GetCallqueue().CallLater(EndClick, 110);
  return false;
 }

 protected void EndClick() { m_Clicked = false; Refresh(); }

 void Detach()
 {
  GetGame().GetCallqueue().Remove(EndClick);
  if (m_Row) m_Row.RemoveHandler(this);
  m_Row = null;
 }
}

modded class PauseMenuUI
{
 protected ref array<ref BLCA_PauseRowStyle> m_BLCA_PauseRows;
 protected Widget m_BLCA_PauseRoot;

 override void OnMenuOpen()
 {
  super.OnMenuOpen();
  if (!m_wRoot) return;
  m_BLCA_PauseRoot = m_wRoot;

  // Preserve the existing menu customization. SA_PausePanel owns the repurposed entries.
  Widget invite = m_wRoot.FindAnyWidget("InviteFriend");
  if (invite) { invite.SetEnabled(false); invite.SetVisible(false); }

  BLCA_ApplyPauseLogo();

  Widget backdrop = m_wRoot.FindAnyWidget("Background Overlay");
  if (backdrop) backdrop.SetVisible(false);
  Widget panel = m_wRoot.FindAnyWidget("Buttons Overlay");
  if (!panel) return;
  panel.SetColor(Color.FromRGBA(0, 0, 0, 0));

  Widget fill = BLCA_FindOwnPauseFill(panel);
  if (!fill) fill = BLCA_AddPanelImage(panel, "BLCA_PausePanel", BLCA_PausePalette.Panel(), -2);

  ImageWidget water = BLCA_AddPanelImage(panel, "BLCA_PauseWater", Color.White, -1);
  if (water && !water.LoadImageTexture(0, "{4170FAE7366525DB}UI/Textures/BLCA/BLCA_PauseWater.edds"))
   water.RemoveFromHierarchy();

  m_BLCA_PauseRows = {};
  Widget row = panel.FindAnyWidget("ButtonRow");
  if (row)
  {
   for (Widget item = row.GetChildren(); item; item = item.GetSibling())
   {
    BLCA_StylePauseRow(item);
    if (item.GetName().StartsWith("Separator"))
     item.SetColor(Color.FromSRGBA(111, 195, 255, 94));
   }
  }
  BLCA_StylePauseRow(panel.FindAnyWidget("Camera"));
  if (m_wVersion) m_wVersion.SetColor(BLCA_PausePalette.Text());
  GetGame().GetCallqueue().Remove(BLCA_ReconcilePauseDecorations);
  GetGame().GetCallqueue().CallLater(BLCA_ReconcilePauseDecorations, 1, false);
 }

 protected void BLCA_ApplyPauseLogo()
 {
  ImageWidget logo = ImageWidget.Cast(m_wRoot.FindAnyWidget("Logo"));
  // Another scenario can select its own image slot on the shared stock widget.
  if (!logo || !logo.LoadImageTexture(0, "{7AA9B5411DF24E02}UI/Textures/BLCA/BLCA_PauseBrand.edds") || !logo.SetImage(0)) return;
  int width, height;
  logo.GetImageSize(0, width, height);
  if (width > 0 && height > 0)
  {
   float scale = Math.Min(448.0 / width, 168.0 / height);
   logo.SetSize(width * scale, height * scale);
  }
  LayoutSlot.SetHorizontalAlign(logo, LayoutHorizontalAlign.Center);
  LayoutSlot.SetPadding(logo, 0, 6, 0, 6);
 }

 protected Widget BLCA_FindOwnPauseFill(Widget panel)
 {
  for (Widget child = panel.GetChildren(); child; child = child.GetSibling())
   if (ImageWidget.Cast(child) && child.GetName() == "BLCA_PausePanel") return child;
  return null;
 }

 protected void BLCA_ReconcilePauseDecorations()
 {
  if (!m_wRoot || m_wRoot != m_BLCA_PauseRoot) return;
  Widget panel = m_wRoot.FindAnyWidget("Buttons Overlay");
  if (!panel) return;
  Widget fill = BLCA_FindOwnPauseFill(panel);
  if (fill)
  {
   for (Widget decoration = panel.GetChildren(); decoration; decoration = decoration.GetSibling())
    if (decoration != fill && ImageWidget.Cast(decoration) && decoration.GetName().EndsWith("PausePanel"))
     decoration.SetVisible(false);
  }
  BLCA_ApplyPauseLogo();
  if (m_BLCA_PauseRows)
   foreach (BLCA_PauseRowStyle style : m_BLCA_PauseRows) style.ReconcileDecorations();
 }

 protected ImageWidget BLCA_AddPanelImage(Widget panel, string name, Color color, int zOrder)
 {
  ImageWidget image = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.STRETCH | WidgetFlags.BLEND, color, zOrder, panel));
  if (!image) return null;
  image.SetName(name);
  OverlaySlot.SetHorizontalAlign(image, LayoutHorizontalAlign.Stretch);
  OverlaySlot.SetVerticalAlign(image, LayoutVerticalAlign.Stretch);
  return image;
 }

 protected void BLCA_StylePauseRow(Widget row)
 {
  if (!row) return;
  SCR_ButtonTextComponent button = SCR_ButtonTextComponent.FindButtonTextComponent(row);
  if (!button) return;
  BLCA_PauseRowStyle style = new BLCA_PauseRowStyle();
  style.Bind(button);
  m_BLCA_PauseRows.Insert(style);
 }

 override void OnMenuClose()
 {
  GetGame().GetCallqueue().Remove(BLCA_ReconcilePauseDecorations);
  m_BLCA_PauseRoot = null;
  if (m_BLCA_PauseRows)
  {
   foreach (BLCA_PauseRowStyle style : m_BLCA_PauseRows) style.Detach();
   m_BLCA_PauseRows = null;
  }
  super.OnMenuClose();
 }
}
