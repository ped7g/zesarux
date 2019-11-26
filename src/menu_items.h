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

#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "cpu.h"


extern void menu_poke(MENU_ITEM_PARAMETERS);
extern void menu_settings_debug(MENU_ITEM_PARAMETERS);
extern void menu_settings_audio(MENU_ITEM_PARAMETERS);
extern void menu_zxvision_test(MENU_ITEM_PARAMETERS);
extern void menu_mem_breakpoints(MENU_ITEM_PARAMETERS);
extern int menu_cond_ay_chip(void);
extern int last_debug_poke_dir;
extern void menu_debug_poke(MENU_ITEM_PARAMETERS);
extern void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS);
extern void menu_about_core_statistics(MENU_ITEM_PARAMETERS);
extern void menu_ay_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_videoregisters(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_spritenav(MENU_ITEM_PARAMETERS);
extern void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS);
extern void menu_audio_new_waveform(MENU_ITEM_PARAMETERS);
extern void menu_debug_new_visualmem(MENU_ITEM_PARAMETERS);
extern void menu_audio_new_ayplayer(MENU_ITEM_PARAMETERS);
extern int menu_audio_new_ayplayer_si_mostrar(void);
extern void menu_debug_hexdump(MENU_ITEM_PARAMETERS);
extern void menu_osd_adventure_keyboard(MENU_ITEM_PARAMETERS);
extern void menu_osd_adventure_keyboard_next(void);
extern void menu_debug_dma_tsconf_zxuno(MENU_ITEM_PARAMETERS);
extern void menu_display_total_palette(MENU_ITEM_PARAMETERS);

extern void menu_debug_disassemble(MENU_ITEM_PARAMETERS);
extern void menu_debug_assemble(MENU_ITEM_PARAMETERS);

extern void menu_cpu_settings(MENU_ITEM_PARAMETERS);
extern void menu_settings_display(MENU_ITEM_PARAMETERS);

extern void menu_draw_background_windows(MENU_ITEM_PARAMETERS);
extern void menu_debug_cpu_stats(MENU_ITEM_PARAMETERS);
extern void menu_tbblue_machine_id(MENU_ITEM_PARAMETERS);

extern void menu_ext_desktop_settings(MENU_ITEM_PARAMETERS);
extern void menu_cpu_transaction_log(MENU_ITEM_PARAMETERS);

extern void menu_debug_view_sprites(MENU_ITEM_PARAMETERS);

extern void menu_debug_registers(MENU_ITEM_PARAMETERS);
extern void menu_debug_registers_splash_memory_zone(void);
extern void menu_breakpoint_fired(char *s);

extern void menu_ay_partitura(MENU_ITEM_PARAMETERS);
extern void menu_record_mid(MENU_ITEM_PARAMETERS);

extern void menu_direct_midi_output(MENU_ITEM_PARAMETERS);
extern void menu_ay_mixer(MENU_ITEM_PARAMETERS);
extern void menu_uartbridge(MENU_ITEM_PARAMETERS);
extern void menu_network(MENU_ITEM_PARAMETERS);
extern void menu_settings_statistics(MENU_ITEM_PARAMETERS);

extern void menu_debug_change_memory_zone_splash(void);


extern void menu_zeng_send_message(MENU_ITEM_PARAMETERS);
extern int menu_zeng_send_message_cond(void);
extern int menu_zsock_http(char *host, char *url,int *http_code,char **mem,int *t_leidos, char **mem_after_headers,int skip_headers,char *add_headers,int use_ssl,char *redirect_url);
extern int menu_download_wos(char *host,char *url,char *archivo_temp,int ssl_use);

#endif

