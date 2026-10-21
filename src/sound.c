// Tiny sound engine: music on channels 2 (melody) + 3 (wave bass),
// sound effects on channels 1 (square w/ sweep) + 4 (noise).
#include <gb/gb.h>
#include <gb/hardware.h>
#include "sound.h"

extern const uint16_t sq_freq[], wave_freq[];
extern const uint8_t mus_theme_mel[], mus_theme_bass[];
extern const uint8_t mus_fanfare_mel[], mus_fanfare_bass[];
extern const uint8_t mus_party_mel[], mus_party_bass[];
extern const uint8_t mus_lose_mel[], mus_lose_bass[];
extern const uint8_t mus_clear_mel[], mus_clear_bass[], mus_end_mel[], mus_end_bass[];

typedef struct { const uint8_t *start, *pos; uint8_t timer; uint8_t on; } Track;
static Track mel, bas;
static uint8_t mel_env = 0x63;
static uint8_t tempo = 8;
static uint8_t song_is_theme = 0;
void music_tempo(uint8_t t) { tempo = t; }
static int8_t transpose = 0;
void music_transpose(int8_t semis) { transpose = semis; }

// ---------------------------------------------------------------- SFX data
typedef struct { uint8_t dur, ch, r0, r1, r2, r3, r4; } Step;
#define LO(f) ((uint8_t)((f) & 0xFF))
#define HI(f) ((uint8_t)(0x80 | ((f) >> 8)))
#define HIL(f) ((uint8_t)(0xC0 | ((f) >> 8)))
#define END {0,0,0,0,0,0,0}

static const Step s_move[]    = {{3,1,0x00,0x80|0x38,0x31,LO(1780),HIL(1780)}, END};
static const Step s_drop[]    = {{40,1,0x3E,0x80,0x97,LO(1900),HI(1900)}, END};
static const Step s_grab[]    = {{4,4,0,0x00,0xC2,0x54,0x80}, {6,1,0x00,0x80,0x91,LO(1985),HI(1985)}, END};
static const Step s_rise[]    = {{40,1,0x35,0x40,0x87,LO(1300),HI(1300)}, END};
static const Step s_plop[]    = {{6,4,0,0x00,0xA1,0x71,0x80}, {10,1,0x1B,0x80,0xA2,LO(1600),HI(1600)}, END};
static const Step s_poof[]    = {{12,4,0,0x00,0x93,0x32,0x80}, END};
static const Step s_giggle[]  = {{4,1,0,0x80,0x81,LO(1899),HI(1899)}, {4,1,0,0x80,0x81,LO(1955),HI(1955)},
                                 {4,1,0,0x80,0x81,LO(1880),HI(1880)}, {4,1,0,0x80,0x81,LO(1960),HI(1960)},
                                 {4,1,0,0x80,0x81,LO(1900),HI(1900)}, {8,1,0,0x80,0x82,LO(1968),HI(1968)}, END};
static const Step s_miss[]    = {{14,1,0x1D,0x80,0x90,LO(1650),HI(1650)}, {24,1,0x1E,0x80,0x93,LO(1550),HI(1550)}, END};
static const Step s_boing[]   = {{8,1,0x16,0x40,0xB1,LO(1400),HI(1400)}, {16,1,0x1E,0x40,0xA2,LO(1800),HI(1800)}, END};
static const Step s_sparkle[] = {{4,1,0,0x40|0x30,0x41,LO(1985),HIL(1985)}, {4,1,0,0x40|0x30,0x31,LO(2003),HIL(2003)}, END};
static const Step s_start[]   = {{5,1,0,0x80,0xC1,LO(1650),HI(1650)}, {5,1,0,0x80,0xC1,LO(1798),HI(1798)},
                                 {5,1,0,0x80,0xC1,LO(1849),HI(1849)}, {16,1,0,0x80,0xC3,LO(1923),HI(1923)}, END};
static const Step s_nope[]    = {{8,1,0,0x00,0x81,LO(1100),HI(1100)}, END};
static const Step s_ding[]    = {{6,1,0,0x80,0xB1,LO(1923),HI(1923)}, {16,1,0,0x80,0xB3,LO(1985),HI(1985)}, END};

static const Step s_alarm[]   = {{6,1,0,0x80,0xC1,LO(1923),HI(1923)}, {6,1,0,0x80,0xC1,LO(1849),HI(1849)},
                                 {6,1,0,0x80,0xC1,LO(1923),HI(1923)}, {6,1,0,0x80,0xC1,LO(1849),HI(1849)}, END};
static const Step s_bonk[]    = {{3,4,0,0x00,0xF1,0x61,0x80}, {14,1,0x2C,0x80,0xF2,LO(1750),HI(1750)}, {12,1,0x1C,0x80,0xA2,LO(1850),HI(1850)}, END};
static const Step s_rasp[]    = {{30,4,0,0x00,0xA3,0x5F,0x80}, {10,1,0x00,0x00,0x83,LO(1000),HI(1000)}, END};
static const Step s_gold[]    = {{3,1,0,0x40,0xA1,LO(1923),HI(1923)}, {3,1,0,0x40,0xA1,LO(1955),HI(1955)},
                                 {3,1,0,0x40,0xA1,LO(1985),HI(1985)}, {3,1,0,0x40,0xA1,LO(2003),HI(2003)},
                                 {3,1,0,0x40,0xA1,LO(2016),HI(2016)}, {10,1,0,0x40,0xA2,LO(2024),HI(2024)}, END};
static const Step s_hop[]     = {{8,1,0x16,0x80,0x91,LO(1500),HI(1500)}, END};
static const Step s_shake[]   = {{20,4,0,0x00,0xC4,0x71,0x80}, END};

static const Step s_crack[]   = {{4,4,0,0x00,0xE1,0x44,0x80}, {6,4,0,0x00,0xB2,0x66,0x80}, {10,4,0,0x00,0x83,0x75,0x80}, END};
static const Step s_squeak[]  = {{3,1,0x00,0x80,0x91,LO(2010),HI(2010)}, {3,1,0x00,0x80,0x91,LO(2030),HI(2030)},
                                 {3,1,0x00,0x80,0x91,LO(2010),HI(2010)}, {5,1,0x00,0x80,0x92,LO(2030),HI(2030)}, END};
static const Step s_jet[]     = {{14,4,0,0x00,0x62,0x47,0x80}, END};
static const Step s_chime[]   = {{5,1,0,0x80,0xA1,LO(1985),HI(1985)}, {12,1,0,0x80,0xA3,LO(2024),HI(2024)}, END};
static const Step s_starpop[] = {{4,1,0x00,0x80,0xC1,LO(1849),HI(1849)}, {4,1,0,0x80,0xC1,LO(1955),HI(1955)}, {12,1,0,0x80,0xC3,LO(2003),HI(2003)}, END};
static const Step s_whoosh[]  = {{24,4,0,0x00,0x73,0x34,0x80}, {24,4,0,0x00,0x53,0x23,0x80}, END};

static const Step * const sfx_table[] = {
    s_move, s_drop, s_grab, s_rise, s_plop, s_poof, s_giggle, s_miss, s_boing, s_sparkle, s_start, s_nope, s_ding,
    s_alarm, s_bonk, s_rasp, s_gold, s_hop, s_shake, s_crack, s_squeak, s_jet, s_chime, s_starpop, s_whoosh
};

static const Step *sfx_pos;
static uint8_t sfx_timer, sfx_prio;

static void write_step(const Step *s) {
    if (s->ch == 1) {
        NR10_REG = s->r0; NR11_REG = s->r1; NR12_REG = s->r2; NR13_REG = s->r3; NR14_REG = s->r4;
    } else {
        NR41_REG = s->r1; NR42_REG = s->r2; NR43_REG = s->r3; NR44_REG = s->r4;
    }
}

void sfx_play(uint8_t id) {
    uint8_t prio = (id == SFX_MOVE) ? 0 : 1;
    if (sfx_pos && prio < sfx_prio) return;
    sfx_pos = sfx_table[id]; sfx_prio = prio; sfx_timer = 0;
}

// ---------------------------------------------------------------- music
static const uint8_t wave[16] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};

void sound_init(void) {
    uint8_t i;
    NR52_REG = 0x80; NR51_REG = 0xFF; NR50_REG = 0x77;
    NR30_REG = 0x00;
    for (i = 0; i < 16; i++) (*(volatile uint8_t *)(0xFF30 + i)) = wave[i];
    NR30_REG = 0x80;
    mel.on = bas.on = 0; sfx_pos = 0;
}

static void start_track(Track *t, const uint8_t *d) { t->start = t->pos = d; t->timer = 0; t->on = 1; }

void music_play(uint8_t song) {
    music_stop();
    if (song != MUS_THEME) tempo = 8;
    song_is_theme = (song == MUS_THEME);
    switch (song) {
        case MUS_THEME:   start_track(&mel, mus_theme_mel);   start_track(&bas, mus_theme_bass);   mel_env = 0x52; break;
        case MUS_TITLE:   start_track(&mel, mus_theme_mel);   start_track(&bas, mus_theme_bass);   mel_env = 0x73; break;
        case MUS_FANFARE: start_track(&mel, mus_fanfare_mel); start_track(&bas, mus_fanfare_bass); mel_env = 0xA4; break;
        case MUS_PARTY:   start_track(&mel, mus_party_mel);   start_track(&bas, mus_party_bass);   mel_env = 0xA4; break;
        case MUS_LOSE:    start_track(&mel, mus_lose_mel);    start_track(&bas, mus_lose_bass);    mel_env = 0x97; break;
        case MUS_CLEAR:   start_track(&mel, mus_clear_mel);   start_track(&bas, mus_clear_bass);   mel_env = 0xA4; break;
        case MUS_END:     start_track(&mel, mus_end_mel);     start_track(&bas, mus_end_bass);     mel_env = 0x83; break;
    }
}

void music_stop(void) {
    mel.on = bas.on = 0;
    NR22_REG = 0x00; NR24_REG = 0x80;
    NR32_REG = 0x00;
}

static void step_track(Track *t, uint8_t ch) {
    uint8_t n;
    if (!t->on) return;
    if (t->timer) { t->timer--; if (t->timer) return; }
    n = t->pos[0];
    if (n == 0xFF) {
        if (t->pos[1]) t->pos = t->start;
        else { t->on = 0; if (ch == 2) { NR22_REG = 0; NR24_REG = 0x80; } else NR32_REG = 0; return; }
        n = t->pos[0];
    }
    t->timer = (uint8_t)(((uint16_t)t->pos[1] * tempo) >> 3);
    if (!t->timer) t->timer = 1;
    t->pos += 2;
    if (n && (song_is_theme)) n = (uint8_t)(n + transpose);
    if (ch == 2) {
        if (n) {
            uint16_t f = sq_freq[n];
            NR21_REG = 0x80; NR22_REG = mel_env; NR23_REG = (uint8_t)f; NR24_REG = 0x80 | (uint8_t)(f >> 8);
        } else { NR22_REG = 0x00; NR24_REG = 0x80; }
    } else {
        if (n) {
            uint16_t f = wave_freq[n];
            NR30_REG = 0x80; NR32_REG = 0x40; NR33_REG = (uint8_t)f; NR34_REG = 0x80 | (uint8_t)(f >> 8);
        } else NR32_REG = 0x00;
    }
}

uint8_t music_playing(void) { return mel.on; }

void sound_update(void) {
    step_track(&mel, 2);
    step_track(&bas, 3);
    if (sfx_pos) {
        if (sfx_timer) sfx_timer--;
        if (!sfx_timer) {
            if (sfx_pos->dur == 0) { sfx_pos = 0; }
            else { write_step(sfx_pos); sfx_timer = sfx_pos->dur; sfx_pos++; }
        }
    }
}
