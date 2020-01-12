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
#include <string.h>

#include "cpu.h"
#include "tbblue.h"
#include "mem128.h"
#include "debug.h"
#include "contend.h"
#include "utils.h"
#include "menu.h"
#include "divmmc.h"
#include "diviface.h"
#include "screen.h"

#include "timex.h"
#include "ula.h"
#include "audio.h"

#include "datagear.h"
#include "ay38912.h"
#include "multiface.h"
#include "uartbridge.h"
#include "chardevice.h"

#define TBBLUE_MAX_SRAM_8KB_BLOCKS 224

//Punteros a los 64 bloques de 8kb de ram de spectrum
z80_byte *tbblue_ram_memory_pages[TBBLUE_MAX_SRAM_8KB_BLOCKS];

//2MB->224 bloques (16+16+64*3) (3 bloques extra de 512)
//1.5 MB->160 bloques (2 bloques extra de 512)
//1 MB->96 bloques (1 bloque extra de 512)
//512 KB->32 bloques de (0 bloque extra de 512)

z80_byte tbblue_extra_512kb_blocks=3;

z80_byte tbblue_return_max_extra_blocks(void)
{
	return 32+tbblue_extra_512kb_blocks*64;
}

//Retorna ram total en KB
int tbblue_get_current_ram(void)
{
	return 256+8*tbblue_return_max_extra_blocks();
}

//Punteros a los 8 bloques de 8kb de rom de spectrum
z80_byte *tbblue_rom_memory_pages[8];

z80_byte *tbblue_fpga_rom;

//Memoria mapeada, en 8 bloques de 8 kb
z80_byte *tbblue_memory_paged[8];


//Si arranca rapido sin pasar por el proceso de boot. Va directamente a rom 48k
z80_bit tbblue_fast_boot_mode={0};

z80_bit tbblue_deny_turbo_rom={0};


//Copper
z80_byte tbblue_copper_memory[TBBLUE_COPPER_MEMORY];

//Indice a la posicion de 16 bits a escribir
//z80_int tbblue_copper_index_write=0;

//Indice al opcode copper a ejecutar
z80_int tbblue_copper_pc=0;

z80_byte tbblue_machine_id=8;

struct s_tbblue_machine_id_definition tbblue_machine_id_list[]=
{
	{1,               "DE-1"},
	{2,               "DE-2"},
	{5,               "FBLabs"},
	{6,               "VTrucco"},
	{7,               "WXEDA"},
	{8,               "Emulators"},
	{10,              "ZX Spectrum Next"},
	{11,              "Multicore"},
	{250,             "ZX Spectrum Next Antibrick"},

 	{255,""}
};

//Obtiene posicion de escritura del copper
z80_int tbblue_copper_get_write_position(void)
{
	z80_int posicion;
	posicion=tbblue_registers[97] | ((tbblue_registers[98]&7)<<8);
	return posicion;
}

//Establece posicion de escritura del copper
void tbblue_copper_set_write_position(z80_int posicion)
{
	tbblue_registers[97]=posicion&0xFF;

	z80_byte msb=(posicion>>8)&7; //3 bits bajos

	z80_byte reg98=tbblue_registers[98];
	reg98 &=(255-7);
	reg98 |=msb;
	tbblue_registers[98]=reg98;
}

//Escribe dato copper en posicion de escritura
void tbblue_copper_write_data(z80_byte value)
{
	z80_int posicion=tbblue_copper_get_write_position();

	posicion &=(TBBLUE_COPPER_MEMORY-1);


	//printf ("Writing copper data index %d data %02XH\n",posicion,value);

	tbblue_copper_memory[posicion]=value;

	posicion++;
	tbblue_copper_set_write_position(posicion);

}

//Escribe dato copper en posicion de escritura, 16 bits
void tbblue_copper_write_data_16b(z80_byte value1, z80_byte value2)
{
	z80_int posicion=tbblue_copper_get_write_position();

	posicion &=(TBBLUE_COPPER_MEMORY-1);

	//After a write to an odd address, the entire 16-bits are written to Copper memory at once.
	if (posicion&1) {	
		tbblue_copper_memory[posicion-1]=value1;
		tbblue_copper_memory[posicion]=value2;
		//printf ("Writing copper 16b data index %d data %02X%02XH\n",posicion-1,value1,value2);
	}

	posicion++;
	tbblue_copper_set_write_position(posicion);

}

//Devuelve el byte donde apunta indice
z80_byte tbblue_copper_get_byte(z80_int posicion)
{
	posicion &=(TBBLUE_COPPER_MEMORY-1);
	return tbblue_copper_memory[posicion];
}

//Devuelve el valor de copper
z80_int tbblue_copper_get_pc(void)
{
	return tbblue_copper_pc & (TBBLUE_COPPER_MEMORY-1);
}

//Devuelve el byte donde apunta pc
z80_byte tbblue_copper_get_byte_pc(void)
{
	return tbblue_copper_get_byte(tbblue_copper_pc);

}

void tbblue_copper_get_wait_opcode_parameters(z80_int *line, z80_int *horiz)
{
	z80_byte byte_a=tbblue_copper_get_byte(tbblue_copper_pc);
	z80_byte byte_b=tbblue_copper_get_byte(tbblue_copper_pc+1);

	*line=byte_b|((byte_a&1)<<8);
	*horiz=((byte_a>>1)&63);
}

void tbblue_copper_reset_pc(void)
{
	tbblue_copper_pc=0;
}

void tbblue_copper_set_stop(void)
{
	tbblue_registers[98] &=63;
}

void tbblue_copper_next_opcode(void)
{
	//Incrementar en 2. 
	tbblue_copper_pc +=2;

  /*
                                                        modos
                                                               01 = Copper start, execute the list, then stop at last adress
       10 = Copper start, execute the list, then loop the list from start
       11 = Copper start, execute the list and restart the list at each frame
                                                        */

                                                   //Si ha ido a posicion 0
                                                   if (tbblue_copper_pc==TBBLUE_COPPER_MEMORY) {
													   z80_byte copper_control_bits=tbblue_copper_get_control_bits();
                                                           switch (copper_control_bits) {
                                                                        case TBBLUE_RCCH_COPPER_STOP:
																			//Se supone que nunca se estara ejecutando cuando el mode sea stop
                                                                           tbblue_copper_set_stop();
                                                                        break;

                                                                        case TBBLUE_RCCH_COPPER_RUN_LOOP:
                                                                                //loop
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode TBBLUE_RCCH_COPPER_RUN_LOOP\n");
                                                                        break;

																		case TBBLUE_RCCH_COPPER_RUN_LOOP_RESET:
                                                                                //loop
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode TBBLUE_RCCH_COPPER_RUN_LOOP_RESET\n");
                                                                        break;

                                                                        case TBBLUE_RCCH_COPPER_RUN_VBI:
                                                                                //loop??
                                                                                tbblue_copper_pc=0;
                                                                                //printf ("Reset copper on mode RUN_VBI\n");
                                                                        break;
                                                           }
												   }

}


//z80_bit tbblue_copper_ejecutando_halt={0};

//Ejecuta opcodes del copper // hasta que se encuentra un wait
void tbblue_copper_run_opcodes(void)
{

	z80_byte byte_leido=tbblue_copper_get_byte_pc();
	z80_byte byte_leido2=tbblue_copper_get_byte(tbblue_copper_pc+1);

	//Asumimos que no
	//tbblue_copper_ejecutando_halt.v=0;

	//if (tbblue_copper_get_pc()==0x24) printf ("%02XH %02XH\n",byte_leido,byte_leido2);

    //Special case of "value 0 to port 0" works as "no operation" (duration 1 CLOCK)
	/*
	Dado que el registro 0 es de solo lectura, no pasa nada si escribe en el: al leerlo se obtiene un valor calculado y no el del array
    if (byte_leido==0 && byte_leido2==0) {
      //printf("NOOP at %04XH\n",tbblue_copper_pc);
	  tbblue_copper_next_opcode();
      return;
    }
	*/

    //Special case of "WAIT 63,511" works as "halt" instruction
	/*
    if (byte_leido==255 && byte_leido2==255) {
	  //printf("HALT at %04XH\n",tbblue_copper_pc);
	  tbblue_copper_ejecutando_halt.v=1;

      return;
    }
	*/

		if ( (byte_leido&128)==0) {
			//Es un move
			z80_byte indice_registro=byte_leido&127;
			//tbblue_copper_pc++;
			
			//tbblue_copper_pc++;
			//printf ("Executing MOVE register %02XH value %02XH\n",indice_registro,valor_registro);
			tbblue_set_value_port_position(indice_registro,byte_leido2);

			tbblue_copper_next_opcode();

		}
		else {
			//Es un wait
			//Si se cumple, saltar siguiente posicion
			//z80_int linea, horiz;
			//tbblue_copper_get_wait_opcode_parameters(&linea,&horiz);
			if (tbblue_copper_wait_cond_fired () ) {
                                                        //printf ("Wait condition positive at copper_pc %02XH scanline %d raster %d\n",tbblue_copper_pc,t_scanline,tbblue_get_current_raster_position() );
                                                        tbblue_copper_next_opcode();
                                                        //printf ("Wait condition positive, after incrementing copper_pc %02XH\n",tbblue_copper_pc);
			}
			//printf ("Waiting until scanline %d horiz %d\n",linea,horiz);
			
		}
	
}

z80_byte tbblue_copper_get_control_bits(void)
{
	//z80_byte control=(tbblue_registers[98]>>6)&3;
	z80_byte control=(tbblue_registers[98])&(128+64);
	/*

# define(`__RCCH_COPPER_STOP', 0x00)
# define(`__RCCH_COPPER_RUN_LOOP_RESET', 0x40)
# define(`__RCCH_COPPER_RUN_LOOP', 0x80)
# define(`__RCCH_COPPER_RUN_VBI', 0xc0)

	*/
	return control;
}



/*int tbblue_copper_is_opcode_wait(void)
{
	z80_byte byte_leido=tbblue_copper_get_byte_pc();
	if ( (byte_leido&128) ) return 1;
	return 0;
}*/

//Si scanline y posicion actual corresponde con instruccion wait
int tbblue_copper_wait_cond_fired(void)
{

	int current_raster=tbblue_get_current_raster_position();
	int current_horizontal=tbblue_get_current_raster_horiz_position();

	//Obtener parametros de instruccion wait
	z80_int linea, horiz;
	tbblue_copper_get_wait_opcode_parameters(&linea,&horiz);

#ifdef TBBLUE_DELAYED_COPPER_WAITS
	// Hack to postpone any changes in the H-blank + left-border area to the next scanline,
	// as NextReg values used to draw scanline are read when the pixel area of next line starts.
	// (i.e. even after the left-border)
	if (38 <= horiz) {
		// all WAIT(line, 38..55) are converted to WAIT(line+1, 0)
		horiz = 0;
		++linea;
		int max_raster = screen_indice_inicio_pant + 192 + screen_total_borde_inferior;
		if (linea == max_raster) {
			linea = 0;
		}
	}
#endif

	//511, 63
	//if (tbblue_copper_get_pc()==0x24) 
	// printf ("Waiting until raster %d horiz %d. current %d on copper_pc=%04X\n",linea,horiz,current_raster,tbblue_copper_get_pc() );

	//comparar vertical
	if (current_raster==linea) {
		//comparar horizontal
		//printf ("Comparing current %d to %d\n",current_horizontal,horiz);
		if (current_horizontal>=horiz) {
			//printf ("Fired wait condition %d,%d at %d,%d (t-states %d)\n",linea,horiz,current_raster,current_horizontal,
			//		t_estados % screen_testados_linea);
			return 1;
		}
	}

	return 0;
}



void tbblue_copper_handle_next_opcode(void)
{

	//Si esta activo copper
    z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits != TBBLUE_RCCH_COPPER_STOP) {
        //printf ("running copper %d\n",tbblue_copper_pc);
        tbblue_copper_run_opcodes();
	}
}                                           


/*
void tbblue_if_copper_halt(void)
{
	//Si esta activo copper
    z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits != TBBLUE_RCCH_COPPER_STOP) {
        //printf ("running copper %d\n",tbblue_copper_pc);
		if (tbblue_copper_ejecutando_halt.v) {
			//liberar el halt
			//printf ("copper was on halt (copper_pc=%04XH). Go to next opcode\n",tbblue_copper_get_pc() );
			tbblue_copper_next_opcode();
			//printf ("copper was on halt (copper_pc after=%04XH)\n",tbblue_copper_get_pc() );
		}
	}	
}
*/					
 

void tbblue_copper_handle_vsync(void)
{
	z80_byte copper_control_bits=tbblue_copper_get_control_bits();
    if (copper_control_bits==TBBLUE_RCCH_COPPER_RUN_VBI) {
    	tbblue_copper_reset_pc();
        //printf ("Reset copper on control bit 3 on vsync\n");
    }
                                                   
}


void tbblue_copper_write_control_hi_byte(z80_byte value, z80_byte old_value)
{
	/*

# define(`__RCCH_COPPER_STOP', 0x00)
# define(`__RCCH_COPPER_RUN_LOOP_RESET', 0x40)
# define(`__RCCH_COPPER_RUN_LOOP', 0x80)
# define(`__RCCH_COPPER_RUN_VBI', 0xc0)

# STOP causes the copper to stop executing instructions
# and hold the instruction pointer at its current position.
#
# RUN_LOOP_RESET causes the copper to reset its instruction
# pointer to 0 and run in LOOP mode (see next).
#
# RUN_LOOP causes the copper to restart with the instruction
# pointer at its current position.  Once the end of the instruction
# list is reached, the copper loops back to the beginning.
#
# RUN_VBI causes the copper to reset its instruction
# pointer to 0 and run in VBI mode.  On vsync interrupt,
# the copper restarts the instruction list from the beginning.

# Note that modes RUN_LOOP_RESET and RUN_VBI will only reset
# the instruction pointer to zero if the mode actually changes
# to RUN_LOOP_RESET or RUN_VBI.  Writing the same mode in a
# second write will not cause the instruction pointer to zero.

# It is possible to write values into the copper's instruction
# space while it is running and since the copper constantly
# refetches a wait instruction it is executing, you can cause
# the wait instruction to end prematurely by changing it to
# something else.

*/

	const z80_byte action=value&(128+64);
	const z80_byte old_action=old_value&(128+64);
	if (action == old_action) return;	//action bits didn't change, do not reset CPC

	switch (action) {
		//Estos dos casos, resetean el puntero de instruccion
		case TBBLUE_RCCH_COPPER_RUN_LOOP_RESET:
			//printf ("Reset copper PC when writing TBBLUE_RCCH_COPPER_RUN_LOOP_RESET to control hi byte\n");
			tbblue_copper_reset_pc();
		break;

		case TBBLUE_RCCH_COPPER_RUN_VBI:
			//printf ("Reset copper PC when writing TBBLUE_RCCH_COPPER_RUN_VBI to control hi byte\n");
			tbblue_copper_reset_pc();
		break;

	}

}

//Fin copper


/*
   //es zona de vsync y borde superior
                                                                //Aqui el contador raster tiene valor (192+56 en adelante)
                                                                //contador de scanlines del core, entre 0 y screen_indice_inicio_pant ,
                                                                if (t_scanline<screen_indice_inicio_pant) {
                                                                        if (t_scanline==linea_raster-192-screen_total_borde_inferior) disparada_raster=1;
                                                                }

                                                                //Esto es zona de paper o borde inferior
                                                                //Aqui el contador raster tiene valor 0 .. <(192+56)
                                                                //contador de scanlines del core, entre screen_indice_inicio_pant y screen_testados_total
                                                                else {
                                                                        if (t_scanline-screen_indice_inicio_pant==linea_raster) disparada_raster=1;
                                                                }

https://github.com/z88dk/z88dk/blob/master/libsrc/_DEVELOPMENT/target/zxn/config/config_zxn_copper.m4#L74

# 50Hz                          60Hz
# Lines                         Lines
#
#   0-191  Display                0-191  Display
# 192-247  Bottom Border        192-223  Bottom Border
# 248-255  Vsync (interrupt)    224-231  Vsync (interrupt)
# 256-311 Top Border 232-261 Top Border


# Horizontally the display is the same in 50Hz or 60Hz mode but it
# varies by model.  It consists of 448 pixels (0-447) in 48k mode
# and 456 pixels (0-455) in 128k mode.  Grouped in eight pixels
# that's screen bytes 0-55 in 48k mode and 0-56 in 128k mode.
#
# 48k mode                      128k mode
# Bytes  Pixels                 Bytes  Pixels
#
#  0-31    0-255  Display        0-31    0-255  Display
# 32-39  256-319  Right Border  32-39  256-319  Right Border
# 40-51  320-415  HBlank        40-51  320-415  HBlank
# 52-55  416-447  Left Border   52-56  416-455  Left Border
#
# The ZXN Copper understands two operations:
#
# (1) Wait for a particular line (0-311 @ 50Hz or 0-261 @ 60Hz)
#     and a horizontal character position (0-55 or 0-56)
#
# (2) Write a value to a nextreg.

int screen_invisible_borde_superior;
//normalmente a 56.
int screen_borde_superior;

//estos dos anteriores se suman aqui. es 64 en 48k, y 63 en 128k. por tanto, uno de los dos valores vale 1 menos
int screen_indice_inicio_pant;

//suma del anterior+192
int screen_indice_fin_pant;

//normalmente a 56
int screen_total_borde_inferior;

zona borde invisible: 0 .. screen_invisible_borde_superior;
zona borde visible: screen_invisible_borde_superior .. screen_invisible_borde_superior+screen_borde_superior
zona visible pantalla: screen_indice_inicio_pant .. screen_indice_inicio_pant+192
zona inferior: screen_indice_fin_pant .. screen_indice_fin_pant+screen_total_borde_inferior 



*/

int tbblue_get_current_raster_position(void)
{
	int raster;

	if (t_scanline<screen_invisible_borde_superior) {
		//En zona borde superior invisible (vsync)
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline;

		//Sumamos offset de la zona raster
		
		raster +=192+screen_total_borde_inferior;
		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
		return raster;
	}

	if (t_scanline<screen_indice_inicio_pant) {
		//En zona borde superior visible
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline-screen_invisible_borde_superior;

		//Sumamos offset de la zona raster
		raster +=192+screen_total_borde_inferior+screen_invisible_borde_superior;

		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
		return raster;
	}

	if (t_scanline<screen_indice_fin_pant) {
		//En zona visible pantalla
		//Ajustamos primero a desplazamiento entre 0 y esa zona
		raster=t_scanline-screen_indice_inicio_pant;

		//Sumamos offset de la zona raster
                raster +=0  ; //solo para que quede mas claro

		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
                return raster;
        }

	//Caso final. Zona borde inferior
		//Ajustamos primero a desplazamiento entre 0 y esa zona
                raster=t_scanline-screen_indice_fin_pant;

		//Sumamos offset de la zona raster
                raster +=192;
		//printf ("scanline: %d raster: %d\n",t_scanline,raster);
                return raster;

}


int tbblue_get_current_raster_horiz_position(void)
{
/*
# Horizontally the display is the same in 50Hz or 60Hz mode but it
# varies by model.  It consists of 448 pixels (0-447) in 48k mode
# and 456 pixels (0-455) in 128k mode.  Grouped in eight pixels
# that's screen bytes 0-55 in 48k mode and 0-56 in 128k mode.
#
# 48k mode                      128k mode
# Bytes  Pixels                 Bytes  Pixels
#
#  0-31    0-255  Display        0-31    0-255  Display
# 32-39  256-319  Right Border  32-39  256-319  Right Border
# 40-51  320-415  HBlank        40-51  320-415  HBlank
# 52-55  416-447  Left Border   52-56  416-455  Left Border
*/
	int estados_en_linea=t_estados % screen_testados_linea;
	int horizontal_actual=estados_en_linea;

	//Dividir por la velocidad turbo
	horizontal_actual /=cpu_turbo_speed;

	//Con esto tendremos rango entre 0 y 223. Multiplicar por dos para ajustar a rango 0-448
	horizontal_actual *=2;

	//Dividir entre 8 para ajustar  rango 0-56
	horizontal_actual /=8;

	return horizontal_actual;

}


//Diferentes paletas
//Total:
//     000 = ULA first palette
//     100 = ULA secondary palette
//     001 = Layer 2 first palette
//    101 = Layer 2 secondary palette
//     010 = Sprites first palette 
//     110 = Sprites secondary palette
//     011 = Tilemap first palette
//     111 = Tilemap second palette
//Paletas de 256 colores formato RGB9 RRRGGGBBB
//Valores son de 9 bits por tanto lo definimos con z80_int que es de 16 bits
z80_int tbblue_palette_ula_first[256];
z80_int tbblue_palette_ula_second[256];
z80_int tbblue_palette_layer2_first[256];
z80_int tbblue_palette_layer2_second[256];
z80_int tbblue_palette_sprite_first[256];
z80_int tbblue_palette_sprite_second[256];
z80_int tbblue_palette_tilemap_first[256];
z80_int tbblue_palette_tilemap_second[256];


//Diferentes layers a componer la imagen final
/*
(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  bits 4-2 = set layers priorities:
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S
 */

//Si en zona pantalla y todo es transparente, se pone un 0
//Layers con el indice al olor final en la paleta RGB9 (0..511)

//borde izquierdo + pantalla + borde derecho, multiplicado por 2
#define TBBLUE_LAYERS_PIXEL_WIDTH ((48+256+48)*2)

z80_int tbblue_layer_ula[TBBLUE_LAYERS_PIXEL_WIDTH];
z80_int tbblue_layer_layer2[TBBLUE_LAYERS_PIXEL_WIDTH];
z80_int tbblue_layer_sprites[TBBLUE_LAYERS_PIXEL_WIDTH];

/* 
Clip window registers

(R/W) 0x18 (24) => Clip Window Layer 2
  bits 7-0 = Coords of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  Reads do not advance the clip position
  The values are 0,255,0,191 after a Reset

(R/W) 0x19 (25) => Clip Window Sprites
  bits 7-0 = Cood. of the clip window
  1st write - X1 position
  2nd write - X2 position
  3rd write - Y1 position
  4rd write - Y2 position
  The values are 0,255,0,191 after a Reset
  Reads do not advance the clip position
  When the clip window is enabled for sprites in "over border" mode,
  the X coords are internally doubled and the clip window origin is
  moved to the sprite origin inside the border.

(R/W) 0x1A (26) => Clip Window ULA/LoRes
  bits 7-0 = Coord. of the clip window
  1st write = X1 position
  2nd write = X2 position
  3rd write = Y1 position
  4rd write = Y2 position
  The values are 0,255,0,191 after a Reset
  Reads do not advance the clip position

(R/W) 0x1B (27) => Clip Window Tilemap
  bits 7-0 = Coord. of the clip window
  1st write = X1 position
  2nd write = X2 position
  3rd write = Y1 position
  4rd write = Y2 position
  The values are 0,159,0,255 after a Reset, Reads do not advance the clip position, The X coords are internally doubled (in 40x32 mode, quadrupled in 80x32)

(W) 0x1C (28) => Clip Window control
  bits 7-4 = Reserved, must be 0
  bit 3 - reset the Tilemap clip index.
  bit 2 - reset the ULA/LoRes clip index.
  bit 1 - reset the sprite clip index.
  bit 0 - reset the Layer 2 clip index.

(R) 0x1C (28) => Clip Window control
  (may change)
  bits 7-6 = Tilemap clip index
  bits 5-4 = Layer 2 clip index
  bits 3-2 = Sprite clip index
  bits 1-0 = ULA clip index
*/

z80_byte clip_windows[4][4];                    // memory array to store actual clip windows

void tbblue_inc_clip_window_index(const z80_byte index_mask) {
    const z80_byte inc_one = (index_mask<<1) ^ index_mask;   // extract bottom bit of mask (+garbage in upper bits)
    const z80_byte inc_index = (tbblue_registers[28] + inc_one) & index_mask;
    tbblue_registers[28] &= ~index_mask;        // clear old index value
    tbblue_registers[28] |= inc_index;          // set new index value
}

// shifts and masks how the clip-window index is stored in tbblue_registers[28]
#define TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT   4
#define TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK    (3<<TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT)
#define TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT  2
#define TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK   (3<<TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT)
#define TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT      0
#define TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK       (3<<TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT)
#define TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT  6
#define TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK   (3<<TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT)

z80_byte tbblue_get_clip_window_layer2_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_LAYER2_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_sprites_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_SPRITES_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_ula_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_ULA_INDEX_SHIFT;
}

z80_byte tbblue_get_clip_window_tilemap_index(void) {
    return (tbblue_registers[28] & TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK)>>TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_SHIFT;
}

void tbblue_inc_clip_window_layer2_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK);
}

void tbblue_reset_clip_window_layer2_index(void) {
    tbblue_registers[28] &= ~TBBLUE_CLIP_WINDOW_LAYER2_INDEX_MASK;
}

void tbblue_inc_clip_window_sprites_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK);
}

void tbblue_reset_clip_window_sprites_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_SPRITES_INDEX_MASK);
}

void tbblue_inc_clip_window_ula_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK);
}

void tbblue_reset_clip_window_ula_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_ULA_INDEX_MASK);
}

void tbblue_inc_clip_window_tilemap_index(void) {
    tbblue_inc_clip_window_index(TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK);
}

void tbblue_reset_clip_window_tilemap_index(void) {
    tbblue_registers[28] &= ~(TBBLUE_CLIP_WINDOW_TILEMAP_INDEX_MASK);
}

//Forzar desde menu a desactivar capas 
z80_bit tbblue_force_disable_layer_ula={0};
z80_bit tbblue_force_disable_layer_tilemap={0};
z80_bit tbblue_force_disable_layer_sprites={0};
z80_bit tbblue_force_disable_layer_layer_two={0};


//Damos la paleta que se esta leyendo o escribiendo en una operacion de I/O
//Para ello mirar bits 6-4  de reg 0x43
z80_int *tbblue_get_palette_rw(void)
{
/*
(R/W) 0x43 (67) => Palette Control
  bit 7 = '1' to disable palette write auto-increment.
  bits 6-4 = Select palette for reading or writing:
     000 = ULA first palette
     100 = ULA secondary palette
     001 = Layer 2 first palette
     101 = Layer 2 secondary palette
     010 = Sprites first palette 
     110 = Sprites secondary palette
     011 = Tilemap first palette
     111 = Tilemap second palette
  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	z80_byte active_palette=(tbblue_registers[0x43]>>4)&7;

	switch (active_palette) {
		case 0:
			return tbblue_palette_ula_first;
		break;

		case 4:
			return tbblue_palette_ula_second;
		break;

		case 1:
			return tbblue_palette_layer2_first;
		break;

		case 5:
			return tbblue_palette_layer2_second;
		break;

		case 2:
			return tbblue_palette_sprite_first;
		break;

		case 6:
			return tbblue_palette_sprite_second;
		break;

		case 3:
			return tbblue_palette_tilemap_first;
		break;		

		case 7:
			return tbblue_palette_tilemap_second;
		break;				

		//por defecto retornar siempre ULA first palette
		default:
			return tbblue_palette_ula_first;
		break;
	}
}

//Damos el valor del color de la paleta que se esta leyendo o escribiendo en una operacion de I/O
z80_int tbblue_get_value_palette_rw(z80_byte index)
{
	z80_int *paleta;

	paleta=tbblue_get_palette_rw();

	return paleta[index];
}


//Modificamos el valor del color de la paleta que se esta leyendo o escribiendo en una operacion de I/O
void tbblue_set_value_palette_rw(z80_byte index,z80_int valor)
{
	z80_int *paleta;

	paleta=tbblue_get_palette_rw();

	paleta[index]=valor;
}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para sprites
//Para ello mirar bit 3 de reg 0x43
z80_int tbblue_get_palette_active_sprite(z80_byte index)
{
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&8) return tbblue_palette_sprite_second[index];
	else return tbblue_palette_sprite_first[index];

}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para layer2
//Para ello mirar bit 2 de reg 0x43
z80_int tbblue_get_palette_active_layer2(z80_byte index)
{
/*
(R/W) 0x70 (112) => Layer 2 Control
  bits 7:6 = Reserved, must be 0
  bits 5:4 = Layer 2 resolution (soft reset = 0): 00 = 256x192x8, 01 = 320x256x8, 10 = 640x256x4
  bits 3:0 = Palette offset (soft reset = 0)
*/
	index += tbblue_registers[112]<<4;
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&4) return tbblue_palette_layer2_second[index];
	else return tbblue_palette_layer2_first[index];

}

//Damos el valor del color de la paleta que se esta mostrando en pantalla para ula
//Para ello mirar bit 1 de reg 0x43
z80_int tbblue_get_palette_active_ula(z80_byte index)
{
/*
(R/W) 0x43 (67) => Palette Control

  bit 3 = Select Sprites palette (0 = first palette, 1 = secondary palette)
  bit 2 = Select Layer 2 palette (0 = first palette, 1 = secondary palette)
  bit 1 = Select ULA palette (0 = first palette, 1 = secondary palette)
*/
	if (tbblue_registers[0x43]&2) return tbblue_palette_ula_second[index];
	else return tbblue_palette_ula_first[index];

}


//Damos el valor del color de la paleta que se esta mostrando en pantalla para tiles
z80_int tbblue_get_palette_active_tilemap(z80_byte index)
{
/*
Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
*/
	if (tbblue_registers[0x6B] & 16) return tbblue_palette_tilemap_second[index];
        else return tbblue_palette_tilemap_first[index];

}


//64 patterns de Sprites
z80_byte tbsprite_patterns[TBBLUE_MAX_PATTERNS*TBBLUE_8BIT_PATTERN_SIZE];

int tbsprite_pattern_get_offset_index(z80_byte pattern_7b_id,z80_byte byte_offset)
{
	return (pattern_7b_id*TBBLUE_4BIT_PATTERN_SIZE+byte_offset)%(TBBLUE_MAX_PATTERNS*TBBLUE_8BIT_PATTERN_SIZE);
}

z80_byte tbsprite_pattern_get_value_index(z80_byte pattern_7b_id,z80_byte byte_offset)
{
	return tbsprite_patterns[tbsprite_pattern_get_offset_index(pattern_7b_id,byte_offset)];
}

void tbsprite_pattern_put_value_index(z80_byte pattern_7b_id,z80_byte byte_offset,z80_byte value)
{
	tbsprite_patterns[tbsprite_pattern_get_offset_index(pattern_7b_id,byte_offset)]=value;
}


/*
Old sprites description, needs update with core3.0 sprites and full description of fifth byte)
[0] 1st: X position (bits 7-0).
[1] 2nd: Y position (0-255).
[2] 3rd: bits 7-4 is palette offset, bit 3 is X MSB, bit 2 is X mirror, bit 1 is Y mirror and bit 0 is visible flag.
[3] 4th: bits 7-6 is reserved, bits 5-0 is Name (pattern index, 0-63).
[4] 5th: anchor/relative/compound extras (only used when [3] bit6 is "1")
*/
z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][TBBLUE_SPRITE_ATTRIBUTE_SIZE];

//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion (0..3 en sprites o 0..255 en pattern ) apunta
z80_byte tbsprite_index_pattern,tbsprite_index_pattern_subindex;
z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;
z80_byte tbsprite_nr_index_sprite;

/*
Port 0x303B, if read, returns some information:

Bits 7-2: Reserved, must be 0.
Bit 1: max sprites per line flag.
Bit 0: Collision flag.
Port 0x303B, if written, defines the sprite slot to be configured by ports 0x55 and 0x57, and also initializes the address of the palette.

*/

z80_byte tbblue_port_303b;		// "read only" part

int tbsprite_is_lockstep()
{
	return (tbblue_registers[9]&0x10);
}

void tbsprite_increment_index_303b() {	// increment the "port" index
	tbsprite_index_sprite_subindex=0;
	tbsprite_index_sprite++;
	tbsprite_index_sprite %= TBBLUE_MAX_SPRITES;
}



/* Informacion relacionada con Layer2. Puede cambiar en el futuro, hay que ir revisando info en web de Next

Registros internos implicados:

(R/W) 0x12 (18) => Layer 2 RAM page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 8 after a Reset)

(R/W) 0x13 (19) => Layer 2 RAM shadow page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 11 after a Reset)

(R/W) 0x14 (20) => Global transparency color
  bits 7-0 = Transparency color value (Reset to 0xE3, after a reset)
  (Note this value is 8-bit only, so the transparency is compared only by the MSB bits of the final colour)



(R/W) 0x16 (22) => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)

(R/W) 0x17 (23) => Layer2 Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)




Posiblemente registro 20 aplica a cuando el layer2 esta por detras de pantalla de spectrum, y dice el color de pantalla de spectrum
que actua como transparente
Cuando layer2 esta encima de pantalla spectrum, el color transparente parece que es el mismo que sprites: TBBLUE_TRANSPARENT_COLOR 0xE3

Formato layer2: 256x192, linear, 8bpp, RRRGGGBB (mismos colores que sprites), ocupa 48kb

Se accede en modo escritura en 0000-3fffh mediante puerto:

Banking in Layer2 is out 4667 ($123B)
bit 0 = write enable, which changes writes from 0-3fff to write to layer2,
bit 1 = Layer2 ON or OFF set=ON,
bit 2 = ????
bit 3 = Use register 19 instead of 18 to tell sram page
bit 4 puts layer 2 behind the normal spectrum screen
bit 6 and 7 are to say which 16K section is paged in,
$03 = 00000011b Layer2 on and writable and top third paged in at $0000,
$43 = 01000011b Layer2 on and writable and middle third paged in at $0000,
$C3 = 11000011b Layer2 on and writable and bottom third paged in at $0000,  ?? sera 100000011b??? TODO
$02 = 00000010b Layer2 on and nothing paged in. etc

Parece que se mapea la pagina de sram indicada en registro 19

*/


/*

IMPORTANT!!

Trying some old layer2 demos that doesn't set register 19 is dangerous.
To avoid problems, first do:
out 9275, 19
out 9531,32
To set layer2 to the extra ram:
0x080000 – 0x0FFFFF (512K) => Extra RAM

Then load the demo program and will work

*/

z80_byte tbblue_port_123b;
z80_byte tbblue_port_123b_b4;	// bank offset register (bits 2-0: +0..+7 relative offset)


//valor inicial para tbblue_port_123b en caso de fast boot mode
int tbblue_initial_123b_port=-1;

int tbblue_write_on_layer2(void)
{
	if (tbblue_port_123b &1) return 1;
	return 0;
}

int tbblue_read_on_layer2(void)
{
	if (tbblue_port_123b & 4) return 1;
	return 0;
}

int tbblue_is_active_layer2(void)
{
	if (tbblue_port_123b & 2) return 1;
	return 0;
}

int tbblue_is_layer2_256height(void)
{
	// returns true for 320x256 or 640x256 Layer 2 modes, false for 256x192 and "11" setting (reserved)
	return (0x20 & (tbblue_registers[112] + 0x10));
}

int tbblue_get_offset_start_layer2_reg(z80_byte register_value)
{
	//since core3.0 the NextRegs 0x12 and 0x13 are 7bit.
	int offset=register_value&127;
	//due to 7bit the value can leak outside of 2MiB
	// in HW the reads outside of SRAM module are "unspecified result", writes are ignored (!)

	offset*=16384;

	//Y empezar en 0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM
	/*
	recordemos
	    0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM			(16 paginas) 
    0x060000 – 0x07FFFF (128K) => Extra RAM				(16 paginas)

    0x080000 – 0x0FFFFF (512K) => 1st Extra IC RAM (if present)		(64 paginas)
    0x100000 – 0x17FFFF (512K) => 2nd Extra IC RAM (if present)		(64 paginas)
    0x180000 – 0xFFFFFF (512K) => 3rd Extra IC RAM (if present)		(64 paginas)

    0x200000 (2 MB)

    	Con and 63, maximo layer 2 son 64 paginas
    	64*16384=1048576  -> 1 mb total
    	empezando en 0x040000 + 1048576 = 0x140000 y no nos salimos de rango (estariamos en el 2nd Extra IC RAM)
    	Dado que siempre asigno 2 mb para tbblue, no hay problema

    	*/



	offset +=0x040000;

	return offset;

}

int tbblue_get_offset_start_layer2(void)
{
	if (tbblue_port_123b & 8 ) return tbblue_get_offset_start_layer2_reg(tbblue_registers[19]);
	else return tbblue_get_offset_start_layer2_reg(tbblue_registers[18]);

}

int tbblue_get_offset_start_tilemap(void)
{
	return tbblue_registers[110]&63;
}


int tbblue_get_offset_start_tiledef(void)
{
	return tbblue_registers[111]&63;
}

int tbblue_get_tilemap_width(void)
{
	if (tbblue_registers[107]&64) return 80;
	else return 40;
}

int tbblue_if_ula_is_enabled(void)
{
	/*
(R/W) 0x68 (104) => ULA Control
  bit 7    = 1 to disable ULA output
  bit 6    = 0 to select the ULA colour for blending in SLU modes 6 & 7
           = 1 to select the ULA/tilemap mix for blending in SLU modes 6 & 7
  bits 5-1 = Reserved must be 0
  bit 0    = 1 to enable stencil mode when both the ULA and tilemap are enabled
            (if either are transparent the result is transparent otherwise the
             result is a logical AND of both colours)
						 */

	if (tbblue_registers[104]&128) return 0;
	else return 1;
}

void tbblue_reset_sprites(void)
{

	int i;

	

	//Resetear patterns todos a transparente
	for (i=0;i<TBBLUE_MAX_PATTERNS;i++) {
		int j;
		for (j=0;j<256;j++) {
			tbsprite_pattern_put_value_index(i<<1,j,TBBLUE_DEFAULT_TRANSPARENT);
		}
	}

	//Poner toda info de sprites a 0. Seria quiza suficiente con poner bit de visible a 0
	for (i=0;i<TBBLUE_MAX_SPRITES;i++) {
		int j;
		for (j=0;j<TBBLUE_SPRITE_ATTRIBUTE_SIZE;++j) {
			tbsprite_sprites[i][j]=0;
		}
	}


	tbsprite_index_pattern=tbsprite_index_pattern_subindex=0;
	tbsprite_index_sprite=tbsprite_index_sprite_subindex=0;
	tbsprite_nr_index_sprite=0;

	tbblue_port_303b=0;

	tbblue_registers[22]=0;
	tbblue_registers[23]=0;


}

z80_int tbblue_get_9bit_colour(z80_byte valor)
{
	//Retorna color de 9 bits en base a 8 bits
	z80_int valor16=valor;

	//Bit bajo sale de hacer or de bit 1 y 0
	//z80_byte bit_bajo=valor&1;
	z80_byte bit_bajo=(valor&1)|((valor&2)>>1);

	//rotamos a la izquierda para que sean los 8 bits altos
	valor16=valor16<<1;

	valor16 |=bit_bajo;

	return valor16;	
}

void tbblue_reset_palettes(void)
{
	//Inicializar Paletas
	int i;

//z80_int valor16=tbblue_get_9bit_colour(valor);

	//Paletas layer2 & sprites son los mismos valores del indice*2 y metiendo bit 0 como el bit1 inicial
	//(cosa absurda pues no sigue la logica de mezclar bit 0 y bit 1 usado en registro 41H)
	for (i=0;i<256;i++) {
		z80_int color;
		color=i*2;
		if (i&2) color |=1;

 		tbblue_palette_layer2_first[i]=color;
 		tbblue_palette_layer2_second[i]=color;
 		tbblue_palette_sprite_first[i]=color;
 		tbblue_palette_sprite_second[i]=color;
	}


	//Se repiten los 16 colores en los 256. Para colores sin brillo, componente de color vale 5 (101b)
	const z80_int tbblue_default_ula_colours[16]={
0 ,   //000000000 
5 ,   //000000101 
320 , //101000000 
325 , //101000101 
40 ,  //000101000 
45 ,  //000101101 
360 , //101101000 
365 , //101101101 
0 ,   //000000000 
7 ,   //000000111 
448 , //111000000 
455 , //111000111 
56 ,  //000111000 
63 ,  //000111111 
504 , //111111000 
511   //111111111 
	};
	/*
	Nota: para convertir una lista de valores binarios en decimal:
	LINEA=""

while read LINEA; do
echo -n "$((2#$LINEA))"
echo " , //$LINEA "


done < /tmp/archivo_lista.txt
	*/

	int j;

	for (j=0;j<16;j++) {
		for (i=0;i<16;i++) {
			int colorpaleta=tbblue_default_ula_colours[i];


	//bright magenta son colores transparentes por defecto (1C7H y 1C6H  / 2 = E3H)
	//lo cambio a 1CF, que es un color FF24FFH, que no es magenta puro, pero evita el problema de transparente por defecto
	//esto lo corrige igualmente nextos al arrancar, pero si arrancamos tbblue en modo fast-boot, pasaria que los bright
	//magenta se verian transparentes
			if (i==11) colorpaleta=0x1CF;


			tbblue_palette_ula_first[j*16+i]=colorpaleta;
			tbblue_palette_ula_second[j*16+i]=colorpaleta;

		}
	}



}


void tbblue_out_port_sprite_index(z80_byte value)
{
	//printf ("Out tbblue_out_port_sprite_index %02XH\n",value);
	tbsprite_index_pattern=value%TBBLUE_MAX_PATTERNS;
	tbsprite_index_pattern_subindex=value&0x80;
	tbsprite_index_sprite=value%TBBLUE_MAX_SPRITES;
	tbsprite_index_sprite_subindex=0;
}


//Indica si al escribir registro 44h de paleta:
//si 0, se escribe 8 bits superiores
//si no 0, se escribe 1 bit inferior
int tbblue_write_palette_state=0;

void tbblue_reset_palette_write_state(void)
{
	tbblue_write_palette_state=0;
}

void tbblue_increment_palette_index(void)
{
//(R/W) 0x43 (67) => Palette Control
//  bit 7 = '1' to disable palette write auto-increment.

	if ((tbblue_registers[0x43] & 128)==0) {
		z80_byte indice=tbblue_registers[0x40];
		indice++;
		tbblue_registers[0x40]=indice;
	}

	tbblue_reset_palette_write_state();
}



//Escribe valor de 8 bits superiores (de total de 9) para indice de color de paleta 
void tbblue_write_palette_value_high8(z80_byte valor)
{
/*
(R/W) 0x40 (64) => Palette Index
  bits 7-0 = Select the palette index to change the default colour. 
  0 to 127 indexes are to ink colours and 128 to 255 indexes are to papers.
  (Except full ink colour mode, that all values 0 to 255 are inks)
  Border colours are the same as paper 0 to 7, positions 128 to 135,
  even at full ink mode. 
  (inks and papers concept only applies to Enhanced ULA palette. 
  Layer 2 and Sprite palettes works as "full ink" mode)

  (R/W) 0x41 (65) => Palette Value (8 bit colour)
  bits 7-0 = Colour for the palette index selected by the register 0x40. Format is RRRGGGBB
  Note the lower blue bit colour will be an OR between bit 1 and bit 0. 
  After the write, the palette index is auto-incremented to the next index. 
  The changed palette remains until a Hard Reset.

(R/W) 0x43 (67) => Palette Control
  bit 7 = '1' to disable palette write auto-increment.

*/
	z80_byte indice=tbblue_registers[0x40];

	z80_int valor16=tbblue_get_9bit_colour(valor);

	tbblue_set_value_palette_rw(indice,valor16);
}

#define TBBLUE_LAYER2_PRIORITY 0x8000

//Escribe valor de paleta de registro 44H, puede que se escriba en 8 bit superiores o en 1 inferior
void tbblue_write_palette_value(z80_byte high8, z80_byte low1)
{
/*
0x40 (64) => Palette Index
(R/W)
  bits 7:0 = Select the palette index to change the associated colour. (soft reset = 0)

0x44 (68) => Palette Value (9 bit colour)
(R/W)
  Two consecutive writes are needed to write the 9 bit colour
  1st write:
    bits 7:0 = RRRGGGBB
  2nd write:
    bits 7:1 = Reserved, must be 0
    bit 0 = lsb B
    If writing to an L2 palette
    bit 7 = 1 for L2 priority colour, 0 for normal.
      An L2 priority colour moves L2 above all layers.  If you need the same
      colour in both priority and normal modes, you will need to have two
      different entries with the same colour one with and one without priority.
  After two consecutive writes the palette index is auto-incremented if
  auto-increment is enabled in nextreg 0x43.
  Reads only return the 2nd byte and do not auto-increment.
*/
	z80_byte indice=tbblue_registers[0x40];

	z80_int color9b=(z80_int)high8+high8+(low1&1);

	if ((low1&128) && 0x10 == (tbblue_registers[0x43]&0x30)) {
		// layer 2 palette has extra priority bit in color (must be removed while mixing layers)
		color9b |= TBBLUE_LAYER2_PRIORITY;
	}

	//printf("writing full 9b color: palette_%d[%d] = %04X [from: %02X, %02X]\n",(tbblue_registers[0x43]>>4)&7,indice,color9b,high8,low1);

	tbblue_set_value_palette_rw(indice,color9b);

	tbblue_increment_palette_index();
}


void tbblue_out_sprite_pattern(z80_byte value)
{




	tbsprite_pattern_put_value_index(tbsprite_index_pattern<<1,tbsprite_index_pattern_subindex,value);



	if (tbsprite_index_pattern_subindex==255) {
		tbsprite_index_pattern_subindex=0;
		tbsprite_index_pattern++;
		if (tbsprite_index_pattern>=TBBLUE_MAX_PATTERNS) tbsprite_index_pattern=0;
	}
	else tbsprite_index_pattern_subindex++;

}

void tbblue_out_sprite_sprite(z80_byte value)
{
	//printf ("Out tbblue_out_sprite_sprite. Index: %d subindex: %d %02XH\n",tbsprite_index_sprite,tbsprite_index_sprite_subindex,value);



	//Indices al indicar paleta, pattern, sprites. Subindex indica dentro de cada pattern o sprite a que posicion (0..3 en sprites o 0..255 en pattern ) apunta
	//z80_byte tbsprite_index_sprite,tbsprite_index_sprite_subindex;

	tbsprite_sprites[tbsprite_index_sprite][tbsprite_index_sprite_subindex]=value;
	if (3 == tbsprite_index_sprite_subindex && 0 == (value&0x40)) {			// 4-byte type, add 0 as fifth
		tbsprite_sprites[tbsprite_index_sprite][++tbsprite_index_sprite_subindex]=0;
	}
	if (++tbsprite_index_sprite_subindex == TBBLUE_SPRITE_ATTRIBUTE_SIZE) {
		tbsprite_increment_index_303b();
	}
}


//Guarda scanline actual y el pattern (los indices a colores) sobre la paleta activa de sprites
//z80_byte sprite_line[MAX_X_SPRITE_LINE];

#define TBBLUE_SPRITE_TRANS_FICT 65535

//Dice si un color de la capa de sprites es igual al color transparente ficticio inicial
int tbblue_si_sprite_transp_ficticio(z80_int color)
{
        if (color==TBBLUE_SPRITE_TRANS_FICT) return 1;
        return 0;
}


//Dice si un color de la paleta rbg9 es transparente
int tbblue_si_transparent(z80_int color)
{
	//if ( (color&0x1FE)==TBBLUE_TRANSPARENT_COLOR) return 1;
	color=(color>>1)&0xFF;
	if (color==TBBLUE_TRANSPARENT_REGISTER) return 1;
	return 0;
}


/*
Port 0x243B is write-only and is used to set the registry number.

Port 0x253B is used to access the registry value.

Register:
(R/W) 21 => Sprite system
 bits 7-2 = Reserved, must be 0
 bit 1 = Over border (1 = yes)
 bit 0 = Sprites visible (1 = visible)
*/
void tbsprite_put_color_line(int x,z80_byte color)
{
	//Si index de color es transparente, no hacer nada
/*
The sprites have now a new register for sprite transparency. Unlike the Global Transparency Colour register this refers to an index and  should be set when using indices other than 0xE3:

(R/W) 0x4B (75) => Transparency index for Sprites
bits 7-0 = Set the index value. (0XE3 after a reset)
	*/

	int xfinal=x;

	xfinal +=screen_total_borde_izquierdo*border_enabled.v;
	xfinal -=TBBLUE_SPRITE_BORDER;

	xfinal *=2; //doble de ancho



	//Ver si habia un color y activar bit colision
	const z80_int color_antes=tbblue_layer_sprites[xfinal];

	//if (!tbblue_si_transparent(color_antes)) {
	if (!tbblue_si_sprite_transp_ficticio(color_antes) ) {
		//colision
		tbblue_port_303b |=1;
		//printf ("set colision flag. result value: %d\n",tbblue_port_303b);

		// NextReg 0x15 bit6 = rendering priority: 1 = Sprite 0 on top, 0 = Sprite 0 at bottom
		const int rendering_priority = tbblue_registers[21]&0x40;
		if (rendering_priority) return;			// keep pixel of previous sprite rendered
	}

	const z80_int color_final=tbblue_get_palette_active_sprite(color);
	//sprite_line[x]=color;
	tbblue_layer_sprites[xfinal]=color_final;
	tbblue_layer_sprites[xfinal+1]=color_final; //doble de ancho

}

z80_byte tbsprite_do_overlay_get_pattern_xy(z80_byte pattern_7b_id,z80_byte sx,z80_byte sy)
{
	return tbsprite_pattern_get_value_index(pattern_7b_id,sy*TBBLUE_SPRITE_WIDTH+sx);
}

z80_byte tbsprite_do_overlay_get_4bpppattern_xy(z80_byte pattern_7b_id,z80_byte sx,z80_byte sy)
{
	z80_byte offset = (sy*TBBLUE_SPRITE_WIDTH+sx)>>1;
	if (sx&1) {
		return tbsprite_pattern_get_value_index(pattern_7b_id,offset)&0x0F;
	} else {
		return tbsprite_pattern_get_value_index(pattern_7b_id,offset)>>4;
	}
}

z80_int tbsprite_return_color_index(z80_byte index)
{
	//z80_int color_final=tbsprite_palette[index];

	z80_int color_final=tbblue_get_palette_active_sprite(index);
	//return RGB9_INDEX_FIRST_COLOR+color_final;
	return color_final;
}

int tbblue_if_sprites_enabled(void) 
{

	return tbblue_registers[21]&1;

}


int tbblue_if_tilemap_enabled(void) 
{

	return tbblue_registers[107]&128;

}

struct s_tbsprite_anchor_data {
	int			x, y;
	z80_byte	pal_offset;
	z80_byte	pattern_7b_id;
	z80_byte	x_scale, y_scale;
	z80_byte	visible : 1, unified_anchor : 1, x_mirror : 1, y_mirror : 1, rotate : 1;
};

int tbsprite_do_overlay(void)
{

		if (!tbblue_if_sprites_enabled() ) return 0;

        int y=t_scanline_draw-screen_indice_inicio_pant+TBBLUE_SPRITE_BORDER;
			//first paper line is +32
			// sprites have coordinate system going +-32px around paper area, i.e. y=0..255 (x=0..319)
			// with top-left pixel of paper being at sprite coordinates [32,32]

		int rangoxmin, rangoxmax, rangoymin, rangoymax;

		if (tbblue_registers[21]&2) {
			// sprites over border are by default not clipped ([0,0]->[319,255] area)
			rangoxmin=0;
			rangoxmax=TBBLUE_SPRITE_BORDER+255+TBBLUE_SPRITE_BORDER;
			rangoymin=0;
			rangoymax=TBBLUE_SPRITE_BORDER+191+TBBLUE_SPRITE_BORDER;
			if (tbblue_registers[21]&0x20) {
				// sprite clipping "over border" enabled, double the X coordinate of clip window
				rangoxmin=2*clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0];
				rangoxmax=2*clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1] + 1;
				rangoymin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2];
				rangoymax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3];
				if (TBBLUE_SPRITE_BORDER+255+TBBLUE_SPRITE_BORDER < rangoxmax) {
					// clamp rangoxmax to 319
					rangoxmax = TBBLUE_SPRITE_BORDER+255+TBBLUE_SPRITE_BORDER;
				}
			}
		} else {
			// take clip window coordinates, but limit them to [0,0]->[255,191] (and offset them +32,+32)
			rangoxmin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0] + TBBLUE_SPRITE_BORDER;
			rangoxmax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1] + TBBLUE_SPRITE_BORDER;
			rangoymin=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2] + TBBLUE_SPRITE_BORDER;
			rangoymax=clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3] + TBBLUE_SPRITE_BORDER;
			if (TBBLUE_SPRITE_BORDER+191 < rangoymax) {
				// clamp rangoymax to 32+191 (bottom edge of PAPER)
				rangoymax = TBBLUE_SPRITE_BORDER+191;
			}
		}

		if (y < rangoymin || rangoymax < y) return 0;

		int total_sprites=0;
		struct s_tbsprite_anchor_data anchor = { 0 };
		int is_4bpp = 0;

        //Bucle para cada sprite
        int conta_sprites;
		z80_byte pattern_7b_id, half_4bpp_pattern;

		int i;
		//int offset_pattern;

        for (conta_sprites=0;conta_sprites<TBBLUE_MAX_SPRITES && total_sprites<MAX_SPRITES_PER_LINE;conta_sprites++) {
			int sprite_x;
			int sprite_y;

			/*
			core3.0
			[0] 1st: X position (bits 7-0) (int9_t in total)   (relative sprites have int8_t -128..+127)
			[1] 2nd: Y position (bits 7-0) (int9_t or uint8_t) (relative sprites have int8_t -128..+127)
			[2] 3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is rotate flag and bit 0 is X MSB.
						(bit 0 "X8" is palette-offset-is-relative flag for relative sprites)
			[3] 4th: bit 7 is visible flag, bit 6 is 4B/5B type, bits 5-0 is Name (pattern index, 0-63)
			[4] 5th: bits 7-6:
				"1x" is 4bpp anchor sprite, "x" half-pattern bit, bit 5 0=composite/1=unified, bits 4-3 scaleX, bits 2-1 scaleY, bit 0 Y8 (ninth bit of Y)
				"00" is 8bpp anchor sprite, bits 5-0 same as "4bpp anchor sprite"
				"01" after composite anchor: bit 5 is half-pattern for 4bpp, bits 4-3 scaleX, bits 2-1 scaleY, bit 0 pattern is relative to anchor
				"01" after unified anchor: bit 5 is half-pattern for 4bpp, bits 4-1 reserved (0), bit 0 pattern is relative to anchor
			When [3] bit 6 is 0, the rendering is same as if [4] is zero (no scale, 8bpp, Y8=0, composite anchor)
			*/
			/*
			Because sprites can be displayed on top of the ZX Spectrum border, the coordinates of each sprite can range
			from 0 to 319 for the X axis and 0 to 255 for the Y axis. For both axes, values from 0 to 31 are reserved
			for the Left or top border, for the X axis the values 288 to 319 is reserved for the right border and for
			the Y axis values 224 to 255 for the lower border.
			With 5byte type both X and Y for anchor sprites are signed 9bit, i.e. "0x1FF" is -1 coordinate.
			Relative sprites have signed 8bit X/Y, i.e. -128..+127 only (X8 and Y8 have different meaning)

			If the display of the sprites on the border is disabled, the visible coordinates of the sprites range from (32,32) to (287,223).
			*/

			const z80_byte attr4 = tbsprite_sprites[conta_sprites][3]&0x40 ? tbsprite_sprites[conta_sprites][4] : 0;
			const int is_anchor = 0x40 != (attr4&0xC0);
			if (!(tbsprite_sprites[conta_sprites][3]&0x80)) {
				if (is_anchor) anchor.visible = 0;		// hides whole relative cluster
				continue;		// not visible
			}
			if (!is_anchor && !anchor.visible) continue;	// anchor not visible, skip this one
			//Si sprite visible
			z80_byte scaleX = (attr4>>3)&3, scaleY = (attr4>>1)&3;
			z80_byte mirror_x=tbsprite_sprites[conta_sprites][2]&8;
			z80_byte mirror_y=tbsprite_sprites[conta_sprites][2]&4;
			z80_byte sprite_rotate=tbsprite_sprites[conta_sprites][2]&2;
			//Offset paleta se lee tal cual sin rotar valor
			z80_byte palette_offset=tbsprite_sprites[conta_sprites][2] & 0xF0;

			if (is_anchor) {
				anchor.visible = 1;
				sprite_x=tbsprite_sprites[conta_sprites][0] | ((tbsprite_sprites[conta_sprites][2]&1)<<8);
				sprite_y=tbsprite_sprites[conta_sprites][1] | ((attr4&1)<<8);
				if (512-128 < sprite_x) sprite_x -= 512;		// -127 .. +384 (cover 8x scaleX)
				if (512-128 < sprite_y) sprite_y -= 512;		// -127 .. +384 (cover 8x scaleY)
				is_4bpp = attr4&0x80;
				half_4bpp_pattern = (attr4&0x40)>>6;	// will result into 0 for 8bpp type, 0/1 for 4bpp
				pattern_7b_id=((tbsprite_sprites[conta_sprites][3]&63)<<1)|half_4bpp_pattern;
				anchor.x = sprite_x;
				anchor.y = sprite_y;
				anchor.pattern_7b_id = pattern_7b_id;
				anchor.pal_offset = palette_offset;
				if (attr4&0x20) {
					anchor.unified_anchor = 1;			// unified type
					anchor.x_mirror = 0 != mirror_x;
					anchor.y_mirror = 0 != mirror_y;
					anchor.rotate = 0 != sprite_rotate;
					anchor.x_scale = scaleX;
					anchor.y_scale = scaleY;
				} else {
					anchor.unified_anchor = 0;			// composite type
				}
			} else {
				sprite_x=(signed char)tbsprite_sprites[conta_sprites][0];
				sprite_y=(signed char)tbsprite_sprites[conta_sprites][1];
				half_4bpp_pattern = is_4bpp ? (attr4&0x20)>>5 : 0;	// 0/1 for 4bpp, 0 for 8bpp
				pattern_7b_id=((tbsprite_sprites[conta_sprites][3]&63)<<1)|half_4bpp_pattern;
				// relative palette offset?
				if (tbsprite_sprites[conta_sprites][2]&1) palette_offset += anchor.pal_offset;
				// relative pattern?
				if (attr4&1) pattern_7b_id += anchor.pattern_7b_id;
				if (anchor.unified_anchor) {
					// transform child of unified anchor by the anchor mirror/scale/rotation
					scaleX = anchor.x_scale;			// scale is just copied from anchor
					scaleY = anchor.y_scale;
					if (anchor.rotate) {
						sprite_rotate = !sprite_rotate;
						int old_x = sprite_x;
						sprite_x = -sprite_y;
						sprite_y = old_x;
						z80_byte old_v = mirror_x;
						mirror_x = sprite_rotate ? mirror_y : !mirror_y;
						mirror_y = sprite_rotate ? old_v : !old_v;
					}
					if (anchor.x_mirror) {
						mirror_x = !mirror_x;
						sprite_x = -sprite_x;
					}
					if (anchor.y_mirror) {
						mirror_y = !mirror_y;
						sprite_y = -sprite_y;
					}
					sprite_x <<= scaleX;
					sprite_y <<= scaleY;
				}
				// update final relative coordinates
				sprite_x+=anchor.x;
				sprite_y+=anchor.y;
			}

			//Si coordenada y esta en margen y sprite activo
			int diferencia=(y-sprite_y)>>scaleY;

			// by here all the anchor/relative stuff has to be resolved (and positioning)
			if (diferencia < 0 || TBBLUE_SPRITE_HEIGHT <= diferencia) continue;

			//Pintar el sprite si esta en rango de coordenada y
			z80_byte sx=0,sy=0; //Coordenadas x,y dentro del pattern
			//offset_pattern=0;

			//Incrementos de x e y
			int incx=+1;
			int incy=0;

			//Aplicar mirror si conviene y situarnos en la ultima linea
			if (mirror_y) {
				//offset_pattern=offset_pattern+TBBLUE_SPRITE_WIDTH*(TBBLUE_SPRITE_HEIGHT-1);
				sy=TBBLUE_SPRITE_HEIGHT-1-diferencia;
				//offset_pattern -=TBBLUE_SPRITE_WIDTH*diferencia;
			} else {
				//offset_pattern +=TBBLUE_SPRITE_WIDTH*diferencia;
				sy=diferencia;
			}



			//Dibujar linea x

			//Cambiar offset si mirror x, ubicarlo a la derecha del todo
			if (mirror_x) {
				//offset_pattern=offset_pattern+TBBLUE_SPRITE_WIDTH-1;
				sx=TBBLUE_SPRITE_WIDTH-1;
				incx=-1;
			}

			/*
			Comparar bits rotacion con ejemplo en media/spectrum/tbblue/sprites/rotate_example.png
			*/
			/*
			Basicamente sin rotar un sprite, se tiene (reduzco el tamaño a la mitad aqui para que ocupe menos)


			El sentido normal de dibujado viene por ->, aumentando coordenada X


		->  ---X----
				---XX---
				---XXX--
				---XXXX-
				---X----
				---X----
				---X----
				---X----

				Luego cuando se rota 90 grados, en vez de empezar de arriba a la izquierda, se empieza desde abajo y reduciendo coordenada Y:

					---X----
						---XX---
						---XXX--
						---XXXX-
						---X----
						---X----
				^ 	---X----
				|		---X----

				Entonces, al dibujar empezando asi, la imagen queda rotada:

				--------
				--------
				XXXXXXXX
				----XXX-
				----XX--
				----X---
				--------

				De ahi que el incremento y sea -incremento x , incremento x sera 0

				Aplicando tambien el comportamiento para mirror, se tiene el resto de combinaciones

				*/


			if (sprite_rotate) {
				z80_byte sy_old=sy;
				sy=(TBBLUE_SPRITE_HEIGHT-1)-sx;
				sx=sy_old;

				incy=-incx;
				incx=0;
			}

			const z80_byte transparent_idx = is_4bpp ? tbblue_registers[75]&0x0F : tbblue_registers[75];

			for (i=0;i<TBBLUE_SPRITE_WIDTH;i++) {
				z80_byte index_color;
				if (is_4bpp) {
					index_color=tbsprite_do_overlay_get_4bpppattern_xy(pattern_7b_id,sx,sy);
				} else {
					index_color=tbsprite_do_overlay_get_pattern_xy(pattern_7b_id,sx,sy);
				}

					//Si index de color es transparente, no hacer nada
/*
The sprites have now a new register for sprite transparency. Unlike the Global Transparency Colour register this refers to an index and  should be set when using indices other than 0xE3:

(R/W) 0x4B (75) => Transparency index for Sprites
bits 7-0 = Set the index value. (0XE3 after a reset)
*/

				sx=sx+incx;
				sy=sy+incy;


				if (index_color!=transparent_idx) {

					//Sumar palette offset. Logicamente si es >256 el resultado, dará la vuelta el contador
					index_color +=palette_offset;

					if (scaleX) {
						const int scaleImax = (1<<scaleX);
						for (int scaleI = 0; scaleI < scaleImax; ++scaleI) {
							if (sprite_x+scaleI < rangoxmin) continue;
							if (rangoxmax < sprite_x+scaleI) break;
							tbsprite_put_color_line(sprite_x+scaleI,index_color);
						}
					} else {
						if (rangoxmin <= sprite_x && sprite_x <= rangoxmax) {
							tbsprite_put_color_line(sprite_x,index_color);
						}
					}

				}
				sprite_x += (1<<scaleX);
				if (rangoxmax < sprite_x) break;
			} //end of for (i=0;i<TBBLUE_SPRITE_WIDTH;i++)

			total_sprites++;
			//printf ("total sprites in this line: %d\n",total_sprites);
			if (total_sprites==MAX_SPRITES_PER_LINE) {
				//max sprites per line flag
				tbblue_port_303b |=2;
				//printf ("set max sprites per line flag\n");
			}

		}

	return 1;
}

z80_byte tbblue_get_port_layer2_value(void)
{
	return tbblue_port_123b;
}

void tbblue_out_port_layer2_value(z80_byte value)
{
	if (value & 0x10) {		// set relative bank offset (bit 4 = 1)
		tbblue_port_123b_b4 = value & 0x07;
	} else {
		tbblue_port_123b=value;
	}
}


z80_byte tbblue_get_port_sprite_index(void)
{
	/*
	Port 0x303B, if read, returns some information:

Bits 7-2: Reserved, must be 0.
Bit 1: max sprites per line flag.
Bit 0: Collision flag.
*/
	z80_byte value=tbblue_port_303b;
	//Cuando se lee, se resetean bits 0 y 1
	//printf ("-----Reading port 303b. result value: %d\n",value);
	tbblue_port_303b &=(255-1-2);

	return value;

}



//'bootrom' takes '1' on hard-reset and takes '0' if there is any writing on the i/o port 'config1'. It can not be read.
z80_bit tbblue_bootrom={1};

//Puerto tbblue de maquina/mapeo
/*
- Write in port 0x24DB (config1)

                case cpu_do(7 downto 6) is
                    when "01"    => maquina <= s_speccy48;
                    when "10"    => maquina <= s_speccy128;
                    when "11"    => maquina <= s_speccy3e;
                    when others    => maquina <= s_config;   ---config mode
                end case;
                romram_page <= cpu_do(4 downto 0);                -- rom or ram page
*/

//z80_byte tbblue_config1=0;

/*
Para habilitar las interfaces y las opciones que utilizo el puerto 0x24DD (config2). Para iniciar la ejecución de la máquina elegida escribo la máquina de la puerta 0x24DB y luego escribir 0x01 en puerta 0x24D9 (hardsoftreset), que activa el "SoftReset" y hace PC = 0.



Si el bit 7 es "0":

6 => "Lightpen" activado;
5 => "Multiface" activado;
4 => "PS/2" teclado o el ratón;
3-2 => el modo de joystick1;
1-0 => el modo de joystick2;

Puerta 0x24D9:

bit 1 => realizar un "hard reset" ( "maquina"=0,  y "bootrom" recibe '1' )
bit 0 => realizar un "soft reset" ( PC = 0x0000 )
*/

//z80_byte tbblue_config2=0;
//z80_byte tbblue_hardsoftreset=0;


//Si segmento bajo (0-16383) es escribible
z80_bit tbblue_low_segment_writable={0};


//Indice a lectura del puerto
//z80_byte tbblue_read_port_24d5_index=0;

//port 0x24DF bit 2 = 1 turbo mode (7Mhz)
//z80_byte tbblue_port_24df;


//Asumimos 256 registros
z80_byte tbblue_registers[256];

//Ultimo registro seleccionado
z80_byte tbblue_last_register;


void tbblue_init_memory_tables(void)
{
/*

Primer bloque de ram: memoria interna de tbblue en principio no accesible por el spectrum:

Mapeo viejo

0x000000 – 0x01FFFF (128K) => DivMMC RAM
0x020000 – 0x03FFFF (128K) => Layer2 RAM
0x040000 – 0x05FFFF (128K) => ??????????
0x060000 – 0x06FFFF (64K) => ESXDOS and Multiface RAM
0x060000 – 0x063FFF (16K) => ESXDOS ROM
0x064000 – 0x067FFF (16K) => Multiface ROM
0x068000 – 0x06BFFF (16K) => Multiface extra ROM
0x06c000 – 0x06FFFF (16K) => Multiface RAM
0x070000 – 0x07FFFF (64K) => ZX Spectrum ROM

Segundo bloque de ram: 512K, todo accesible para Spectrum. Se mapean 256 o 512 mediante bit 6 y 7 de puerto 32765
0x080000 - 0x0FFFFF (512K) => Speccy RAM

Luego 8 KB de rom de la fpga
0x100000 - 0x101FFF

Nuevo:

0x000000 – 0x00FFFF (64K) => ZX Spectrum ROM
0x010000 – 0x013FFF (16K) => ESXDOS ROM
0x014000 – 0x017FFF (16K) => Multiface ROM
0x018000 – 0x01BFFF (16K) => Multiface extra ROM
0x01c000 – 0x01FFFF (16K) => Multiface RAM
0x020000 – 0x05FFFF (256K) => divMMC RAM
0x060000 – 0x07FFFF (128K) => ZX Spectrum RAM
0x080000 – 0x0FFFFF (512K) => Extra RAM


Nuevo oct 2017:

    0x000000 – 0x00FFFF (64K) => ZX Spectrum ROM
    0x010000 – 0x013FFF (16K) => ESXDOS ROM
    0x014000 – 0x017FFF (16K) => Multiface ROM
    0x018000 – 0x01BFFF (16K) => Multiface extra ROM
    0x01c000 – 0x01FFFF (16K) => Multiface RAM
    0x020000 – 0x03FFFF (128K) => divMMC RAM
    0x040000 – 0x05FFFF (128K) => ZX Spectrum RAM			(16 paginas) 
    0x060000 – 0x07FFFF (128K) => Extra RAM				(16 paginas)

    0x080000 – 0x0FFFFF (512K) => 1st Extra IC RAM (if present)		(64 paginas)
    0x100000 – 0x17FFFF (512K) => 2nd Extra IC RAM (if present)		(64 paginas)
    0x180000 – 0xFFFFFF (512K) => 3rd Extra IC RAM (if present)		(64 paginas)

    0x200000 (2 MB)


*/





	int i,indice;

	//Los 8 KB de la fpga ROM estan al final
	tbblue_fpga_rom=&memoria_spectrum[2*1024*1024];

	//224 Paginas RAM spectrum 512k
	for (i=0;i<TBBLUE_MAX_SRAM_8KB_BLOCKS;i++) {
		indice=0x040000+8192*i;
		tbblue_ram_memory_pages[i]=&memoria_spectrum[indice];
	}

	//4 Paginas ROM
	for (i=0;i<8;i++) {
		indice=0+8192*i;
		tbblue_rom_memory_pages[i]=&memoria_spectrum[indice];
	}

}

int tbblue_get_limit_sram_page(int page)
{

	z80_byte max=tbblue_return_max_extra_blocks();

	if (page>max-1) page=max-1;

	return page;
}

void tbblue_set_ram_page(z80_byte segment)
{
	z80_byte tbblue_register=80+segment;
	z80_byte reg_value=tbblue_registers[tbblue_register];

	//tbblue_memory_paged[segment]=tbblue_ram_memory_pages[page];
	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];

	debug_paginas_memoria_mapeadas[segment]=reg_value;
}


void tbblue_set_rom_page_no_255(z80_byte segment)
{
        z80_byte tbblue_register=80+segment;
        z80_byte reg_value=tbblue_registers[tbblue_register];

	reg_value=tbblue_get_limit_sram_page(reg_value);
	tbblue_memory_paged[segment]=tbblue_ram_memory_pages[reg_value];
	debug_paginas_memoria_mapeadas[segment]=reg_value;
}

void tbblue_set_rom_page(z80_byte segment,z80_byte page)
{
	z80_byte tbblue_register=80+segment;
	z80_byte reg_value=tbblue_registers[tbblue_register];

	if (reg_value==255) {
		page=tbblue_get_limit_sram_page(page);
		tbblue_memory_paged[segment]=tbblue_rom_memory_pages[page];
		debug_paginas_memoria_mapeadas[segment]=DEBUG_PAGINA_MAP_ES_ROM+page;
	}
	else {
		tbblue_set_rom_page_no_255(segment);
	}
}


void tbblue_mem_page_ram_rom(void)
{
	z80_byte page_type;

	page_type=(puerto_8189 >>1) & 3;

	switch (page_type) {
		case 0:
			debug_printf (VERBOSE_DEBUG,"Pages 0,1,2,3");
			tbblue_registers[80]=0*2;
			tbblue_registers[81]=0*2+1;
			tbblue_registers[82]=1*2;
			tbblue_registers[83]=1*2+1;
			tbblue_registers[84]=2*2;
			tbblue_registers[85]=2*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;


			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[0];
			contend_pages_actual[1]=contend_pages_128k_p2a[1];
			contend_pages_actual[2]=contend_pages_128k_p2a[2];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];



			break;

		case 1:
			debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,7");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=5*2;
			tbblue_registers[83]=5*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=7*2;
			tbblue_registers[87]=7*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);


			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[5];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[7];





			break;

		case 2:
			debug_printf (VERBOSE_DEBUG,"Pages 4,5,6,3");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=5*2;
			tbblue_registers[83]=5*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[5];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];




			break;

		case 3:
			debug_printf (VERBOSE_DEBUG,"Pages 4,7,6,3");

			tbblue_registers[80]=4*2;
			tbblue_registers[81]=4*2+1;
			tbblue_registers[82]=7*2;
			tbblue_registers[83]=7*2+1;
			tbblue_registers[84]=6*2;
			tbblue_registers[85]=6*2+1;
			tbblue_registers[86]=3*2;
			tbblue_registers[87]=3*2+1;

			tbblue_set_ram_page(0*2);
			tbblue_set_ram_page(0*2+1);
			tbblue_set_ram_page(1*2);
			tbblue_set_ram_page(1*2+1);
			tbblue_set_ram_page(2*2);
			tbblue_set_ram_page(2*2+1);
			tbblue_set_ram_page(3*2);
			tbblue_set_ram_page(3*2+1);

			contend_pages_actual[0]=contend_pages_128k_p2a[4];
			contend_pages_actual[1]=contend_pages_128k_p2a[7];
			contend_pages_actual[2]=contend_pages_128k_p2a[6];
			contend_pages_actual[3]=contend_pages_128k_p2a[3];




			break;

	}
}

z80_byte disabled_tbblue_mem_get_ram_page(void)
{

	//printf ("Valor 32765: %d\n",puerto_32765);

	z80_byte ram_entra=puerto_32765&7;

	z80_byte bit3=0;
	z80_byte bit4=0;

	//Forzamos a que lea siempre bit 6 y 7 del puerto 32765
	//Dejamos esto asi por si en un futuro hay manera de limitar la lectura de esos bits

	int multiplicador=4; //multiplicamos 128*4

	if (multiplicador==2 || multiplicador==4) {
		bit3=puerto_32765&64;  //Bit 6
		//Lo movemos a bit 3
		bit3=bit3>>3;
	}

  if (multiplicador==4) {
      bit4=puerto_32765&128;  //Bit 7
      //Lo movemos a bit 4
      bit4=bit4>>3;
  }


	ram_entra=ram_entra|bit3|bit4;

	//printf ("ram entra: %d\n",ram_entra);

	return ram_entra;
}


void tbblue_set_mmu_128k_default(void)
{
	//rom default, paginas ram 5,2,0
	tbblue_registers[80]=255;
	tbblue_registers[81]=255;
	tbblue_registers[82]=10;
	tbblue_registers[83]=11;
	tbblue_registers[84]=4;
	tbblue_registers[85]=5;
	tbblue_registers[86]=0;
	tbblue_registers[87]=1;

	debug_paginas_memoria_mapeadas[0]=0;
	debug_paginas_memoria_mapeadas[1]=1;
	debug_paginas_memoria_mapeadas[2]=10;
	debug_paginas_memoria_mapeadas[3]=11;
	debug_paginas_memoria_mapeadas[4]=4;
	debug_paginas_memoria_mapeadas[5]=5;
	debug_paginas_memoria_mapeadas[6]=0;
	debug_paginas_memoria_mapeadas[7]=1;

}

//Indica si estamos en modo ram in rom del +2a
z80_bit tbblue_was_in_p2a_ram_in_rom={0};


void tbblue_set_memory_pages(void)
{
	//Mapeamos paginas de RAM segun config maquina
	z80_byte maquina=(tbblue_registers[3])&7;

	int romram_page;
	//int ram_page;
	int rom_page;
	int indice;

	//Por defecto
	tbblue_low_segment_writable.v=0;

	//printf ("tbblue set memory pages. maquina=%d\n",maquina);
	/*
	bits 1-0 = Machine type:
		00 = Config mode (bootrom)
		01 = ZX 48K
		10 = ZX 128K
		11 = ZX +2/+3e
	*/

	z80_byte contend_page_high_segment=tbblue_registers[86]/2;

	switch (maquina) {
		case 1:
                    //001 = ZX 48K
			tbblue_set_rom_page(0,0*2);
			tbblue_set_rom_page(1,0*2+1);
			tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);
			tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);
			tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);


		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[0];


			//tbblue_low_segment_writable.v=0;
		break;

		case 2:
                    //010 = ZX 128K
			rom_page=(puerto_32765>>4)&1;
                        tbblue_set_rom_page(0,rom_page*2);
			tbblue_set_rom_page(1,rom_page*2+1);

                        tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);

                        tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//ram_page=tbblue_mem_get_ram_page();
			//tbblue_registers[80+6]=ram_page*2;
			//tbblue_registers[80+7]=ram_page*2+1;


                        tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);



			//tbblue_low_segment_writable.v=0;
		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


		break;

		case 3:
			//011 = ZX +2/+3e
			//Si RAM en ROM
			if (puerto_8189&1) {

				tbblue_mem_page_ram_rom();
				//printf ("setting low segment writeable as port 8189 bit 1\n");
				tbblue_low_segment_writable.v=1;

				tbblue_was_in_p2a_ram_in_rom.v=1;
			}

			else {

				//printf ("NOT setting low segment writeable as port 8189 bit 1\n");

			 	//Si se cambiaba de modo ram in rom a normal
				if (tbblue_was_in_p2a_ram_in_rom.v) {
					debug_printf(VERBOSE_DEBUG,"Going from ram in rom mode to normal mode. Setting default ram pages");
					tbblue_set_mmu_128k_default();
					tbblue_was_in_p2a_ram_in_rom.v=0;
				}

                    //when "11"    => maquina <= s_speccy3e;
                        	rom_page=(puerto_32765>>4)&1;

		        	z80_byte rom1f=(puerto_8189>>1)&2;
		        	z80_byte rom7f=(puerto_32765>>4)&1;

				z80_byte rom_page=rom1f | rom7f;

				//printf ("rom: %d:\n",rom_page);


                        	tbblue_set_rom_page(0,rom_page*2);
				tbblue_set_rom_page(1,rom_page*2+1);

                        	tbblue_set_ram_page(2);
				tbblue_set_ram_page(3);

                        	tbblue_set_ram_page(4);
				tbblue_set_ram_page(5);

                        	//ram_page=tbblue_mem_get_ram_page();
				//tbblue_registers[80+6]=ram_page*2;
				//tbblue_registers[80+7]=ram_page*2+1;
                        	tbblue_set_ram_page(6);
				tbblue_set_ram_page(7);

		        	contend_pages_actual[0]=0;
		        	contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        	contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        	contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


			}
		break;

		case 4:
                    //100 = Pentagon 128K. TODO. de momento tal cual 128kb
			rom_page=(puerto_32765>>4)&1;
                        tbblue_set_rom_page(0,rom_page*2);
			tbblue_set_rom_page(1,rom_page*2+1);

                        tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);

                        tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//ram_page=tbblue_mem_get_ram_page();
			//tbblue_registers[80+6]=ram_page*2;
			//tbblue_registers[80+7]=ram_page*2+1;
                        tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);


			//tbblue_low_segment_writable.v=0;
		        contend_pages_actual[0]=0;
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[contend_page_high_segment];


		break;

		default:

			//Caso maquina 0 u otros no contemplados
			//000 = Config mode

			//printf ("tbblue_bootrom.v=%d\n",tbblue_bootrom.v);

			if (tbblue_bootrom.v==0) {
				/*
When the variable 'bootrom' takes '0', page 0 (0-16383) is mapped to the RAM 1024K,
and the page mapping is configured by bits 5-0 of the I/O port 'config1'.
These 6 bits maps 64 16K pages the start of 1024K SRAM space 0-16363 the Speccy,
which allows you access to all SRAM.

->Ampliado a 7 bits (0..128)
*/
				//romram_page=(tbblue_registers[4]&63);
				romram_page=(tbblue_registers[4]&127);
				indice=romram_page*16384;
				//printf ("page on 0-16383: %d offset: %06X\n",romram_page,indice);
				tbblue_memory_paged[0]=&memoria_spectrum[indice];
				tbblue_memory_paged[1]=&memoria_spectrum[indice+8192];
				tbblue_low_segment_writable.v=1;
				//printf ("low segment writable for machine default\n");

				debug_paginas_memoria_mapeadas[0]=romram_page;
				debug_paginas_memoria_mapeadas[1]=romram_page;
			}
			else {
				//In this setting state, the page 0 repeats the content of the ROM 'loader', ie 0-8191 appear memory contents, and repeats 8192-16383
				//La rom es de 8 kb pero la hemos cargado dos veces
				tbblue_memory_paged[0]=tbblue_fpga_rom;
				tbblue_memory_paged[1]=&tbblue_fpga_rom[8192];
				//tbblue_low_segment_writable.v=0;
				//printf ("low segment NON writable for machine default\n");
				debug_paginas_memoria_mapeadas[0]=0;
				debug_paginas_memoria_mapeadas[1]=0;
			}

			tbblue_set_ram_page(2);
			tbblue_set_ram_page(3);
			tbblue_set_ram_page(4);
			tbblue_set_ram_page(5);

			//En modo config, ram7 esta en segmento 3
			tbblue_registers[80+6]=7*2;
			tbblue_registers[80+7]=7*2+1;


			tbblue_set_ram_page(6);
			tbblue_set_ram_page(7);




		        contend_pages_actual[0]=0; //Suponemos que esa pagina no tiene contienda
		        contend_pages_actual[1]=contend_pages_128k_p2a[5];
		        contend_pages_actual[2]=contend_pages_128k_p2a[2];
		        contend_pages_actual[3]=contend_pages_128k_p2a[7];

		break;
	}

}

/*void tbblue_set_emulator_setting_multiface(void)
{
	
	//(R/W) 0x06 (06) => Peripheral 2 setting:
  //bit 3 = Enable Multiface (1 = enabled)(0 after a PoR or Hard-reset)
	

	//de momento nada
	//return;

	multiface_type=MULTIFACE_TYPE_THREE; //Vamos a suponer este tipo
	z80_byte multisetting=tbblue_registers[6]&8;

	if (multisetting) {
		//printf ("Enabling multiface\n");
		//sleep (1);
		//temp multiface_enable();
	}
	else {
		//printf ("Disabling multiface\n");
		//sleep (1);
		//temp multiface_disable();
	}
}
*/

void tbblue_set_emulator_setting_divmmc(void)
{

/*
(W)		06 => Peripheral 2 setting, only in bootrom or config mode:
			bit 7 = Enable turbo mode key (0 = disabled, 1 = enabled)
			bit 6 = DAC chip mode (0 = I2S, 1 = JAP)
			bit 5 = Enable Lightpen  (1 = enabled)
			bit 4 = Enable DivMMC (1 = enabled) -> divmmc automatic paging. divmmc memory is supported using manual
		*/
        //z80_byte diven=tbblue_config2&4;
				z80_byte diven=tbblue_registers[6]&16;
        debug_printf (VERBOSE_INFO,"Apply config.divmmc change: %s",(diven ? "enabled" : "disabled") );
        //printf ("Apply config2.divmmc change: %s\n",(diven ? "enabled" : "disabled") );

				if (diven) {
					//printf ("Activando diviface automatic paging\n");
					divmmc_diviface_enable();
					diviface_allow_automatic_paging.v=1;
				}

        //else divmmc_diviface_disable();
				else {
					//printf ("Desactivando diviface automatic paging\n");
					diviface_allow_automatic_paging.v=0;
					//Y hacer un page-out si hay alguna pagina activa
					diviface_paginacion_automatica_activa.v=0;
				}

}




void tbblue_set_emulator_setting_turbo(void)
{
	/*
	(R/W)	07 => Turbo mode
	bit 1-0 = Turbo (00 = 3.5MHz, 01 = 7MHz, 10 = 14MHz, 11 = 28Mhz)
	  (00 after a PoR or Hard-reset)

	(the 28MHz should take extra clock for each opcode SRAM read or something like that, it's
	not full 8x, just almost => not implemented in this version of ZEsarUX)
				*/

	z80_byte t=tbblue_registers[7] & 3;

	//printf ("Setting turbo: value %d on pc %04XH\n",t,reg_pc);			

	if (tbblue_deny_turbo_rom.v && reg_pc<16384) {
		//printf ("denying cpu turbo change\n");
		return;
	}

	cpu_turbo_speed = 1 << t;	//1x, 2x, 4x, 8x

	cpu_set_turbo_speed();
}

void tbblue_set_emulator_setting_reg_8(void)
{
/*
(R/W) 0x08 (08) => Peripheral 3 setting:
  bit 7 = 128K paging enable (inverse of port 0x7ffd, bit 5)
          Unlike the paging lock in port 0x7ffd,
          this may be enabled or disabled at any time.
          Use "1" to disable the locked paging.
  bit 6 = "1" to disable RAM contention. (0 after a reset)
  bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
  bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
  bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
  bit 2 = Enable Timex modes (1 = enabled)(0 after a PoR or Hard-reset)
  bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
  bit 0 = Reserved, must be 0
*/
	z80_byte value=tbblue_registers[8];

	debug_printf (VERBOSE_DEBUG,"Setting register 8 to %02XH",value);

	//bit 6 = "1" to disable RAM contention. (0 after a reset)
	if (value&64) {
		//Desactivar contention. Solo hacerlo cuando hay cambio
		if (contend_enabled.v) {
			debug_printf (VERBOSE_DEBUG,"Disabling contention");
        	contend_enabled.v=0;
	        inicializa_tabla_contend();
		}
	}

	else {
		//Activar contention. Solo hacerlo cuando hay cambio
		if (contend_enabled.v==0) {
			debug_printf (VERBOSE_DEBUG,"Enabling contention");
        	contend_enabled.v=1;
	        inicializa_tabla_contend();
		}		

	}

  	//bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
	//ay3_stereo_mode;
	//1=ACB Stereo (Canal A=Izq,Canal C=Centro,Canal B=Der)
    //2=ABC Stereo (Canal A=Izq,Canal B=Centro,Canal C=Der)	  
	if (value&32) {
		//ACB
		ay3_stereo_mode=1;
		debug_printf (VERBOSE_DEBUG,"Setting ACB stereo");
	}
	else {
		//ABC
		ay3_stereo_mode=2;
		debug_printf (VERBOSE_DEBUG,"Setting ABC stereo");
	}


  
  	//bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
	if (value&16) {
		beeper_enabled.v=1;
		debug_printf (VERBOSE_DEBUG,"Enabling beeper");
	}
	else {
		beeper_enabled.v=0;
		debug_printf (VERBOSE_DEBUG,"Disabling beeper");
	}

  	//bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
	if (value&8) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
		debug_printf (VERBOSE_DEBUG,"Enabling audiodac Specdrum");
	}
	else {
		audiodac_enabled.v=0;
		debug_printf (VERBOSE_DEBUG,"Disabling audiodac Specdrum");
	}


	//bit 2 = Enable port $FF Timex video mode *read* (disables floating bus on 0xff) (hard reset = 0)
	// nothing to do here, affects idle_bus_port_atribute() in operaciones.c
  	
	//bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
	if (value &2) set_total_ay_chips(3);
	else set_total_ay_chips(1);

	//bit 0 = Implement Issue 2 keyboard (port $FE reads as early ZX boards) (hard reset = 0)
	keyboard_issue2.v = value&1;

}

void tbblue_reset_common(void)
{
	tbblue_registers[6] |= 128 + 32;	//soft reset: set Enable "F8" CPU speed key, and F3 50/60Hz key

	//TODO how to set machine back to 3.5MHz without doing too much extras?
// 	tbblue_registers[7]=0;
// 	cpu_turbo_speed = 1;	//1x, 2x, 4x, 8x
// 	cpu_set_turbo_speed();	- this seems to do way too much, not a good fit for "soft reset"

	tbblue_registers[8]&=255-64;	// clear "disable RAM and port contention"
	tbblue_registers[9]&=255-16;	// clear "sprite ID lockstep"

	tbblue_registers[18]=8;
	tbblue_registers[19]=11;

	tbblue_registers[20]=TBBLUE_DEFAULT_TRANSPARENT;

	tbblue_registers[21]=0;
	tbblue_registers[22]=0;
	tbblue_registers[23]=0;

	tbblue_registers[28]=0;

	tbblue_registers[30]=0;
	tbblue_registers[31]=0;
	tbblue_registers[34]=0;
	tbblue_registers[35]=0;
	tbblue_registers[38]=0;
	tbblue_registers[39]=0;
	tbblue_registers[44]=0x80;
	tbblue_registers[45]=0x80;
	tbblue_registers[46]=0x80;
	tbblue_registers[47]=0;
	tbblue_registers[48]=0;
	tbblue_registers[49]=0;
	tbblue_registers[50]=0;
	tbblue_registers[51]=0;
	tbblue_registers[64]=0;
	tbblue_registers[66]=7;
	tbblue_registers[67]=0;
	tbblue_registers[74]=TBBLUE_DEFAULT_TRANSPARENT;
	tbblue_registers[75]=TBBLUE_DEFAULT_TRANSPARENT;

/*
(R/W) 0x4C (76) => Transparency index for the tilemap
  bits 7-4 = Reserved, must be 0
  bits 3-0 = Set the index value (0xF after reset)
	*/
	tbblue_registers[76]=0xF;


	tbblue_registers[97]=0;
	tbblue_registers[98]=0;

	tbblue_registers[104]=0;
	tbblue_registers[105]=0;
	tbblue_registers[106]=0;
	tbblue_registers[107]=0;
	tbblue_registers[108]=0;
	tbblue_registers[110]=0;
	tbblue_registers[111]=0;
	tbblue_registers[112]=0;
	tbblue_registers[113]=0;
	tbblue_registers[127]=255;

	tbblue_registers[128]=(tbblue_registers[128]&0x0F) | (tbblue_registers[128]<<4);
	tbblue_registers[130]=255;
	tbblue_registers[131]=255;
	tbblue_registers[132]=255;
	tbblue_registers[133]=255;
	tbblue_registers[144]=0;
	tbblue_registers[145]=0;
	tbblue_registers[146]=0;
	tbblue_registers[147]=0;
	tbblue_registers[152]=0xFF;
	tbblue_registers[153]=0x01;
	tbblue_registers[154]=0x00;
	tbblue_registers[155]=0x00;
	tbblue_registers[160]=0;
	tbblue_registers[162]=0;
	tbblue_registers[163]=11;

	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_ULA][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][1]=255;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]=191;

	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]=159;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2]=0;
	clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]=255;



	tbblue_copper_pc=0;
	
	tbblue_set_mmu_128k_default();

	tbblue_was_in_p2a_ram_in_rom.v=0;


}

void tbblue_reset(void)
{

	//Los bits reservados los metemos a 0 también

	/*
	(R/W) 02 => Reset:
  bits 7-3 = Reserved, must be 0
  bit 2 = (R) Power-on reset (PoR)
  bit 1 = (R/W) Reading 1 indicates a Hard-reset. If written 1 causes a Hard Reset.
  bit 0 = (R/W) Reading 1 indicates a Soft-reset. If written 1 causes a Soft Reset.
	*/
	tbblue_registers[2]=1;
	

	tbblue_reset_common();



}

void tbblue_hard_reset(void)
{

	/*
	(R/W) 02 => Reset:
	  bits 7-3 = Reserved, must be 0
	  bit 2 = (R) Power-on reset (PoR)
	  bit 1 = (R/W) Reading 1 indicates a Hard-reset. If written 1 causes a Hard Reset.
	  bit 0 = (R/W) Reading 1 indicates a Soft-reset. If written 1 causes a Soft Reset.
	*/

	//Aqui no estoy distinguiendo entre hard reset y power-on reset, dado que al iniciar maquina siempre llama a hard reset
	tbblue_registers[2]=4+2;


	tbblue_registers[3]=0;
	tbblue_registers[4]=0;
	tbblue_registers[5]=1;
	tbblue_registers[6]=0;
	tbblue_registers[7]=0;
	tbblue_registers[8]=16;
	tbblue_registers[9]=0;
	tbblue_registers[128]=0;
	tbblue_registers[129]=0;
	tbblue_registers[134]=255;
	tbblue_registers[135]=255;
	tbblue_registers[136]=255;
	tbblue_registers[137]=255;
	tbblue_registers[138]=1;
	tbblue_registers[140]=0;

	tbblue_reset_common();


	tbblue_reset_palette_write_state();

	tbblue_port_123b=0;
	tbblue_port_123b_b4=0;


	if (tbblue_fast_boot_mode.v) {
		tbblue_registers[3]=3;

		tbblue_registers[8]=2+8; //turbosound 3 chips, specdrum

		set_total_ay_chips(3);

		audiodac_enabled.v=1;
		audiodac_selected_type=0;


		tbblue_registers[80]=0xff;
		tbblue_registers[81]=0xff;
		tbblue_set_memory_pages();

		if (tbblue_initial_123b_port>=0) tbblue_port_123b=tbblue_initial_123b_port;
	}

	else {
		tbblue_bootrom.v=1;
	//printf ("----setting bootrom to 1\n");
		tbblue_set_memory_pages();
		tbblue_set_emulator_setting_divmmc();
	}



	tbblue_reset_sprites();
	tbblue_reset_palettes();


}





/*
z80_byte old_tbblue_read_port_24d5(void)
{

//- port 0x24D5: 1st read = return hardware number (below), 2nd read = return firmware version number (first nibble.second nibble) e.g. 0x12 = 1.02
//
//hardware numbers
//1 = DE-1 (old)
//2 = DE-2 (old)
//3 = DE-2 (new)
//4 = DE-1 (new)
//5 = FBLabs
//6 = VTrucco
//7 = WXEDA hardware (placa de desarrollo)
//8 = Emulators
//9 = ZX Spectrum Next


/

}

*/

void tbblue_set_timing_128k(void)
{
        contend_read=contend_read_128k;
        contend_read_no_mreq=contend_read_no_mreq_128k;
        contend_write_no_mreq=contend_write_no_mreq_128k;

        ula_contend_port_early=ula_contend_port_early_128k;
        ula_contend_port_late=ula_contend_port_late_128k;


        screen_testados_linea=228;
        screen_invisible_borde_superior=7;
        screen_invisible_borde_derecho=104;

        port_from_ula=port_from_ula_p2a;
        contend_pages_128k_p2a=contend_pages_p2a;

}


void tbblue_set_timing_48k(void)
{
        contend_read=contend_read_48k;
        contend_read_no_mreq=contend_read_no_mreq_48k;
        contend_write_no_mreq=contend_write_no_mreq_48k;

        ula_contend_port_early=ula_contend_port_early_48k;
        ula_contend_port_late=ula_contend_port_late_48k;

        screen_testados_linea=224;
        screen_invisible_borde_superior=8;
        screen_invisible_borde_derecho=96;

        port_from_ula=port_from_ula_48k;

        //esto no se usara...
        contend_pages_128k_p2a=contend_pages_128k;

}

void tbblue_change_timing(int timing)
{
        if (timing==0) tbblue_set_timing_48k();
        else if (timing==1) tbblue_set_timing_128k();

        screen_set_video_params_indices();
        inicializa_tabla_contend();

}

/*
*/
void tbblue_set_emulator_setting_timing(void)
{
	/*
	(W) 0x03 (03) => Set machine type, only in IPL or config mode:
	A write in this register disables the IPL
	(0x0000-0x3FFF are mapped to the RAM instead of the internal ROM)
	bit 7 = lock timing

	bits 6-4 = Timing:
	000 or 001 = ZX 48K
	010 = ZX 128K
	011 = ZX +2/+3e
	100 = Pentagon 128K

	bit 3 = Reserved, must be 0

	bits 2-0 = Machine type:
	000 = Config mode
	001 = ZX 48K
	010 = ZX 128K
	011 = ZX +2/+3e
	100 = Pentagon 128K
	*/


                //z80_byte t=(tbblue_config1 >> 6)&3;
		z80_byte t=(tbblue_registers[3]>>4)&7;

		//TODO: otros timings

                if (t<=1) {
		//48k
				debug_printf (VERBOSE_INFO,"Apply config.timing. change:48k");
				tbblue_change_timing(0);
		}
		else {
		//128k
				debug_printf (VERBOSE_INFO,"Apply config.timing. change:128k");
				tbblue_change_timing(1);
		}




}


void tbblue_set_register_port(z80_byte value)
{
	tbblue_last_register=value;
}

z80_byte tbblue_get_register_port(void)
{
	return tbblue_last_register;
}

int tbblue_next_palette_format_as_mask_only(void) {
/*
	checks if the ink-mask in 0x42 is full-ink or invalid value
	-> if yes, then it should be applied as bit-mask, and paper+border is "fallback" 0x4A
	-> if no, then unset bits in mask are used to calculate paper color, starting at 128 in pal.

	0x42 (66) => ULANext Attribute Byte Format
	(R/W)
	bits 7:0 = Mask indicating which bits of an attribute byte are used to represent INK.
		Other bits represent PAPER. (soft reset = 0x07)
		The mask can only indicate a solid sequence of bits on the right side of an attribute
		byte (1, 3, 7, 15, 31, 63, 127 or 255).
		INKs are mapped to base index 0 in the palette and PAPERs and border are
		mapped to base index 128 in the palette.
		The 255 value enables the full ink colour mode making all the palette entries INK.
		In this case PAPER and border are both taken from the fallback colour in nextreg 0x4A.
		If the mask is not one of those listed above, the INK is taken as the logical AND of
		the mask with the attribute byte and the PAPER and border colour are again both taken
		from the fallback colour in nextreg 0x4A.
 */
	z80_byte mask = tbblue_registers[0x42];
	// 0 is invalid -> mask-mode any way, forcing ink = 0 color only
	// 255 is "full-ink" -> mask-mode too (for practical reasons, making background pick 0x4A reg)
	if (0 == mask || 255 == mask) return 1;

	// return zero when mask is "power of two minus one" value, the valid format
	// return non-zero when invalid (mask-mode)
	return ((mask + 1) & mask);
}

void tbblue_get_string_palette_format(char *texto)
{
//if (value&128) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
	/*
	(R/W) 0x43 (67) => Palette Control
	bit 0 = Disable the standard Spectrum flash feature to enable the extra colours.
  (Reset to 0 after a reset)

	(R/W) 0x42 (66) => Palette Format
  bits 7-0 = Number of the last ink colour entry on palette. (Reset to 15 after a Reset)
  This number can be 1, 3, 7, 15, 31, 63, 127 or 255.
	

	*/


	if ((tbblue_registers[67]&1)==0) strcpy (texto,"Normal Color palette");
	else {

		z80_byte palformat=tbblue_registers[66];

		/*
		Ejemplo: mascara 3:   00000011
		Son 4 tintas
		64 papeles

		Para pasar de tintas a papeles :    00000011 -> inverso -> 11111100
		Dividimos 11111100 entre tintas, para rotar el valor 2 veces a la derecha = 252 / 4 = 63   -> +1 -> 64
		*/

		int tintas=palformat+1;

		if (0 == palformat || 0 != (tintas & palformat)) {	//checks if "tintas" is power of two

			// invalid ink format, the HW will use it as bit-mask for ink color, and fallback 0x4A for paper
			sprintf(texto,"Extra colors %02XH mask (invalid)",palformat);

		} else {

			// tintas is power of two -> 256/tintas is then papeles (8*32 = 256, 2*128=256, etc)
			int papeles=256/tintas;
			sprintf (texto,"Extra colors %d inks %d papers",tintas,papeles);

		}

	}

}


void tbblue_splash_palette_format(void)
{
	char mensaje[200];
	char videomode[100];

	tbblue_get_string_palette_format(videomode);

	sprintf (mensaje,"Setting %s",videomode);

	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);

}

	
//tbblue_last_register
//void tbblue_set_value_port(z80_byte value)
void tbblue_set_value_port_position(const z80_byte index_position,z80_byte value)
{

	//Nota: algunos registros como el 0 y el 1, que son read only, deja escribirlos en el array,
	//pero luego cuando se van a leer, mediante la funcion tbblue_get_value_port_register, se obtienen de otro sitio, no del array
	//por lo que todo ok



	//printf ("register port %02XH value %02XH\n",tbblue_last_register,value);

	z80_byte last_register_6=tbblue_registers[6];
	z80_byte last_register_7=tbblue_registers[7];
	z80_byte last_register_8=tbblue_registers[8];
	z80_byte last_register_21=tbblue_registers[21];
	z80_byte last_register_66=tbblue_registers[66];
	z80_byte last_register_67=tbblue_registers[67];
	z80_byte last_register_99=tbblue_registers[99];

	switch(index_position) {

		case 3:
		{
			//Controlar caso especial
			//(W) 0x03 (03) => Set machine type, only in IPL or config mode
			//   		bits 2-0 = Machine type:
			//      		000 = Config mode
			z80_byte machine_type=tbblue_registers[3]&7;

			if (!(machine_type==0 || tbblue_bootrom.v)) {
				debug_printf(VERBOSE_DEBUG,"Can not change machine type (to %02XH) while in non config mode or non IPL mode",value);
				return;
			}
		}
		break;

		case 28:
			/*
			(W) 0x1C (28) => Clip Window control
				bits 7-4 = Reserved, must be 0
				bit 3 - reset the Tilemap clip index.
				bit 2 - reset the ULA/LoRes clip index.
				bit 1 - reset the sprite clip index.
				bit 0 - reset the Layer 2 clip index.
			*/
			if (value&1) tbblue_reset_clip_window_layer2_index();
			if (value&2) tbblue_reset_clip_window_sprites_index();
			if (value&4) tbblue_reset_clip_window_ula_index();
			if (value&8) tbblue_reset_clip_window_tilemap_index();
			return;

		case 40:
		/*
		0x28 (40) => PS/2 Keymap Address MSB
		(R) bits 7:0 = Stored palette value from nextreg 0x44
		(W)
			bits 7:1 = Reserved, must be 0
			bit 0 = MSB address
		*/
		// no functionality in ZEsarUX when written to, but keep value stored by 0x44 palette reg.
		// = return before that color value is overwritten
		return;

		case 98:
/*
(W) 0x62 (98) => Copper control HI bit
   bits 7-6 = Start control
       00 = STOP Copper (CPC is kept at current value)
       01 = Reset CPC to 0 and START Copper
       10 = START Copper (does resume at current CPC)
       11 = Reset CPC to 0 and START Copper, reset CPC at each video frame at (0,0) (top left of PAPER)
   bits 2-0 = Copper list index address MSB

   When "Control mode" bits are identical with previously set ones, they are ignored - allowing for
   index change without restarting currently running Copper program.
*/
			tbblue_copper_write_control_hi_byte(value, tbblue_registers[98]);
		break;

	}

	/////////////////////////////////////////////////
	// write new value into register
	tbblue_registers[index_position]=value;


	switch(index_position)
	{

		case 2:
		/*
		(R/W)	02 => Reset:
					bits 7-3 = Reserved, must be 0
					bit 2 = (R) Power-on reset
					bit 1 = (R/W) if 1 Hard Reset
					bit 0 = (R/W) if 1 Soft Reset
					*/

						//tbblue_hardsoftreset=value;
						if (value&1) {
							//printf ("Doing soft reset due to writing to port 24D9H\n");
							reg_pc=0;
						}
						if (value&2) {
							//printf ("Doing hard reset due to writing to port 24D9H\n");
							tbblue_bootrom.v=1;
							//printf ("----setting bootrom to 1. when writing register 2 and bit 1\n");
							tbblue_registers[3]=0;
							//tbblue_config1=0;
							tbblue_set_memory_pages();
							reg_pc=0;
						}

					break;


		break;

		case 3:
		/*
		(W) 0x03 (03) => Set machine type, only in IPL or config mode:
   		A write in this register disables the IPL
   		(0x0000-0x3FFF are mapped to the RAM instead of the internal ROM)
   		bit 7 = lock timing
   		bits 6-4 = Timing:
      		000 or 001 = ZX 48K
      		010 = ZX 128K
      		011 = ZX +2/+3e
      		100 = Pentagon 128K
   		bit 3 = Reserved, must be 0
   		bits 2-0 = Machine type:
      		000 = Config mode
      		001 = ZX 48K
      		010 = ZX 128K
      		011 = ZX +2/+3e
      		100 = Pentagon 128K
      		*/

		/*  OLD:
				(W)		03 => Set machine type, only in bootrom or config mode:
							A write in this register disables the bootrom mode (0000 to 3FFF are mapped to the RAM instead of the internal ROM)
							bits 7-5 = Reserved, must be 0
							bits 4-3 = Timing:
								00,
								01 = ZX 48K
								10 = ZX 128K
								11 = ZX +2/+3e
							bit 2 = Reserved, must be 0
							bits 1-0 = Machine type:
								00 = Config mode (bootrom)
								01 = ZX 48K
								10 = ZX 128K
								11 = ZX +2/+3e
								*/
			//Pentagon not supported yet. TODO
			//last_value=tbblue_config1;
			tbblue_bootrom.v=0;
			//printf ("----setting bootrom to 0\n");

			//printf ("Writing register 3 value %02XH\n",value);

			tbblue_set_memory_pages();


			//Solo cuando hay cambio
			//if ( last_register_3 != value )
			tbblue_set_emulator_setting_timing();
		break;


		case 4:

/*
		(W)		04 => Set page RAM, only in config mode (no bootrom):
					bits 7-5 = Reserved, must be 0
					bits 4-0 = RAM page mapped in 0000-3FFF (32 pages of 16K = 512K)
			*/

			tbblue_set_memory_pages();

		break;

		

		case 6:

			//Bit 7 no me afecta, solo afecta a cambios por teclado en maquina real
			//bit 7 = Enable turbo mode (0 = disabled, 1 = enabled)(0 after a PoR or Hard-reset)

			//Si hay cambio en DivMMC
			/*
			(W)		06 => Peripheral 2 setting, only in bootrom or config mode:

						bit 4 = Enable DivMMC (1 = enabled)
						bit 3 = Enable Multiface (1 = enabled)(0 after a PoR or Hard-reset)
					*/
			if ( (last_register_6&16) != (value&16)) tbblue_set_emulator_setting_divmmc();
			//if ( (last_register_6&8) != (value&8)) tbblue_set_emulator_setting_multiface();
			zxndma.emulate_Zilog.v = 0 != (value&0x40);
			//zxndma.emulate_UA858D.v = zxndma.emulate_Zilog.v;	// FIXME DEBUG test
		break;


		case 7:
		/*
		(R/W)	07 => Turbo mode
					*/
					if ( last_register_7 != value ) tbblue_set_emulator_setting_turbo();
		break;

		case 8:
/*
(R/W) 0x08 (08) => Peripheral 3 setting:
  bit 7 = 128K paging enable (inverse of port 0x7ffd, bit 5) 
          Unlike the paging lock in port 0x7ffd, 
          this may be enabled or disabled at any time.
          Use "1" to disable the locked paging.
  bit 6 = "1" to disable RAM contention. (0 after a reset) 
  bit 5 = Stereo mode (0 = ABC, 1 = ACB)(0 after a PoR or Hard-reset)
  bit 4 = Enable internal speaker (1 = enabled)(1 after a PoR or Hard-reset)
  bit 3 = Enable Specdrum/Covox (1 = enabled)(0 after a PoR or Hard-reset)
  bit 2 = Enable Timex modes (1 = enabled)(0 after a PoR or Hard-reset)
  bit 1 = Enable TurboSound (1 = enabled)(0 after a PoR or Hard-reset)
  bit 0 = Reserved, must be 0
*/

			if ( last_register_8 != value ) tbblue_set_emulator_setting_reg_8();

		break;

		case 21:
			//modo lores
			if ( (last_register_21&128) != (value&128)) {
				if (value&128) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Enabling lores video mode. 128x96 256 colours");
				else screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Disabling lores video mode");
			}
		break;





		case 24:
			//(W) 0x18 (24) => Clip Window Layer 2
			clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][tbblue_get_clip_window_layer2_index()]=value;
            tbblue_inc_clip_window_layer2_index();

			//debug
			//printf ("layer2 %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]);
		break;



		case 25:
			//((W) 0x19 (25) => Clip Window Sprites
			clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][tbblue_get_clip_window_sprites_index()]=value;
            tbblue_inc_clip_window_sprites_index();

			//debug
			//printf ("sprites %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]);
		break;



		case 26:
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			clip_windows[TBBLUE_CLIP_WINDOW_ULA][tbblue_get_clip_window_ula_index()]=value;
            tbblue_inc_clip_window_ula_index();

			//debug
			//printf ("ula %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_ULA][0],clip_windows[TBBLUE_CLIP_WINDOW_ULA][1],clip_windows[TBBLUE_CLIP_WINDOW_ULA][2],clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]);
		break;



		case 27:
			//(W) 0x1B (27) => Clip Window Tilemap
			clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][tbblue_get_clip_window_tilemap_index()]=value;
            tbblue_inc_clip_window_tilemap_index();

			//debug
			//printf ("tilemap %d %d %d %d\n",clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]);
		break;



/*
(W) 0x2D (45) => SoundDrive (SpecDrum) port 0xDF mirror
 bits 7-0 = Data to be written at Soundrive
 this port can be used to send data to the SoundDrive using the Copper co-processor
*/

		case 45:

		if (audiodac_enabled.v) audiodac_send_sample_value(value);

		break;


		case 52:	//0x34 - sprite index
			if (tbsprite_is_lockstep()) {
				tbblue_out_port_sprite_index(value);
			} else {
				tbsprite_nr_index_sprite=value%TBBLUE_MAX_SPRITES;
			}
		break;

		// sprite attribute registers
		case 53:	case 54:	case 55:	case 56:	case 57:	//0x35, 0x36, 0x37, 0x38, 0x39
		case 117:	case 118:	case 119:	case 120:	case 121:	//0x75, 0x76, 0x77, 0x78, 0x79
		{
			int attribute_id = (index_position-0x35)&7;				//0..4
			int sprite_id = tbsprite_is_lockstep() ? tbsprite_index_sprite : tbsprite_nr_index_sprite;
			tbsprite_sprites[sprite_id][attribute_id] = value;
			if (index_position < 0x70) break;	//0x35, 0x36, 0x37, 0x38, 0x39 = done
			//0x75, 0x76, 0x77, 0x78, 0x79 = increment sprite id
			if (tbsprite_is_lockstep()) {
				tbsprite_increment_index_303b();
			} else {
				++tbsprite_nr_index_sprite;
				tbsprite_nr_index_sprite%=TBBLUE_MAX_SPRITES;
			}
		}
		break;


		case 64:
			//palette index
			tbblue_reset_palette_write_state();
		break;


		//(R/W) 0x41 (65) => Palette Value
		case 65:
			tbblue_write_palette_value_high8(value);
			tbblue_increment_palette_index();
		break;

		case 66:
			if ( (last_register_66 != value) && tbblue_registers[67]&1) tbblue_splash_palette_format();
		break;

		case 67:
			if ( (last_register_67&1) != (value&1) ) tbblue_splash_palette_format();
		break;

		
		case 68:
			// values are set in the palette after full 16b were sent to this register, not partially
			if (tbblue_write_palette_state == 0) {
				tbblue_registers[40] = value;	// NextReg 0x28 (40) should read as "first-byte" of 0x44
				tbblue_write_palette_state++;
			} else {
				tbblue_write_palette_value(tbblue_registers[40], value);
			}
		break;


		//MMU
		case 80:
		case 81:
			tbblue_set_memory_pages();
		break;

		case 82:
		case 83:
		case 84:
		case 85:
		case 86:
		case 87:
			tbblue_set_memory_pages();
		break;

		case 96:
/*
(W) 0x60 (96) => Copper data
  bits 7-0 = Byte to write at "Copper list"
  Note that each copper instruction is composed by two bytes (16 bits).
*/

		//printf ("0x60 (96) => Copper data value %02XH\n",value);

		tbblue_copper_write_data(value);

		break;

		case 97:
/*
(W) 0x61 (97) => Copper control LO bit
  bits 7-0 = Copper list index address LSB.
  After the write, the index is auto-incremented to the next memory position.
  (Index is set to 0 after a reset)
*/

		//printf ("0x61 (97) => Copper control LO bit value %02XH\n",value);

		break;


		case 99:
/*
(W) 0x63 (99) => Copper Data 16-bit Write

Similar to Copper Data ($60), allows to upload Copper instructions to the copper memory, 
but the difference is that writes are committed to copper memory in 16-bit words 
(only half-written instructions by using NextReg $60 may get executed, $63 prevents that).

The first write to this register is MSB of Copper Instruction destined for even copper instruction address.
The second write to this register is LSB of Copper Instruction destined for odd copper instruction address.
After any write, the copper address is auto-incremented to the next memory position.
After a write to an odd address, the entire 16-bits are written to Copper memory at once.

*/

		tbblue_copper_write_data_16b(last_register_99,value);

		break;		

		case 105:
/*
(R/W) 0x69 (105) => Display Control 1
  bit 7 = Enable layer 2 (alias port 0x123B bit 1)
  bit 6 = Enable ULA shadow display (alias port 0x7FFD bit 3)
  bits 5:0 = Port 0xFF bits 5:0 alias (Timex display modes)
*/
			// port 0x123B bit 1 mirror (Layer 2 - visible/invisible)
			if (value&0x80) {
				tbblue_port_123b |= 2;	// set bit 1
			} else {
				tbblue_port_123b &= ~2;	// clear bit 1
			}
			// port 7FFD bit 3 mirror (ULA shadow Bank 7)
			if (value&0x40) {
				puerto_32765 |= 8;		// set bit 3
			} else {
				puerto_32765 &= ~8;		// clear bit 3
			}
			// timex port $FF mirror, bits 5:0
			value &= 0x3F;				// keep only bits 5:0 from new value
			value |= timex_port_ff & 0xC0;	// keep current port 255 bits 7:6
			set_timex_port_ff(value);
		break;

	}


}


//tbblue_last_register
void tbblue_set_value_port(z80_byte value)
{
	tbblue_set_value_port_position(tbblue_last_register,value);
}

int tbblue_get_raster_line(void)
{
	/*
	Line 0 is first video line. In truth the line is the Y counter, Video is from 0 to 191, borders and hsync is >192
Same this page: http://www.zxdesign.info/vertcontrol.shtml


Row	Start	Row End	Length	Description
0		191	192	Video Display
192	247	56	Bottom Border
248	255	8	Vertical Sync
256	312	56	Top Border

*/
	if (t_scanline>=screen_indice_inicio_pant) return t_scanline-screen_indice_inicio_pant;
	else return t_scanline+192+screen_total_borde_inferior;


}


z80_byte tbblue_get_value_port_register(z80_byte registro)
{

	int linea_raster;

	//Casos especiales. Registros que no se obtienen leyendo del array de registros. En principio todos estos están marcados
	//como read-only en la documentacion de tbblue
	/*
	(R) 0x00 (00) => Machine ID

	(R) 0x01 (01) => Version (Nibble most significant = Major, Nibble less significant = Minor)
	*/

	

	switch(registro)
	{
		case 0:

/*
hardware numbers
#define HWID_DE1A               1               DE-1 
#define HWID_DE2A               2               DE-2  
#define HWID_DE2N               3               DE-2 (new) 
#define HWID_DE1N               4               DE-1 (new) 
#define HWID_FBLABS             5               FBLabs 
#define HWID_VTRUCCO   				 	6               VTrucco 
#define HWID_WXEDA              7               WXEDA 
#define HWID_EMULATORS  				8               Emulators 
#define HWID_ZXNEXT             10              ZX Spectrum Next 
#define HWID_MC                 11              Multicore 
#define HWID_ZXNEXT_AB  				250             ZX Spectrum Next Anti-brick 
*/


			return tbblue_machine_id; //8;
		break;


/*
(R) 0x01 (01) => Core Version 
  bits 7-4 = Major version number
  bits 3-0 = Minor version number
  (see register 0x0E for sub minor version number)


#define TBBLUE_CORE_VERSION_MAJOR     1 
#define TBBLUE_CORE_VERSION_MINOR     10
#define TBBLUE_CORE_VERSION_SUBMINOR  31

  */
		case 1:
			return (TBBLUE_CORE_VERSION_MAJOR<<4 | TBBLUE_CORE_VERSION_MINOR);
		break;

		case 7:
		{
			z80_byte programmed_speed = tbblue_registers[registro] & 3;
			return programmed_speed | (programmed_speed<<4);	// return programmed also as current
		}
		break;

		case 0xE:
			return TBBLUE_CORE_VERSION_SUBMINOR;
		break;		

		case 24:
			//(W) 0x18 (24) => Clip Window Layer 2
            return clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][tbblue_get_clip_window_layer2_index()];

		case 25:
			//((W) 0x19 (25) => Clip Window Sprites
            return clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][tbblue_get_clip_window_sprites_index()];

		case 26:
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			return clip_windows[TBBLUE_CLIP_WINDOW_ULA][tbblue_get_clip_window_ula_index()];

		case 27:
			//(W) 0x1B (27) => Clip Window Tilemap
			return clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][tbblue_get_clip_window_tilemap_index()];

		/*
		(R) 0x1E (30) => Active video line (MSB)
  bits 7-1 = Reserved, always 0
  bit 0 = Active line MSB (Reset to 0 after a reset)

(R) 0x1F (31) = Active video line (LSB)
  bits 7-0 = Active line LSB (0-255)(Reset to 0 after a reset)
		*/

		case 30:
			linea_raster=tbblue_get_raster_line();
			linea_raster=linea_raster >> 8;
			return (linea_raster&1);
		break;

		case 31:
			linea_raster=tbblue_get_raster_line();
			return (linea_raster&0xFF);
		break;

		case 52:	//0x34 - sprite index
			if (tbsprite_is_lockstep()) {
				return tbsprite_index_sprite;
			} else {
				return tbsprite_nr_index_sprite;
			}
		break;

		case 105:
/*
(R/W) 0x69 (105) => Display Control 1
  bit 7 = Enable layer 2 (alias port 0x123B bit 1)
  bit 6 = Enable ULA shadow display (alias port 0x7FFD bit 3)
  bits 5:0 = Port 0xFF bits 5:0 alias (Timex display modes)
*/
			// timex port $FF mirror, bits 5:0
			// and clear bits 7:6 of temporary return value
			tbblue_registers[105] = timex_port_ff&0x3F;
			// port 0x123B bit 1 mirror (Layer2 visibility)
			if (tbblue_is_active_layer2()) tbblue_registers[105] |= 0x80;
			// port 7FFD bit 3 mirror (ULA shadow Bank 7)
			if (puerto_32765 & 8) tbblue_registers[105] |= 0x40;
			return tbblue_registers[105];

		// (R/W) 0x6E (110) => Tilemap Base Address
		// (R/W) 0x6F (111) => Tile Definitions Base Address
		// * BOTH registers have "bits 7-6 = Read back as zero, write values ignored"
		case 110:
		case 111:
			return tbblue_registers[registro]&0x3F;



	}


	return tbblue_registers[registro];
}



z80_byte tbblue_get_value_port(void)
{
	return tbblue_get_value_port_register(tbblue_last_register);
}



//Devuelve puntero a direccion de memoria donde esta el scanline en modo lores para direccion y
z80_byte *get_lores_pointer(int y)
{
	z80_byte *base_pointer;

	//Siempre saldra de ram 5
	base_pointer=tbblue_ram_memory_pages[5*2];	

	//128x96 one byte per pixel in left to right, top to bottom order so that the 
	//top half of the screen is in the first timex display file at 0x4000 
	//and the bottom half is in the second timex display file at 0x6000
	
	z80_int offset=0;

	//int yorig=y;

	const int mitad_alto=96/2;

	//Segunda mitad
	if (y>=mitad_alto) {
		//printf ("segundo bloque. y=%d offset=%d\n",y,offset);
		offset +=0x2000;
		y=y-mitad_alto;
	}

	//Sumamos desplazamiento por y
	offset +=y*128;

	//printf ("y: %d offset: %d\n",yorig,offset);

	base_pointer +=offset;

	return base_pointer;
}


struct s_tbblue_priorities_names {
	char layers[3][20];
};


struct s_tbblue_priorities_names tbblue_priorities_names[8]={
	{ { "Sprites" ,  "Layer 2"  ,  "ULA&Tiles" } },
	{ { "Layer 2" ,  "Sprites"  ,  "ULA&Tiles" } },
	{ { "Sprites" ,  "ULA&Tiles"  ,  "Layer 2" } },
	{ { "Layer 2" ,  "ULA&Tiles"  ,  "Sprites" } },
	{ { "ULA&Tiles" ,  "Sprites"  ,  "Layer 2" } },
	{ { "ULA&Tiles" ,  "Layer 2"  ,  "Sprites" } },
	{ { "Sprites" ,  "ULA+L2"   ,  "-" } },
	{ { "Sprites" ,  "ULA+L2-5" ,  "-" } },
};

//Retorna el texto de la capa que corresponde segun el byte de prioridad y la capa demandada en layer
//La capa de arriba del todo, es capa 0. La de en medio, la 1, etc
char *tbblue_get_string_layer_prio(int layer,z80_byte prio)
{
/*
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S
     110 - S(U+L) ULA and Layer 2 combined, colours clamped to 7
     111 - S(U+L-5) ULA and Layer 2 combined, colours clamped to [0,7]
*/

	//por si acaso. capa entre 0 y 7
	prio = prio & 7;

	//layer entre 0 y 2
	layer = layer % 3;

	return tbblue_priorities_names[prio].layers[layer];

}



//Inicializa punteros a los 3 layers
z80_int *p_layer_first;
z80_int *p_layer_second;
z80_int *p_layer_third;



//+int tbblue_si_sprite_transp_ficticio(z80_int color)

//z80_byte (*peek_byte_no_time)(z80_int dir);

z80_byte tbblue_get_layers_priorities(void)
{
	return (tbblue_registers[0x15] >> 2)&7;
}

void tbblue_set_layer_priorities(void)
{
	//Por defecto
	//sprites over the Layer 2, over the ULA graphics
	p_layer_first=tbblue_layer_sprites;
	p_layer_second=tbblue_layer_layer2;
	p_layer_third=tbblue_layer_ula;


	/*
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  bits 4-2 = set layers priorities:
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
     011 - L U S
     100 - U S L
     101 - U L S
     110 - S(U+L) ULA and Layer 2 combined, colours clamped to 7
     111 - S(U+L-5) ULA and Layer 2 combined, colours clamped to [0,7]
  bit 1 = Over border (1 = yes)(Back to 0 after a reset)
  bit 0 = Sprites visible (1 = visible)(Back to 0 after a reset)
  */
	z80_byte prio=tbblue_get_layers_priorities();

	//printf ("prio: %d\n",prio);

	switch (prio) {
		case 0:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_ula;

		break;

		case 1:
			p_layer_first=tbblue_layer_layer2;
			p_layer_second=tbblue_layer_sprites;
			p_layer_third=tbblue_layer_ula;

		break;


		case 2:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_ula;
			p_layer_third=tbblue_layer_layer2;

		break;

		case 3:
			p_layer_first=tbblue_layer_layer2;
			p_layer_second=tbblue_layer_ula;
			p_layer_third=tbblue_layer_sprites;

		break;

		case 4:
			p_layer_first=tbblue_layer_ula;
			p_layer_second=tbblue_layer_sprites;
			p_layer_third=tbblue_layer_layer2;

		break;

		case 5:
			p_layer_first=tbblue_layer_ula;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_sprites;

		break;

		default:
			p_layer_first=tbblue_layer_sprites;
			p_layer_second=tbblue_layer_layer2;
			p_layer_third=tbblue_layer_ula;

		break;	
	}

}

void tbblue_set_layer_priorities_border_only(void)
{
	/*
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  bits 6-5 = Reserved, must be 0
  bits 4-2 = set layers priorities:
     Reset default is 000, sprites over the Layer 2, over the ULA graphics
     000 - S L U
     001 - L S U
     010 - S U L
*    011 - L U S
*    100 - U S L
*    101 - U L S
     110 - S(U+L) ULA and Layer 2 combined, colours clamped to 7
     111 - S(U+L-5) ULA and Layer 2 combined, colours clamped to [0,7]
  bit 1 = Over border (1 = yes)(Back to 0 after a reset)
  bit 0 = Sprites visible (1 = visible)(Back to 0 after a reset)
  */
	// border area => no Layer 2, only Sprites + tilemap (ULA)
	z80_byte prio=tbblue_get_layers_priorities();
	// all modes where sprites are on top of tilemap (ULA)
	p_layer_first=tbblue_layer_sprites;
	p_layer_second=tbblue_layer_ula;
	p_layer_third=NULL;
	if (prio < 3 || 5 < prio) return;
	// all modes where tilemap (ULA) is on top of Sprites
	p_layer_first=tbblue_layer_ula;
	p_layer_second=tbblue_layer_sprites;
	return;
}

z80_int tbblue_get_border_color(z80_int color)
{
    int flash_disabled = tbblue_registers[0x43]&1;  //flash_disabled se llamaba antes. ahora indica "enable ulanext"
    int is_timex_hires = timex_video_emulation.v && ((timex_port_ff&7) == 6);
    // 1) calculate correct color index into palette
	if (is_timex_hires) {
        // Timex HiRes 512x256 enforces border color by the FF port value, with priority over other methods
        color=get_timex_paper_mode6_color();        //0..7 PAPER index
        if (flash_disabled) color += 128;           // current HW does not bother with Bright in ULANext ON mode
        else color += 8 + 16;                       // +8 for BRIGHT 1, +16 for PAPER color in ULANext OFF mode
	}
    else if (flash_disabled) {   // ULANext mode ON

        if (tbblue_next_palette_format_as_mask_only()) {	// invalid-mask or full-ink mode
            // in such case this is final result, just return it (no further processing needed)
            return RGB9_INDEX_FIRST_COLOR + tbblue_get_9bit_colour(tbblue_registers[0x4A]);
        }

        // other ULANext modes take border color from palette starting at 128..135
        color += 128;
    }
    else {  // ULANext mode OFF (border colors are 16..23)
        color += 16;
    }
    // 2) convert index to actual color from palette
    color = tbblue_get_palette_active_ula(color);
    // 3) check for transparent colour -> use fallback colour if border is "transparent"
    if (tbblue_si_transparent(color)) {
        color = tbblue_get_9bit_colour(tbblue_registers[0x4A]);
    }
    return color + RGB9_INDEX_FIRST_COLOR;
}

void get_ula_pixel_9b_color_tbblue(z80_byte attribute,z80_int *tinta_orig, z80_int *papel_orig)
{

	/*

(R/W) 0x43 (67) => Palette Control
  bit 0 = Enabe ULANext mode if 1. (0 after a reset)

	*/

	z80_byte ink=*tinta_orig;
	z80_byte paper=*papel_orig;

	z80_byte flash_disabled=tbblue_registers[0x43]&1; //flash_disabled se llamaba antes. ahora indica "enable ulanext"


        z80_byte bright,flash;
        z80_int aux;



	if (!flash_disabled) {

/*
(R/W) 0x40 (64) => Palette Index
  bits 7-0 = Select the palette index to change the associated colour.

  For the ULA only, INKs are mapped to indices 0-7, Bright INKS to indices 8-15,
   PAPERs to indices 16-23 and Bright PAPERs to indices 24-31.

  In ULANext mode, INKs come from a subset of indices 0-127 and PAPERs come from
   a subset of indices 128-255.  The number of active indices depends on the number
   of attribute bits assigned to INK and PAPER out of the attribute byte.
  The ULA always takes border colour from paper.
*/

                        ink=attribute &7; 
                        paper=((attribute>>3) &7)+16; //colores papel empiezan en 16
                        bright=(attribute)&64; 
                        flash=(attribute)&128; 
                        if (flash) { 
                                if (estado_parpadeo.v) { 
                                        aux=paper; 
                                        paper=ink; 
                                        ink=aux; 
                                } 
                        } 
            
            if (bright) {   
                paper+=8; 
                ink+=8; 
            } 

	}

	else {
      /*

core3.0:

0x42 (66) => ULANext Attribute Byte Format
(R/W)
  bits 7:0 = Mask indicating which bits of an attribute byte are used to represent INK.
    Other bits represent PAPER. (soft reset = 0x07)
    The mask can only indicate a solid sequence of bits on the right side of an attribute
    byte (1, 3, 7, 15, 31, 63, 127 or 255).
    INKs are mapped to base index 0 in the palette and PAPERs and border are
    mapped to base index 128 in the palette.
    The 255 value enables the full ink colour mode making all the palette entries INK.
    In this case PAPER and border are both taken from the fallback colour in nextreg 0x4A.
    If the mask is not one of those listed above, the INK is taken as the logical AND of
    the mask with the attribute byte and the PAPER and border colour are again both taken
    from the fallback colour in nextreg 0x4A.
		*/
		z80_byte mascara_tinta=tbblue_registers[0x42];
		if (tbblue_next_palette_format_as_mask_only()) {
			//invalid mask or full-ink mode, the background color is "transparency fallback" register
			ink=attribute&mascara_tinta;
			*tinta_orig=tbblue_get_palette_active_ula(ink);
			*papel_orig=tbblue_get_9bit_colour(tbblue_registers[0x4A]);
			return;
		}

		//only valid ink mask (and not "full-ink")
		int rotacion_papel=1;

		//Estos valores se podrian tener ya calculados al llamar desde la funcion de screen_store_scanline_rainbow_solo_display_tbblue
		//o incluso calcularlos en cuanto se modificase el registro 42h o 43h
		//Como realmente son pocas variables a calcular, quiza ni merece la pena

		switch (mascara_tinta) {
			case 1:
				rotacion_papel=1;
			break;
	
			case 3:
				rotacion_papel=2;
			break;

			case 7:
				rotacion_papel=3;
			break;

			case 15:
				rotacion_papel=4;
			break;

			case 31:
				rotacion_papel=5;
			break;

			case 63:
				rotacion_papel=6;
			break;

			case 127:
				rotacion_papel=7;
			break;

		}

		ink=attribute & mascara_tinta;
		paper=(attribute >> rotacion_papel)+128;

	}

	*tinta_orig=tbblue_get_palette_active_ula(ink);
	*papel_orig=tbblue_get_palette_active_ula(paper);
}

z80_int tbblue_tile_return_color_index(z80_byte index)
{
        z80_int color_final=tbblue_get_palette_active_tilemap(index);
        return color_final;
}

void tbblue_do_tile_putpixel_monocrome(z80_byte pixel_color,z80_int *puntero_a_layer,int ula_over_tilemap)
{
	// in monocrome mode translate color index into 9bit colour from palette
	z80_int color = tbblue_tile_return_color_index(pixel_color);
	// then check against Global Transparency Color register (0x14 == TBBLUE_TRANSPARENT_REGISTER)
	if (!tbblue_si_transparent(color)) {
		//No es color transparente el que ponemos

		//Vemos lo que hay en la capa
		z80_int color_previo_capa;
		color_previo_capa=*puntero_a_layer;

		//Poner pixel tile si color de ula era transparente o bien la ula está por debajo
		if (tbblue_si_sprite_transp_ficticio(color_previo_capa) || !ula_over_tilemap) {
			*puntero_a_layer = color;
		}

	}

}

void tbblue_do_tile_putpixel(z80_byte pixel_color,z80_byte transparent_colour,z80_byte tpal,z80_int *puntero_a_layer,int ula_over_tilemap)
{
	if (tbblue_tiles_are_monocrome()) {

		tbblue_do_tile_putpixel_monocrome(pixel_color|tpal, puntero_a_layer, ula_over_tilemap);

	} else {

			if (pixel_color!=transparent_colour) {
				//No es color transparente el que ponemos
				pixel_color |=tpal;

				//Vemos lo que hay en la capa
				z80_int color_previo_capa;
				color_previo_capa=*puntero_a_layer;

				//Poner pixel tile si color de ula era transparente o bien la ula está por debajo
				if (tbblue_si_sprite_transp_ficticio(color_previo_capa) || !ula_over_tilemap) { 
					*puntero_a_layer=tbblue_tile_return_color_index(pixel_color);
				}

			}

	}
}

//Devuelve el color del pixel dentro de un tilemap
z80_byte tbblue_get_pixel_tile_xy_4bpp(int x,int y,z80_byte *puntero_this_tiledef)
{
	//4bpp
	int offset_x=x/2;

	int pixel_a_derecha=x%2;

	int offset_y=y*4; //Cada linea ocupa 4 bytes

	int offset_final=offset_y+offset_x;


	z80_byte byte_leido=puntero_this_tiledef[offset_final];
	if (pixel_a_derecha) {
		return byte_leido & 0xF;
	}

	else {
		return (byte_leido>>4) & 0xF;
	}

}

int tbblue_tiles_are_monocrome(void)
{
/*
Registro 6BH


  
    (R/W) 0x6B (107) => Tilemap Control


Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector

0	1 to enforce "tilemap over ULA" layer priority

Bits 7 & 6 enable the tilemap and select resolution.

Bit 5 changes the structure of the tilemap so that it contains only 8-bit tilemap-id entries instead of 16-bit tilemap-id and tile-attribute entries.

If 8-bit tilemap is selected, the tilemap contains only tile numbers and the attributes are taken from Default Tilemap Attribute Register ($6C).

Bit 4 selects one of two tilemap palettes used for final colour lookup.

Bit 1 enables the 512-tile-mode when the tile attribute (either global in $6C or per tile in map data) contains ninth bit of tile-id value. 
In this mode the tiles are drawn under ULA pixels, unless bit 0 is used to force whole tilemap over ULA.

Bit 0 can enforce tilemap over ULA either in 512-tile-mode, or even override the per-tile bit selector from tile attributes. 
If zero, the tilemap priority is either decided by attribute bit or in 512-tile-mode it is under ULA.


*/


	return tbblue_registers[0x6b] & 8; //bit "text mode"/monocromo

}


//Devuelve el color del pixel dentro de un tilemap
z80_byte tbblue_get_pixel_tile_xy_monocromo(int x,int y,z80_byte *puntero_this_tiledef)
{
	//4bpp
	int offset_x=0;

	//int pixel_a_derecha=x%2;

	int offset_y=y; //Cada linea ocupa 1 bytes

	int offset_final=offset_y+offset_x;


	z80_byte byte_leido=puntero_this_tiledef[offset_final];

	return (byte_leido>> (7-x) ) & 0x1;

}


z80_byte tbblue_get_pixel_tile_xy(int x,int y,z80_byte *puntero_this_tiledef)
{
	//si monocromo
	if (tbblue_tiles_are_monocrome() ) {
		return tbblue_get_pixel_tile_xy_monocromo(x,y,puntero_this_tiledef);
	}

	else {
		return tbblue_get_pixel_tile_xy_4bpp(x,y,puntero_this_tiledef);
	}
}

/*int temp_tile_rebote_x=10;
int temp_tile_rebote_y=10;
int temp_tile_rebote_incx=+1;
int temp_tile_rebote_incy=+1;
int temp_tile_rebote_veces=0;*/

void tbblue_do_tile_overlay(int scanline)
{
	//Gestion scroll vertical
	int scroll_y=tbblue_registers[49];

	//Renderizar en array tbblue_layer_ula el scanline indicado
	//leemos del tile y indicado, sumando scroll vertical
	int scanline_efectivo=scanline+scroll_y;
	scanline_efectivo %=256; 
	

	int posicion_y=scanline_efectivo/8;

	int linea_en_tile=scanline_efectivo %8;

  int tbblue_bytes_per_tile=2;

	int tilemap_width=tbblue_get_tilemap_width();

	int multiplicador_ancho=1;
	if (tilemap_width==40) multiplicador_ancho=2;
/*
//borde izquierdo + pantalla + borde derecho
#define TBBLUE_LAYERS_PIXEL_WIDTH (48+256+48)

z80_int tbblue_layer_ula[TBBLUE_LAYERS_PIXEL_WIDTH];
*/

	z80_int *puntero_a_layer;
	puntero_a_layer=&tbblue_layer_ula[(48-32)*2]; //Inicio de pantalla es en offset 48, restamos 32 pixeles que es donde empieza el tile
																								//*2 porque es doble de ancho

	z80_int *orig_puntero_a_layer;
	orig_puntero_a_layer=puntero_a_layer;

  /*
Bit	Function
7	1 to enable the tilemap
6	0 for 40x32, 1 for 80x32
5	1 to eliminate the attribute entry in the tilemap
4	palette select (0 = first Tilemap palette, 1 = second)
3	(core 3.0) enable "text mode"
2	Reserved, must be 0
1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
   */
	z80_byte tbblue_tilemap_control=tbblue_registers[107];

	if (tbblue_tilemap_control&32) tbblue_bytes_per_tile=1;




	z80_byte *puntero_tilemap;	
	z80_byte *puntero_tiledef;

	//Gestion scroll
/*(R/W) 0x2F (47) => Tilemap Offset X MSB
  bits 7-2 = Reserved, must be 0
  bits 1-0 = MSB X Offset
  Meaningful Range is 0-319 in 40 char mode, 0-639 in 80 char mode

(R/W) 0x30 (48) => Tilemap Offset X LSB
  bits 7-0 = LSB X Offset
  Meaningful range is 0-319 in 40 char mode, 0-639 in 80 char mode

(R/W) 0x31 (49) => Tilemap Offset Y
  bits 7-0 = Y Offset (0-191)
*/
	int scroll_x=tbblue_registers[48]+256*(tbblue_registers[47] & 3);

	//Llevar control de posicion x pixel en destino dentro del rango (0..40*8, 0..80*8)
	int max_destino_x_pixel=tilemap_width*8;

	scroll_x %=max_destino_x_pixel;

	int destino_x_pixel=0;

	int offset_sumar=0;
	if (scroll_x) {
		//Si hay scroll_x, no que hacemos es empezar a escribir por la parte final derecha
		destino_x_pixel=max_destino_x_pixel-scroll_x;
		offset_sumar=destino_x_pixel;
	}


	offset_sumar *=multiplicador_ancho;
	puntero_a_layer +=offset_sumar;


	//Clipwindow horizontal. Limites
	
				/*
				The tilemap display surface extends 32 pixels around the central 256×192 display.
The origin of the clip window is the top left corner of this area 32 pixels to the left and 32 pixels above 
the central 256×192 display. The X coordinates are internally doubled to cover the full 320 pixel width of the surface.
 The clip window indicates the portion of the tilemap display that is non-transparent and its indicated extent is inclusive; 
 it will extend from X1*2 to X2*2+1 horizontally and from Y1 to Y2 vertically.
			*/




	int clipwindow_min_x=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]*2;
	int clipwindow_max_x=(clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]+1)*2;




	//Para controlar clipwindow. Coordenadas de destino_x_pixel van de 0 a 319 en modo 40 columnas, o de 0 a 639 en modo 80 columnas
	if (tilemap_width==80) {
		clipwindow_min_x *=2;
		clipwindow_max_x *=2;
	}

	//printf ("clipwindow_min_x %d clipwindow_max_x %d\n",clipwindow_min_x,clipwindow_max_x);

	//Inicio del tilemap
	puntero_tilemap=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tilemap());

	//Obtener offset sobre tilemap
	int offset_tilemap=tbblue_bytes_per_tile*tilemap_width*posicion_y;


	puntero_tilemap +=offset_tilemap;  //Esto apuntara al primer tile de esa posicion y y con x=0


	//Inicio del tiledef
	puntero_tiledef=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tiledef());

	//puntero_a_layer -=scroll_x; //temp chapuza


	int x;

	int xmirror,ymirror,rotate;
	z80_byte tpal;

	z80_byte byte_first;
	z80_byte byte_second;

	int ula_over_tilemap;

	// 0 when tilemap-over-ULA is enforced, 1 when attribute ULA-over-tilemap bit should be used
	int ula_over_tilemap_mask = (tbblue_tilemap_control&1)^1;

	//tilemap_width=40;
/*
(R/W) 0x4C (76) => Transparency index for the tilemap
  bits 7-4 = Reserved, must be 0
  bits 3-0 = Set the index value (0xF after reset)
Defines the transparent colour index for tiles. The 4-bit pixels of a tile definition are compared to this value to determine if they are transparent.
*/
	z80_byte transparent_colour=tbblue_registers[76] & 0xF;



	z80_byte tbblue_default_tilemap_attr=tbblue_registers[108];



		

	for (x=0;x<tilemap_width;x++) {
		//TODO stencil mode
		byte_first=*puntero_tilemap;
		puntero_tilemap++;
		if (tbblue_bytes_per_tile==2) {
			byte_second=*puntero_tilemap;
			puntero_tilemap++;
		} else {
			byte_second = tbblue_default_tilemap_attr;
		}
                                        
		int tnum=byte_first;

/*
  bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap OR bit 8 of tile number (512 tile mode)
  bits   7-0 : tile number
  */                                      

		if (tbblue_tiles_are_monocrome()) {
			tpal=(byte_second)&0xFE;
			xmirror=0;
			ymirror=0;
			rotate=0;
		} else {
			tpal=(byte_second)&0xF0;
			xmirror=(byte_second>>3)&1;
			ymirror=(byte_second>>2)&1;
			rotate=(byte_second>>1)&1;
		}

		if (tbblue_tilemap_control&2) {
			// 512 tile mode
			tnum |= (byte_second&1)<<8;
			ula_over_tilemap = ula_over_tilemap_mask;
		} else {
			// 256 tile mode, "ULA over tilemap" bit used from attribute (plus "force tilemap")
			ula_over_tilemap = byte_second & ula_over_tilemap_mask;
		}

		//printf ("Color independiente. tpal:%d byte_second: %02XH\n",tpal,byte_second);

		//Sacar puntero a principio tiledef. 
		int offset_tiledef;


		if (tbblue_tiles_are_monocrome()) {
			offset_tiledef=tnum*TBBLUE_TILE_HEIGHT;
		}
		else {
			//4 bpp. cada tiledef ocupa 4 bytes * 8 = 32
			offset_tiledef=tnum*(TBBLUE_TILE_WIDTH/2)*TBBLUE_TILE_HEIGHT;
		}

		//sumar posicion y
		//offset_tiledef += linea_en_tile*4;

		//tiledef

		//printf ("tpal %d\n",tpal);		

		//Renderizar los 8 pixeles del tile
		int pixel_tile;
		z80_byte *puntero_this_tiledef;
		puntero_this_tiledef=&puntero_tiledef[offset_tiledef];


		//Incrementos de x e y
		int incx=+1;
		int incy=0;

		z80_byte sx=0,sy=0; //Coordenadas x,y dentro del tile

		//sumar posicion y
		sy += linea_en_tile;		


		//Aplicar mirror si conviene y situarnos en la ultima linea
		if (ymirror) {
			//sy=TBBLUE_TILE_HEIGHT-1-diferencia;
			sy=TBBLUE_TILE_HEIGHT-1-linea_en_tile;
		}
		else {
			//sy=diferencia;
		}

		//Cambiar offset si mirror x, ubicarlo a la derecha del todo
		if (xmirror) {
			sx=TBBLUE_TILE_WIDTH-1;
			incx=-1;
		}


	//Rotacion. Mismo metodo que con sprites
							/*
              Comparar bits rotacion con ejemplo en media/spectrum/tbblue/sprites/rotate_example.png
              */
              /*
             Basicamente sin rotar un sprite, se tiene (reduzco el tamaño a la mitad aqui para que ocupe menos)


            El sentido normal de dibujado viene por ->, aumentando coordenada X


				->  ---X----
						---XX---
						---XXX--
						---XXXX-
						---X----
						---X----
						---X----
						---X----

				Luego cuando se rota 90 grados, en vez de empezar de arriba a la izquierda, se empieza desde abajo y reduciendo coordenada Y:

						---X----
						---XX---
						---XXX--
						---XXXX-
						---X----
						---X----
		^       ---X----
		|     	---X----

				Entonces, al dibujar empezando asi, la imagen queda rotada:

						--------
						--------
						XXXXXXXX
						----XXX-
						----XX--
						----X---
						--------

				De ahi que el incremento y sea -incremento x , incremento x sera 0

				Aplicando tambien el comportamiento para mirror, se tiene el resto de combinaciones

				*/



			
		if (rotate) {
			z80_byte sy_old=sy;
			sy=(TBBLUE_TILE_HEIGHT-1)-sx;
			sx=sy_old;

			incy=-incx;
			incx=0;
		}


		for (pixel_tile=0;pixel_tile<8;pixel_tile++) {

			z80_byte pixel;
			pixel=tbblue_get_pixel_tile_xy(sx,sy,puntero_this_tiledef);

			if (destino_x_pixel>=clipwindow_min_x && destino_x_pixel<clipwindow_max_x) {
				tbblue_do_tile_putpixel(pixel,transparent_colour,tpal,puntero_a_layer,ula_over_tilemap);
				if (tilemap_width==40) tbblue_do_tile_putpixel(pixel,transparent_colour,tpal,puntero_a_layer+1,ula_over_tilemap);
			}
			puntero_a_layer++;
			if (tilemap_width==40) puntero_a_layer++;
			destino_x_pixel++;

			sx=sx+incx;
			sy=sy+incy;

			//Controlar si se sale por la derecha (pues hay scroll)
			if (destino_x_pixel==max_destino_x_pixel) {
				destino_x_pixel=0;
				puntero_a_layer=orig_puntero_a_layer;
			}

		}


  }

}

void tbblue_fast_render_ula_layer(z80_int *puntero_final_rainbow,int estamos_borde_supinf,
	int final_borde_izquierdo,int inicio_borde_derecho,int ancho_rainbow)
{


	int i;
		z80_int color;


	//(R/W) 0x4A (74) => Transparency colour fallback
	//	bits 7-0 = Set the 8 bit colour.
	//	(0 = black on reset on reset)
	z80_int fallbackcolour = RGB9_INDEX_FIRST_COLOR + tbblue_get_9bit_colour(tbblue_registers[74]);

	for (i=0;i<ancho_rainbow;i++) {


		//Primera capa
		color=tbblue_layer_ula[i];
		if (!tbblue_si_sprite_transp_ficticio(color) ) {
			*puntero_final_rainbow=RGB9_INDEX_FIRST_COLOR+color;
			//doble de alto
			puntero_final_rainbow[ancho_rainbow]=RGB9_INDEX_FIRST_COLOR+color; 
		}

	
					
				else {
					if (estamos_borde_supinf) {
						//Si estamos en borde inferior o superior, no hacemos nada, dibujar color borde
					}

					else {
						//Borde izquierdo o derecho o pantalla. Ver si estamos en pantalla
						if (i>=final_borde_izquierdo && i<inicio_borde_derecho) {
							//Poner color indicado por "Transparency colour fallback" registro
							*puntero_final_rainbow=fallbackcolour;
							//doble de alto
							puntero_final_rainbow[ancho_rainbow]=fallbackcolour;								
						}
						else {
							//Es borde. dejar ese color
						}
					
					}
				}


	

		puntero_final_rainbow++;

		
	}

}

void tbblue_render_blended_rainbow(z80_int *puntero_final_rainbow, int final_borde_izquierdo,
	int inicio_borde_derecho, int ancho_rainbow, z80_int fallbackcolour)
{
	const int sub = (6 == tbblue_get_layers_priorities()) ? 0 : -5;	// subtract to blend value
	int i;
	for (i=0;i<ancho_rainbow;i++) {

		z80_int l2_color = tbblue_layer_layer2[i];
		int priority = (l2_color&TBBLUE_LAYER2_PRIORITY) && !tbblue_si_sprite_transp_ficticio(l2_color);
		z80_int ula_color = tbblue_layer_ula[i];
		//TODO
		// ula_color should be only ULA or ULA+tile, depending on 0x68 (104) => ULA Control bit 6
		// but at this point I have only ULA+tile in the layer buffer, no idea how to get ULA-only
		// (as the tilemap may be drawn above blended pixel, this probably requires fourth buffer!)

		// if L2 priority bit is set, ignore sprites pixel (priority bit should win)
		z80_int color = priority ? TBBLUE_SPRITE_TRANS_FICT : tbblue_layer_sprites[i];
		if (tbblue_si_sprite_transp_ficticio(color)) {
			// check if blending is possible (no transparent color allowed)
			if (tbblue_si_sprite_transp_ficticio(l2_color)) {
				if (tbblue_si_sprite_transp_ficticio(ula_color)) {
					// all layers transparent
					if (i>=final_borde_izquierdo && i<inicio_borde_derecho) {
						color = fallbackcolour;
					}
					// else color is still transparent, do not modify the buffer
				} else {
					color = ula_color;	// only ULA color
				}
			} else {
				if (tbblue_si_sprite_transp_ficticio(ula_color)) {
					color = l2_color&0x1FF;	// only L2 color (remove priority bit)
				} else {
					// blend L2 + ULA
					int channel_b = (l2_color&0x007) + (ula_color&0x007);
					int channel_g = (l2_color&0x038) + (ula_color&0x038);
					int channel_r = (l2_color&0x1C0) + (ula_color&0x1C0);
					// subtract -5 and clamp on zero value (if in "U+L-5" mode)
					if (sub) {
						channel_b += sub<<0;
						if (channel_b < 0) channel_b = 0;
						channel_g += sub<<3;
						if (channel_g < 0) channel_g = 0;
						channel_r += sub<<6;
						if (channel_r < 0) channel_r = 0;
					}
					// clamp on value 7 (both blend modes can overflow)
					if ((7<<0) < channel_b) channel_b = (7<<0);
					if ((7<<3) < channel_g) channel_g = (7<<3);
					if ((7<<6) < channel_r) channel_r = (7<<6);
					// final color
					color = channel_b + channel_g + channel_r;
				}
			}
		} // else the sprite color is solid and above

		if (!tbblue_si_sprite_transp_ficticio(color)) {
			*puntero_final_rainbow = RGB9_INDEX_FIRST_COLOR + color;
		}

		puntero_final_rainbow++;

	}
	//doble de alto
	for (i=0;i<ancho_rainbow;i++) {		// just copy the previous line
		*puntero_final_rainbow = puntero_final_rainbow[-ancho_rainbow];
		puntero_final_rainbow++;
	}
}

//int tempconta;

//Nos situamos en la linea justo donde empiezan los tiles
void tbblue_render_layers_rainbow(int capalayer2,int capasprites)
{


	//(R/W) 0x4A (74) => Transparency colour fallback
		//	bits 7-0 = Set the 8 bit colour.
		//	(0 = black on reset on reset)
	z80_int fallbackcolour = tbblue_get_9bit_colour(tbblue_registers[74]);

	int tiles_top_y=screen_indice_inicio_pant-TBBLUE_TILES_BORDER;
	int tiles_bottom_y=screen_indice_inicio_pant+192+TBBLUE_TILES_BORDER;
	if (t_scanline_draw<tiles_top_y || t_scanline_draw>=tiles_bottom_y) {
		return; //Si estamos por encima o por debajo de la zona de tiles,
	}
	//que es la mas alta de todas las capas
	int y=t_scanline_draw-screen_invisible_borde_superior;

    if (border_enabled.v==0) y=y-screen_borde_superior;

		//Calcular donde hay border
		int final_border_superior=screen_indice_inicio_pant-screen_invisible_borde_superior;
		int inicio_border_inferior=final_border_superior+192;

		//Doble de alto
		y *=2;

		final_border_superior *=2;
		inicio_border_inferior *=2;

		//Vemos si linea esta en zona border
		int estamos_borde_supinf=0;
		if (y<final_border_superior || y>=inicio_border_inferior) estamos_borde_supinf=1;

		//Zona borde izquierdo y derecho
		int final_borde_izquierdo=2*screen_total_borde_izquierdo*border_enabled.v;
		int inicio_borde_derecho=final_borde_izquierdo+TBBLUE_DISPLAY_WIDTH;




		int ancho_rainbow=get_total_ancho_rainbow();

	z80_int *puntero_final_rainbow=&rainbow_buffer[ y*ancho_rainbow ];

	//Por defecto
	//sprites over the Layer 2, over the ULA graphics

	if (estamos_borde_supinf && !tbblue_is_layer2_256height()) {
		tbblue_set_layer_priorities_border_only();
	} else {
		tbblue_set_layer_priorities();
	}

	// resolve blending modes by specialized routine
	if (6 <= tbblue_get_layers_priorities()) {
		if (!estamos_borde_supinf) {
			tbblue_render_blended_rainbow(puntero_final_rainbow, final_borde_izquierdo, inicio_borde_derecho, ancho_rainbow, fallbackcolour);
			if (!capasprites) {		// inside paper area, or no sprites -> enough was done
				return;
			}
		}
	}

	//printf ("ancho total: %d size layers: %d\n",get_total_ancho_rainbow(),TBBLUE_LAYERS_PIXEL_WIDTH );

	int i;

	//Si solo hay capa ula, hacer render mas rapido
	//printf ("%d %d %d\n",capalayer2,capasprites,tbblue_get_layers_priorities());
	//if (capalayer2==0 && capasprites==0 && tbblue_get_layers_priorities()==0) {  //prio 0=S L U
	if (capalayer2==0 && capasprites==0) {
		//Hará fast render cuando no haya capa de layer2 o sprites, aunque tambien,
		//estando esas capas, cuando este en zona de border o no visible de dichas capas
		tbblue_fast_render_ula_layer(puntero_final_rainbow,estamos_borde_supinf,final_borde_izquierdo,inicio_borde_derecho,ancho_rainbow);

	} else {
		if (!estamos_borde_supinf || tbblue_is_layer2_256height()) {

			for (i=0;i<ancho_rainbow;i++) {

				//find non-transparent pixel
				z80_int color = tbblue_layer_layer2[i];
				if (tbblue_si_sprite_transp_ficticio(color) || 0 == (color&TBBLUE_LAYER2_PRIORITY)) {
					// if layer2 pixel is transparent, or normal-priority, go through the three layers
					color=p_layer_first[i];
					if (tbblue_si_sprite_transp_ficticio(color)) {
						color=p_layer_second[i];
						if (tbblue_si_sprite_transp_ficticio(color)) {
							color=p_layer_third[i];
							if (tbblue_si_sprite_transp_ficticio(color)) {
								if (i>=final_borde_izquierdo && i<inicio_borde_derecho) {
									color=fallbackcolour;
								}
								// else color is still transparent, do not modify the buffer
							}
						}
					}
				} else {
					// else there is priority color in layer2, ignore other layers, remove priority bit
					color &= 0x1FF;
				}

				if (!tbblue_si_sprite_transp_ficticio(color)) {
					*puntero_final_rainbow = RGB9_INDEX_FIRST_COLOR + color;
				}

				puntero_final_rainbow++;

			}

		} else {
			//estamos_borde_supinf = 1 (outside of Layer 2 and ULA, only sprites + tiles)
			for (i=0;i<ancho_rainbow;i++) {

				//find non-transparent pixel
				z80_int color=p_layer_first[i];
				if (tbblue_si_sprite_transp_ficticio(color)) {
					color=p_layer_second[i];
				}
				if (!tbblue_si_sprite_transp_ficticio(color)) {
					*puntero_final_rainbow = RGB9_INDEX_FIRST_COLOR + color;
				}

				puntero_final_rainbow++;
			}
		}

		//doble de alto
		for (i=0;i<ancho_rainbow;i++) {		// just copy the previous line
			*puntero_final_rainbow = puntero_final_rainbow[-ancho_rainbow];
			puntero_final_rainbow++;
		}

	}

}


void tbblue_do_layer2_overlay(const int l2Y)
{
	// NextReg 0x12 is always on display, "shadow" 0x13 is only for write/read over ROM
	int tbblue_layer2_offset=tbblue_get_offset_start_layer2_reg(tbblue_registers[18]);

	//Mantener el offset y en 0..191
	int offset_scroll = tbblue_registers[23] + l2Y;
	offset_scroll %= 192;

	tbblue_layer2_offset += offset_scroll * 256;
	tbblue_layer2_offset &= 0x1FFF00;	// limit reading to 2MiB address space

	z80_byte tbblue_reg_22=tbblue_registers[22];

/*
(R/W) 22 => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)

(R/W) 0x17 (23) => Layer2 Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)
*/

	int posicion_array_layer = screen_total_borde_izquierdo * border_enabled.v * 2; //doble de ancho

	int posx;
	for (posx=0;posx<256;posx++) {
	
		if (posx>=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0] && posx<=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1] ) {

			z80_byte color_layer2=memoria_spectrum[tbblue_layer2_offset+tbblue_reg_22];
			z80_int final_color_layer2=tbblue_get_palette_active_layer2(color_layer2);

			//Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
			if (tbblue_si_transparent(final_color_layer2)) final_color_layer2=TBBLUE_SPRITE_TRANS_FICT;

			tbblue_layer_layer2[posicion_array_layer]=final_color_layer2;
			tbblue_layer_layer2[posicion_array_layer+1]=final_color_layer2; //doble de ancho
		}

		posicion_array_layer+=2; //doble de ancho

		tbblue_reg_22++;

	}

}

// variant for 320x256 and 640x256 Layer 2 modes
void tbblue_do_layer2_256h_overlay(const int l2Y)
{
	// The 640 mode has identical memory layout as 320, but every byte is then two 4bpp pixels
	const int is4bpp = (0x20 == (0x30 & tbblue_registers[112]));

	// NextReg 0x12 is always on display, "shadow" 0x13 is only for write/read over ROM
	int tbblue_layer2_offset = tbblue_get_offset_start_layer2_reg(tbblue_registers[18]);

/*
(R/W) 0x16 (22) => Layer2 Offset X
  bits 7-0 = X Offset (0-255)(Reset to 0 after a reset)

(R/W) 0x17 (23) => Layer2 Offset Y
  bits 7-0 = Y Offset (0-191)(Reset to 0 after a reset)

(R/W) 0x71 (113) => Layer 2 X Scroll MSB
	bits 7:1 = Reserved, must be 0
	bit 0 = MSB of scroll amount
*/

	// adjust bottom byte of offset with correct Y coordinate ((Y_offset + l2Y) mod 256)
	tbblue_layer2_offset += (tbblue_registers[23] + l2Y) & 255;

	const int minx = clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0] * 2;
	int maxx = (clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1]+1) * 2;
	if (320 < maxx) maxx = 320;

	int x = tbblue_registers[22] + ((tbblue_registers[113]&1)<<8);

	int posicion_array_layer = (screen_total_borde_izquierdo - 32) * 2;
		// no more support for `border_enabled` - TODO remove/disable/exit from TBBLUE completely
		// currently it will probably just crash on invalid memory access

	// start at clipped X1
	posicion_array_layer += minx * 2;
	x = (x + minx) % 320;

	for (int posx = minx; posx < maxx; ++posx) {

		int pixels_address = tbblue_layer2_offset + 256 * x;
		pixels_address &= 0x1FFFFF;		// limit reading to 2MiB address space
		z80_byte color_layer2 = memoria_spectrum[pixels_address];
		z80_int pixel_left, pixel_right;
		if (is4bpp) {	// 640x256 mode
			pixel_left = tbblue_get_palette_active_layer2(color_layer2>>4);
			if (tbblue_si_transparent(pixel_left)) pixel_left = TBBLUE_SPRITE_TRANS_FICT;
			pixel_right = tbblue_get_palette_active_layer2(color_layer2&0x0F);
			if (tbblue_si_transparent(pixel_right)) pixel_right = TBBLUE_SPRITE_TRANS_FICT;
		} else {		// 320x256 mode
			pixel_left = tbblue_get_palette_active_layer2(color_layer2);
			if (tbblue_si_transparent(pixel_left)) pixel_left = TBBLUE_SPRITE_TRANS_FICT;
			pixel_right = pixel_left;
		}

		tbblue_layer_layer2[posicion_array_layer] = pixel_left;
		tbblue_layer_layer2[posicion_array_layer+1] = pixel_right;

		posicion_array_layer += 2;

		if (++x == 320) x = 0;

	}

}

void tbblue_reveal_layer_draw(z80_int *layer)
{
	int i;

	for (i=0;i<TBBLUE_LAYERS_PIXEL_WIDTH;i++) {
		z80_int color=*layer;
	
		if (!tbblue_si_sprite_transp_ficticio(color)) {

			//Color de revelado es blanco o negro segun cuadricula:
			// Negro Blanco Negro ...
			// Blanco Negro Blanco ...
			// Negro Blanco Negro ....
			// .....

			//Por tanto tener en cuenta posicion x e y
			int posx=i&1;
			int posy=t_scanline_draw&1;

			//0,0: 0
			//0,1: 1
			//1,0: 1
			//1,0: 0
			//Es un xor

			int si_blanco_negro=posx ^ posy;

			*layer=511*si_blanco_negro; //ultimo de los colores en paleta rgb9 de tbblue -> blanco, negro es 0
		}

		layer++;
	}
}


//Forzar a dibujar capa con color fijo, para debug
z80_bit tbblue_reveal_layer_ula={0};
z80_bit tbblue_reveal_layer_layer2={0};
z80_bit tbblue_reveal_layer_sprites={0};

void tbblue_do_ula_standard_overlay()
{

	if (tbblue_force_disable_layer_ula.v) return;

	//Render de capa standard ULA (normal, timex) 

	//printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

	//linea que se debe leer
	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;

	if (scanline_copia < clip_windows[TBBLUE_CLIP_WINDOW_ULA][2] || clip_windows[TBBLUE_CLIP_WINDOW_ULA][3] < scanline_copia) {
		return;		// clipped on Y-position
	}

	int x,bit;
	z80_int direccion;
	z80_byte byte_leido;


	int color=0;
	z80_byte attribute;
	z80_int ink,paper;


	z80_byte *screen=get_base_mem_pantalla();

/*
	(R/W) 0x26 (38) => ULA X Scroll
	bits 7:0 = X Offset (0-255) (soft reset = 0)

	(R/W) 0x27 (39) => ULA Y Scroll
	bits 7:0 = Y Offset (0-191) (soft reset = 0)
*/

	z80_byte ula_offset_x=tbblue_registers[0x26];
	int indice_origen_bytes=(ula_offset_x/8)*2; //*2 dado que leemos del puntero_buffer_atributos que guarda 2 bytes: pixel y atributo	
	int pixelOffset=(ula_offset_x&7);

	z80_byte tbblue_scroll_y=tbblue_registers[0x27];

	scanline_copia +=tbblue_scroll_y;
	scanline_copia=scanline_copia % 192;



	//Usado cuando hay scroll vertical y por tanto los pixeles y atributos salen de la pantalla tal cual (modo sin rainbow)
	int pos_no_rainbow_pix_x;


	//scroll x para modo no rainbow (es decir, cuando hay scroll vertical)
	pos_no_rainbow_pix_x=ula_offset_x/8;
	pos_no_rainbow_pix_x %=32;	


	//Estos direccion y dir_atributo usados cuando hay scroll vertical y por tanto los pixeles y atributos salen de la pantalla tal cual (modo sin rainbow),
	//y tambien en timex 512x192
	direccion=screen_addr_table[(scanline_copia<<5)];

	int fila=scanline_copia/8;
	int dir_atributo=6144+(fila*32);


	z80_byte *puntero_buffer_atributos;
	z80_byte col6;
	z80_byte tin6, pap6;

	z80_byte timex_video_mode=timex_port_ff&7;
	z80_bit si_timex_hires={0};
	z80_bit si_timex_8_1={0};

	if (timex_video_mode==2) si_timex_8_1.v=1;

	//Por defecto
	puntero_buffer_atributos=scanline_buffer;

	if (timex_video_emulation.v) {
	//Modos de video Timex
	/*
000 - Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum);

001 - Video data at address 24576 and 8x8 color attributes at address 30720;

010 - Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576;

110 - Extended resolution: without color attributes, even columns of video data are taken from address 16384, and odd columns of video data are taken from address 24576
	*/
		switch (timex_video_mode) {

			case 4:
			case 6:
				//512x192 monocromo. 
				//y color siempre fijo
				/*
	bits D3-D5: Selection of ink and paper color in extended screen resolution mode (000=black/white, 001=blue/yellow, 010=red/cyan, 011=magenta/green, 100=green/magenta, 101=cyan/red, 110=yellow/blue, 111=white/black); these bits are ignored when D2=0

				black, blue, red, magenta, green, cyan, yellow, white
				*/

				//Si D2==0, these bits are ignored when D2=0?? Modo 4 que es??

				tin6=get_timex_ink_mode6_color();


				//Obtenemos color
				pap6=get_timex_paper_mode6_color();
				//printf ("papel: %d\n",pap6);

				//Y con brillo
				col6=((pap6*8)+tin6)+64;

			
				si_timex_hires.v=1;
			break;


		}
	}

	//Capa de destino
	int posicion_array_layer=0;
	posicion_array_layer +=(screen_total_borde_izquierdo*border_enabled.v*2); //Doble de ancho


	int columnas=33;

	if (si_timex_hires.v) {
		columnas=66;
	}

    for (x=0;x<columnas;x++) {

		if (tbblue_scroll_y) {
			//Si hay scroll vertical (no es 0) entonces el origen de los bytes no se obtiene del buffer de pixeles y color en alta resolucion,
			//Si no que se obtiene de la pantalla tal cual
			//TODO: esto es una limitacion de tal y como hace el render el tbblue, en que hago render de una linea cada vez,
			//para corregir esto, habria que tener un buffer destino con todas las lineas de ula y hacer luego overlay con cada
			//capa por separado, algo completamente impensable
			//de todas maneras esto es algo extraño que suceda: que alguien le de por hacer efectos en color en alta resolucion, en capa ula,
			//y activar el scroll vertical. En teoria tambien puede hacer parpadeos en juegos normales, pero quien va a querer cambiar el scroll en juegos
			//que no estan preparados para hacer scroll?
			byte_leido=screen[direccion+pos_no_rainbow_pix_x];


			if (si_timex_8_1.v==0) {
				attribute=screen[dir_atributo+pos_no_rainbow_pix_x];	
			}

			else {
				//timex 8x1
				attribute=screen[direccion+pos_no_rainbow_pix_x+8192];
			}



		}

		else {

			//Modo sin scroll vertical. Permite scroll horizontal. Es modo rainbow

			byte_leido=puntero_buffer_atributos[indice_origen_bytes++];

			attribute=puntero_buffer_atributos[indice_origen_bytes++];

		}



		//32 columnas
		//truncar siempre a modulo 64 (2 bytes: pixel y atributo)
		indice_origen_bytes %=64;

		if (si_timex_hires.v) {
			if ((x&1)==0) byte_leido=screen[direccion+pos_no_rainbow_pix_x];
			else byte_leido=screen[direccion+pos_no_rainbow_pix_x+8192];

			attribute=col6;
		}			
			
		get_ula_pixel_9b_color_tbblue(attribute,&ink,&paper);
			
    	for (bit=0;bit<8;bit++) {			
			color= ( byte_leido & 128 ? ink : paper ) ;

			int posx=x*8+bit; //Posicion pixel. Para clip window registers	
			if (si_timex_hires.v) posx /=2;
			posx -= pixelOffset;

			//Tener en cuenta valor clip window
			
			//(W) 0x1A (26) => Clip Window ULA/LoRes
			if (posx>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][0] && posx<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][1]) {
				//Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
				if (tbblue_si_transparent(color)) color=TBBLUE_SPRITE_TRANS_FICT;

				tbblue_layer_ula[posicion_array_layer-pixelOffset*2]=color;
				if (si_timex_hires.v==0) tbblue_layer_ula[posicion_array_layer-pixelOffset*2+1]=color; //doble de ancho
			}

		
			posicion_array_layer++;
			if (si_timex_hires.v==0) posicion_array_layer++; //doble de ancho
        	byte_leido=byte_leido<<1;
				
      	}

		if (si_timex_hires.v) {
				if (x&1) {
					pos_no_rainbow_pix_x++;
					//direccion++;
				}
		}

		else {
			//direccion++;
			pos_no_rainbow_pix_x++;
		}


			
		pos_no_rainbow_pix_x %=32;		

	  }
	
}




void tbblue_do_ula_lores_overlay()
{


	//Render de capa ULA LORES
	//printf ("scan line de pantalla fisica (no border): %d\n",t_scanline_draw);

	//linea que se debe leer
	int scanline_copia=t_scanline_draw-screen_indice_inicio_pant;


	int color;

	/* modo lores
	(R/W) 0x15 (21) => Sprite and Layers system
  bit 7 - LoRes mode, 128 x 96 x 256 colours (1 = enabled)
  	*/

	  	

	z80_byte *lores_pointer;
	z80_byte posicion_x_lores_pointer;

	
	int linea_lores=scanline_copia;  
	//Sumamos offset y
	/*
	(R/W) 0x32 (50) => LoRes X Scroll
	bits 7:0 = X Offset (0-255) (soft reset = 0)
	LoRes scrolls in "half-pixels" at the same resolution and smoothness as Layer 2.

	(R/W) 0x33 (51) => LoRes Y Scroll
	bits 7:0 = Y Offset (0-191) (soft reset = 0)
	LoRes scrolls in "half-pixels" at the same resolution and smoothness as Layer 2.
	*/
	linea_lores +=tbblue_registers[0x33];

	linea_lores=linea_lores % 192;

	lores_pointer=get_lores_pointer(linea_lores/2);  //admite hasta y=95, dividimos entre 2 linea actual

	//Y scroll horizontal
	posicion_x_lores_pointer=tbblue_registers[0x32];
  		


	int posicion_array_layer=0;
	posicion_array_layer +=(screen_total_borde_izquierdo*border_enabled.v*2); //Doble de ancho


	int posx;
	z80_int color_final;

	for (posx=0;posx<256;posx++) {
				
		color=lores_pointer[posicion_x_lores_pointer/2];
		//tenemos indice color de paleta
		//transformar a color final segun paleta ula activa
		//color=tbblue_get_palette_active_ula(lorescolor);

		posicion_x_lores_pointer++; 
		//nota: dado que es una variable de 8 bits, automaticamente se trunca al pasar de 255 a 0, por tanto no hay que sacar el modulo de division con 256
		
		//Tener en cuenta valor clip window
		
		//(W) 0x1A (26) => Clip Window ULA/LoRes
		if (posx>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][0] && posx<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][1] && scanline_copia>=clip_windows[TBBLUE_CLIP_WINDOW_ULA][2] && scanline_copia<=clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]) {
			if (!tbblue_force_disable_layer_ula.v) {
				color_final=tbblue_get_palette_active_ula(color);

				//Ver si color resultante es el transparente de ula, y cambiarlo por el color transparente ficticio
				if (tbblue_si_transparent(color_final)) color_final=TBBLUE_SPRITE_TRANS_FICT;

				tbblue_layer_ula[posicion_array_layer]=color_final;
				tbblue_layer_ula[posicion_array_layer+1]=color_final; //doble de ancho

			}
		}

		posicion_array_layer+=2; //doble de ancho
				
    }


}

//Guardar en buffer rainbow la linea actual. Para Spectrum. solo display
//Tener en cuenta que si border esta desactivado, la primera linea del buffer sera de display,
//en cambio, si border esta activado, la primera linea del buffer sera de border
void screen_store_scanline_rainbow_solo_display_tbblue(void)
{

	//si linea no coincide con entrelazado, volvemos
	if (if_store_scanline_interlace(t_scanline_draw)==0) return;

	

	int i;

	z80_int *clear_p_ula=tbblue_layer_ula;
	z80_int *clear_p_layer2=tbblue_layer_layer2;
	z80_int *clear_p_sprites=tbblue_layer_sprites;

	for (i=0;i<TBBLUE_LAYERS_PIXEL_WIDTH;i++) {

		//Esto es un pelin mas rapido hacerlo asi, con punteros e incrementarlos, en vez de indices a array
		*clear_p_ula=TBBLUE_SPRITE_TRANS_FICT;
		//*clear_p_layer2=TBBLUE_TRANSPARENT_REGISTER_9;
		*clear_p_layer2=TBBLUE_SPRITE_TRANS_FICT;
		*clear_p_sprites=TBBLUE_SPRITE_TRANS_FICT;

		clear_p_ula++;
		clear_p_layer2++;
		clear_p_sprites++;

	}

	const int paperY = t_scanline_draw - screen_indice_inicio_pant;
	// fullY is 0..255 (for 320x256 like Tiles, Sprites, Layer 2, ...), PAPER area starts at fullY==32
	const int fullY = paperY + TBBLUE_SPRITE_BORDER;

	// check for scanlines completely outside of any layer range
	if (fullY < 0 || 32+192+32 <= fullY) {
		tbblue_render_layers_rainbow(0,0);
		return;
	}

	int capalayer2=0;
	int capasprites=0;

  	// LoRes and ULA modes are visible only in PAPER area
  	if (0 <= paperY && paperY < 192) {
		int tbblue_lores=tbblue_registers[0x15] & 128;
		if (tbblue_lores) tbblue_do_ula_lores_overlay();
		else if (tbblue_if_ula_is_enabled()) tbblue_do_ula_standard_overlay();
	}


	//Overlay de layer2
	if (tbblue_is_active_layer2() && !tbblue_force_disable_layer_layer_two.v) {
		const int isLayer2height256 = tbblue_is_layer2_256height();
		const int l2Y = isLayer2height256 ? fullY : paperY;
		if (l2Y>=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2] && l2Y<=clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]) {
			capalayer2=1;
			if (isLayer2height256) {
				tbblue_do_layer2_256h_overlay(l2Y);
			} else {
				tbblue_do_layer2_overlay(l2Y);
			}
			if (tbblue_reveal_layer_layer2.v) {
					tbblue_reveal_layer_draw(tbblue_layer_layer2);
			}
		}
	}


	//Overlay de Tilemap
	if ( tbblue_if_tilemap_enabled() && tbblue_force_disable_layer_tilemap.v==0) {
		/*
				The tilemap display surface extends 32 pixels around the central 256×192 display.
The origin of the clip window is the top left corner of this area 32 pixels to the left and 32 pixels above 
the central 256×192 display. The X coordinates are internally doubled to cover the full 320 pixel width of the surface.
 The clip window indicates the portion of the tilemap display that is non-transparent and its indicated extent is inclusive; 
 it will extend from X1*2 to X2*2+1 horizontally and from Y1 to Y2 vertically.
			*/

		//Tener en cuenta clip window
		if (fullY>=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2] && fullY<=clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]) {
			tbblue_do_tile_overlay(fullY);
		}
	}


	if (tbblue_reveal_layer_ula.v) {
		tbblue_reveal_layer_draw(tbblue_layer_ula);
	}



	// the tbsprite_do_overlay returns 0 when the line is completely clipped or sprites are disabled
	if (!tbblue_force_disable_layer_sprites.v && tbsprite_do_overlay()) {
		// else some sprite is active and maybe visible on screen
		capasprites=1;

		if (tbblue_reveal_layer_sprites.v) {
				tbblue_reveal_layer_draw(tbblue_layer_sprites);
		}
	}




  //Renderizamos las 3 capas buffer rainbow
	tbblue_render_layers_rainbow(capalayer2,capasprites);



}




z80_byte return_tbblue_mmu_segment(z80_int dir)
{
        int segmento=dir/8192;
        z80_byte reg_mmu_value=tbblue_registers[80+segmento];
        return reg_mmu_value;
}


//Si la zona de 0-16383 es escribible por mmu (registro 80/81 contiene no 255)
int tbblue_is_writable_segment_mmu_rom_space(z80_int dir)
{
	//En maquina en config mode no tiene sentido
	z80_byte maquina=(tbblue_registers[3])&7;
	if (maquina==0) return 0;

	z80_byte mmu_value=return_tbblue_mmu_segment(dir);
	if (mmu_value!=255) return 1;
	else return 0;
}




void screen_tbblue_refresca_pantalla_comun_tbblue(int x,int y,unsigned int color)
{

        int dibujar=0;

        //if (x>255) dibujar=1;
        //else if (y>191) dibujar=1;
        if (scr_ver_si_refrescar_por_menu_activo(x/8,y/8)) dibujar=1;

        if (dibujar) {
		scr_putpixel_zoom(x,y,color);
                scr_putpixel_zoom(x,y+1,color);
                scr_putpixel_zoom(x+1,y,color);
                scr_putpixel_zoom(x+1,y+1,color);
        }
}


//Refresco pantalla sin rainbow para tbblue
void screen_tbblue_refresca_pantalla_comun(void)
{
        int x,y,bit;
        z80_int direccion,dir_atributo;
        z80_byte byte_leido;
        int color=0;
        int fila;
        //int zx,zy;

        z80_byte attribute,ink,paper,bright,flash,aux;


       z80_byte *screen=get_base_mem_pantalla();

        //printf ("dpy=%x ventana=%x gc=%x image=%x\n",dpy,ventana,gc,image);
        z80_byte x_hi;

        for (y=0;y<192;y++) {
                //direccion=16384 | devuelve_direccion_pantalla(0,y);

                //direccion=16384 | screen_addr_table[(y<<5)];
                direccion=screen_addr_table[(y<<5)];


                fila=y/8;
                dir_atributo=6144+(fila*32);
                for (x=0,x_hi=0;x<32;x++,x_hi +=8) {



                                byte_leido=screen[direccion];
                                attribute=screen[dir_atributo];


                                ink=attribute &7;
                                paper=(attribute>>3) &7;
											bright=(attribute) &64;
                                flash=(attribute)&128;
                                if (flash) {
                                        //intercambiar si conviene
                                        if (estado_parpadeo.v) {
                                                aux=paper;
                                                paper=ink;
                                                ink=aux;
                                        }
                                }

                                if (bright) {
                                        ink +=8;
                                        paper +=8;
                                }

                                for (bit=0;bit<8;bit++) {

                                        color= ( byte_leido & 128 ? ink : paper );

					//Por cada pixel, hacer *2s en ancho y alto.
					//Esto es muy simple dado que no soporta modo rainbow y solo el estandard 256x192
					screen_tbblue_refresca_pantalla_comun_tbblue((x_hi+bit)*2,y*2,color);
		

                                        byte_leido=byte_leido<<1;
                                }
                        

     
                        direccion++;
                        dir_atributo++;
                }

        }

}



void screen_tbblue_refresca_no_rainbow_border(void)
{
	int color;

	if (simulate_screen_zx8081.v==1) color=15;
	else color=out_254 & 7;

	if (scr_refresca_sin_colores.v) color=7;

int x,y;



       //parte superior
        for (y=0;y<TBBLUE_TOP_BORDER;y++) {
                for (x=0;x<TBBLUE_DISPLAY_WIDTH*zoom_x+TBBLUE_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,y,color);


                }
        }

        //parte inferior
        for (y=0;y<TBBLUE_TOP_BORDER;y++) {
                for (x=0;x<TBBLUE_DISPLAY_WIDTH*zoom_x+TBBLUE_LEFT_BORDER*2;x++) {
                                scr_putpixel(x,TBBLUE_TOP_BORDER+y+TBBLUE_DISPLAY_HEIGHT*zoom_y,color);


                }
        }


        //laterales
        for (y=0;y<TBBLUE_DISPLAY_HEIGHT*zoom_y;y++) {
                for (x=0;x<TBBLUE_LEFT_BORDER;x++) {
                        scr_putpixel(x,TBBLUE_TOP_BORDER+y,color);
                        scr_putpixel(TBBLUE_LEFT_BORDER+TBBLUE_DISPLAY_WIDTH*zoom_x+x,TBBLUE_TOP_BORDER+y,color);
                }

        }



}


//Refresco pantalla con rainbow. Nota. esto deberia ser una funcion comun y no tener diferentes para comun, prism, tbblue, etc
void screen_tbblue_refresca_rainbow(void)
{


	//aqui no tiene sentido (o si?) el modo simular video zx80/81 en spectrum
	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

	int x,y,bit;

	//margenes de zona interior de pantalla. Para overlay menu
	int margenx_izq=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v;
	int margenx_der=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v+TBBLUE_DISPLAY_WIDTH;
	int margeny_arr=TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v;
	int margeny_aba=TBBLUE_BOTTOM_BORDER_NO_ZOOM*border_enabled.v+TBBLUE_DISPLAY_HEIGHT;

	z80_int color_pixel;
	z80_int *puntero;

	puntero=rainbow_buffer;
	int dibujar;


	//Si se reduce la pantalla 0.75
	if (screen_reduce_075.v) {
		screen_scale_075_function(ancho,alto);
		puntero=new_scalled_rainbow_buffer;
	}
	//Fin reduccion pantalla 0.75





	for (y=0;y<alto;y++) {


		for (x=0;x<ancho;x+=8) {
			dibujar=1;

			//Ver si esa zona esta ocupada por texto de menu u overlay

			if (y>=margeny_arr && y<margeny_aba && x>=margenx_izq && x<margenx_der) {
				if (!scr_ver_si_refrescar_por_menu_activo( (x-margenx_izq)/8, (y-margeny_arr)/8) )
					dibujar=0;
			}


			if (dibujar==1) {
					for (bit=0;bit<8;bit++) {
						color_pixel=*puntero++;
						scr_putpixel_zoom_rainbow(x+bit,y,color_pixel);
					}
			}
			else puntero+=8;

		}
		
	}


}





void screen_tbblue_refresca_no_rainbow(void)
{
                //modo clasico. sin rainbow
                if (rainbow_enabled.v==0) {
                        if (border_enabled.v) {
                                //ver si hay que refrescar border
                                if (modificado_border.v)
                                {
                                        //scr_refresca_border();
																				screen_tbblue_refresca_no_rainbow_border();
                                        modificado_border.v=0;
                                }

                        }

                        screen_tbblue_refresca_pantalla_comun();
                }
}


void tbblue_out_port_32765(z80_byte value)
{
				//printf ("TBBLUE changing port 32765 value=0x%02XH\n",value);
                                puerto_32765=value;

				//para indicar a la MMU la  pagina en los segmentos 6 y 7
				tbblue_registers[80+6]=(value&7)*2;
				tbblue_registers[80+7]=(value&7)*2+1;

				//En rom entra la pagina habitual de modo 128k, evitando lo que diga la mmu
				tbblue_registers[80]=255;
				tbblue_registers[81]=255;

                tbblue_set_memory_pages();
}


z80_byte tbblue_uartbridge_readdata(void)
{

	return uartbridge_readdata();
}


void tbblue_uartbridge_writedata(z80_byte value)
{
 
	uartbridge_writedata(value);


}

z80_byte tbblue_uartbridge_readstatus(void)
{
	//No dispositivo abierto
	if (!uartbridge_available()) return 0;

	
	int status=chardevice_status(uartbridge_handler);
	

	z80_byte status_retorno=0;

	if (status & CHDEV_ST_RD_AVAIL_DATA) status_retorno |= TBBLUE_UART_STATUS_DATA_READY;

	return status_retorno;
}
