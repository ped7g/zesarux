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

#include <stdio.h>
#include <stdlib.h>

#include "datagear.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "screen.h"

///////////////////////////////////////////////////////////////////
// zxnDMA emulation rewritten by Peter Helcmanovsky (Ped7g)
// - original datagear-like emulation written by Cesar is below
///////////////////////////////////////////////////////////////////

// single global instance of zxnDMA
//   keep all new functions using pointer to instance)
//   because there's some chance zxnDMA will get exteneded (multi-channel maybe)
struct s_zxndma zxndma;

#define WRITE_MODE_WR0	0	// has four extra bytes (portA.wr_address, length)
#define WRITE_MODE_WR1	1	// has one extra byte (portA.timing)
#define WRITE_MODE_WR2	2	// has one extra byte (portB.timing), and timing has one (prescalar)
#define WRITE_MODE_WR3	3	// has two extra bytes (_mask_byte, _match_byte)
#define WRITE_MODE_WR4	4	// has three extra (2x portB.wr_address, _interrupt_control_byte)
	// the _interrupt_control_byte has two more extra: _icb_pulse_control_byte, _icb_interrupt_vector
//WR5 has zero extra bytes (no need to define it)
#define WRITE_MODE_WR6	5	// has one extra byte (read mask)

#define COMMAND_RESET					0xC3
#define COMMAND_RESET_PORT_A_TIMING		0xC7
#define COMMAND_RESET_PORT_B_TIMING		0xCB
#define COMMAND_LOAD					0xcF
#define COMMAND_CONTINUE				0xD3
#define COMMAND_DISABLE_INTERUPTS		0xAF
#define COMMAND_ENABLE_INTERUPTS		0xAB
#define COMMAND_RESET_DISABLE_INTERUPTS	0xA3
#define COMMAND_ENABLE_AFTER_RETI		0xB7
#define COMMAND_READ_STATUS_BYTE		0xBF
#define COMMAND_REINIT_STATUS_BYTE		0x8B
#define COMMAND_START_READ_SEQUENCE		0xA7
#define COMMAND_FORCE_READY				0xB3
#define COMMAND_DISABLE					0x83
#define COMMAND_ENABLE					0x87
#define COMMAND_READ_MASK_FOLLOWS		0xBB

static void set_address_low(struct s_zxndma_port* const port, const z80_byte lsb) {
	port->wr_address = (port->wr_address & 0xFF00) | lsb;
}

static void set_address_high(struct s_zxndma_port* const port, const z80_byte msb) {
	port->wr_address = (port->wr_address & 0x00FF) | (msb<<8);
}

static void set_port_config(struct s_zxndma_port* const port, const z80_byte wr1_or_wr2) {
	port->config = wr1_or_wr2 & 0b01111000;
}

static void set_port_cycles(struct s_zxndma_port* const port, const z80_byte cycles) {
	port->timing = cycles & 0b00100011;		// other bits are ignored in zxnDMA emulation
}

static void set_length_low(struct s_zxndma* const dma, const z80_byte lsb) {
	dma->length = (dma->length & 0xFF00) | lsb;
}

static void set_length_high(struct s_zxndma* const dma, const z80_byte msb) {
	dma->length = (dma->length & 0x00FF) | (msb<<8);
}

static void reset_port_timing(struct s_zxndma_port* const port) {
	port->config &= ~0b01000000;		// clear "variable timing" bit in config
}

static void adjust_port_address(struct s_zxndma_port* const port) {
	switch (zxndma_get_port_increment_type(port)) {
		case 0:	--port->address; break;
		case 1:	++port->address; break;
	}
}

static void set_is_transfering(struct s_zxndma* const dma, const int enable) {
	if (enable) {
		dma->status |= 0x80;
		dma->transfer_start_t = t_estados;
		//FIXME reset also next prescalar (to transfer 1B instantly before waiting)
	} else {
		dma->status &= ~0x80;
	}
}

static int is_transfering(const struct s_zxndma* const dma) {
	return (0x80 <= dma->status);
}

static void expect_extra_bytes(struct s_zxndma* const dma, const z80_byte mode, const z80_byte mask) {
	dma->write_index = 0;
	dma->write_mode = mode;
	dma->write_mask = mask;
}

static void do_command(struct s_zxndma* const dma, const z80_byte command) {
	// any command except COMMAND_ENABLE will disable current transfer
	set_is_transfering(dma, 0);		// so set this any way, ENABLE will override it
	switch (command) {
		case COMMAND_RESET:
			// Not implemented in emulation:
			//  interrupts, bus-request logic, interrupt latches, FORCE_READY
			dma->wr5 &= ~0b00110000;	// clear auto-repeat, clear /CE+/WAIT mode
			// set "not end-of-block" and "match-not-found" (matching is not implemented in zxnDMA)
			dma->status |= 0b00110000;
			reset_port_timing(&dma->portA);
			reset_port_timing(&dma->portB);
			break;
		case COMMAND_RESET_PORT_A_TIMING:
			reset_port_timing(&dma->portA);
			break;
		case COMMAND_RESET_PORT_B_TIMING:
			reset_port_timing(&dma->portB);
			break;
		case COMMAND_LOAD:
			//FORCE_READY not implemented (should be cleared by LOAD)
			// the original Zilog DMA chip would not load destination address if in "fixed" mode
			// but the UA858D clone and zxnDMA are not that strict, and will load it any way
			dma->portA.address = dma->portA.wr_address;
			dma->portB.address = dma->portB.wr_address;
			dma->counter = 0;
			dma->status &= ~0b00000001;		// clear status "any byte transferred"
			dma->status |=  0b00100000;		// set "not end-of-block"
			break;
		case COMMAND_REINIT_STATUS_BYTE:
			// set "not end-of-block" and "match-not-found" (matching is not implemented in zxnDMA)
			dma->status |= 0b00110000;
			break;
		case COMMAND_CONTINUE:
			dma->counter = 0;
			dma->status |=  0b00100000;		// set "not end-of-block"
			break;
		case COMMAND_READ_STATUS_BYTE:
			// any ongoing sequence of reading must be finished first before invoking this command
			if (dma->read_mask) break;
			dma->read_mask = 0x01;			// only status byte
			dma->read_index = 0;
			break;
		case COMMAND_READ_MASK_FOLLOWS:
			expect_extra_bytes(dma, WRITE_MODE_WR6, 0x01);
			break;
		case COMMAND_START_READ_SEQUENCE:
			// any ongoing sequence of reading must be finished first before invoking this command
			if (dma->read_mask) break;
			dma->read_mask = dma->wr6_read_mask;
			dma->read_index = 0;
			break;
		case COMMAND_ENABLE:
			set_is_transfering(dma, 1);
			break;
		case COMMAND_DISABLE:
			// already done at the beginning of this function
			break;
		// not implemented in zxnDMA (and in this emulation)
		case COMMAND_DISABLE_INTERUPTS:
		case COMMAND_ENABLE_INTERUPTS:
		case COMMAND_RESET_DISABLE_INTERUPTS:
		case COMMAND_ENABLE_AFTER_RETI:
		case COMMAND_FORCE_READY:
			// not implemented
			break;
	}
}

int zxndma_is_port_io(const struct s_zxndma_port* const port) {
	return 0b00001000 == (0b00001000 & port->config);
}

// 0 = decrement, 1 = increment, 2..3 = fixed
int zxndma_get_port_increment_type(const struct s_zxndma_port* const port) {
	return (port->config>>4) & 0b11;
}

// 0 = default Z80 timing, otherwise amount of cycles specified in config
int zxndma_get_port_cycles(const struct s_zxndma_port* const port) {
	if (0b01000000 & port->config) {	// variable timing
		return 4 - (port->timing & 0b11);	// 0b11 will produce timing "1" which is illegal, but..
	}
	return 3;	// standard Z80 timing (wild guess done in chat with Allen A., may be wrong)
}

// 1 = A->B, 0 = B->A
int zxndma_is_direction_a_to_b(const struct s_zxndma* const dma) {
	return 0 < (0b100 & dma->wr0);
}

int zxndma_transfer_mode(const struct s_zxndma* const dma) {
	return (dma->wr4>>5) & 0b11;
}

void zxndma_reset(struct s_zxndma* const dma) {
	dma->status = 0b00111010;	// transfer is disabled, full block "no", any transfer "no"
	dma->prescalar = 0;
	dma->portA.address = dma->portB.address = dma->counter = 0;	// internal trasnfer variables
	dma->length = dma->wr0 = dma->wr3 = dma->wr4 = dma->wr5 = dma->wr6 = 0;
	dma->portA.wr_address = dma->portB.wr_address = 0;
	dma->portA.config = dma->portB.config = 0;
	dma->wr6_read_mask = 0b01111111;		// return all RR0-RR6 by default (after power-on)
	// internal emulation related variables (mask==0 is enough, no need to init mode/index vars)
	dma->write_mask = dma->read_mask = 0;	// no extra bytes are expected
}

void zxndma_write_value(struct s_zxndma* const dma, const z80_byte value) {

	// check if some extra bytes in particular register group is expected, handle it if yes
	if (dma->write_mask) {

		// search for some bit in mask set (keep advancing index, so index is 1..n when mask is hit)
		int current_mask_bit0;
		do {
			++dma->write_index;
			current_mask_bit0 = dma->write_mask & 1;
			dma->write_mask >>= 1;
		} while (0 == current_mask_bit0);

		switch (dma->write_mode) {

			case WRITE_MODE_WR0:
				switch (dma->write_index) {
					case 1:	set_address_low(&dma->portA, value);	break;
					case 2:	set_address_high(&dma->portA, value);	break;
					case 3:	set_length_low(dma, value);				break;
					case 4:	set_length_high(dma, value);			break;
				}
				break;

			case WRITE_MODE_WR1:
				set_port_cycles(&dma->portA, value);	// only possibility
				break;

			case WRITE_MODE_WR2:
				switch (dma->write_index) {
					case 1:
						set_port_cycles(&dma->portB, value);
						if (0b00100000 & value) dma->write_mask = 1;	// extra prescalar byte following
						break;
					case 2: dma->prescalar = value;					break;
				}
				break;

			case WRITE_MODE_WR3:
				switch (dma->write_index) {
					case 1:	dma->_mask_byte = value;				break;
					case 2: dma->_match_byte = value;				break;
				}
				// if no extra bytes are expected and bit ENABLE is flipped, start transfer
				if (!dma->write_mask && dma->wr3&0x40) do_command(dma, COMMAND_ENABLE);
				break;

			case WRITE_MODE_WR4:
				switch (dma->write_index) {
					case 1:	set_address_low(&dma->portB, value);	break;
					case 2:	set_address_high(&dma->portB, value);	break;
					case 3:
						dma->_interrupt_control_byte = value;
						dma->write_mask = (value >> 3) & 0b11;		// may have two extra bytes
						break;
					case 4: dma->_icb_pulse_control_byte = value;	break;
					case 5:	dma->_icb_interrupt_vector = value;		break;
				}
				break;

			case WRITE_MODE_WR6:
				dma->wr6_read_mask = value & 0x7F;		//only possibility
				break;

		} // switch (dma->write_mode)

		// extra byte was accepted, wait for next one
		return;

	} // if (dma->write_mask)

	// no extra byte was expected, this is write into one of the base WR0..WR6 registers
	if (value & 0b10000000) {
		// WR3, WR4, WR5, WR6 (bit7 = 1)
		switch (value & 0b00000011) {
			case 0:						//WR3 (0b1xxxxx00)
				dma->wr3 = value;
				expect_extra_bytes(dma, WRITE_MODE_WR3, (value >> 3) & 0b11);
				// if no extra bytes are expected and bit ENABLE is flipped, start transfer
				if (!dma->write_mask && dma->wr3&0x40) do_command(dma, COMMAND_ENABLE);
					// maybe too smart, maybe Zilog DMA will start without waiting for extra bytes?
				break;
			case 1:						//WR4 (0b1xxxxx01)
				dma->wr4 = value;
				expect_extra_bytes(dma, WRITE_MODE_WR4, (value >> 2) & 0b111);
				break;
			case 2:						//WR5 (0b10xxx010)
				dma->wr5 = value;
				break;
			case 3:						//WR6 (0b1xxxxx11)
				dma->wr6 = value;
				do_command(dma, value);
				break;
		}
	} else {
		// WR0, WR1, WR2
		if (value & 0x03) {				// WR0 (0b0xxxxxAA, AA != 00)
			dma->wr0 = value;
			expect_extra_bytes(dma, WRITE_MODE_WR0, (value >> 3) & 0b1111);
		} else if (value & 0x04) {		// WR1 (0b0xxxx100)
			set_port_config(&dma->portA, value);
			expect_extra_bytes(dma, WRITE_MODE_WR1, (value >> 6) & 0b1);
		} else {						// WR2 (0b0xxxx000)
			set_port_config(&dma->portB, value);
			expect_extra_bytes(dma, WRITE_MODE_WR2, (value >> 6) & 0b1);
		}
	}

}

z80_byte zxndma_read_value(struct s_zxndma* const dma) {
	if (0 == dma->read_mask) return 0;		// nothing to be seen here, proceed further

	// search for some bit in mask set (keep advancing index, so index is 1..n when mask is hit)
	int current_mask_bit0;
	do {
		++dma->read_index;
		current_mask_bit0 = dma->read_mask & 1;
		dma->read_mask >>= 1;
	} while (0 == current_mask_bit0);

	switch (dma->write_index) {
		case 1:	return dma->status & 0x7F;	// hide bit7 which is used internally for emulation
		case 2:	return dma->counter & 0xFF;
		case 3:	return dma->counter>>8;
		case 4:	return dma->portA.address & 0xFF;
		case 5:	return dma->portA.address>>8;
		case 6:	return dma->portB.address & 0xFF;
		case 7:	return dma->portB.address>>8;
		default:
			return 0;
	}
}

void zxndma_emulate(struct s_zxndma* const dma) {

	if (!is_transfering(dma)) return;

	int mode = zxndma_transfer_mode(dma);
	if (ZXNDMA_MODE_BYTE == mode || ZXNDMA_MODE_INVALID == mode) {
		printf("DMA Transfer with mode %d (%s) requested - ignored\n",
				mode, (ZXNDMA_MODE_INVALID == mode) ? "invalid" : "single byte");
		set_is_transfering(dma, 0);
		return;
	}

	struct s_zxndma_port* const src = zxndma_is_direction_a_to_b(dma) ? &dma->portA : &dma->portB;
	struct s_zxndma_port* const dst = zxndma_is_direction_a_to_b(dma) ? &dma->portB : &dma->portA;
// 	printf("Copying %d bytes from %04XH to %04XH\n", dma->length, src->address, dst->address);

	int t_per_byte = zxndma_get_port_cycles(src) + zxndma_get_port_cycles(dst);
// 	if (zxndma_is_port_io(dst)) ++t_per_byte;		// I/O out is costing +1 extra in Cesar's old version
	//TODO verify with Allen:
	// - the timings vs contention
	// - timing is port+port?
	// - final values after full block is transfered (RR1-6) (Zilog doc has nice table)

	int remaining = t_estados - dma->transfer_start_t;
	if (remaining < 0) remaining += screen_testados_total;
	
	//FIXME seems like DMA is not advancing t_estados by itself, which it probably should
	// or find out what kind of hack is Cesar using
	// -> after DMA will advance states correctly and reserve bus from CPU correctly, modify "while"
	//    below to use available states in all modes, to give emulation time to refresh also Copper/screen
	//    but it has to kill CPU between when in continuous or burst+ready!
	//// currently the "continuous" mode transfers are basically "instant" from t_estados point of view

// 	if (ZXNDMA_MODE_CONTINUOUS == mode)
// 	printf("Before transfer: dma->transfer_start_t %d t_estados %d remaining %d length %d t_per_byte %d\n",
// 		dma->transfer_start_t, t_estados, remaining, dma->length, t_per_byte);

	if (ZXNDMA_MODE_CONTINUOUS == mode || t_per_byte <= remaining) {
		dma->status |= 1;		// some byte will be surely transferred this time
	}

	//FIXME quick hack preventing BURST mode to go back to CPU when prescalar == 0 (for CORE305.SNA)
	// remove it after burst will correctly recognize when it should give back time to CPU
	if (ZXNDMA_MODE_BURST == mode && 0 == dma->prescalar) mode = ZXNDMA_MODE_CONTINUOUS;

	while (ZXNDMA_MODE_CONTINUOUS == mode || t_per_byte <= remaining) {

		// read byte
		z80_byte value = zxndma_is_port_io(src) ?
			lee_puerto_spectrum_no_time((src->address>>8)&0xFF,src->address & 0xFF) :
			peek_byte_no_time(src->address);

		// write byte
		if (zxndma_is_port_io(dst)) {
			out_port_spectrum_no_time(dst->address, value);
		} else {
			poke_byte_no_time(dst->address, value);
		}

		dma->transfer_start_t += t_per_byte;
		if (screen_testados_total <= dma->transfer_start_t) {
			dma->transfer_start_t -= screen_testados_total;
		}

		remaining -= t_per_byte;

		adjust_port_address(src);

		// Zilog DMA length compare would be here (doing +1 byte transfer, and reading src.adr+1, dst.adr, counter)

		adjust_port_address(dst);

		++dma->counter;

		//zxnDMA length compare (length == transferred bytes)
		if (dma->counter == dma->length) {
			dma->status &= ~0b00100000;		// whole block transferred at least once
			if (dma->wr5 & 0b00100000) {	// auto-restart feature (very limited LOAD command)
				dma->portA.address = dma->portA.wr_address;
				dma->portB.address = dma->portB.wr_address;
				dma->counter = 0;
			} else {						// no auto-restart
				set_is_transfering(dma, 0);
				break;
			}
		}

	}
}

///////////////////////////////////////////////////////////////////
// original DMA emulation written by Cesar Hernandez Bano
///////////////////////////////////////////////////////////////////

//Si esta recibiendo parametros de comando.
//Si no es 0, indica cuantos parámetros le quedan por recibir
//int datagear_receiving_parameters=0;

//Mascara de los parametros a leer. Por ejemplo si WR0 enviara los 4 parametros, tendra valor 00001111b
z80_byte datagear_mask_commands=0;

//Indice al numero de parametro leido
int datagear_command_index;
//Ejemplo, en WR0 vale 0 cuando vamos a leer el primer parametro (Port A starting address Low byte), vale 1 cuando vamos a leer Port A adress high byte

//Indica ultimo comando leido, tal cual el primer byte
//z80_byte datagear_last_command_byte;


//Indica ultimo comando leido, 0=WR0, 1=WR1, etc. Caso especial: 128+2 (130) para valor de ZXN PRESCALAR (FIXED TIME TRANSFER) de WR2 en TBBLUE

z80_byte datagear_last_command;

z80_byte datagear_port_a_start_addr_low;
z80_byte datagear_port_a_start_addr_high;

z80_byte datagear_port_b_start_addr_low;
z80_byte datagear_port_b_start_addr_high;

z80_byte datagear_block_length_low;
z80_byte datagear_block_length_high;

z80_byte datagear_port_a_variable_timing_byte;
z80_byte datagear_port_b_variable_timing_byte;

//Ultimo valor recibido para los registros
z80_byte datagear_wr0;
z80_byte datagear_wr1;
z80_byte datagear_wr2;
z80_byte datagear_wr3;
z80_byte datagear_wr4;
z80_byte datagear_wr5;
z80_byte datagear_wr6;

//Si esta activada la emulacion de dma
z80_bit datagear_dma_emulation={0};

//Si esta desactivada la dma. Si esta desactivada, se puede acceder igualmente a todo, excepto que no ejecuta transferencias DMA
z80_bit datagear_dma_is_disabled={0};

//Si hay transferencia de dma activa
z80_bit datagear_is_dma_transfering={0};

//Valor de la DMA de TBBLUE de prescaler
z80_byte datagear_dma_tbblue_prescaler=0;

int datagear_dma_last_testados=0;

void datagear_dma_disable(void)
{
    datagear_dma_emulation.v=0;
}

void datagear_dma_enable(void)
{
    datagear_dma_emulation.v=1;
}


void datagear_reset(void)
{
    datagear_mask_commands=0;

    datagear_wr0=datagear_wr1=datagear_wr2=datagear_wr3=datagear_wr4=datagear_wr5=datagear_wr6=0;
    datagear_is_dma_transfering.v=0;
    datagear_dma_tbblue_prescaler=0;
}

/*void datagear_do_transfer(void)
{
    if (datagear_dma_is_disabled.v) return;

				z80_int transfer_length=value_8_to_16(datagear_block_length_high,datagear_block_length_low);
				z80_int transfer_port_a,transfer_port_b;

		
					transfer_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
					transfer_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);
							

				if (datagear_wr0 & 4) printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_a,transfer_port_b);
                else printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_b,transfer_port_a);

                if (datagear_wr1 & 8) printf ("Port A I/O. not implemented yet\n");
                if (datagear_wr2 & 8) printf ("Port B I/O. not implemented yet\n");

				//while (transfer_length) {
                    z80_byte byte_leido;
                    if (datagear_wr0 & 4) {
                        byte_leido=peek_byte_no_time(transfer_port_a);
					    poke_byte_no_time(transfer_port_b,byte_leido);
                    }

                    else {
                        byte_leido=peek_byte_no_time(transfer_port_b);
					    poke_byte_no_time(transfer_port_a,byte_leido);                        
                    }

                    if ( (datagear_wr1 & 32) == 0 ) {
                        if (datagear_wr1 & 16) transfer_port_a++;
                        else transfer_port_a--;
                    }

                    if ( (datagear_wr2 & 32) == 0 ) {
                        if (datagear_wr2 & 16) transfer_port_b++;
                        else transfer_port_b--;
                    }                    

					transfer_length--;
				//}

    if (transfer_length==0) datagear_is_dma_transfering.v=0;

}
*/

void datagear_write_value(z80_byte value)
{
	//gestionar si estamos esperando parametros de comando
	if (datagear_mask_commands) {
		switch (datagear_last_command) {

			//WR0
			case 0:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A starting address Low byte
				//1=Port A starting address High byte
				//2=Block length low byte
				//3=Block length high byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_a_start_addr_low=value;
						//printf ("Setting port a start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_a_start_addr_high=value;
						//printf ("Setting port a start address high to %02XH\n",value);
					break;					

					case 2:
						datagear_block_length_low=value;
						//printf ("Setting block length low to %02XH\n",value);
					break;

					case 3:
						datagear_block_length_high=value;
						//printf ("Setting block length high to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;

			break;

			//WR1
			case 1:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port A variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_a_variable_timing_byte=value;
						//printf ("Setting port a variable timing byte to %02XH\n",value);
					break;

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;			
			break;

			//WR2
			case 2:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B variable timing byte

				switch (datagear_command_index) {
					case 0:
						datagear_port_b_variable_timing_byte=value;
						//printf ("Setting port b variable timing byte to %02XH\n",value);
						if (value&32 && MACHINE_IS_TBBLUE) {

							//De momento solo leo el valor del prescaler, aunque no hago nada con el

/*
TODO: Solo en Next. Si bit 5 no es 0, se leera otro parametro:
D7  D6  D5  D4  D3  D2  D1  D0  ZXN PRESCALAR (FIXED TIME TRANSFER)
#
# The ZXN PRESCALAR is a feature of the ZXN DMA implementation.
# If non-zero, a delay will be inserted after each byte is transferred
# such that the total time needed for the transfer is at least the number
# of cycles indicated by the prescalar.  This works in both the continuous
# mode and the burst mode.

# The ZXN DMA's speed matches the current CPU speed so it can operate
# at 3.5MHz, 7MHz or 14MHz.  Since the prescalar delay is a cycle count,
# the actual duration depends on the speed of the DMA.  A prescalar
# delay set to N cycles will result in a real time transfer taking N/fCPU
# seconds.  For example, if the DMA is operating at 3.5MHz and the max
# prescalar of 255 is set, the transfer time for each byte will be
# 255/3.5MHz = 72.9us.  If the DMA is used to send sampled audio, the
# sample rate would be 13.7kHz and this is the lowest sample rate possible
# using the prescalar.
#
# If the DMA is operated in burst mode, the DMA will give up any waiting
# time to the CPU so that the CPU can run while the DMA is idle.



*/

						//printf ("Will receive ZXN Prescaler\n");
						datagear_last_command=128+2;

						datagear_mask_commands=1;        //Realmente esto cualquier cosa diferente de 0 nos vale
						
						}
					break;
				}

				//Siempre que no vayamos a recibir el prescaler
				if (datagear_last_command!=130) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;	            
				}
			break;

			//WR3
			case 3:
			break;

			//WR4
			case 4:
				//si parametro de indice actual se salta porque mascara vale 0 en bit bajo
				while ( (datagear_mask_commands&1)==0) {
					datagear_mask_commands=datagear_mask_commands >> 1;
					datagear_command_index++;
				}

				//Aqui tenemos, en datagear_command_index, el parametro que vamos a leer ahora:
				//0=Port B starting address Low byte
				//1=Port B starting address High byte
				switch (datagear_command_index) {
					case 0:
						datagear_port_b_start_addr_low=value;
						//printf ("Setting port b start address low to %02XH\n",value);
					break;

					case 1:
						datagear_port_b_start_addr_high=value;
						//printf ("Setting port b start address high to %02XH\n",value);
					break;					

				}

				datagear_mask_commands=datagear_mask_commands >> 1;
				datagear_command_index++;				
			break;

			//WR5
			case 5:
			break;

			//WR6
			case 6:
			break;


			case 130:
				//printf ("Reading ZXN Prescaler = %02XH\n",value);
				datagear_dma_tbblue_prescaler=value;
				datagear_mask_commands=0;
			break;

		}

	}

	else {

		//datagear_last_command_byte=value;
		datagear_command_index=0;
	//Obtener tipo de comando
	//SI WR0

	z80_byte value_mask_wr0_wr3=value&(128+2+1);
	if (value_mask_wr0_wr3==1 || value_mask_wr0_wr3==2 ||value_mask_wr0_wr3==3 ) {
		//printf ("WR0\n");
		datagear_last_command=0;
		datagear_wr0=value;

		//Ver bits 4,5,6,7 y longitud comando
/*
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (LOW BYTE)
#       |   |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT A STARTING ADDRESS (HIGH BYTE)
#       |   V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (LOW BYTE)
#       V
#  D7  D6  D5  D4  D3  D2  D1  D0  BLOCK LENGTH (HIGH BYTE)
*/		

		datagear_mask_commands=(value>>3)&15;

		//z80_byte transfer_type=value&3;
		/*if (transfer_type==1) printf ("Type: transfer\n");
		else if (transfer_type==2) printf ("Type: search\n");
		else if (transfer_type==3) printf ("Type: search/transfer\n");

		if (value&4) printf ("Port A -> Port B\n");
		else printf ("Port B -> Port A\n");*/


	}

	if (value_mask_wr0_wr3==128) {
		//printf ("WR3\n");
		datagear_last_command=3;
		datagear_wr3=value;
	}

	if (value_mask_wr0_wr3==129) {
		//printf ("WR4 = %02XH\n",value);
		datagear_last_command=4;
		datagear_wr4=value;

		datagear_mask_commands=(value>>2)&3;



	}	

	if (value_mask_wr0_wr3==128+2+1) {
		//printf ("WR6\n");
		datagear_last_command=6;
		datagear_wr6=value;

		//Tratar todos los diferentes comandos
		switch (value) {
			case 0xCF:
				//printf ("Load starting address for both ports, clear byte counter\n");
			break;

			case 0xAB:
				//printf ("Enable interrupts\n");
			break;

			case 0x87:
				//printf ("Enable DMA\n");
                datagear_is_dma_transfering.v=1;
                //datagear_do_transfer();
                datagear_dma_last_testados=t_estados;

			break;	

			case 0x83:
				//printf ("Disable DMA\n");
                datagear_is_dma_transfering.v=0;

			break;					
			
			case 0xB3:
				//printf ("Force an internal ready condition independent 'on the rdy' input\n");
			break;				

			case 0xB7:
				//printf ("Enable after RETI so dma requests bus only after receiving a reti\n");
			break;


		}		


	}	

	z80_byte value_mask_wr1_wr2=value&(128+4+2+1);
	if (value_mask_wr1_wr2==4) {
		//printf ("WR1\n");
		datagear_last_command=1;
		datagear_wr1=value;

		//Ver bits D6
        //D6 Port A variable timing byte
	
		datagear_mask_commands=(value>>6)&1;

	}

	if (value_mask_wr1_wr2==0) {
		//printf ("WR2\n");
		datagear_last_command=2;
		datagear_wr2=value;

		//Ver bits D6
        //D6 Port B variable timing byte
	
		datagear_mask_commands=(value>>6)&1;        
	}

	z80_byte value_mask_wr5=value&(128+64+4+2+1);
	if (value_mask_wr5==128+2) {
		//printf ("WR5 = %02XH\n",value);
		datagear_last_command=5;
		datagear_wr5=value;
	}

	}
}




z80_byte datagear_read_operation(z80_int address,z80_byte dma_mem_type)
{

    z80_byte byte_leido;

    if (dma_mem_type) byte_leido=lee_puerto_spectrum_no_time((address>>8)&0xFF,address & 0xFF);
    else byte_leido=peek_byte_no_time(address);

    return byte_leido;
}

void datagear_write_operation(z80_int address,z80_byte value,z80_byte dma_mem_type)
{
    if (dma_mem_type) {
		out_port_spectrum_no_time(address,value);
		//printf ("Port %04XH value %02XH\n",address,value);
		t_estados +=1; //Por ejemplo ;)
	}

    else {
		poke_byte_no_time(address,value);
	}
}


int datagear_return_resta_testados(int anterior, int actual)
{
	//screen_testados_total

	int resta=actual-anterior;

	if (resta<0) resta=screen_testados_total-anterior+actual;

	return resta; 
}		

int datagear_condicion_transferencia(z80_int transfer_length,int dma_continuous,int resta,int dmapre)
{

	//printf ("dma condicion length: %d dma_cont %d resta %d dmapre %d\n",transfer_length,dma_continuous,resta,dmapre);

	//Si hay bytes a transferir
	if (transfer_length==0) return 0;

	//Si es modo continuo
	if (dma_continuous) return 1;

	//Modo burst. Permitiendo ejecutar la cpu entre medio
	if (resta>=dmapre) return 1;


	//Otro caso, retornar 0
	return 0;


}

void datagear_handle_dma(void)
{
        if (datagear_is_dma_transfering.v==0) return;


      				z80_int transfer_length=value_8_to_16(datagear_block_length_high,datagear_block_length_low);
				z80_int transfer_port_a,transfer_port_b;

		
					transfer_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
					transfer_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);
							

				//if (datagear_wr0 & 4) printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_a,transfer_port_b);
                //else printf ("Copying %d bytes from %04XH to %04XH\n",transfer_length,transfer_port_b,transfer_port_a);

                




        int dmapre=2; //Cada 2 estados, una transferencia

		int resta=datagear_return_resta_testados(datagear_dma_last_testados,t_estados);

		//dmapre *=cpu_turbo_speed;

		int resta_antes=resta;


		//printf ("Antes transferencia: dmapre: %d datagear_dma_last_testados %d t_estados %d resta %d dmapre %d length %d\n",
		//	dmapre,datagear_dma_last_testados,t_estados,resta,dmapre,transfer_length);

		//Ver si modo continuo o modo burst
		//WR4. Bits D6 D5:
		//#       0   0 = Byte mode -> Do not use (Behaves like Continuous mode, Byte mode on Z80 DMA)
		//#       0   1 = Continuous mode
		//#       1   0 = Burst mode
		//#       1   1 = Do not use

		//Por defecto, modo continuo (todo de golpe) (dma_continuous=1). Modo burst (dma_continuous=0), permite ejecutar la cpu entre medio
		int dma_continuous=1;

		z80_byte modo_transferencia=datagear_wr4 & (64+32);
		if (modo_transferencia==64) dma_continuous=0;

/*
Excepción:
# The ZXN DMA can operate in either burst or continuous mode.  Continuous mode means the DMA chip
# runs to completion without allowing the CPU to run.  Burst mode nominally means the DMA lets the
# CPU run if either port is not ready.  This condition can't happen in the ZXN DMA chip except when
# operated in the special fixed time transfer mode.  In this mode, the ZXN DMA chip will let the CPU
# run while it waits for the fixed time to expire between bytes transferred.  Note that there is no
# byte transfer mode as in the Z80 DMA.
*/		

		//Por tanto de momento:
		if (MACHINE_IS_TBBLUE && dma_continuous==0) {
			//Modo burst en tbblue

			//Si tiene pre escalar, se permite modo burst. Si no, no

			//printf ("Tbblue and burst mode\n");

			if ( (datagear_port_b_variable_timing_byte & 32)==0 || 0 == datagear_dma_tbblue_prescaler) {
				//printf ("burst mode not allowed on tbblue because it has no pre escalar\n");
				dma_continuous=1; //no tiene pre escalar
			}
		}
			

		//TODO Ver ese delay
		/*
# WR2 - Write Register Group 2
#
#  D7  D6  D5  D4  D3  D2  D1  D0  BASE REGISTER BYTE
#   0   |   |   |   |   0   0   0
#       |   |   |   |
#       |   |   |   0 = Port B is memory
#       |   |   |   1 = Port B is IO
#       |   |   |
#       |   0   0 = Port B address decrements
#       |   0   1 = Port B address increments
#       |   1   0 = Port B address is fixed
#       |   1   1 = Port B address is fixed
#       |
#       V
#  D7  D6  D5  D4  D3  D2  D1  D0  PORT B VARIABLE TIMING BYTE
#   0   0   |   0   0   0   |   |
#           |               0   0 = Cycle Length = 4
#           |               0   1 = Cycle Length = 3
#           |               1   0 = Cycle Length = 2
#           |               1   1 = Do not use
#           |
#           V
#  D7  D6  D5  D4  D3  D2  D1  D0  ZXN PRESCALAR (FIXED TIME TRANSFER)
#
# The ZXN PRESCALAR is a feature of the ZXN DMA implementation.
# If non-zero, a delay will be inserted after each byte is transferred
# such that the total time needed for the transfer is at least the number
# of cycles indicated by the prescalar.  This works in both the continuous
# mode and the burst mode.		
		*/

		//TEMP hacerlo de golpe. ejemplo: dmafill
		//while (transfer_length>0) {

		//TEMP hacerlo combinando tiempo con cpu
		//while (resta>=dmapre && transfer_length>0) {

			//dma_continuous=1;

		//if (dma_continuous) printf ("Transferencia modo continuous\n");
		//else printf ("Transferencia modo burst\n");

		while ( datagear_condicion_transferencia(transfer_length,dma_continuous,resta,dmapre) ) {

			//for (i=0;i<cpu_turbo_speed;i++) {
				//printf ("dma op ");
			           z80_byte byte_leido;
                    if (datagear_wr0 & 4) {
                        byte_leido=datagear_read_operation(transfer_port_a,datagear_wr1 & 8);
					    datagear_write_operation(transfer_port_b,byte_leido,datagear_wr2 & 8);
                    }

                    else {
                        byte_leido=datagear_read_operation(transfer_port_b,datagear_wr2 & 8);
					    datagear_write_operation(transfer_port_a,byte_leido,datagear_wr1 & 8);                        
                    }

                    if ( (datagear_wr1 & 32) == 0 ) {
                        if (datagear_wr1 & 16) transfer_port_a++;
                        else transfer_port_a--;
                    }

                    if ( (datagear_wr2 & 32) == 0 ) {
                        if (datagear_wr2 & 16) transfer_port_b++;
                        else transfer_port_b--;
                    }                    

					transfer_length--;

			//}

			datagear_dma_last_testados +=dmapre;

			//Ajustar a total t-estados
			//printf ("pre ajuste %d\n",datagear_dma_last_testados);
			datagear_dma_last_testados %=screen_testados_total;
			//printf ("post ajuste %d\n",datagear_dma_last_testados);

			resta=datagear_return_resta_testados(datagear_dma_last_testados,t_estados);
	
			//printf ("En transferencia: dmapre: %6d datagear_dma_last_testados %6d t_estados %6d resta %6d\n",	dmapre,datagear_dma_last_testados,t_estados,resta);

		//si da la vuelta
			if (resta<resta_antes) {
				//provocar fin
				resta=0;
			}


			resta_antes=resta;

		}


        //Guardar valores contadores
  
        datagear_block_length_low=value_16_to_8l(transfer_length);
        datagear_block_length_high=value_16_to_8h(transfer_length);

        datagear_port_a_start_addr_low=value_16_to_8l(transfer_port_a);
        datagear_port_a_start_addr_high=value_16_to_8h(transfer_port_a);

        datagear_port_b_start_addr_low=value_16_to_8l(transfer_port_b);
        datagear_port_b_start_addr_high=value_16_to_8h(transfer_port_b);        
	
   
				//}

    if (transfer_length==0) datagear_is_dma_transfering.v=0;

	//printf ("length: %d\n",transfer_length);

}
