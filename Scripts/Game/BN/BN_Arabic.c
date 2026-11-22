// UTF-8 is decoded into whole byte strings. Never reverse bytes of Arabic text.
class BN_Arabic
{
 static void Split(string text, array<string> chars)
 {
  for (int i = 0; i < text.Length();)
  {
   int b = text.ToAscii(i);
   if (b < 0) b += 256;
   int n = 1;
   if (b >= 240) n = 4;
   else if (b >= 224) n = 3;
   else if (b >= 192) n = 2;
   if (i + n > text.Length()) break;
   chars.Insert(text.Substring(i, n));
   i += n;
  }
 }
 static bool IsRTL(string c, map<string,string> forms)
 {
  return forms.Contains(c) || c == "،" || c == "؟" || c == "؛";
 }
 static void Visual(string text, map<string,string> forms, array<string> result)
 {
  array<string> input = {}; Split(text, input);
  array<string> chars = {};
  // Tashkeel is omitted in this single-baseline atlas. Tatweel remains supported.
  string marks = "ًٌٍَُِّْٰ";
  bool rtl = false;
  foreach (string c : input)
  {
   if (marks.Contains(c) && c != "") continue;
   chars.Insert(c);
   if (IsRTL(c, forms)) rtl = true;
  }
  array<string> shaped = {};
  for (int i = 0; i < chars.Count(); i++)
  {
   string current = chars[i];
   if (!forms.Contains(current)) { shaped.Insert(current); continue; }
   array<string> f = {}; forms.Get(current).Split("|", f, false);
   bool prev = false; bool next = false;
   array<string> neighbour = {};
   if (i > 0 && forms.Contains(chars[i-1]))
   {
    forms.Get(chars[i-1]).Split("|", neighbour, false);
    prev = neighbour[0].Substring(1,1) == "1" && f[0].Substring(0,1) == "1";
   }
   neighbour.Clear();
   if (i+1 < chars.Count() && forms.Contains(chars[i+1]))
   {
    forms.Get(chars[i+1]).Split("|", neighbour, false);
    next = f[0].Substring(1,1) == "1" && neighbour[0].Substring(0,1) == "1";
   }
   // Lam-alef joins using the proper ligature instead of overlapping isolated glyphs.
   if (current == "ل" && i+1 < chars.Count())
   {
    string a = chars[i+1]; string lig;
    if (a == "آ") { if (prev) lig = "ﻶ"; else lig = "ﻵ"; }
    if (a == "أ") { if (prev) lig = "ﻸ"; else lig = "ﻷ"; }
    if (a == "إ") { if (prev) lig = "ﻺ"; else lig = "ﻹ"; }
    if (a == "ا") { if (prev) lig = "ﻼ"; else lig = "ﻻ"; }
    if (!lig.IsEmpty()) { shaped.Insert(lig); i++; continue; }
   }
   int index = 1;
   if (prev) index = 2;
   if (next) index = 3;
   if (prev && next) index = 4;
   shaped.Insert(f[index]);
  }
  if (!rtl) { result.Copy(shaped); return; }
  // RTL base direction, preserving Latin/numeric runs (including English word spaces).
  for (int j = shaped.Count()-1; j >= 0; j--)
  {
   if (chars.Count() == 0) break;
   string s = shaped[j];
   int b = s.ToAscii(0); if (b < 0) b += 256;
   bool ltr = (b < 128 && s != " ") || "٠١٢٣٤٥٦٧٨٩".Contains(s);
   if (!ltr) { result.Insert(s); continue; }
   int end = j;
   while (j > 0)
   {
    string p = shaped[j-1]; int pb = p.ToAscii(0); if (pb < 0) pb += 256;
    if (pb >= 128 && !"٠١٢٣٤٥٦٧٨٩".Contains(p)) break;
    j--;
   }
   for (int k = j; k <= end; k++) result.Insert(shaped[k]);
  }
 }
}
