// Only this presenter owns visibility of the two repurposed native pause buttons.
class SA_PausePanel
{
 protected SCR_ButtonTextComponent m_Admin;
 protected SCR_ButtonTextComponent m_Players;
 protected bool m_Applied;
 protected bool m_Eligible;
 protected bool m_Active;
 protected bool m_ShowPlayers;
 protected int m_ApplyCount;
 void Bind(Widget root)
 {
  m_Admin = SCR_ButtonTextComponent.GetButtonText("FieldManual", root);
  m_Players = SCR_ButtonTextComponent.GetButtonText("Players", root);
  m_Applied = false; m_ApplyCount = 0;
 }
 bool Apply(bool eligible, bool active, bool canListPlayers)
 {
  bool showPlayers = eligible && active && canListPlayers;
  if (m_Applied && m_Eligible == eligible && m_Active == active && m_ShowPlayers == showPlayers) return false;
  if (m_Admin)
  {
   if (!m_Applied || m_Eligible != eligible)
   { m_Admin.SetVisible(eligible, false); m_Admin.SetEnabled(eligible, false); }
   if (!m_Applied || m_Active != active)
   {
    if (active) m_Admin.SetText("Logout from Admin Panel");
    else m_Admin.SetText("Admin Panel");
   }
  }
  if (m_Players && (!m_Applied || m_ShowPlayers != showPlayers))
  {
   m_Players.SetVisible(showPlayers, false); m_Players.SetEnabled(showPlayers, false);
   if (!m_Applied) m_Players.SetText("Admin Players");
  }
  m_Eligible = eligible; m_Active = active; m_ShowPlayers = showPlayers; m_Applied = true; m_ApplyCount++;
  return true;
 }
 int GetApplyCount() { return m_ApplyCount; }
}
