//
// Copyright (C) 2015-2016  Markus Hiienkari <mhiienka@niksula.hut.fi>
//
// This file is part of Open Source Scan Converter project.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <unistd.h>
#include "string.h"
#include "altera_avalon_pio_regs.h"
#include "altera_epcq_controller_mod.h"
#include "cfg.h"
#include "av_controller.h"
#include "lcd.h"
#include "video_modes.h"
#include "controls.h"
#include "menu.h"

const char const *rc_keydesc[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "MENU", "OK", "BACK", "UP", "DOWN", "LEFT", "RIGHT", "INFO", "LCD_BACKLIGHT", "SCANLINE_TGL", "SCANLINE_INT+", "SCANLINE_INT-"};
// #define REMOTE_MAX_KEYS (sizeof(rc_keydesc)/sizeof(char*))
alt_u16 rc_keymap[REMOTE_MAX_KEYS] = {0x3E29, 0x3EA9, 0x3E69, 0x3EE9, 0x3E19, 0x3E99, 0x3E59, 0x3ED9, 0x3E39, 0x3EC9, 0x3E4D, 0x3E1D, 0x3EED, 0x3E2D, 0x3ECD, 0x3EAD, 0x3E6D, 0x3E65, 0x3E01, 0x1C48, 0x1C50, 0x1CD0};

alt_u8 remote_rpt, remote_rpt_prev;
alt_u32 btn_code, btn_code_prev;

extern char menu_row1[LCD_ROW_LEN+1], menu_row2[LCD_ROW_LEN+1];
alt_u32 remote_code;

alt_u8 menu_active = 0;
extern alt_u8 show_submenu;

extern volatile avconfig_t tc;
extern volatile avmode_t cm;

avinput_t target_mode;

extern mode_data_t video_modes[];

void setup_rc()
{
    int i, confirm;
    alt_u32 remote_code_prev = 0x0000;

    for (i=0; i<REMOTE_MAX_KEYS; i++) {
        strncpy(menu_row1, "Press", LCD_ROW_LEN+1);
        strncpy(menu_row2, rc_keydesc[i], LCD_ROW_LEN+1);
        lcd_write_menu();
        confirm = 0;

        while (1) {
            remote_code = IORD_ALTERA_AVALON_PIO_DATA(PIO_1_BASE) & RC_MASK;

            if (remote_code && (remote_code != remote_code_prev)) {
                if (confirm == 0) {
                    rc_keymap[i] = remote_code;
                    strncpy(menu_row1, "Confirm", LCD_ROW_LEN+1);
                    lcd_write_menu();
                    confirm = 1;
                } else {
                    if (remote_code == rc_keymap[i]) {
                        confirm = 2;
                    } else {
                        strncpy(menu_row1, "Mismatch, retry", LCD_ROW_LEN+1);
                        lcd_write_menu();
                        confirm = 0;
                    }
                }

            }

            remote_code_prev = remote_code;

            if (confirm == 2)
                break;

            usleep(MAINLOOP_SLEEP_US);
        }
    }
}

void parse_control()
{
    if (remote_code)
        printf("RCODE: 0x%.4x, %u\n", remote_code, remote_rpt);

    if (btn_code_prev == 0 && btn_code != 0)
        printf("BCODE: 0x%.2x\n", btn_code>>16);

    int i;
    for (i = RC_BTN1; i < REMOTE_MAX_KEYS; i++) {
        if (remote_code == rc_keymap[i])
            break;
        if (i == REMOTE_MAX_KEYS - 1)
            goto Button_Check;
    }

    switch (i) {
    case RC_BTN1: target_mode = AV1_RGBs; break;
    case RC_BTN4: target_mode = AV1_RGsB; break;
    case RC_BTN7: target_mode = AV1_YPBPR; break;
    case RC_BTN2: target_mode = AV2_YPBPR; break;
    case RC_BTN5: target_mode = AV2_RGsB; break;
    case RC_BTN3: target_mode = AV3_RGBHV; break;
    case RC_BTN6: target_mode = AV3_RGBs; break;
    case RC_BTN9: target_mode = AV3_RGsB; break;
    case RC_BTN0: target_mode = AV3_YPBPR; break;
    case RC_MENU:
        menu_active = !menu_active;
        if (menu_active) {
            display_menu(1);
        } else {
            show_submenu = 0;
            lcd_write_status();
        }
        break;
    case RC_BACK:
        if (show_submenu == 0) {
          menu_active = 0;
          lcd_write_status();
        }
        break;
    case RC_INFO:
        sniprintf(menu_row1, LCD_ROW_LEN+1, "VMod: %s", video_modes[cm.id].name);
        //sniprintf(menu_row1, LCD_ROW_LEN+1, "0x%x 0x%x 0x%x", ths_readreg(THS_CH1), ths_readreg(THS_CH2), ths_readreg(THS_CH3));
        sniprintf(menu_row2, LCD_ROW_LEN+1, "LO: %u VSM: %u", IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) & 0xffff, (IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) >> 16) & 0x3);
        lcd_write_menu();
        printf("Mod: %s\n", video_modes[cm.id].name);
        printf("Lines: %u M: %u\n", IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) & 0xffff, cm.macrovis);
        break;
    case RC_LCDBL: IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, (IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE) ^ (1<<1))); break;
    case RC_SL_TGL: tc.sl_mode = tc.sl_mode < SL_MODE_MAX ? tc.sl_mode + 1 : 0; break;
    case RC_SL_MINUS: tc.sl_str = tc.sl_str ? tc.sl_mode - 1 : 0; break;
    case RC_SL_PLUS: tc.sl_str = tc.sl_str < SCANLINESTR_MAX ? tc.sl_mode + 1 : SCANLINESTR_MAX; break;
    default: break;
    }
/*    if (remote_code == rc_keymap[RC_BTN1]) {
        target_mode = AV1_RGBs;
    } else if (remote_code == rc_keymap[RC_BTN4]) {
        target_mode = AV1_RGsB;
    } else if (remote_code == rc_keymap[RC_BTN7]) {
        target_mode = AV1_YPBPR;
    } else if (remote_code == rc_keymap[RC_BTN2]) {
        target_mode = AV2_YPBPR;
    } else if (remote_code == rc_keymap[RC_BTN5]) {
        target_mode = AV2_RGsB;
    } else if (remote_code == rc_keymap[RC_BTN3]) {
        target_mode = AV3_RGBHV;
    } else if (remote_code == rc_keymap[RC_BTN6]) {
        target_mode = AV3_RGBs;
    } else if (remote_code == rc_keymap[RC_BTN9]) {
        target_mode = AV3_RGsB;
    } else if (remote_code == rc_keymap[RC_BTN0]) {
        target_mode = AV3_YPBPR;
    } else if (remote_code == rc_keymap[RC_MENU]) {
        menu_active = !menu_active;

        if (menu_active) {
            display_menu(1);
        } else {
            show_submenu = 0;
            lcd_write_status();
        }
    } else if (~show_submenu && (remote_code == rc_keymap[RC_BACK])) {
        menu_active = 0;
        lcd_write_status();
    } else if (remote_code == rc_keymap[RC_INFO]) {
        sniprintf(menu_row1, LCD_ROW_LEN+1, "VMod: %s", video_modes[cm.id].name);
        //sniprintf(menu_row1, LCD_ROW_LEN+1, "0x%x 0x%x 0x%x", ths_readreg(THS_CH1), ths_readreg(THS_CH2), ths_readreg(THS_CH3));
        sniprintf(menu_row2, LCD_ROW_LEN+1, "LO: %u VSM: %u", IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) & 0xffff, (IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) >> 16) & 0x3);
        lcd_write_menu();
        printf("Mod: %s\n", video_modes[cm.id].name);
        printf("Lines: %u M: %u\n", IORD_ALTERA_AVALON_PIO_DATA(PIO_4_BASE) & 0xffff, cm.macrovis);
    } else if (remote_code == rc_keymap[RC_LCDBL]) {
        IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, (IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE) ^ (1<<1)));
    } else if (remote_code == rc_keymap[RC_SL_TGL]) {
        tc.sl_mode = tc.sl_mode < SL_MODE_MAX ? tc.sl_mode + 1 : 0;
    } else if (remote_code == rc_keymap[RC_SL_MINUS]) {
        if (tc.sl_str > 0)
            tc.sl_str--;
    } else if (remote_code == rc_keymap[RC_SL_PLUS]) {
        if (tc.sl_str < SCANLINESTR_MAX)
            tc.sl_str++;
    }*/
Button_Check:
    if (btn_code_prev == 0) {
        if (btn_code & PB0_BIT)
            target_mode = (cm.avinput == AV3_YPBPR) ? AV1_RGBs : (cm.avinput+1);
        if (btn_code & PB1_BIT)
            tc.sl_mode = tc.sl_mode < SL_MODE_MAX ? tc.sl_mode + 1 : 0;
    }
}
