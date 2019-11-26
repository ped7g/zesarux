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

#ifndef CPC_H
#define CPC_H


#include "cpu.h"

extern z80_byte cpc_gate_registers[];

extern z80_byte *cpc_rom_mem_table[];

extern z80_byte *cpc_ram_mem_table[];

extern z80_byte *cpc_memory_paged_read[];
extern z80_byte *cpc_memory_paged_write[];

#define CPC_MEMORY_TYPE_ROM 0
#define CPC_MEMORY_TYPE_RAM 1

extern z80_byte debug_cpc_type_memory_paged_read[];
extern z80_byte debug_cpc_paginas_memoria_mapeadas_read[];
extern void init_cpc_line_display_table(void);

//Hacer que estos valores de border sean multiples de 8
#define CPC_LEFT_BORDER_NO_ZOOM 48
#define CPC_TOP_BORDER_NO_ZOOM 24

#define CPC_LEFT_BORDER CPC_LEFT_BORDER_NO_ZOOM*zoom_x
#define CPC_TOP_BORDER CPC_TOP_BORDER_NO_ZOOM*zoom_y

//#define CPC_LEFT_BORDER 0
//#define CPC_TOP_BORDER 0
#define CPC_DISPLAY_WIDTH 640
#define CPC_DISPLAY_HEIGHT 400

extern z80_int cpc_line_display_table[];


extern int cpc_rgb_table[];

extern z80_byte cpc_palette_table[];

extern z80_byte cpc_keyboard_table[];

extern z80_byte cpc_border_color;

extern void cpc_out_port_gate(z80_byte value);

extern void cpc_out_ppi(z80_byte puerto_h,z80_byte value);

extern z80_byte cpc_in_ppi(z80_byte puerto_h);

extern void cpc_set_memory_pages();

extern void cpc_init_memory_tables();

extern void cpc_out_port_crtc(z80_int puerto,z80_byte value);

extern z80_byte cpc_crtc_registers[];

extern z80_byte cpc_ppi_ports[];

extern z80_byte cpc_crtc_last_selected_register;

extern z80_byte cpc_scanline_counter;

extern z80_bit cpc_forzar_modo_video;
extern z80_byte cpc_forzar_modo_video_modo;

extern void cpc_splash_videomode_change(void);

extern z80_bit cpc_send_double_vsync;

extern void cpc_handle_vsync_state(void);

#endif
