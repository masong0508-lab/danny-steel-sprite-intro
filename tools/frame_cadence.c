/* Noninvasive visible-frame cadence probe for the production GBA ROM.
 * Build: gcc -std=c11 -O2 -Wall -Wextra -Werror tools/frame_cadence.c -lmgba -o /tmp/danny_frame_cadence
 * Run:   /tmp/danny_frame_cadence danny_steel_title.gba [existing-picture-directory]
 *
 * Public APIs checked against mGBA 0.10.3 include/mgba/core/core.h.
 * runFrame advances emulated video time, NOT one completed game render.
 * Count DISPCNT page-select transitions at those boundaries instead. This
 * measures this ROM's displayed-page cadence on a scripted opening/town
 * route; it is not renderer CPU time, physical hardware timing or coverage
 * of every scene. Multiple flips within one video frame would be invisible
 * to this sampler, but play.c deliberately flips at most once per VBlank.
 * No game memory, save files, runtime code or emulator speed is modified.
 */
#define _POSIX_C_SOURCE 200809L
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/log.h>
#include <mgba/core/version.h>
#include <mgba-util/vfs.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

enum { SAMPLE_FRAMES = 600, KEY_A = 1, KEY_B = 2, KEY_START = 8 };

static void log_errors(struct mLogger *logger, int category,
                       enum mLogLevel level, const char *format, va_list args) {
    (void)logger;
    (void)category;
    if (level & (mLOG_FATAL | mLOG_ERROR)) {
        vfprintf(stderr, format, args);
        fputc('\n', stderr);
    }
}
static struct mLogger logger = { .log = log_errors, .filter = NULL };

static void advance(struct mCore *core, unsigned frames) {
    while (frames--) {
        core->runFrame(core);
        /* Discard only host output queues; emulated PSG/timers still run. */
        for (int channel = 0; channel < 2; ++channel) {
            struct blip_t *audio = core->getAudioChannel(core, channel);
            if (audio) blip_clear(audio);
        }
    }
}

static void press(struct mCore *core, unsigned key) {
    /* Polling happens between real renders. Two video-frame pulses can be
       missed while a slow draw is still running; hold through that work.
       Menu/story handlers consume pressed edges, so this is still one tap. */
    core->setKeys(core, key);
    advance(core, 32);
    core->setKeys(core, 0);
    advance(core, 32);
}

static int is_mode4(uint32_t display) {
    return (display & 7u) == 4u && (display & 0x400u) && !(display & 0x80u);
}

static void picture(struct mCore *core, const char *directory,
                    const char *name) {
    if (!directory) return;
    char path[1024];
    int n = snprintf(path, sizeof(path), "%s/cadence-%s.png", directory, name);
    if (n < 0 || (size_t)n >= sizeof(path)) {
        fprintf(stderr, "Cadence picture path is too long\n");
        return;
    }
    struct VFile *file = VFileOpen(path, O_WRONLY | O_CREAT | O_TRUNC);
    if (!file) {
        fprintf(stderr, "Cannot create %s\n", path);
        return;
    }
    bool ok = mCoreTakeScreenshotVF(core, file);
    file->close(file);
    if (!ok) fprintf(stderr, "Cannot encode %s\n", path);
}

static void measure(struct mCore *core, const char *name, double video_hz) {
    unsigned flips = 0, mode4 = 0, intervals = 0, total_gap = 0;
    unsigned shortest = UINT_MAX, longest = 0, over_two = 0, previous_flip = 0;
    unsigned leading = SAMPLE_FRAMES, trailing = 0;
    uint32_t previous = core->busRead16(core, 0x04000000u);
    uint32_t start = core->frameCounter(core);
    for (unsigned f = 1; f <= SAMPLE_FRAMES; ++f) {
        advance(core, 1);
        uint32_t current = core->busRead16(core, 0x04000000u);
        if (is_mode4(current)) ++mode4;
        if (is_mode4(previous) && is_mode4(current) &&
            ((current ^ previous) & 0x10u)) {
            if (flips) {
                unsigned gap = f - previous_flip;
                if (gap < shortest) shortest = gap;
                if (gap > longest) longest = gap;
                if (gap > 2) ++over_two;
                total_gap += gap;
                ++intervals;
            } else leading = f;
            previous_flip = f;
            ++flips;
        }
        previous = current;
    }
    trailing = SAMPLE_FRAMES - previous_flip;
    uint32_t elapsed = core->frameCounter(core) - start;
    double seconds = (double)elapsed / video_hz;
    printf("%s: video_frames=%u, active_mode4_samples=%u/%u, page_flips=%u, "
           "emulated_seconds=%.6f, observed_page_flips_per_second=%.3f\n",
           name, elapsed, mode4, SAMPLE_FRAMES, flips, seconds,
           seconds > 0.0 ? flips / seconds : 0.0);
    if (intervals) {
        printf("  complete flip intervals: count=%u, min=%u, mean=%.3f, "
               "max=%u video_frames (max %.3f ms), over_two_frames=%u\n",
               intervals, shortest, (double)total_gap / intervals, longest,
               1000.0 * longest / video_hz, over_two);
    } else puts("  no complete flip-to-flip interval observed");
    printf("  window edges: %u frames before first observed flip, "
           "%u after last (partial intervals)\n", leading, trailing);
    if (mode4 != SAMPLE_FRAMES || !flips || elapsed != SAMPLE_FRAMES)
        puts("  WARNING: scripted route/timing needs screenshot review; "
             "do not treat this sample as a validated gameplay benchmark.");
    fflush(stdout);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s ROM [existing-picture-directory]\n", argv[0]);
        return 1;
    }
    mLogSetDefaultLogger(&logger);
    struct mCore *core = mCoreFind(argv[1]);
    if (!core) { fputs("Cannot find a core for ROM\n", stderr); return 1; }
    mCoreInitConfig(core, NULL);
    if (!core->init(core)) { fputs("Cannot initialize mGBA\n", stderr); return 1; }
    if (core->platform(core) != mPLATFORM_GBA || !mCoreLoadFile(core, argv[1])) {
        fputs("Cannot load a GBA ROM\n", stderr);
        mCoreConfigDeinit(&core->config);
        core->deinit(core);
        return 1;
    }
    /* Intentionally do not autoload a user's save/config/cheats. */
    unsigned width = 0, height = 0;
    core->desiredVideoDimensions(core, &width, &height);
    color_t *pixels = calloc((size_t)width * height, sizeof(*pixels));
    if (!pixels || width != 240 || height != 160) {
        fputs("Cannot allocate native GBA video output\n", stderr);
        free(pixels);
        mCoreConfigDeinit(&core->config);
        core->deinit(core);
        return 1;
    }
    core->setVideoBuffer(core, pixels, width);
    core->reset(core);
    double video_hz = (double)core->frequency(core) / core->frameCycles(core);
    printf("mGBA %s; emulated video %.9f Hz (%d cycles/frame, %d Hz CPU)\n",
           projectVersion, video_hz, core->frameCycles(core), core->frequency(core));
    puts("Cadence counts presented Mode4 page changes, including identical "
         "content. It does not measure host speed or certify all-scene 30 FPS.");
    puts("Stage names describe the scripted route; verify the accompanying "
         "cadence pictures before treating them as scene measurements.");
    fflush(stdout);
    advance(core, 1650);
    press(core, KEY_START); advance(core, 95);
    press(core, KEY_A); advance(core, 95);
    press(core, KEY_A); advance(core, 120);
    const char *directory = argc == 3 ? argv[2] : NULL;
    picture(core, directory, "opening-start");
    measure(core, "opening after New Game", video_hz);
    picture(core, directory, "opening-end");
    press(core, KEY_B); advance(core, 120);
    picture(core, directory, "town-start");
    measure(core, "town after B dismisses opening", video_hz);
    picture(core, directory, "town-end");
    mCoreConfigDeinit(&core->config);
    core->deinit(core);
    free(pixels);
    return 0; /* Informational: no invented FPS pass/fail threshold. */
}
