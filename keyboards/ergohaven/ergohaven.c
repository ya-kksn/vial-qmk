#include "ergohaven.h"
#include "lang_ru_en.h"
#include "ergohaven_rgb.h"
#include "hid.h"

#ifdef AUDIO_ENABLE
float base_sound[][2] = SONG(TERMINAL_SOUND);
float caps_sound[][2] = SONG(CAPS_LOCK_ON_SOUND);
#endif

bool is_alt_tab_active = false;
uint16_t alt_tab_timer = 0;



// Custom keycodes
__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
  return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // #ifdef WPM_ENABLE
  //   if (record->event.pressed) {
  //       extern uint32_t tap_timer;
  //       tap_timer = timer_read32();
  //   }
  // #endif

  switch (keycode) { // This will do most of the grunt work with the keycodes.
    case ALT_TAB:
      if (record->event.pressed) {
        if (!is_alt_tab_active) {
          is_alt_tab_active = true;
          register_code(KC_LALT);
        }
        alt_tab_timer = timer_read();
        register_code(KC_TAB);
      } else {
        unregister_code(KC_TAB);
      }
      break;

    case NEXTSEN:  // Next sentence macro.
      if (record->event.pressed) {
        SEND_STRING(". ");
    #ifdef AUDIO_ENABLE
        PLAY_SONG(caps_sound);
    #endif
        add_oneshot_mods(MOD_BIT(KC_LSFT));  // Set one-shot mod for shift.
      }
      return false;

    case PREDL:  // Next sentence macro.
      if (record->event.pressed) {
        SEND_STRING("/ ");
    #ifdef AUDIO_ENABLE
        PLAY_SONG(caps_sound);
    #endif
        add_oneshot_mods(MOD_BIT(KC_LSFT));  // Set one-shot mod for shift.
      }
      return false;

    case BRACES:
            if (record->event.pressed) {
                uint8_t shifted = get_mods() & (MOD_MASK_SHIFT);
                    if (shifted) {
                        unregister_code(KC_LSFT);
                        unregister_code(KC_RSFT);
                        SEND_STRING("{}"SS_TAP(X_LEFT));
                    }
                    else {
                        SEND_STRING("[]"SS_TAP(X_LEFT));
                    }
            }
            break;

    case PARENTH:
            if (record->event.pressed) {
                SEND_STRING("()");
                tap_code(KC_LEFT);
            }
          break;

    case KC_CAPS:
      if (record->event.pressed) {
    #ifdef AUDIO_ENABLE
        PLAY_SONG(caps_sound);
    #endif
        }
      return true; // Let QMK send the enter press/release events

    case LAYER_NEXT:
      // Our logic will happen on presses, nothing is done on releases
      if (!record->event.pressed) {
        // We've already handled the keycode (doing nothing), let QMK know so no further code is run unnecessarily
        return false;
      }

      uint8_t current_layer = get_highest_layer(layer_state);

      // Check if we are within the range, if not quit
      if (current_layer > LAYER_CYCLE_END || current_layer < LAYER_CYCLE_START) {
        return false;
      }

      uint8_t next_layer = current_layer + 1;
      if (next_layer > LAYER_CYCLE_END) {
          next_layer = LAYER_CYCLE_START;
      }
      layer_move(next_layer);
      return false;

    case LAYER_PREV:
      // Our logic will happen on presses, nothing is done on releases
      if (!record->event.pressed) {
        // We've already handled the keycode (doing nothing), let QMK know so no further code is run unnecessarily
        return false;
      }

      uint8_t this_layer  = get_highest_layer(layer_state);

      // Check if we are within the range, if not quit
      if (this_layer > LAYER_CYCLE_END || this_layer < LAYER_CYCLE_START) {
        return false;
      }

      uint8_t prev_layer = this_layer - 1;
      if (prev_layer > LAYER_CYCLE_END) {
          prev_layer = LAYER_CYCLE_START;
      }
      layer_move(prev_layer);
      return false;

    case CAPS_WORD:
      if (record->event.pressed) {
        caps_word_toggle();
      }
      return false;

    default:
      return process_record_lang(keycode, record);
  }
   return process_record_keymap(keycode, record);
}

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // Keycodes for russian symbols
        case KC_SCLN:
        case KC_QUOT:
        case KC_LBRC:
        case KC_RBRC:
        case KC_GRAVE:
            if (get_cur_lang() == LANG_RU) {
                add_weak_mods(MOD_BIT(KC_LSFT));
                return true;
            } else
                return false;

        // Keycodes that continue Caps Word, with shift applied.
        case TD(0):
        case TD(1):
        case TD(2):
        case TD(3):
        case TD(4):
        case TD(5):
        case TD(6):
        case TD(7):
        case KC_A ... KC_Z:
        case KC_MINS:
            add_weak_mods(MOD_BIT(KC_LSFT)); // Apply shift to next key.
            return true;

        // Keycodes that continue Caps Word, without shifting.
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_UNDS:
            return true;

        default:
            return false; // Deactivate Caps Word.
    }
}

void matrix_scan_user(void) { // The very important timer.
  if (is_alt_tab_active) {
    if (timer_elapsed(alt_tab_timer) > 650) {
      unregister_code(KC_LALT);
      is_alt_tab_active = false;
    }
  }


// __attribute__ ((weak))
// layer_state_t layer_state_set_keymap (layer_state_t state) {
//   return state;
// }

// layer_state_t layer_state_set_user (layer_state_t state) {
//       #if defined(AUDIO_ENABLE)
//         static bool is_base_on = false;
//     if (layer_state_cmp(state, _BASE) != is_base_on) {
//             is_base_on = layer_state_cmp(state, _BASE);
//             if (is_base_on) {
//                 stop_all_notes();
//             } else {
//                 PLAY_SONG(base_sound);
//             }
//         }
//     #endif
//   return layer_state_set_keymap (state);
//     }
}

void keyboard_post_init_kb(void) {
#ifdef RGBLIGHT_ENABLE
    keyboard_post_init_rgb();
#endif
    keyboard_post_init_hid();
    keyboard_post_init_user();
}

layer_state_t default_layer_state_set_kb(layer_state_t state) {
    state = default_layer_state_set_user(state);
#ifdef RGBLIGHT_ENABLE
    default_layer_state_set_rgb(state);
#endif
    return state;
}

layer_state_t layer_state_set_kb(layer_state_t state) {
  state = layer_state_set_user(state);
#ifdef RGBLIGHT_ENABLE
    layer_state_set_rgb(state);
#endif
    return state;
}

static const char* PROGMEM LAYER_NAME[] =   {
    "Base ",
    "Raise",
    "Lower",
    "Adjst",
    "Four ",
    "Five ",
    "Six  ",
    "Seven",
    "Eight",
    "Nine ",
    "Ten  ",
    "Elevn",
    "Twlve",
    "Thrtn",
    "Frtn ",
    "Fiftn",
};

static const char* PROGMEM LAYER_UPPER_NAME[] =   {
    "BASE ",
    "RAISE",
    "LOWER",
    "ADJST",
    "FOUR ",
    "FIVE ",
    "SIX  ",
    "SEVEN",
    "EIGHT",
    "NINE ",
    "TEN  ",
    "ELEVN",
    "TWLVE",
    "THRTN",
    "FRTN ",
    "FIFTN",
};

const char* layer_name(int layer) {
    if (layer >= 0 && layer <= 15)
        return LAYER_NAME[layer];
    else
        return "Undef";
}

const char* layer_upper_name(int layer) {
    if (layer >= 0 && layer <= 15)
        return LAYER_UPPER_NAME[layer];
    else
        return "UNDEF";
}
