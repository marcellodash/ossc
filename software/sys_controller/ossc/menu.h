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

#ifndef MENU_H_
#define MENU_H_

#include "alt_types.h"
#include "controls.h"

typedef enum {
    OPT_AVCONFIG_SELECTION,
    OPT_AVCONFIG_NUMVALUE,
    OPT_SUBMENU,
    OPT_FUNC_CALL,
} menuitem_type;

typedef int (*func_call)(void);
typedef void (*disp_func)(alt_u8);


typedef struct {
    alt_u8 *data;
    alt_u8 wrap_cfg;
    alt_u8 min;
    alt_u8 max;
    const char **setting_str;
} opt_avconfig_selection;

typedef struct {
    alt_u8 *data;
    alt_u8 wrap_cfg;
    alt_u8 min;
    alt_u8 max;
    disp_func f;
} opt_avconfig_numvalue;

typedef struct {
    func_call f;
    char *text_success;
} opt_func_call;

typedef struct menustruct menu_t;

typedef struct {
    char *name;
    menuitem_type type;
    union {
        opt_avconfig_selection sel;
        opt_avconfig_numvalue num;
        const menu_t *sub;
        opt_func_call fun;
    };
} menuitem_t;

struct menustruct {
    alt_u8 num_items;
    menuitem_t *items;
};

typedef struct {
    menu_t *menu;
} opt_submenu;

#define SETTING_ITEM(x) 0, sizeof(x)/sizeof(char*)-1, x
#define MENU(X, Y) menuitem_t X##_items[] = Y; const menu_t X = { sizeof(X##_items)/sizeof(menuitem_t), X##_items };
#define P99_PROTECT(...) __VA_ARGS__

typedef enum {
    NO_ACTION    = 0,
    OPT_SELECT   = RC_OK,
    PREV_MENU    = RC_BACK,
    PREV_PAGE    = RC_UP,
    NEXT_PAGE    = RC_DOWN,
    VAL_MINUS    = RC_LEFT,
    VAL_PLUS     = RC_RIGHT,
} menucode_id; // order must be consequential with rc_code_t

typedef struct {
    const menu_t *m;
    alt_u8 mp;
} menunavi;

void display_menu(alt_u8 forcedisp);

#endif
