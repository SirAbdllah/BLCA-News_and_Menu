// Open the normal pause menu locally without Workbench's Escape shortcut.
class BLCA_MenuPreviewCommand : BN_NewsCommand
{
 override string GetKeyword() { return "newmenu"; }
 override bool IsServerSide() { return false; }
 override int RequiredChatPermission() { return 0; }
 override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
 {
  // Defer until chat has finished processing and released its input context.
  GetGame().GetCallqueue().CallLater(BLCA_OpenMenu, 150, false);
  return new ScrServerCmdResult("", EServerCmdResultType.OK);
 }
 protected void BLCA_OpenMenu()
 {
  ArmaReforgerScripted.OpenPauseMenu();
 }
}
