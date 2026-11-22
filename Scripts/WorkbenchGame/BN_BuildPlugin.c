[WorkbenchPluginAttribute(name: "Build Breaking News resources", wbModules: {"ResourceManager"})]
class BN_BuildPlugin : WorkbenchPlugin
{
 override void Run()
 {
  ValidateResources();
 }
 static void ValidateResources()
 {
  ResourceManager rm = Workbench.GetModule(ResourceManager);
  Resource mission = Resource.Load("{C716458FF9B84A05}Missions/BLCA_NewsTest.conf");
  PrintFormat("BLCA_TEST_MISSION_VALID=%1", mission && mission.IsValid());
  ImageWidget test = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets("{A1B4A620769B4B03}UI/Layouts/BN/Glyph.layout"));
  PrintFormat("BN_ATLAS_VALID=%1", test.LoadImageTexture(0, "{E55AFC7B0268F144}UI/Textures/BN/Glyphs.edds"));
  PrintFormat("BLCA_ARMY_PORTRAIT_VALID=%1", test.LoadImageTexture(0, BN_SenderPortrait.ForFaction("US")));
  PrintFormat("BLCA_USSR_PORTRAIT_VALID=%1", test.LoadImageTexture(0, BN_SenderPortrait.ForFaction("USSR")));
  PrintFormat("BLCA_NEUTRAL_PORTRAIT_HIDDEN=%1", BN_SenderPortrait.ForFaction("").IsEmpty());
  PrintFormat("BLCA_UNKNOWN_PORTRAIT_HIDDEN=%1", BN_SenderPortrait.ForFaction("UNKNOWN").IsEmpty());
  test.RemoveFromHierarchy();
  BN_NewsHUD hud = new BN_NewsHUD();
  hud.Show("عاجل: تم فتح الطريق الرئيسي 120 | BLCA", "US", 30);
  PrintFormat("BN_HUD_VISIBLE=%1", hud.IsVisible());
  PrintFormat("BLCA_NEWS_POSITIVE_BOUNDS=%1", hud.HasVisibleBounds());
  PrintFormat("BLCA_PORTRAIT_OUTSIDE_BANNER_VALID=%1", hud.HasUnclippedPortraitBounds());
  hud.Update(0.1);
  hud.Hide();
  map<string,int> indices = new map<string,int>();
  array<float> widths = {};
  map<string,string> forms = new map<string,string>();
  BN_GlyphData.Init(indices, widths, forms);
  array<string> visual = {}; BN_Arabic.Visual("سلام", forms, visual);
  string joined;
  foreach (string c : visual) joined += c;
  PrintFormat("BN_ARABIC_SHAPING_VALID=%1", joined == "ﻡﻼﺳ");
  visual.Clear(); BN_Arabic.Visual("الطريق 120 مفتوح", forms, visual);
  joined = "";
  foreach (string c : visual) joined += c;
  PrintFormat("BN_DIGIT_ORDER_VALID=%1", joined.Contains("120"));
  Print("BN_LAYOUT_VALIDATION_FINISHED");
  PrintFormat("BLCA_LOCAL_PREVIEW_VALID=%1", BN_LocalNews.Show("عاجل: تجربة الأخبار 120", "US", 10));
  BN_LocalNews.Tick();
  BN_LocalNews.Stop();
  string message, error;
  int duration;
  bool parsed = BN_NewsCommand.ParseMessage({"news", "تم", "فتح", "الطريق", "الرئيسي"}, message, duration, error);
  PrintFormat("BLCA_NEWS_DEFAULT_SYNTAX_VALID=%1", parsed && duration == 60 && message == "تم فتح الطريق الرئيسي");
  parsed = BN_NewsCommand.ParseMessage({"news", "90", "عاجل", "الطريق مفتوح"}, message, duration, error);
  PrintFormat("BLCA_NEWS_CUSTOM_DURATION_VALID=%1", parsed && duration == 90 && message == "عاجل الطريق مفتوح");
  parsed = BN_NewsCommand.ParseMessage({"news", "عاجل"}, message, duration, error);
  PrintFormat("BLCA_NEWS_SINGLE_WORD_VALID=%1", parsed && duration == 60 && message == "عاجل");
  parsed = BN_NewsCommand.ParseMessage({"news"}, message, duration, error);
  PrintFormat("BLCA_NEWS_EMPTY_REJECTED=%1", !parsed);
  parsed = BN_NewsCommand.ParseMessage({"news", "999", "test"}, message, duration, error);
  PrintFormat("BLCA_NEWS_BAD_DURATION_REJECTED=%1", !parsed);
  BN_NewsRequest request = BN_NewsCommand.ParseRequest({"news", "us", "20", "تم", "فتح الطريق"}, error);
  PrintFormat("BLCA_US_NEWS_PARSE_VALID=%1", request && request.Audience == "us" && request.DisplaySeconds == 20 && request.Message == "تم فتح الطريق");
  request = BN_NewsCommand.ParseRequest({"news", "ussr", "خبر"}, error);
  PrintFormat("BLCA_USSR_NEWS_PARSE_VALID=%1", request && request.Audience == "ussr" && request.DisplaySeconds == 60 && request.Message == "خبر");
  request = BN_NewsCommand.ParseRequest({"news", "Convoy", "20", "vehicles", "arrived"}, error);
  PrintFormat("BLCA_PLAIN_ASCII_NUMBER_MESSAGE_VALID=%1", request && request.Audience == "self" && request.Message == "Convoy 20 vehicles arrived");
  request = BN_NewsCommand.ParseRequest({"news", "all", "20", "خبر للجميع"}, error);
  PrintFormat("BLCA_ALL_NEWS_PARSE_VALID=%1", request && request.Audience == "all" && !request.ServerBrand && request.DisplaySeconds == 20);
  request = BN_NewsCommand.ParseRequest({"news", "Server", "20", "إعلان السيرفر"}, error);
  PrintFormat("BLCA_SERVER_NEWS_PARSE_VALID=%1", request && request.Audience == "all" && request.ServerBrand && request.DisplaySeconds == 20 && request.Message == "إعلان السيرفر");
  request = BN_NewsCommand.ParseRequest({"news", "all", "20"}, error);
  PrintFormat("BLCA_MISSING_NEWS_REJECTED=%1", !request);
  PrintFormat("BLCA_RECIPIENT_FILTER_VALID=%1", BN_NewsAudience.Matches("us", "US") && !BN_NewsAudience.Matches("us", "USSR") && BN_NewsAudience.Matches("ussr", "USSR") && !BN_NewsAudience.Matches("ussr", "US") && !BN_NewsAudience.Matches("ussr", "UNKNOWN") && BN_NewsAudience.Matches("all", ""));
  request = BN_NewsCommand.ParseRequest({"news", "20", "رسالة لفصيلي"}, error);
  PrintFormat("BLCA_DEFAULT_OWN_FACTION_VALID=%1", request && request.Audience == "self" && request.DisplaySeconds == 20 && request.Message == "رسالة لفصيلي");
  PrintFormat("BLCA_EXACT_FACTION_FILTER_VALID=%1", BN_NewsAudience.Matches("faction:USSR", "USSR") && !BN_NewsAudience.Matches("faction:USSR", "US") && !BN_NewsAudience.Matches("faction:US", "USSR"));
  ImageWidget cellTest = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets("{A1B4A620769B4B03}UI/Layouts/BN/Glyph.layout"));
  array<ResourceName> cells = {}; BN_GlyphResources.Init(cells);
  PrintFormat("BLCA_STANDALONE_GLYPH_VALID=%1", cellTest.LoadImageTexture(0, cells[100]));
  PrintFormat("BLCA_SERVER_LOGO_VALID=%1", cellTest.LoadImageTexture(0, "{16F849822AA54A70}UI/Textures/BN/ServerLogo.edds"));
  cellTest.RemoveFromHierarchy();
  Resource soundSetting = BaseContainerTools.LoadContainer("{57C3F6639C3C4201}Configs/Editor/BN/NewsSound.conf");
  PrintFormat("BLCA_GM_SOUND_ATTRIBUTE_VALID=%1", soundSetting && soundSetting.IsValid());
  hud.Show("عاجل: تم تأمين المنطقة وفتح الطريق الرئيسي تم تأمين المنطقة وفتح الطريق الرئيسي تم تأمين المنطقة وفتح الطريق الرئيسي", "SERVER", 20);
  PrintFormat("BLCA_ADAPTIVE_TICKER_VALID=%1", hud.GetTextWidth() > 0 && hud.GetTickerSpeed() * 17 >= hud.GetTextWidth() + 1316);
  PrintFormat("BLCA_FIRST_LETTER_ENTRY_VALID=%1", hud.GetTickerPosition() < -hud.GetTextWidth());
  hud.RequestHide(); hud.Update(1.2);
  PrintFormat("BLCA_ANIMATED_EXIT_VALID=%1", !hud.IsVisible());
  BN_NewsAudio.Play();
  PrintFormat("BLCA_NEWS_AUDIO_PLAYBACK_VALID=%1", BN_NewsAudio.IsPlaying());
  BN_NewsAudio.Stop();
  AudioHandle nativeSound = AudioSystem.PlaySound("{E89D9A1F4BA63CDC}Sounds/Props/Furniture/Piano/Samples/Props_Piano_Jingle_1.wav");
  PrintFormat("BLCA_NATIVE_AUDIO_CONTROL=%1", AudioSystem.IsSoundPlayed(nativeSound));
  AudioSystem.TerminateSound(nativeSound);
 }
 override void RunCommandline() { Run(); }
}


