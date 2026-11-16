// Fixture sanity check: parse a moduleinfo.json with the SDK parser that the
// scanner uses and print what the scanner would see in it.
#include <pluginterfaces/base/ipluginbase.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <public.sdk/source/vst/moduleinfo/moduleinfoparser.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
int main(int argc, char **argv) {
  if (argc != 2) return 2;
  std::ifstream in(argv[1]);
  std::stringstream text;
  text << in.rdbuf();
  auto info = Steinberg::ModuleInfoLib::parseJson(text.str(), &std::cerr);
  if (!info) return 1;
  int audio = 0;
  for (const auto &c : info->classes) audio += c.category == kVstAudioEffectClass;
  printf("classes=%zu audio=%d discardable=%d\n", info->classes.size(), audio,
         (info->factoryInfo.flags & Steinberg::PFactoryInfo::kClassesDiscardable) ? 1 : 0);
  return 0;
}
