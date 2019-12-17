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

#ifndef DATAGEAR_H
#define DATAGEAR_H

#include "cpu.h"

#define DATAGEAR_DMA_FIRST_PORT 0x0b
#define DATAGEAR_DMA_SECOND_PORT 0x6b

#define ZXNDMA_MODE_BYTE		0
#define ZXNDMA_MODE_CONTINUOUS	1
#define ZXNDMA_MODE_BURST		2
#define ZXNDMA_MODE_INVALID		3

struct s_zxndma_port {
	// wr_address is set by WR0/WR4, address is the working register (RR3..RR6)
	z80_int		wr_address, address;
	// 0vAAT000 AA=increment type, T = type, v = variable timing
	z80_byte	config;
	// when v=1: **s0**cc s = prescalar (port B only), cc = fixed timing, * = Zilog DMA specific
	z80_byte	timing;
};
//public API for `s_zxndma_port` structure
extern int zxndma_is_port_io(const struct s_zxndma_port* const port);
extern int zxndma_get_port_increment_type(const struct s_zxndma_port* const port);
extern int zxndma_get_port_cycles(const struct s_zxndma_port* const port);

// main structure holding everything what zxnDMA emulation requires
struct s_zxndma {
	// RR registers
	z80_int					counter;	// bytes already transferred (RR1..RR2)
	z80_byte				status;		// RR0
		// bit 7 is emulation-abused as "enabled/disabled transfer"
		// zxnDMA return status as 00E1101T (E=whole block at least once=0, T=some byte=1)

	// write registers and values
	z80_byte				wr0;
	z80_int					length;
	struct s_zxndma_port	portA;
	struct s_zxndma_port	portB;
	z80_byte				prescalar;	// only in zxndma mode (not in Zilog)
	z80_byte				wr3;		// only "dma enable" (0x40) implemented in zxndma
	z80_byte				_mask_byte;					// not used by zxndma
	z80_byte				_match_byte;				// not used by zxndma
	z80_byte				wr4;		// continuous/burst mode, portB address, interrupt controls
	z80_byte				_interrupt_control_byte;	// not used by zxndma
	z80_byte				_icb_pulse_control_byte;	// not used by zxndma
	z80_byte				_icb_interrupt_vector;		// not used by zxndma
	z80_byte				wr5;		// stop/restart, signals config
	z80_byte				wr6;		// commands
	z80_byte				wr6_read_mask;	// read mask set by 0xBB command

	// emulator related values
	z80_bit					emulate;	// global flag if zxnDMA is emulated at all
	z80_bit					menu_enabled;	// enable/disable in options menu
	z80_bit					emulate_Zilog;	// 1 = Zilog DMA (+1 length of transfers), 0 = zxnDMA
	z80_bit					bus_master;		// 1 = DMA holds bus, no CPU operation allowed
	z80_bit					emulate_UA858D;	// 1 = UA858D (only when emulate_Zilog is already set)
	// emulation related internal values
	z80_byte				write_mode;		// which group of registers is being written to (by write_mask)
	z80_byte				write_index;	// index of next write (corresponding to bit0 of write_mask)
	z80_byte				write_mask;		// which further values are expected to be written to
	z80_byte				read_index;		// index of next read (corresponding to bit0 of read_mask)
	z80_byte				read_mask;		// further values to be read
	int						transfer_start_t;	// start "t_estados" for next transfer-chunk
		// whole continuous transfer may be broken into multiple chunks by emulator (to refresh screen)
		// so the "transfer_start_t" will be advancing every call to zxndma_emulate during active transfer
};

//public API for `s_zxndma` structure
extern int zxndma_is_direction_a_to_b(const struct s_zxndma* const dma);
extern int zxndma_transfer_mode(const struct s_zxndma* const dma);
extern void zxndma_reset(struct s_zxndma* const dma);
extern void zxndma_write_value(struct s_zxndma* const dma, const z80_byte value);
extern void zxndma_emulate(struct s_zxndma* const dma);
extern z80_byte zxndma_read_value(struct s_zxndma* const dma);

// single global instance of the zxnDMA chip (enough for core3.00.5 emulation)
extern struct s_zxndma zxndma;

///////////////////////////////////////////////////////////

extern void datagear_reset(void);
extern void datagear_write_value(z80_byte value);

extern z80_bit datagear_dma_emulation;
extern z80_bit datagear_dma_is_disabled;

extern void datagear_dma_disable(void);
extern void datagear_dma_enable(void);


extern z80_byte datagear_port_a_start_addr_low;
extern z80_byte datagear_port_a_start_addr_high;

extern z80_byte datagear_port_b_start_addr_low;
extern z80_byte datagear_port_b_start_addr_high;

extern z80_byte datagear_block_length_low;
extern z80_byte datagear_block_length_high;

extern z80_byte datagear_wr0;
extern z80_byte datagear_wr1;
extern z80_byte datagear_wr2;
extern z80_byte datagear_wr3;
extern z80_byte datagear_wr4;
extern z80_byte datagear_wr5;
extern z80_byte datagear_wr6;

extern void datagear_handle_dma(void);

#endif
