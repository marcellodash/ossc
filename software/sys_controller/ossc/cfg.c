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

#include "string.h"
#include "cfg.h"
#include "av_controller.h"

#define DEFAULT_ON             1
#define DEFAULT_PRE_COAST      1
#define DEFAULT_POST_COAST     0
#define DEFAULT_SAMPLER_PHASE 16
#define DEFAULT_SYNC_VTH      11

avconfig_t tc_default = {
  .sl_mode = 0,
  .sl_str = 0,
  .sl_id = 0,
  .linemult_target = 0,
  .l3_mode = 0,
  .h_mask = 0,
  .v_mask = 0,
  .tx_mode = 0,
  .s480p_mode = 0,
  .sampler_phase = DEFAULT_SAMPLER_PHASE,
  .ypbpr_cs = 0,
  .sync_vth = DEFAULT_SYNC_VTH,
  .vsync_thold = DEFAULT_VSYNC_THOLD,
  .sync_lpf = 0,
  .video_lpf = 0,
  .en_alc = DEFAULT_ON,
  .pre_coast = DEFAULT_PRE_COAST,
  .post_coast = DEFAULT_POST_COAST,
  .audio_dw_sampl = DEFAULT_ON,
  .audio_swap_lr = 0,
  .audio_mute = 0
};

// Target configuration
extern avconfig_t tc;

int set_default_avconfig()
{
    memcpy(&tc, &tc_default, sizeof(avconfig_t));
    return 0;
}
