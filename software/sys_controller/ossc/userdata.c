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
#include "userdata.h"
#include "flash.h"
#include "firmware.h"
#include "controls.h"
#include "av_controller.h"

extern alt_u16 rc_keymap[REMOTE_MAX_KEYS];
extern avconfig_t tc;
extern mode_data_t video_modes[];
extern alt_u8 update_cur_vm;

int write_userdata(alt_u8 entry)
{
    alt_u8 databuf[PAGESIZE];
    alt_u16 vm_to_write;
    alt_u16 pageoffset, srcoffset;
    alt_u8 pageno;
    int retval;

    if (entry > MAX_USERDATA_ENTRY) {
        printf("invalid entry\n");
        return -1;
    }

    strncpy(((ude_hdr*)databuf)->userdata_key, "USRDATA", 8);
    ((ude_hdr*)databuf)->version_major = FW_VER_MAJOR;
    ((ude_hdr*)databuf)->version_minor = FW_VER_MINOR;
    ((ude_hdr*)databuf)->type = (entry > MAX_PROFILE) ? UDE_REMOTE_MAP : UDE_PROFILE;

    switch (((ude_hdr*)databuf)->type) {
    case UDE_REMOTE_MAP:
        ((ude_remote_map*)databuf)->data_len = sizeof(rc_keymap);
        memcpy(((ude_remote_map*)databuf)->keys, rc_keymap, sizeof(rc_keymap));
        retval = write_flash_page(databuf, sizeof(ude_remote_map), (USERDATA_OFFSET+entry*SECTORSIZE)/PAGESIZE);
        if (retval != 0) {
            return -1;
        }
        break;
    case UDE_PROFILE:
        vm_to_write = VIDEO_MODES_SIZE;
        ((ude_profile*)databuf)->avc_data_len = sizeof(avconfig_t);
        ((ude_profile*)databuf)->vm_data_len = vm_to_write;

        pageno = 0;
        pageoffset = offsetof(ude_profile, avc);

        // assume that sizeof(avconfig_t) << PAGESIZE
        memcpy(databuf+pageoffset, &tc, sizeof(avconfig_t));
        pageoffset += sizeof(avconfig_t);

        srcoffset = 0;
        while (vm_to_write > 0) {
            if (vm_to_write >= PAGESIZE-pageoffset) {
                memcpy(databuf+pageoffset, (char*)video_modes+srcoffset, PAGESIZE-pageoffset);
                srcoffset += PAGESIZE-pageoffset;
                pageoffset = 0;
                vm_to_write -= PAGESIZE-pageoffset;
                // check
                write_flash_page(databuf, PAGESIZE, ((USERDATA_OFFSET+entry*SECTORSIZE)/PAGESIZE) + pageno);
                pageno++;
            } else {
                memcpy(databuf+pageoffset, (char*)video_modes+srcoffset, vm_to_write);
                pageoffset += vm_to_write;
                vm_to_write = 0;
                // check
                write_flash_page(databuf, PAGESIZE, ((USERDATA_OFFSET+entry*SECTORSIZE)/PAGESIZE) + pageno);
            }
        }
        printf("Profile %u data written (%u bytes)\n", entry, sizeof(avconfig_t)+VIDEO_MODES_SIZE);
        break;
    default:
        break;
    }

    return 0;
}

int read_userdata(alt_u8 entry)
{
    int retval, i;
    alt_u8 databuf[PAGESIZE];
    alt_u16 vm_to_read;
    alt_u16 pageoffset, dstoffset;
    alt_u8 pageno;

    if (entry > MAX_USERDATA_ENTRY) {
        printf("invalid entry\n");
        return -1;
    }

    retval = read_flash(USERDATA_OFFSET+(entry*SECTORSIZE), PAGESIZE, databuf);
    if (retval != 0) {
        printf("Flash read error\n");
        return -1;
    }

    if (strncmp(((ude_hdr*)databuf)->userdata_key, "USRDATA", 8)) {
        printf("No userdata found on entry %u\n", entry);
        return 1;
    }

    if ((((ude_hdr*)databuf)->version_major != FW_VER_MAJOR) || (((ude_hdr*)databuf)->version_minor != FW_VER_MINOR)) {
        printf("Data version %u.%u does not match fw\n", ((ude_hdr*)databuf)->version_major, ((ude_hdr*)databuf)->version_minor);
        return 2;
    }

    switch (((ude_hdr*)databuf)->type) {
    case UDE_REMOTE_MAP:
        if (((ude_remote_map*)databuf)->data_len == sizeof(rc_keymap)) {
            memcpy(rc_keymap, ((ude_remote_map*)databuf)->keys, sizeof(rc_keymap));
            printf("RC data read (%u bytes)\n", sizeof(rc_keymap));
        }
        break;
    case UDE_PROFILE:
        if ((((ude_profile*)databuf)->avc_data_len == sizeof(avconfig_t)) && (((ude_profile*)databuf)->vm_data_len == VIDEO_MODES_SIZE)) {
            vm_to_read = ((ude_profile*)databuf)->vm_data_len;

            pageno = 0;
            pageoffset = offsetof(ude_profile, avc);

            // assume that sizeof(avconfig_t) << PAGESIZE
            memcpy(&tc, databuf+pageoffset, sizeof(avconfig_t));
            pageoffset += sizeof(avconfig_t);

            dstoffset = 0;
            while (vm_to_read > 0) {
                if (vm_to_read >= PAGESIZE-pageoffset) {
                    memcpy((char*)video_modes+dstoffset, databuf+pageoffset, PAGESIZE-pageoffset);
                    dstoffset += PAGESIZE-pageoffset;
                    pageoffset = 0;
                    vm_to_read -= PAGESIZE-pageoffset;
                    pageno++;
                    // check
                    read_flash(USERDATA_OFFSET+(entry*SECTORSIZE)+pageno*PAGESIZE, PAGESIZE, databuf);
                } else {
                    memcpy((char*)video_modes+dstoffset, databuf+pageoffset, vm_to_read);
                    pageoffset += vm_to_read;
                    vm_to_read = 0;
                }
            }
            update_cur_vm = 1;

            printf("Profile %u data read (%u bytes)\n", entry, sizeof(avconfig_t)+VIDEO_MODES_SIZE);
        }
        break;
    default:
        printf("Unknown userdata entry\n");
        break;
    }

    return 0;
}
