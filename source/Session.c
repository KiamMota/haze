#include "Session.h"
#include <corecrt_search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <urlmon.h>

#define NAME_MAX 15
static const char *session_name_random(void) {
  srand(time(NULL));
  const char *first[NAME_MAX] = {
      "goofball", "biscuit", "wombat", "noodle", "badger",
      "gizmo",    "doofus",  "gecko",  "rascal", "walrus",
      "dingo",    "muppet",  "turnip", "hobbit", "chimp",
  };

  const char *second[NAME_MAX] = {
      "flabby", "haughty",  "clumsy", "grumpy", "sneaky",
      "goofy",  "sluggish", "silly",  "cheeky", "wobbly",
      "cranky", "greedy",   "shabby", "lousy",  "spunky",
  };

  int first_v = rand() % NAME_MAX;
  int second_v = rand() % NAME_MAX;
  int tamanho = strlen(first[first_v]) + 1 + strlen(second[second_v]) + 1;

  char *name = malloc(tamanho);
  snprintf(name, tamanho, "%s_%s", first[first_v], second[second_v]);
  return name;
}

Session *SessionNew(const char *session_name) {
  Session *s = (Session *)malloc(sizeof(Session));
  s->working_time = time(NULL);
  s->session_name = malloc(sizeof(char *));
  // no project path yet.
  s->project_path = NULL;
  if (session_name == NULL) {
    s->session_name = strdup(session_name_random());
  } else {
    s->session_name = strdup(session_name);
  }
  return s;
}

bool SessionSetName(Session *s, const char *SessionName) {
  int max_name_len = 255;
  if (!s || !s->session_name || !SessionName) {
    return false;
  }

  if (strlen(SessionName) >= max_name_len) {
    return false;
  }

  snprintf(s->session_name, max_name_len, "%s", SessionName);

  return true;
}
