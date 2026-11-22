class BN_NewsRequest
{
 string Audience = "self";
 bool ServerBrand;
 int DisplaySeconds = 60;
 string Message;
}
class BN_NewsAudience
{
 static int GroupForFaction(string key)
 {
  key.ToUpper();
  if (key == "US" || key == "USA" || key == "ARMY") return 1;
  if (key == "USSR") return 2;
  return 0;
 }
 static bool Matches(string audience, string faction)
 {
  if (audience == "all") return true;
  if (audience.IndexOf("faction:") == 0)
  {
   string requested = audience.Substring(8, audience.Length() - 8);
   requested.ToUpper(); faction.ToUpper();
   return !requested.IsEmpty() && requested == faction;
  }
  int group = GroupForFaction(faction);
  return (audience == "us" && group == 1) || (audience == "ussr" && group == 2);
 }
 static string PlayerFaction(PlayerController controller)
 {
  if (!controller) return "";
  SCR_PlayerFactionAffiliationComponent affiliation = SCR_PlayerFactionAffiliationComponent.Cast(controller.FindComponent(SCR_PlayerFactionAffiliationComponent));
  if (affiliation && affiliation.GetAffiliatedFaction()) return affiliation.GetAffiliatedFaction().GetFactionKey();
  return "";
 }
}
