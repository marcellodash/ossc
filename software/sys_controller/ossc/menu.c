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

#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "string.h"
#include "altera_epcq_controller_mod.h"
#include "lcd.h"
#include "av_controller.h"
#include "controls.h"
#include "memory.h"
#include "menu.h"
#include "menu_list.h"
#include "cfg.h"


typedef enum {
    NO_ACTION    = 0,
    OPT_SELECT   = 1, // remote_code == rc_keymap[RC_OK]
    EXIT_SUBMENU = 2, // remote_code == rc_keymap[RC_BACK]
    PREV_PAGE    = 3, // remote_code == rc_keymap[RC_UP]
    NEXT_PAGE    = 4, // remote_code == rc_keymap[RC_DOWN]
    VAL_MINUS    = 5, // remote_code == rc_keymap[RC_LEFT]
    VAL_PLUS     = 6, // remote_code == rc_keymap[RC_RIGHT]
} menucode_id; // order must the same as in corresponding rc_code_t (controls.h) part

volatile alt_u8 show_submenu = 0;
mainmenu_t *current_mainmanu = &videoproc_mm;
menuitem_t *current_menuitem = &video_lpf_m;

extern char menu_row1[LCD_ROW_LEN+1], menu_row2[LCD_ROW_LEN+1];

extern alt_u16 rc_keymap[REMOTE_MAX_KEYS];
extern volatile alt_u32 remote_code;

extern ypbpr_to_rgb_csc_t csc_coeffs[];



// Target configuration
extern avconfig_t tc;

void display_menu(alt_u8 forcedisp)
{
    menucode_id code = NO_ACTION;
    int retval, i;

    for (i = RC_OK; i < RC_INFO; i++) {
	if (remote_code == rc_keymap[i]) {
            code = i - RC_MENU;
            break;
	}
    }
/*    if (remote_code == rc_keymap[RC_UP])
        code = PREV_PAGE;
    else if (remote_code == rc_keymap[RC_DOWN])
        code = NEXT_PAGE;
    else if (remote_code == rc_keymap[RC_RIGHT])
        code = VAL_PLUS;
    else if (remote_code == rc_keymap[RC_LEFT])
        code = VAL_MINUS;
    else if (remote_code == rc_keymap[RC_OK])
        code = OPT_SELECT;
    else if (remote_code == rc_keymap[RC_BACK])
        code = EXIT_SUBMENU;
    else
        code = NO_ACTION;*/

    if (!forcedisp && (code == NO_ACTION))
        return;
    
    if (show_submenu) {
        if (code == EXIT_SUBMENU) {
            show_submenu = FALSE;
            strncpy(menu_row1, current_mainmanu->desc, LCD_ROW_LEN+1);
            sniprintf(menu_row2, LCD_ROW_LEN+1,  " "); // current_mainmenu must have a submenu here
        } else {
            alt_8 tmp_val = *current_menuitem->effected_avconfig;
            if (code == PREV_PAGE)
                current_menuitem = current_menuitem->previous;
            else if (code == NEXT_PAGE)
                current_menuitem = current_menuitem->next;
            else if (code == VAL_MINUS)
                *current_menuitem->effected_avconfig = tmp_val > current_menuitem->min_value ? tmp_val-1 : current_menuitem->wraparound ? current_menuitem->max_value : tmp_val;
            else if (code == VAL_PLUS)
                *current_menuitem->effected_avconfig = tmp_val < current_menuitem->max_value ? tmp_val+1 : current_menuitem->wraparound ? current_menuitem->min_value : tmp_val;

            if ((current_menuitem->id == TX_MODE) && ((code == VAL_MINUS) || (code == VAL_PLUS))) {
//            if ((current_menuitem->id == TX_MODE) && (!(IORD_ALTERA_AVALON_PIO_DATA(PIO_1_BASE) & HDMITX_MODE_MASK) && ((code == VAL_MINUS) || (code == VAL_PLUS)))) {
//                *current_menuitem->id = TX_DVI; // force stay in DVI mode
                TX_enable(tc.tx_mode);
                SetupAudio(!tc.audio_mute);
            }
            
            strncpy(menu_row1, current_menuitem->desc, LCD_ROW_LEN+1);
            current_menuitem->disp_value_func(*current_menuitem->effected_avconfig);
        }
    } else {
        if (code == PREV_PAGE)
            current_mainmanu = current_mainmanu->previous;
        else if (code == NEXT_PAGE)
            current_mainmanu = current_mainmanu->next;
        
        strncpy(menu_row1, current_mainmanu->desc, LCD_ROW_LEN+1);
        
        switch (current_mainmanu->id) {
        #ifndef DEBUG
            case FW_UPDATE:
                if (code == OPT_SELECT) {
                    retval = fw_update();
                    if (retval == 0) {
                        sniprintf(menu_row1, LCD_ROW_LEN+1, "Fw update OK");
                        sniprintf(menu_row2, LCD_ROW_LEN+1, "Please restart");
                    } else {
                        sniprintf(menu_row1, LCD_ROW_LEN+1, "FW not");
                        sniprintf(menu_row2, LCD_ROW_LEN+1, "updated");
                    }
                } else {
                    sniprintf(menu_row2, LCD_ROW_LEN+1,  " ");
                }
                break;
        #endif
        case RESET_CONFIG:
            if (code == OPT_SELECT) {
                set_default_avconfig();
                sniprintf(menu_row2, LCD_ROW_LEN+1, "Done");
            } else {
                sniprintf(menu_row2, LCD_ROW_LEN+1,  " ");
            }
        case SAVE_CONFIG:
            if (code == OPT_SELECT) {
                retval = write_userdata();
                if (retval == 0) {
                    sniprintf(menu_row2, LCD_ROW_LEN+1, "Done");
                } else {
                    sniprintf(menu_row2, LCD_ROW_LEN+1, "error");
                }
            } else {
                sniprintf(menu_row2, LCD_ROW_LEN+1,  " ");
            }
            break;
        default:
            current_menuitem = current_mainmanu->submenu_start;
            if ((code == OPT_SELECT) || (code == VAL_PLUS)) {
                show_submenu = TRUE;
                strncpy(menu_row1, current_menuitem->desc, LCD_ROW_LEN+1);
                current_menuitem->disp_value_func(*current_menuitem->effected_avconfig);
            } else
                sniprintf(menu_row2, LCD_ROW_LEN+1,  " ");
            break;
        }
    }


    lcd_write_menu();

    return;
}
