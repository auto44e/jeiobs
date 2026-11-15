// Throwaway test plugin: measure whether audio really passes through a VST3 filter.
// Plays a WAV (silence 0.5 s, then 1 kHz tone), taps the final mix and reports when
// the tone first appears. With ADelay (default: pure 1.0 s delay) it must arrive 1 s later.
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QTimer>
#include <QMainWindow>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
OBS_DECLARE_MODULE()
static std::atomic<uint64_t> g_frames{0};
static std::atomic<int64_t> g_onset{-1};
static std::atomic<bool> g_bad{false};
static float g_peak = 0;
static obs_source_t *g_src, *g_filter;
static void tap(void *, size_t, struct audio_data *a) {
  const float *l = (const float *)a->data[0];
  for (uint32_t i = 0; i < a->frames; i++) {
    float v = l[i];
    if (!std::isfinite(v)) g_bad = true;
    if (std::fabs(v) > g_peak) g_peak = std::fabs(v);
    if (g_onset < 0 && std::fabs(v) > 0.05f) g_onset = (int64_t)(g_frames + i);
  }
  g_frames += a->frames;
}
static void report() {
  const char *mode = getenv("DSP_FILTER");
  double sr = (double)audio_output_get_sample_rate(obs_get_audio());
  long long on = g_onset.load();
  fprintf(stderr, "DSPPROBE mode=%s sr=%.0f frames=%llu onset_s=%s%.4f peak=%.3f nonfinite=%d\n",
          mode ? mode : "none", sr, (unsigned long long)g_frames.load(), on < 0 ? "NONE " : "",
          on < 0 ? -1.0 : on / sr, g_peak, (int)g_bad.load());
  if (g_filter) {
    obs_data_t *st = obs_source_get_settings(g_filter);
    fprintf(stderr, "DSPPROBE filter_settings plugin=%s path=%s state_len=%zu\n",
            obs_data_get_string(st, "vst3_plugin"), obs_data_get_string(st, "vst3_path"),
            strlen(obs_data_get_string(st, "vst3_state")));
    obs_data_release(st);
  }
  obs_remove_raw_audio_callback(0, tap, nullptr);
  obs_set_output_source(10, nullptr);
  if (g_filter) { obs_source_filter_remove(g_src, g_filter); obs_source_release(g_filter); }
  obs_source_release(g_src);
  if (auto *mw = static_cast<QMainWindow *>(obs_frontend_get_main_window())) mw->close();
}
static void start() {
  obs_data_t *s = obs_data_create();
  obs_data_set_bool(s, "is_local_file", true);
  obs_data_set_string(s, "local_file", getenv("DSP_WAV"));
  obs_data_set_bool(s, "looping", false);
  g_src = obs_source_create("ffmpeg_source", "dsp-probe-src", s, nullptr);
  obs_data_release(s);
  if (const char *id = getenv("DSP_FILTER")) {
    obs_data_t *f = obs_data_create();
    obs_data_set_string(f, "vst3_plugin", id);
    obs_data_set_string(f, "vst3_path", getenv("DSP_PATH"));
    obs_data_set_string(f, "vst3_name", "probe");
    g_filter = obs_source_create("vst3_filter", "dsp-probe-vst3", f, nullptr);
    obs_data_release(f);
    obs_source_filter_add(g_src, g_filter);
  }
  obs_add_raw_audio_callback(0, nullptr, tap, nullptr);   // count frames from here
  obs_set_output_source(10, g_src);                       // make it audible in the mix
  fprintf(stderr, "DSPPROBE started filter=%s\n", g_filter ? "yes" : "no");
  QTimer::singleShot(5500, report);
}
bool obs_module_load(void) {
  obs_frontend_add_event_callback([](enum obs_frontend_event e, void *) {
    if (e == OBS_FRONTEND_EVENT_FINISHED_LOADING) QTimer::singleShot(1500, start);
  }, nullptr);
  return true;
}
