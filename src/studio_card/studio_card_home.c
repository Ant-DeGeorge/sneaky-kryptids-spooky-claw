/* studio_card_home.c — the only part of the studio card that must sit in ROM0:
   the STAT (HBlank) vector and the tile loader. ~90 bytes.

   Raster mode (default): this file owns the STAT interrupt vector (0x48).
     - while the card runs:  SCX = sc_cur[LY+1]   (~30 cycles, AF/HL only)
     - otherwise:            calls sc_stat_hook if the host set one
   => Host games must NOT use add_LCD()/remove_LCD() or stdio.h (both claim 0x48).
      Put your own STAT code in a function and set  sc_stat_hook = my_handler;
   If something else ends up owning 0x48 (the linker does not complain), the
   card detects it at runtime and uses a whole-screen shake instead.
   Build with -DSC_NO_RASTER to leave the vector alone entirely. */
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <string.h>

BANKREF_EXTERN(studio_card_gfx)
BANKREF_EXTERN(studio_card_scx)
extern const uint16_t sc_scx_off[];
extern const uint8_t sc_scx_blob[];
extern const uint8_t sc_tiles0[];
extern const uint8_t sc_tiles1[];
uint8_t * volatile sc_cur;
volatile uint8_t sc_stat_on;

#include "studio_card_counts.h"

#ifndef SC_NO_RASTER
#include <gb/isr.h>
void (* volatile sc_stat_hook)(void);

void sc_stat_vec(void) NONBANKED __naked {
__asm
    push af
    push hl
    ld   a, (_sc_stat_on)
    or   a
    jr   z, 00010$
    ld   hl, #_sc_cur
    ld   a, (hl+)
    ld   h, (hl)
    ld   l, a
    ldh  a, (0x44)          ; LY
    inc  a
    add  a, l
    ld   l, a
    jr   nc, 00001$
    inc  h
00001$:
    ld   a, (hl)
    ldh  (0x43), a          ; SCX
    pop  hl
    pop  af
    reti
00010$:                     ; card idle: forward to the host hook, if any
    ld   hl, #_sc_stat_hook
    ld   a, (hl+)
    ld   h, (hl)
    ld   l, a
    or   h
    jr   z, 00011$
    push bc
    push de
    call ___sdcc_call_hl
    pop  de
    pop  bc
00011$:
    pop  hl
    pop  af
    reti
__endasm;
}
ISR_VECTOR(VECTOR_STAT, sc_stat_vec)

/* 1 if the STAT vector really jumps to sc_stat_vec. The linker silently lets
   stdio / add_LCD overwrite 0x48, so check at runtime and fall back if not. */
uint8_t sc_raster_ok(void) NONBANKED {
    const uint8_t *v = (const uint8_t *)0x48;
    return v[0] == 0xC3 && v[1] == (uint8_t)(uint16_t)&sc_stat_vec && v[2] == (uint8_t)((uint16_t)&sc_stat_vec >> 8);
}
#else
uint8_t sc_raster_ok(void) NONBANKED { return 0; }
#endif

void sc_load_gfx(void) NONBANKED {
    uint8_t save = _current_bank;
    SWITCH_ROM(BANK(studio_card_gfx));
    VBK_REG = VBK_BANK_1; set_bkg_data(0, SC_TILES1, sc_tiles1);
    VBK_REG = VBK_BANK_0; set_bkg_data(0, SC_TILES0, sc_tiles0);
    SWITCH_ROM(save);
}

/* Fill dst[0..144] with the scanline SCX table for frame fr. Returns 1 if any row moves. */
uint8_t sc_load_scx(uint16_t fr, uint8_t *dst) NONBANKED {
    uint8_t save = _current_bank, moved = 0;
    uint16_t o;
    SWITCH_ROM(BANK(studio_card_scx));
    o = sc_scx_off[fr];
    memset(dst, 0, 145);
    if (o != 0xFFFF) {
        const uint8_t *p = sc_scx_blob + o;
        memcpy(dst + p[0], p + 2, p[1]);
        moved = 1;
    }
    SWITCH_ROM(save);
    return moved;
}
