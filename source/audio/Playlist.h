#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "audio/Channel.h"
#include "audio/ResultAudio.h"
#include "audio/Sample.h"

typedef struct {
    uint64_t id;
    Sample   *sample;        // referência ao recurso compartilhado (não dono)
    Channel  *channel;       // decidido: mora aqui, não na linha
    uint32_t  bar;           // compasso, 0-indexed
    uint32_t  tick;          // posição dentro do compasso, 0..(TICKS_PER_BEAT * beats_per_bar - 1)
    uint32_t  length_ticks;  
    uint16_t  row;           // puramente visual, não afeta áudio
    ma_sound  instance;     
    bool      armed;
} ClipInstance;


typedef struct {
    ClipInstance **clips;
    size_t quantity;
} Playlist;

Playlist* PlaylistNew(void);
void PlaylistFree(Playlist** p);
ResultAudio PlaylistAddSample(Playlist *p, Sample *sample, Channel *channel,
                          uint32_t bar, uint32_t tick, uint32_t length_ticks,
                          uint16_t row, uint64_t *out_id);

#endif
