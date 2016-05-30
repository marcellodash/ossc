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

#ifndef CFG_H_
#define CFG_H_

#include "altera_epcq_controller_mod.h"

typedef struct {
    alt_8 sl_mode;
    alt_8 sl_str;
    alt_8 sl_id;
    alt_8 linemult_target;
    alt_8 l3_mode;
    alt_8 h_mask;
    alt_8 v_mask;
    alt_8 tx_mode;
    alt_8 s480p_mode;
    alt_8 sampler_phase;
    alt_8 ypbpr_cs;
    alt_8 sync_vth;
    alt_8 vsync_thold;
    alt_8 sync_lpf;
    alt_8 video_lpf;
    alt_8 en_alc;
    alt_8 pre_coast;
    alt_8 post_coast;
    alt_8 audio_dw_sampl;
    alt_8 audio_swap_lr;
    alt_8 audio_mute;
} avconfig_t;

#define MAINLOOP_SLEEP_US 10000

#define CONV_SINGED_UNSINGED_OFFSET 127

#define SCANLINESTR_MAX    15
#define HV_MASK_MAX        63
#define L3_MODE_MAX         3
#define S480P_MODE_MAX      2
#define SL_MODE_MAX         2
#define SYNC_LPF_MAX        3
#define VIDEO_LPF_MAX       5
#define SAMPLER_PHASE_MIN -16
#define SAMPLER_PHASE_MAX  15
#define SYNC_VTH_MIN      -11
#define SYNC_VTH_MAX       20
#define VSYNC_THOLD_MIN  ( 10 - CONV_SINGED_UNSINGED_OFFSET)
#define VSYNC_THOLD_MAX  (200 - CONV_SINGED_UNSINGED_OFFSET)
#define PLL_COAST_MIN       0
#define PLL_COAST_MAX       5

void set_default_avconfig(void);

#endif /* CFG_H_ */
