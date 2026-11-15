// Throwaway test plugin: ask the real app to quit shortly after it finishes loading.
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QApplication>
#include <QTimer>
#include <QMainWindow>
#include <cstdio>
OBS_DECLARE_MODULE()
bool obs_module_load(void) {
  obs_frontend_add_event_callback([](enum obs_frontend_event e, void *) {
    if (e == OBS_FRONTEND_EVENT_FINISHED_LOADING)
      QTimer::singleShot(2000, [] {
        fprintf(stderr, "QUITPROBE requesting quit\n");
        auto *mw = static_cast<QMainWindow *>(obs_frontend_get_main_window());
        if (mw) mw->close();
      });
  }, nullptr);
  return true;
}
