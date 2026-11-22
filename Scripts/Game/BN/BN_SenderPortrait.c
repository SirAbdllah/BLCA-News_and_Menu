// Map faction keys, never the content of a news message, to the sender's portrait.
class BN_SenderPortrait
{
 static ResourceName ForFaction(string factionKey)
 {
  factionKey.ToUpper();
  // Resolve only the explicit US and USSR sides.
  if (factionKey == "US" || factionKey == "USA" || factionKey == "ARMY")
   return "{CB620701458A4D31}UI/Textures/BN/ArmyPortrait.edds";
  if (factionKey == "USSR")
   return "{CB620701458A4D32}UI/Textures/BN/USSRPortrait.edds";
  // Neutral/server announcements have no combatant portrait.
  return "";
 }
}
