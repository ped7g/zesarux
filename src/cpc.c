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
#include <string.h>

#include "cpc.h"
#include "cpu.h"
#include "screen.h"
#include "debug.h"
#include "contend.h"
#include "joystick.h"
#include "menu.h"
#include "operaciones.h"
#include "utils.h"
#include "audio.h"
#include "ay38912.h"
#include "tape.h"


//Direcciones donde estan cada pagina de rom. 2 paginas de 16 kb
z80_byte *cpc_rom_mem_table[2];

//Direcciones donde estan cada pagina de ram. 8 paginas de 16 kb cada una
z80_byte *cpc_ram_mem_table[8];

//Direcciones actuales mapeadas para lectura, bloques de 16 kb
z80_byte *cpc_memory_paged_read[4];
//Direcciones actuales mapeadas para escritura, bloques de 16 kb
z80_byte *cpc_memory_paged_write[4];

//Constantes definidas en CPC_MEMORY_TYPE_ROM, _RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_cpc_type_memory_paged_read[4];

//Paginas mapeadas en cada zona de RAM. Se solamente usa en menu debug y breakpoints, no para el core de emulacion
z80_byte debug_cpc_paginas_memoria_mapeadas_read[4];

//Offset a cada linea de pantalla
z80_int cpc_line_display_table[200];

//Forzar modo video para algunos juegos (p.ej. Paperboy)
z80_bit cpc_forzar_modo_video={0};
z80_byte cpc_forzar_modo_video_modo=0;


z80_byte cpc_gate_registers[4];


//Contador de linea para lanzar interrupcion.
z80_byte cpc_scanline_counter;

/*

Registro 2:

Bit	Value	Function
7	1	Gate Array function
6	0
5	-	not used
4	x	Interrupt generation control
3	x	1=Upper ROM area disable, 0=Upper ROM area enable
2	x	1=Lower ROM area disable, 0=Lower ROM area enable
1	x	Screen Mode slection
0	x

*/

z80_byte cpc_border_color;

z80_byte cpc_crtc_registers[32];

z80_byte cpc_crtc_last_selected_register=0;

//0=Port A
//1=Port B
//2=Port C
//3=Control
z80_byte cpc_ppi_ports[4];

//Paleta actual de CPC
z80_byte cpc_palette_table[16];

//tabla de conversion de colores de rgb cpc (8 bit) a 32 bit
//Cada elemento del array contiene el valor rgb real, por ejemplo,
//un valor rgb 11 de cpc, en la posicion 11 de este array retorna el color en formato rgb de 32 bits
int cpc_rgb_table[32]={

//0x000000,  //negro
//0x0000CD,  //azul
//0xCD0000,  //rojo
//0xCD00CD,  //magenta
//0x00CD00,  //verde
//0x00CDCD,  //cyan

0x808080, //0
0x808080, //1
0x00FF80, //2
0xFFFF80, //3

0x000080, //4 Azul
0xFF0080, //5
0x008080, //6
0xFF8080, //7

0xFF0080, //8
0xFFFF80, //9  //Pastel Yellow
0xFFFF00, //10 //Bright Yellow
0xFFFFFF, //11

0xFF0000, //12
0xFF00FF, //13
0xFF8000, //14
0xFF80FF, //15

0x0000FF, //16
0x00FF80, //17
0x00FF00, //18
0x00FFFF, //19

0x000000, //20
0x0000FF, //21
0x008000, //22
0x0080FF, //23

0x800080, //24 
0x80FF80, //25 
0x80FF00, //26
0x80FFFF, //27

0x800000, //28
0x8000FF, //29
0x808000, //30
0x8080FF //31

};



/*Tablas teclado
Bit:
Line	7	6	5	4	3	2	1	0
&40	F Dot	ENTER	F3	F6	F9	CURDOWN	CURRIGHT	CURUP
&41	F0	F2	F1	F5	F8	F7	COPY	CURLEFT
&42	CONTROL	\	SHIFT	F4	]	RETURN	[	CLR
&43	.	/	 :	 ;	P	@	-	^
&44	,	M	K	L	I	O	9	0
&45	SPACE	N	J	H	Y	U	7	8
&46	V	B	F	G (Joy2 fire)	T (Joy2 right)	R (Joy2 left)	5 (Joy2 down)	6 (Joy 2 up)
&47	X	C	D	S	W	E	3	4
&48	Z	CAPSLOCK	A	TAB	Q	ESC	2	1
&49	DEL	Joy 1 Fire 3 (CPC only)	Joy 1 Fire 2	Joy1 Fire 1	Joy1 right	Joy1 left	Joy1 down	Joy1 up

*/


//Aunque solo son 10 filas, metemos array de 16 pues es el maximo valor de indice seleccionable por el PPI
z80_byte cpc_keyboard_table[16]={
255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255
};


z80_bit cpc_send_double_vsync={1};

z80_bit cpc_vsync_signal={0};

void cpc_set_memory_pages()
{

	//Array de paginas que entran. Por defecto en 64kb, paginas 0,1,2,3
	int pages_array[4];

	//Por defecto
	pages_array[0]=0;
	pages_array[1]=1;
	pages_array[2]=2;
	pages_array[3]=3;


	//Si es maquina de 128kb, reasignar paginas
	if (MACHINE_IS_CPC_4128) {
		z80_byte ram_config=cpc_gate_registers[3] & 7;

		//printf ("Setting 128k ram config value %d\n",ram_config);


/*
memoria extendida mas alla de 64 kb

Register 3 - RAM Banking
This register exists only in CPCs with 128K RAM (like the CPC 6128, or CPCs with Standard Memory Expansions). Note: In the CPC 6128, the register is a separate PAL that assists the Gate Array chip.

Bit	Value	Function
7	1	Gate Array function 3
6	1
5	b	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
4	b
3	b
2	x	RAM Config (0..7)
1	x
0	x


The 3bit RAM Config value is used to access the second 64K of the total 128K RAM that is built into the CPC 6128 
or the additional 64K-512K of standard memory expansions. These contain up to eight 64K ram banks, 
which are selected with bit 3-5. A standard CPC 6128 only contains bank 0. 
Normally the register is set to 0, so that only the first 64K RAM are used (identical to the CPC 464 and 664 models). 
The register can be used to select between the following eight predefined configurations only:

 -Address-     0      1      2      3      4      5      6      7
 0000-3FFF   RAM_0  RAM_0  RAM_4  RAM_0  RAM_0  RAM_0  RAM_0  RAM_0
 4000-7FFF   RAM_1  RAM_1  RAM_5  RAM_3  RAM_4  RAM_5  RAM_6  RAM_7
 8000-BFFF   RAM_2  RAM_2  RAM_6  RAM_2  RAM_2  RAM_2  RAM_2  RAM_2
 C000-FFFF   RAM_3  RAM_7  RAM_7  RAM_7  RAM_3  RAM_3  RAM_3  RAM_3
The Video RAM is always located in the first 64K, VRAM is in no way affected by this register.

*/	
		switch (ram_config) {

			case 1:
				pages_array[3]=7;			
			break;

			case 2:
				pages_array[0]=4;
				pages_array[1]=5;
				pages_array[2]=6;
				pages_array[3]=7;			
			break;

			case 3:
				pages_array[1]=3;
				pages_array[3]=7;			
			break;						

			case 4:
				pages_array[1]=4;
			break;	

			case 5:
				pages_array[1]=5;		
			break;	

			case 6:
				pages_array[1]=6;	
			break;	

			case 7:
				pages_array[1]=7;		
			break;													

		}


	}


	//printf ("paginas que entran: %d %d %d %d\n",pages_array[0],pages_array[1],pages_array[2],pages_array[3]);

	//Escritura siempre en RAM
	int i;
	for (i=0;i<4;i++) {
		//cpc_memory_paged_write[i]=cpc_ram_mem_table[i];
		int pagina_entra=pages_array[i];
		cpc_memory_paged_write[i]=cpc_ram_mem_table[pagina_entra];
	}



	int pagina_entra;

	//Bloque 0-16383
	if (cpc_gate_registers[2] &4 ) {
		//Entra RAM
		pagina_entra=pages_array[0];
		cpc_memory_paged_read[0]=cpc_ram_mem_table[pagina_entra];
		debug_cpc_type_memory_paged_read[0]=CPC_MEMORY_TYPE_RAM;
		debug_cpc_paginas_memoria_mapeadas_read[0]=pagina_entra;
	}
	else {
		//Entra ROM
		cpc_memory_paged_read[0]=cpc_rom_mem_table[0];
		debug_cpc_type_memory_paged_read[0]=CPC_MEMORY_TYPE_ROM;
		debug_cpc_paginas_memoria_mapeadas_read[0]=0;
	}




	//Bloque 16384-32767
	//RAM
	pagina_entra=pages_array[1];
	cpc_memory_paged_read[1]=cpc_ram_mem_table[pagina_entra];	
	debug_cpc_type_memory_paged_read[1]=CPC_MEMORY_TYPE_RAM;		
	debug_cpc_paginas_memoria_mapeadas_read[1]=pagina_entra;
	

	//Bloque 32768-49151
	//RAM
	pagina_entra=pages_array[2];
	cpc_memory_paged_read[2]=cpc_ram_mem_table[pagina_entra];	
	debug_cpc_type_memory_paged_read[2]=CPC_MEMORY_TYPE_RAM;
	debug_cpc_paginas_memoria_mapeadas_read[2]=2;
	
    //Bloque 49152-65535
    if (cpc_gate_registers[2] &8 ) {
    	//Entra RAM
		pagina_entra=pages_array[3];
        cpc_memory_paged_read[3]=cpc_ram_mem_table[pagina_entra];
		debug_cpc_type_memory_paged_read[3]=CPC_MEMORY_TYPE_RAM;
		debug_cpc_paginas_memoria_mapeadas_read[3]=pagina_entra;
    }
    else {
        //Entra ROM 
        cpc_memory_paged_read[3]=cpc_rom_mem_table[1];
		debug_cpc_type_memory_paged_read[3]=CPC_MEMORY_TYPE_ROM;
		debug_cpc_paginas_memoria_mapeadas_read[3]=1;
    }

}


void cpc_init_memory_tables()
{
	debug_printf (VERBOSE_DEBUG,"Initializing cpc memory tables");

        z80_byte *puntero;
        puntero=memoria_spectrum;

        cpc_rom_mem_table[0]=puntero;
        puntero +=16384;
        cpc_rom_mem_table[1]=puntero;
        puntero +=16384;


        int i;
        for (i=0;i<8;i++) {
                cpc_ram_mem_table[i]=puntero;
                puntero +=16384;
        }

}

void cpc_out_port_gate(z80_byte value)
{
	/*
Data Bit 7	Data Bit 6	Function
0	0	Select pen
0	1	Select colour for selected pen
1	0	Select screen mode, ROM configuration and interrupt control
1	1	RAM Memory Management (note 1)

note 1: This function is not available in the Gate-Array, but is performed by a device at the same I/O port address location. In the CPC464, CPC664 and KC compact, this function is performed in a memory-expansion (e.g. Dk'Tronics 64K RAM Expansion), if this expansion is not present then the function is not available. In the CPC6128, this function is performed by a PAL located on the main PCB, or a memory-expansion. In the 464+ and 6128+ this function is performed by the ASIC or a memory expansion. Please read the document on RAM management for more information.
	*/


        z80_byte modo_video_antes=cpc_gate_registers[2] &3;


	z80_byte funcion=(value>>6)&3;


	cpc_gate_registers[funcion]=value;


	z80_byte modo_video_despues=cpc_gate_registers[2] &3;

	if (modo_video_despues!=modo_video_antes) cpc_splash_videomode_change();


	//printf ("Changing register %d of gate array\n",funcion);

	switch (funcion) {
		case 0:
			//Seleccion indice color a paleta o border
			if (value&16) {
				//printf ("Seleccion border. TODO\n");
				z80_byte color=value&15;
				if (cpc_border_color!=color) {
					cpc_border_color=color;
					//printf ("Setting border color with value %d\n",color);
					modificado_border.v=1;
				}
			}

			else {
				//Seleccion indice. Guardado en cpc_gate_registers[0] el indice en los 4 bits inferiores
				//printf ("Setting index palette %d\n",cpc_gate_registers[0]&15);
			}
		break;

		case 1:
                        if (cpc_gate_registers[0] & 16) {
                                //printf ("Seleccion border. sin sentido aqui\n");
                        }

                        else {
                                //Seleccion color para indice. Guardado en cpc_gate_registers[0] el indice en los 4 bits inferiores
				z80_byte indice=cpc_gate_registers[0]&15;

				z80_byte color=value&31;

				cpc_palette_table[indice]=color;

				//printf ("Setting index color %d with value %d\n",indice,color);


				//Si se cambia color para indice de color de border, refrescar border
				if (indice==cpc_border_color) modificado_border.v=1;

			}


		break;
		
		case 2:
			//Cambio paginacion y modo video y gestion interrupcion
			cpc_set_memory_pages();

			if (value&16) {
				//Esto resetea bit 5 de contador de scanline
				//printf ("Resetting bit 5 of cpc_scanline_counter\n");
				cpc_scanline_counter &=(255-32);
			}
		break;

		case 3:
			//printf ("Memory management only on cpc 6128. Setting value %02XH\n",value);
			//Cambio paginacion en modos 128kb ram
			cpc_set_memory_pages();
/*
Register 3 - RAM Banking
This register exists only in CPCs with 128K RAM (like the CPC 6128, or CPCs with Standard Memory Expansions). Note: In the CPC 6128, the register is a separate PAL that assists the Gate Array chip.

Bit	Value	Function
7	1	Gate Array function 3
6	1
5	b	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
4	b
3	b
2	x	RAM Config (0..7)
1	x
0	x


The 3bit RAM Config value is used to access the second 64K of the total 128K RAM that is built into the CPC 6128 
or the additional 64K-512K of standard memory expansions. 
These contain up to eight 64K ram banks, which are selected with bit 3-5. A standard CPC 6128 only contains bank 0. 
Normally the register is set to 0, so that only the first 64K RAM are used (identical to the CPC 464 and 664 models). 
The register can be used to select between the following eight predefined configurations only:

 -Address-     0      1      2      3      4      5      6      7
 0000-3FFF   RAM_0  RAM_0  RAM_4  RAM_0  RAM_0  RAM_0  RAM_0  RAM_0
 4000-7FFF   RAM_1  RAM_1  RAM_5  RAM_3  RAM_4  RAM_5  RAM_6  RAM_7
 8000-BFFF   RAM_2  RAM_2  RAM_6  RAM_2  RAM_2  RAM_2  RAM_2  RAM_2
 C000-FFFF   RAM_3  RAM_7  RAM_7  RAM_7  RAM_3  RAM_3  RAM_3  RAM_3
The Video RAM is always located in the first 64K, VRAM is in no way affected by this register.



*/			
		break;
	}
}




//cpc_line_display_table

void init_cpc_line_display_table(void)
{
	debug_printf (VERBOSE_DEBUG,"Init CPC Line Display Table");
	int y;
	z80_int offset;
	for (y=0;y<200;y++) {
		offset=((y / 8) * 80) + ((y % 8) * 2048);
		cpc_line_display_table[y]=offset;
		debug_printf (VERBOSE_DEBUG,"Line: %d Offset: 0x%04X",y,offset);
	}
}


//  return (unsigned char *)0xC000 + ((nLine / 8) * 80) + ((nLine % 8) * 2048);








//Decir si vsync está activo o no, según en qué posición de pantalla estamos,
//y resetear t_scanline_draw a 0 cuando finaliza dicha vsync
//Ver http://www.cpcwiki.eu/index.php/CRTC#HSYNC_and_VSYNC
/*
The VSYNC is also modified before being sent to the monitor. It happens two lines* after the VSYNC from the CRTC 
and stay two lines (same cut rule if VSYNC is lower than 4). PAL (50Hz) does need two lines VSYNC_width, and 4us HSYNC_width.
*/
void cpc_handle_vsync_state(void)
{
	//Duracion vsync
	int vsync_lenght=cpc_crtc_registers[3]&15;

	//Si es 0, en algunos chips significa 16
	if (vsync_lenght==0) vsync_lenght=16;
	//cpc_ppi_ports[1];

	if (cpc_send_double_vsync.v) vsync_lenght *=2;	

	int vsync_position=cpc_crtc_registers[7]&127;
	//esta en caracteres
	vsync_position *=8;


	int final_vsync=vsync_position+vsync_lenght;

	//Lo modificamos
	//vsync_position +=2;

	//final_vsync +=2;


	if (t_scanline_draw>=vsync_position && t_scanline_draw<final_vsync) cpc_vsync_signal.v=1;
	else cpc_vsync_signal.v=0;

	//Y si está justo después, resetear posicion
	if (t_scanline_draw==final_vsync) t_scanline_draw=0;

	//printf ("vsync %d scanline %d scanline_draw %d\n",cpc_vsync_signal.v,t_scanline,t_scanline_draw);

}

z80_byte cpc_get_vsync_bit(void)
{

	//printf ("get vsync scanline %d scanline_draw %d : vsync %d\n",t_scanline,t_scanline_draw,cpc_vsync_signal.v);

	//if (cpc_vsync_signal.v) printf ("1111111######\n");

	return cpc_vsync_signal.v;
}

z80_byte old_cpc_get_vsync_bit(void)
{
				//Bit de vsync
			//Duracion vsync
			z80_byte vsync_lenght=cpc_crtc_registers[3]&15;

			//Si es 0, en algunos chips significa 16
			if (vsync_lenght==0) vsync_lenght=16;
			//cpc_ppi_ports[1];

		int vsync_position=cpc_crtc_registers[7]&127;
		//esta en caracteres
		vsync_position *=8;

		int vertical_total=cpc_crtc_registers[4]+1; //en R0 tambien se suma 1
		vertical_total *=8;

		int vertical_displayed=cpc_crtc_registers[6];
		vertical_displayed *=8;



		//Dynamite dan 1 se pone a comprobar bit de rsync en lineas:
		//0,52,104,156,208,260
		//y vsync empieza en linea 240 y dura 14 lineas... no se cumple nunca
		//esto sera debido a que los timings los tengo mal o las lineas se empiezan a contar diferente... anyway
		//O al obtener la linea actual, no deberia ser t_scanline, sino t_scanline quitando la duracion del ultimo vsync

		//workaround para algunos juegos, como bubble bobble. Lo hacemos durar mas
		if (cpc_send_double_vsync.v) vsync_lenght *=2;		


		int linea_actual=t_scanline;

		//linea actual hay que evitar la zona no visible de arriba (precisamente el vsync)
		linea_actual -=vsync_lenght;

		if (linea_actual<0) {
			//esta en vsync
			return 1;
		}

		//Ver si esta en zona de vsync
		//printf ("linea %d. lenght: %d vsync pos: %d vertical total: %d vertical displayed: %d\n",t_scanline,vsync_lenght,vsync_position,vertical_total,vertical_displayed);
			if (linea_actual>=vsync_position && linea_actual<=vsync_position+vsync_lenght-1) {
			//if (t_scanline>=0 && t_scanline<=7) {
				//printf ("Enviando vsync\n");
				return 1;
			}

			else {
				//printf ("No Enviando vsync\n");
				return 0;
			}

}

//http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning
//http://www.cpcwiki.eu/index.php/8255
z80_byte cpc_in_ppi(z80_byte puerto_h)
{
	
/*
Bit 9	Bit 8	PPI Function	Read/Write status
0	0	Port A data	Read/Write
0	1	Port B data	Read/Write
1	0	Port C data	Read/Write
1	1	Control	Write Only


I/O address	A9	A8	Description	Read/Write status	Used Direction	Used for
&F4xx		0	0	Port A Data	Read/Write		In/Out		PSG (Sound/Keyboard/Joystick)
&F5xx		0	1	Port B Data	Read/Write		In		Vsync/Jumpers/PrinterBusy/CasIn/Exp
&F6xx		1	0	Port C Data	Read/Write		Out		KeybRow/CasOut/PSG
&F7xx		1	1	Control		Write Only		Out		Control
*/

	z80_byte port_number=puerto_h&3;
	z80_byte valor;

	z80_byte psg_function;

	switch (port_number) {
		case 0:
			//printf ("Reading PPI port A\n");
			// cpc_ppi_ports[3]
			psg_function=(cpc_ppi_ports[2]>>6)&3;
                        if (psg_function==1) {
				//printf ("Leer de registro PSG. Registro = %d\n",ay_3_8912_registro_sel);
				if (ay_3_8912_registro_sel[ay_chip_selected]==14) {
					//leer teclado
					//Linea a leer
					z80_byte linea_teclado=cpc_ppi_ports[2] & 15;
					//printf ("Registro 14. Lee fila teclado: 0x%02X\n",linea_teclado | 0x40);


					if (initial_tap_load.v==1 && initial_tap_sequence) {
						return envia_load_ctrlenter_cpc(linea_teclado);
					}

			                //si estamos en el menu, no devolver tecla
			                if (zxvision_key_not_sent_emulated_mach() ) return 255;

           //Si esta spool file activo, generar siguiente tecla
                if (input_file_keyboard_is_playing() ) {
                                input_file_keyboard_get_key();
                }


					
					return cpc_keyboard_table[linea_teclado];
					//if (linea_teclado==8) {
					//	return 255-32;
					//}
					//return 255;
				}


				else if (ay_3_8912_registro_sel[ay_chip_selected]<14) {
					//Registros chip ay
					return in_port_ay(0xFF);
				}
			}
		break;

		case 1:
			//printf ("Reading PPI port B\n");
			valor=cpc_ppi_ports[1];

			//Metemos fabricante amstrad
			valor |= (2+4+8);

			//Refresco 50 hz
			valor |=16;

			//Parallel, expansion port a 0
			valor &=(255-64-32);

			//Bit 0 (vsync) 
			valor &=(255-1);
			valor |=cpc_get_vsync_bit();

 			if (realtape_inserted.v && realtape_playing.v) {
                        	if (realtape_last_value>=realtape_volumen) {
                                	valor=valor|128;
	                                //printf ("1 ");
        	                }
                	        else {
                        	        valor=(valor & (255-128));
                                	//printf ("0 ");
	                        }
			}
			return valor;

			
		break;

		case 2:
			//printf ("Reading PPI port C\n");
			return cpc_ppi_ports[2];
		break;

		case 3:
			//printf ("Reading PPI port control write only\n");
		break;

	}

	return 255;

}


void cpc_cassette_motor_control (int valor_bit) {
                                        //primero vemos si hay cinta insertada
                                        if (realtape_name!=NULL && realtape_inserted.v) {

                                                if (valor_bit) {
                                                        //Activar motor si es que no estaba ya activado
                                                        if (realtape_playing.v==0) {
                                                                debug_printf (VERBOSE_INFO,"CPC motor on function received. Start playing real tape");
                                                                realtape_start_playing();
                                                        }
                                                }
                                                else {
                                                        //Desactivar motor si es que estaba funcionando
                                                        if (realtape_playing.v) {
                                                                debug_printf (VERBOSE_INFO,"CPC motor off function received. Stop playing real tape");
                                                                //desactivado, hay juegos que envian motor off cuando no conviene realtape_stop_playing();
                                                        }
                                                }
                                        }
}


void cpc_out_ppi(z80_byte puerto_h,z80_byte value)
{
/*
Bit 9   Bit 8   PPI Function    Read/Write status
0       0       Port A data     Read/Write
0       1       Port B data     Read/Write
1       0       Port C data     Read/Write
1       1       Control Write Only
*/

        z80_byte port_number=puerto_h&3;
	z80_byte psg_function;

        switch (port_number) {
                case 0:
                        //printf ("Writing PPI port A value 0x%02X\n",value);
			cpc_ppi_ports[0]=value;
                break;

                case 1:
                        //printf ("Writing PPI port B value 0x%02X\n",value);
			cpc_ppi_ports[1]=value;
                break;

                case 2:
                        //printf ("Writing PPI port C value 0x%02X\n",value);
/*
Bit 7	Bit 6	Function
0	0	Inactive
0	1	Read from selected PSG register
1	0	Write to selected PSG register
1	1	Select PSG register

*/
			psg_function=(value>>6)&3;
			if (psg_function==3) {
				//Seleccionar ay chip registro indicado en port A
				//printf ("Seleccionamos PSG registro %d\n",cpc_ppi_ports[0]);
				out_port_ay(65533,cpc_ppi_ports[0]);
			}


			//temp prueba sonido AY
			if (psg_function==2) {
				//Enviar valor a psg
				//printf ("Enviamos PSG valor 0x%02X\n",cpc_ppi_ports[0]);
                                out_port_ay(49149,cpc_ppi_ports[0]);
                        }

			cpc_ppi_ports[2]=value;


                break;

                case 3:
                        //printf ("Writing PPI port control write only value 0x%02X\n",value);
			cpc_ppi_ports[3]=value;
			//CAUTION: Writing to PIO Control Register (with Bit7 set), automatically resets PIO Ports A,B,C to 00h each!
			if (value&128) {
				cpc_ppi_ports[0]=cpc_ppi_ports[1]=cpc_ppi_ports[2]=0;
				//tambien motor off
				//cpc_cassette_motor_control(0);
			}

			else {
				//Bit 7 a 0.
				/*
Otherwise, if Bit 7 is "0" then the register is used to set or clear a single bit in Port C:

 Bit 0    B        New value for the specified bit (0=Clear, 1=Set)
 Bit 1-3  N0,N1,N2 Specifies the number of a bit (0-7) in Port C
 Bit 4-6  -        Not Used
 Bit 7    SF       Must be "0" in this case
				*/
				z80_byte valor_bit=value&1;
				z80_byte mascara=1;
				z80_byte numero_bit=(value>>1)&7;
				if (numero_bit) {
					valor_bit=valor_bit << numero_bit;
					mascara=mascara<<numero_bit;
				}
				mascara = ~mascara;

				//printf ("Estableciendo Reg C: numero bit: %d valor: %d. valor antes Reg C: %d\n",numero_bit,valor_bit,cpc_ppi_ports[2]);

				cpc_ppi_ports[2] &=mascara;
				cpc_ppi_ports[2] |=valor_bit;

				//printf ("Valor despues Reg C: %d\n",cpc_ppi_ports[2]);

				if (numero_bit==4) {
					//motor control	
					cpc_cassette_motor_control(valor_bit);
				}
			}
				
                break;


        }

}



void cpc_out_port_crtc(z80_int puerto,z80_byte value)
{

	z80_byte puerto_h=(puerto>>8)&0xFF;

	z80_byte funcion=puerto_h&3;

	switch (funcion) {
		case 0:
			cpc_crtc_last_selected_register=value&31;
			//printf ("Select 6845 register %d\n",cpc_crtc_last_selected_register);
		break;

		case 1:
			//printf ("Write 6845 register %d with 0x%02X\n",cpc_crtc_last_selected_register,value);
			cpc_crtc_registers[cpc_crtc_last_selected_register]=value;
		break;

		case 2:
		break;

		case 3:
		break;
	}

}


void cpc_splash_videomode_change(void) {

	z80_byte modo_video=cpc_gate_registers[2] &3;

        char mensaje[32*24];

        switch (modo_video) {
                case 0:
                        sprintf (mensaje,"Setting screen mode 0, 160x200, 16 colours");
                break;

                case 1:
                        sprintf (mensaje,"Setting screen mode 1, 320x200, 4 colours");
                break;

                case 2:
                        sprintf (mensaje,"Setting screen mode 2, 640x200, 2 colours");
                break;

                case 3:
                        sprintf (mensaje,"Setting screen mode 3, 160x200, 4 colours (undocumented)");
                break;

                default:
                        //Esto no deberia suceder nunca
                        sprintf (mensaje,"Setting unknown vide mode");
                break;

        }

        screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,mensaje);

}
