#ifndef SOUND_H
#define SOUND_H
#include <stdint.h>

enum { SFX_MOVE, SFX_DROP, SFX_GRAB, SFX_RISE, SFX_PLOP, SFX_POOF, SFX_GIGGLE, SFX_MISS,
       SFX_BOING, SFX_SPARKLE, SFX_START, SFX_NOPE, SFX_DING,
       SFX_ALARM, SFX_BONK, SFX_RASP, SFX_GOLD, SFX_HOP, SFX_SHAKE,
       SFX_CRACK, SFX_SQUEAK, SFX_JET, SFX_CHIME, SFX_STARPOP, SFX_WHOOSH };
enum { MUS_THEME, MUS_TITLE, MUS_FANFARE, MUS_PARTY, MUS_LOSE, MUS_CLEAR, MUS_END };

void sound_init(void);
void sound_update(void);
void sfx_play(uint8_t id);
void music_play(uint8_t song);
void music_stop(void);
uint8_t music_playing(void);
void music_tempo(uint8_t t);   // 8 = normal, lower = faster
void music_transpose(int8_t semis);

#endif
