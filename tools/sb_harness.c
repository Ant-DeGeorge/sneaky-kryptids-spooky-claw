// Scripted SameBoy harness: accurate CGB emulation with an input script and screenshots.
// Script lines: "<frame> press|release <A|B|START|SELECT|LEFT|RIGHT|UP|DOWN>" or "<frame> shot <name>" or "<frame> end"
#include <Core/gb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t bitmap[256 * 224];
static volatile int vblank_hit;
static const char *boot_dir;
static int cc_mode;

static uint32_t rgb_encode(GB_gameboy_t *gb, uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
static void vblank(GB_gameboy_t *gb, GB_vblank_type_t type) { vblank_hit = 1; }
static void log_cb(GB_gameboy_t *gb, const char *s, GB_log_attributes_t a) { fprintf(stderr, "[sb] %s", s); }
static void load_boot(GB_gameboy_t *gb, GB_boot_rom_t type) {
    char p[1024]; snprintf(p, sizeof p, "%s/cgb_boot.bin", boot_dir); GB_load_boot_rom(gb, p);
}
static int key_of(const char *s) {
    if (!strcmp(s, "A")) return GB_KEY_A; if (!strcmp(s, "B")) return GB_KEY_B;
    if (!strcmp(s, "START")) return GB_KEY_START; if (!strcmp(s, "SELECT")) return GB_KEY_SELECT;
    if (!strcmp(s, "LEFT")) return GB_KEY_LEFT; if (!strcmp(s, "RIGHT")) return GB_KEY_RIGHT;
    if (!strcmp(s, "UP")) return GB_KEY_UP; return GB_KEY_DOWN;
}
static void write_ppm(const char *path) {
    FILE *f = fopen(path, "wb"); fprintf(f, "P6\n160 144\n255\n");
    for (int i = 0; i < 160 * 144; i++) { uint32_t c = bitmap[i]; fputc(c >> 16, f); fputc((c >> 8) & 0xFF, f); fputc(c & 0xFF, f); }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 5) { fprintf(stderr, "usage: rom script outdir bootdir [cc]\n"); return 1; }
    boot_dir = argv[4]; cc_mode = argc > 5 ? atoi(argv[5]) : 0;
    static GB_gameboy_t gb;
    GB_init(&gb, GB_MODEL_CGB_E);
    GB_set_boot_rom_load_callback(&gb, load_boot);
    GB_set_vblank_callback(&gb, vblank);
    GB_set_pixels_output(&gb, bitmap);
    GB_set_rgb_encode_callback(&gb, rgb_encode);
    GB_set_log_callback(&gb, log_cb);
    GB_set_color_correction_mode(&gb, cc_mode ? GB_COLOR_CORRECTION_MODERN_BALANCED : GB_COLOR_CORRECTION_DISABLED);
    if (GB_load_rom(&gb, argv[1])) { perror("rom"); return 1; }
    FILE *sf = fopen(argv[2], "r");
    int ef[4096], ek[4096], ea[4096]; char en[4096][64]; int n = 0;
    char act[32], arg[64];
    while (n < 4096 && fscanf(sf, "%d %31s", &ef[n], act) == 2) {
        arg[0] = 0;
        if (strcmp(act, "end")) fscanf(sf, "%63s", arg);
        ea[n] = !strcmp(act, "press") ? 1 : !strcmp(act, "release") ? 0 : !strcmp(act, "shot") ? 2 : 3;
        ek[n] = (ea[n] < 2) ? key_of(arg) : 0; strcpy(en[n], arg); n++;
    }
    int frame = 0, idx = 0;
    while (1) {
        vblank_hit = 0;
        while (!vblank_hit) GB_run(&gb);
        frame++;
        while (idx < n && ef[idx] <= frame) {
            if (ea[idx] == 0 || ea[idx] == 1) GB_set_key_state(&gb, ek[idx], ea[idx]);
            else if (ea[idx] == 2) { char p[512]; snprintf(p, sizeof p, "%s/%s.ppm", argv[3], en[idx]); write_ppm(p); }
            else { printf("end at frame %d, PC=%04x SP=%04x\n", frame, gb.pc, gb.registers[GB_REGISTER_SP]); return 0; }
            idx++;
        }
    }
}
