class SA_WeatherBridge
{
 protected static TimeAndWeatherManagerEntity Manager()
 {
  if (!GetGame() || !Replication.IsServer()) return null;
  ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
  if (!world) return null;
  return world.GetTimeAndWeatherManager();
 }

 static void ReadStatus(SA_Status status)
 {
  status.weatherCurrent = "";
  status.weatherAutomatic = false;
  status.weatherPresets.Clear();

  TimeAndWeatherManagerEntity weather = Manager();
  if (!weather) return;
  BaseWeatherStateTransitionManager transitions = weather.GetTransitionManager();
  if (!transitions) return;
  WeatherState current = transitions.GetCurrentState();
  if (!current) return;

  status.weatherCurrent = current.GetStateName();
  status.weatherAutomatic = !weather.IsWeatherLooping();
  array<ref WeatherState> states = {};
  weather.GetWeatherStatesList(states);
  foreach (WeatherState state : states)
  {
   if (!state) continue;
   string name = state.GetStateName();
   if (!name.IsEmpty()) status.weatherPresets.Insert(name);
  }
 }

 protected static string ResolvePreset(TimeAndWeatherManagerEntity weather, string requested)
 {
  string folded = requested;
  folded.ToLower();
  string match;
  bool ambiguous;
  array<ref WeatherState> states = {};
  weather.GetWeatherStatesList(states);
  foreach (WeatherState state : states)
  {
   if (!state) continue;
   string name = state.GetStateName();
   if (name == requested) return name;
   string normalized = name;
   normalized.ToLower();
   if (normalized != folded) continue;
   if (!match.IsEmpty()) ambiguous = true;
   match = name;
  }
  if (ambiguous) return "";
  return match;
 }

 static bool Apply(string requested, out string message)
 {
  message = "Weather manager is not available in this world";
  TimeAndWeatherManagerEntity weather = Manager();
  if (!weather) return false;
  BaseWeatherStateTransitionManager transitions = weather.GetTransitionManager();
  if (!transitions || !transitions.GetCurrentState()) return false;

  // The native flag loops the selected preset. Automatic weather disables that loop.
  if (requested.IsEmpty())
  {
   if (weather.IsWeatherLooping()) weather.SetCurrentWeatherLooping(false);
   if (weather.IsWeatherLooping())
   {
    message = "The server could not enable automatic weather";
    return false;
   }
   message = "Automatic weather enabled";
   return true;
  }

  string preset;
  if (requested.Length() <= 128) preset = ResolvePreset(weather, requested);
  if (preset.IsEmpty())
  {
   message = "Weather preset is not available or is ambiguous in this world";
   return false;
  }
  weather.ForceWeatherTo(true, preset);
  WeatherState current = transitions.GetCurrentState();
  if (!weather.IsWeatherLooping() || !current || current.GetStateName() != preset)
  {
   message = "The server could not confirm the weather change to " + preset;
   return false;
  }
  message = "Weather set to " + preset;
  return true;
 }
}
