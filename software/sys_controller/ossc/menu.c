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

#include <string.h>
#include "menu.h"
#include "av_controller.h"
#include "firmware.h"
#include "userdata.h"
#include "controls.h"
#include "lcd.h"
#include "tvp7002.h"

#define OPT_NOWRAP  0
#define OPT_WRAP    1

extern char row1[LCD_ROW_LEN+1], row2[LCD_ROW_LEN+1], menu_row1[LCD_ROW_LEN+1], menu_row2[LCD_ROW_LEN+1];
extern avconfig_t tc;
extern alt_u16 tc_h_samplerate, tc_h_synclen, tc_h_active, tc_v_active, tc_h_bporch, tc_v_bporch;
extern alt_u32 remote_code;
extern alt_u16 rc_keymap[REMOTE_MAX_KEYS];

alt_u8 menu_active;

static const char *off_on_desc[] = { "Off", "On" };
static const char *video_lpf_desc[] = { "Auto", "Off", "95MHz (HDTV II)", "35MHz (HDTV I)", "16MHz (EDTV)", "9MHz (SDTV)" };
static const char *ypbpr_cs_desc[] = { "Rec. 601", "Rec. 709" };
static const char *s480p_mode_desc[] = { "Auto", "DTV 480p", "VESA 640x480@60" };
static const char *sync_lpf_desc[] = { "Off", "33MHz (min)", "10MHz (med)", "2.5MHz (max)" };
static const char *l3_mode_desc[] = { "Generic 16:9", "Generic 4:3", "320x240 optim.", "256x240 optim." };
static const char *tx_mode_desc[] = { "HDMI", "DVI" };
static const char *sl_mode_desc[] = { "Off", "Auto", "Manual" };
static const char *sl_type_desc[] = { "Horizontal", "Vertical", "Alternating" };
static const char *sl_id_desc[] = { "Top", "Bottom" };
static const char *audio_dw_sampl_desc[] = { "Off (fs = 96kHz)", "2x  (fs = 48kHz)" };

static void sampler_phase_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%d deg", (v*1125)/100); }
static void sync_vth_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%d mV", (v*1127)/100); }
static void intclks_to_time_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%u.%.2u us", (unsigned)(((1000000U*v)/(clkrate[REFCLK_INTCLK]/1000))/1000), (unsigned)((((1000000U*v)/(clkrate[REFCLK_INTCLK]/1000))%1000)/10)); }
static void extclks_to_time_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%u.%.2u us", (unsigned)(((1000000U*v)/(clkrate[REFCLK_EXT27]/1000))/1000), (unsigned)((((1000000U*v)/(clkrate[REFCLK_EXT27]/1000))%1000)/10)); }
static void sl_str_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%u%%", ((v+1)*625)/100); }
static void lines_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%u lines", v); }
static void pixels_disp(alt_u8 v) { sniprintf(menu_row2, LCD_ROW_LEN+1, "%u pixels", v); }

MENU(menu_advtiming, P99_PROTECT({ \
    { "H. samplerate",      OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_h_samplerate, H_TOTAL_MIN,   H_TOTAL_MAX, vm_tweak } } },
    { "H. synclen",         OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_h_synclen,    H_SYNCLEN_MIN, H_SYNCLEN_MAX, vm_tweak } } },
    { "H. active",          OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_h_active,     H_ACTIVE_MIN,  H_ACTIVE_MAX, vm_tweak } } },
    { "V. active",          OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_v_active,     V_ACTIVE_MIN,  V_ACTIVE_MAX, vm_tweak } } },
    { "H. backporch",       OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_h_bporch,     H_BPORCH_MIN,  H_BPORCH_MAX, vm_tweak } } },
    { "V. backporch",       OPT_AVCONFIG_NUMVAL_U16,{ .num_u16 = { &tc_v_bporch,     V_BPORCH_MIN,  V_BPORCH_MAX, vm_tweak } } },
}))


MENU(menu_vinputproc, P99_PROTECT({ \
    { "Video LPF",          OPT_AVCONFIG_SELECTION, { .sel = { &tc.video_lpf,   OPT_WRAP,   SETTING_ITEM(video_lpf_desc) } } },
    { "YPbPr in ColSpa",    OPT_AVCONFIG_SELECTION, { .sel = { &tc.ypbpr_cs,    OPT_WRAP,   SETTING_ITEM(ypbpr_cs_desc) } } },
    { "Auto lev. ctrl",     OPT_AVCONFIG_SELECTION, { .sel = { &tc.en_alc,      OPT_WRAP,   SETTING_ITEM(off_on_desc) } } },
}))

MENU(menu_sampling, P99_PROTECT({ \
    { "Sampling phase",     OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.sampler_phase, OPT_NOWRAP, 0, SAMPLER_PHASE_MAX, sampler_phase_disp } } },
    { "480p in sampler",    OPT_AVCONFIG_SELECTION, { .sel = { &tc.s480p_mode,    OPT_WRAP, SETTING_ITEM(s480p_mode_desc) } } },
    { "<Adv. timing   >",   OPT_SUBMENU,            { .sub = { &menu_advtiming, vm_display } } }, \
}))

MENU(menu_sync, P99_PROTECT({ \
    { "Analog sync LPF",    OPT_AVCONFIG_SELECTION, { .sel = { &tc.sync_lpf,    OPT_WRAP,   SETTING_ITEM(sync_lpf_desc) } } },
    { "Analog sync Vth",    OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.sync_vth,    OPT_NOWRAP, 0, SYNC_VTH_MAX, sync_vth_disp } } },
    { "Hsync window len",   OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.sd_sync_win, OPT_NOWRAP, 0, SD_SYNC_WIN_MAX, extclks_to_time_disp } } },
    { "Vsync threshold",    OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.vsync_thold, OPT_NOWRAP, VSYNC_THOLD_MIN, VSYNC_THOLD_MAX, intclks_to_time_disp } } },
    { "H-PLL Pre-Coast",    OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.pre_coast,   OPT_NOWRAP, 0, PLL_COAST_MAX, lines_disp } } },
    { "H-PLL Post-Coast",   OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.post_coast,  OPT_NOWRAP, 0, PLL_COAST_MAX, lines_disp } } },
}))

MENU(menu_output, P99_PROTECT({ \
    { "240p/288p lineX3",   OPT_AVCONFIG_SELECTION, { .sel = { &tc.linemult_target, OPT_WRAP, SETTING_ITEM(off_on_desc) } } },
    { "Linetriple mode",    OPT_AVCONFIG_SELECTION, { .sel = { &tc.l3_mode,         OPT_WRAP, SETTING_ITEM(l3_mode_desc) } } },
    { "480p/576p lineX2",   OPT_AVCONFIG_SELECTION, { .sel = { &tc.edtv_l2x,        OPT_WRAP, SETTING_ITEM(off_on_desc) } } },
    { "480i/576i passtr",   OPT_AVCONFIG_SELECTION, { .sel = { &tc.interlace_pt,    OPT_WRAP, SETTING_ITEM(off_on_desc) } } },
    { "TX mode",            OPT_AVCONFIG_SELECTION, { .sel = { &tc.tx_mode,         OPT_WRAP, SETTING_ITEM(tx_mode_desc) } } },
    { "Initial input",      OPT_AVCONFIG_SELECTION, { .sel = { &tc.def_input,       OPT_WRAP, SETTING_ITEM(avinput_str) } } },
}))

MENU(menu_postproc, P99_PROTECT({ \
    { "Scanlines",          OPT_AVCONFIG_SELECTION, { .sel = { &tc.sl_mode,     OPT_WRAP,   SETTING_ITEM(sl_mode_desc) } } },
    { "Scanline str.",      OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.sl_str,      OPT_NOWRAP, 0, SCANLINESTR_MAX, sl_str_disp } } },
    { "Scanline type",      OPT_AVCONFIG_SELECTION, { .sel = { &tc.sl_type,     OPT_WRAP,   SETTING_ITEM(sl_type_desc) } } },
    { "Scanline alignm.",   OPT_AVCONFIG_SELECTION, { .sel = { &tc.sl_id,       OPT_WRAP,   SETTING_ITEM(sl_id_desc) } } },
    { "Horizontal mask",    OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.h_mask,      OPT_NOWRAP, 0, HV_MASK_MAX, pixels_disp } } },
    { "Vertical mask",      OPT_AVCONFIG_NUMVALUE,  { .num = { &tc.v_mask,      OPT_NOWRAP, 0, HV_MASK_MAX, pixels_disp } } },
}))

MENU(menu_audio, P99_PROTECT({ \
    { "Down-sampling",      OPT_AVCONFIG_SELECTION, { .sel = { &tc.audio_dw_sampl, OPT_WRAP, SETTING_ITEM(audio_dw_sampl_desc) } } },
    { "Swap left/right",    OPT_AVCONFIG_SELECTION, { .sel = { &tc.audio_swap_lr,  OPT_WRAP, SETTING_ITEM(off_on_desc) } } },
    { "Mute",               OPT_AVCONFIG_SELECTION, { .sel = { &tc.audio_mute,     OPT_WRAP, SETTING_ITEM(off_on_desc) } } },
}))

MENU(menu_main, P99_PROTECT({ \
    { "Video in proc  >",   OPT_SUBMENU,            { .sub = { &menu_vinputproc, NULL } } }, \
    { "Sampling opt.  >",   OPT_SUBMENU,            { .sub = { &menu_sampling, NULL } } }, \
    { "Sync opt.      >",   OPT_SUBMENU,            { .sub = { &menu_sync, NULL } } }, \
    { "Output opt.    >",   OPT_SUBMENU,            { .sub = { &menu_output, NULL } } }, \
    { "Post-proc.     >",   OPT_SUBMENU,            { .sub = { &menu_postproc, NULL } } }, \
    { "Audio options  >",   OPT_SUBMENU,            { .sub = { &menu_audio, NULL } } }, \
    { "<Fw. update    >",   OPT_FUNC_CALL,          { .fun = { fw_update, "OK - pls restart" } } }, \
    { "<Reset settings>",   OPT_FUNC_CALL,          { .fun = { set_default_avconfig, "Reset done" } } }, \
    { "<Save settings >",   OPT_FUNC_CALL,          { .fun = { write_userdata, "Saved" } } }, \
}))

// Max 3 levels currently
menunavi navi[] = {{&menu_main, 0}, {NULL, 0}, {NULL, 0}};
alt_u8 navlvl = 0;


void display_menu(alt_u8 forcedisp)
{
    menucode_id code = NO_ACTION;
    menuitem_type type;
    alt_u8 *val, val_wrap, val_min, val_max;
    alt_u16 *val_u16;
    int i, retval = 0;

    for (i=RC_OK; i < RC_INFO; i++) {
        if (remote_code == rc_keymap[i]) {
            code = i;
            break;
        }
    }

    if (!forcedisp && (code == NO_ACTION))
        return;

    type = navi[navlvl].m->items[navi[navlvl].mp].type;

    // Parse menu control
    switch (code) {
    case PREV_PAGE:
        navi[navlvl].mp = (navi[navlvl].mp == 0) ? navi[navlvl].m->num_items-1 : (navi[navlvl].mp-1);
        break;
    case NEXT_PAGE:
        navi[navlvl].mp = (navi[navlvl].mp+1) % navi[navlvl].m->num_items;
        break;
    case PREV_MENU:
        if (navlvl > 0) {
            navlvl--;
        } else {
            menu_active = 0;
            lcd_write_status();
            return;
        }
        break;
    case OPT_SELECT:
        switch (navi[navlvl].m->items[navi[navlvl].mp].type) {
            case OPT_SUBMENU:
                if (navi[navlvl+1].m != navi[navlvl].m->items[navi[navlvl].mp].sub.menu)
                    navi[navlvl+1].mp = 0;
                navi[navlvl+1].m = navi[navlvl].m->items[navi[navlvl].mp].sub.menu;
                if (navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f)
                    navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f(code);
                navlvl++;
                break;
            case OPT_FUNC_CALL:
                retval = navi[navlvl].m->items[navi[navlvl].mp].fun.f();
                break;
            default:
                break;
        }
        break;
    case VAL_MINUS:
    case VAL_PLUS:
        switch (navi[navlvl].m->items[navi[navlvl].mp].type) {
            case OPT_AVCONFIG_SELECTION:
            case OPT_AVCONFIG_NUMVALUE:
                val = navi[navlvl].m->items[navi[navlvl].mp].sel.data;
                val_wrap = navi[navlvl].m->items[navi[navlvl].mp].sel.wrap_cfg;
                val_min = navi[navlvl].m->items[navi[navlvl].mp].sel.min;
                val_max = navi[navlvl].m->items[navi[navlvl].mp].sel.max;

                if (code == VAL_MINUS)
                    *val = (*val > val_min) ? (*val-1) : (val_wrap ? val_max : val_min);
                else
                    *val = (*val < val_max) ? (*val+1) : (val_wrap ? val_min : val_max);
                break;
            case OPT_AVCONFIG_NUMVAL_U16:
                val_u16 = navi[navlvl].m->items[navi[navlvl].mp].num_u16.data;
                if (code == VAL_MINUS)
                    *val_u16 = (*val_u16 > navi[navlvl].m->items[navi[navlvl].mp].num_u16.min) ? (*val_u16-1) : *val_u16;
                else
                    *val_u16 = (*val_u16 < navi[navlvl].m->items[navi[navlvl].mp].num_u16.max) ? (*val_u16+1) : *val_u16;
                break;
            case OPT_SUBMENU:
                if (navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f)
                    navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f(code);
                break;
            default:
                break;
        }
        break;
    default:
        break;
    }

    // Generate menu text
    type = navi[navlvl].m->items[navi[navlvl].mp].type;
    strncpy(menu_row1, navi[navlvl].m->items[navi[navlvl].mp].name, LCD_ROW_LEN+1);
    switch (navi[navlvl].m->items[navi[navlvl].mp].type) {
        case OPT_AVCONFIG_SELECTION:
            strncpy(menu_row2, navi[navlvl].m->items[navi[navlvl].mp].sel.setting_str[*(navi[navlvl].m->items[navi[navlvl].mp].sel.data)], LCD_ROW_LEN+1);
            break;
        case OPT_AVCONFIG_NUMVALUE:
            navi[navlvl].m->items[navi[navlvl].mp].num.df(*(navi[navlvl].m->items[navi[navlvl].mp].num.data));
            break;
        case OPT_AVCONFIG_NUMVAL_U16:
            navi[navlvl].m->items[navi[navlvl].mp].num_u16.df(*(navi[navlvl].m->items[navi[navlvl].mp].num_u16.data));
            break;
        case OPT_SUBMENU:
            if (navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f)
                navi[navlvl].m->items[navi[navlvl].mp].sub.arg_f(NO_ACTION);
            else
                menu_row2[0] = 0;
            break;
        case OPT_FUNC_CALL:
            if (code == OPT_SELECT)
                sniprintf(menu_row2, LCD_ROW_LEN+1, "%s", (retval==0) ? navi[navlvl].m->items[navi[navlvl].mp].fun.text_success : "failed");
            else
                 menu_row2[0] = 0;
            break;
        default:
            break;
    }

    lcd_write_menu();
}
