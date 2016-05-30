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
#include "altera_avalon_pio_regs.h"
#include "Altera_UP_SD_Card_Avalon_Interface_mod.h"
#include "altera_epcq_controller_mod.h"
#include "i2c_opencores.h"
#include "tvp7002.h"
#include "ths7353.h"
#include "lcd.h"
#include "sysconfig.h"
#include "hdmitx.h"
#include "ci_crc.h"
#include "cfg.h"
#include "av_controller.h"
#include "controls.h"
#include "memory.h"
#include "menu.h"
#include "fw_version.h"


extern const char const *avinput_str[];

extern char row1[LCD_ROW_LEN+1], row2[LCD_ROW_LEN+1], menu_row1[LCD_ROW_LEN+1], menu_row2[LCD_ROW_LEN+1];

extern alt_u16 rc_keymap[REMOTE_MAX_KEYS];

extern alt_u32 remote_code;
extern alt_u8 remote_rpt, remote_rpt_prev;
extern alt_u32 btn_code, btn_code_prev;

extern alt_u8 menu_active;

extern alt_u8 target_typemask;

// Target configuration
extern avconfig_t tc;

// Current mode
extern avmode_t cm;

extern avinput_t target_mode;

int main()
{
    tvp_input_t target_input = 0;
    ths_input_t target_ths = 0;
    video_format target_format = 0;

    alt_u8 av_init = 0;
    status_t status;

    alt_u32 input_vec;

    int init_stat;

    init_stat = init_hw();

    if (init_stat >= 0) {
        printf("### DIY VIDEO DIGITIZER / SCANCONVERTER INIT OK ###\n\n");
        sniprintf(row1, LCD_ROW_LEN+1, "OSSC  fw. %u.%.2ua", FW_VER_MAJOR, FW_VER_MINOR);
#ifndef DEBUG
        strncpy(row2, "2014-2016  marqs", LCD_ROW_LEN+1);
#else
        strncpy(row2, "** DEBUG BUILD *", LCD_ROW_LEN+1);
#endif
        lcd_write_status();
    } else {
        sniprintf(row1, LCD_ROW_LEN+1, "Init error  %d", init_stat);
        strncpy(row2, "", LCD_ROW_LEN+1);
        lcd_write_status();
        while (1) {}
    }

    // Mainloop
    while(1) {
        // Read remote control and PCB button status
        input_vec = IORD_ALTERA_AVALON_PIO_DATA(PIO_1_BASE);
        remote_code = input_vec & RC_MASK;
        btn_code = ~input_vec & PB_MASK;
        remote_rpt = input_vec >> 24;
        target_mode = AV_KEEP;

        if ((remote_rpt == 0) || ((remote_rpt > 1) && (remote_rpt < 6)) || (remote_rpt == remote_rpt_prev))
            remote_code = 0;

        parse_control();

        if (menu_active)
            display_menu(0);

        if (target_mode == cm.avinput)
            target_mode = AV_KEEP;

        if (target_mode != AV_KEEP) {
            if (target_input > 5) {
                target_format = (target_mode == AV3_RGBHV) ? FORMAT_RGBHV : (target_mode == AV3_RGBs) ? FORMAT_RGBS : (target_mode == AV3_RGsB) ? FORMAT_RGsB : FORMAT_YPbPr;
        	target_ths = THS_STANDBY;
            } else if (target_input > 3) {
            	target_format = (target_mode == AV2_YPBPR) ? FORMAT_YPbPr : FORMAT_RGsB;
            	target_ths = THS_INPUT_A;
            } else {
            	target_format = (target_mode == AV1_RGBs) ? FORMAT_RGBS : (target_mode == AV1_RGsB) ? FORMAT_RGsB : FORMAT_YPbPr;
            	target_ths = THS_INPUT_B;
            }
            target_input = (target_mode > 5) ? TVP_INPUT3 : TVP_INPUT1;
//            target_format = ((target_mode == AV1_RGBs) || (target_mode == AV3_RGBs)) ? FORMAT_RGBS : ((target_mode == AV1_RGsB) || (target_mode == AV2_RGsB) || (target_mode == AV3_RGsB)) ? FORMAT_RGsB : (target_mode == AV3_RGBHV) ? FORMAT_RGBHV : FORMAT_YPbPr;
            target_typemask = (target_mode == AV3_RGBHV) ? VIDEO_PC : VIDEO_LDTV|VIDEO_SDTV|VIDEO_EDTV|VIDEO_HDTV;
//            target_ths = (target_mode > 5) ? THS_STANDBY : (target_mode > 3) ? THS_INPUT_A : THS_INPUT_B;

            printf("### SWITCH MODE TO %s ###\n", avinput_str[target_mode]);
            av_init = 1;
            cm.avinput = target_mode;
            cm.sync_active = 0;
            ths_source_sel(target_ths, (cm.cc.video_lpf > 1) ? (VIDEO_LPF_MAX-cm.cc.video_lpf) : THS_LPF_BYPASS);
            tvp_disable_output();
            DisableAudioOutput();
            tvp_source_sel(target_input, target_format);
            cm.clkcnt = 0; //TODO: proper invalidate
            strncpy(row1, avinput_str[cm.avinput], LCD_ROW_LEN+1);
            strncpy(row2, "    NO SYNC", LCD_ROW_LEN+1);
            if (!menu_active)
                lcd_write_status();
        }

        // Check here to enable regardless of av_init
        if (tc.tx_mode != cm.cc.tx_mode) {
            TX_enable(tc.tx_mode);
            cm.cc.tx_mode = tc.tx_mode;
        }

        if (av_init) {
            status = get_status(target_input, target_format);

            switch (status) {
            case ACTIVITY_CHANGE:
                if (cm.sync_active) {
                    printf("Sync up\n");
                    IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, (IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE) | (1<<2)));  //disable videogen
                    enable_outputs();
                } else {
                    printf("Sync lost\n");
                    cm.clkcnt = 0; //TODO: proper invalidate
                    tvp_disable_output();
                    DisableAudioOutput();
                    //ths_source_sel(THS_STANDBY, 0);
                    strncpy(row1, avinput_str[cm.avinput], LCD_ROW_LEN+1);
                    strncpy(row2, "    NO SYNC", LCD_ROW_LEN+1);
                    if (!menu_active)
                        lcd_write_status();
                }
                break;
            case MODE_CHANGE:
                if (cm.sync_active) {
                    printf("Mode change\n");
                    program_mode();
                }
                break;
            case INFO_CHANGE:
                if (cm.sync_active) {
                    printf("Info change\n");
                    set_videoinfo();
                }
                break;
            default:
                break;
            }
        }

        btn_code_prev = btn_code;
        remote_rpt_prev = remote_rpt;
        usleep(300);    // Avoid executing mainloop multiple times per vsync
    }

    return 0;
}
