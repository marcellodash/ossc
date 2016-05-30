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

#ifndef MEMORY_H_
#define MEMORY_H_

#include <alt_types.h>

#define USERDATA_HDR_SIZE 11
typedef struct {
    char   userdata_key[8];
    alt_u8 version_major;
    alt_u8 version_minor;
    alt_u8 num_entries;
} userdata_hdr;

#define USERDATA_ENTRY_HDR_SIZE 2
typedef struct {
    alt_u8 type;
    alt_u8 entry_len;
} userdata_entry;

typedef enum {
    UDE_REMOTE_MAP  = 0,
    UDE_AVCONFIG,
} userdata_entry_type;


typedef struct {
    char    fw_key[4];
    alt_u8  version_major;
    alt_u8  version_minor;
    char    version_suffix[8];
    alt_u32 hdr_len;
    alt_u32 data_len;
    alt_u32 data_crc;
    alt_u32 hdr_crc;
} fw_hdr;


int check_fw_header(alt_u8 *databuf, fw_hdr *hdr);
int check_fw_image(alt_u32 offset, alt_u32 size, alt_u32 golden_crc, alt_u8 *tmpbuf);
int fw_update(void);

int write_userdata(void);
int read_userdata(void);

#endif /* MEMORY_H_ */
