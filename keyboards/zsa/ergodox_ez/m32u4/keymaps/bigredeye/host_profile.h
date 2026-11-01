#pragma once

#include "quantum.h"

// Resolved to ordinary QMK keycodes before tap/hold processing.
enum host_profile_keycodes {
    HOST_TOGGLE = QK_USER,
    HOST_ESC,
    HOST_QUOTE,
    HOST_LANGUAGE,
};

void host_profile_init(void);
bool host_profile_is_mac(void);
bool host_profile_process_record(uint16_t keycode, keyrecord_t *record);
