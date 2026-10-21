// SNEAKY KRYPTID'S SPOOKY CLAW - Redcap Trouble
// A haunted claw-machine race for Game Boy Color: 4 worlds x 2 levels vs. Lil Redcap.
// Built with GBDK-2020 (64 KB, MBC5, optional battery save).
#include <gb/gb.h>
#include <gb/cgb.h>
#include <rand.h>
#include <string.h>
#pragma bank 1
#include "assets.h"
#include "sound.h"
#include "loader.h"
#define BK_A 3   // bank of assets1 (common, sprites, title, rays, world 1)
#define BK_B 2   // bank of assets2 (worlds 2-4, bats, space redcap, flag, ship)

// ------------------------------------------------------------------ constants
#define ST_TITLE  0
#define ST_CARD   1
#define ST_PLAY   2
#define ST_LOSE   3
#define ST_STARS  4
#define ST_FINAL  5

#define P_READY   0
#define P_IDLE    1
#define P_DROP    2
#define P_GRAB    3
#define P_RISE    4
#define P_SLIP    5
#define P_CARRY   6
#define P_RELEASE 7
#define P_FLY     8
#define P_REFILL  9
#define P_STEAL   10
#define P_SCOOT   11
#define P_SNEAK   12
#define P_SHOO    13
#define P_TAKE    14
#define P_DONE    15
#define P_CRACK   16

#define M_NONE  0
#define M_SNEAK 1
#define M_SCOOT 2

#define CLAW_MINX 20
#define CLAW_MAXX 144
#define DROP_MINX 34
#define HOME_Y    32
#define GRAB_Y    106
#define CHUTE_X   20
#define EMPTY     0xFF
#define N_SLOTS   5
#define N_IN_MACHINE 4
#define N_PRIZES  8
#define N_LEVELS  8
#define WIN_SCORE 5
#define RC_FLOOR_Y 96
#define RC_TOP_Y   6
#define PRIZE_Y   112

// OAM layout (8x16 sprites)
#define O_CARR   0
#define O_CLAW   2
#define O_CABLE  4
#define N_CABLE  5
#define O_HELD   9
#define O_FLY    11
#define O_RCO    13
#define O_RC     15
#define O_HAT    17
#define O_BUB    19
#define O_ARROW  20
#define O_SPARK  22
#define O_BAT    24
#define O_PRZ    28

static const uint8_t slot_col[N_SLOTS] = {5, 8, 11, 14, 17};
#define SLOT_ROW 14
static const int8_t bob[8] = {0, -1, -2, -3, -3, -2, -1, 0};
static const int8_t wob[16] = {0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1};

// ---- difficulty per level (0..7): world = level>>1
static const uint8_t lv_window[N_LEVELS]  = {8, 7, 7, 6, 6, 6, 5, 5};
static const uint8_t lv_vmax[N_LEVELS]    = {16, 16, 18, 18, 20, 20, 22, 22};
static const uint8_t lv_accel[N_LEVELS]   = {16, 16, 5, 5, 4, 4, 3, 3};
static const uint8_t lv_decel[N_LEVELS]   = {16, 16, 5, 5, 4, 4, 3, 2};
static const uint8_t lv_misfreq[N_LEVELS] = {99, 4, 3, 2, 3, 2, 2, 2};
static const uint8_t lv_sneak[N_LEVELS]   = {200, 200, 180, 150, 150, 130, 120, 100};
static const uint8_t lv_jiggle[N_LEVELS]  = {0, 0, 0, 0, 0, 3, 4, 5};
static const uint8_t lv_stone[N_LEVELS]   = {0, 0, 2, 4, 0, 0, 0, 0};    // refill chance /8
static const uint8_t lv_bats[N_LEVELS]    = {0, 0, 0, 0, 1, 2, 0, 0};
static const uint8_t lv_batspd[N_LEVELS]  = {0, 0, 0, 0, 12, 18, 0, 0};  // 1/16 px
static const uint8_t lv_drift[N_LEVELS]   = {0, 0, 0, 0, 0, 0, 5, 7};
static const int8_t  world_transpose[4]   = {0, -12, 5, 12};
#define SCOOT_ON  (level >= 1)
#define SNEAK_ON  (level >= 3)
#define JIGGLE_ON (level >= 5)
#define SPACE     (level >= 6)
#define SPACE_RC  (level == 7)

// ------------------------------------------------------------------ state
static uint8_t state, frame, sub;
static uint8_t keys, prev_keys, pressed;
static uint8_t collected[N_PRIZES];
static uint8_t level, assist, losses_in_row;
static uint8_t level_stars[N_LEVELS];
static uint8_t slot_type[N_SLOTS], slot_stone[N_SLOTS];
static uint8_t pscore, rscore, shoos;
static uint8_t sack_type[WIN_SCORE], rc_loot[WIN_SCORE];

static uint8_t pstate, ptimer;
static uint8_t clawX, clawY, claw_closed;
static uint16_t claw_fx; static int16_t claw_vx; static int8_t drift_cur;
static int8_t target_slot;
static uint8_t held_type, grabbed_slot, jiggle, jiggle_last, broke_stone, knocked;
static uint16_t fall_y, fall_vy;
static uint8_t idle_frames, still_frames, moving_tick;
static uint8_t golden, golden_last, turns, mis_count, mis_pending, sneak_delay;
static uint8_t claw_pal_cur = 0xFF;
static uint8_t shake_t;

static uint8_t rc_on, rc_frame, rc_flip, rc_x, rc_y, rc_hat;
static uint16_t rc_fx; static uint8_t rc_target, rc_phase, rc_t, rc_slot, rc_dest;

static uint8_t fly_type, fly_t, fly_x0, fly_y0, fly_x1, fly_y1, fly_kind;
static int8_t poof_slot; static uint8_t poof_t;

static uint16_t bat_fx[2]; static uint8_t bat_y[2], bat_dir[2];

static uint8_t scr_timer, stars_earned;
static uint8_t spark_x[6], spark_y[6], spark_t[6];
static palette_color_t pal_work[4];
static uint8_t music_muted;

// ------------------------------------------------------------------ helpers
static void read_keys(void) { prev_keys = keys; keys = joypad(); pressed = keys & ~prev_keys; }
static void hide_all(void) { uint8_t i; for (i = 0; i < 40; i++) hide_sprite(i); }
static uint8_t abs_diff(uint8_t a, uint8_t b) { return a > b ? a - b : b - a; }
static uint8_t world(void) { return level >> 1; }

static void spr16(uint8_t oam, uint8_t tile, uint8_t x, uint8_t y, uint8_t prop) {
    set_sprite_tile(oam, tile); set_sprite_prop(oam, prop); move_sprite(oam, x + 8, y + 16);
}
static void meta16(uint8_t oam, uint8_t tile, uint8_t x, uint8_t y, uint8_t prop) {
    spr16(oam, tile, x, y, prop); spr16(oam + 1, tile + 2, x + 8, y, prop);
}
static void meta16_flip(uint8_t oam, uint8_t tile, uint8_t x, uint8_t y, uint8_t prop) {
    prop |= S_FLIPX;
    spr16(oam, tile + 2, x, y, prop); spr16(oam + 1, tile, x + 8, y, prop);
}
static void hide16(uint8_t oam) { hide_sprite(oam); hide_sprite(oam + 1); }

static void draw_prize_spr(uint8_t oam, uint8_t type, uint8_t x, uint8_t y) {
    meta16(oam, SPR_PRIZE + (type << 2), x, y, prize_pal[type]);
}

static void rc_draw_at(uint8_t oam_over, uint8_t oam_base, uint8_t fr, uint8_t x, uint8_t y, uint8_t flip) {
    uint8_t t = SPR_RC + (fr << 3);
    if (flip) { meta16_flip(oam_over, t + 4, x, y, 3); meta16_flip(oam_base, t, x, y, 2); }
    else      { meta16(oam_over, t + 4, x, y, 3);      meta16(oam_base, t, x, y, 2); }
}

static void rc_show(void) {
    if (!rc_on) { hide16(O_RCO); hide16(O_RC); hide16(O_HAT); return; }
    rc_draw_at(O_RCO, O_RC, rc_frame, rc_x, rc_y, rc_flip);
    if (rc_hat != EMPTY) draw_prize_spr(O_HAT, rc_hat, rc_x, rc_y - 13);
    else hide16(O_HAT);
}

static uint8_t char_tile(char c) {
    uint8_t i;
    if (c == ' ') return 0;
    if (c == '*') return STAR_T;
    for (i = 0; font_order[i]; i++) if (font_order[i] == c) return FONT_BASE + i;
    return 0;
}

static void draw_text(uint8_t x, uint8_t y, const char *s, uint8_t pal) {
    while (*s) { set_bkg_tile_xy(x, y, char_tile(*s)); set_bkg_attribute_xy(x, y, pal); x++; s++; }
}
static void draw_text_center(uint8_t y, const char *s, uint8_t pal) { draw_text((20 - (uint8_t)strlen(s)) >> 1, y, s, pal); }

static void draw_2x2(uint8_t col, uint8_t row, uint8_t t, uint8_t pal) {
    set_bkg_tile_xy(col, row, t);             set_bkg_attribute_xy(col, row, pal);
    set_bkg_tile_xy(col + 1, row, t + 1);     set_bkg_attribute_xy(col + 1, row, pal);
    set_bkg_tile_xy(col, row + 1, t + 2);     set_bkg_attribute_xy(col, row + 1, pal);
    set_bkg_tile_xy(col + 1, row + 1, t + 3); set_bkg_attribute_xy(col + 1, row + 1, pal);
}
static void draw_prize_bg(uint8_t col, uint8_t row, uint8_t type, uint8_t pal) { draw_2x2(col, row, PRIZE_BG_BASE + (type << 2), pal); }

static void restore_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t wd = world();
    ld_submap(world_bank[wd], x, y, w, h, world_map[wd], world_attr[wd]);
}

static void draw_slot(uint8_t s) {
    if (slot_type[s] == EMPTY || SPACE) restore_rect(slot_col[s], SLOT_ROW, 2, 2);
    else if (slot_stone[s]) draw_2x2(slot_col[s], SLOT_ROW, TOMB_T, 3);
    else draw_prize_bg(slot_col[s], SLOT_ROW, slot_type[s], prize_pal[slot_type[s]]);
}

static void draw_sack(void) {
    uint8_t i, x;
    for (i = 0; i < WIN_SCORE; i++) {
        x = 1 + (i << 1);
        if (i < pscore) draw_prize_bg(x, 0, sack_type[i], prize_pal[sack_type[i]]);
        else draw_2x2(x, 0, SOCKET_T, 0);
    }
}

static void draw_pips(void) {
    uint8_t i, a;
    for (i = 0; i < WIN_SCORE; i++) {
        a = (i < rscore) ? 2 : 0;
        set_bkg_attribute_xy(12 + i, 0, a); set_bkg_attribute_xy(12 + i, 1, a);
    }
}

static void draw_level_tag(void) {   // bottom right: "W-L"
    set_bkg_tile_xy(15, 17, char_tile('W')); set_bkg_attribute_xy(15, 17, 1);
    set_bkg_tile_xy(16, 17, char_tile('1' + world())); set_bkg_attribute_xy(16, 17, 1);
    set_bkg_tile_xy(17, 17, char_tile('-')); set_bkg_attribute_xy(17, 17, 1);
    set_bkg_tile_xy(18, 17, char_tile('1' + (level & 1))); set_bkg_attribute_xy(18, 17, 1);
}

static uint8_t slot_cx(uint8_t s) { return (slot_col[s] << 3) + 8; }
static uint8_t sack_x(uint8_t i) { return 8 + (i << 4); }
static uint8_t pip_x(uint8_t i)  { return 96 + (i << 3); }

static uint8_t in_machine(uint8_t type) {
    uint8_t i, n = 0;
    for (i = 0; i < N_SLOTS; i++) if (slot_type[i] == type) n++;
    return n;
}
static uint8_t pick_prize(void) {
    uint8_t cand[N_PRIZES], n = 0, i;
    if ((rand() & 3) != 0) for (i = 0; i < N_PRIZES; i++) if (!collected[i] && !in_machine(i)) cand[n++] = i;
    if (!n) for (i = 0; i < N_PRIZES; i++) if (!in_machine(i)) cand[n++] = i;
    if (!n) return rand() & 7;
    return cand[rand() % n];
}
static uint8_t count_in_machine(void) {
    uint8_t i, n = 0;
    for (i = 0; i < N_SLOTS; i++) if (slot_type[i] != EMPTY) n++;
    return n;
}
static uint8_t count_stones(void) {
    uint8_t i, n = 0;
    for (i = 0; i < N_SLOTS; i++) if (slot_type[i] != EMPTY && slot_stone[i]) n++;
    return n;
}
static int8_t random_slot(uint8_t want_empty, int8_t avoid) {
    uint8_t cand[N_SLOTS], n = 0, i;
    for (i = 0; i < N_SLOTS; i++)
        if ((int8_t)i != avoid && ((slot_type[i] == EMPTY) == want_empty)) cand[n++] = i;
    if (!n) return -1;
    return cand[rand() % n];
}
static int8_t slot_under(uint8_t x, uint8_t w) {
    uint8_t i, d, best = 0xFF; int8_t r = -1;
    for (i = 0; i < N_SLOTS; i++) {
        if (slot_type[i] == EMPTY) continue;
        d = abs_diff(x, slot_cx(i));
        if (d <= w && d < best) { best = d; r = i; }
    }
    return r;
}
static uint8_t grab_window(void) { return lv_window[level] + assist + (golden ? 3 : 0); }

static void set_claw_pal(uint8_t p) {
    if (p != claw_pal_cur) { claw_pal_cur = p; set_sprite_palette(0, 1, pal_claw + (p << 2)); }
}

static void fill_slot(uint8_t s) {   // refill with a prize, maybe under a tombstone
    slot_type[s] = pick_prize();
    slot_stone[s] = (lv_stone[level] && count_stones() < 2 && (rand() & 7) < lv_stone[level]);
}

// ------------------------------------------------------------------ save (battery RAM, optional)
#define SAVE_MAGIC0 'S'
#define SAVE_MAGIC1 'K'
#define SAVE_VER    3
static uint8_t *const sram = (uint8_t *)0xA000;

static uint8_t save_sum(void) {
    uint8_t i, s = 0;
    for (i = 3; i < 13; i++) s += sram[i];
    return (uint8_t)(s ^ 0x5A);
}
static void save_write(void) {
    uint8_t i;
    ENABLE_RAM; SWITCH_RAM(0);
    sram[0] = SAVE_MAGIC0; sram[1] = SAVE_MAGIC1; sram[2] = SAVE_VER; sram[3] = level;
    for (i = 0; i < N_LEVELS; i++) sram[4 + i] = level_stars[i];
    sram[12] = 0;
    for (i = 0; i < N_PRIZES; i++) if (collected[i]) sram[12] |= (1 << i);
    sram[13] = save_sum();
    DISABLE_RAM;
}
static uint8_t save_load(void) {
    uint8_t i, ok;
    ENABLE_RAM; SWITCH_RAM(0);
    ok = (sram[0] == SAVE_MAGIC0 && sram[1] == SAVE_MAGIC1 && sram[2] == SAVE_VER && sram[13] == save_sum() && sram[3] < N_LEVELS);
    if (ok) {
        level = sram[3];
        for (i = 0; i < N_LEVELS; i++) level_stars[i] = sram[4 + i] > 5 ? 5 : sram[4 + i];
        for (i = 0; i < N_PRIZES; i++) collected[i] = (sram[12] >> i) & 1;
    }
    DISABLE_RAM;
    return ok;
}
static void save_clear(void) {
    ENABLE_RAM; SWITCH_RAM(0); sram[0] = 0; sram[1] = 0; DISABLE_RAM;
}
static void new_game(void) {
    uint8_t i;
    level = 0; assist = 0; losses_in_row = 0;
    for (i = 0; i < N_LEVELS; i++) level_stars[i] = 0;
    for (i = 0; i < N_PRIZES; i++) collected[i] = 0;
}
static uint8_t total_stars(void) {
    uint8_t i, t = 0;
    for (i = 0; i < N_LEVELS; i++) t += level_stars[i];
    return t;
}

// ------------------------------------------------------------------ sparks
static void spark_at(uint8_t i, uint8_t x, uint8_t y) { spark_x[i] = x; spark_y[i] = y; spark_t[i] = 1; }
static void update_play_sparks(void) {
    uint8_t i;
    for (i = 0; i < 2; i++) {
        if (spark_t[i]) {
            spark_t[i]++;
            if (spark_t[i] > 24) { spark_t[i] = 0; hide_sprite(O_SPARK + i); continue; }
            spr16(O_SPARK + i, (spark_t[i] & 4) ? SPR_SPARK_A : SPR_SPARK_B, spark_x[i], spark_y[i], 1);
        }
    }
}
static void spark_respawn(uint8_t i) {
    uint8_t r;
    do {
        spark_x[i] = 24 + (rand() % 112); spark_y[i] = 28 + (rand() % 92);
        r = (spark_x[i] > 40 && spark_x[i] < 120 && spark_y[i] > 30 && spark_y[i] < 110);
    } while (r);
    spark_t[i] = rand() & 15;
}
static void update_sparks(uint8_t oam, uint8_t n) {
    uint8_t i;
    for (i = 0; i < n; i++) {
        spark_t[i]++;
        if (spark_t[i] > 40) spark_respawn(i);
        if (spark_t[i] < 32) spr16(oam + i, (spark_t[i] & 8) ? SPR_SPARK_A : SPR_SPARK_B, spark_x[i], spark_y[i], 1);
        else hide_sprite(oam + i);
    }
}

// ------------------------------------------------------------------ screen loading
static void load_common(void) {
    ld_bkg_data(BK_A, 0, N_COMMON_TILES, common_tiles);
    ld_spr_data(BK_A, 0, N_SPRITE_TILES, sprite_tiles);
    set_sprite_palette(0, 8, pal_obj);
    claw_pal_cur = 0xFF;
}
static void load_rays_bg(void) {
    uint8_t i;
    move_bkg(0, 0);
    ld_bkg_data(BK_A, SCREEN_BASE, N_REVEAL_TILES, reveal_tiles);
    ld_screen(BK_A, reveal_map, reveal_attr);
    set_bkg_palette(0, 8, pal_bg_world[0]);
    set_bkg_palette(0, 1, pal_rays);
    set_bkg_palette(1, 1, pal_text);
    set_bkg_palette(2, 1, pal_star_dim);
    for (i = 0; i < 6; i++) spark_respawn(i);
}
static void cycle_rays(void) {
    if ((frame & 7) == 0) {
        uint8_t k = (frame >> 3) % 3;
        if (state == ST_LOSE) k = (frame >> 4) % 3;
        pal_work[0] = pal_rays[0];
        pal_work[1] = pal_rays[1 + k];
        pal_work[2] = pal_rays[1 + (k + 1) % 3];
        pal_work[3] = pal_rays[1 + (k + 2) % 3];
        set_bkg_palette(0, 1, pal_work);
    }
}
static void load_normal_rc_tiles(void) { ld_spr_data(BK_A, SPR_RC, 48, sprite_tiles + (SPR_RC << 4)); }

// ------------------------------------------------------------------ title
static void enter_title(void) {
    uint8_t i;
    DISPLAY_OFF;
    hide_all();
    state = ST_TITLE; scr_timer = 0;
    load_normal_rc_tiles();
    ld_bkg_data(BK_A, SCREEN_BASE, N_TITLE_TILES, title_tiles);
    ld_screen(BK_A, title_map, title_attr);
    set_bkg_palette(0, 8, pal_bg_world[0]);
    set_bkg_palette(1, 1, pal_text);
    draw_text_center(0, "SNEAKY KRYPTID'S", 1);
    draw_text_center(8, "REDCAP TROUBLE", 1);
    for (i = 0; i < N_PRIZES; i++) draw_prize_bg(2 + (i << 1), 10, i, collected[i] ? prize_pal[i] : 0);
    if (level > 0) {
        static char ct[] = "CONTINUE W1-1";
        ct[10] = '1' + world(); ct[12] = '1' + (level & 1);
        draw_text_center(13, ct, 1);
    }
    draw_text_center(15, "A GAME BY", 1);
    draw_text_center(16, "ANTHONY DEGEORGE", 1);
    DISPLAY_ON;
    if (!music_muted) { music_transpose(0); music_play(MUS_TITLE); }
}
static void update_title(void) {
    scr_timer++;
    rc_draw_at(O_RCO, O_RC, ((frame >> 4) & 1) ? RC_GIGGLE : RC_IDLE, 124, 40 + (((frame >> 4) & 1) ? -2 : 0), 1);
    if ((scr_timer & 31) == 0) draw_text(6, 12, "PRESS A", 1);
    else if ((scr_timer & 31) == 22) draw_text(6, 12, "       ", 1);
}

// ------------------------------------------------------------------ play
static void draw_claw(void) {
    uint8_t i, n, x = clawX - 8;
    meta16(O_CARR, SPR_CARRIAGE, x, 24, 0);
    meta16(O_CLAW, claw_closed ? SPR_CLAW_CLOSED : SPR_CLAW_OPEN, x, clawY, 0);
    n = (clawY - HOME_Y + 15) >> 4;
    if (n > N_CABLE) n = N_CABLE;
    for (i = 0; i < N_CABLE; i++) {
        if (i < n) spr16(O_CABLE + i, SPR_CABLE, clawX - 4, HOME_Y + (i << 4), 0);
        else hide_sprite(O_CABLE + i);
    }
}
static void place_held(uint8_t x, uint8_t y, uint8_t behind) {
    meta16(O_HELD, SPR_PRIZE + (held_type << 2), x, y, prize_pal[held_type] | (behind ? S_PRIORITY : 0));
}
static void update_tempo(void) {
    uint8_t m = pscore > rscore ? pscore : rscore;
    music_tempo(m >= 4 ? 5 : (m >= 3 ? 6 : 8));
}
static void bats_init(void) {
    uint8_t i;
    for (i = 0; i < 2; i++) { bat_fx[i] = (uint16_t)(40 + i * 70) << 4; bat_y[i] = 52 + i * 14; bat_dir[i] = i; }
}
static void update_bats(void) {
    uint8_t i, n = lv_bats[level], x, y;
    for (i = 0; i < 2; i++) {
        if (i >= n || state != ST_PLAY) { hide16(O_BAT + (i << 1)); continue; }
        {
            uint8_t sp = lv_batspd[level] - (assist << 1);     // after a loss the bats slow down too
            if (bat_dir[i]) { bat_fx[i] += sp; if ((bat_fx[i] >> 4) >= 132) bat_dir[i] = 0; }
            else            { bat_fx[i] -= sp; if ((bat_fx[i] >> 4) <= 22)  bat_dir[i] = 1; }
        }
        x = bat_fx[i] >> 4; y = bat_y[i] + wob[((frame >> 2) + (i << 3)) & 15];
        {
            uint8_t t = SPR_SWAP + (((frame >> 3) & 1) ? 4 : 0);
            if (bat_dir[i]) { spr16(O_BAT + (i << 1), t + 2, x, y, 6 | S_FLIPX); spr16(O_BAT + (i << 1) + 1, t, x + 8, y, 6 | S_FLIPX); }
            else            { spr16(O_BAT + (i << 1), t, x, y, 6);              spr16(O_BAT + (i << 1) + 1, t + 2, x + 8, y, 6); }
        }
        // knock the dropping claw sideways
        if (pstate == P_DROP && !knocked && abs_diff(x + 8, clawX) < 12 && abs_diff(y, clawY) < 12) {
            knocked = 1; target_slot = -1; shake_t = 6;
            if (bat_dir[i]) { clawX = (clawX + 8 > CLAW_MAXX) ? CLAW_MAXX : clawX + 8; }
            else            { clawX = (clawX < CLAW_MINX + 8) ? CLAW_MINX : clawX - 8; }
            sfx_play(SFX_SQUEAK);
        }
    }
}
static void update_space_prizes(void) {
    uint8_t s;
    for (s = 0; s < N_SLOTS; s++) {
        if (!SPACE || slot_type[s] == EMPTY || state != ST_PLAY) { hide16(O_PRZ + (s << 1)); continue; }
        draw_prize_spr(O_PRZ + (s << 1), slot_type[s], slot_cx(s) - 8, PRIZE_Y + bob[((frame >> 3) + (s << 1)) & 7]);
    }
}

static void new_round(void) {
    uint8_t i, s;
    pscore = rscore = 0; shoos = 0; turns = 0; mis_count = 0; mis_pending = M_NONE;
    golden = golden_last = 0; jiggle_last = 0;
    for (i = 0; i < N_SLOTS; i++) { slot_type[i] = EMPTY; slot_stone[i] = 0; }
    for (i = 0; i < N_IN_MACHINE; i++) { s = (uint8_t)random_slot(1, -1); slot_type[s] = pick_prize(); }
    if (lv_stone[level]) {               // graveyard: start with 1 (W2-1) or 2 (W2-2) tombstones
        uint8_t want = (lv_stone[level] >= 4) ? 2 : 1;
        while (count_stones() < want) { s = (uint8_t)random_slot(0, -1); slot_stone[s] = 1; }
    }
    bats_init();
}

static void enter_play(void) {
    uint8_t i, wd = world();
    DISPLAY_OFF;
    hide_all();
    state = ST_PLAY;
    ld_spr_data(BK_A, 0, 40, sprite_tiles);
    load_normal_rc_tiles();
    if (SPACE_RC) {
        ld_spr_data(BK_B, SPR_RC + (RC_IDLE << 3), 8, space_rc_tiles);
        ld_spr_data(BK_B, SPR_RC + (RC_WALK << 3), 8, space_rc_tiles);
        ld_spr_data(BK_B, SPR_RC + (RC_SNEAK << 3), 8, space_rc_tiles);
        ld_spr_data(BK_B, SPR_RC + (RC_GIGGLE << 3), 8, space_rc_tiles + 128);
    }
    if (lv_bats[level]) ld_spr_data(BK_B, SPR_SWAP, 8, bat_tiles);
    ld_bkg_data(world_bank[wd], SCREEN_BASE, world_ntiles[wd], world_tiles[wd]);
    ld_screen(world_bank[wd], world_map[wd], world_attr[wd]);
    set_bkg_palette(0, 8, pal_bg_world[wd]);
    for (i = 0; i < N_SLOTS; i++) draw_slot(i);
    draw_sack(); draw_pips(); draw_level_tag();
    pstate = P_READY; ptimer = 0;
    claw_closed = 0; clawY = HOME_Y; clawX = CLAW_MINX; claw_fx = (uint16_t)clawX << 4; claw_vx = 0; drift_cur = 0;
    rc_on = 0; rc_hat = EMPTY; poof_slot = -1; shake_t = 0; knocked = 0;
    spark_t[0] = spark_t[1] = 0;
    set_claw_pal(0);
    draw_claw();
    draw_text_center(8, "READY?", 1);
    DISPLAY_ON;
    music_transpose(world_transpose[wd]);
    if (!music_muted) { music_play(MUS_THEME); update_tempo(); } else music_stop();
}

static void enter_lose(void);
static void enter_stars(void);

static void end_turn(uint8_t count_turn) {
    rc_on = 0; rc_show(); hide_sprite(O_BUB);
    if (pscore >= WIN_SCORE || rscore >= WIN_SCORE) { pstate = P_DONE; ptimer = 0; return; }
    if (count_turn) {
        turns++; mis_count++;
        golden_last = golden;
        golden = (!golden_last && turns >= 2 && (rand() & 7) == 0);
        if (mis_pending == M_NONE && turns >= 2 && mis_count >= lv_misfreq[level]) {
            mis_count = 0;
            if (SNEAK_ON && pscore >= 1 && (rand() & 1)) mis_pending = M_SNEAK;
            else if (SCOOT_ON) mis_pending = M_SCOOT;
            sneak_delay = 40 + (rand() % 90);
        }
        if (golden) sfx_play(SFX_GOLD);
    }
    update_tempo();
    claw_fx = (uint16_t)clawX << 4; claw_vx = 0; knocked = 0; broke_stone = 0;
    pstate = P_IDLE; idle_frames = 0; still_frames = 0; claw_closed = 0;
}

static void start_fly(uint8_t type, uint8_t x0, uint8_t y0, uint8_t kind) {
    fly_type = type; fly_t = 0; fly_x0 = x0; fly_y0 = y0; fly_kind = kind;
    fly_x1 = sack_x(pscore); fly_y1 = 0;
    pstate = P_FLY;
}
static void start_refill(void) {
    poof_slot = (count_in_machine() < N_IN_MACHINE) ? random_slot(1, -1) : -1;
    poof_t = 0; pstate = P_REFILL;
}
static void rc_enter_floor(uint8_t target_x) {
    rc_on = 1; rc_x = 168; rc_y = RC_FLOOR_Y; rc_flip = 0; rc_frame = RC_WALK; rc_hat = EMPTY;
    rc_target = target_x; rc_phase = 0; rc_t = 0;
}
static uint8_t rc_run_to(uint8_t speed) {
    uint8_t d = abs_diff(rc_x, rc_target);
    if (SPACE_RC && (frame & 15) == 0) sfx_play(SFX_JET);
    if (d <= speed) { rc_x = rc_target; rc_frame = RC_IDLE; return 1; }
    if (rc_x > rc_target) { rc_x -= speed; rc_flip = 0; } else { rc_x += speed; rc_flip = 1; }
    rc_frame = ((frame >> 2) & 1) ? RC_WALK : RC_IDLE;
    return 0;
}

static void move_claw(void) {
    uint8_t lv = level;
    int16_t vmax = lv_vmax[lv];
    if (keys & J_LEFT) {
        if (lv_accel[lv] >= 16) claw_vx = -16; else { claw_vx -= lv_accel[lv]; if (claw_vx < -vmax) claw_vx = -vmax; }
    } else if (keys & J_RIGHT) {
        if (lv_accel[lv] >= 16) claw_vx = 16; else { claw_vx += lv_accel[lv]; if (claw_vx > vmax) claw_vx = vmax; }
    } else {
        if (claw_vx > 0) { claw_vx -= lv_decel[lv]; if (claw_vx < 0) claw_vx = 0; }
        else if (claw_vx < 0) { claw_vx += lv_decel[lv]; if (claw_vx > 0) claw_vx = 0; }
    }
    if (claw_vx) {
        int16_t nx = (int16_t)claw_fx + claw_vx;
        if (nx <= (CLAW_MINX << 4)) { nx = CLAW_MINX << 4; claw_vx = 0; }
        if (nx >= (CLAW_MAXX << 4)) { nx = CLAW_MAXX << 4; claw_vx = 0; }
        claw_fx = (uint16_t)nx;
        still_frames = 0;
        if (++moving_tick >= 8) { moving_tick = 0; sfx_play(SFX_MOVE); }
    } else if (still_frames < 250) still_frames++;
    // low gravity: the claw drifts side to side on its own
    if (lv_drift[lv]) drift_cur = (int8_t)((wob[(frame >> 3) & 15] * lv_drift[lv]) / 3);
    {
        int16_t x = (int16_t)(claw_fx >> 4) + drift_cur;
        if (x < CLAW_MINX) x = CLAW_MINX; if (x > CLAW_MAXX) x = CLAW_MAXX;
        clawX = (uint8_t)x;
    }
}
static void update_arrows(void) {
    uint8_t show = (pstate == P_IDLE) && still_frames > 120 && ((frame & 31) < 22);
    if (show && clawX > CLAW_MINX + 2) spr16(O_ARROW, SPR_ARROW_L, clawX - 22, clawY + 4, 1); else hide_sprite(O_ARROW);
    if (show && clawX < CLAW_MAXX - 2) spr16(O_ARROW + 1, SPR_ARROW_R, clawX + 14, clawY + 4, 1); else hide_sprite(O_ARROW + 1);
}
static void claw_glow(void) {
    uint8_t over = (pstate == P_IDLE && clawX >= DROP_MINX && slot_under(clawX, grab_window()) >= 0);
    if (golden && pstate <= P_RELEASE) set_claw_pal(over ? 3 : 2);
    else set_claw_pal(over ? 1 : 0);
}
static void start_drop(void) {
    target_slot = slot_under(clawX, grab_window());
    claw_vx = 0; knocked = 0; broke_stone = 0;
    pstate = P_DROP; sfx_play(SFX_DROP);
}
static void begin_sneak(void) {
    mis_pending = M_NONE;
    rc_on = 1; rc_x = 160; rc_y = RC_TOP_Y; rc_flip = 0; rc_frame = RC_SNEAK; rc_hat = EMPTY;
    rc_target = sack_x(pscore - 1);
    rc_fx = (uint16_t)rc_x << 8; rc_t = 0;
    pstate = P_SNEAK; sfx_play(SFX_ALARM);
}
static void begin_scoot(int8_t from) {
    int8_t to;
    mis_pending = M_NONE;
    if (from < 0) from = random_slot(0, -1);
    to = random_slot(1, from);
    if (from < 0 || to < 0) { end_turn(0); return; }
    rc_slot = (uint8_t)from; rc_dest = (uint8_t)to;
    rc_enter_floor(slot_cx(rc_slot) - 8);
    pstate = P_SCOOT;
}

static void update_play(void) {
    switch (pstate) {
    case P_READY:
        ptimer++;
        if (ptimer == 50) { restore_rect(1, 8, 18, 1); draw_text_center(8, "GO!", 1); sfx_play(SFX_DING); }
        if (ptimer == 80) { restore_rect(1, 8, 18, 1); end_turn(0); }
        break;

    case P_IDLE:
        move_claw();
        if (idle_frames < 250) idle_frames++;
        if (pressed & (J_A | J_B)) {
            if (clawX < DROP_MINX) { sfx_play(SFX_NOPE); still_frames = 200; }
            else { start_drop(); break; }
        }
        if (mis_pending == M_SNEAK && pscore >= 1 && idle_frames >= sneak_delay) begin_sneak();
        else if (mis_pending == M_SNEAK && pscore == 0) mis_pending = SCOOT_ON ? M_SCOOT : M_NONE;
        else if (mis_pending == M_SCOOT) {
            int8_t s = slot_under(clawX, 10);
            if ((still_frames >= 30 && s >= 0) || idle_frames >= 200) begin_scoot(s);
        }
        break;

    case P_DROP:
        if (!SPACE || (frame & 1)) clawY++;
        if (target_slot >= 0 && !SPACE) {
            uint8_t cx = slot_cx(target_slot);
            if ((clawY & 1) == 0) { if (clawX < cx) clawX++; else if (clawX > cx) clawX--; }
        }
        if (clawY >= GRAB_Y) { clawY = GRAB_Y; pstate = P_GRAB; ptimer = 0; }
        break;

    case P_GRAB:
        ptimer++;
        if (ptimer == 8) {
            claw_closed = 1; sfx_play(SFX_GRAB);
            held_type = EMPTY;
            target_slot = slot_under(clawX, grab_window() + 1);     // where the claw really is
            if (target_slot >= 0) {
                grabbed_slot = target_slot;
                if (slot_stone[grabbed_slot]) {
                    slot_stone[grabbed_slot] = 0; broke_stone = 1;
                    draw_slot(grabbed_slot);
                    sfx_play(SFX_CRACK); shake_t = 8;
                    poof_slot = grabbed_slot; poof_t = 0;
                } else {
                    held_type = slot_type[grabbed_slot];
                    slot_type[grabbed_slot] = EMPTY;
                    draw_slot(grabbed_slot);
                    place_held(clawX - 8, clawY + 6, 0);
                }
            }
        }
        if (broke_stone && poof_slot >= 0) {
            poof_t++;
            meta16(O_FLY, (poof_t < 12) ? SPR_POOF_A : SPR_POOF_B, slot_cx(poof_slot) - 8, SLOT_ROW << 3, 3);
            if (poof_t >= 22) { hide16(O_FLY); poof_slot = -1; }
        }
        if (ptimer == 24) {
            jiggle = 0;
            if (held_type != EMPTY && JIGGLE_ON && !jiggle_last && turns >= 2 && (rand() & 15) < lv_jiggle[level]) {
                jiggle = 1;
                rc_enter_floor(abs_diff(120, clawX) < 24 ? 64 : 120);
            }
            jiggle_last = jiggle;
            pstate = P_RISE; sfx_play(SFX_RISE);
        }
        break;

    case P_RISE:
        if (clawY > HOME_Y) clawY--;
        if (held_type != EMPTY) place_held(clawX - 8, clawY + 6, 0);
        if (jiggle && rc_on) {
            if (rc_phase == 0) { if (rc_run_to(3)) { rc_phase = 1; rc_t = 0; } }
            else if (rc_phase == 1) {
                static const int8_t jump[16] = {-4,-4,-3,-3,-2,-2,-1,-1,1,1,2,2,3,3,4,4};
                rc_frame = RC_GIGGLE; rc_y += jump[rc_t];
                if (++rc_t == 16) {
                    rc_y = RC_FLOOR_Y; rc_phase = 2; rc_t = 0;
                    shake_t = 20; sfx_play(SFX_SHAKE);
                    if (held_type != EMPTY) {
                        pstate = P_SLIP; claw_closed = 0;
                        fall_y = (uint16_t)(clawY + 6) << 8; fall_vy = 0;
                        break;
                    }
                }
            }
        }
        if (clawY == HOME_Y && pstate == P_RISE) {
            if (held_type != EMPTY) { pstate = P_CARRY; rc_on = 0; }
            else if (broke_stone) { claw_closed = 0; end_turn(1); }
            else {
                int8_t s = slot_under(clawX, 160);
                claw_closed = 0;
                if (s < 0) { end_turn(1); break; }
                rc_slot = (uint8_t)s;
                rc_enter_floor(slot_cx(rc_slot) - 8);
                pstate = P_STEAL; sfx_play(SFX_GIGGLE);
            }
        }
        break;

    case P_SLIP:
        fall_vy += 40; fall_y += fall_vy;
        if (clawY > HOME_Y) clawY--;
        if ((fall_y >> 8) >= (SLOT_ROW << 3)) {
            slot_type[grabbed_slot] = held_type; draw_slot(grabbed_slot);
            hide16(O_HELD); held_type = EMPTY;
            sfx_play(SFX_GIGGLE);
            pstate = P_TAKE; rc_phase = 9; rc_t = 0;
        } else place_held(clawX - 8, fall_y >> 8, 0);
        break;

    case P_CARRY:
        if (clawX > CHUTE_X + 1) clawX -= 2; else clawX = CHUTE_X;
        place_held(clawX - 8, clawY + 6, 0);
        if (clawX == CHUTE_X) { pstate = P_RELEASE; ptimer = 0; fall_y = (uint16_t)(clawY + 6) << 8; fall_vy = 0; }
        break;

    case P_RELEASE:
        ptimer++;
        if (ptimer == 10) claw_closed = 0;
        if (ptimer > 10) {
            uint8_t y;
            fall_vy += 36; fall_y += fall_vy;
            y = fall_y >> 8;
            if (y < 128) place_held(clawX - 8, y, 1);
            else { hide16(O_HELD); sfx_play(SFX_PLOP); claw_fx = (uint16_t)clawX << 4; start_fly(held_type, 12, 96, 0); }
        }
        break;

    case P_FLY: {
        int16_t t = fly_t, x, y;
        fly_t++;
        x = fly_x0 + (((int16_t)fly_x1 - fly_x0) * t) / 32;
        y = fly_y0 + (((int16_t)fly_y1 - fly_y0) * t) / 32 - ((t * (32 - t)) >> 3);
        if (y < 0) y = 0;
        meta16(O_FLY, SPR_PRIZE + (fly_type << 2), (uint8_t)x, (uint8_t)y, prize_pal[fly_type]);
        if (fly_t >= 32) {
            hide16(O_FLY);
            sack_type[pscore] = fly_type; collected[fly_type] = 1;
            pscore++;
            draw_sack();
            spark_at(0, sack_x(pscore - 1) - 2, 2); spark_at(1, sack_x(pscore - 1) + 10, 8);
            sfx_play(SFX_DING);
            if (fly_kind == 0 && golden && pscore < WIN_SCORE) {
                if (rscore > 0) { rscore--; draw_pips(); start_fly(rc_loot[rscore], pip_x(rscore) - 4, 0, 1); }
                else start_fly(pick_prize(), 12, 96, 1);
                sfx_play(SFX_GOLD);
            } else start_refill();
        }
        break;
    }

    case P_REFILL:
        if (poof_slot < 0) { end_turn(1); break; }
        if (poof_t == 0) sfx_play(SFX_POOF);
        poof_t++;
        meta16(O_FLY, (poof_t < 12) ? SPR_POOF_A : SPR_POOF_B, slot_cx(poof_slot) - 8, SLOT_ROW << 3, 3);
        if (poof_t == 10) { fill_slot(poof_slot); draw_slot(poof_slot); }
        if (poof_t >= 24) {
            hide16(O_FLY); poof_slot = -1;
            if (count_in_machine() < N_IN_MACHINE) start_refill(); else end_turn(1);
        }
        break;

    case P_STEAL:
        if (rc_phase == 0) { if (rc_run_to(3)) { rc_phase = 1; rc_t = 0; } }
        else if (rc_phase == 1) {
            rc_t++; rc_frame = RC_GIGGLE;
            if (rc_t == 6) { rc_hat = slot_type[rc_slot]; slot_type[rc_slot] = EMPTY; slot_stone[rc_slot] = 0; draw_slot(rc_slot); sfx_play(SFX_HOP); }
            rc_y = RC_FLOOR_Y + (((rc_t >> 3) & 1) ? -2 : 0);
            if (rc_t == 40) { rc_phase = 2; rc_target = 168; sfx_play(SFX_GIGGLE); }
        } else {
            rc_y = RC_FLOOR_Y;
            if (rc_run_to(3)) {
                rc_loot[rscore] = rc_hat; rscore++; draw_pips();
                spark_at(0, pip_x(rscore - 1), 4);
                rc_hat = EMPTY; sfx_play(SFX_RASP);
                start_refill();
            }
        }
        break;

    case P_SCOOT:
        move_claw();
        if (pressed & (J_A | J_B)) sfx_play(SFX_NOPE);
        if (rc_phase == 0) { if (rc_run_to(3)) { rc_phase = 1; rc_t = 0; } }
        else if (rc_phase == 1) {
            rc_t++; rc_frame = RC_GIGGLE;
            if (rc_t == 6) { rc_hat = slot_type[rc_slot]; slot_type[rc_slot] = EMPTY; slot_stone[rc_slot] = 0; draw_slot(rc_slot); sfx_play(SFX_HOP); }
            if (rc_t == 24) { rc_phase = 2; rc_target = slot_cx(rc_dest) - 8; }
        } else if (rc_phase == 2) {
            rc_y = RC_FLOOR_Y - (((frame >> 2) & 1) ? 3 : 0);
            if (rc_run_to(2)) {
                rc_y = RC_FLOOR_Y;
                slot_type[rc_dest] = rc_hat; rc_hat = EMPTY; draw_slot(rc_dest);
                sfx_play(SFX_GIGGLE);
                rc_phase = 3; rc_t = 0;
            }
        } else if (rc_phase == 3) {
            rc_frame = RC_GIGGLE;
            if (++rc_t == 30) { rc_phase = 4; rc_target = 168; }
        } else if (rc_run_to(3)) end_turn(0);
        break;

    case P_SNEAK: {
        uint16_t speed;
        move_claw();
        rc_t++;
        if (rc_t >= 30 && (pressed & (J_A | J_B))) {
            rc_frame = RC_BONK; rc_phase = 0; rc_t = 0; shoos++;
            sfx_play(SFX_BONK);
            spark_at(0, rc_x - 2, rc_y); spark_at(1, rc_x + 12, rc_y + 2);
            hide_sprite(O_BUB);
            pstate = P_SHOO;
            break;
        }
        speed = (uint16_t)(((uint16_t)(160 - rc_target) << 8) / lv_sneak[level]);
        if ((rc_fx >> 8) <= rc_target + 1) {
            rc_x = rc_target;
            if (pscore) { pscore--; rc_hat = sack_type[pscore]; draw_sack(); }
            sfx_play(SFX_GIGGLE);
            hide_sprite(O_BUB);
            pstate = P_TAKE; rc_phase = 0; rc_t = 0; rc_target = 168;
            break;
        }
        rc_fx -= speed; rc_x = rc_fx >> 8;
        rc_frame = ((rc_t >> 3) & 1) ? RC_SNEAK : RC_WALK;
        rc_y = RC_TOP_Y + (((rc_t >> 3) & 1) ? 0 : 1);
        if ((rc_t & 31) == 0) sfx_play(SFX_ALARM);
        if (SPACE_RC && (rc_t & 15) == 0) sfx_play(SFX_JET);
        if ((rc_t & 15) < 11) spr16(O_BUB, SPR_BUBBLE, rc_x + 16, rc_y, 1); else hide_sprite(O_BUB);
        break;
    }

    case P_SHOO:
        move_claw();
        rc_t++;
        if (rc_t < 20) { rc_frame = RC_BONK; rc_y = RC_TOP_Y + ((rc_t & 2) ? 1 : 0); }
        else {
            rc_frame = RC_BONK; rc_flip = 1;
            rc_x += 3; if (rc_y > 0) rc_y--;
            if (rc_x >= 166) end_turn(0);
        }
        break;

    case P_TAKE:
        if (rc_phase == 9) {
            rc_frame = RC_GIGGLE;
            if (++rc_t >= 30) { rc_phase = 0; rc_target = 168; }
            if (clawY > HOME_Y) clawY--;
        } else {
            if (clawY > HOME_Y) clawY--;
            if (rc_run_to(3) && clawY == HOME_Y) end_turn(rc_y == RC_FLOOR_Y);
        }
        break;

    case P_DONE:
        if (++ptimer > 50) { if (pscore >= WIN_SCORE) enter_stars(); else enter_lose(); return; }
        break;
    }
    if (shake_t) { shake_t--; move_bkg((shake_t & 2) ? 2 : -2, 0); if (!shake_t) move_bkg(0, 0); }
    claw_glow();
    draw_claw();
    update_arrows();
    rc_show();
    update_play_sparks();
    update_bats();
    update_space_prizes();
}

// ------------------------------------------------------------------ world card
static void enter_card(void) {
    static char wt[] = "WORLD 1";
    uint8_t wd = world();
    DISPLAY_OFF;
    hide_all();
    state = ST_CARD; scr_timer = 0;
    load_rays_bg();
    ld_spr_data(BK_A, 0, 40, sprite_tiles); load_normal_rc_tiles();
    if (wd == 2) ld_spr_data(BK_B, SPR_SWAP, 8, bat_tiles);
    wt[6] = '1' + wd;
    draw_text_center(1, wt, 1);
    draw_text_center(3, world_names[wd], 1);
    if (wd == 0) draw_text_center(16, "GRAB 5 PRIZES!", 1);
    if (wd == 1) { draw_2x2(9, 9, TOMB_T, 3); draw_text_center(16, "BREAK THE STONE!", 1); }
    if (wd == 2) draw_text_center(16, "WATCH THE BATS!", 1);
    if (wd == 3) draw_text_center(16, "LOW GRAVITY!", 1);
    bats_init(); bat_y[0] = 60; bat_y[1] = 84;
    DISPLAY_ON;
    music_transpose(world_transpose[wd]);
    if (!music_muted) music_play(MUS_THEME);
}
static void update_card(void) {
    uint8_t wd = world(), i;
    if (scr_timer < 255) scr_timer++;
    if (wd == 0) {
        meta16(0, SPR_CLAW_OPEN, 72, 48 + bob[(frame >> 3) & 7], 1);
        draw_prize_spr(2, 1, 72, 80);
    } else if (wd == 1) {
        meta16(0, (scr_timer & 32) ? SPR_CLAW_CLOSED : SPR_CLAW_OPEN, 72, 56 + ((scr_timer & 32) ? 4 : 0), 0);
    } else if (wd == 2) {
        for (i = 0; i < 2; i++) {
            uint8_t x, y, t = SPR_SWAP + (((frame >> 3) & 1) ? 4 : 0);
            if (bat_dir[i]) { bat_fx[i] += 24; if ((bat_fx[i] >> 4) >= 132) bat_dir[i] = 0; }
            else            { bat_fx[i] -= 24; if ((bat_fx[i] >> 4) <= 22)  bat_dir[i] = 1; }
            x = bat_fx[i] >> 4; y = bat_y[i] + wob[((frame >> 2) + (i << 3)) & 15];
            if (bat_dir[i]) { spr16(i << 1, t + 2, x, y, 6 | S_FLIPX); spr16((i << 1) + 1, t, x + 8, y, 6 | S_FLIPX); }
            else            { spr16(i << 1, t, x, y, 6); spr16((i << 1) + 1, t + 2, x + 8, y, 6); }
        }
    } else {
        draw_prize_spr(0, 0, 56, 72 + bob[(frame >> 3) & 7] * 2);
        draw_prize_spr(2, 2, 88, 64 + bob[((frame >> 3) + 4) & 7] * 2);
        meta16(4, SPR_CLAW_OPEN, 72, 40 + wob[(frame >> 3) & 15] / 2, 0);
    }
    if ((scr_timer > 40 && (pressed & (J_A | J_B | J_START))) || scr_timer == 220) { new_round(); enter_play(); }
}

// ------------------------------------------------------------------ lose
static void enter_lose(void) {
    DISPLAY_OFF;
    hide_all();
    state = ST_LOSE; scr_timer = 0;
    losses_in_row++;
    assist = (assist + 2 > 6) ? 6 : assist + 2;
    load_rays_bg();
    ld_spr_data(BK_A, 0, 32, big_rc_tiles);
    draw_text_center(1, "REDCAP WINS!", 1);
    draw_text_center(16, "TRY AGAIN!", 1);
    DISPLAY_ON;
    music_play(MUS_LOSE);
    sfx_play(SFX_RASP);
}
static void update_lose(void) {
    uint8_t i, n = 0, j, col, up = ((frame >> 3) & 1) ? 4 : 0;
    uint8_t flip = (frame >> 5) & 1, y = 64 - up, x;
    if (scr_timer < 255) scr_timer++;
    for (i = 0; i < 8; i++) {
        if (BIG_RC_OVER_MASK & (1u << i)) {
            col = flip ? 3 - (i >> 1) : (i >> 1);
            spr16(n++, 16 + (i << 1), 64 + (col << 3), y + ((i & 1) << 4), 3 | (flip ? S_FLIPX : 0));
        }
    }
    for (i = 0; i < 8; i++) {
        col = flip ? 3 - (i >> 1) : (i >> 1);
        spr16(n++, i << 1, 64 + (col << 3), y + ((i & 1) << 4), 2 | (flip ? S_FLIPX : 0));
    }
    for (j = 0; j < 3 && j < rscore; j++) { x = 72 + ((j & 1) ? 2 : -2); draw_prize_spr(n, rc_loot[j], x, y - 14 - j * 13); n += 2; }
    if (scr_timer == 100) sfx_play(SFX_GIGGLE);
    if ((scr_timer > 60 && (pressed & (J_A | J_B | J_START))) || scr_timer == 255) { new_round(); enter_play(); }
}

// ------------------------------------------------------------------ star ranking
static void draw_total(uint8_t row) {
    static char tt[] = "TOTAL 00/40";
    uint8_t t = total_stars();
    tt[6] = '0' + t / 10; tt[7] = '0' + t % 10;
    draw_text_center(row, tt, 1);
}
static void enter_stars(void) {
    static char ct[] = "WORLD 1-1 CLEAR!";
    uint8_t i;
    stars_earned = 5 + shoos;
    stars_earned = (stars_earned > rscore) ? stars_earned - rscore : 1;
    if (stars_earned < 1) stars_earned = 1;
    if (stars_earned > 5) stars_earned = 5;
    if (stars_earned > level_stars[level]) level_stars[level] = stars_earned;
    losses_in_row = 0; assist = 0;
    DISPLAY_OFF;
    hide_all();
    state = ST_STARS; scr_timer = 0;
    load_rays_bg();
    ld_spr_data(BK_A, 0, 40, sprite_tiles); load_normal_rc_tiles();
    ct[6] = '1' + world(); ct[8] = '1' + (level & 1);
    draw_text_center(1, ct, 1);
    for (i = 0; i < 5; i++) draw_2x2(1 + (i << 2), 7, BIGSTAR_T, 2);
    draw_total(16);
    // save progress: next level unlocked (or stay on 7 until the ending plays)
    {
        uint8_t keep = level;
        if (level < N_LEVELS - 1) level++;
        save_write();
        level = keep;
    }
    DISPLAY_ON;
    music_play(MUS_CLEAR);
}
static void update_stars(void) {
    uint8_t i, t;
    if (scr_timer < 255) scr_timer++;
    for (i = 0; i < stars_earned; i++) {
        if (scr_timer == 30 + i * 18) { draw_2x2(1 + (i << 2), 7, BIGSTAR_T, 1); sfx_play(SFX_STARPOP); spark_at(0, 8 + (i << 5) - 2, 50); }
    }
    for (i = 0; i < WIN_SCORE; i++) {
        t = (frame >> 2) + (i << 1);
        draw_prize_spr(i << 1, sack_type[i], 24 + i * 24, 100 + bob[t & 7] * 2);
    }
    update_play_sparks();
    if (scr_timer > 30 + stars_earned * 18 + 20 && ((pressed & (J_A | J_B | J_START)) || scr_timer == 255)) {
        if (level == N_LEVELS - 1) { sub = 0; state = ST_FINAL; scr_timer = 0; /* enter_final below */ }
        else { level++; if ((level & 1) == 0) enter_card(); else { new_round(); enter_play(); } return; }
    }
}

// ------------------------------------------------------------------ ending: player 1, player 2, YOU WON
static uint8_t final_loaded;
static int16_t ship_x; static uint8_t ship_y;
static void enter_final_sub(void) {
    DISPLAY_OFF;
    hide_all();
    scr_timer = 0;
    load_rays_bg();
    ld_spr_data(BK_A, 0, 40, sprite_tiles); load_normal_rc_tiles();
    if (sub == 0) { draw_text_center(2, "CONGRATS", 1); draw_text_center(4, "PLAYER 1!", 1); draw_text_center(15, "YOU BEAT REDCAP!", 1); }
    if (sub == 1) { draw_text_center(2, "CONGRATS", 1); draw_text_center(4, "PLAYER 2!", 1); draw_text_center(15, "YOU BEAT REDCAP!", 1); }
    if (sub == 2) {
        ld_spr_data(BK_B, SPR_SWAP, 4, flag_tiles); ld_spr_data(BK_B, 0, 8, ship_tiles);
        draw_text_center(2, "YOU WON!", 1);
        draw_total(14);
        draw_text_center(16, "THE END", 1);
        ship_x = -40; ship_y = 76;
    }
    DISPLAY_ON;
    if (sub == 0) { music_transpose(0); music_play(MUS_END); }
}
static void update_final(void) {
    uint8_t i, x, y;
    if (!final_loaded) { final_loaded = 1; enter_final_sub(); return; }
    if (scr_timer < 255) scr_timer++;
    if (sub < 2) {
        // parade of all eight monsters, Redcap pouting in the corner
        for (i = 0; i < N_PRIZES; i++) {
            x = (uint8_t)((frame + i * 24) % 192);
            if (x < 176) draw_prize_spr(i << 1, i, x - 16 + 16, 66 + bob[((frame >> 2) + i) & 7] * 2);
            else hide16(i << 1);
        }
        rc_draw_at(16, 18, ((frame >> 5) & 1) ? RC_POUT : RC_BONK, 128, 104, 0);
        update_sparks(22, 6);
        if ((frame & 31) == 0) sfx_play(SFX_SPARKLE);
        if ((scr_timer > 90 && (pressed & (J_A | J_B | J_START))) || scr_timer == 255) { sub++; enter_final_sub(); }
    } else {
        // Redcap waves a white flag, the Sneaky Kryptid ship flies off, prizes dance
        rc_draw_at(16, 18, RC_IDLE, 40, 88, 0);
        spr16(20, SPR_SWAP + (((frame >> 3) & 1) ? 2 : 0), 56, 74, 3);
        for (i = 0; i < 4; i++) draw_prize_spr(22 + (i << 1), i + 2, 84 + i * 20, 86 + bob[((frame >> 2) + i * 2) & 7] * 2);
        if ((frame & 1) == 0) { ship_x += 2; if (ship_y > 24 && (frame & 3) == 0) ship_y--; }
        if (ship_x > 180) { ship_x = -40; ship_y = 76; sfx_play(SFX_WHOOSH); }
        if (ship_x >= -32 && ship_x < 168) {
            for (i = 0; i < 4; i++) {
                y = ship_y + bob[((frame >> 3) + i) & 7];
                spr16(0 + i, i << 1, (uint8_t)(ship_x + (i << 3)), y, 0);
            }
        } else for (i = 0; i < 4; i++) hide_sprite(i);
        update_sparks(30, 6);
        if (scr_timer > 120 && (pressed & (J_A | J_B | J_START))) {
            new_game(); save_clear(); final_loaded = 0;
            enter_title();
        }
    }
}

// ------------------------------------------------------------------ main
static void frame_palettes(void) {
    uint8_t wd = world();
    if (state == ST_PLAY && (frame & 15) == 0) {
        const palette_color_t *p = pal_bg_world[wd];
        pal_work[0] = p[4];
        if (frame & 16) { pal_work[1] = p[6]; pal_work[2] = p[5]; } else { pal_work[1] = p[5]; pal_work[2] = p[6]; }
        pal_work[3] = p[7];
        set_bkg_palette(1, 1, pal_work);
    }
    if (state == ST_LOSE || state == ST_STARS || state == ST_CARD || state == ST_FINAL) cycle_rays();
}

void game_main(void) BANKED {
    if (_cpu != CGB_TYPE) { DISPLAY_ON; SHOW_BKG; while (1) vsync(); }
    DISPLAY_OFF;
    SPRITES_8x16;
    sound_init();
    load_common();
    new_game();
    save_load();
    enter_title();
    SHOW_BKG; SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        vsync();
        frame++;
        frame_palettes();
        sound_update();
        read_keys();
        if (pressed & J_SELECT) {
            music_muted ^= 1;
            if (music_muted) music_stop();
            else if (state == ST_PLAY) { music_play(MUS_THEME); update_tempo(); }
            else if (state == ST_TITLE) music_play(MUS_TITLE);
        }
        switch (state) {
        case ST_TITLE:
            update_title();
            if (pressed & (J_A | J_START)) {
                initrand(((uint16_t)DIV_REG << 8) | frame);
                sfx_play(SFX_START);
                if (keys & J_B) { new_game(); save_clear(); }
                if ((level & 1) == 0) enter_card(); else { new_round(); enter_play(); }
            }
            break;
        case ST_CARD:  update_card();  break;
        case ST_PLAY:  update_play();  break;
        case ST_LOSE:  update_lose();  break;
        case ST_STARS: update_stars(); break;
        case ST_FINAL: update_final(); break;
        }
    }
}
