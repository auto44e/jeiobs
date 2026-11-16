// Throwaway test plugin: ask the real app to quit shortly after it finishes loading.
// At that moment it also logs how many obs-vst3-scanner children of OBS are
// running (-1 if /proc cannot be read), so a check can tell a quit mid-scan.
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QApplication>
#include <QTimer>
#include <QMainWindow>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
OBS_DECLARE_MODULE()
static int runningScanners() {
  DIR *proc = opendir("/proc");
  if (!proc) return -1;
  int n = 0;
  while (dirent *e = readdir(proc)) {
    if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
    const std::string dir = std::string("/proc/") + e->d_name;
    std::string stat, exe;
    if (!std::getline(std::ifstream(dir + "/stat"), stat)) continue;
    const size_t end = stat.rfind(')');  // "pid (comm) state ppid ..."
    if (end == std::string::npos) continue;
    std::istringstream rest(stat.substr(end + 1));
    char state = 0;
    long ppid = 0;
    if (!(rest >> state >> ppid) || ppid != (long)getpid() || state == 'Z') continue;
    std::getline(std::ifstream(dir + "/cmdline"), exe, '\0');  // argv[0]
    const std::string name = "obs-vst3-scanner";
    if (exe == name || (exe.size() > name.size() && exe.compare(exe.size() - name.size() - 1, std::string::npos, "/" + name) == 0))
      n++;
  }
  closedir(proc);
  return n;
}
bool obs_module_load(void) {
  obs_frontend_add_event_callback([](enum obs_frontend_event e, void *) {
    if (e != OBS_FRONTEND_EVENT_FINISHED_LOADING) return;
    // Normally quit 2 s after loading. With QUITPROBE_WAIT_SCANNER=1, quit as
    // soon as a scanner child is running (at least 0.5 s in), or after 10 s, so
    // a slow start cannot make the quit happen before the scan begins.
    const bool waitScanner = getenv("QUITPROBE_WAIT_SCANNER") != nullptr;
    auto *poll = new QTimer(qApp);
    auto *elapsedMs = new int(0);
    QObject::connect(poll, &QTimer::timeout, [poll, elapsedMs, waitScanner] {
      *elapsedMs += 100;
      const int running = runningScanners();
      const bool due = waitScanner ? ((*elapsedMs >= 500 && running > 0) || *elapsedMs >= 10000) : *elapsedMs >= 2000;
      if (!due) return;
      poll->stop();
      fprintf(stderr, "QUITPROBE requesting quit; obs-vst3-scanner children running: %d\n", running);
      auto *mw = static_cast<QMainWindow *>(obs_frontend_get_main_window());
      if (mw) mw->close();
      poll->deleteLater();
      delete elapsedMs;
    });
    poll->start(100);
  }, nullptr);
  return true;
}
