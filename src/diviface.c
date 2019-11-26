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
#include <dirent.h>


#include "diviface.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "menu.h"
#include "screen.h"
#include "operaciones.h"
#include "zxuno.h"
#include "contend.h"
#include "prism.h"
#include "tbblue.h"
#include "tsconf.h"

z80_bit diviface_enabled={0};

//Si ha entrado pagina divmmc de manera automatica
z80_bit diviface_paginacion_automatica_activa={0};


z80_byte diviface_control_register=0;

z80_bit diviface_allow_automatic_paging={0};


//Si esta la paginacion manual de diviface activa
//z80_bit diviface_paginacion_manual_activa={0};

//Donde empieza la memoria asignada y por tanto la rom
z80_byte *diviface_memory_pointer;

//Donde empieza la ram. En zxuno y spectrum, va justo despues de el firmware (DIVIFACE_FIRMWARE_ALLOCATED_KB)
//En caso de tbblue, va en otro sitio
z80_byte *diviface_ram_memory_pointer;


z80_bit diviface_eprom_write_jumper={0};



//puntero a cpu core normal sin diviface
//void (*cpu_core_loop_no_diviface) (void);

int diviface_current_ram_memory_bits=4; //Por defecto 128 KB
/*
-using 2 bits: 32 kb
-using 3 bits: 64 kb
-using 4 bits: 128 kb (default)
-using 5 bits: 256 kb
-using 6 bits: 512 kb
*/

int get_diviface_ram_mask(void)
{
	int value_mask;

	if (diviface_current_ram_memory_bits<2 || diviface_current_ram_memory_bits>6)
		cpu_panic("Invalid bit mask value for diviface");

	value_mask=(1<<diviface_current_ram_memory_bits)-1;
	//Ej: si vale 2, (1<<2)-1=3
	//si vale 3, (1<<3)-1=7
	//si vale 4, (1<<4)-1=15
	//etc


	//printf ("diviface_current_ram_memory_bits: %d value_mask: %d\n",diviface_current_ram_memory_bits,value_mask);

	return value_mask;
}

int get_diviface_total_ram(void)
{
	return (8<<diviface_current_ram_memory_bits);
}





        int diviface_salta_trap_antes=0;
        int diviface_salta_trap_despues=0;
	int diviface_salta_trap_despaginacion_despues=0;


//Core de cpu loop para hacer traps de cpu
void diviface_pre_opcode_fetch(void)
{

/*
Memory mapping could be invoked manually (by setting CONMEM), or automatically
(CPU has fetched opcode form an entry-point). Automatic mapping is active
only if EPROM/EEPROM is present (jumper EPROM is closed) or bit MAPRAM is set.
Automatic mapping occurs at the begining of refresh cycle after fetching
opcodes (M1 cycle) from 0000h, 0008h, 0038h, 0066h, 04c6h and 0562h. It's
also mapped instantly (100ns after /MREQ of that fetch is falling down) after
executing opcode from area 3d00..3dffh. Memory is automatically disconnected in
refresh cycle of the instruction fetch from so called off-area, which is
1ff8-1fffh.
*/



	diviface_salta_trap_antes=0;
	diviface_salta_trap_despues=0;
	diviface_salta_trap_despaginacion_despues=0;

	//Si en ZXUNO y DIVEN desactivado
	if (MACHINE_IS_ZXUNO_DIVEN_DISABLED) return;

	//Si en Prism y rom 0, no saltar
	if (MACHINE_IS_PRISM) {
		if (prism_rom_page==0) {
			//printf ("rom 0 en prism, no saltar diviface. pc=%d\n",reg_pc);
			return;
		}
	}

	//Si en tsconf y rom menu. Esto es un parchecillo y habria que ver si realmente funciona bien. TODO

	if (MACHINE_IS_TSCONF) {
		//printf ("antes\n");
		if ((tsconf_get_memconfig()&8)==0) {
			if (tsconf_get_rom_bank()==0) return;
		}
		//printf ("despues\n");
		//&& tsconf_in_system_rom() ) return;
	}




	if (diviface_allow_automatic_paging.v) {
	//Traps que paginan memoria y saltan despues de leer instruccion
	switch (reg_pc) {
		case 0x0000:
		case 0x0008:
		case 0x0038:
		case 0x0066:
		case 0x04c6:
		case 0x0562:
			if (diviface_paginacion_automatica_activa.v==0) diviface_salta_trap_despues=1;
		break;
	}


	//Traps que paginan memoria y saltan antes de leer instruccion
	if (reg_pc>=0x3d00 && reg_pc<=0x3dff) diviface_salta_trap_antes=1;

	//Traps que despaginan memoria antes de leer instruccion
	if (reg_pc>=0x1ff8 && reg_pc<=0x1fff && diviface_paginacion_automatica_activa.v) {
		//printf ("Saltado trap de despaginacion pc actual: %d\n",reg_pc);
		diviface_salta_trap_despaginacion_despues=1;
        }


	}


	if (diviface_salta_trap_antes && diviface_paginacion_automatica_activa.v==0) {
		//printf ("Saltado trap de paginacion antes pc actual: %d\n",reg_pc);
		diviface_paginacion_automatica_activa.v=1;
        }
}



void diviface_post_opcode_fetch(void)
{

	if (diviface_salta_trap_despues) {
		//printf ("Saltado trap de paginacion despues. pc actual: %d\n",reg_pc);
		diviface_paginacion_automatica_activa.v=1;
	}

	if (diviface_salta_trap_despaginacion_despues) {
		//printf ("Saltado trap de despaginacion despues. pc actual: %d\n",reg_pc);
		diviface_paginacion_automatica_activa.v=0;
	}


}



//Escritura de puerto de control divide/divmmc. Activar o desactivar paginacion solamente
void diviface_write_control_register(z80_byte value)
{

	//printf ("Escribiendo registro de control diviface valor: 0x%02X (antes: %02XH)\n",value,diviface_control_register);
	if (value&128) {
		//printf ("Activando paginacion\n");
		//diviface_paginacion_manual_activa.v=1;
	}

	else {
		//printf ("Desactivando paginacion\n");
		//diviface_paginacion_manual_activa.v=0;
	}

	diviface_control_register=value;
}


//Lectura de puerto de control divide/divmmc
z80_byte diviface_read_control_register(void)
{

	return diviface_control_register;
}

//Rutinas originales antes de cambiarlas
//void (*diviface_original_poke_byte)(z80_int dir,z80_byte valor);
//void (*diviface_original_poke_byte_no_time)(z80_int dir,z80_byte valor);
//z80_byte (*diviface_original_peek_byte)(z80_int dir);
//z80_byte (*diviface_original_peek_byte_no_time)(z80_int dir);

int diviface_nested_id_peek_byte;
int diviface_nested_id_peek_byte_no_time;
int diviface_nested_id_poke_byte;
int diviface_nested_id_poke_byte_no_time;



//Retorna puntero a direccion apuntada por dir, en caso de paginacion diviface activa y dir <4000h
z80_byte *diviface_return_memory_paged_pointer(z80_int dir)
{
/*


    7        6     5  4  3  2   1       0
[ CONMEM , MAPRAM, X, X, X, X, BANK1, BANK0 ]

So, when CONMEM is set, there is:
0000-1fffh - EEPROM/EPROM/NOTHING(if empty socket), and this area is
	     flash-writable if EPROM jumper is open.
2000-3fffh - 8k bank, selected by BANK 0..1 bits, always writable.

When MAPRAM is set, but CONMEM is zero, and entrypoint was reached:
0000-1fffh - Bank No.3, read-only
2000-3fffh - 8k bank, selected by BANK 0..1. If it's different from Bank No.3,
	     it's writable.

When MAPRAM is zero, CONMEM is zero, EPROM jumper is closed and entrypoint was
reached:
0000-1fffh - EEPROM/EPROM/NOTHING(if empty socket, so open jumper in this case),
	     read-only.
2000-3fffh - 8k bank, selected by BANK 0..1, always writable.
*/


	//De momento facil
	if (dir<=0x1fff) {

		if (diviface_control_register&128) {
			//Devolver eprom
			return &diviface_memory_pointer[dir];
		}

		else {

			if (diviface_control_register&64) {
				//printf ("Retornando direccion cuando MAPRAM activo\n");
				//pagina ram 3.
				int pagina=3;
				//int offset=DIVIFACE_FIRMWARE_ALLOCATED_KB*1024;
				int offset=0;
				offset +=dir;
				offset +=pagina*8192;
				return &diviface_ram_memory_pointer[offset];
			}

			else {
				//Devolver eprom
				return &diviface_memory_pointer[dir];
			}
		}
	}

	else {
		//de 2000h a 3fffh
		//Devolver pagina de memoria ram
		int pagina=diviface_control_register&get_diviface_ram_mask();

		int offset=0;

		//Nos ubicamos en la pagina
		offset +=pagina*8192;

		//Nos ubicamos en la direccion
		offset +=(dir-0x2000);
		return &diviface_ram_memory_pointer[offset];
	}

}


//Rutinas propias

//Si hay que hacer poke a memoria interna.
//Retorna 0 si no ha escrito en divmmc memoria. Cualquier cosa diferente de 0, es que ha escrito
//Nota: El valor de retorno solo se usa en chloe, para decirle que si ha escrito en memoria divmmc/ide no escriba en la memoria que haya por debajo,
//pues desde chloe mete memoria RAM dock en el segmento 0000-3fffh y provocaria escritura ahí, cuando no debe hacerlo si esta encima la divmmc
int diviface_poke_byte_to_internal_memory(z80_int dir,z80_byte valor)
{

	//Si en tbblue y escribiendo en memoria layer2, ignorar, dado que esa memoria layer2 en escritura se mapea en 0-3fffh
	if (MACHINE_IS_TBBLUE && tbblue_write_on_layer2() ) return 0;

	if ((diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0) {
  	return 0;
  }

  else {

	//Si la mmu del tbblue esta alterando esta zona de memoria
	if (MACHINE_IS_TBBLUE && dir<16384) {
		//Despues de salir de esta funcion siempre llama a escritura de memoria de la capa que haya por debajo
		//Por tanto solo hay que determinar si la memoria divmmc se debe escribir (valor en MMU 255),
		//Y si no se debe, simplemente hacer return de aqui
	        z80_byte reg_mmu_value=return_tbblue_mmu_segment(dir);
                if (reg_mmu_value!=255) return 0;
	}

		//Si poke a eprom o ram 3 read only.

  	if (dir<8192) {

			//Si poke a eprom cuando conmem=1
			if (diviface_control_register&128 && diviface_eprom_write_jumper.v) {
				debug_printf (VERBOSE_DEBUG,"Diviface eprom writing address: %04XH value: %02XH",dir,valor);
				//Escribir en eprom siempre que jumper eprom está permitiendolo
				z80_byte *puntero=diviface_return_memory_paged_pointer(dir);
				*puntero=valor;
				return 1;
			}

			//No escribir
			return 0;
		}

    if (dir<16384) {
    	z80_byte *puntero=diviface_return_memory_paged_pointer(dir);
			*puntero=valor;
			return 1;
    }
		return 0;

  }
}

z80_byte diviface_poke_byte_no_time(z80_int dir,z80_byte valor)
{
	diviface_poke_byte_to_internal_memory(dir,valor);

	debug_nested_poke_byte_no_time_call_previous(diviface_nested_id_poke_byte_no_time,dir,valor);

	//Para que no se queje el compilador
	return 0;
}

z80_byte diviface_poke_byte(z80_int dir,z80_byte valor)
{
	diviface_poke_byte_to_internal_memory(dir,valor);

	debug_nested_poke_byte_call_previous(diviface_nested_id_poke_byte,dir,valor);

	//Para que no se queje el compilador
	return 0;
}



z80_byte diviface_chloe_poke_byte_no_time(z80_int dir,z80_byte valor)
{
	//Si ya escribe en divmmc memoria, no escribir en el layer de debajo
	if (diviface_poke_byte_to_internal_memory(dir,valor)) return 0;

	debug_nested_poke_byte_no_time_call_previous(diviface_nested_id_poke_byte_no_time,dir,valor);

	//Para que no se queje el compilador
	return 0;
}

z80_byte diviface_chloe_poke_byte(z80_int dir,z80_byte valor)
{
	//Si ya escribe en divmmc memoria, no escribir en el layer de debajo
	if (diviface_poke_byte_to_internal_memory(dir,valor)) return 0;

	debug_nested_poke_byte_call_previous(diviface_nested_id_poke_byte,dir,valor);

	//Para que no se queje el compilador
	return 0;
}

z80_byte *diviface_return_tbblue_memory_pointer(z80_int dir)
{
	z80_byte *puntero;
	puntero=tbblue_return_segment_memory(dir);

        dir = dir & 8191;
        puntero=puntero+dir;

	return puntero;
}

z80_byte diviface_peek_byte_to_internal_memory(z80_int dir)
{
	//printf ("returning diviface internal memory address from diviface_peek_byte_no_time %XH\n",dir);
	z80_byte *puntero=diviface_return_memory_paged_pointer(dir);
	if (MACHINE_IS_TBBLUE) {
		//Ver si hay mapeo de MMU diferente a la de por defecto
		/*
		
(R/W) 0x50 (80) => MMU slot 0
  bits 7-0 = Set a Spectrum RAM page at position 0x0000 to 0x1fff
  (Reset to 255 after a reset)
  Pages can be from 0 to 223 on a full expanded Next. 
  A 255 value remove the RAM page and map the current ROM

(R/W) 0x51 (81) => MMU slot 1
  bits 7-0 = Set a Spectrum RAM page at position 0x2000 to 0x3fff
  (Reset to 255 after a reset)
  Pages can be from 0 to 223 on a full expanded Next. 
  A 255 value remove the RAM page and map the current ROM
		*/
		z80_byte reg_mmu_value=return_tbblue_mmu_segment(dir);
		if (reg_mmu_value!=255) {
			//Mapeo diferente
            puntero=diviface_return_tbblue_memory_pointer(dir);
		}
	}
	return *puntero;
}

z80_byte diviface_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(diviface_nested_id_peek_byte_no_time,dir);

	if ((diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0) {
		//printf ("returning NON diviface internal memory address from diviface_peek_byte_no_time %XH\n",dir);
		return valor_leido;
	}
	else {

		if (dir<16384) {
			return diviface_peek_byte_to_internal_memory(dir);
			//printf ("returning diviface internal memory address from diviface_peek_byte_no_time %XH\n",dir);
      //z80_byte *puntero=diviface_return_memory_paged_pointer(dir);
      //return *puntero;
    }

		else return valor_leido;

	}
}

z80_byte diviface_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_call_previous(diviface_nested_id_peek_byte,dir);

  if ((diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0) {
	  //printf ("returning NON diviface internal memory address from diviface_peek_byte %XH\n",dir);
    return valor_leido;
  }

	else {
  	if (dir<16384) {
		return diviface_peek_byte_to_internal_memory(dir);
		//printf ("returning diviface internal memory address from diviface_peek_byte %XH\n",dir);
      //z80_byte *puntero=diviface_return_memory_paged_pointer(dir);
      //return *puntero;
    }

		else return valor_leido;
  }

}

//Establecer rutinas propias
void diviface_set_peek_poke_functions(void)
{

	int activar=0;

	//Ver si ya no estaban activas
	if (poke_byte!=poke_byte_nested_handler) {
		debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is not enabled calling diviface_set_peek_poke_functions. Enabling");
		activar=1;
	}

	else {
		//Esta activo el handler. Vamos a ver si esta activo el diviface dentro
		if (debug_nested_find_function_name(nested_list_poke_byte,"Diviface poke_byte")==NULL) {
			//No estaba en la lista
			activar=1;
			debug_printf (VERBOSE_DEBUG,"poke_byte_nested_handler is enabled but not found Diviface poke_byte. Enabling");
		}

	}


	if (activar) {
		debug_printf (VERBOSE_DEBUG,"Setting DIVIFACE poke / peek functions");

				if (MACHINE_IS_CHLOE) {
        	diviface_nested_id_poke_byte=debug_nested_poke_byte_add(diviface_chloe_poke_byte,"Diviface poke_byte");
        	diviface_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(diviface_chloe_poke_byte_no_time,"Diviface poke_byte_no_time");
				}

				else {
        	diviface_nested_id_poke_byte=debug_nested_poke_byte_add(diviface_poke_byte,"Diviface poke_byte");
        	diviface_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(diviface_poke_byte_no_time,"Diviface poke_byte_no_time");
				}


        diviface_nested_id_peek_byte=debug_nested_peek_byte_add(diviface_peek_byte,"Diviface peek_byte");
        diviface_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(diviface_peek_byte_no_time,"Diviface peek_byte_no_time");

	}
}

//Restaurar rutinas de diviface
void diviface_restore_peek_poke_functions(void)
{
		debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before DIVIFACE");
                //poke_byte=diviface_original_poke_byte;
                //poke_byte_no_time=diviface_original_poke_byte_no_time;
                //peek_byte=diviface_original_peek_byte;
                //peek_byte_no_time=diviface_original_peek_byte_no_time;

        debug_nested_poke_byte_del(diviface_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(diviface_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(diviface_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(diviface_nested_id_peek_byte_no_time);

}


//Retorna 0 si ok
int diviface_load_firmware(char *romfile)
{
        FILE *ptr_diviface_romfile;
        int leidos=0;

	debug_printf (VERBOSE_INFO,"Loading diviface firmware: %s",romfile);

	open_sharedfile(romfile,&ptr_diviface_romfile);


        if (ptr_diviface_romfile!=NULL) {

                leidos=fread(diviface_memory_pointer,1,DIVIFACE_FIRMWARE_KB*1024,ptr_diviface_romfile);
                fclose(ptr_diviface_romfile);

        }



        if (leidos!=DIVIFACE_FIRMWARE_KB*1024 || ptr_diviface_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading DIVIFACE firmware");
		//Lo desactivamos asi porque el disable hace otras cosas, como cambiar el core loop, que no queremos
		diviface_enabled.v=0;
		return 1;
        }

	return 0;
}






void diviface_alloc_memory(void)
{
	int size=(DIVIFACE_FIRMWARE_ALLOCATED_KB+DIVIFACE_RAM_ALLOCATED_KB)*1024;

	debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for diviface emulation",size/1024);

        diviface_memory_pointer=malloc(size);
        if (diviface_memory_pointer==NULL) {
                cpu_panic ("No enough memory for diviface emulation");
        }


}

void diviface_enable(char *romfile)
{

	debug_printf (VERBOSE_INFO,"Enabling diviface");

	//Asignar memoria, si es que no estamos en modo ZXUNO
	//Cargar firmware, si es que no estamos en modo ZXUNO

	if (MACHINE_IS_ZXUNO) {
		//Pagina 12 del ZX-Uno
		diviface_memory_pointer=zxuno_sram_mem_table_new[12];
		diviface_ram_memory_pointer=&diviface_memory_pointer[DIVIFACE_FIRMWARE_ALLOCATED_KB*1024];

	}

	else if (MACHINE_IS_TBBLUE) {
		//Pagina 0 del tbblue
		//diviface_memory_pointer=memoria_spectrum;

		diviface_memory_pointer=&memoria_spectrum[0x010000];   //Donde va la ROM de diviface, normalmente esxdos
		diviface_ram_memory_pointer=&memoria_spectrum[0x020000];   //Donde va la ram de diviface
	}

	else {

		//Memoria asignada para diviface:
		//Primeros 64kb: solo se usan 8kb para el firmware
		//Siguientes 128kb: paginas ram para diviface
		//Este modelo de memoria es compatible con zxuno

		diviface_alloc_memory();

		if (diviface_load_firmware(romfile)) return;

		diviface_ram_memory_pointer=&diviface_memory_pointer[DIVIFACE_FIRMWARE_ALLOCATED_KB*1024];

	}


	//Cambiar poke, peek funciones
	diviface_set_peek_poke_functions();


	diviface_control_register=0;
	//diviface_paginacion_manual_activa.v=0;
	diviface_control_register&=(255-128);
	diviface_paginacion_automatica_activa.v=0;
	diviface_enabled.v=1;
	diviface_allow_automatic_paging.v=1;

}

void diviface_disable(void)
{

	if (diviface_enabled.v==0) return;

	debug_printf (VERBOSE_INFO,"Disabling diviface");


	//Restaurar poke, peek funciones
	diviface_restore_peek_poke_functions();


        diviface_enabled.v=0;
				diviface_allow_automatic_paging.v=0;
}
