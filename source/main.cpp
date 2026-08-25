#include "audio/HazeEngine.h"
#include "audio/HazeSample.h"
#include "modules/HazeLog.h"
#include "modules/HazeVersion.h"
#include "modules/core/server/HazeServer.h"
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
static void term_raw(void) {}
static void term_restore(void) {}
static int term_getkey(void) { return _kbhit() ? _getch() : -1; }
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
static struct termios term_orig;
static void term_raw(void) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, &term_orig);
  raw = term_orig;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}
static void term_restore(void) {
  fcntl(STDIN_FILENO, F_SETFL, 0);
  tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);
}
static int term_getkey(void) {
  char c;
  return read(STDIN_FILENO, &c, 1) == 1 ? c : -1;
}
#endif

typedef struct {
  bool headless;
} CliArgs;

void HelpMessage(void) {
  fputs("Usage: haze [options]\n"
        "\n"
        "Options:\n"
        "  --headless    Start Haze in headless mode (server only)\n"
        "  -h, --help    Show this message\n"
        "\n"
        "Examples:\n"
        "  haze --headless\n",
        stdout);
}

void VersionMessage(void) {
  fprintf(stdout, "haze %s\n", HAZE_VERSION_STR);
  return;
}

void PlayMusic(const char *music_path) {
  HazeSample s = {};
  if (!HazeSampleInitFromFile(&s, music_path)) {
    printf("fatal: could not play '%s'\n", music_path);
    return;
  }
  printf("Now playing: %s\n", HazeSampleGetName(&s));
  printf("Duration: %f\n", HazeSampleGetDuration(&s));
  printf("Sample Rate: %f\n", HazeSampleGetSampleRate(&s));

  HazeSamplePlay(&s);
  term_raw();

  bool playing = true;
  bool running = true;

  while (running) {
    float current = HazeSampleGetCursor(&s);
    float total = HazeSampleGetDuration(&s);
    float volume = HazeSampleGetVolume(&s);
    int bar_width = 30;
    int filled = total > 0 ? (int)(current / total * bar_width) : 0;

    printf("\033[2K\r");
    printf("  %s  \033[90m[\033[0m",
           playing ? "\033[32m▶\033[0m" : "\033[33m⏸\033[0m");
    for (int i = 0; i < bar_width; i++)
      printf(i < filled ? "\033[32m=\033[0m" : "\033[90m-\033[0m");
    printf("\033[90m]\033[0m");
    printf("  \033[36m%d:%02d\033[0m / \033[90m%d:%02d\033[0m",
           (int)current / 60, (int)current % 60, (int)total / 60,
           (int)total % 60);
    printf("  \033[90mspace=pause  q=quit\033[0m");
    fflush(stdout);

    int k = term_getkey();
    switch (k) {
    case ' ':
      playing = !playing;
      playing ? HazeSamplePlay(&s) : HazeSampleStop(&s);
      break;
    case 'q':
      running = false;
      break;
    case '+': {
      float volume = HazeSampleGetVolume(&s) + 0.1f;
      if (volume > 1.0f)
        volume = 1.0f;

      HazeSampleSetVolume(&s, volume);
      break;
    }

    case '-': {
      float volume = HazeSampleGetVolume(&s) - 0.1f;
      if (volume < 0.0f)
        volume = 0.0f;

      HazeSampleSetVolume(&s, volume);
      break;
    }
    }

    if (!HazeSampleIsPlaying(&s) && playing)
      running = false;

    SLEEP_MS(50);
  }

  printf("\n");
  term_restore();
  HazeSampleFree(&s);
}

CliArgs ParseArgs(int argc, char **argv) {
  CliArgs args;

  if (argc == 1) {
    args.headless = false;
  }
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      HelpMessage();
      break;
    }
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      VersionMessage();
      break;
    }
    if (strcmp(argv[i], "--headless") == 0) {
      args.headless = true;
    }
    if (strcmp(argv[i], "play") == 0) {
      if (!argv[i + 1]) {
        printf("fatal: what song?\n");
        printf("use: haze play <song_path>\n");
        break;
      }
      char *music_path = argv[i + 1];
      PlayMusic(music_path);
      break;
    }
  }
  return args;
}

int HeadlessMode(CliArgs args) {
  if (!args.headless)
    return 1;
  HazeLogInfo("Starting headless Haze...");
  HazeLogInfo("Starting Haze Server...");
  HazeServer *mainServer = HazeServerNew(NULL, 7192);

  if (mainServer) {
    HazeLogInfo("Haze Server started on %s:%d", HazeServerAddress(mainServer),
                HazeServerPort(mainServer));
    int err = HazeServerStart(mainServer);
    if (err != 0) {
      HazeLogError("Failed to start server: %s", uv_strerror(err));
      return 1;
    }
    HazeServerRun(mainServer); // bloqueia aqui
  }
  HazeLogInfo("Server process terminated.");

  return 0;
}

int main(int argc, char **argv) {

  HazeEngineInit();
  CliArgs args = ParseArgs(argc, argv);
  if (args.headless) {
    return HeadlessMode(args);
  }

  return 0;
}
