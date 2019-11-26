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

#ifndef SNAP_ZSF_H
#define SNAP_ZSF_H

extern void load_zsf_snapshot(char *filename);
extern void save_zsf_snapshot(char *filename);

extern char *zsf_get_block_id_name(int block_id);

extern char zsf_magic_header[];

extern int zsf_force_uncompressed;

extern void save_zsf_snapshot_file_mem(char *filename,z80_byte *destination_memory,int *longitud_total);
extern void load_zsf_snapshot_file_mem(char *filename,z80_byte *origin_memory,int longitud_memoria,int load_fast_mode);

extern z80_byte *pending_zrcp_put_snapshot_buffer_destino;
extern int pending_zrcp_put_snapshot_longitud;
extern void check_pending_zrcp_put_snapshot(void);

#endif
