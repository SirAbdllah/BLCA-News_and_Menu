// Decorative handlers leave the engine's modal stack, input actions and callbacks in control.
class BLCA_DialogButtonVisual : ScriptedWidgetEventHandler
{
 protected Widget m_Button;
 protected Widget m_Fill;
 protected Widget m_Outline;
 protected Widget m_Label;
 protected bool m_Hovered;
 protected bool m_Focused;
 protected bool m_Pressed;

 static BLCA_DialogButtonVisual Bind(Widget button, Widget fill, Widget label)
 {
  if (!button) return null;

  BLCA_DialogButtonVisual visual = new BLCA_DialogButtonVisual();
  visual.m_Fill = fill;
  visual.m_Label = label;
  button.AddHandler(visual);
  return visual;
 }

 override void HandlerAttached(Widget w)
 {
  super.HandlerAttached(w);
  m_Button = w;
  m_Focused = GetGame().GetWorkspace().GetFocusedWidget() == w;

  Widget content = DecorationContainer(w);
  if (!content) return;

  if (!m_Fill)
  {
   Widget backdrop = Layer(content, "BLCA_DialogBackdrop", -1);
   if (backdrop)
    m_Fill = Rectangle(backdrop, BLCA_PausePalette.Button(), 0, 0, 1, 1, 0, 0, 0, 0);
  }

  m_Outline = Layer(content, "BLCA_DialogOutline", 1);
  if (m_Outline)
  {
   Color color = BLCA_PausePalette.Outline();
   Rectangle(m_Outline, color, 0, 0, 1, 0, 0, 0, 0, -2);
   Rectangle(m_Outline, color, 0, 1, 1, 1, 0, -2, 0, 0);
   Rectangle(m_Outline, color, 0, 0, 0, 1, 0, 0, -2, 0);
   Rectangle(m_Outline, color, 1, 0, 1, 1, -2, 0, 0, 0);
  }
  Paint();
 }

 // ButtonWidget has a single child slot. Reuse the custom frame or wrap the stock content.
 protected static Widget DecorationContainer(Widget button)
 {
  Widget content = button.GetChildren();
  if (!content || FrameWidget.Cast(content) || OverlayWidget.Cast(content)) return content;

  Widget wrapper = GetGame().GetWorkspace().CreateWidget(WidgetType.OverlayWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS, Color.White, 0);
  if (!wrapper) return null;

  LayoutHorizontalAlign horizontal = ButtonSlot.GetHorizontalAlign(content);
  LayoutVerticalAlign vertical = ButtonSlot.GetVerticalAlign(content);
  float left, top, right, bottom;
  ButtonSlot.GetPadding(content, left, top, right, bottom);
  wrapper.AddChild(content);
  button.AddChild(wrapper);
  wrapper.SetName("BLCA_DialogContent");
  ButtonSlot.SetHorizontalAlign(wrapper, horizontal);
  ButtonSlot.SetVerticalAlign(wrapper, vertical);
  ButtonSlot.SetPadding(wrapper, left, top, right, bottom);
  OverlaySlot.SetHorizontalAlign(content, LayoutHorizontalAlign.Stretch);
  OverlaySlot.SetVerticalAlign(content, LayoutVerticalAlign.Stretch);
  return wrapper;
 }

 protected static Widget Layer(Widget parent, string name, int zOrder)
 {
  Widget layer = GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS, Color.White, zOrder, parent);
  if (!layer) return null;

  layer.SetName(name);
  if (OverlayWidget.Cast(parent))
  {
   OverlaySlot.SetHorizontalAlign(layer, LayoutHorizontalAlign.Stretch);
   OverlaySlot.SetVerticalAlign(layer, LayoutVerticalAlign.Stretch);
  }
  else
  {
   FrameSlot.SetAnchorMin(layer, 0, 0);
   FrameSlot.SetAnchorMax(layer, 1, 1);
   FrameSlot.SetOffsets(layer, 0, 0, 0, 0);
  }
  return layer;
 }

 protected static Widget Rectangle(Widget parent, Color color, float x0, float y0, float x1, float y1, float left, float top, float right, float bottom)
 {
  Widget rect = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS | WidgetFlags.STRETCH | WidgetFlags.BLEND, color, 0, parent);
  if (!rect) return null;

  FrameSlot.SetAnchorMin(rect, x0, y0);
  FrameSlot.SetAnchorMax(rect, x1, y1);
  FrameSlot.SetOffsets(rect, left, top, right, bottom);
  return rect;
 }

 protected void Paint()
 {
  if (!m_Button) return;

  bool enabled = m_Button.IsEnabledInHierarchy();
  Color color = BLCA_PausePalette.Button();
  if (!enabled) color = BLCA_PausePalette.Disabled();
  else if (m_Pressed) color = BLCA_PausePalette.Pressed();
  else if (m_Focused) color = BLCA_PausePalette.Selected();
  else if (m_Hovered) color = BLCA_PausePalette.Hover();

  if (m_Fill) m_Fill.SetColor(color);
  if (m_Outline) m_Outline.SetVisible(enabled && (m_Focused || m_Hovered || m_Pressed));
  if (m_Label)
  {
   if (enabled) m_Label.SetColor(BLCA_PausePalette.Text());
   else m_Label.SetColor(BLCA_PausePalette.DisabledText());
  }
 }

 override bool OnEnable(Widget w) { if (w == m_Button) Paint(); return false; }
 override bool OnDisable(Widget w)
 {
  if (w == m_Button) { m_Pressed = false; Paint(); }
  return false;
 }
 override bool OnShow(Widget w) { if (w == m_Button) Paint(); return false; }
 override bool OnFocus(Widget w, int x, int y) { m_Focused = true; Paint(); return false; }
 override bool OnFocusLost(Widget w, int x, int y) { m_Focused = false; m_Pressed = false; Paint(); return false; }
 override bool OnMouseEnter(Widget w, int x, int y) { m_Hovered = true; Paint(); return false; }
 override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
 {
  if (!enterW || !SCR_WidgetTools.InHierarchy(enterW, m_Button))
  {
   m_Hovered = false;
   m_Pressed = false;
   Paint();
  }
  return false;
 }

 override bool OnMouseButtonDown(Widget w, int x, int y, int button)
 {
  if (button == 0) { m_Pressed = true; Paint(); }
  return false;
 }

 override bool OnMouseButtonUp(Widget w, int x, int y, int button)
 {
  if (button == 0) { m_Pressed = false; Paint(); }
  return false;
 }
}

class BLCA_PauseDialogStyle
{
 static void Tint(Widget root, string name, Color color)
 {
  Widget child = root.FindAnyWidget(name);
  if (child) child.SetColor(color);
 }

 static void ApplyCustomPanel(Widget root)
 {
  Tint(root, "BG", BLCA_PausePalette.Panel());
  Tint(root, "Gold", BLCA_PausePalette.Outline());
  Tint(root, "Title", BLCA_PausePalette.Text());
 }

 static BLCA_DialogButtonVisual ApplyCustomButton(Widget root, string name)
 {
  return BLCA_DialogButtonVisual.Bind(root.FindAnyWidget(name), root.FindAnyWidget(name + "BG"), root.FindAnyWidget(name + "Text"));
 }

 static void ApplyBack(Widget root)
 {
  if (!root || !PauseMenuUI.BLCA_HasPauseMenu()) return;

  SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, root);
  if (back) back.BLCA_ApplyDialogStyle();
 }
}

modded class PauseMenuUI
{
 static bool BLCA_HasPauseMenu()
 {
  return s_Instance && s_Instance.GetRootWidget();
 }
}

modded class SCR_ConfigurableDialogUi
{
 override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
 {
  super.Init(root, preset, proxyMenu);
  if (!root || !PauseMenuUI.BLCA_HasPauseMenu()) return;

  Widget panel = root.FindAnyWidget("DialogBase");
  if (panel) BLCA_PauseDialogStyle.Tint(panel, "Background", BLCA_PausePalette.Panel());
  BLCA_PauseDialogStyle.Tint(root, "OuterBackground", Color.FromSRGBA(8, 21, 35, 102));
  BLCA_PauseDialogStyle.Tint(root, "Separator", BLCA_PausePalette.Outline());
  BLCA_PauseDialogStyle.Tint(root, "ImgTitleIcon", BLCA_PausePalette.Text());
  BLCA_PauseDialogStyle.Tint(root, "Title", BLCA_PausePalette.Text());
  BLCA_PauseDialogStyle.Tint(root, "Message", BLCA_PausePalette.Text());

  foreach (string tag, SCR_InputButtonComponent button : m_aButtonComponents)
   if (button) button.BLCA_ApplyDialogStyle();
 }
}

modded class SCR_InputButtonComponent
{
 protected ref BLCA_DialogButtonVisual m_BLCA_DialogVisual;

 void BLCA_ApplyDialogStyle()
 {
  if (!m_wRoot || m_BLCA_DialogVisual) return;

  m_LabelDefault = BLCA_PausePalette.Text();
  m_ActionDefault = BLCA_PausePalette.Selected();
  m_ActionHovered = BLCA_PausePalette.Hover();
  m_ActionDisabled = BLCA_PausePalette.Disabled();
  m_ActionToggled = BLCA_PausePalette.Pressed();
  m_bChangeLabelColorOnDisabled = false;
  m_BLCA_DialogVisual = BLCA_DialogButtonVisual.Bind(m_wRoot, null, m_wTextHint);
  BLCA_TintDialogGlyphs();
 }

 protected void BLCA_TintDialogGlyphs()
 {
  BLCA_PauseDialogStyle.Tint(m_wRoot, "m_ButtonText", BLCA_PausePalette.Text());
  BLCA_PauseDialogStyle.Tint(m_wRoot, "m_ButtonImg", BLCA_PausePalette.Text());
  BLCA_PauseDialogStyle.Tint(m_wRoot, "m_Outline", BLCA_PausePalette.Outline());
  BLCA_PauseDialogStyle.Tint(m_wRoot, "m_OutlineClear", BLCA_PausePalette.Outline());
  if (m_ButtonDisplay) m_ButtonDisplay.ResetColor();
 }

 override protected bool ChangeInputDevice(EInputDeviceType inputDevice, bool hasFocus, bool resetOverride = false)
 {
  bool changed = super.ChangeInputDevice(inputDevice, hasFocus, resetOverride);
  if (m_BLCA_DialogVisual && m_wRoot) BLCA_TintDialogGlyphs();
  return changed;
 }
}

modded class SCR_SettingsSuperMenu
{
 override void OnMenuOpen()
 {
  super.OnMenuOpen();
  BLCA_PauseDialogStyle.ApplyBack(GetRootWidget());
 }
}

modded class SCR_GroupMenu
{
 override void OnMenuOpen()
 {
  super.OnMenuOpen();
  BLCA_PauseDialogStyle.ApplyBack(GetRootWidget());
 }
}
