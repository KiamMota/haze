#include "Tempo.h"

uint64_t TempoMapToFrame(const TempoMap *map, uint32_t targetBar, uint32_t targetTick,
                          uint32_t sampleRate, uint32_t beatsPerBar) {
    uint64_t frame = 0;
    uint64_t lastBar = 0, lastTick = 0;
    float currentBpm = map->points[0].bpm;

    for (size_t i = 0; i < map->quantity; i++) {
        TempoPoint p = map->points[i];

        if (p.bar > targetBar || (p.bar == targetBar && p.tick > targetTick)) break;

        uint64_t deltaTicks = (p.bar - lastBar) * beatsPerBar * TICKS_PER_BEAT
                             + (int64_t)p.tick - (int64_t)lastTick;
        double framesPerTick = (60.0 * sampleRate) / (currentBpm * TICKS_PER_BEAT);
        frame += (uint64_t)(deltaTicks * framesPerTick);

        lastBar = p.bar; lastTick = p.tick;
        currentBpm = p.bpm;   
    }

    uint64_t deltaTicks = (targetBar - lastBar) * beatsPerBar * TICKS_PER_BEAT
                         + (int64_t)targetTick - (int64_t)lastTick;
    double framesPerTick = (60.0 * sampleRate) / (currentBpm * TICKS_PER_BEAT);
    frame += (uint64_t)(deltaTicks * framesPerTick);

    return frame;
}
