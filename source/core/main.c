#include "HazeLog.h"
#include "HazeVersion.h"
#include "core/audio/HazeEngine.h"
#include "core/audio/HazeSample.h"
#include "core/server/HazeServer.h"
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
#define SLEEP_MS(ms) sleep((ms) * 1000)
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

typedef enum {
  MODE_HEADLESS,
  MODE_PLAY,
  MODE_HELP,
  MODE_VERSION
} ExecutionMode;

typedef struct {
  ExecutionMode mode;
  const char *music_path;
} CliArgs;

void HelpMessage(void) {
  fputs("Usage: haze [command|options]\n"
        "\n"
        "Commands:\n"
        "  play <file>    Play an audio file\n"
        "\n"
        "Options:\n"
        "  -v, --version  Show version information\n"
        "  -h, --help     Show this message\n"
        "\n"
        "Examples:\n"
        "  haze\n"
        "  haze play song.mp3\n",
        stdout);
}

void VersionMessage(void) { fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); }

void PlayMusic(const char *music_path) {
  HazeSample s = {};
  if (!HazeSampleInitFromFile(&s, music_path)) {
    printf("fatal: could not play '%s'\n", music_path);
    return;
  }
  printf("Now playing: %s\n", HazeSampleGetName(&s));
  printf("Duration: %.2f s\n", HazeSampleGetDuration(&s));
  printf("Sample Rate: %.0f Hz\n", HazeSampleGetSampleRate(&s));

  HazeSamplePlay(&s);
  term_raw();

  bool playing = true;
  bool running = true;

  while (running) {
    float current = HazeSampleGetCursor(&s);
    float total = HazeSampleGetDuration(&s);
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
  // Padrão explicitamente definido como MODE_HEADLESS
  CliArgs args = {.mode = MODE_HEADLESS, .music_path = NULL};

  if (argc == 1) {
    return args;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      args.mode = MODE_HELP;
      return args;
    }
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      args.mode = MODE_VERSION;
      return args;
    }
    if (strcmp(argv[i], "play") == 0) {
      if (i + 1 < argc) {
        args.mode = MODE_PLAY;
        args.music_path = argv[i + 1];
      } else {
        printf(
            "fatal: missing audio file path.\nUsage: haze play <song_path>\n");
        args.mode = MODE_HELP;
      }
      return args;
    }
  }

  return args;
}

int HeadlessMode(void) {
  HazeLogInfo("Starting headless Haze...");
  HazeLogInfo("Starting Haze Server...");

  int port = 7192;
  HazeServer *mainServer = NULL;

  while (1) {
    mainServer = HazeServerNew(NULL, port);

    if (!mainServer) {
      HazeLogError("Failed to create Haze Server instance.");
      return 1;
    }

    int err = HazeServerStart(mainServer);

    // Se falhar por porta indisponível, limpa e tenta a próxima
    if (err == UV_EADDRINUSE || err == UV_EACCES) {
      HazeLogInfo("Port %d is unavailable (%s), trying %d...", 
                  port, uv_strerror(err), port + 1);

      HazeServerFree(&mainServer);
      port++;
      continue;
    }

    // Se ocorrei QUALQUER outro erro, encerra a execução
    if (err != 0) {
      HazeLogError("Failed to start server: %s", uv_strerror(err));
      HazeServerFree(&mainServer);
      return 1;
    }

    // Se err == 0, o servidor iniciou com SUCESSO. Registra o log e sai do loop.
    HazeLogInfo("Haze Server started on %s:%d",
                HazeServerAddress(mainServer),
                HazeServerPort(mainServer));
    break; 
  }

  // Executa o loop bloqueante do libuv
  HazeServerRun(mainServer);

  HazeLogInfo("Server process terminated.");
  HazeServerFree(&mainServer);

  return 0;
}
int main(int argc, char **argv) {
  CliArgs args = ParseArgs(argc, argv);

  // Comandos informativos saem direto sem tocar no audio engine
  if (args.mode == MODE_VERSION) {
    VersionMessage();
    return 0;
  }
  if (args.mode == MODE_HELP) {
    HelpMessage();
    return 0;
  }

  // Inicializa o engine de áudio para comandos operacionais (HEADLESS ou PLAY)
  HazeLogInfo("starting audio engine...");
  if (!HazeEngineInit()) {
    HazeLogError("failed to start audio engine.");
    return 1;
  }

  if (args.mode == MODE_PLAY && args.music_path) {
    PlayMusic(args.music_path);
    return 0;
  }

  // Comportamento Padrão (Sem argumentos)
  return HeadlessMode();
}
