/*
    ZEsarUX  ZX Second-Emulator And Released for UniX 
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "cpu.h"

#define JOYSTICK_TOTAL 11
#define JOYSTICK_KEMPSTON 1
#define JOYSTICK_SINCLAIR_1 2
#define JOYSTICK_SINCLAIR_2 3
#define JOYSTICK_CURSOR 4
#define JOYSTICK_CURSOR_WITH_SHIFT 5
#define JOYSTICK_OPQA_SPACE 6
#define JOYSTICK_FULLER 7
#define JOYSTICK_ZEBRA 8
#define JOYSTICK_MIKROGEN 9
#define JOYSTICK_ZXPAND 10
#define JOYSTICK_CURSOR_SAM 11


extern z80_byte puerto_especial_joystick;

extern int joystick_emulation;
extern int joystick_autofire_frequency;
extern int joystick_autofire_counter;


extern char *joystick_texto[];

extern void joystick_set_right(int si_enviar_zeng_event);
extern void joystick_release_right(int si_enviar_zeng_event);
extern void joystick_set_left(int si_enviar_zeng_event);
extern void joystick_release_left(int si_enviar_zeng_event);
extern void joystick_set_down(int si_enviar_zeng_event);
extern void joystick_release_down(int si_enviar_zeng_event);
extern void joystick_set_up(int si_enviar_zeng_event);
extern void joystick_release_up(int si_enviar_zeng_event);
extern void joystick_set_fire(int si_enviar_zeng_event);
extern void joystick_release_fire(int si_enviar_zeng_event);

extern int gunstick_emulation;

#define GUNSTICK_TOTAL 4
#define GUNSTICK_SINCLAIR_1 1
#define GUNSTICK_SINCLAIR_2 2
#define GUNSTICK_KEMPSTON 3
#define GUNSTICK_AYCHIP 4

//Puerto DF como lightgun no implementado
//#define GUNSTICK_PORT_DF 5

extern char *gunstick_texto[];

//extern z80_byte puerto_especial_gunstick;
extern int gunstick_x;
extern int gunstick_y;
extern int gunstick_view_white(void);
extern int gunstick_view_electron(void);

extern int gunstick_range_x,gunstick_range_y,gunstick_y_offset,gunstick_solo_brillo;



extern int mouse_x,mouse_y;
extern int mouse_wheel_vertical,mouse_wheel_horizontal;
extern z80_bit kempston_mouse_emulation;
extern int mouse_left,mouse_right,mouse_middle,mouse_pressed_close_window;

extern z80_byte kempston_mouse_x,kempston_mouse_y;

extern void joystick_print_types(void);

extern void joystick_cycle_next_type(void);



#endif
