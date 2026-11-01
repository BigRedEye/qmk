#include QMK_KEYBOARD_H
#include "host_profile.h"
#include "keymap_introspection.h"
#include "matrix.h"
#include "os_detection.h"

typedef enum { HOST_PC, HOST_MAC } host_profile_t;

static host_profile_t profile = HOST_PC;
static host_profile_t requested_profile = HOST_PC;
static os_variant_t host_os = OS_UNSURE;
static bool manual_profile = false;
static bool profile_pending = false;

bool host_profile_is_mac(void) {
    return profile == HOST_MAC;
}

static void update_profile_light(void) {
    layer_state_set_user(layer_state);
#ifdef RGBLIGHT_ENABLE
    uint8_t hue = 85; // PC/Linux: green; also the manual PC fallback.
    uint8_t saturation = 255;
    if (profile == HOST_MAC) {
        hue = 170; // Mac: blue, matching status LED 3.
    } else if (host_os == OS_WINDOWS) {
        hue = 128; // Windows: cyan.
    } else if (host_os == OS_UNSURE && !manual_profile) {
        saturation = 0; // Detection unknown: white, with PC bindings.
    }
    uint8_t brightness = rgblight_get_val();
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_sethsv_noeeprom(hue, saturation, brightness);
#endif
}

void host_profile_init(void) {
    update_profile_light();
}

bool process_detected_host_os_user(os_variant_t os) {
    host_os = os;
    if (!manual_profile && os != OS_UNSURE) {
        requested_profile = (os == OS_MACOS || os == OS_IOS) ? HOST_MAC : HOST_PC;
    }
    profile_pending = true;
    return true;
}

bool host_profile_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode != HOST_TOGGLE) {
        return true;
    }
    if (record->event.pressed) {
        requested_profile = requested_profile == HOST_PC ? HOST_MAC : HOST_PC;
        manual_profile = true; // Autodetection cannot undo a manual choice until restart.
        profile_pending = true;
    }
    return false;
}

void housekeeping_task_user(void) {
    if (!profile_pending || get_mods() || get_weak_mods() || get_oneshot_mods()) {
        return;
    }
    // A held key must be released using the same mapping as its press.
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        if (matrix_get_row(row)) {
            return;
        }
    }
    profile = requested_profile;
    profile_pending = false;
    update_profile_light();
}

uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (key.row >= MATRIX_ROWS || key.col >= MATRIX_COLS) {
        return KC_NO;
    }
    // Leave transparent keys and explicit layer overrides to QMK's layer lookup.
    uint16_t keycode = keycode_at_keymap_location(layer, key.row, key.col);
    switch (keycode) {
        case HOST_ESC:
            return profile == HOST_MAC ? MT(MOD_LGUI, KC_ESCAPE) : MT(MOD_LCTL, KC_ESCAPE);
        case HOST_QUOTE:
            return profile == HOST_MAC ? MT(MOD_RCTL, KC_QUOTE) : MT(MOD_LGUI, KC_QUOTE);
        case HOST_LANGUAGE:
            return profile == HOST_MAC ? LALT(LGUI(LCTL(LSFT(KC_SPACE)))) : KC_CAPS;
        default:
            return keycode;
    }
}
