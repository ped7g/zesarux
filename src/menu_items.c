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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


#include "menu.h"
#include "menu_items.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "zx8081.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "snap.h"
#include "operaciones.h"
#include "disassemble.h"
#include "utils.h"
#include "contend.h"
#include "joystick.h"
#include "ula.h"
#include "printers.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "z88.h"
#include "ulaplus.h"
#include "autoselectoptions.h"
#include "zxuno.h"
#include "charset.h"
#include "chardetect.h"
#include "textspeech.h"
#include "mmc.h"
#include "ide.h"
#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "zxpand.h"
#include "spectra.h"
#include "spritechip.h"
#include "jupiterace.h"
#include "timex.h"
#include "chloe.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "atomlite.h"
#include "if1.h"
#include "pd765.h"
#include "tbblue.h"
#include "dandanator.h"
#include "superupgrade.h"
#include "m68k.h"
#include "remote.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "scmp.h"
#include "esxdos_handler.h"
#include "tsconf.h"
#include "kartusho.h"
#include "ifrom.h"
#include "spritefinder.h"
#include "snap_spg.h"
#include "betadisk.h"
#include "tape_tzx.h" 
#include "snap_zsf.h"
#include "compileoptions.h"
#include "settings.h"
#include "datagear.h"
#include "assemble.h"
#include "expression_parser.h"
#include "uartbridge.h"
#include "zeng.h"
#include "network.h"
#include "stats.h"


#ifdef COMPILE_ALSA
#include "audioalsa.h"
#endif

 
#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
#endif


#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif

#ifdef COMPILE_CURSESW
	#include "cursesw_ext.h"
#endif

//
// Archivo para entradas de menu, excluyendo funciones auxiliares de soporte de menu
// Las funciones auxiliares de menu estan en menu.c
// Aunque aun falta mucho por mover, la mayoria de entradas de menu siguen en menu.c y habria que moverlas aqui
//


//Opciones seleccionadas para cada menu
int debug_pok_file_opcion_seleccionada=0;
int poke_opcion_seleccionada=0; 
int settings_debug_opcion_seleccionada=0;
int change_audio_driver_opcion_seleccionada=0;
int settings_audio_opcion_seleccionada=0;
int mem_breakpoints_opcion_seleccionada=0;
int audio_new_waveform_opcion_seleccionada=0;
int debug_new_visualmem_opcion_seleccionada=0;
int audio_new_ayplayer_opcion_seleccionada=0;
int osd_adventure_keyboard_opcion_seleccionada=0;
int debug_tsconf_dma_opcion_seleccionada=0;
int tsconf_layer_settings_opcion_seleccionada=0;
int cpu_settings_opcion_seleccionada=0;
int textdrivers_settings_opcion_seleccionada=0;
int settings_display_opcion_seleccionada=0;
int cpu_stats_opcion_seleccionada=0;
int menu_tbblue_hardware_id_opcion_seleccionada=0;
int ext_desktop_settings_opcion_seleccionada=0;
int cpu_transaction_log_opcion_seleccionada=0;
int daad_tipo_mensaje_opcion_seleccionada=0;
int watches_opcion_seleccionada=0;
int breakpoints_opcion_seleccionada=0;
int menu_watches_opcion_seleccionada=0;
int record_mid_opcion_seleccionada=0;


int direct_midi_output_opcion_seleccionada=0;


int ay_mixer_opcion_seleccionada=0;
int uartbridge_opcion_seleccionada=0;
int network_opcion_seleccionada=0;
int zeng_opcion_seleccionada=0;
int zx81_online_browser_opcion_seleccionada=0;
int online_browse_zx81_letter_opcion_seleccionada=0;
int settings_statistics_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


//Ultima direccion pokeada
int last_debug_poke_dir=16384; 

//aofile. aofilename apuntara aqui
char aofilename_file[PATH_MAX];

char last_ay_file[PATH_MAX]="";

//vofile. vofilename apuntara aqui
char vofilename_file[PATH_MAX];


void menu_mem_breakpoints_edit(MENU_ITEM_PARAMETERS)
{


        int brkp_type,dir;

        char string_type[4];
        char string_dir[10];

        strcpy (string_dir,"0");

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }				

        strcpy (string_type,"0");

        menu_ventana_scanf("Type (1:RD,2:WR,3:RW)",string_type,4);

        brkp_type=parse_string_to_number(string_type);

        if (brkp_type<0 || brkp_type>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",brkp_type);
                return;
        }

	debug_set_mem_breakpoint(dir,brkp_type);
	//mem_breakpoint_array[dir]=brkp_type;
	

}

void menu_mem_breakpoints_list(MENU_ITEM_PARAMETERS)
{

        //int index_find;
		int index_buffer;

        char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

        //margen suficiente para que quepa una linea
        //direccion+salto linea+codigo 0
        char buf_linea[33];

        index_buffer=0;

        int encontrados=0;

        int salir=0;

		int i;

        for (i=0;i<65536 && salir==0;i++) {
			z80_byte tipo=mem_breakpoint_array[i];
			if (tipo) {
				if (tipo<MAX_MEM_BREAKPOINT_TYPES) {
					sprintf (buf_linea,"%04XH : %s\n",i,mem_breakpoint_types_strings[tipo]);
				}
				else {
					sprintf (buf_linea,"%04XH : Unknown (%d)\n",i,tipo);
				}

				sprintf (&results_buffer[index_buffer],"%s\n",buf_linea);
                index_buffer +=strlen(buf_linea);
                encontrados++;
                

                //controlar maximo
                //33 bytes de margen
                if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-33) {
                        debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first %d",encontrados);
                        //forzar salir
                        salir=1;
                }
			}

        }

        results_buffer[index_buffer]=0;

        menu_generic_message("List Memory Breakpoints",results_buffer);
}

void menu_mem_breakpoints_clear(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear Mem breakpoints")) {
		clear_mem_breakpoints();
		menu_generic_message("Clear Mem breakpoints","OK. All memory breakpoints cleared");
	}
}


void menu_clear_all_breakpoints(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear breakpoints")) {
		init_breakpoints_table();
		menu_generic_message("Clear breakpoints","OK. All breakpoints cleared");
	}
}

void menu_mem_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_mem_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_edit,NULL,"~~Edit Breakpoint");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Edit Breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Edit Breakpoints");

		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_list,NULL,"~~List breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'l');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"List breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"List enabled memory breakpoints");


		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_clear,NULL,"~~Clear breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'c');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Clear all memory breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Clear all memory breakpoints");


                menu_add_item_menu(array_menu_mem_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_mem_breakpoints);
                retorno_menu=menu_dibuja_menu(&mem_breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_mem_breakpoints,"Memory Breakpoints" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


 
void menu_debug_poke(MENU_ITEM_PARAMETERS)
{

        int valor_poke,dir,veces;

        char string_poke[4];
        char string_dir[10];
	char string_veces[6];

        sprintf (string_dir,"%XH",last_debug_poke_dir);

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        /*if ( (dir<0 || dir>65535) && MACHINE_IS_SPECTRUM) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }*/

				last_debug_poke_dir=dir;

        sprintf (string_poke,"0");

        menu_ventana_scanf("Poke Value",string_poke,4);

        valor_poke=parse_string_to_number(string_poke);

        if (valor_poke<0 || valor_poke>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",valor_poke);
                return;
        }


	sprintf (string_veces,"1");

	menu_ventana_scanf("How many bytes?",string_veces,6);

	veces=parse_string_to_number(string_veces);

	if (veces<1 || veces>65536) {
                debug_printf (VERBOSE_ERR,"Invalid quantity %d",veces);
		return;
	}


	for (;veces;veces--,dir++) {

	        //poke_byte_no_time(dir,valor_poke);
		//poke_byte_z80_moto(dir,valor_poke);
		menu_debug_write_mapped_byte(dir,valor_poke);

	}

}



void menu_debug_poke_pok_file(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="pok";
        filtros[1]=0;

	char pokfile[PATH_MAX];

        int ret;

        ret=menu_filesel("Select POK File",filtros,pokfile);

	//contenido
	//MAX_LINEAS_POK_FILE es maximo de lineas de pok file
	//normalmente la tabla de pokes sera menor que el numero de lineas en el archivo .pok
	struct s_pokfile tabla_pokes[MAX_LINEAS_POK_FILE];

	//punteros
	struct s_pokfile *punteros_pokes[MAX_LINEAS_POK_FILE];

	int i;
	for (i=0;i<MAX_LINEAS_POK_FILE;i++) punteros_pokes[i]=&tabla_pokes[i];


        if (ret==1) {

                //cls_menu_overlay();
		int total=util_parse_pok_file(pokfile,punteros_pokes);

		if (total<1) {
			debug_printf (VERBOSE_ERR,"Error parsing POK file");
			return;
		}

		int j;
		for (j=0;j<total;j++) {
			debug_printf (VERBOSE_DEBUG,"menu poke index %d text %s bank %d address %d value %d value_orig %d",
				punteros_pokes[j]->indice_accion,
				punteros_pokes[j]->texto,
				punteros_pokes[j]->banco,
				punteros_pokes[j]->direccion,
				punteros_pokes[j]->valor,
				punteros_pokes[j]->valor_orig);
		}


		//Meter cada poke en un menu




        menu_item *array_menu_debug_pok_file;
        menu_item item_seleccionado;
        int retorno_menu;
	//Resetear siempre ultima linea = 0
	debug_pok_file_opcion_seleccionada=0;

	//temporal para mostrar todos los caracteres 0-255
	//int temp_conta=1;

        do {



		//Meter primer item de menu
		//truncar texto a 28 caracteres si excede de eso
		if (strlen(punteros_pokes[0]->texto)>28) punteros_pokes[0]->texto[28]=0;
                menu_add_item_menu_inicial_format(&array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[0]->texto);


		//Luego recorrer array de pokes y cuando el numero de poke se incrementa, agregar
		int poke_anterior=0;

		int total_elementos=1;

		for (j=1;j<total;j++) {
			if (punteros_pokes[j]->indice_accion!=poke_anterior) {

				//temp para mostrar todos los caracteres 0-255
				//int kk;
				//for (kk=0;kk<strlen(punteros_pokes[j]->texto);kk++) {
				//	punteros_pokes[j]->texto[kk]=temp_conta++;
				//	if (temp_conta==256) temp_conta=1;
				//}

				poke_anterior=punteros_pokes[j]->indice_accion;
				//truncar texto a 28 caracteres si excede de eso
				if (strlen(punteros_pokes[j]->texto)>28) punteros_pokes[j]->texto[28]=0;
				menu_add_item_menu_format(array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[j]->texto);

				total_elementos++;
				if (total_elementos==20) {
					debug_printf (VERBOSE_DEBUG,"Too many pokes to show on Window. Showing only first 20");
					menu_warn_message("Too many pokes to show on Window. Showing only first 20");
					break;
				}


			}
		}



                menu_add_item_menu(array_menu_debug_pok_file,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_debug_pok_file,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_debug_pok_file);

                retorno_menu=menu_dibuja_menu(&debug_pok_file_opcion_seleccionada,&item_seleccionado,array_menu_debug_pok_file,"Select Poke" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion

			//Hacer poke sabiendo la linea seleccionada. Desde ahi, ejecutar todos los pokes de dicha accion
			debug_printf (VERBOSE_DEBUG,"Doing poke/s from line %d",debug_pok_file_opcion_seleccionada);

			z80_byte banco;
			z80_int direccion;
			z80_byte valor;

			//buscar indice_accion
			int result_poke=0;
			for (j=0;j<total && result_poke==0;j++) {

				debug_printf (VERBOSE_DEBUG,"index %d looking %d current %d",j,debug_pok_file_opcion_seleccionada,punteros_pokes[j]->indice_accion);

				if (punteros_pokes[j]->indice_accion==debug_pok_file_opcion_seleccionada) {
					banco=punteros_pokes[j]->banco;
					direccion=punteros_pokes[j]->direccion;
					valor=punteros_pokes[j]->valor;
					debug_printf (VERBOSE_DEBUG,"Doing poke bank %d address %d value %d",banco,direccion,valor);
					result_poke=util_poke(banco,direccion,valor);
				}


                      

			}
			if (result_poke==0) menu_generic_message("Poke","OK. Poke applied");
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




        }

}





void menu_poke(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_poke;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial_format(&array_menu_poke,MENU_OPCION_NORMAL,menu_debug_poke,NULL,"~~Poke");
                menu_add_item_menu_shortcut(array_menu_poke,'p');
                menu_add_item_menu_tooltip(array_menu_poke,"Poke address");
                menu_add_item_menu_ayuda(array_menu_poke,"Poke address for infinite lives, etc... This item follows active memory zone. "
					"You can also poke on read-only memory, depending on the current memory zone");

		//No tiene sentido pues se puede usar las memory zones para esto
		/*if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
			menu_add_item_menu(array_menu_poke,"Poke 128~~k mode",MENU_OPCION_NORMAL,menu_debug_poke_128k,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'k');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke bank & address");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke bank & address");
		}*/

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu(array_menu_poke,"Poke from .POK ~~file",MENU_OPCION_NORMAL,menu_debug_poke_pok_file,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'f');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke reading .POK file");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke reading .POK file");
		}


                menu_add_item_menu(array_menu_poke,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_poke,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_poke);

                retorno_menu=menu_dibuja_menu(&poke_opcion_seleccionada,&item_seleccionado,array_menu_poke,"Poke" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_debug_registers_console(MENU_ITEM_PARAMETERS) {
	debug_registers ^=1;
}

void menu_debug_configuration_stepover(MENU_ITEM_PARAMETERS)
{
	//debug_core_evitamos_inter.v ^=1;
	remote_debug_settings ^=32;
}


void menu_breakpoints_condition_behaviour(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_cond_behaviour.v ^=1;
}

void menu_debug_configuration_remoteproto_port(MENU_ITEM_PARAMETERS)
{
	char string_port[6];
	int port;

	sprintf (string_port,"%d",remote_protocol_port);

	menu_ventana_scanf("Port",string_port,6);

	if (string_port[0]==0) return;

	else {
			port=parse_string_to_number(string_port);

			if (port<1 || port>65535) {
								debug_printf (VERBOSE_ERR,"Invalid port %d",port);
								return;
			}


			end_remote_protocol();
			remote_protocol_port=port;
			init_remote_protocol();
	}

}

void menu_debug_shows_invalid_opcode(MENU_ITEM_PARAMETERS)
{
	debug_shows_invalid_opcode.v ^=1;
}

void menu_debug_settings_show_fired_breakpoint(MENU_ITEM_PARAMETERS)
{
	debug_show_fired_breakpoints_type++;
	if (debug_show_fired_breakpoints_type==3) debug_show_fired_breakpoints_type=0;
}

void menu_debug_settings_show_screen(MENU_ITEM_PARAMETERS)
{
	debug_settings_show_screen.v ^=1;
}
void menu_debug_settings_show_scanline(MENU_ITEM_PARAMETERS)
{
	menu_debug_registers_if_showscan.v ^=1;
}

void menu_debug_configuration_remoteproto(MENU_ITEM_PARAMETERS)
{
	if (remote_protocol_enabled.v) {
		end_remote_protocol();
		remote_protocol_enabled.v=0;
	}

	else {
		enable_and_init_remote_protocol();
	}
}


void menu_debug_verbose(MENU_ITEM_PARAMETERS)
{
	verbose_level++;
	if (verbose_level>4) verbose_level=0;
}

void menu_zesarux_zxi_hardware_debug_file(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

    filtros[0]="";
    filtros[1]=0;


    if (menu_filesel("Select Debug File",filtros,zesarux_zxi_hardware_debug_file)==1) {
    	//Ver si archivo existe y preguntar
		if (si_existe_archivo(zesarux_zxi_hardware_debug_file)) {
            if (menu_confirm_yesno_texto("File exists","Append?")==0) {
				zesarux_zxi_hardware_debug_file[0]=0;
				return;
			}
        }

    }

	else zesarux_zxi_hardware_debug_file[0]=0;

}

void menu_hardware_debug_port(MENU_ITEM_PARAMETERS)
{
	hardware_debug_port.v ^=1;
}


void menu_debug_settings_dump_snap_panic(MENU_ITEM_PARAMETERS)
{
	debug_dump_zsf_on_cpu_panic.v ^=1;
}

void menu_debug_verbose_always_console(MENU_ITEM_PARAMETERS)
{
	debug_always_show_messages_in_console.v ^=1;
}

//menu debug settings
void menu_settings_debug(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_debug;
        menu_item item_seleccionado;
	int retorno_menu;
        do {


      char string_zesarux_zxi_hardware_debug_file_shown[18];
      


		menu_add_item_menu_inicial_format(&array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_registers_console,NULL,"[%c] Show r~~egisters in console",(debug_registers==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_debug,'e');

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_shows_invalid_opcode,NULL,"[%c] Show ~~invalid opcode",
			(debug_shows_invalid_opcode.v ? 'X' : ' ') ); 
		menu_add_item_menu_shortcut(array_menu_settings_debug,'i');
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes). "
								"A message will be shown on console, when verbose level is 2 or higher");



       


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_configuration_stepover,NULL,"[%c] Step ~~over interrupt",(remote_debug_settings&32 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'o');





		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_screen,NULL,"[%c] Show display on debug",
			( debug_settings_show_screen.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_scanline,NULL,"[%c] Shows electron on debug",
			( menu_debug_registers_if_showscan.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose,NULL,"[%d] Verbose ~~level",verbose_level);
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');		

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose_always_console,NULL,"[%c] Always verbose console",
			( debug_always_show_messages_in_console.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Always show messages in console (using simple printf) additionally to the default video driver");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Always show messages in console (using simple printf) additionally to the default video driver. Interesting in some cases as curses, aa or caca video drivers");
				


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_dump_snap_panic,NULL,"[%c] Dump snapshot on panic",
			( debug_dump_zsf_on_cpu_panic.v ? 'X' : ' ') );	
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Dump .zsf snapshot when a cpu panic is fired");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Dump .zsf snapshot when a cpu panic is fired");	




		menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);		


#ifdef USE_PTHREADS
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto,NULL,"[%c] ZRCP Remote protocol",(remote_protocol_enabled.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'r');

		if (remote_protocol_enabled.v) {
			menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto_port,NULL,"[%d] Remote protocol ~~port",remote_protocol_port );
			menu_add_item_menu_tooltip(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_shortcut(array_menu_settings_debug,'p');
		}

#endif


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_hardware_debug_port,NULL,"[%c] Hardware ~~debug ports",(hardware_debug_port.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If hardware debug ports are enabled");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"These ports are used to interact with ZEsarUX, for example showing a ASCII character on console, read ZEsarUX version, etc. "
														"Read file extras/docs/zesarux_zxi_registers.txt for more information");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'d');


		if (hardware_debug_port.v) {
			menu_tape_settings_trunc_name(zesarux_zxi_hardware_debug_file,string_zesarux_zxi_hardware_debug_file_shown,18);
        	menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_zesarux_zxi_hardware_debug_file,NULL,"Byte ~~file [%s]",string_zesarux_zxi_hardware_debug_file_shown);
			menu_add_item_menu_tooltip(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");		
			menu_add_item_menu_ayuda(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");	
			menu_add_item_menu_shortcut(array_menu_settings_debug,'f');							
		}


		menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_breakpoints_condition_behaviour,NULL,"~~Breakp. behaviour [%s]",(debug_breakpoints_cond_behaviour.v ? "On Change" : "Always") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'b');


		char show_fired_breakpoint_type[30];
		if (debug_show_fired_breakpoints_type==0) strcpy(show_fired_breakpoint_type,"Always");
		else if (debug_show_fired_breakpoints_type==1) strcpy(show_fired_breakpoint_type,"NoPC");
		else strcpy(show_fired_breakpoint_type,"Never");																	//						   OnlyNonPC
																															//  01234567890123456789012345678901
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_fired_breakpoint,NULL,"Show fired breakpoint [%s]",show_fired_breakpoint_type);
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired. "
								"Possible values:\n"
								"Always: always shows the condition\n"
								"NoPC: only shows conditions that are not like PC=XXXX\n"
								"Never: never shows conditions\n" );			


                menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_debug,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_debug);

                retorno_menu=menu_dibuja_menu(&settings_debug_opcion_seleccionada,&item_seleccionado,array_menu_settings_debug,"Debug Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



int num_menu_audio_driver;
int num_previo_menu_audio_driver;


//Determina cual es el audio driver actual
void menu_change_audio_driver_get(void)
{
        int i;
        for (i=0;i<num_audio_driver_array;i++) {
		//printf ("actual: %s buscado: %s indice: %d\n",audio_driver_name,audio_driver_array[i].driver_name,i);
                if (!strcmp(audio_driver_name,audio_driver_array[i].driver_name)) {
                        num_menu_audio_driver=i;
                        num_previo_menu_audio_driver=i;
			return;
                }

        }

}


void menu_change_audio_driver_change(MENU_ITEM_PARAMETERS)
{
        num_menu_audio_driver++;
        if (num_menu_audio_driver==num_audio_driver_array) num_menu_audio_driver=0;
}

void menu_change_audio_driver_apply(MENU_ITEM_PARAMETERS)
{

	audio_end();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=audio_driver_array[num_menu_audio_driver].funcion_init;
        funcion_set=audio_driver_array[num_menu_audio_driver].funcion_set;
                if ( (funcion_init()) ==0) {
                        funcion_set();
			menu_generic_message_splash("Apply Driver","OK. Driver applied");
			salir_todos_menus=1;
                }

                else {
                        debug_printf(VERBOSE_ERR,"Can not set audio driver. Restoring to previous driver %s",audio_driver_name);
			menu_change_audio_driver_get();

                        //Restaurar audio driver
                        funcion_init=audio_driver_array[num_previo_menu_audio_driver].funcion_init;
                        funcion_set=audio_driver_array[num_previo_menu_audio_driver].funcion_set;

                        funcion_init();
                        funcion_set();
                }



}


void menu_change_audio_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_audio_driver;
        menu_item item_seleccionado;
        int retorno_menu;

       	menu_change_audio_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_change,NULL,"Audio Driver: %s",audio_driver_array[num_menu_audio_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_audio_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_audio_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_audio_driver);

                retorno_menu=menu_dibuja_menu(&change_audio_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_audio_driver,"Change Audio Driver" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}






int menu_cond_ay_chip(void)
{
	return ay_chip_present.v;
}


void menu_audio_beep_filter_on_rom_save(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_on_rom_save.v ^=1;
}


void menu_audio_beep_alter_volume(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_alter_volume.v ^=1;
}


void menu_audio_beep_volume(MENU_ITEM_PARAMETERS)
{

        char string_vol[4];

        sprintf (string_vol,"%d",output_beep_filter_volume);


        menu_ventana_scanf("Volume (0-127)",string_vol,4);

        int v=parse_string_to_number(string_vol);

        if (v>127 || v<0) {
                debug_printf (VERBOSE_ERR,"Invalid volume value");
                return;
        }

        output_beep_filter_volume=v;
}

void menu_audio_beeper_real (MENU_ITEM_PARAMETERS)
{
	beeper_real_enabled ^=1;
}

void menu_audio_volume(MENU_ITEM_PARAMETERS)
{
        char string_perc[4];

        sprintf (string_perc,"%d",audiovolume);


        menu_ventana_scanf("Volume in %",string_perc,4);

        int v=parse_string_to_number(string_perc);

	if (v>100 || v<0) {
		debug_printf (VERBOSE_ERR,"Invalid volume value");
		return;
	}

	audiovolume=v;
}

void menu_audio_ay_chip(MENU_ITEM_PARAMETERS)
{
	ay_chip_present.v^=1;
}

void menu_audio_ay_chip_autoenable(MENU_ITEM_PARAMETERS)
{
	autoenable_ay_chip.v^=1;
}



void menu_audio_sound_zx8081(MENU_ITEM_PARAMETERS)
{
	zx8081_vsync_sound.v^=1;
}

void menu_audio_zx8081_detect_vsync_sound(MENU_ITEM_PARAMETERS)
{
	zx8081_detect_vsync_sound.v ^=1;
}



void menu_setting_ay_piano_grafico(MENU_ITEM_PARAMETERS)
{
	setting_mostrar_ay_piano_grafico.v ^=1;
}


void menu_aofile_insert(MENU_ITEM_PARAMETERS)
{

	if (aofile_inserted.v==0) {
		init_aofile();

		//Si todo ha ido bien
		if (aofile_inserted.v) {
			menu_generic_message_format("File information","%s\n%s\n\n%s",
			last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_audio,last_message_helper_aofile_vofile_util);
		}

	}

        else if (aofile_inserted.v==1) {
                close_aofile();
        }

}

int menu_aofile_cond(void)
{
	if (aofilename!=NULL) return 1;
	else return 0;
}

void menu_aofile(MENU_ITEM_PARAMETERS)
{

	aofile_inserted.v=0;


        char *filtros[3];

#ifdef USE_SNDFILE
        filtros[0]="rwa";
        filtros[1]="wav";
        filtros[2]=0;
#else
        filtros[0]="rwa";
        filtros[1]=0;
#endif


        if (menu_filesel("Select Audio File",filtros,aofilename_file)==1) {

       	        if (si_existe_archivo(aofilename_file)) {

               	        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
				aofilename=NULL;
				return;
			}

       	        }

                aofilename=aofilename_file;


        }

	else {
		aofilename=NULL;
	}


}




/*void menu_audio_audiodac(MENU_ITEM_PARAMETERS)
{
	audiodac_enabled.v ^=1;
}*/

void menu_audio_audiodac_type(MENU_ITEM_PARAMETERS)
{
	if (audiodac_enabled.v==0) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
	}

	else {
		audiodac_selected_type++;
		if (audiodac_selected_type==MAX_AUDIODAC_TYPES) {
			audiodac_selected_type=0;
			audiodac_enabled.v=0;
		}
	}
}

void menu_audio_audiodac_set_port(MENU_ITEM_PARAMETERS)
{
	char string_port[4];

	sprintf (string_port,"%02XH",audiodac_types[MAX_AUDIODAC_TYPES-1].port);

	menu_ventana_scanf("Port Value",string_port,4);

	int valor_port=parse_string_to_number(string_port);

	if (valor_port<0 || valor_port>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_port);
					return;
	}

	audiodac_set_custom_port(valor_port);
	//audiodac_types[MAX_AUDIODAC_TYPES-1].port=valor_port;
	//audiodac_selected_type=MAX_AUDIODAC_TYPES-1;

}

void menu_audio_beeper(MENU_ITEM_PARAMETERS)
{
	beeper_enabled.v ^=1;
}

void menu_audio_change_ay_chips(MENU_ITEM_PARAMETERS)
{
	if (total_ay_chips==MAX_AY_CHIPS) total_ay_chips=1;
	else total_ay_chips++;

	ay_chip_selected=0;
}

void menu_audio_ay_stereo(MENU_ITEM_PARAMETERS)
{
	ay3_stereo_mode++;

	if (ay3_stereo_mode==5) ay3_stereo_mode=0;
}


void menu_audio_ay_stereo_custom(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) {
		ay3_custom_stereo_A=2;

		ay3_custom_stereo_B++;
		if (ay3_custom_stereo_B==3) {
			ay3_custom_stereo_B=2;

			ay3_custom_stereo_C++;
			if (ay3_custom_stereo_C==3) {
				ay3_custom_stereo_A=0;
				ay3_custom_stereo_B=0;
				ay3_custom_stereo_C=0;
			}
		}
	}	
}




void menu_silence_detector(MENU_ITEM_PARAMETERS)
{
	silence_detector_setting.v ^=1;
}

void menu_settings_audio(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_audio;
	menu_item item_seleccionado;
	int retorno_menu;

        do {

		//hotkeys usadas: vuacpdrbfoilh

		menu_add_item_menu_inicial_format(&array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_volume,NULL,"    Output ~~Volume [%d%%]", audiovolume);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'v');

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip_autoenable,NULL,"[%c] A~~utoenable AY Chip",(autoenable_ay_chip.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'u');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip automatically when it is needed");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"This option is usefor for example on Spectrum 48k games that uses AY Chip "
					"and for some ZX80/81 games that also uses it (Bi-Pak ZON-X81, but not Quicksilva QS Sound board)");		

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip,NULL,"[%c] ~~AY Chip", (ay_chip_present.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'a');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip on this machine");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"It enables the AY Chip for the machine, by activating the following hardware:\n"
					"-Normal AY Chip for Spectrum\n"
					"-Fuller audio box for Spectrum\n"
					"-Quicksilva QS Sound board on ZX80/81\n"
					"-Bi-Pak ZON-X81 Sound on ZX80/81\n"
			);



			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_change_ay_chips,menu_cond_ay_chip,"[%d] AY ~~Chips %s",total_ay_chips,
				(total_ay_chips>1 ? "(Turbosound)" : "") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'c');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Total number of AY Chips");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Total number of AY Chips");


		if (si_complete_video_driver() ) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_setting_ay_piano_grafico,NULL,"    Show ~~Piano: %s",
					(setting_mostrar_ay_piano_grafico.v ? "Graphic" : "Text") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'p');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");

		}





		



		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			char string_audiodac[32];

				if (audiodac_enabled.v) {
					sprintf (string_audiodac,": %s",audiodac_types[audiodac_selected_type].name);
				}
				else {
					strcpy(string_audiodac,"");
				}

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_type,NULL,"[%c] ~~DAC%s",(audiodac_enabled.v ? 'X' : ' ' ),
						string_audiodac);
				menu_add_item_menu_shortcut(array_menu_settings_audio,'d');
				if (audiodac_enabled.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_set_port,NULL,"[%02XH] DAC port",audiodac_types[audiodac_selected_type].port);
				}



		}


    menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (!MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper,NULL,"[%c] Beepe~~r",(beeper_enabled.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'r');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable beeper output");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable beeper output");

		}



		if (MACHINE_IS_ZX8081) {
			//sound on zx80/81

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_zx8081_detect_vsync_sound,menu_cond_zx8081,"[%c] Detect VSYNC Sound",(zx8081_detect_vsync_sound.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");


			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_sound_zx8081,menu_cond_zx8081,"[%c] VSYNC Sound on zx80/81", (zx8081_vsync_sound.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enables or disables VSYNC sound on ZX80 and ZX81");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"This method uses the VSYNC signal on the TV to make sound");


		}




		int mostrar_real_beeper=0;

		if (MACHINE_IS_ZX8081) {
			if (zx8081_vsync_sound.v) mostrar_real_beeper=1;
		}

		else {
			if (beeper_enabled.v) mostrar_real_beeper=1;
		}

		if (mostrar_real_beeper) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper_real,NULL,"[%c] Real ~~Beeper",(beeper_real_enabled==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'b');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable Real Beeper enhanced sound. ");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Real beeper produces beeper sound more realistic but uses a bit more cpu. Needs beeper enabled (or vsync sound on zx80/81)");
		}


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_filter_on_rom_save,NULL,"[%c] Audio ~~filter ROM SAVE",(output_beep_filter_on_rom_save.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'f');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Apply filter on ROM save routines");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"It detects when on ROM save routines and alter audio output to use only "
					"the MIC bit of the FEH port");

//extern z80_bit output_beep_filter_alter_volume;
//extern char output_beep_filter_volume;

			if (output_beep_filter_on_rom_save.v) {
				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_alter_volume,NULL,"[%c] Alter beeper volume",
				(output_beep_filter_alter_volume.v ? 'X' : ' ') );

				menu_add_item_menu_tooltip(array_menu_settings_audio,"Alter output beeper volume");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Alter output beeper volume. You can set to a maximum to "
							"send the audio to a real spectrum to load it");


				if (output_beep_filter_alter_volume.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_volume,NULL,"[%d] Volume",output_beep_filter_volume);
				}
			}

		}

	
		menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		char string_aofile_shown[10];
		menu_tape_settings_trunc_name(aofilename,string_aofile_shown,10);
		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile,NULL,"Audio ~~out to file [%s]",string_aofile_shown);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'o');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Saves the generated sound to a file");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"You can save .raw format and if compiled with sndfile, to .wav format. "
					"You can see the file parameters on the console enabling verbose debug level to 2 minimum");



		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile_insert,menu_aofile_cond,"[%c] Audio file ~~inserted",(aofile_inserted.v ? 'X' : ' ' ));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'i');


				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_silence_detector,NULL,"[%c] Si~~lence detector",(silence_detector_setting.v ? 'X' : ' ' ));
				menu_add_item_menu_shortcut(array_menu_settings_audio,'l');
				menu_add_item_menu_tooltip(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");

                menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_change_audio_driver,NULL,"    C~~hange Audio Driver");
				menu_add_item_menu_shortcut(array_menu_settings_audio,'h');



					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_direct_midi_output,audio_midi_available,"AY to ~~MIDI Output");
					menu_add_item_menu_tooltip(array_menu_settings_audio,"Direct AY music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.");


#ifdef COMPILE_ALSA

					menu_add_item_menu_ayuda(array_menu_settings_audio,"Direct AY music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.\n"
						"On Linux you can simulate an external midi device by using timidity. If you have it installed, it may probably be running in memory as "
						"an alsa sequencer client. If not, run it with the command line:\n"
						"timidity -iA -Os -B2,8 -EFreverb=0\n"
						"Running timidity that way, would probably require that you use another audio driver in ZEsarUX different than alsa, "
						"unless you have alsa software mixing enabled"
					);

#else
					menu_add_item_menu_ayuda(array_menu_settings_audio,"Direct AY music output to a real MIDI device. Supported on Linux, Mac and Windows. On Linux, needs alsa driver compiled.");
#endif

					menu_add_item_menu_shortcut(array_menu_settings_audio,'m');
		








                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_ESC_item(array_menu_settings_audio);

                retorno_menu=menu_dibuja_menu(&settings_audio_opcion_seleccionada,&item_seleccionado,array_menu_settings_audio,"Audio Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



#ifdef EMULATE_CPU_STATS

void menu_debug_cpu_stats_clear_disassemble_array(void)
{
	int i;

	for (i=0;i<DISASSEMBLE_ARRAY_LENGTH;i++) disassemble_array[i]=0;
}

void menu_debug_cpu_stats_diss_no_print(z80_byte index,z80_byte opcode,char *dumpassembler)
{

        size_t longitud_opcode;

        disassemble_array[index]=opcode;

        debugger_disassemble_array(dumpassembler,31,&longitud_opcode,0);
}


/*
void menu_debug_cpu_stats_diss(z80_byte index,z80_byte opcode,int linea)
{

	char dumpassembler[32];

	//Empezar con espacio
	dumpassembler[0]=' ';

	menu_debug_cpu_stats_diss_no_print(index,opcode,&dumpassembler[1]);

	menu_escribe_linea_opcion(linea,-1,1,dumpassembler);
}
*/

void menu_debug_cpu_stats_diss_complete_no_print (z80_byte opcode,char *buffer,z80_byte preffix1,z80_byte preffix2)
{

	//Sin prefijo
	if (preffix1==0) {
                        menu_debug_cpu_stats_clear_disassemble_array();
                        menu_debug_cpu_stats_diss_no_print(0,opcode,buffer);
	}

	//Con 1 solo prefijo
	else if (preffix2==0) {
                        menu_debug_cpu_stats_clear_disassemble_array();
                        disassemble_array[0]=preffix1;
                        menu_debug_cpu_stats_diss_no_print(1,opcode,buffer);
	}

	//Con 2 prefijos (DD/FD + CB)
	else {
                        //Opcode
                        menu_debug_cpu_stats_clear_disassemble_array();
                        disassemble_array[0]=preffix1;
                        disassemble_array[1]=preffix2;
                        disassemble_array[2]=0;
                        menu_debug_cpu_stats_diss_no_print(3,opcode,buffer);
	}
}



void menu_cpu_full_stats(unsigned int *stats_table,char *title,z80_byte preffix1,z80_byte preffix2)
{

	int index_op,index_buffer;
	unsigned int counter;

	char stats_buffer[MAX_TEXTO_GENERIC_MESSAGE];

	char dumpassembler[32];

	//margen suficiente para que quepa una linea y un contador int de 32 bits...
	//aunque si pasa el ancho de linea, la rutina de generic_message lo troceara
	char buf_linea[64];

	index_buffer=0;

	for (index_op=0;index_op<256;index_op++) {
		counter=util_stats_get_counter(stats_table,index_op);

		menu_debug_cpu_stats_diss_complete_no_print(index_op,dumpassembler,preffix1,preffix2);

		sprintf (buf_linea,"%02X %-16s: %u \n",index_op,dumpassembler,counter);
		//16 ocupa la instruccion mas larga: LD B,RLC (IX+dd)

		sprintf (&stats_buffer[index_buffer],"%s\n",buf_linea);
		//sprintf (&stats_buffer[index_buffer],"%02X: %11d\n",index_op,counter);
		//index_buffer +=16;
		index_buffer +=strlen(buf_linea);
	}

	stats_buffer[index_buffer]=0;

	menu_generic_message(title,stats_buffer);

}

void menu_cpu_full_stats_codsinpr(MENU_ITEM_PARAMETERS)
{
	menu_cpu_full_stats(stats_codsinpr,"Full statistic no preffix",0,0);
}

void menu_cpu_full_stats_codpred(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codpred,"Full statistic pref ED",0xED,0);
}

void menu_cpu_full_stats_codprcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprcb,"Full statistic pref CB",0xCB,0);
}

void menu_cpu_full_stats_codprdd(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprdd,"Full statistic pref DD",0xDD,0);
}

void menu_cpu_full_stats_codprfd(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprfd,"Full statistic pref FD",0xFD,0);
}

void menu_cpu_full_stats_codprddcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprddcb,"Full statistic pref DDCB",0xDD,0xCB);
}

void menu_cpu_full_stats_codprfdcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprfdcb,"Full statistic pref FDCB",0xFD,0xCB);
}


void menu_cpu_full_stats_clear(MENU_ITEM_PARAMETERS)
{
	util_stats_init();

	menu_generic_message_splash("Clear CPU statistics","OK. Statistics cleared");
}


void menu_debug_cpu_stats(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_cpu_stats;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
                menu_add_item_menu_inicial_format(&array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_debug_cpu_resumen_stats,NULL,"Compact Statistics");
                menu_add_item_menu_tooltip(array_menu_cpu_stats,"Shows Compact CPU Statistics");
                menu_add_item_menu_ayuda(array_menu_cpu_stats,"Shows the most used opcode for every type: without preffix, with ED preffix, "
					"etc. CPU Statistics are reset when changing machine or resetting CPU.");


                menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codsinpr,NULL,"Full Statistics No pref");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codpred,NULL,"Full Statistics Pref ED");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprcb,NULL,"Full Statistics Pref CB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprdd,NULL,"Full Statistics Pref DD");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprfd,NULL,"Full Statistics Pref FD");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprddcb,NULL,"Full Statistics Pref DDCB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprfdcb,NULL,"Full Statistics Pref FDCB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_clear,NULL,"Clear Statistics");


                menu_add_item_menu(array_menu_cpu_stats,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_cpu_stats,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_cpu_stats);

                retorno_menu=menu_dibuja_menu(&cpu_stats_opcion_seleccionada,&item_seleccionado,array_menu_cpu_stats,"CPU Statistics" );
		

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int cpu_stats_valor_contador_segundo_anterior;

zxvision_window *menu_debug_cpu_resumen_stats_overlay_window;

void menu_debug_cpu_resumen_stats_overlay(void)
{
	if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	    char textostats[32];
	zxvision_window *ventana;

	ventana=menu_debug_cpu_resumen_stats_overlay_window;


        char dumpassembler[32];

        //Empezar con espacio
        dumpassembler[0]=' ';

				//int valor_contador_segundo_anterior;



		//z80_byte tecla;

		//printf ("%d %d\n",contador_segundo,cpu_stats_valor_contador_segundo_anterior);
     

			//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && cpu_stats_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											cpu_stats_valor_contador_segundo_anterior=contador_segundo;
				//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

			int linea=0;
                        int opcode;

			unsigned int sumatotal; 
                        sumatotal=util_stats_sum_all_counters();
                    	sprintf (textostats,"Total opcodes run: %u",sumatotal);
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        


						//menu_escribe_linea_opcion(linea++,-1,1,"Most used op. for each preffix");
						zxvision_print_string_defaults(ventana,1,linea++,"Most used op. for each preffix");

                        opcode=util_stats_find_max_counter(stats_codsinpr);
                        sprintf (textostats,"Op nopref:    %02XH: %u",opcode,util_stats_get_counter(stats_codsinpr,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        

                        //Opcode
						menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],0,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);
						



                        opcode=util_stats_find_max_counter(stats_codpred);
                        sprintf (textostats,"Op pref ED:   %02XH: %u",opcode,util_stats_get_counter(stats_codpred,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],237,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);
                        

	
                        opcode=util_stats_find_max_counter(stats_codprcb);
                        sprintf (textostats,"Op pref CB:   %02XH: %u",opcode,util_stats_get_counter(stats_codprcb,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);


                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],203,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);




                        opcode=util_stats_find_max_counter(stats_codprdd);
                        sprintf (textostats,"Op pref DD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprdd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprfd);
                        sprintf (textostats,"Op pref FD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprfd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprddcb);
                        sprintf (textostats,"Op pref DDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprddcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);



                        opcode=util_stats_find_max_counter(stats_codprfdcb);
                        sprintf (textostats,"Op pref FDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprfdcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


						zxvision_draw_window_contents(ventana);


                }



}

zxvision_window menu_debug_cpu_resumen_stats_ventana;

void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS)
{

    

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

		zxvision_window *ventana;
		
		ventana=&menu_debug_cpu_resumen_stats_ventana;

	int originx=menu_origin_x();

	zxvision_new_window(ventana,originx,1,32,18,
							31,16,"CPU Compact Statistics");
	zxvision_draw_window(ventana);
		


		menu_debug_cpu_resumen_stats_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

						cpu_stats_valor_contador_segundo_anterior=contador_segundo;

        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_debug_cpu_resumen_stats_overlay);

	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();		
		zxvision_handle_cursors_pgupdn(ventana,tecla);
		//printf ("tecla: %d\n",tecla);
	} while (tecla!=2 && tecla!=3);				

	//Gestionar salir con tecla background
 
	menu_espera_no_tecla(); //Si no, se va al menu anterior.
	//En AY Piano por ejemplo esto no pasa aunque el estilo del menu es el mismo...

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();	


	if (tecla==3) {
		//zxvision_ay_registers_overlay
		ventana->overlay_function=menu_debug_cpu_resumen_stats_overlay;
		printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
		menu_generic_message("Background task","OK. Window put in background");
	}

	else {
		zxvision_destroy_window(ventana);		
 	}




}

#endif

void zxvision_test_sleep_quarter(void)
{
	int previo_contador_segundo=contador_segundo;

	while (1) {
		menu_cpu_core_loop();
		if (previo_contador_segundo!=contador_segundo && (contador_segundo%250)==0) return;

		if (menu_get_pressed_key()!=0) return;
	
	}
}


void menu_zxvision_test(MENU_ITEM_PARAMETERS)
{

        //Desactivamos interlace - si esta. Con interlace la forma de onda se dibuja encima continuamente, sin borrar
        //z80_bit copia_video_interlaced_mode;
        //copia_video_interlaced_mode.v=video_interlaced_mode.v;

        //disable_interlace();


        menu_espera_no_tecla();


	//Prueba filesel
       /* char *filtros[2];

        filtros[0]="";
        filtros[1]=0;

        char nombredestino[PATH_MAX];
	nombredestino[0]=0;


	int retorno=menu_filesel("Select Target File",filtros,nombredestino);
	printf ("retorno: %d nombredestino: [%s]\n",retorno,nombredestino);
	return;*/



		//zxvision_generic_message_tooltip("pruebas", 30, 0, 0, generic_message_tooltip_return *retorno, const char * texto_format , ...)
		zxvision_generic_message_tooltip("Pruebas" , 0 , 0, 0, 0, NULL, 0, "Hola que tal como estas esto es una prueba de escribir texto. "
					"No se que mas poner pero me voy a empezar a repetir, " 
					"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore "
					"et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip"
					" ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore "
					" eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia "
					"deserunt mollit anim id est laborum. Adios");
        //z80_byte acumulado;



        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	//set_menu_overlay_function(menu_audio_draw_sound_wave);

	zxvision_window ventana;

#define SOUND_ZXVISION_WAVE_X 1
#define SOUND_ZXVISION_WAVE_Y 3
#define SOUND_ZXVISION_WAVE_ANCHO 27
#define SOUND_ZXVISION_WAVE_ALTO 14

	int ancho_visible=SOUND_ZXVISION_WAVE_ANCHO;
	int alto_visible=SOUND_ZXVISION_WAVE_ALTO+4;

	int ancho_total=20;
	int alto_total=alto_visible+2;

	//menu_item *array_menu_audio_new_waveform;
      //  menu_item item_seleccionado;
        //int retorno_menu;
        


	  //Hay que redibujar la ventana desde este bucle
	zxvision_new_window(&ventana,SOUND_ZXVISION_WAVE_X,SOUND_ZXVISION_WAVE_Y-2,ancho_visible,alto_visible,
							ancho_total,alto_total,"ZXVision Test");
	zxvision_draw_window(&ventana);

	printf ("Created window\n");

	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	printf ("Drawn window contents\n");

	menu_espera_tecla(); 
	menu_espera_no_tecla();


	overlay_screen caracter;

/*	struct s_overlay_screen {
	z80_byte tinta,papel,parpadeo;
	z80_byte caracter;
};*/

	caracter.tinta=ESTILO_GUI_TINTA_NORMAL;
	caracter.papel=ESTILO_GUI_PAPEL_NORMAL;
	caracter.parpadeo=0;

	//Relleno pantalla
	z80_byte caracter_print=32;



	int x,y;

	for (y=0;y<alto_total;y++) {
		for (x=0;x<ancho_total;x++) {
			caracter.caracter=caracter_print;
			zxvision_print_char(&ventana,x,y,&caracter);	

			caracter_print++;
			if (caracter_print>126) caracter_print=32;		
		}
	}

	caracter.caracter='A';

	zxvision_print_char(&ventana,0,0,&caracter);

	caracter.caracter='B';

	zxvision_print_char(&ventana,0,1,&caracter);

	zxvision_print_string(&ventana,0,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 1 ");
	zxvision_print_string(&ventana,0,1,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 2 ");
	zxvision_print_string(&ventana,0,2,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 3 ");

	zxvision_print_string(&ventana,2,3,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," This is a test ");

	zxvision_print_string(&ventana,2,4,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Press a key ");
	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to next step ");

	zxvision_print_string(&ventana,2,6,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --^^flash^^--");
	zxvision_print_string(&ventana,2,7,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --~~inverse--");
	zxvision_print_string(&ventana,2,8,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --$$2ink--");


                    /*    menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"Change wave ~~Shape");
                        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

                        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
                        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
                        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						//0123456789
						// Change wave Shape
						
			menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);





		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );*/


	ventana.upper_margin=2;
	ventana.lower_margin=1;


	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	menu_espera_tecla(); 
	menu_espera_no_tecla();

	//Jugar con offset
/*	int i;

	for (i=0;i<7;i++) {
		zxvision_set_offset_x(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}


	for (i=0;i<7;i++) {
		zxvision_set_offset_y(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}



	for (i=0;i<10;i++) {
		zxvision_set_x_position(&ventana,i);

		printf ("Move x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	for (i=0;i<10;i++) {
		zxvision_set_y_position(&ventana,i);

		printf ("Move y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	zxvision_set_x_position(&ventana,0);
	zxvision_set_y_position(&ventana,0);

	for (i=25;i<35;i++) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	for (i=18;i<28;i++) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


	for (i=5;i>=0;i--) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	zxvision_set_visible_width(&ventana,20);

	for (i=5;i>=0;i--) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


*/
	zxvision_print_string(&ventana,2,3,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use cursors ");
	zxvision_print_string(&ventana,2,4,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move offset ");	
	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," QAOP size");	
	zxvision_print_string(&ventana,2,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," ESC exit ");	


	//Rebotar
	int contador=0;

	int xpos=0;
	int ypos=0;

	int incx=+1;
	int incy=+1;

	int offsetx=0;
	int offsety=0;	

	int ancho=22;
	int alto=10;

	zxvision_set_visible_height(&ventana,alto);
	zxvision_set_visible_width(&ventana,ancho);	

	z80_byte tecla=0;

	//Salir con ESC
	while (tecla!=2) {

		zxvision_set_offset_x(&ventana,offsetx);
		zxvision_set_offset_y(&ventana,offsety);

		zxvision_set_x_position(&ventana,xpos);
		zxvision_set_y_position(&ventana,ypos);		

		zxvision_test_sleep_quarter();

		tecla=menu_get_pressed_key();
		//Cambio offset con cursores
		if (tecla==8) {
			offsetx--;
			printf ("Decrement offset x to %d\n",offsetx);
		}

		if (tecla==9) {
			offsetx++;
			printf ("Increment offset x to %d\n",offsetx);
		}

		if (tecla==10) {
			offsety++;
			printf ("Increment offset y to %d\n",offsety);
		}

		if (tecla==11) {
			offsety--;
			printf ("Decrement offset y to %d\n",offsety);
		}

		//Cambio tamanyo
		if (tecla=='a' && ypos+alto<24) {
			alto++;
			printf ("Increment height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='q' && alto>1) {
			alto--;
			printf ("Decrement height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='p' && xpos+ancho<32) {
			ancho++;
			printf ("Increment width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		if (tecla=='o' && ancho>7) {
			ancho--;
			printf ("Decrement width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		xpos +=incx;
		if (xpos+ancho>=32 || xpos<=0) {
			incx=-incx;
		}

		ypos +=incy;
		if (ypos+alto>=24 || ypos<=0) {
			incy=-incy;
		}		
		
		contador++;

		if (tecla!=0) menu_espera_no_tecla();

	}


	zxvision_print_string(&ventana,1,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use mouse    ");
	zxvision_print_string(&ventana,1,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move and   ");	
	zxvision_print_string(&ventana,1,7,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," resize window  ");	
	zxvision_print_string(&ventana,1,8,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Right button exits ");	

	zxvision_draw_window_contents(&ventana);

	tecla=0;

	while (!mouse_right) {
		//menu_espera_tecla();
		tecla=menu_get_pressed_key();
		//Comprobar eventos raton
		menu_cpu_core_loop();
		//zxvision_handle_mouse_events(&ventana);
	} 

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);
            




}


void menu_about_core_statistics(MENU_ITEM_PARAMETERS)
{

    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();


	zxvision_window ventana;

	int alto_ventana=9;
	int ancho_ventana=32;

	int x_ventana=menu_center_x()-ancho_ventana/2; //0;
	int y_ventana=menu_center_y()-alto_ventana/2; //7;


	zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Core Statistics");

	zxvision_draw_window(&ventana);


        char texto_buffer[33];


        //Empezar con espacio
    texto_buffer[0]=' ';

        int valor_contador_segundo_anterior;

        valor_contador_segundo_anterior=contador_segundo;


		z80_byte tecla;

        do {


                //esto hara ejecutar esto 2 veces por segundo
                if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
                     valor_contador_segundo_anterior=contador_segundo;
                     //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

                                int linea=0;
                                //int opcode;
                                //int sumatotal;

/*

Nota: calcular el tiempo entre ejecuciones de cada opcode no penaliza mucho el uso de cpu real.
Ejemplo:
--vo null --machine 48k 

Sin calcular ese tiempo: 9% cpu
Calculando ese tiempo: 12% cpu

*/


//Ultimo intervalo de tiempo
//long core_cpu_timer_frame_difftime;
//Media de todos los intervalos
//long core_cpu_timer_frame_media=0;

				long valor_mostrar;
				valor_mostrar=core_cpu_timer_frame_difftime;
				//controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
			     //01234567890123456789012345678901
			     // Last core frame: 999999 us
				sprintf (texto_buffer,"Last core frame:     %6ld us",valor_mostrar);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_frame_media;
                                //controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core frame: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_refresca_pantalla_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Last render display: 999999 us
                                sprintf (texto_buffer,"Last full render:    %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_refresca_pantalla_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core refresca_pantalla: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_each_frame_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Time between frames: 999999 us
                                sprintf (texto_buffer,"Time between frames: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_each_frame_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core each_frame: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

								 //menu_escribe_linea_opcion(linea++,-1,1," (ideal):  20000 us");
								 zxvision_print_string_defaults(&ventana,1,linea++," (expected): 20000 us");


								zxvision_draw_window_contents(&ventana);


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();				


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

        cls_menu_overlay();
		zxvision_destroy_window(&ventana);


}




int ayregisters_previo_valor_volume_A[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_B[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_C[MAX_AY_CHIPS];

	int menu_ayregisters_valor_contador_segundo_anterior;

zxvision_window *menu_ay_registers_overlay_window;

void menu_ay_registers_overlay(void)
{

	//NOTA: //Hemos de suponer que current window es esta de ay registers

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	char volumen[32],textotono[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo


	int total_chips=ay_retorna_numero_chips();

	//Como maximo mostrarmos 3 canales ay
	if (total_chips>3) total_chips=3;

	int chip;

	int linea=0;

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

		int vol_A[MAX_AY_CHIPS],vol_B[MAX_AY_CHIPS],vol_C[MAX_AY_CHIPS];

	for (chip=0;chip<total_chips;chip++) {


        	vol_A[chip]=ay_3_8912_registros[chip][8] & 15;
        	vol_B[chip]=ay_3_8912_registros[chip][9] & 15;
        	vol_C[chip]=ay_3_8912_registros[chip][10] & 15;

			//Controlar limites, dado que las variables entran sin inicializar
			if (ayregisters_previo_valor_volume_A[chip]>16) ayregisters_previo_valor_volume_A[chip]=16;
			if (ayregisters_previo_valor_volume_B[chip]>16) ayregisters_previo_valor_volume_B[chip]=16;
			if (ayregisters_previo_valor_volume_C[chip]>16) ayregisters_previo_valor_volume_C[chip]=16;
			

			ayregisters_previo_valor_volume_A[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
			ayregisters_previo_valor_volume_B[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
			ayregisters_previo_valor_volume_C[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);


        		//if (ayregisters_previo_valor_volume_A[chip]<vol_A[chip]) ayregisters_previo_valor_volume_A[chip]=vol_A[chip];
        		//if (ayregisters_previo_valor_volume_B[chip]<vol_B[chip]) ayregisters_previo_valor_volume_B[chip]=vol_B[chip];
        		//if (ayregisters_previo_valor_volume_C[chip]<vol_C[chip]) ayregisters_previo_valor_volume_C[chip]=vol_C[chip];


			menu_string_volumen(volumen,ay_3_8912_registros[chip][8],ayregisters_previo_valor_volume_A[chip]);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][9],ayregisters_previo_valor_volume_B[chip]);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][10],ayregisters_previo_valor_volume_C[chip]);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);



			int freq_a=ay_retorna_frecuencia(0,chip);
			int freq_b=ay_retorna_frecuencia(1,chip);
			int freq_c=ay_retorna_frecuencia(2,chip);
			sprintf (textotono,"Channel A:  %3s %7d Hz",get_note_name(freq_a),freq_a);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel B:  %3s %7d Hz",get_note_name(freq_b),freq_b);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel C:  %3s %7d Hz",get_note_name(freq_c),freq_c);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

			                        //Frecuencia ruido
                        int freq_temp=ay_3_8912_registros[chip][6] & 31;
                        //printf ("Valor registros ruido : %d Hz\n",freq_temp);
                        freq_temp=freq_temp*16;

                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;

                        int freq_ruido=FRECUENCIA_NOISE/freq_temp;

                        sprintf (textotono,"Frequency Noise: %6d Hz",freq_ruido);
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Envelope

                        freq_temp=ay_3_8912_registros[chip][11]+256*(ay_3_8912_registros[chip][12] & 0xFF);


                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;
                        int freq_envelope=FRECUENCIA_ENVELOPE/freq_temp;

                        //sprintf (textotono,"Freq Envelope(*10): %5d Hz",freq_envelope);

			int freq_env_10=freq_envelope/10;
			int freq_env_decimal=freq_envelope-(freq_env_10*10);

			sprintf (textotono,"Freq Envelope:   %4d.%1d Hz",freq_env_10,freq_env_decimal);
      		//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);



			char envelope_name[32];
			z80_byte env_type=ay_3_8912_registros[chip][13] & 0x0F;
			return_envelope_name(env_type,envelope_name);
			sprintf (textotono,"Env.: %2d (%s)",env_type,envelope_name);
            //menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			}



			sprintf (textotono,"Tone Channels:  %c %c %c",
				( (ay_3_8912_registros[chip][7]&1)==0 ? 'A' : ' '),
				( (ay_3_8912_registros[chip][7]&2)==0 ? 'B' : ' '),
				( (ay_3_8912_registros[chip][7]&4)==0 ? 'C' : ' '));
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

                        sprintf (textotono,"Noise Channels: %c %c %c",
                                ( (ay_3_8912_registros[chip][7]&8)==0  ? 'A' : ' '),
                                ( (ay_3_8912_registros[chip][7]&16)==0 ? 'B' : ' '),
                                ( (ay_3_8912_registros[chip][7]&32)==0 ? 'C' : ' '));
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);
			}

	}




	//Hacer decaer volumenes
                        //Decrementar volumenes que caen, pero hacerlo no siempre, sino 2 veces por segundo
            //esto hara ejecutar esto 2 veces por segundo
            if ( ((contador_segundo%500) == 0 && menu_ayregisters_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

                                 menu_ayregisters_valor_contador_segundo_anterior=contador_segundo;
                                //printf ("Refrescando. contador_segundo=%d. chip: %d\n",contador_segundo,chip);

				for (chip=0;chip<total_chips;chip++) {

					ayregisters_previo_valor_volume_A[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
					ayregisters_previo_valor_volume_B[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
					ayregisters_previo_valor_volume_C[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);


				}


        }


	zxvision_draw_window_contents(menu_ay_registers_overlay_window); 


}


//Ventana como variable global
zxvision_window zxvision_ay_registers_overlay;

void menu_ay_registers(MENU_ITEM_PARAMETERS)
{
    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		int total_chips=ay_retorna_numero_chips();
		if (total_chips>3) total_chips=3;

		int xventana,yventana;
		int ancho_ventana,alto_ventana;

		if (!util_find_window_geometry("ayregisters",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {

        	if (total_chips==1) {
				yventana=5;
			}
			else {
				yventana=0;
			}

			xventana=menu_origin_x()+1;
			ancho_ventana=30;

		}

		//El alto siempre lo cambiamos segun el numero de chips
        if (total_chips==1) {
				alto_ventana=14;
		}
		else {
				alto_ventana=24;
		}		

		zxvision_window *ventana;
		ventana=&zxvision_ay_registers_overlay;



		zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"AY Registers");

		zxvision_draw_window(ventana);		


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_ay_registers_overlay);

		menu_ay_registers_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();		
		zxvision_handle_cursors_pgupdn(ventana,tecla);
		//printf ("tecla: %d\n",tecla);
	} while (tecla!=2 && tecla!=3);				

	//Gestionar salir con tecla background
 
	menu_espera_no_tecla(); //Si no, se va al menu anterior.
	//En AY Piano por ejemplo esto no pasa aunque el estilo del menu es el mismo...

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();	


	if (tecla==3) {
		//zxvision_ay_registers_overlay
		ventana->overlay_function=menu_ay_registers_overlay;
		printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
		menu_generic_message("Background task","OK. Window put in background");
	}

	else {
		util_add_window_geometry_compact("ayregisters",ventana);
		zxvision_destroy_window(ventana);		
 	}
}



void menu_draw_background_windows_overlay(void)
{

	//menu_ay_registers_overlay();
	//return;
	normal_overlay_texto_menu();

	zxvision_window *ventana;
	ventana=zxvision_current_window;
	zxvision_draw_below_windows_with_overlay(ventana);
	printf ("overlay funcion desde menu_draw_background_windows_overlay\n");
}

void menu_draw_background_windows(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
        menu_reset_counters_tecla_repeticion();

                if (!menu_multitarea) {
                        menu_warn_message("This menu item needs multitask enabled");
                        return;
                }

	if (zxvision_current_window==NULL) {
		printf ("No windows in background\n");
		return;
	}

                //zxvision_window *ventana;
                //ventana=zxvision_current_window;

	//Metemos funcion de overlay que se encarga de repintar ventanas de debajo con overlay
	set_menu_overlay_function(menu_draw_background_windows_overlay);


        z80_byte tecla;

        do {
                tecla=zxvision_common_getkey_refresh();

                printf ("tecla: %d\n",tecla);
        } while (tecla!=2);


        menu_espera_no_tecla(); //Si no, se va al menu anterior.

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();


}



void menu_debug_tsconf_tbblue_videoregisters(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	int ancho_ventana=32;
	int xventana=menu_center_x()-ancho_ventana/2;

	int yventana;
	int alto_ventana;
    

	if (MACHINE_IS_TBBLUE) {
		//yventana=0;
		alto_ventana=24;
	}

	else {
		//yventana=7;
		alto_ventana=8;
	}

	yventana=menu_center_y()-alto_ventana/2;

	zxvision_window ventana;

		zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Video Info");							

	zxvision_draw_window(&ventana);




	char texto_buffer[64];

	char texto_buffer2[64];

	//Empezar con espacio
    texto_buffer[0]=' ';

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla;


    	do {


        	//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
				//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

				int linea=0;
				//int opcode;
				//int sumatotal;


				if (MACHINE_IS_TSCONF) {

				int vpage_addr=tsconf_get_vram_page()*16384;

				tsconf_get_current_video_mode(texto_buffer2);
				sprintf (texto_buffer,"Video mode: %s",texto_buffer2);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);
				
                sprintf (texto_buffer,"Video addr: %06XH",vpage_addr);
                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile Map Page: %06XH",tsconf_return_tilemappage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 0 Graphics addr: %06XH",tsconf_return_tilegraphicspage(0) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 1 Graphics addr: %06XH",tsconf_return_tilegraphicspage(1) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


				sprintf (texto_buffer,"Sprite Graphics addr: %06XH",tsconf_return_spritesgraphicspage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				}

				if (MACHINE_IS_TBBLUE) {

					//menu_escribe_linea_opcion(linea++,-1,1,"ULA Video mode:");		
					zxvision_print_string_defaults(&ventana,1,linea++,"ULA Video mode:");

					//menu_escribe_linea_opcion(linea++,-1,1,get_spectrum_ula_string_video_mode() );
					zxvision_print_string_defaults(&ventana,1,linea++,get_spectrum_ula_string_video_mode() );

					linea++;

					//menu_escribe_linea_opcion(linea++,-1,1,"Palette format:");
					//zxvision_print_string_defaults(&ventana,1,linea++,"Palette:");

					tbblue_get_string_palette_format(texto_buffer2);
					sprintf (texto_buffer,"Palette: %s",texto_buffer2);
					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					linea++;

					/*
					(R/W) 0x12 (18) => Layer 2 RAM page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 8 after a Reset)

(R/W) 0x13 (19) => Layer 2 RAM shadow page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 11 after a Reset)
					*/

				//tbblue_get_offset_start_layer2_reg
					sprintf (texto_buffer,"Layer 2 addr:        %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[18]) );
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer 2 shadow addr: %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[19]) );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Tilemap base addr:     %02X00H",0x40+tbblue_get_offset_start_tilemap() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Tile definitions addr: %02X00H",0x40+tbblue_get_offset_start_tiledef() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);					

					sprintf (texto_buffer,"Tile width: %d columns",tbblue_get_tilemap_width() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Tile bpp: %d", (tbblue_tiles_are_monocrome() ? 1 : 4)  );
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);							

					/*
					z80_byte clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_ULA][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][4];
					*/

					linea++;
					sprintf (texto_buffer,"Clip Windows:");
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2:  X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    const static int clip_index_string_pos_x[4] = { 11, 15, 21, 25};
//                    int clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_layer2_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);


					sprintf (texto_buffer,"Sprites: X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_sprites_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);



					sprintf (texto_buffer,"ULA:     X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_ULA][0],clip_windows[TBBLUE_CLIP_WINDOW_ULA][1],clip_windows[TBBLUE_CLIP_WINDOW_ULA][2],clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_ula_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string_defaults(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);




					sprintf (texto_buffer,"Tilemap: X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]*2,clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]*2+1,clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_tilemap_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);


					linea++;
					sprintf (texto_buffer,"Offset Windows:");
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2:     X=%4d  Y=%3d",tbblue_registers[22],tbblue_registers[23]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


					sprintf (texto_buffer,"ULA/LoRes:  X=%4d  Y=%3d",tbblue_registers[50],tbblue_registers[51]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					//Offset X puede llegar hasta 1023. Por tanto 4 cifras. El resto X solo 3 cifras, pero los dejamos a 4 para que formato quede igual en pantalla
					sprintf (texto_buffer,"Tilemap:    X=%4d  Y=%3d",tbblue_registers[48]+256*(tbblue_registers[47]&3),tbblue_registers[49]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);					

				}



				zxvision_draw_window_contents(&ventana);


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

        cls_menu_overlay();
		zxvision_destroy_window(&ventana);


}



#define TSCONF_SPRITENAV_WINDOW_ANCHO 32
#define TSCONF_SPRITENAV_WINDOW_ALTO 20
#define TSCONF_SPRITENAV_WINDOW_X (menu_center_x()-TSCONF_SPRITENAV_WINDOW_ANCHO/2 )
#define TSCONF_SPRITENAV_WINDOW_Y (menu_center_y()-TSCONF_SPRITENAV_WINDOW_ALTO/2)




//int menu_debug_tsconf_tbblue_spritenav_current_palette=0;
int menu_debug_tsconf_tbblue_spritenav_current_sprite=0;


zxvision_window *menu_debug_tsconf_tbblue_spritenav_draw_sprites_window;

int menu_debug_tsconf_tbblue_spritenav_get_total_sprites(void)
{
	int limite;

	limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;

	return limite;
}

//Muestra lista de sprites
void menu_debug_tsconf_tbblue_spritenav_lista_sprites(void)
{

	char dumpmemoria[33];

	int linea_color;
	int limite;

	int linea=0;
	/*limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;*/

	limite=menu_debug_tsconf_tbblue_spritenav_get_total_sprites();

	//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

	int current_sprite;


		/*for (linea_color=0;linea_color<TSCONF_SPRITENAV_SPRITES_PER_WINDOW &&
				menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color<limite;
				linea_color++) {*/
		for (linea_color=0;linea_color<limite;linea_color++) {					

			current_sprite=menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color;

			if (MACHINE_IS_TSCONF) {

			int offset=current_sprite*6;
			z80_byte sprite_r0h=tsconf_fmaps[0x200+offset+1];

			z80_byte sprite_leap=sprite_r0h&64;

			int sprite_act=sprite_r0h&32;
        	int y=tsconf_fmaps[0x200+offset]+256*(sprite_r0h&1);
	      	z80_byte ysize=8*(1+((sprite_r0h>>1)&7));
	               

        	z80_byte sprite_r1h=tsconf_fmaps[0x200+offset+3];
		    int x=tsconf_fmaps[0x200+offset+2]+256*(sprite_r1h&1);
			z80_byte xsize=8*(1+((sprite_r1h>>1)&7));

			z80_byte sprite_r2h=tsconf_fmaps[0x200+offset+5];
			z80_int tnum=(tsconf_fmaps[0x200+offset+4])+256*(sprite_r2h&15);
			    	//Tile Number for upper left corner. Bits 0-5 are X Position in Graphics Bitmap, bits 6-11 - Y Position.
			z80_int tnum_x=tnum & 63;
    		z80_int tnum_y=(tnum>>6)&63;

		    z80_byte spal=(sprite_r2h>>4)&15;

			z80_byte sprite_xf=sprite_r1h&128;
			z80_byte sprite_yf=sprite_r0h&128;

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d (%2dX%2d)",current_sprite,x,y,xsize,ysize);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria,"Tile:%2d,%2d %s %s %s %s P:%2d",tnum_x,tnum_y,
				(sprite_act ? "ACT" : "   "),(sprite_leap ? "LEAP": "    "),
				(sprite_xf ? "XF" : "  "),(sprite_yf ? "YF": "  "),
				spal );

			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);
			}

			if (MACHINE_IS_TBBLUE) {
					//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

				z80_int x=tbsprite_sprites[current_sprite][0]; //
				z80_byte y=tbsprite_sprites[current_sprite][1];  //

				z80_byte byte_3=tbsprite_sprites[current_sprite][2];
				z80_byte paloff=byte_3 & 0xF0; //
				z80_byte mirror_x=byte_3 & 8; //
				z80_byte mirror_y=byte_3 & 4; //
				z80_byte rotate=byte_3 & 2; //
				z80_byte msb_x=byte_3 &1; //

				x +=msb_x*256;

				z80_byte byte_4=tbsprite_sprites[current_sprite][3];
				z80_byte visible=byte_4 & 128; //
				z80_byte pattern=byte_4 & 63; //

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d %s %s %s",current_sprite,x,y,
					(mirror_x ? "MIRX" : "    "),(mirror_y ? "MIRY" : "    "),(rotate ? "ROT" : "   ")
			);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria," Pattn: %2d Palof: %3d Vis: %s"
				,pattern,paloff, (visible ? "Yes" : "No ") );


				//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			}			

			


					
		}

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window); 


}

void menu_debug_tsconf_tbblue_spritenav_draw_sprites(void)
{



				/*menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista sprites
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();

				//Esto tiene que estar despues de escribir la lista de sprites, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();*/


				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();



}



void menu_debug_tsconf_tbblue_spritenav(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
	zxvision_window ventana;

	zxvision_new_window(&ventana,TSCONF_SPRITENAV_WINDOW_X,TSCONF_SPRITENAV_WINDOW_Y,TSCONF_SPRITENAV_WINDOW_ANCHO,TSCONF_SPRITENAV_WINDOW_ALTO,
							TSCONF_SPRITENAV_WINDOW_ANCHO-1,menu_debug_tsconf_tbblue_spritenav_get_total_sprites()*2,"Sprite navigator");

	zxvision_draw_window(&ventana);		

    set_menu_overlay_function(menu_debug_tsconf_tbblue_spritenav_draw_sprites);

	menu_debug_tsconf_tbblue_spritenav_draw_sprites_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


	z80_byte tecla;

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			//printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}			

	
    do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech
   		tecla=zxvision_common_getkey_refresh();
		zxvision_handle_cursors_pgupdn(&ventana,tecla);
	} while (tecla!=2);  

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);
}




#define TSCONF_TILENAV_WINDOW_ANCHO 32
#define TSCONF_TILENAV_WINDOW_ALTO 24
#define TSCONF_TILENAV_WINDOW_X (menu_center_x()-TSCONF_TILENAV_WINDOW_ANCHO/2 )
#define TSCONF_TILENAV_WINDOW_Y (menu_center_y()-TSCONF_TILENAV_WINDOW_ALTO/2)
#define TSCONF_TILENAV_TILES_VERT_PER_WINDOW 64
#define TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW 64



//int menu_debug_tsconf_tbblue_tilenav_current_palette=0;
//int menu_debug_tsconf_tbblue_tilenav_current_tile=0;

int menu_debug_tsconf_tbblue_tilenav_current_tilelayer=0;

z80_bit menu_debug_tsconf_tbblue_tilenav_showmap={0};

zxvision_window *menu_debug_tsconf_tbblue_tilenav_lista_tiles_window;


#define DEBUG_TSCONF_TILENAV_MAX_TILES (64*64)
//#define DEBUG_TBBLUE_TILENAV_MAX_TILES_8032 (80*32)
//#define DEBUG_TBBLUE_TILENAV_MAX_TILES_4032 (40*32)


char menu_debug_tsconf_tbblue_tiles_retorna_visualchar(int tnum)
{
	//Hacer un conjunto de 64 caracteres. Mismo set de caracteres que para Base64. Por que? Por que si :)
			   //0123456789012345678901234567890123456789012345678901234567890123
	char *caracter_list="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int index=tnum % 64;
	return caracter_list[index];
}

int menu_debug_tsconf_tbblue_tilenav_total_vert(void)
{

	int limite_vertical;

	if (MACHINE_IS_TSCONF) {
		limite_vertical=DEBUG_TSCONF_TILENAV_MAX_TILES;
		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=TSCONF_TILENAV_TILES_VERT_PER_WINDOW;	
	}

	else  { //TBBLUE
		limite_vertical=tbblue_get_tilemap_width()*32;

		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=32;	
	}

	return limite_vertical;
}

//Muestra lista de tiles
void menu_debug_tsconf_tbblue_tilenav_lista_tiles(void)
{

	//Suficientemente grande para almacenar regla superior en modo visual
	char dumpmemoria[84]; //80 + 3 espacios izquierda + 0 final

	
	//int limite;

	int linea=0;
	//limite=DEBUG_TSCONF_TILENAV_MAX_TILES;

	int current_tile;

	z80_byte *puntero_tilemap;
	z80_byte *puntero_tilemap_orig;

	if (MACHINE_IS_TSCONF) {
		puntero_tilemap=tsconf_ram_mem_table[0]+tsconf_return_tilemappage();
	}

	else {  //TBBLUE
		//puntero_tilemap=tbblue_ram_mem_table[5]+tbblue_get_offset_start_tilemap();
			//Siempre saldra de ram 5
		puntero_tilemap=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tilemap());	
		//printf ("%XH\n",tbblue_get_offset_start_tilemap() );

	}

	z80_byte tbblue_tilemap_control;
	int tilemap_width;


	int tbblue_bytes_per_tile=2;

	if (MACHINE_IS_TBBLUE) {
					/*
					(R/W) 0x6B (107) => Tilemap Control
  bit 7    = 1 to enable the tilemap
  bit 6    = 0 for 40x32, 1 for 80x32
  bit 5    = 1 to eliminate the attribute entry in the tilemap
  bit 4    = palette select
  bits 3-0 = Reserved set to 0
					*/
					tbblue_tilemap_control=tbblue_registers[107];

					if (tbblue_tilemap_control&32) tbblue_bytes_per_tile=1;

					tilemap_width=tbblue_get_tilemap_width();

	}

	puntero_tilemap_orig=puntero_tilemap;

	int limite_vertical=menu_debug_tsconf_tbblue_tilenav_total_vert();


	int offset_vertical=0;

	if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
		if (MACHINE_IS_TSCONF) {
				  //0123456789012345678901234567890123456789012345678901234567890123
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   40   45   50   55   60  ");
		}

		else { //TBBLUE
			if (tilemap_width==40) {
				             //0123456789012345678901234567890123456789012345678901234567890123
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   ");
			}
			else {
				             //01234567890123456789012345678901234567890123456789012345678901234567890123456789
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   40   45   50   55   60   65   70   75   ");
			}

		}

		//Indicar codigo 0 de final
		//dumpmemoria[current_tile_x+TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+3]=0;  //3 espacios al inicio

		//menu_escribe_linea_opcion(linea++,-1,1,&dumpmemoria[current_tile_x]); //Mostrar regla superior
		zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,0,dumpmemoria);
	}
	else {
		//Aumentarlo en cuanto al offset que estamos (si modo lista)

		int offset_y=menu_debug_tsconf_tbblue_tilenav_lista_tiles_window->offset_y;
		

		offset_vertical=offset_y/2;
		linea=offset_vertical*2;



		limite_vertical=offset_vertical+((24-2)/2)+1; //El maximo que cabe en pantalla, +1 para cuando se baja 1 posicion con cursor

	}

	//linea destino es +3, pues las tres primeras son de leyenda
	linea +=3;	



		for (;offset_vertical<limite_vertical;offset_vertical++) {

			int repetir_ancho=1;
			int mapa_tile_x=3;
			if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
				//Modo lista tiles
				current_tile=offset_vertical;
			}

			else {
				//Modo mapa tiles
				if (MACHINE_IS_TSCONF) {
					current_tile=offset_vertical*64;
					repetir_ancho=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW;

					//poner regla vertical
					int linea_tile=current_tile/64;
					if ( (linea_tile%5)==0) sprintf (dumpmemoria,"%2d ",linea_tile);
					else sprintf (dumpmemoria,"   ");
				}

				else { //TBBLUE
					current_tile=offset_vertical*tilemap_width;
					repetir_ancho=tilemap_width;

					//poner regla vertical
					int linea_tile=current_tile/tilemap_width;
					if ( (linea_tile%5)==0) sprintf (dumpmemoria,"%2d ",linea_tile);
					else sprintf (dumpmemoria,"   ");				
				}
			}

			//printf ("linea: %3d current tile: %10d puntero: %10d\n",linea_color,current_tile,puntero_tilemap-tsconf_ram_mem_table[0]-tsconf_return_tilemappage()	);

			do {
				if (MACHINE_IS_TSCONF) {
					int y=current_tile/64;
					int x=current_tile%64; 

					//printf ("x: %d y: %d\n",x,y);
				

					int offset=(256*y)+(x*2);

					offset+=menu_debug_tsconf_tbblue_tilenav_current_tilelayer*128;

					int tnum=puntero_tilemap[offset]+256*(puntero_tilemap[offset+1]&0xF);

					//printf ("Current tile: %d  x: %d y: %d  tnum: %d\n",current_tile,x,y,tnum);

					z80_byte tnum_x=tnum&63;
					z80_byte tnum_y=(tnum>>6)&63;

		    		z80_byte tpal=(puntero_tilemap[offset+1]>>4)&3;

					z80_byte tile_xf=puntero_tilemap[offset+1]&64;
					z80_byte tile_yf=puntero_tilemap[offset+1]&128;

					if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
						//Modo lista tiles
						sprintf (dumpmemoria,"X: %3d Y: %3d                   ",x,y);

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

						sprintf (dumpmemoria," Tile: %2d,%2d %s %s P:%2d",tnum_x,tnum_y,
							(tile_xf ? "XF" : "  "),(tile_yf ? "YF": "  "),
							tpal );

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
					}
					else {
						//Modo mapa tiles
						z80_byte caracter_final;

						if (tnum==0) {
							caracter_final=' '; 
						}
						else {
							caracter_final=menu_debug_tsconf_tbblue_tiles_retorna_visualchar(tnum);
						}

						dumpmemoria[mapa_tile_x++]=caracter_final;
					}
				}

				if (MACHINE_IS_TBBLUE) {

					int y=current_tile/tilemap_width;
					int x=current_tile%tilemap_width; 

					int offset=(tilemap_width*tbblue_bytes_per_tile*y)+(x*tbblue_bytes_per_tile);
					/*
					 bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
  bits   7-0 : tile number
					*/

					int xmirror,ymirror,rotate;
					z80_byte tpal;

					z80_byte byte_first;
					z80_byte byte_second;

					byte_first=puntero_tilemap[offset];
					byte_second=puntero_tilemap[offset+1];					

					int tnum=byte_first;
					int ula_over_tilemap;

					z80_byte tbblue_default_tilemap_attr=tbblue_registers[108];

					if (tbblue_bytes_per_tile==1) {
						/*
						(R/W) 0x6C (108) => Default Tilemap Attribute
  bits 7-4 = Palette Offset
  bit 3    = X mirror
  bit 2    = Y mirror
  bit 1    = Rotate
  bit 0    = ULA over tilemap
             (bit 8 of tile id if the ULA is disabled)	
			 			*/
					 	tpal=(tbblue_default_tilemap_attr>>4)&15;
						xmirror=(tbblue_default_tilemap_attr>>3)&1;
						ymirror=(tbblue_default_tilemap_attr>>2)&1;
						rotate=(tbblue_default_tilemap_attr>>1)&1;

						if (tbblue_if_ula_is_enabled() ) {
						/*
						108
						  bit 0    = ULA over tilemap
             (bit 8 of tile id if the ULA is disabled)
						*/							
							ula_over_tilemap=tbblue_default_tilemap_attr &1;
						}

						else {
							tnum |=(tbblue_default_tilemap_attr&1)<<8; // bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						}
						
					}

					else {
						/*
							
					 bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
					*/	
					 	tpal=(byte_second>>4)&15;
						xmirror=(byte_second>>3)&1;
						ymirror=(byte_second>>2)&1;
						rotate=(byte_second>>1)&1;
						//ula_over_tilemap=byte_second &1;

					if (tbblue_if_ula_is_enabled() ) {
						/*
						  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						*/							
							ula_over_tilemap=byte_second &1;
						}

						else {
							tnum |=(byte_second&1)<<8; // bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						}


					}

					//printf ("tnum: %d\n",tnum);


					if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
						//Modo lista tiles
						sprintf (dumpmemoria,"X: %3d Y: %3d                   ",x,y);

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

						sprintf (dumpmemoria," Tile: %3d %s %s %s %s P:%2d ",tnum,
							(xmirror ? "MX" : "  "),(ymirror ? "MY": "  "),
							(rotate ? "R" : " "),(ula_over_tilemap ? "U": " "),
							tpal );

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
					}
					else {
						//Modo mapa tiles
						int caracter_final;

						if (tnum==0) {
							caracter_final=' '; 
						}
						else {
							caracter_final=menu_debug_tsconf_tbblue_tiles_retorna_visualchar(tnum);
						}

						dumpmemoria[mapa_tile_x++]=caracter_final;
					}					


				}

				current_tile++;

				repetir_ancho--;
			} while (repetir_ancho);

			if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
				puntero_tilemap=puntero_tilemap_orig;
			}
					
		}



	//return linea;

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window); 
}

void menu_debug_tsconf_tbblue_tilenav_draw_tiles(void)
{
/*
				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista tiles
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();

				//Esto tiene que estar despues de escribir la lista de tiles, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();
				*/



				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();				

}



void menu_debug_tsconf_tbblue_tilenav_new_window(zxvision_window *ventana)
{

		char titulo[33];
		
		char linea_leyenda[64];
		sprintf (titulo,"Tile Navigator");

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;		

		int total_height=menu_debug_tsconf_tbblue_tilenav_total_vert();
		int total_width=31;

		char texto_layer[32];

		//En caso de tbblue, solo hay una capa
		if (MACHINE_IS_TBBLUE) texto_layer[0]=0;

		else sprintf (texto_layer,"~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);

		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (linea_leyenda,"~~Mode: Visual %s",texto_layer);

			if (MACHINE_IS_TSCONF) {
			total_width=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+4;
			}
			else {
				//TBBLUE
				total_width=tbblue_get_tilemap_width()+4;
			}

		}

		else {
			sprintf (linea_leyenda,"~~Mode: List %s",texto_layer);
			total_height*=2;
		}

		//tres mas para ubicar las lineas de leyenda
		total_height+=3;

		zxvision_new_window(ventana,TSCONF_TILENAV_WINDOW_X,TSCONF_TILENAV_WINDOW_Y,TSCONF_TILENAV_WINDOW_ANCHO,TSCONF_TILENAV_WINDOW_ALTO,
							total_width,total_height,titulo);


		//Establecer leyenda en la parte de abajo
		ventana->lower_margin=2;
		//Texto sera el de la primera linea
		ventana->upper_margin=1;


		
		//Leyenda inferior
		//zxvision_print_string_defaults_fillspc(ventana,1,1,"-----");
		zxvision_print_string_defaults_fillspc(ventana,1,2,linea_leyenda);

		zxvision_draw_window(ventana);	

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;
		//Nota: los atajos se "pintan" en la memoria de la ventana ya con el color inverso
		//por tanto con meter al principio la variable de inverse_color es suficiente
		//y no hay que activar inverse color cada vez que se redibuja ventana,
		//pues al redibujar ventana está leyendo el contenido de la memoria de la ventana, y ahí ya está con color inverso		

}

void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
		zxvision_window ventana;


		menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);



        set_menu_overlay_function(menu_debug_tsconf_tbblue_tilenav_draw_tiles);


		menu_debug_tsconf_tbblue_tilenav_lista_tiles_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


		z80_byte tecla;

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			//printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}				


	do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech


			

		tecla=zxvision_common_getkey_refresh();				

        
				switch (tecla) {

					case 'l':
						//En caso de tbblue, hay una sola capa
						if (!MACHINE_IS_TBBLUE) {					
							zxvision_destroy_window(&ventana);	
							menu_debug_tsconf_tbblue_tilenav_current_tilelayer ^=1;
							menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);
						}
					break;

					case 'm':

						zxvision_destroy_window(&ventana);		
						menu_debug_tsconf_tbblue_tilenav_showmap.v ^=1;
						menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);

					break;


					default:
						zxvision_handle_cursors_pgupdn(&ventana,tecla);
					break;
				}		

		


	} while (tecla!=2); 

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		


	
    

}




#define SOUND_WAVE_X (menu_origin_x()+1)
#define SOUND_WAVE_Y 3
#define SOUND_WAVE_ANCHO 30
#define SOUND_WAVE_ALTO 15

int menu_sound_wave_llena=1;
int menu_audio_draw_sound_wave_ycentro;

char menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_max,menu_audio_draw_sound_wave_valor_min;
int menu_audio_draw_sound_wave_frecuencia_aproximada;

int menu_audio_draw_sound_wave_volumen=0;
int menu_audio_draw_sound_wave_volumen_escalado=0;


//Usado dentro del overlay de waveform, para mostrar dos veces por segundo el texto que average, etc
int menu_waveform_valor_contador_segundo_anterior;

int menu_waveform_previous_volume=0;



zxvision_window *menu_audio_draw_sound_wave_window;

void menu_audio_draw_sound_wave(void)
{

	if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	//workaround_pentagon_clear_putpixel_cache();

				char buffer_texto_medio[40]; //32+3+margen de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	//esto hara ejecutar esto 2 veces por segundo
	if ( ((contador_segundo%500) == 0 && menu_waveform_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

		menu_waveform_valor_contador_segundo_anterior=contador_segundo;
		//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

		//menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

			//Average, min, max    

			sprintf (buffer_texto_medio,"Av.: %d Min: %d Max: %d",
			menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_min,menu_audio_draw_sound_wave_valor_max);


			//menu_escribe_linea_opcion(1,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,1,buffer_texto_medio);


	

			//Hacer decaer el volumen
			//if (menu_waveform_previous_volume>menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume--;
			menu_waveform_previous_volume=menu_decae_dec_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);


			//Frecuency
			sprintf (buffer_texto_medio,"Average freq: %d Hz (%s)",
				menu_audio_draw_sound_wave_frecuencia_aproximada,get_note_name(menu_audio_draw_sound_wave_frecuencia_aproximada));
			//menu_escribe_linea_opcion(3,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,3,buffer_texto_medio);

			//printf ("menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);
	}


	int ancho;
	//ancho=(SOUND_WAVE_ANCHO-2);

	//Ancho de zona waveform variable segun el tamanyo de ventana
	ancho=menu_audio_draw_sound_wave_window->visible_width-2;

	//Por si acaso, no vayamos a provocar alguna division por cero
	if (ancho<1) ancho=1;

	int alto;

	int lineas_cabecera=4;

	alto=menu_audio_draw_sound_wave_window->visible_height-lineas_cabecera-2;

	//Por si acaso, no vayamos a provocar alguna division por cero
	if (alto<1) alto=1;

	//int xorigen=(SOUND_WAVE_X+1);
	//int yorigen=(SOUND_WAVE_Y+4);

	int xorigen=1;
	int yorigen;

	//yorigen=lineas_cabecera+alto/2;

	//if (yorigen<lineas_cabecera) yorigen=lineas_cabecera;
	yorigen=lineas_cabecera;


	if (si_complete_video_driver() ) {
        	ancho *=menu_char_width;
	        alto *=8;
        	xorigen *=menu_char_width;
	        yorigen *=8;
	}


	//int ycentro=yorigen+alto/2;
	menu_audio_draw_sound_wave_ycentro=yorigen+alto/2;

	int x,y,lasty;


	//Para drivers de texto, borrar zona

	if (!si_complete_video_driver() ) {
	        for (x=xorigen;x<xorigen+ancho;x++) {
        	        for (y=yorigen;y<yorigen+alto;y++) {
				//putchar_menu_overlay(x,y,' ',0,7);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);

				zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
	                }
        	}
	}


	

	audiobuffer_stats audiostats;
	audio_get_audiobuffer_stats(&audiostats);


	menu_audio_draw_sound_wave_valor_max=audiostats.maximo;
	menu_audio_draw_sound_wave_valor_min=audiostats.minimo;
	menu_audio_draw_sound_wave_frecuencia_aproximada=audiostats.frecuencia;
	menu_audio_draw_sound_wave_volumen=audiostats.volumen;
	menu_audio_draw_sound_wave_volumen_escalado=audiostats.volumen_escalado;

	int audiomedio=audiostats.medio;
	menu_audio_draw_sound_wave_valor_medio=audiomedio;
        audiomedio=audiomedio*alto/256;

        //Lo situamos en el centro. Negativo hacia abajo (Y positiva)
        audiomedio=menu_audio_draw_sound_wave_ycentro-audiomedio;

	int puntero_audio=0;
	char valor_audio;
	for (x=xorigen;x<xorigen+ancho;x++) {

		//Obtenemos valor medio de audio
		int valor_medio=0;

		//Calcular cuantos valores representa un pixel, teniendo en cuenta maximo buffer
		const int max_valores=AUDIO_BUFFER_SIZE/ancho;

		int valores=max_valores;
		for (;valores>0;valores--,puntero_audio++) {
			if (puntero_audio>=AUDIO_BUFFER_SIZE) {
				//por si el calculo no es correcto, salir.
				//esto no deberia suceder ya que el calculo de max_valores se hace en base al maximo
				cpu_panic("menu_audio_draw_sound_wave: pointer beyond AUDIO_BUFFER_SIZE");
			}

			//stereo 
			//if (audio_driver_accepts_stereo.v) {
				int suma_canales=audio_buffer[puntero_audio*2]+audio_buffer[(puntero_audio*2)+1];
				suma_canales /=2;
				valor_medio=valor_medio+suma_canales;
			//}

			//else valor_medio=valor_medio+audio_buffer[puntero_audio];


		}

		valor_medio=valor_medio/max_valores;

		valor_audio=valor_medio;

		//Lo escalamos a maximo alto

		y=valor_audio;
		y=valor_audio*alto/256;

		//Lo situamos en el centro. Negativo hacia abajo (Y positiva)
		y=menu_audio_draw_sound_wave_ycentro-y;


		//unimos valor anterior con actual con una linea vertical
		if (x!=xorigen) {
			if (si_complete_video_driver() ) {

				//Onda no llena
				if (!menu_sound_wave_llena) menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,lasty,y,ESTILO_GUI_COLOR_WAVEFORM);

        			//dibujar la onda "llena", es decir, siempre contar desde centro
			        //el centro de la y de la onda es variable... se saca valor medio de todos los valores mostrados en pantalla

				//Onda llena
				else menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,audiomedio,y,ESTILO_GUI_COLOR_WAVEFORM);



			}
		}

		lasty=y;

		//dibujamos valor actual
		if (si_complete_video_driver() ) {
			zxvision_putpixel(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_COLOR_WAVEFORM);
		}

		else {
			//putchar_menu_overlay(SOUND_WAVE_X+x,SOUND_WAVE_Y+y,'#',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);
			zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
		}


	}

	//printf ("%d ",puntero_audio);


	//Volume. Mostrarlo siempre, no solo dos veces por segundo, para que se actualice mas frecuentemente
	//if (menu_waveform_previous_volume<menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume=menu_audio_draw_sound_wave_volumen_escalado;
	menu_waveform_previous_volume=menu_decae_ajusta_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);

	char texto_volumen[32]; 
    menu_string_volumen(texto_volumen,menu_audio_draw_sound_wave_volumen_escalado,menu_waveform_previous_volume);
                                                                //"Volume C: %s"

	sprintf (buffer_texto_medio,"Volume: %3d %s",menu_audio_draw_sound_wave_volumen,texto_volumen);
	//menu_escribe_linea_opcion(2,-1,1,buffer_texto_medio);
	zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,2,buffer_texto_medio);


	zxvision_draw_window_contents(menu_audio_draw_sound_wave_window); 

}




void menu_audio_new_waveform_shape(MENU_ITEM_PARAMETERS)
{
	menu_sound_wave_llena ^=1;
}



void menu_audio_new_waveform(MENU_ITEM_PARAMETERS)
{

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int x,y,ancho,alto;

	if (!util_find_window_geometry("waveform",&x,&y,&ancho,&alto)) {
		x=SOUND_WAVE_X;
		y=SOUND_WAVE_Y-2;
		ancho=SOUND_WAVE_ANCHO;
		alto=SOUND_WAVE_ALTO+4;
	}

	zxvision_new_window_nocheck_staticsize(&ventana,x,y,ancho,alto,ancho-1,alto-2,"Waveform");
	zxvision_draw_window(&ventana);		

    
    //Cambiamos funcion overlay de texto de menu
    //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_draw_sound_wave);

	menu_audio_draw_sound_wave_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

	menu_item *array_menu_audio_new_waveform;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"[%s] Wave ~~Shape",
				(menu_sound_wave_llena ? "Fill" : "Line") );
        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						
		menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);


		//Nombre de ventana solo aparece en el caso de stdout
    	retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );


		//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
		cls_menu_overlay();
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
        	//llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
		//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();

	//Grabar geometria ventana
	util_add_window_geometry("waveform",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);

}


zxvision_window *menu_debug_draw_visualmem_window;



#ifdef EMULATE_VISUALMEM


//#define visualmem_ancho_variable (menu_debug_draw_visualmem_window->visible_width-1)
//#define visualmem_alto_variable (menu_debug_draw_visualmem_window->visible_height-1)

#define VISUALMEM_MIN_X (menu_origin_x())
#define VISUALMEM_MIN_Y 0

#define VISUALMEM_DEFAULT_X (VISUALMEM_MIN_X+1)

//int visualmem_x_variable=VISUALMEM_DEFAULT_X;

#define VISUALMEM_DEFAULT_Y (VISUALMEM_MIN_Y+1)
int visualmem_y_variable=VISUALMEM_DEFAULT_Y;

#define VISUALMEM_ANCHO (menu_debug_draw_visualmem_window->visible_width)
#define VISUALMEM_ALTO (menu_debug_draw_visualmem_window->visible_height)

#define VISUALMEM_DEFAULT_WINDOW_ANCHO 30
#define VISUALMEM_DEFAULT_WINDOW_ALTO 22

//0=vemos visualmem write
//1=vemos visualmem read
//2=vemos visualmem opcode
//3=vemos todos a la vez
int menu_visualmem_donde=0;


int visualmem_bright_multiplier=10;


void menu_debug_draw_visualmem(void)
{

        normal_overlay_texto_menu();

		//workaround_pentagon_clear_putpixel_cache();


        int ancho=(VISUALMEM_ANCHO-2);
        int alto=(VISUALMEM_ALTO-5);

		if (ancho<1 || alto<1) return;

        int xorigen=1;
        int yorigen=3;


        if (si_complete_video_driver() ) {
                ancho *=menu_char_width;
                alto *=8;
                xorigen *=menu_char_width;
                yorigen *=8;
        }


	int tamanyo_total=ancho*alto;

        int x,y;


        int inicio_puntero_membuffer,final_puntero_membuffer;

	//Por defecto
	inicio_puntero_membuffer=16384;
	final_puntero_membuffer=65536;

	//printf ("ancho: %d alto: %d\n",ancho,alto);

	//En spectrum 16kb
	if (MACHINE_IS_SPECTRUM_16) {
		//printf ("spec 16kb\n");
		final_puntero_membuffer=32768;
	}

	if (MACHINE_IS_Z88) {
		        inicio_puntero_membuffer=0;
	}

	//En Inves, mostramos modificaciones a la RAM baja
	if (MACHINE_IS_INVES) {
                        inicio_puntero_membuffer=0;
        }



	//En zx80, zx81 mostrar desde 8192 por si tenemos expansion packs
	if (MACHINE_IS_ZX8081) {
		//por defecto
		//printf ("ramtop_zx8081: %d\n",ramtop_zx8081);
		final_puntero_membuffer=ramtop_zx8081+1;

		if (ram_in_8192.v) {
			//printf ("memoria en 8192\n");
			inicio_puntero_membuffer=8192;
		}

        	if (ram_in_32768.v) {
			//printf ("memoria en 32768\n");
			final_puntero_membuffer=49152;
		}

	        if (ram_in_49152.v) {
			//printf ("memoria en 49152\n");
			final_puntero_membuffer=65536;
		}

	}

        //En Jupiter Ace, desde 8192
        if (MACHINE_IS_ACE) {
                //por defecto
                final_puntero_membuffer=ramtop_ace+1;
                inicio_puntero_membuffer=8192;

        }


	//En CPC, desde 0
	if (MACHINE_IS_CPC) {
		inicio_puntero_membuffer=0;
	}

	if (MACHINE_IS_SAM) {
                inicio_puntero_membuffer=0;
        }



	//En modos de RAM en ROM de +2a en puntero apuntara a direccion 0
	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		if ( (puerto_32765 & 32) == 0 ) {
			//paginacion habilitada

			if ( (puerto_8189 & 1) ) {
				//paginacion de ram en rom
				//printf ("paginacion de ram en rom\n");
				inicio_puntero_membuffer=0;
			}
		}
	}

	if (MACHINE_IS_QL) {
		//inicio_puntero_membuffer=0x18000;
		//la ram propiamente empieza en 20000H
		inicio_puntero_membuffer=0x20000;
		final_puntero_membuffer=QL_MEM_LIMIT+1;
	}

	//Si es de opcode o read, puede ser desde cualquier sitio desde la rom
	if (menu_visualmem_donde>0) {
		inicio_puntero_membuffer=0;
	}

	//Valores entre 0 y 255: numero de veces byte modificado
	//Valor 65535 especial
        //int si_modificado;


             //Calcular cuantos bytes modificados representa un pixel, teniendo en cuenta maximo buffer
	int max_valores=(final_puntero_membuffer-inicio_puntero_membuffer)/tamanyo_total;

	//printf ("max_valores: %d\n",max_valores);
	//printf ("tamanyo total: %d\n",tamanyo_total);
	//le damos uno mas para poder llenar la ventana
	//printf ("inicio: %06XH final: %06XH\n",inicio_puntero_membuffer,final_puntero_membuffer);
	max_valores++;

	for (y=yorigen;y<yorigen+alto;y++) {
        for (x=xorigen;x<xorigen+ancho;x++) {

                //Obtenemos conjunto de bytes modificados

                int valores=max_valores;

		int acumulado=0;

		int acumulado_written,acumulado_read,acumulado_opcode;
		acumulado_written=acumulado_read=acumulado_opcode=0; //Estos usados al visualizar los 3 a la vez

		//si_modificado=0;
                for (;valores>0;valores--,inicio_puntero_membuffer++) {
                        if (inicio_puntero_membuffer>=final_puntero_membuffer) {
				//printf ("llegado a final con x: %d y: %d ",x,y);
				//Fuera de memoria direccionable. Zona gris. Decrementamos valor
				//Como se lee a trozos de "max_valores" tamanyo, cuando este trozo empieza ya fuera de memoria
				//acumulado acabara siendo <0 y saldra gris. Si es a medias, si acaba restando mucho, saldra gris tambien
				//(eso solo pasara en el ultimo pixel de la zona direccionable)
				acumulado--;
                        }
			else {
				//Es en memoria direccionable. Sumar valor de visualmem y luego haremos valor medio
				//0: written, 1: read, 2: opcode
				if (menu_visualmem_donde==0) {
					acumulado +=visualmem_buffer[inicio_puntero_membuffer];
					clear_visualmembuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==1) {
					acumulado +=visualmem_read_buffer[inicio_puntero_membuffer];
					clear_visualmemreadbuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==2) {
					acumulado +=visualmem_opcode_buffer[inicio_puntero_membuffer];
					clear_visualmemopcodebuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==3) {
					acumulado_written +=visualmem_buffer[inicio_puntero_membuffer];
					acumulado_read +=visualmem_read_buffer[inicio_puntero_membuffer];
					acumulado_opcode +=visualmem_opcode_buffer[inicio_puntero_membuffer];
					clear_visualmembuffer(inicio_puntero_membuffer);
					clear_visualmemreadbuffer(inicio_puntero_membuffer);
					clear_visualmemopcodebuffer(inicio_puntero_membuffer);
				}				


			}
                }
		//if (acumulado>0) printf ("final pixel %d %d (divisor: %d)\n",inicio_puntero_membuffer,acumulado,max_valores);

            //dibujamos valor medio
            if (acumulado>0 || acumulado_written>0 || acumulado_read>0 || acumulado_opcode>0) {

			if (si_complete_video_driver() ) {

				//Sacar valor medio
				int color_final=acumulado/max_valores;

				//printf ("color final: %d\n",color_final);

				//Aumentar el brillo del color
				color_final=color_final*visualmem_bright_multiplier;
				if (color_final>255) color_final=255;




				if (menu_visualmem_donde==3) {
					//Los 3 a la vez. Combinamos color RGB sacando color de paleta tsconf (15 bits)
					//Paleta es RGB R: 5 bits altos, G: 5 bits medios, B:5 bits bajos


					//Sacar valor medio de los 3 componentes
					int color_final_written=acumulado_written/max_valores;
					color_final_written=color_final_written*visualmem_bright_multiplier;
					if (color_final_written>31) color_final_written=31;

					int color_final_read=acumulado_read/max_valores;
					color_final_read=color_final_read*visualmem_bright_multiplier;
					if (color_final_read>31) color_final_read=31;		

					int color_final_opcode=acumulado_opcode/max_valores;
					color_final_opcode=color_final_opcode*visualmem_bright_multiplier;
					if (color_final_opcode>31) color_final_opcode=31;	

					//Blue sera para los written
					//Green sera para los read
					//Red sera para los opcode
					int color_final_rgb=(color_final_opcode<<10)|(color_final_read<<5)|color_final_written;
					zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,TSCONF_INDEX_FIRST_COLOR+color_final_rgb);

				}
				else zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,HEATMAP_INDEX_FIRST_COLOR+color_final);
			}

			else {
				//putchar_menu_overlay(x,y,'#',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
			}
		}

		//color ficticio para indicar fuera de memoria y por tanto final de ventana... para saber donde acaba
		else if (acumulado<0) {
			if (si_complete_video_driver() ) {
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_COLOR_UNUSED_VISUALMEM);
			}
			else {
				//putchar_menu_overlay(x,y,'-',ESTILO_GUI_COLOR_UNUSED_VISUALMEM,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'-');
            }

		}

		//Valor 0
		else {
			if (si_complete_video_driver() ) {
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_PAPEL_NORMAL);
			}
			else {
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
			}
		}

        }
	}

	zxvision_draw_window_contents(menu_debug_draw_visualmem_window); 

}









void menu_debug_new_visualmem_looking(MENU_ITEM_PARAMETERS)
{
	menu_visualmem_donde++;
	if (menu_visualmem_donde==4) menu_visualmem_donde=0;
}


void menu_debug_new_visualmem_bright(MENU_ITEM_PARAMETERS)
{
                        if (visualmem_bright_multiplier>=200) visualmem_bright_multiplier=1;
                        else if (visualmem_bright_multiplier==1) visualmem_bright_multiplier=10;
                        else visualmem_bright_multiplier +=10;
}


void menu_debug_new_visualmem(MENU_ITEM_PARAMETERS)
{


 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int x,y,ancho,alto;


	if (!util_find_window_geometry("visualmem",&x,&y,&ancho,&alto)) {
		x=VISUALMEM_DEFAULT_X;
		y=visualmem_y_variable;
		ancho=VISUALMEM_DEFAULT_WINDOW_ANCHO;
		alto=VISUALMEM_DEFAULT_WINDOW_ALTO;
	}


	zxvision_new_window_nocheck_staticsize(&ventana,x,y,ancho,alto,ancho-1,alto-2,"Visual memory");
	zxvision_draw_window(&ventana);				


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de visualmem + texto
        set_menu_overlay_function(menu_debug_draw_visualmem);


	menu_debug_draw_visualmem_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		


	menu_dibuja_menu_permite_repeticiones_hotk=1;


	menu_item *array_menu_debug_new_visualmem;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


	


	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
	menu_writing_inverse_color.v=1;


	char texto_linea[33];
	//sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
	sprintf (texto_linea,"~~Bright: %d",visualmem_bright_multiplier);
	//menu_escribe_linea_opcion(0,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,0,texto_linea);



	if (menu_visualmem_donde == 0) sprintf (texto_linea,"~~Looking: Written Mem");
	else if (menu_visualmem_donde == 1) sprintf (texto_linea,"~~Looking: Read Mem");
	else if (menu_visualmem_donde == 2) sprintf (texto_linea,"~~Looking: Opcode");
	else sprintf (texto_linea,"~~Looking: All");


	//sprintf (texto_linea,"~~Looking: %s",(menu_visualmem_donde == 0 ? "Written Mem" : "Opcode") );
	//menu_escribe_linea_opcion(1,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,1,texto_linea);


	//Restaurar comportamiento atajos
	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


	//Que sentido tiene el texto de antes? Si vamos a abrir menu con las mismas lineas....

//        char texto_linea[33];
//        sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
//        menu_escribe_linea_opcion(VISUALMEM_Y,-1,1,texto_linea);



						menu_add_item_menu_inicial_format(&array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        //menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'b');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Change bright value");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Change bright value");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,0);


			char texto_looking[32];
	        	if (menu_visualmem_donde == 0) sprintf (texto_looking,"Written Mem");
        		else if (menu_visualmem_donde == 1) sprintf (texto_looking,"Read Mem");
		        else if (menu_visualmem_donde == 2) sprintf (texto_looking,"Opcode");
				else sprintf (texto_looking,"All");

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_looking,NULL,"~~Looking: %s",texto_looking);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'l');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Which visualmem to look at");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Which visualmem to look at. If you select all, the final color will be a RGB color result of:\n"
									"Blue component por Written Mem\nGreen component for Read mem\nRed component for Opcode.\n"
									"Yellow for example is red+green, so opcode fetch+read memory. As an opcode fetch implies a read access,"
									" you won't ever see a red pixel (only opcode fetch) but all opcode fetch will always be yellow.\n"
									"Cyan is green+blue, so read+write\n"
									
									);
                        menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,1);



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&debug_new_visualmem_opcion_seleccionada,&item_seleccionado,array_menu_debug_new_visualmem,"Visual memory" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



	menu_dibuja_menu_permite_repeticiones_hotk=0;



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();

	util_add_window_geometry("visualmem",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);		


}





#endif




void menu_ay_player_load(MENU_ITEM_PARAMETERS)
{
	char *filtros[2];

	filtros[0]="ay";

	filtros[1]=0;

	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de ultimo archivo
	//si no hay directorio, vamos a rutas predefinidas
	if (last_ay_file[0]==0) menu_chdir_sharedfiles();

	else {
					char directorio[PATH_MAX];
					util_get_dir(last_ay_file,directorio);
					//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

					//cambiamos a ese directorio, siempre que no sea nulo
					if (directorio[0]!=0) {
									debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
									menu_filesel_chdir(directorio);
					}
	}


	int ret;

	ret=menu_filesel("Select AY File",filtros,last_ay_file);
	//volvemos a directorio inicial
	menu_filesel_chdir(directorio_actual);


	if (ret==1) {


		ay_player_load_and_play(last_ay_file);

	}
}

/*
void menu_ay_player_exit_tracker(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();
}
*/

//Retorna indice a string teniendo en cuenta maximo en pantalla e incrementando en 1
int menu_ay_player_get_continuous_string(int indice_actual,int max_length,char *string,int *retardo)
{

	if ( (*retardo)<10 ) {
		(*retardo)++;
		return 0;
	}

	int longitud=strlen(&string[indice_actual]);
	if (longitud<=max_length) {
		indice_actual=0;
		*retardo=0;
	}
	else {
		indice_actual++;
	}

	return indice_actual;
}





int menu_audio_new_ayplayer_si_mostrar(void)
{
	int mostrar_player;


	mostrar_player=1;
	if (audio_ay_player_mem==NULL) mostrar_player=0;
	if (ay_player_playing.v==0) mostrar_player=0;

	return mostrar_player;
}


//Usado dentro del overlay de ayplayer
int menu_ayplayer_valor_contador_segundo_anterior;

int ayplayer_new_contador_string_author=0;
int ayplayer_new_contador_string_track_name=0;
int ayplayer_new_contador_string_misc=0;
int ayplayer_new_retardo_song_name=0;
int ayplayer_new_retardo_author=0;
int ayplayer_new_retardo_misc=0;


//Para hacer las barras de volumen con "caracter" que decae
int ayplayer_previo_valor_escalado=0;

int ayplayer_previo_valor_volume_A=0;
int ayplayer_previo_valor_volume_B=0;
int ayplayer_previo_valor_volume_C=0;

zxvision_window *menu_audio_new_ayplayer_overlay_window;	

void menu_audio_new_ayplayer_overlay(void)
{

    normal_overlay_texto_menu();

                        int linea;


    linea=7;
	int valor_escalado; 

	int vol_A,vol_B,vol_C;

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


    if (menu_audio_new_ayplayer_si_mostrar()) {
    	//Los volumenes mostrarlos siempre a cada refresco
	char volumen[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	vol_A=ay_3_8912_registros[0][8] & 15;
	vol_B=ay_3_8912_registros[0][9] & 15;
	vol_C=ay_3_8912_registros[0][10] & 15;

	ayplayer_previo_valor_volume_A=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);





	menu_string_volumen(volumen,ay_3_8912_registros[0][8],ayplayer_previo_valor_volume_A);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][9],ayplayer_previo_valor_volume_B);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][10],ayplayer_previo_valor_volume_C);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);



	//Obtenemos antes valor medio total y tambien maximo y minimo
	//Esto solo es necesario para dibujar onda llena



        audiobuffer_stats audiostats;
        audio_get_audiobuffer_stats(&audiostats);

	//int volumen_buffer=audiostats.volumen;

	//Ahora tenemos valor entre 0 y 128. Pasar a entre 0 y 15 
	//int valor_escalado=(mayor*16)/128;

	valor_escalado=audiostats.volumen_escalado;

	/*

	int valor_escalado=(volumen_buffer*16)/128;

	//Vigilar que no pase de 15
	if (valor_escalado>15) valor_escalado=15;
	*/

	//printf ("audiomin: %d audiomax: %d maximo: %d valor_escalado: %d\n",audiomin,audiomax,mayor,valor_escalado);

	//Y mostramos indicador volumen
	/*
	Nota: realmente este calculo de volumen no es del todo cierto, estoy viendo el valor maximo de la onda, aunque se puede generar
	sonido muy bajo, por ejemplo, oscilando valores entre 100 y 120 (considerando signed 8 bits), es mas, hay juegos que, al usar beeper,
	"mueven" esa onda hacia arriba, y aunque el indicador de volumen diga que esta muy alto, realmente se oye a volumen normal
	Pero bueno, la mayoria de las veces si que coincide bien el valor de volumen
	*/
	

	ayplayer_previo_valor_escalado=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);
	//if (ayplayer_previo_valor_escalado<valor_escalado) ayplayer_previo_valor_escalado=valor_escalado;

			menu_string_volumen(volumen,valor_escalado,ayplayer_previo_valor_escalado);




								//"Volume C: %s"
			sprintf (textovolumen,"Output:   %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

	}

	else {
		//Borrar lineas
		int i;
		linea=0;
		for (i=0;i<11;i++) zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"");
	}

    //esto hara ejecutar esto 2 veces por segundo
    if ( ((contador_segundo%500) == 0 && menu_ayplayer_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

        menu_ayplayer_valor_contador_segundo_anterior=contador_segundo;
        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
       
    


char textoplayer[40];

       


int mostrar_player;



	mostrar_player=menu_audio_new_ayplayer_si_mostrar();

       


		if (mostrar_player) {

			linea=0;

	//Indicadores de volumen que decaen
	ayplayer_previo_valor_escalado=menu_decae_dec_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);


	ayplayer_previo_valor_volume_A=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);




			//printf ("Dibujando player\n");

			z80_byte minutos,segundos,minutos_total,segundos_total;
			minutos=ay_song_length_counter/60/50;
			segundos=(ay_song_length_counter/50)%60;
			minutos_total=ay_song_length/60/50;
			segundos_total=(ay_song_length/50)%60;

//printf ("segundo. contador segundo: %d\n",contador_segundo);

			sprintf (textoplayer,"Track: %03d/%03d  (%02d:%02d/%02d:%02d)",ay_player_pista_actual,ay_player_total_songs(),minutos,segundos,minutos_total,segundos_total);
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);


			strncpy(textoplayer,&ay_player_file_song_name[ayplayer_new_contador_string_track_name],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_track_name=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_track_name,28,ay_player_file_song_name,&ayplayer_new_retardo_song_name);

			//menu_escribe_linea_opcion(linea++,-1,1,"Author");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Author");
			strncpy(textoplayer,&ay_player_file_author[ayplayer_new_contador_string_author],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_author=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_author,28,ay_player_file_author,&ayplayer_new_retardo_author);

			//menu_escribe_linea_opcion(linea++,-1,1,"Misc");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Misc");
			strncpy(textoplayer,&ay_player_file_misc[ayplayer_new_contador_string_misc],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_misc=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_misc,28,ay_player_file_misc,&ayplayer_new_retardo_misc);




		}
	}

	zxvision_draw_window_contents(menu_audio_new_ayplayer_overlay_window);
}



void menu_audio_new_ayplayer_load(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	menu_ay_player_load(0);

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);

}

void menu_audio_new_ayplayer_prev(MENU_ITEM_PARAMETERS)
{
	ay_player_previous_track();

}

void menu_audio_new_ayplayer_next(MENU_ITEM_PARAMETERS)
{
	ay_player_next_track();

}

void menu_audio_new_ayplayer_stop(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();

}

void menu_audio_new_ayplayer_repeat(MENU_ITEM_PARAMETERS)
{
	ay_player_repeat_file.v ^=1;

}


void menu_audio_new_ayplayer_exitend(MENU_ITEM_PARAMETERS)
{
	ay_player_exit_emulator_when_finish.v ^=1;
}

void menu_audio_new_ayplayer_cpcmode(MENU_ITEM_PARAMETERS)
{
	ay_player_cpc_mode.v ^=1;
	audio_ay_player_play_song(ay_player_pista_actual);
}

												

void menu_audio_new_ayplayer_inftracks(MENU_ITEM_PARAMETERS)
{
	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_infinite_tracks/50);

        menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_infinite_tracks=l*50;

	

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}

void menu_audio_new_ayplayer_len_anytracks(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_any_track/50);

	menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_any_track=l*50;


	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}



void menu_audio_new_ayplayer(MENU_ITEM_PARAMETERS)
{


        //menu_espera_no_tecla();




				if (!menu_multitarea) {
					menu_warn_message("This menu item needs multitask enabled");
					return;
				}

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int xventana=menu_origin_x();

	zxvision_new_window(&ventana,xventana,1,32,20,
							32-1,20-2,"AY Player");
	zxvision_draw_window(&ventana);			


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);


	menu_audio_new_ayplayer_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		



	menu_item *array_menu_audio_new_ayplayer;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

        			ayplayer_new_contador_string_author=0;
				ayplayer_new_contador_string_track_name=0;
				ayplayer_new_contador_string_misc=0;

 				ayplayer_new_retardo_song_name=0;
				ayplayer_new_retardo_author=0;
				ayplayer_new_retardo_misc=0;

        	char textoplayer[40];
            //Hay que redibujar la ventana desde este bucle
            //menu_audio_new_ayplayer_dibuja_ventana();



            menu_add_item_menu_inicial_format(&array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_load,NULL,"~~Load");
            menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'l');
            menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Load AY file");

						
			

			int lin=12;

			menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+5);

				//Vamos a borrar con espacios para que no quede rastro de opciones anteriores, como Yes/No 
				//Si no, pasaria que mostraria "Nos" como parte de la s final de Yes
				int i;
				for (i=12;i<=16;i++) {
					zxvision_fill_width_spaces(&ventana,i);
				}			

			if (menu_audio_new_ayplayer_si_mostrar() ) {


				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_prev,NULL,"~~Prev");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'p');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Previous song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_stop,NULL,"~~Stop");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'s');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Stop song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,6,lin);	

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_next,NULL,"~~Next");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'n');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Next song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,11,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_repeat,NULL,"[%c] Repeat",
					(ay_player_repeat_file.v ? 'X' : ' '));

				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'r');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Repeat from the beginning when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+1);	

				
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_exitend,NULL,"[%c] Exit end",
					(ay_player_exit_emulator_when_finish.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'e');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Exit emulator when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,13,lin+1);	


				if (ay_player_limit_infinite_tracks==0) sprintf(textoplayer,"Length ~~infinite tracks: inf");
				else sprintf(textoplayer,"Length ~~infinite tracks: %d s",ay_player_limit_infinite_tracks/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_inftracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'i');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for songs which doesn't have time limit");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+2);			


				if (ay_player_limit_any_track==0) sprintf(textoplayer,"Length ~~any track: No limit");
				else sprintf(textoplayer,"Length ~~any track: %d s",ay_player_limit_any_track/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_len_anytracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'a');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+3);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_cpcmode,NULL,"[%c] CPC mode",
					(ay_player_cpc_mode.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'c');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Switch to AY CPC mode");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+4);		


			}			
/*


			sprintf(textoplayer,"~~CPC mode: %s",(ay_player_cpc_mode.v ? 'X' : ' '));
			menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
*/



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_ayplayer_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_ayplayer,"AY Player" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);				

}








#define DEBUG_HEXDUMP_WINDOW_X (menu_origin_x() )
#define DEBUG_HEXDUMP_WINDOW_Y 1
#define DEBUG_HEXDUMP_WINDOW_ANCHO 32
#define DEBUG_HEXDUMP_WINDOW_ALTO 23



void menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor)
{
	//dir_leida=adjust_address_space_cpu(dir_leida);

	menu_debug_set_memory_zone_attr();


	int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	menu_debug_print_address_memory_zone(dumpmemoria,dir_leida);

	

	//cambiamos el 0 final por un espacio
	dumpmemoria[longitud_direccion]=' ';

	menu_debug_registers_dump_hex(&dumpmemoria[longitud_direccion+1],dir_leida,bytes_por_linea);

	//01234567890123456789012345678901
	//000FFF ABCDABCDABCDABCD 12345678

	//metemos espacio
	int offset=longitud_direccion+1+bytes_por_linea*2;

	dumpmemoria[offset]=' ';
	//dumpmemoria[offset]='X';

	//Tener en cuenta el valor xor

	menu_debug_registers_dump_ascii(&dumpmemoria[offset+1],dir_leida,bytes_por_linea,menu_debug_hexdump_with_ascii_modo_ascii,valor_xor);

	//printf ("%s\n",dumpmemoria);
}


menu_z80_moto_int menu_debug_hexdump_direccion=0;

int menu_hexdump_edit_position_x=0; //Posicion del cursor relativa al inicio del volcado hexa
int menu_hexdump_edit_position_y=0; //Posicion del cursor relativa al inicio del volcado hexa
int menu_hexdump_lineas_total=13;

int menu_hexdump_edit_mode=0;
const int menu_hexdump_bytes_por_linea=8;

void menu_debug_hexdump_cursor_abajo(void);
void menu_debug_hexdump_cursor_arriba(void);



void menu_debug_hexdump_print_editcursor(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//Si multitarea esta off, no se vera el parpadeo. Entonces cambiar el caracter por cursor '_'
	if (!menu_multitarea) caracter='_';

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,1);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,1,caracter);

}

void menu_debug_hexdump_print_editcursor_nibble(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,0,caracter);

}

void menu_debug_hexdump_edit_cursor_izquierda(void)
{
	if (menu_hexdump_edit_position_x>0) {
		menu_hexdump_edit_position_x--;

		//Si en medio del espacio entre hexa y ascii
		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) menu_hexdump_edit_position_x--;
	}

	else {
		//Aparecer por la derecha
		menu_debug_hexdump_cursor_arriba();
		menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*3;
	}

}

//escribiendo_memoria cursor indica que si estamos a la derecha de zona de edicion escribiendo,
//tiene que saltar a la zona izquierda de la zona ascii o hexa, al llegar a la derecha de dicha zona
void menu_debug_hexdump_edit_cursor_derecha(int escribiendo_memoria)
{

	//Hexdump. bytes_por_linea*2 espacio bytes_por_linea

	int ancho_linea=menu_hexdump_bytes_por_linea*3+1;

	if (menu_hexdump_edit_position_x<ancho_linea-1) {
		menu_hexdump_edit_position_x++;

		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) { //Fin zona derecha hexa
			if (escribiendo_memoria) {
				//Ponernos al inicio zona hexa de nuevo saltando siguiente linea
				menu_hexdump_edit_position_x=0;
				menu_debug_hexdump_cursor_abajo();
			}
			else {
				//Saltar a zona ascii
				menu_hexdump_edit_position_x++;
			}
		}
	}

	else {
		//Fin zona derecha ascii. 
		menu_debug_hexdump_cursor_abajo();

		if (escribiendo_memoria) {
			//Ponernos en el principio zona ascii
			menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*2+1;
		}
		else {
			menu_hexdump_edit_position_x=0;
		}
	}

}

void menu_debug_hexdump_cursor_arriba(void)
{
	int alterar_ptr=0;
						//arriba
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y>0) menu_hexdump_edit_position_y--;
							else alterar_ptr=1;
						}

						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion -=menu_hexdump_bytes_por_linea;
							menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,menu_hexdump_bytes_por_linea);
						}
}

void menu_debug_hexdump_cursor_abajo(void)
{
	int alterar_ptr=0;
						//abajo
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y<menu_hexdump_lineas_total-1) menu_hexdump_edit_position_y++;
							else alterar_ptr=1;
						}						
						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion +=menu_hexdump_bytes_por_linea;
						}
}

void menu_debug_hexdump_copy(void)
{


    char string_address[10];

    sprintf (string_address,"%XH",menu_debug_hexdump_direccion);
    menu_ventana_scanf("Source?",string_address,10);
	menu_z80_moto_int source=parse_string_to_number(string_address);

    sprintf (string_address,"%XH",source);
    menu_ventana_scanf("Destination?",string_address,10);
	menu_z80_moto_int destination=parse_string_to_number(string_address);
	
	int destzone=menu_change_memory_zone_list_title("Destination Zone");
	if (destzone==-2) return; //Pulsado ESC
	
	int origzone=menu_debug_memory_zone;
	

    strcpy (string_address,"1");
    menu_ventana_scanf("Length?",string_address,10);
	menu_z80_moto_int longitud=parse_string_to_number(string_address);	
	
	

	if (menu_confirm_yesno("Copy bytes")) {
		for (;longitud>0;source++,destination++,longitud--) {
			menu_set_memzone(origzone);
			//Antes de escribir o leer, normalizar zona memoria
			menu_debug_set_memory_zone_attr();
			source=adjust_address_memory_size(source);
			z80_byte valor=menu_debug_get_mapped_byte(source);
			
			
			menu_set_memzone(destzone);
			//Antes de escribir o leer, normalizar zona memoria
			menu_debug_set_memory_zone_attr();
			destination=adjust_address_memory_size(destination);
			menu_debug_write_mapped_byte(destination,valor);
		}
		
		//dejar la zona origen tal cual
		menu_set_memzone(origzone);
	}


}

void menu_debug_hexdump_aviso_edit_filezone(zxvision_window *w)
{
							menu_warn_message("Memory zone is File zone. Changes won't be saved to the file");
							//Volver a dibujar ventana, pues se ha borrado al aparecer el aviso
							//menu_debug_hexdump_ventana();	
	zxvision_draw_window(w);
}

void menu_debug_hexdump_info_subzones(void)
{
	
		int x=1;
		int y=1;
		int ancho=30;
		int alto=22;



        subzone_info *puntero;
        puntero=machine_get_memory_subzone_array(menu_debug_memory_zone,current_machine_type);
        if (puntero==NULL) return;

		zxvision_window ventana;

                zxvision_new_window(&ventana,x,y,ancho,alto,
                                                        64,alto-2,"Memory subzones");

                zxvision_draw_window(&ventana);		

        int i;

		char buffer_linea[64];
        for (i=0;puntero[i].nombre[0]!=0;i++) {

			//printf ("inicio: %d fin: %d texto: %s\n",puntero[i].inicio,puntero[i].fin,puntero[i].nombre);
			sprintf (buffer_linea,"%06X-%06X %s",puntero[i].inicio,puntero[i].fin,puntero[i].nombre);
			zxvision_print_string_defaults_fillspc(&ventana,1,i,buffer_linea);
			
		}

		zxvision_draw_window_contents(&ventana);

		zxvision_wait_until_esc(&ventana);

        cls_menu_overlay();

                zxvision_destroy_window(&ventana);					


}

void menu_debug_hexdump_crea_ventana(zxvision_window *ventana,int x,int y,int ancho,int alto)
{
	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window_nocheck_staticsize(ventana,x,y,ancho,alto,ancho,alto-2,"Hexadecimal Editor");

	//printf ("ancho: %d alto: %d\n",ancho,alto);

	ventana->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll

	zxvision_draw_window(ventana);

}

void menu_debug_hexdump(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;
	int xventana,yventana,ancho_ventana,alto_ventana;
	
	if (!util_find_window_geometry("hexeditor",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {
		xventana=DEBUG_HEXDUMP_WINDOW_X;
		yventana=DEBUG_HEXDUMP_WINDOW_Y;
		ancho_ventana=DEBUG_HEXDUMP_WINDOW_ANCHO;
		alto_ventana=DEBUG_HEXDUMP_WINDOW_ALTO;
	}


	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	//zxvision_new_window_nocheck_staticsize(&ventana,x,y,ancho,alto,ancho,alto-2,"Hexadecimal Editor");
	menu_debug_hexdump_crea_ventana(&ventana,xventana,yventana,ancho_ventana,alto_ventana);

	//Nos guardamos alto y ancho anterior. Si el usuario redimensiona la ventana, la recreamos
	int alto_anterior=alto_ventana;
	int ancho_anterior=ancho_ventana;

	



	//ventana.can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll

	//zxvision_draw_window(&ventana);


    z80_byte tecla;

	int salir=0;

	z80_byte valor_xor=0;



	if (MACHINE_IS_ZX80) menu_debug_hexdump_with_ascii_modo_ascii=1;
	else if (MACHINE_IS_ZX81) menu_debug_hexdump_with_ascii_modo_ascii=2;

	else menu_debug_hexdump_with_ascii_modo_ascii=0;


	int asked_about_writing_rom=0;

                //Guardar estado atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;	



        do {

		//printf ("dibujamos ventana\n");
			//Alto 23. lineas 13
		menu_hexdump_lineas_total=ventana.visible_height-10;

			if (menu_hexdump_lineas_total<3) menu_hexdump_lineas_total=3;

			int cursor_en_zona_ascii=0;
			int editando_en_zona_ascii=0;


					//Si maquina no es QL, direccion siempre entre 0 y 65535
					//menu_debug_hexdump_direccion=adjust_address_space_cpu(menu_debug_hexdump_direccion);
					menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);


		int linea=0;

		int lineas_hex=0;



		int bytes_por_ventana=menu_hexdump_bytes_por_linea*menu_hexdump_lineas_total;

		char dumpmemoria[33];



		//Antes de escribir, normalizar zona memoria
		menu_debug_set_memory_zone_attr();

				char textoshow[33];

		sprintf (textoshow,"Showing %d bytes per page:",bytes_por_ventana);
        //menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");


		//Hacer que texto ventana empiece pegado a la izquierda
		menu_escribe_linea_startx=0;

		//No mostrar caracteres especiales
		menu_disable_special_chars.v=1;

		//Donde esta el otro caracter que acompanya al nibble, en caso de cursor en zona hexa
		int menu_hexdump_edit_position_x_nibble=menu_hexdump_edit_position_x^1;

		
		if (menu_hexdump_edit_position_x>menu_hexdump_bytes_por_linea*2) cursor_en_zona_ascii=1;


		if (menu_hexdump_edit_mode && cursor_en_zona_ascii) editando_en_zona_ascii=1;		

		char nibble_char='X';	
		char nibble_char_cursor='X';	

		for (;lineas_hex<menu_hexdump_lineas_total;lineas_hex++,linea++) {

			menu_z80_moto_int dir_leida=menu_debug_hexdump_direccion+lineas_hex*menu_hexdump_bytes_por_linea;
			menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);

			menu_debug_hexdump_with_ascii(dumpmemoria,dir_leida,menu_hexdump_bytes_por_linea,valor_xor);

			zxvision_print_string_defaults_fillspc(&ventana,0,linea,dumpmemoria);

			//Meter el nibble_char si corresponde
			if (lineas_hex==menu_hexdump_edit_position_y) {
				nibble_char_cursor=dumpmemoria[7+menu_hexdump_edit_position_x];
				if (!editando_en_zona_ascii) nibble_char=dumpmemoria[7+menu_hexdump_edit_position_x_nibble];
			}
		}


		menu_escribe_linea_startx=1;

		//Volver a mostrar caracteres especiales
		menu_disable_special_chars.v=0;		



		//Mostrar cursor si en modo edicion
		
		//Al mostrar en cursor: si esta en parte ascii, hacer parpadear el caracter en esa zona, metiendo color de opcion seleccionada
		//Si esta en parte hexa, parpadeamos la parte del nibble que editamos, el otro nibble no parpadea. Ambos tienen color de opcion seleccionada
		//Si multitarea esta a off, no existe el parpadeo, y por tanto, para que se viera en que nibble edita, se mostrara el caracter _, logicamente
		//tapando el caracter de debajo
		//Para ver los caracteres de debajo, los asignamos antes, en el bucle que hace el volcado hexa, y lo guardo en las variables
		//nibble_char_cursor (que dice el caracter de debajo del cursor) y nibble_char (que dice el otro caracter que acompanya al nibble)
	
		
		if (menu_hexdump_edit_mode) {
			int xfinal=7+menu_hexdump_edit_position_x;
			int yfinal=2+menu_hexdump_edit_position_y;			

			menu_debug_hexdump_print_editcursor(&ventana,xfinal,yfinal,nibble_char_cursor);

			//Indicar nibble entero. En caso de edit hexa
			if (!editando_en_zona_ascii) {
				xfinal=7+menu_hexdump_edit_position_x_nibble;
				menu_debug_hexdump_print_editcursor_nibble(&ventana,xfinal,yfinal,nibble_char);
			}
		}


				//Forzar a mostrar atajos
                menu_writing_inverse_color.v=1;


//printf ("zone size: %x dir: %x\n",menu_debug_memory_zone_size,menu_debug_hexdump_direccion);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		char buffer_linea[64]; //Por si acaso, entre negritas y demas

		char buffer_char_type[20];

		char string_atajos[3]="~~"; 
		//Si esta en edit mode y en zona de ascii, no hay atajos



		if (editando_en_zona_ascii) string_atajos[0]=0;

				if (menu_debug_hexdump_with_ascii_modo_ascii==0) {
					sprintf (buffer_char_type,"ASCII");
				}

				else if (menu_debug_hexdump_with_ascii_modo_ascii==1) {
                	sprintf (buffer_char_type,"ZX80");
                }

				else sprintf (buffer_char_type,"ZX81");



				sprintf (buffer_linea,"%smemptr C%sopy",string_atajos,string_atajos);


				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

				sprintf (buffer_linea,"[%c] %sinvert [%c] Edi%st C%shar:%s",
					(valor_xor==0 ? ' ' : 'X'), 
					string_atajos,
					
					(menu_hexdump_edit_mode==0 ? ' ' : 'X' ),
					string_atajos,
					
					string_atajos,
					buffer_char_type
					);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);


				char memory_zone_text[64]; //espacio temporal mas grande por si acaso
				if (menu_debug_show_memory_zones==0) {
					sprintf (memory_zone_text,"Mem %szone (mapped memory)",string_atajos);
				}
				else {
					//printf ("Info zona %d\n",menu_debug_memory_zone);
					char buffer_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
					//int readwrite;
					machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);
					sprintf (memory_zone_text,"Mem %szone (%d %s)",string_atajos,menu_debug_memory_zone,buffer_name);
					//printf ("size: %X\n",menu_debug_memory_zone_size);
					//printf ("Despues zona %d\n",menu_debug_memory_zone);
				}

				//truncar texto a 32 por si acaso
				memory_zone_text[32]=0;
				//menu_escribe_linea_opcion(linea++,-1,1,memory_zone_text);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,memory_zone_text);

				sprintf (textoshow," Size: %d (%d KB)",menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);
				//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

		
				char subzone_info[33];
				machine_get_memory_subzone_name(menu_debug_memory_zone,current_machine_type, menu_debug_hexdump_direccion, subzone_info);
				if (subzone_info[0]!=0) {
					sprintf(buffer_linea," S~~ubzone info: %s",subzone_info);
					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);
				}
				else {
					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
				}


//Restaurar comportamiento atajos
menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

			zxvision_draw_window_contents(&ventana);
			//NOTA: este menu no acostumbra a refrescar rapido la ventana cuando la redimensionamos con el raton
			//es una razon facil: el volcado de hexa usa relativamente mucha cpu,
			//cada vez que redimensionamos ventana, se llama al bucle continuamente, usando mucha cpu y si esta el autoframeskip,
			//hace saltar frames
			

			tecla=zxvision_common_getkey_refresh();		


				//Aviso: hay que conseguir que las letras de accion no esten entre la a-f, porque asi,
				//podemos usar dichas letras para editar hexa

				switch (tecla) {

					case 11:
						menu_debug_hexdump_cursor_arriba();

					break;

					case 10:
						menu_debug_hexdump_cursor_abajo();

					break;

					case 8:
					case 12:
						//izquierda o delete
						if (menu_hexdump_edit_mode) {
							//if (menu_hexdump_edit_position_x>0) menu_hexdump_edit_position_x--;
							menu_debug_hexdump_edit_cursor_izquierda();
						}
					break;

					case 9:
						//derecha
						if (menu_hexdump_edit_mode) {
							menu_debug_hexdump_edit_cursor_derecha(0);
							//if (menu_hexdump_edit_position_x<(bytes_por_linea*2)-1) menu_hexdump_edit_position_x++;
						}
					break;					

					case 24:
						//PgUp
						menu_debug_hexdump_direccion -=bytes_por_ventana;
						menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,bytes_por_ventana);
					break;

					case 25:
						//PgDn
						menu_debug_hexdump_direccion +=bytes_por_ventana;
					break;

					case 'm':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_direccion=menu_debug_hexdump_change_pointer(menu_debug_hexdump_direccion);
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;

					case 'o':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_copy();
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;					

					case 'h':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_with_ascii_modo_ascii++;
							if (menu_debug_hexdump_with_ascii_modo_ascii==3) menu_debug_hexdump_with_ascii_modo_ascii=0;
						}
					break;

					case 'i':
						if (!editando_en_zona_ascii) valor_xor ^= 255;
					break;

					case 't':
						if (!editando_en_zona_ascii) {
							menu_hexdump_edit_mode ^= 1;
							menu_espera_no_tecla();
							tecla=0; //para no enviar dicha tecla al editor
						}

						//Si zona de filemem
						if (menu_hexdump_edit_mode && menu_debug_memory_zone==MEMORY_ZONE_NUM_FILE_ZONE) {
							menu_debug_hexdump_aviso_edit_filezone(&ventana);				
						}
					break;	

					case 'u':
						//Ver info subzonas
						menu_debug_hexdump_info_subzones();
					break;				

					//case 'l':
					//	menu_debug_hex_shows_inves_low_ram.v ^=1;
					//break;

					case 'z':

						if (!editando_en_zona_ascii) {
							menu_debug_change_memory_zone();
							asked_about_writing_rom=0;
						}

						break;

					//Salir con ESC si no es modo edit
					case 2:
						if (menu_hexdump_edit_mode) {
							menu_hexdump_edit_mode=0;
						}
						else salir=1;
					break;

					//Enter tambien sale de modo edit
					case 13:
						if (menu_hexdump_edit_mode) menu_hexdump_edit_mode=0;
					break;



				}

				//Y ahora para el caso de edit_mode y pulsar tecla hexa o ascii segun la zona
				int editar_byte=0;
				if (menu_hexdump_edit_mode) {
					//Para la zona ascii
					if (cursor_en_zona_ascii && tecla>=32 && tecla<=126) editar_byte=1; 

					//Para la zona hexa
					if (
						!cursor_en_zona_ascii && 
						( (tecla>='0' && tecla<='9') || (tecla>='a' && tecla<='f') )
						) {
						editar_byte=1; 
					}
				}

				//Ver si vamos a editar en zona de rom
				if (editar_byte) {
					int attribute_zone;
					//Si zona por defecto mapped memory, asumimos lectura/escritura
					if (menu_debug_memory_zone==-1) attribute_zone=3;
					else machine_get_memory_zone_attrib(menu_debug_memory_zone, &attribute_zone);

					//printf ("Attrib zone %d asked %d\n",attribute_zone,asked_about_writing_rom);

					//Attrib: bit 0: read, bit 1: write
					//Si no tiene atributo de escritura y no se ha pedido antes si se quiere escribir en rom
					if ( (attribute_zone&2)==0 && !asked_about_writing_rom) {
						if (menu_confirm_yesno_texto("Memory zone is ROM","Sure you want to edit?")==0) {
							editar_byte=0;
						}
						else {
							asked_about_writing_rom=1;
						}

						//Volver a dibujar ventana, pues se ha borrado al pregutar confirmacion
						//menu_debug_hexdump_ventana();
						zxvision_draw_window(&ventana);
					}

 
				}


				//asked_about_writing_rom

				if (editar_byte) {
						//Obtener direccion puntero
						menu_z80_moto_int direccion_cursor=menu_debug_hexdump_direccion;

						//int si_zona_hexa=0; //en zona hexa o ascii
						//if (menu_hexdump_edit_position_x<bytes_por_linea*2) si_zona_hexa=1;


						if (!cursor_en_zona_ascii) {
							//Sumar x (cada dos, una posicion)
							direccion_cursor +=menu_hexdump_edit_position_x/2;
						}
						else {
							int indice_hasta_ascii=menu_hexdump_bytes_por_linea*2+1; //el hexa y el espacio
							direccion_cursor +=menu_hexdump_edit_position_x-indice_hasta_ascii;
						}

						//Sumar y. 
						direccion_cursor +=menu_hexdump_edit_position_y*menu_hexdump_bytes_por_linea;

						//Ajustar direccion a zona memoria
						direccion_cursor=adjust_address_memory_size(direccion_cursor);

						//TODO: ver si se sale de tamanyo zona memoria

						//printf ("Direccion edicion: %X\n",direccion_cursor);

						//Obtenemos byte en esa posicion
						z80_byte valor_leido=menu_debug_get_mapped_byte(direccion_cursor);


						//Estamos en zona hexa o ascii

						if (!cursor_en_zona_ascii) {
							//printf ("Zona hexa\n");
							//Zona hexa

							//Obtener valor nibble
							z80_byte valor_nibble;

							if (tecla>='0' && tecla<='9') valor_nibble=tecla-'0';
							else valor_nibble=tecla-'a'+10;

							//Ver si par o impar
							if ( (menu_hexdump_edit_position_x %2) ==0) {
								//par. alterar nibble alto
								valor_leido=(valor_leido&0x0F) | (valor_nibble<<4);
							}

							else {
								//impar. alterar nibble bajo
								valor_leido=(valor_leido&0xF0) | valor_nibble;
							}
						}

						else {
							//printf ("Zona ascii\n");
							valor_leido=tecla;

							//En el caso de zx80/81

							if (menu_debug_hexdump_with_ascii_modo_ascii==1) valor_leido=ascii_to_zx80(valor_leido);
							if (menu_debug_hexdump_with_ascii_modo_ascii==2) valor_leido=ascii_to_zx81(valor_leido);
						}



						//Escribimos valor
						menu_debug_write_mapped_byte(direccion_cursor,valor_leido);

						//Y mover cursor a la derecha
						menu_debug_hexdump_edit_cursor_derecha(1);

						//Si se llega a detecha de hexa o ascii, saltar linea

					
				}

		//Si ha cambiado el alto
		alto_ventana=ventana.visible_height;
		ancho_ventana=ventana.visible_width;
		xventana=ventana.x;
		yventana=ventana.y;
		if (alto_ventana!=alto_anterior || ancho_ventana!=ancho_anterior) {
			//printf ("recrear ventana\n");
			//Recrear ventana
			//Cancelamos edicion si estaba ahi
			editando_en_zona_ascii=0;
			menu_hexdump_edit_mode=0;
			menu_hexdump_edit_position_x=0;
			menu_hexdump_edit_position_y=0;

			zxvision_destroy_window(&ventana);
			menu_debug_hexdump_crea_ventana(&ventana,xventana,yventana,ancho_ventana,alto_ventana);
			alto_anterior=alto_ventana;
			ancho_anterior=ancho_ventana;
		}
			


        } while (salir==0);

	cls_menu_overlay();

	util_add_window_geometry("hexeditor",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);

	zxvision_destroy_window(&ventana);
	

}



//Entrada seleccionada
int adventure_keyboard_selected_item=0;

//Posicion dentro del string
int adventure_keyboard_index_selected_item=0;

//z80_bit menu_osd_adventure_sending_keys={0};




int adventure_keyboard_pending_send_final_spc=1;

void menu_osd_adventure_kb_press_key_variable(char letra)
{
	if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; 
}

void menu_osd_adventure_kb_press_key(void)
{

	//Aunque el usuario haya puesto alguna mayuscula, metemos minusculas
	char letra;

	//Ignorar ~~

	do {
		letra=letra_minuscula(osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]);
		if (letra=='~') adventure_keyboard_index_selected_item++;
	} while (letra=='~' && letra!=0);

	menu_osd_adventure_kb_press_key_variable(letra);

	/*if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; */

}


void menu_osd_adventure_keyboard_action(MENU_ITEM_PARAMETERS)
{
	//printf ("opcion seleccionada: %d\n",valor_opcion);
	adventure_keyboard_selected_item=valor_opcion;
	adventure_keyboard_index_selected_item=0;
	adventure_keyboard_pending_send_final_spc=1;


	//Estamos enviando teclas
	//menu_osd_adventure_sending_keys.v=1;

	menu_osd_adventure_kb_press_key();
}

//Retorno desde el core
void menu_osd_adventure_keyboard_next(void)
{

	//if (menu_osd_adventure_sending_keys.v==0 return;

	//Si final de string
	adventure_keyboard_index_selected_item++;
	if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
		//printf ("Fin texto\n");
		
		//Ver si habia que enviar un espacio al final
		if (adventure_keyboard_send_final_spc && adventure_keyboard_pending_send_final_spc) {
			menu_osd_adventure_kb_press_key_variable(' ');
			adventure_keyboard_pending_send_final_spc=0;
		}
		else {
			//En este caso reabrir el menu
			menu_osd_adventure_keyboard(0);
			return;
		}
	}

	//Siguiente tecla
	else menu_osd_adventure_kb_press_key();
}


#define ADVENTURE_KB_X (menu_origin_x() )
#define ADVENTURE_KB_Y 0

//Le ponemos maximo ancho 32 que es el mismo que gestiona la funcion de dibujar menu
#define ADVENTURE_KB_ANCHO 32

//Le ponemos maximo alto 24 que es el mismo que gestiona la funcion de dibujar menu
#define ADVENTURE_KB_ALTO 24

//maximo de alto total admitido para la ventana
#define ADVENTURE_KB_MAX_TOTAL_HEIGHT 500

//conservar valor de scroll ultimo para que cuando listado sea grande,
//poder conservar ultima posicion
int menu_osd_advkb_last_offset_y=0;

void menu_osd_adventure_keyboard(MENU_ITEM_PARAMETERS)
{

	//Si estamos enviando teclas
	//if (menu_osd_adventure_sending_keys.v) {
	//	menu_osd_adventure_keyboard_next();
	//	return;
	//}

	//Si lista vacia, error
	if (osd_adv_kbd_defined==0) {
		debug_printf (VERBOSE_ERR,"Empty list");
		return;
	}

	//Si estamos enviando teclas, desactivar
	timer_on_screen_adv_key=0;



 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,ADVENTURE_KB_X,ADVENTURE_KB_Y,ADVENTURE_KB_ANCHO,ADVENTURE_KB_ALTO,
							ADVENTURE_KB_ANCHO-1,ADVENTURE_KB_MAX_TOTAL_HEIGHT,"OSD Adventure Keyboard");
	zxvision_draw_window(&ventana);		

//printf ("ancho: %d\n",ADVENTURE_KB_ANCHO);

       
        menu_item *array_menu_osd_adventure_keyboard;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		int initial_test; //si es 1, haremos el calculo inicial del alto

		int alto_ventana=ADVENTURE_KB_ALTO;
		int y_ventana=ADVENTURE_KB_Y;

		for (initial_test=1;initial_test>=0;initial_test--) {


          //Hay que redibujar la ventana desde este bucle
          if (!initial_test) {
			  //menu_dibuja_ventana(ADVENTURE_KB_X,y_ventana,ADVENTURE_KB_ANCHO,alto_ventana,"OSD Adventure Keyboard");
			  zxvision_set_y_position(&ventana,y_ventana);
			  zxvision_set_visible_height(&ventana,alto_ventana);
			  
			  //Alteramos alto total para que coincida con alto ventana (siempre que sea menor que el alto actual)
			  //si fuese mayor el alto, estariamos necesitando mas memoria y seria un problema
			  //esto es un poco feo realmente, pero bueno, al reducir el tamaño no hay problema de que nos salgamos de la memoria
			  int current_height=ventana.total_height;
			  int desired_height=alto_ventana-2;

			  if (desired_height<current_height) {
				  ventana.total_height=desired_height;
			  }
		  }


        //Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
        menu_add_item_menu_inicial(&array_menu_osd_adventure_keyboard,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	//if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
	//osd_adv_kbd_defined
		int i;
		int last_x=1;
		int last_y=0;
		int salir=0;

		//Asignar hotkeys, segun si se han asignado antes o no
		//int hotkeys_assigned[26]; //de la A a la Z
		//for (i=0;i<26;i++) hotkeys_assigned[i]=0;
		
		
		for (i=0;i<osd_adv_kbd_defined && !salir;i++) {
			int longitud_texto=strlen(osd_adv_kbd_list[i])+1; //Espacio para la entrada y 1 espacio
			if (last_x+longitud_texto>ADVENTURE_KB_ANCHO) {
				last_x=1;
				last_y++; 
			}

			//controlar maximo de alto
			if (last_y>=ADVENTURE_KB_MAX_TOTAL_HEIGHT) {
				debug_printf (VERBOSE_DEBUG,"Reached maximum window height");
				last_y--;
				salir=1;
			}

			else {
				//Si es cadena vacia, ignorarla. No deberia pasar pues se debe denegar desde donde se lee la configuracion, pero por si acaso
				if (osd_adv_kbd_list[i][0]==0) {
					debug_printf (VERBOSE_DEBUG,"Null string at %d",i);
				}

				else {

					int tiene_hotkey=0;

					char texto_opcion[64];
					strcpy(texto_opcion,osd_adv_kbd_list[i]);

					char hotkey;
					
					//Caracter de hotkey. Crearlo automaticamente
					//hotkey=letra_minuscula(osd_adv_kbd_list[i][0]);

					//Caracter de hotkey. Dejar que el usuario lo escriba en la cadena de texto. Ver si dicha cadena lo tiene

					int j;
					for (j=0;texto_opcion[j];j++) {
						if (texto_opcion[j]=='~' && texto_opcion[j+1]=='~') {
							//Si hay letra detras
							hotkey=letra_minuscula(texto_opcion[j+2]);
							if (hotkey) tiene_hotkey=1;
						}
					}
					

				    menu_add_item_menu_format(array_menu_osd_adventure_keyboard,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard_action,NULL,texto_opcion);
        		    menu_add_item_menu_tabulado(array_menu_osd_adventure_keyboard,last_x,last_y);
					menu_add_item_menu_valor_opcion(array_menu_osd_adventure_keyboard,i);
					//printf ("Agregando palabra %s en %d,%d\n",texto_opcion,last_x,last_y);

					if (tiene_hotkey) {
						menu_add_item_menu_shortcut(array_menu_osd_adventure_keyboard,hotkey);
						longitud_texto -=2;
					}

				}

				last_x+=longitud_texto;
			}

		}

		//Recalcular alto, y_inivial
		//del alto, se pierden 2 siempre
		//si tuvieramos el maximo de y, valdria 21. Y el maximo de alto es 24
		//printf ("ultima y: %d\n",last_y);
		alto_ventana=last_y+3;
		y_ventana=menu_center_y()-alto_ventana/2;
		if (y_ventana<0) y_ventana=0;	


		}


		//Recuperamos antiguo offset de ventana
		zxvision_set_offset_y(&ventana,menu_osd_advkb_last_offset_y);


		//Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu(&osd_adventure_keyboard_opcion_seleccionada,&item_seleccionado,array_menu_osd_adventure_keyboard,"OSD Adventure KB" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
				//printf ("Item seleccionado: %d\n",item_seleccionado.valor_opcion);
                                //printf ("actuamos por funcion\n");

	                        salir_todos_menus=1;

                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


		//Guardamos offset de ventana actual
		menu_osd_advkb_last_offset_y=ventana.offset_y;

        cls_menu_overlay();
		//menu_espera_no_tecla();

		//menu_abierto=1;
		//Si con control de joystick se ha salido con tecla ESCMenu, esa tecla de joystick lo que hace es ESC
		//pero luego fuerza a abrir el menu de nuevo. Por tanto, decimos que no hay que abrir menu
		menu_event_open_menu.v=0;

		//printf ("en final de funcion\n");
		zxvision_destroy_window(&ventana);

}









//Usado dentro del overlay de tsconf_dma
//int menu_tsconf_dma_valor_contador_segundo_anterior;

zxvision_window *menu_debug_dma_tsconf_zxuno_overlay_window;


void menu_debug_dma_tsconf_zxuno_overlay(void)
{

    normal_overlay_texto_menu();

    int linea=0;
   

    
    	//mostrarlos siempre a cada refresco
    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	char texto_dma[33];

	if (datagear_dma_emulation.v) {
		//NOTA: Si se activa datagear, no se vera si hay dma de tsconf o zxuno
		z80_int dma_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
		z80_int dma_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);

		z80_int dma_len=value_8_to_16(datagear_block_length_high,datagear_block_length_low);	

		sprintf (texto_dma,"Port A:      %04XH",dma_port_a);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Port B:      %04XH",dma_port_b);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Length:      %5d",dma_len);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		if (datagear_wr0 & 4) sprintf (texto_dma,"Port A->B");
		else sprintf (texto_dma,"Port B->A");

		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);



		char access_type[20];

        if (datagear_wr1 & 8) sprintf (access_type,"I/O"); 
		else sprintf (access_type,"Memory");

		if ( (datagear_wr1 & 32) == 0 ) {
            if (datagear_wr1 & 16) sprintf (texto_dma,"Port A++. %s",access_type);
            else sprintf (texto_dma,"Port A--. %s",access_type);
        }
		else sprintf (texto_dma,"Port A fixed. %s",access_type);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);


        if (datagear_wr2 & 8) sprintf (access_type,"I/O"); 
		else sprintf (access_type,"Memory");

		if ( (datagear_wr2 & 32) == 0 ) {
            if (datagear_wr2 & 16) sprintf (texto_dma,"Port B++. %s",access_type);
            else sprintf (texto_dma,"Port B--. %s",access_type);
        }
		else sprintf (texto_dma,"Port B fixed. %s",access_type);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);	
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		//WR4. Bits D6 D5:
		//#       0   0 = Byte mode -> Do not use (Behaves like Continuous mode, Byte mode on Z80 DMA)
		//#       0   1 = Continuous mode
		//#       1   0 = Burst mode
		//#       1   1 = Do not use

		z80_byte modo_transferencia=(datagear_wr4>>5)&3;
		if (modo_transferencia==0) 		sprintf (texto_dma,"Mode: Byte mode");
		else if (modo_transferencia==1) sprintf (texto_dma,"Mode: Continuous");
		else if (modo_transferencia==2) sprintf (texto_dma,"Mode: Burst");
		else 							sprintf (texto_dma,"Mode: Do not use");

		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);	
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);



	}

	else {

	if (MACHINE_IS_TSCONF) {
		//Construimos 16 valores posibles segun rw (bit bajo) y ddev (bits altos)
		int dma_type=debug_tsconf_dma_ddev*2+debug_tsconf_dma_rw;
						//18 maximo el tipo
						//  012345678901234567890123
						//24. mas dos de margen banda y banda: 26
		sprintf (texto_dma,"Type: %s",tsconf_dma_types[dma_type]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Source:      %06XH",debug_tsconf_dma_source);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Destination: %06XH",debug_tsconf_dma_destination);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Burst length: %3d",debug_tsconf_dma_burst_length);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Burst number: %3d",debug_tsconf_dma_burst_number);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

						//Maximo 25
		sprintf (texto_dma,"Align: %s %s",(debug_tsconf_dma_s_align ? "Source" : "      "),(debug_tsconf_dma_d_align ? "Destination" : "") );
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Align size: %d",(debug_tsconf_dma_addr_align_size+1)*256);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

	}

	if (MACHINE_IS_ZXUNO) {
		z80_byte dma_ctrl=zxuno_ports[0xa0];
		z80_byte dma_type=(dma_ctrl & (4+8))>>2;
		z80_byte dma_mode=dma_ctrl & 3;

		z80_int dma_src=value_8_to_16(zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
		z80_int dma_dst=value_8_to_16(zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
		z80_int dma_pre=value_8_to_16(zxuno_dmareg[2][1],zxuno_dmareg[2][0]);
		z80_int dma_len=value_8_to_16(zxuno_dmareg[3][1],zxuno_dmareg[3][0]);	
		z80_int dma_prob=value_8_to_16(zxuno_dmareg[4][1],zxuno_dmareg[4][0]);		
		z80_byte dma_stat=zxuno_ports[0xa6];

		sprintf (texto_dma,"Type: %s",zxuno_dma_types[dma_type]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Mode: %s",zxuno_dma_modes[dma_mode]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);		
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Source:      %04XH",dma_src);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Destination: %04XH",dma_dst);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Length:      %5d",dma_len);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Preescaler:  %5d",dma_pre);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		char prob_type[10];
		if (dma_ctrl&16) strcpy(prob_type,"dst");
		else strcpy(prob_type,"src");

		sprintf (texto_dma,"Prob: (%s)  %04XH",prob_type,dma_prob);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);		
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Stat:          %02XH",dma_stat);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);			
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

	}

	}

	zxvision_draw_window_contents(menu_debug_dma_tsconf_zxuno_overlay_window);
}






void menu_debug_dma_tsconf_zxuno_disable(MENU_ITEM_PARAMETERS)
{
	if (datagear_dma_emulation.v) datagear_dma_is_disabled.v ^=1;

	else {
		if (MACHINE_IS_TSCONF) tsconf_dma_disabled.v ^=1;
		if (MACHINE_IS_ZXUNO) zxuno_dma_disabled.v ^=1;
	}
}


void menu_debug_dma_tsconf_zxuno(MENU_ITEM_PARAMETERS)
{
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		





	char texto_ventana[33];
	//por defecto por si acaso
	strcpy(texto_ventana,"DMA");
	int alto_ventana=11;


	if (MACHINE_IS_ZXUNO) {
		strcpy(texto_ventana,"ZXUNO DMA");
		alto_ventana++;
	}

	if (MACHINE_IS_TSCONF) strcpy(texto_ventana,"TSConf DMA");

	if (datagear_dma_emulation.v) strcpy(texto_ventana,"Datagear DMA");	


	//menu_dibuja_ventana(2,6,27,alto,texto_ventana);
	zxvision_window ventana;

	//int posicionx=menu_origin_x()+2;
	int ancho_ventana=27;
	int posicionx=menu_center_x()-ancho_ventana/2;

	int posiciony=menu_center_y()-alto_ventana/2;

	zxvision_new_window(&ventana,posicionx,posiciony,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,texto_ventana);
	zxvision_draw_window(&ventana);			



    //Cambiamos funcion overlay de texto de menu
	set_menu_overlay_function(menu_debug_dma_tsconf_zxuno_overlay);

	menu_debug_dma_tsconf_zxuno_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui			



	menu_item *array_menu_debug_dma_tsconf_zxuno;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

        			
            //Hay que redibujar la ventana desde este bucle
            //menu_debug_dma_tsconf_zxuno_dibuja_ventana();

	

			int lin=8;

			

			int condicion_dma_disabled=tsconf_dma_disabled.v;


			if (MACHINE_IS_ZXUNO) {
				lin++;	
				condicion_dma_disabled=zxuno_dma_disabled.v;
			}

			if (datagear_dma_emulation.v) condicion_dma_disabled=datagear_dma_is_disabled.v;
		
				menu_add_item_menu_inicial_format(&array_menu_debug_dma_tsconf_zxuno,MENU_OPCION_NORMAL,menu_debug_dma_tsconf_zxuno_disable,NULL,"~~DMA: %s",
					(condicion_dma_disabled ? "Disabled" : "Enabled ") );  //Enabled acaba con espacio para borrar rastro de texto "Disabled"
				menu_add_item_menu_shortcut(array_menu_debug_dma_tsconf_zxuno,'d');
				menu_add_item_menu_ayuda(array_menu_debug_dma_tsconf_zxuno,"Disable DMA");
				menu_add_item_menu_tabulado(array_menu_debug_dma_tsconf_zxuno,1,lin);




		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&debug_tsconf_dma_opcion_seleccionada,&item_seleccionado,array_menu_debug_dma_tsconf_zxuno,"TSConf DMA" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);				

}




int menu_tsconf_layer_valor_contador_segundo_anterior;

char *menu_tsconf_layer_aux_usedunused_used="In use";
char *menu_tsconf_layer_aux_usedunused_unused="Unused";

char *menu_tsconf_layer_aux_usedunused(int value)
{
	if (value) return menu_tsconf_layer_aux_usedunused_used;
	else return menu_tsconf_layer_aux_usedunused_unused;
}


zxvision_window *menu_tsconf_layer_overlay_window;

void menu_tsconf_layer_overlay_mostrar_texto(void)
{
 int linea;

    linea=0;

    
        //mostrarlos siempre a cada refresco

                char texto_layer[33];

				if (MACHINE_IS_TSCONF) {

				//menu_escribe_linea_opcion(linea,-1,1,"Border: ");
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,"Border: ");
				linea +=3;

                sprintf (texto_layer,"ULA:       %s",menu_tsconf_layer_aux_usedunused(tsconf_if_ula_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;

                sprintf (texto_layer,"Sprites 0: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;		

				sprintf (texto_layer,"Tiles 0:   %s",menu_tsconf_layer_aux_usedunused(tsconf_if_tiles_zero_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;	

                sprintf (texto_layer,"Sprites 1: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;	

		    	sprintf (texto_layer,"Tiles 1:   %s",menu_tsconf_layer_aux_usedunused(tsconf_if_tiles_one_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;

                sprintf (texto_layer,"Sprites 2: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;		
				}

				if (MACHINE_IS_TBBLUE) {
	                sprintf (texto_layer,"ULA:       %s",menu_tsconf_layer_aux_usedunused(tbblue_if_ula_is_enabled()) ); 
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;

	                sprintf (texto_layer,"Tiles:     %s",menu_tsconf_layer_aux_usedunused(tbblue_if_tilemap_enabled()) ); 
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;			

					zxvision_print_string_defaults(menu_tsconf_layer_overlay_window,1,linea,"ULA&Tiles:");
					linea +=2;									

                	sprintf (texto_layer,"Sprites:   %s",menu_tsconf_layer_aux_usedunused(tbblue_if_sprites_enabled() ));
                	//menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;		

					sprintf (texto_layer,"Layer 2:   %s",menu_tsconf_layer_aux_usedunused(tbblue_is_active_layer2() ) );
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;						


					//Layer priorities

					z80_byte prio=tbblue_get_layers_priorities();
					sprintf (texto_layer,"Priorities: (%d)",prio);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea++,texto_layer);

				
					int i;
					for (i=0;i<3;i++) {
						char nombre_capa[32];
						strcpy(nombre_capa,tbblue_get_string_layer_prio(i,prio) );
						//if (strcmp(nombre_capa,"ULA&Tiles")) strcpy(nombre_capa,"  ULA  "); //meter espacios para centrarlo
						//las otras capas son "Sprites" y "Layer 2" y ocupan lo mismo

													//     Sprites
													//    ULA&Tiles
						if (i!=2) strcpy (texto_layer,"|---------------|");
						else      strcpy (texto_layer,"v---------------v");

						//Centrar el nombre de capa
						int longitud_medio=strlen(nombre_capa)/2;
						int medio=strlen(texto_layer)/2;
						int pos=medio-longitud_medio;
						if (pos<0) pos=0;

						//Meter texto centrado y quitar 0 del final
						strcpy(&texto_layer[pos],nombre_capa);

						int final=strlen(texto_layer);
						texto_layer[final]='-';

						//menu_escribe_linea_opcion(linea++,-1,1,texto_layer);
						zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea++,texto_layer);

					}
				
				}			


         



}



void menu_tsconf_layer_overlay(void)
{

    normal_overlay_texto_menu();

 	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

 
    //esto hara ejecutar esto 2 veces por segundo
    if ( ((contador_segundo%500) == 0 && menu_tsconf_layer_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

        menu_tsconf_layer_valor_contador_segundo_anterior=contador_segundo;
        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
       

		menu_tsconf_layer_overlay_mostrar_texto();
		zxvision_draw_window_contents(menu_tsconf_layer_overlay_window);

    }
}


void menu_tsconf_layer_settings_ula(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_ula.v ^=1;
}


void menu_tsconf_layer_settings_sprites_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_zero.v ^=1;
}

void menu_tsconf_layer_settings_sprites_one(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_one.v ^=1;
}

void menu_tsconf_layer_settings_sprites_two(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_two.v ^=1;
}

void menu_tsconf_layer_settings_tiles_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_tiles_zero.v ^=1;
}

void menu_tsconf_layer_settings_tiles_one(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_tiles_one.v ^=1;
}

void menu_tsconf_layer_settings_border(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_border.v ^=1;
}

/*
extern z80_bit tbblue_force_disable_layer_ula;
extern z80_bit tbblue_force_disable_layer_sprites;
extern z80_bit tbblue_force_disable_layer_layer_two;
*/

void menu_tbblue_layer_settings_sprites(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_sprites.v ^=1;
}

void menu_tbblue_layer_settings_ula(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_ula.v ^=1;
}

void menu_tbblue_layer_settings_tilemap(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_tilemap.v ^=1;
}

void menu_tbblue_layer_settings_layer_two(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_layer_two.v ^=1;
}

void menu_tbblue_layer_reveal_ula(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_ula.v ^=1;
}

void menu_tbblue_layer_reveal_layer2(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_layer2.v ^=1;
}

void menu_tbblue_layer_reveal_sprites(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_sprites.v ^=1;
}



void menu_tsconf_layer_reveal_ula(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_ula.v ^=1;
}

void menu_tsconf_layer_reveal_sprites_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_zero.v ^=1;
}

void menu_tsconf_layer_reveal_sprites_one(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_one.v ^=1;
}


void menu_tsconf_layer_reveal_sprites_two(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_two.v ^=1;
}


void menu_tsconf_layer_reveal_tiles_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_tiles_zero.v ^=1;
}

void menu_tsconf_layer_reveal_tiles_one(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_tiles_one.v ^=1;
}

void menu_tsconf_layer_settings(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	int ancho=20;
	int alto=22;

	int x=menu_center_x()-ancho/2;
	int y;
	//y=1;	

	if (MACHINE_IS_TBBLUE) {
		alto=20;
		//y=1;
	}


	y=menu_center_y()-alto/2;

	zxvision_window ventana;

	zxvision_new_window(&ventana,x,y,ancho,alto,
							ancho-1,alto-2,"Video Layers");
	zxvision_draw_window(&ventana);		





    //Cambiamos funcion overlay de texto de menu
    set_menu_overlay_function(menu_tsconf_layer_overlay);

	menu_tsconf_layer_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui	

    menu_item *array_menu_tsconf_layer_settings;
    menu_item item_seleccionado;
    int retorno_menu;						

    do {

		//Valido tanto para cuando multitarea es off y para que nada mas entrar aqui, se vea, sin tener que esperar el medio segundo 
		//que he definido en el overlay para que aparezca
		menu_tsconf_layer_overlay_mostrar_texto();

        int lin=1;

		if (MACHINE_IS_TSCONF) {

 			menu_add_item_menu_inicial_format(&array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_border,NULL,"%s",(tsconf_force_disable_layer_border.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			lin+=3;			

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_ula,NULL,"%s",(tsconf_force_disable_layer_ula.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_ula,NULL,"%s",(tsconf_reveal_layer_ula.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);		
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_zero,NULL,"%s",(tsconf_force_disable_layer_sprites_zero.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_zero,NULL,"%s",(tsconf_reveal_layer_sprites_zero.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_tiles_zero,NULL,"%s",(tsconf_force_disable_layer_tiles_zero.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_tiles_zero,NULL,"%s",(tsconf_reveal_layer_tiles_zero.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);						
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_one,NULL,"%s",(tsconf_force_disable_layer_sprites_one.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_one,NULL,"%s",(tsconf_reveal_layer_sprites_one.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);					
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_tiles_one,NULL,"%s",(tsconf_force_disable_layer_tiles_one.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_tiles_one,NULL,"%s",(tsconf_reveal_layer_tiles_one.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_two,NULL,"%s",(tsconf_force_disable_layer_sprites_two.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_two,NULL,"%s",(tsconf_reveal_layer_sprites_two.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);					
			lin+=3;

		}

		if (MACHINE_IS_TBBLUE) {
 			menu_add_item_menu_inicial_format(&array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_ula,NULL,"%s",(tbblue_force_disable_layer_ula.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);		
			lin+=3;			

 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_tilemap,NULL,"%s",(tbblue_force_disable_layer_tilemap.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);			
			lin+=2;

 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_ula,NULL,"%s",(tbblue_reveal_layer_ula.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);	

			lin+=3;					

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_sprites,NULL,"%s",(tbblue_force_disable_layer_sprites.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_sprites,NULL,"%s",(tbblue_reveal_layer_sprites.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_layer_two,NULL,"%s",(tbblue_force_disable_layer_layer_two.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_layer2,NULL,"%s",(tbblue_reveal_layer_layer2.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;				
		}


				

        retorno_menu=menu_dibuja_menu(&tsconf_layer_settings_opcion_seleccionada,&item_seleccionado,array_menu_tsconf_layer_settings,"TSConf Layers" );

	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();

				//Nombre de ventana solo aparece en el caso de stdout
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);

	   cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);			   


}







#define TOTAL_PALETTE_WINDOW_X (menu_origin_x() )
#define TOTAL_PALETTE_WINDOW_Y 0
#define TOTAL_PALETTE_WINDOW_ANCHO 32
#define TOTAL_PALETTE_WINDOW_ALTO 24
#define TOTAL_PALETTE_COLORS_PER_WINDOW 16



int menu_display_total_palette_current_palette=0;
int menu_display_total_palette_current_colour=0;

//Si se muestra paleta total o paleta mapeada
int menu_display_total_palette_show_mapped=0;

//Retorna colores totales de una paleta ya sea total o mapeada
int menu_display_total_palette_get_total_colors(void)
{
	int limite;

	if (menu_display_total_palette_show_mapped==0) {
		limite=total_palette_colours_array[menu_display_total_palette_current_palette].total_colores;
	}
	else {
		limite=menu_debug_sprites_total_colors_mapped_palette(menu_display_total_palette_current_palette);
	}

	return limite;
}

zxvision_window *menu_display_total_palette_draw_barras_window;

//Muestra lista de colores o barras de colores, para una paleta total, o para la paleta mapeada
int menu_display_total_palette_lista_colores(int linea,int si_barras)
{

	char dumpmemoria[33];

	int linea_color;
	int limite;


	limite=menu_display_total_palette_get_total_colors();

	int current_color;
	int indice_paleta;
	int indice_color_final_rgb;
	int color_final_rgb;

		for (linea_color=0;linea_color<TOTAL_PALETTE_COLORS_PER_WINDOW &&
				menu_display_total_palette_current_colour+linea_color<limite;
				linea_color++) {

					current_color=menu_display_total_palette_current_colour+linea_color;

					int digitos_hexa;
					int digitos_dec;

					digitos_dec=menu_debug_get_total_digits_dec(limite-1);



					if (menu_display_total_palette_show_mapped==0) {

						indice_paleta=total_palette_colours_array[menu_display_total_palette_current_palette].indice_inicial;
						indice_color_final_rgb=indice_paleta+current_color;
						color_final_rgb=spectrum_colortable_normal[indice_color_final_rgb];



						sprintf (dumpmemoria,"%*d: RGB %06XH",digitos_dec,current_color,color_final_rgb);
					}

					else {
						indice_paleta=menu_debug_sprites_return_index_palette(menu_display_total_palette_current_palette, current_color);
						indice_color_final_rgb=menu_debug_sprites_return_color_palette(menu_display_total_palette_current_palette,current_color);
						color_final_rgb=spectrum_colortable_normal[indice_color_final_rgb];
						digitos_hexa=menu_debug_get_total_digits_hexa((menu_debug_sprites_max_value_mapped_palette(menu_display_total_palette_current_palette))-1);
						char priorityBit = ' ';
						if (MACHINE_IS_TBBLUE) {	// TBBlue colours can have priority bit set
							priorityBit = (indice_paleta&0x8000) ? '!' : ' ';
							indice_paleta &= 0x1FF;	// keep only 9b colors after priority bit check
						}

						int no_mostrar_indice=0;

						//Spectra ni speccy base no usan tabla de paleta
						if (menu_display_total_palette_current_palette==2 || menu_display_total_palette_current_palette==0) no_mostrar_indice=1;

						if (no_mostrar_indice) {
							sprintf (dumpmemoria,"%*d: RGB %06XH",digitos_dec,indice_paleta,color_final_rgb);
						}
						else {
							sprintf (dumpmemoria,"%*d:%c%0*XH RGB %06XH",digitos_dec,current_color,priorityBit,digitos_hexa,indice_paleta,color_final_rgb);
						}

					}



					int longitud_texto=strlen(dumpmemoria);

					//int posicion_barra_color_x=TOTAL_PALETTE_WINDOW_X+longitud_texto+2;
					//int posicion_barra_color_y=TOTAL_PALETTE_WINDOW_Y+3+linea_color;
					int posicion_barra_color_x=longitud_texto+2;
					int posicion_barra_color_y=3+linea_color;					

					//dibujar la barra de color
					if (si_barras) {
						menu_dibuja_rectangulo_relleno(menu_display_total_palette_draw_barras_window,posicion_barra_color_x*menu_char_width,posicion_barra_color_y*8,
											menu_char_width*(TOTAL_PALETTE_WINDOW_ANCHO-longitud_texto-3),8,indice_color_final_rgb);
					}

			 		else {
						//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
						zxvision_print_string_defaults_fillspc(menu_display_total_palette_draw_barras_window,1,linea++,dumpmemoria);
					}
		}

	zxvision_draw_window_contents(menu_display_total_palette_draw_barras_window);
 

	return linea;
}




void menu_display_total_palette_draw_barras(void)
{

				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista colores
				menu_display_total_palette_lista_colores(TOTAL_PALETTE_WINDOW_Y+3,0);

				//Esto tiene que estar despues de escribir la lista de colores, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();

				if (si_complete_video_driver()) {
					//Mostrar colores
					menu_display_total_palette_lista_colores(0,1);
				}
}

void menu_display_total_palette_cursor_arriba(void)
{
	if (menu_display_total_palette_current_colour>0) {
		menu_display_total_palette_current_colour--;
	}
}

void menu_display_total_palette_cursor_abajo(void)
{

	int limite=menu_display_total_palette_get_total_colors();

	if (menu_display_total_palette_current_colour<limite-1) {
		menu_display_total_palette_current_colour++;
	}

}

void menu_display_total_palette(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,TOTAL_PALETTE_WINDOW_X,TOTAL_PALETTE_WINDOW_Y,TOTAL_PALETTE_WINDOW_ANCHO,TOTAL_PALETTE_WINDOW_ALTO,
							TOTAL_PALETTE_WINDOW_ANCHO-1,TOTAL_PALETTE_WINDOW_ALTO-2,"Colour palettes");
	zxvision_draw_window(&ventana);

	z80_byte tecla;


	int salir=0;


	set_menu_overlay_function(menu_display_total_palette_draw_barras);
	menu_display_total_palette_draw_barras_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

    do {

		//Borramos lista de colores con espacios por si hay estos de antes

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;		
		
		int i;
		for (i=0;i<16;i++) zxvision_print_string_defaults_fillspc(&ventana,0,TOTAL_PALETTE_WINDOW_Y+3+i,"");

        menu_speech_tecla_pulsada=0; //Que envie a speech

		int linea=0;

		char textoshow[33];

		char nombre_paleta[33];

		if (menu_display_total_palette_show_mapped==0) {
			strcpy(nombre_paleta,total_palette_colours_array[menu_display_total_palette_current_palette].nombre_paleta);
		}
		else {
			menu_debug_sprites_get_palette_name(menu_display_total_palette_current_palette,nombre_paleta);
		}

		sprintf (textoshow,"Palette %d: %s",menu_display_total_palette_current_palette,nombre_paleta);
       	//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

		if (menu_display_total_palette_show_mapped==0) {
			sprintf (textoshow,"%s",total_palette_colours_array[menu_display_total_palette_current_palette].descripcion_paleta);
		}
		else {
			sprintf (textoshow,"Total colours in array: %d",menu_display_total_palette_get_total_colors() );
		}
		//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

   		//menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		//linea=menu_display_total_palette_lista_colores(linea,0);
		linea +=16;


		//printf ("zone size: %x dir: %x\n",menu_display_memory_zone_size,menu_display_total_palette_direccion);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		char buffer_linea[40];

		linea=TOTAL_PALETTE_WINDOW_Y+TOTAL_PALETTE_COLORS_PER_WINDOW+4;

															// 01234567890123456789012345678901
		sprintf (buffer_linea,"Move: Cursors,Q,A,PgUp,PgDn");

		//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

		sprintf (buffer_linea,"[%c] ~~Mapped palette",(menu_display_total_palette_show_mapped ? 'X' : ' ') );
		//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

		zxvision_draw_window_contents(&ventana);
			
		tecla=zxvision_common_getkey_refresh();		

		int aux_pgdnup;
		int limite;

				switch (tecla) {

					case 11:
						//arriba
						menu_display_total_palette_cursor_arriba();

						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion -=bytes_por_linea;
						//menu_display_total_palette_direccion=menu_display_total_palette_adjusta_en_negativo(menu_display_total_palette_direccion,bytes_por_linea);
					break;

					case 10:
						//abajo
						menu_display_total_palette_cursor_abajo();

						//menu_display_total_palette_ventana();


					break;

					case 24:
						//PgUp
						for (aux_pgdnup=0;aux_pgdnup<TOTAL_PALETTE_COLORS_PER_WINDOW;aux_pgdnup++) {
							menu_display_total_palette_cursor_arriba();
						}
						//menu_display_total_palette_ventana();

						//menu_display_total_palette_direccion -=bytes_por_ventana;
						//menu_display_total_palette_direccion=menu_display_total_palette_adjusta_en_negativo(menu_display_total_palette_direccion,bytes_por_ventana);
					break;

					case 25:
						//PgDn
						for (aux_pgdnup=0;aux_pgdnup<TOTAL_PALETTE_COLORS_PER_WINDOW;aux_pgdnup++) {
							menu_display_total_palette_cursor_abajo();
						}
						//menu_display_total_palette_ventana();
					break;

					case 'q':
						if (menu_display_total_palette_current_palette>0) {
							menu_display_total_palette_current_palette--;
							menu_display_total_palette_current_colour=0;
						}

						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion=menu_display_total_palette_change_pointer(menu_display_total_palette_direccion);
						//menu_display_total_palette_ventana();
					break;

					case 'a':
						if (menu_display_total_palette_show_mapped==0) {
							limite=TOTAL_PALETAS_COLORES;
						}

						else {
							limite=MENU_TOTAL_MAPPED_PALETTES;
						}

						if (menu_display_total_palette_current_palette<limite-1) {
							menu_display_total_palette_current_palette++;
							menu_display_total_palette_current_colour=0;
						}


						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion=menu_display_total_palette_change_pointer(menu_display_total_palette_direccion);
						//menu_display_total_palette_ventana();
					break;

					case 'm':
						menu_display_total_palette_show_mapped ^=1;
						menu_display_total_palette_current_palette=0;
						menu_display_total_palette_current_colour=0;
						//menu_display_total_palette_ventana();
					break;


					//Salir con ESC
					case 2:
						salir=1;
					break;
				}


        } while (salir==0);

				//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);


	cls_menu_overlay();
	zxvision_destroy_window(&ventana);


}

void menu_debug_disassemble_export(int p)
{

	char string_address[10];
	sprintf (string_address,"%XH",p);


    menu_ventana_scanf("Start?",string_address,10);

	menu_z80_moto_int inicio=parse_string_to_number(string_address);

	menu_ventana_scanf("End?",string_address,10);
	menu_z80_moto_int final=parse_string_to_number(string_address);

	if (final<inicio){
		menu_warn_message("End address must be higher or equal than start address");
		return;
	}

	char file_save[PATH_MAX];	
	int ret=menu_ask_file_to_save("Destination file","asm",file_save);

	if (!ret) {
		menu_warn_message("Export cancelled");
		return;
	}


	FILE *ptr_asmfile;
    ptr_asmfile=fopen(file_save,"wb");
    if (!ptr_asmfile) {
		debug_printf (VERBOSE_ERR,"Unable to open asm file");
		return;
    }
                  
 
	char dumpassembler[65];


	int longitud_opcode;

	//ponemos un final de un numero maximo de instrucciones
	//sera igual al tamaño de la zona de memoria
	int limite_instrucciones=menu_debug_memory_zone_size;

	int instrucciones=0;

	for (;inicio<=final && instrucciones<limite_instrucciones;instrucciones++) {

		menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,inicio,&longitud_opcode,0,0);


		inicio +=longitud_opcode;
		debug_printf (VERBOSE_DEBUG,"Exporting asm: %s",dumpassembler);

		//Agregar salto de linea
		int longitud_linea=strlen(dumpassembler);
		dumpassembler[longitud_linea++]='\n';
		dumpassembler[longitud_linea]=0;
		fwrite(&dumpassembler,1,longitud_linea,ptr_asmfile);
		//zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
	}	

	fclose(ptr_asmfile);

	menu_generic_message_splash("Export disassembly","Ok process finished");

}

z80_bit menu_debug_disassemble_hexa_view={0};

void menu_debug_disassemble(MENU_ITEM_PARAMETERS)
{

	//printf ("Opening disassemble menu\n");
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int ancho_total=32-1;

	if (CPU_IS_MOTOROLA) ancho_total=64-1;

	zxvision_new_window(&ventana,0,1,32,20,
							ancho_total,20-2,"Disassemble");
	zxvision_draw_window(&ventana);			

    //Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();



	z80_byte tecla;

    
    menu_z80_moto_int direccion=menu_debug_disassemble_last_ptr;
		

	do {
		int linea=0;

		int lineas_disass=0;
		const int lineas_total=15;

		char dumpassembler[65];

		int longitud_opcode;
		int longitud_opcode_primera_linea;

		menu_z80_moto_int dir=direccion;

		for (;lineas_disass<lineas_total;lineas_disass++,linea++) {

			//Formato de texto en buffer:
			//0123456789012345678901234567890
			//DDDD AABBCCDD OPCODE-----------
			//DDDD: Direccion
			//AABBCCDD: Volcado hexa

			//Metemos 30 espacios
		


			//menu_debug_dissassemble_una_instruccion(dumpassembler,dir,&longitud_opcode);
			menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,&longitud_opcode,menu_debug_disassemble_hexa_view.v,1);


			if (lineas_disass==0) longitud_opcode_primera_linea=longitud_opcode;

			dir +=longitud_opcode;
			zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
		}


	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
	menu_writing_inverse_color.v=1;



        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~M: Ch. pointer ~~E: Export");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~T: Toggle hexa");

		zxvision_draw_window_contents(&ventana);


	//Restaurar comportamiento atajos
	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;




		tecla=zxvision_common_getkey_refresh();				

                     
		int i;

        switch (tecla) {

			case 11:
				//arriba
				direccion=menu_debug_disassemble_subir(direccion);
			break;

			case 10:
				//abajo
				direccion +=longitud_opcode_primera_linea;
			break;

			//No llamamos a zxvision_handle_cursors_pgupdn para solo poder gestionar scroll ventana en horizontal,
			//el resto de teclas (cursores, pgup, dn etc) las gestionamos desde aqui de manera diferente

            //izquierda
            case 8:
				/*
				//Decir que se ha pulsado tecla para que no se relea
				menu_speech_tecla_pulsada=1;*/
				zxvision_handle_cursors_pgupdn(&ventana,tecla);
            break;

            //derecha
            case 9:
				/*
				//Decir que se ha pulsado tecla para que no se relea
				menu_speech_tecla_pulsada=1;*/
				zxvision_handle_cursors_pgupdn(&ventana,tecla);
			break;					

			case 24:
				//PgUp
				for (i=0;i<lineas_total;i++) direccion=menu_debug_disassemble_subir(direccion);
			break;

			case 25:
				//PgDn
				direccion=dir;
			break;

			case 'm':
				//Usamos misma funcion de menu_debug_hexdump_change_pointer
				direccion=menu_debug_hexdump_change_pointer(direccion);
				zxvision_draw_window(&ventana);
			break;

			case 'e':
				menu_debug_disassemble_export(direccion);
				zxvision_draw_window(&ventana);
			break;

			case 't':
				menu_debug_disassemble_hexa_view.v ^=1;
			break;

		}


	} while (tecla!=2); 


    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		

 

}


void menu_debug_assemble(MENU_ITEM_PARAMETERS)
{

	//printf ("Opening disassemble menu\n");
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int ancho_total=32-1;

	if (CPU_IS_MOTOROLA) ancho_total=64-1;

	zxvision_new_window(&ventana,0,1,32,20,
							ancho_total,20-2,"Assemble");
	zxvision_draw_window(&ventana);			

    //Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();


    
    menu_z80_moto_int direccion=menu_debug_disassemble_last_ptr;

	menu_z80_moto_int direccion_ensamblado=direccion;

	int salir=0;
	int lineas_ensambladas=0;
		

	do {
		int linea=0;

		int lineas_disass=0;
		const int lineas_total=15;

		char dumpassembler[65];

		int longitud_opcode;
		int longitud_opcode_primera_linea;

		menu_z80_moto_int dir=direccion;

		for (;lineas_disass<lineas_total;lineas_disass++,linea++) {

			//Formato de texto en buffer:
			//0123456789012345678901234567890
			//DDDD AABBCCDD OPCODE-----------
			//DDDD: Direccion
			//AABBCCDD: Volcado hexa

			//Metemos 30 espacios
		


			//menu_debug_dissassemble_una_instruccion(dumpassembler,dir,&longitud_opcode);
			menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,&longitud_opcode,menu_debug_disassemble_hexa_view.v,1);


			if (lineas_disass==0) longitud_opcode_primera_linea=longitud_opcode;

			dir +=longitud_opcode;
			zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
		}


		zxvision_draw_window_contents(&ventana);



		char string_opcode[256];
		string_opcode[0]=0;


		char texto_titulo[256];
		sprintf (texto_titulo,"Assemble at %XH",direccion_ensamblado);

    	menu_ventana_scanf(texto_titulo,string_opcode,256);
		zxvision_draw_window(&ventana);

		if (string_opcode[0]==0) salir=1;
		else {


				z80_byte destino_ensamblado[MAX_DESTINO_ENSAMBLADO];


				int longitud_destino=assemble_opcode(direccion_ensamblado,string_opcode,destino_ensamblado);

				if (longitud_destino==0) {
					menu_error_message("Error. Invalid opcode");
					//escribir_socket_format(misocket,"Error. Invalid opcode: %s\n",texto);
					salir=1;
				}

				else {
					menu_debug_set_memory_zone_attr();
					unsigned int direccion_escribir=direccion_ensamblado;
					int i;
					for (i=0;i<longitud_destino;i++) {
						menu_debug_write_mapped_byte(direccion_escribir++,destino_ensamblado[i]);
					}

				}

				direccion_ensamblado+=longitud_destino;

				lineas_ensambladas++;

				//Desensamblar desde la siguiente instruccion si conviene
				if (lineas_ensambladas>5) {
					direccion +=longitud_opcode_primera_linea;
				}

		}
	
	} while (!salir);


		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		

 

}



void menu_spectrum_core_reduced(MENU_ITEM_PARAMETERS)
{
	core_spectrum_uses_reduced.v ^=1;

	set_cpu_core_loop();

}



void menu_tbblue_deny_turbo_rom(MENU_ITEM_PARAMETERS)
{

	tbblue_deny_turbo_rom.v ^=1;
}

void menu_hardware_top_speed(MENU_ITEM_PARAMETERS)
{
	top_speed_timer.v ^=1;
}

void menu_turbo_mode(MENU_ITEM_PARAMETERS)
{
	if (cpu_turbo_speed==MAX_CPU_TURBO_SPEED) cpu_turbo_speed=1;
	else cpu_turbo_speed *=2;

	cpu_set_turbo_speed();
}

void menu_zxuno_deny_turbo_bios_boot(MENU_ITEM_PARAMETERS)
{
	zxuno_deny_turbo_bios_boot.v ^=1;
}

void menu_cpu_type(MENU_ITEM_PARAMETERS)
{
	z80_cpu_current_type++;
	if (z80_cpu_current_type>=TOTAL_Z80_CPU_TYPES) z80_cpu_current_type=0;
}

//menu cpu settings
void menu_cpu_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_cpu_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

		//hotkeys usadas: todc

		char buffer_velocidad[16];

		if (CPU_IS_Z80 && !MACHINE_IS_Z88) {
			int cpu_hz=get_cpu_frequency();
			int cpu_khz=cpu_hz/1000;

			//Obtener decimales
			int mhz_enteros=cpu_khz/1000;
			int decimal_mhz=cpu_khz-(mhz_enteros*1000);

								//01234567890123456789012345678901
								//           1234567890
								//Turbo: 16X 99.999 MHz
			sprintf(buffer_velocidad,"%d.%d MHz",mhz_enteros,decimal_mhz);
		}
		else {
			buffer_velocidad[0]=0;
		}

		menu_add_item_menu_inicial_format(&array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_turbo_mode,NULL,"~~Turbo [%dX %s]",cpu_turbo_speed,buffer_velocidad);
		menu_add_item_menu_shortcut(array_menu_cpu_settings,'t');
		menu_add_item_menu_tooltip(array_menu_cpu_settings,"Changes only the Z80 speed");
		menu_add_item_menu_ayuda(array_menu_cpu_settings,"Changes only the Z80 speed. Do not modify FPS, interrupts or any other parameter. "
					"Some machines, like ZX-Uno or Chloe, change this setting");





		if (MACHINE_IS_ZXUNO) {
					menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_zxuno_deny_turbo_bios_boot,NULL,"[%c] ~~Deny turbo on boot",
							(zxuno_deny_turbo_bios_boot.v ? 'X' : ' ') );
					menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
					menu_add_item_menu_tooltip(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
					menu_add_item_menu_ayuda(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
	  }

		if (MACHINE_IS_TBBLUE) {
					menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_tbblue_deny_turbo_rom,NULL,"[%c] ~~Deny turbo on ROM",
							(tbblue_deny_turbo_rom.v ? 'X' : ' ') );
					menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
					menu_add_item_menu_tooltip(array_menu_cpu_settings,"Denies changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");
					menu_add_item_menu_ayuda(array_menu_cpu_settings,"Denies changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");
	  }	  

		if (!MACHINE_IS_Z88) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_hardware_top_speed,NULL,"[%c] T~~op Speed",(top_speed_timer.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'o');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Runs at maximum speed, when menu closed. Not available on Z88");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"Runs at maximum speed, using 100% of CPU of host machine, when menu closed. "
						"The display is refreshed 1 time per second. This mode is also entered when loading a real tape and "
						"accelerate loaders setting is enabled. Not available on Z88");

		}	  

		if (CPU_IS_Z80) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_cpu_type,NULL,"Z80 CPU Type [%s]",z80_cpu_types_strings[z80_cpu_current_type]);
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Chooses the cpu type");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"CPU type modifies the way the CPU fires an IM0 interrupt, or the behaviour of opcode OUT (C),0, for example");
		}		

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_spectrum_core_reduced,NULL,"Spectrum ~~core [%s]",
			(core_spectrum_uses_reduced.v ? "Reduced" : "Normal") );
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'c');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Switches between the normal Spectrum core or the reduced core");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"When using the Spectrum reduced core, the following features are NOT available or are NOT properly emulated:\n"
				"Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, Multiface, RZX, Raster interrupts, ZX-Uno DMA, TBBlue DMA, Datagear DMA, TBBlue Copper, Audio DAC, Stereo AY, Video out to file, Last core frame statistics");
		}



                menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_cpu_settings);

                retorno_menu=menu_dibuja_menu(&cpu_settings_opcion_seleccionada,&item_seleccionado,array_menu_cpu_settings,"CPU Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



}



void menu_display_snow_effect(MENU_ITEM_PARAMETERS)
{
	snow_effect_enabled.v ^=1;
}


void menu_display_inves_ula_bright_error(MENU_ITEM_PARAMETERS)
{
	inves_ula_bright_error.v ^=1;
}


void menu_display_slow_adjust(MENU_ITEM_PARAMETERS)
{
	video_zx8081_lnctr_adjust.v ^=1;
}



void menu_display_estabilizador_imagen(MENU_ITEM_PARAMETERS)
{
	video_zx8081_estabilizador_imagen.v ^=1;
}

void menu_display_interlace(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_mode.v) disable_interlace();
	else enable_interlace();
}


void menu_display_interlace_scanlines(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_scanlines.v) disable_scanlines();
	else enable_scanlines();
}

void menu_display_gigascreen(MENU_ITEM_PARAMETERS)
{
        if (gigascreen_enabled.v) disable_gigascreen();
        else enable_gigascreen();
}

void menu_display_chroma81(MENU_ITEM_PARAMETERS)
{
        if (chroma81.v) disable_chroma81();
        else enable_chroma81();
}

void menu_display_ulaplus(MENU_ITEM_PARAMETERS)
{
        if (ulaplus_presente.v) disable_ulaplus();
        else enable_ulaplus();
}


void menu_display_autodetect_chroma81(MENU_ITEM_PARAMETERS)
{
	autodetect_chroma81.v ^=1;
}


void menu_display_spectra(MENU_ITEM_PARAMETERS)
{
	if (spectra_enabled.v) spectra_disable();
	else spectra_enable();
}

void menu_display_snow_effect_margin(MENU_ITEM_PARAMETERS)
{
	snow_effect_min_value++;
	if (snow_effect_min_value==8) snow_effect_min_value=1;
}

void menu_display_timex_video(MENU_ITEM_PARAMETERS)
{
	if (timex_video_emulation.v) disable_timex_video();
	else enable_timex_video();
}

void menu_display_minimo_vsync(MENU_ITEM_PARAMETERS)
{

        menu_hardware_advanced_input_value(100,999,"Minimum vsync length",&minimo_duracion_vsync);
}

void menu_display_timex_video_512192(MENU_ITEM_PARAMETERS)
{

	timex_mode_512192_real.v ^=1;
}

void menu_display_cpc_force_mode(MENU_ITEM_PARAMETERS)
{
	if (cpc_forzar_modo_video.v==0) {
		cpc_forzar_modo_video.v=1;
		cpc_forzar_modo_video_modo=0;
	}
	else {
		cpc_forzar_modo_video_modo++;
		if (cpc_forzar_modo_video_modo==4) {
			cpc_forzar_modo_video_modo=0;
			cpc_forzar_modo_video.v=0;
		}
	}
}

void menu_display_refresca_sin_colores(MENU_ITEM_PARAMETERS)
{
	scr_refresca_sin_colores.v ^=1;
	modificado_border.v=1;
}


void menu_display_timex_force_line_512192(MENU_ITEM_PARAMETERS)
{
	if (timex_ugly_hack_last_hires==0) timex_ugly_hack_last_hires=198;

	        char string_num[4];

        sprintf (string_num,"%d",timex_ugly_hack_last_hires);

        menu_ventana_scanf("Scanline",string_num,4);

        timex_ugly_hack_last_hires=parse_string_to_number(string_num);
}

void menu_display_timex_ugly_hack(MENU_ITEM_PARAMETERS)
{
	timex_ugly_hack_enabled ^=1;
}

void menu_spritechip(MENU_ITEM_PARAMETERS)
{
	if (spritechip_enabled.v) spritechip_disable();
	else spritechip_enable();
}


void menu_display_emulate_fast_zx8081(MENU_ITEM_PARAMETERS)
{
	video_fast_mode_emulation.v ^=1;
	modificado_border.v=1;
}



void menu_display_emulate_zx8081display_spec(MENU_ITEM_PARAMETERS)
{
	if (simulate_screen_zx8081.v==1) simulate_screen_zx8081.v=0;
	else {
		simulate_screen_zx8081.v=1;
		umbral_simulate_screen_zx8081=4;
	}
	modificado_border.v=1;
}


void menu_display_osd_word_kb_length(MENU_ITEM_PARAMETERS)
{

	char string_length[4];

        sprintf (string_length,"%d",adventure_keyboard_key_length);

        menu_ventana_scanf("Length? (10-100)",string_length,4);

        int valor=parse_string_to_number(string_length);
	if (valor<10 || valor>100) {
		debug_printf (VERBOSE_ERR,"Invalid value");
	}

	else {
		adventure_keyboard_key_length=valor;
	}

}


void menu_display_osd_word_kb_finalspc(MENU_ITEM_PARAMETERS)
{
	adventure_keyboard_send_final_spc ^=1;
}


void menu_display_emulate_zx8081_thres(MENU_ITEM_PARAMETERS)
{

        char string_thres[3];

        sprintf (string_thres,"%d",umbral_simulate_screen_zx8081);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

	umbral_simulate_screen_zx8081=parse_string_to_number(string_thres);
	if (umbral_simulate_screen_zx8081<1 || umbral_simulate_screen_zx8081>16) umbral_simulate_screen_zx8081=4;

}


int menu_display_settings_disp_zx8081_spectrum(void)
{

	//esto solo en spectrum y si el driver no es curses y si no hay rainbow
	if (!strcmp(scr_driver_name,"curses")) return 0;
	if (rainbow_enabled.v==1) return 0;

	return !menu_cond_zx8081();
}


void menu_display_arttext(MENU_ITEM_PARAMETERS)
{
	texto_artistico.v ^=1;
}



#ifdef COMPILE_AA
void menu_display_slowaa(MENU_ITEM_PARAMETERS)
{
	scraa_fast ^=1;
}
#else
void menu_display_slowaa(MENU_ITEM_PARAMETERS){}
#endif



void menu_display_zx8081_wrx(MENU_ITEM_PARAMETERS)
{
	if (wrx_present.v) {
		disable_wrx();
	}

	else {
		enable_wrx();
	}
	//wrx_present.v ^=1;
}





void menu_display_x_offset(MENU_ITEM_PARAMETERS)
{
	offset_zx8081_t_coordx +=8;
        if (offset_zx8081_t_coordx>=30*8) offset_zx8081_t_coordx=-30*8;
}


int menu_display_emulate_zx8081_cond(void)
{
	return simulate_screen_zx8081.v;
}


void menu_display_autodetect_rainbow(MENU_ITEM_PARAMETERS)
{
	autodetect_rainbow.v ^=1;
}

void menu_display_autodetect_wrx(MENU_ITEM_PARAMETERS)
{
        autodetect_wrx.v ^=1;
}

int menu_display_aa_cond(void)
{
        if (!strcmp(scr_driver_name,"aa")) return 1;

        else return 0;
}


void menu_display_tsconf_vdac(MENU_ITEM_PARAMETERS)
{
	tsconf_vdac_with_pwm.v ^=1;

	menu_interface_rgb_inverse_common();
}

void menu_display_tsconf_pal_depth(MENU_ITEM_PARAMETERS)
{
	tsconf_palette_depth--;
	if (tsconf_palette_depth<2) tsconf_palette_depth=5;

	menu_interface_rgb_inverse_common();

}

void menu_display_rainbow(MENU_ITEM_PARAMETERS)
{
	if (rainbow_enabled.v==0) enable_rainbow();
	else disable_rainbow();


}

void menu_vofile_insert(MENU_ITEM_PARAMETERS)
{

        if (vofile_inserted.v==0) {
                init_vofile();
                //Si todo ha ido bien
                if (vofile_inserted.v) {
                        menu_generic_message_format("File information","%s\n%s\n\n%s",
												last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_video,last_message_helper_aofile_vofile_util);
                }

        }

        else if (vofile_inserted.v==1) {
                close_vofile();
        }

}


int menu_vofile_cond(void)
{
        if (vofilename!=NULL) return 1;
        else return 0;
}

void menu_vofile(MENU_ITEM_PARAMETERS)
{

        vofile_inserted.v=0;


        char *filtros[2];

        filtros[0]="rwv";
        filtros[1]=0;


        if (menu_filesel("Select Video File",filtros,vofilename_file)==1) {

                 //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(vofilename_file, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
                                vofilename=NULL;
                                return;
                        }

                }



                vofilename=vofilename_file;
        }

        else {
                vofilename=NULL;
        }


}

void menu_vofile_fps(MENU_ITEM_PARAMETERS)
{
	if (vofile_fps==1) {
		vofile_fps=50;
		return;
	}

        if (vofile_fps==2) {
                vofile_fps=1;
                return;
        }


        if (vofile_fps==5) {
                vofile_fps=2;
                return;
        }

        if (vofile_fps==10) {
                vofile_fps=5;
                return;
        }

        if (vofile_fps==25) {
                vofile_fps=10;
                return;
        }


        if (vofile_fps==50) {
                vofile_fps=25;
                return;
        }

}

int menu_display_curses_cond(void)
{
	if (!strcmp(scr_driver_name,"curses")) return 1;

	else return 0;
}


int menu_display_cursesstdout_cond(void)
{
	if (menu_display_curses_cond() ) return 1;
	if (menu_cond_stdout() ) return 1;

	return 0;
}



int menu_display_cursesstdoutsimpletext_cond(void)
{
	if (menu_display_cursesstdout_cond() ) return 1;
	if (menu_cond_simpletext() ) return 1;

	return 0;
}



int menu_display_arttext_cond(void)
{

	if (!menu_display_cursesstdout_cond()) return 0;

	//en zx80 y 81 no hay umbral, no tiene sentido. ahora si. hay rainbow de zx8081
	//if (machine_type>=20 && machine_type<=21) return 0;
	if (use_scrcursesw.v) return 1;
	if (texto_artistico.v) return 1;

    return 0;
}

int menu_cond_stdout_simpletext(void)
{
	if (menu_cond_stdout() || menu_cond_simpletext() ) return 1;
	return 0;
}


//En curses y stdout solo se permite para zx8081
int menu_cond_realvideo_curses_stdout_zx8081(void)
{
	if (menu_cond_stdout() || menu_cond_curses() ) {
		if (MACHINE_IS_SPECTRUM ) return 0;
	}

	return 1;
}


void menu_display_stdout_simpletext_automatic_redraw(MENU_ITEM_PARAMETERS)
{
	stdout_simpletext_automatic_redraw.v ^=1;
}



void menu_display_send_ansi(MENU_ITEM_PARAMETERS)
{
	screen_text_accept_ansi ^=1;
}

void menu_display_arttext_thres(MENU_ITEM_PARAMETERS)
{

        char string_thres[3];

        sprintf (string_thres,"%d",umbral_arttext);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

        umbral_arttext=parse_string_to_number(string_thres);
        if (umbral_arttext<1 || umbral_arttext>16) umbral_arttext=4;

}


void menu_display_text_brightness(MENU_ITEM_PARAMETERS)
{

        char string_bri[4];

        sprintf (string_bri,"%d",screen_text_brightness);

        menu_ventana_scanf("Brightness? (0-100)",string_bri,4);

	int valor=parse_string_to_number(string_bri);
	if (valor<0 || valor>100) debug_printf (VERBOSE_ERR,"Invalid brightness value %d",valor);

	else screen_text_brightness=valor;

}






void menu_display_stdout_simpletext_fps(MENU_ITEM_PARAMETERS)
{
	    char string_fps[3];

        sprintf (string_fps,"%d",50/scrstdout_simpletext_refresh_factor);

        menu_ventana_scanf("FPS? (1-50)",string_fps,3);

        int valor=parse_string_to_number(string_fps);
		scr_set_fps_stdout_simpletext(valor);

}


void menu_display_ocr_23606(MENU_ITEM_PARAMETERS)
{

ocr_settings_not_look_23606.v ^=1;

}



#ifdef COMPILE_CURSESW
void menu_display_cursesw_ext(MENU_ITEM_PARAMETERS)
{
	use_scrcursesw.v ^=1;

	if (use_scrcursesw.v) {
		//Reiniciar locale
		cursesw_ext_init();
	}
}
#endif
						

void menu_textdrivers_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_textdrivers_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		char buffer_string[50];


		//Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
		//menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"---Text Driver Settings--",MENU_OPCION_UNASSIGNED,NULL,NULL);
		menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


                //para stdout y simpletext
                if (menu_cond_stdout_simpletext() ) {
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"[%c]   Stdout automatic redraw", (stdout_simpletext_automatic_redraw.v==1 ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"It enables automatic display redraw");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"It enables automatic display redraw");


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"[%c]   Send ANSI Ctrl Sequence",(screen_text_accept_ansi==1 ? 'X' : ' ') );

						if (stdout_simpletext_automatic_redraw.v) {
							menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_fps,NULL,"[%2d]  Redraw fps", 50/scrstdout_simpletext_refresh_factor);
						}

                }

		if (menu_display_cursesstdout_cond() ) {
                        //solo en caso de curses o stdout

						
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext,menu_display_cursesstdout_cond,"[%c]   Text artistic emulation", (texto_artistico.v==1 ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on stdout and curses drivers");

                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on curses, stdout and simpletext drivers. "
                                        "If disabled, unknown characters are written with ?");

#ifdef COMPILE_CURSESW
						menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_cursesw_ext,NULL,"[%c]   Extended utf blocky", (use_scrcursesw.v ? 'X' : ' ') );
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Use extended utf characters to have 64x48 display, only on Spectrum and curses drivers");
						menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Use extended utf characters to have 64x48 display, only on Spectrum and curses drivers");
#endif
								


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext_thres,menu_display_arttext_cond,"[%2d]  Pixel threshold",umbral_arttext);
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation or utf enabled");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation or utf enabled");
                                        
                                        





                        if (rainbow_enabled.v) {
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_brightness,NULL,"[%3d] Text brightness",screen_text_brightness);
                                menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                                menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                        }

                }






if (menu_display_aa_cond() ) {

#ifdef COMPILE_AA
                        sprintf (buffer_string,"Slow AAlib emulation: %s", (scraa_fast==0 ? "On" : "Off"));
#else
                        sprintf (buffer_string,"Slow AAlib emulation: Off");
#endif
                        menu_add_item_menu(array_menu_textdrivers_settings,buffer_string,MENU_OPCION_NORMAL,menu_display_slowaa,menu_display_aa_cond);

                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");

                }



                menu_add_item_menu(array_menu_textdrivers_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_textdrivers_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_textdrivers_settings);

                retorno_menu=menu_dibuja_menu(&textdrivers_settings_opcion_seleccionada,&item_seleccionado,array_menu_textdrivers_settings,"Text Driver Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



void menu_display_cpc_double_vsync(MENU_ITEM_PARAMETERS)
{
	cpc_send_double_vsync.v ^=1;
}

void menu_display_16c_mode(MENU_ITEM_PARAMETERS)
{
    if (pentagon_16c_mode_available.v) disable_16c_mode();
    else enable_16c_mode();
}


//menu display settings
void menu_settings_display(MENU_ITEM_PARAMETERS)
{

	menu_item *array_menu_settings_display;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		//char string_aux[50],string_aux2[50],emulate_zx8081_disp[50],string_arttext[50],string_aaslow[50],emulate_zx8081_thres[50],string_arttext_threshold[50];
		//char buffer_string[50];

		//hotkeys usadas: ricglwmptez


                char string_vofile_shown[10];
                menu_tape_settings_trunc_name(vofilename,string_vofile_shown,10);


	                menu_add_item_menu_inicial_format(&array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile,NULL,"Video out to file: %s",string_vofile_shown);
        	        menu_add_item_menu_tooltip(array_menu_settings_display,"Saves the video output to a file");
                	menu_add_item_menu_ayuda(array_menu_settings_display,"The generated file have raw format. You can see the file parameters "
					"on the console enabling verbose debug level to 2 minimum.\n"
					"A watermark is added to the final video, as you may see when you activate it\n"
					"Note: Gigascreen, Interlaced effects or menu windows are not saved to file."

					);

			if (menu_vofile_cond() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_fps,menu_vofile_cond,"[%d] FPS Video file",50/vofile_fps);
	        	menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_insert,menu_vofile_cond,"[%c] Video file enabled",(vofile_inserted.v ? 'X' : ' ' ));
			}

			else {
                	  menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
			}



                if (!MACHINE_IS_Z88) {


                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_rainbow,NULL,"[%c] Autodetect Real Video",(autodetect_rainbow.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable Real Video");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable Real Video. "
                                        "On Spectrum, it detects the reading of idle bus or repeated border changes. "
                                        "On ZX80/81, it detects the I register on a non-normal value when executing video display. "
					"On all machines, it also detects when loading a real tape. "
                                        );
                }




			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_rainbow,menu_display_rainbow_cond,"[%c] ~~Real Video",(rainbow_enabled.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'r');

			menu_add_item_menu_tooltip(array_menu_settings_display,"Enable Real Video. Enabling it makes display as a real machine");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Real Video makes display works as in the real machine. It uses a bit more CPU than disabling it.\n\n"
					"On Spectrum, display is drawn every scanline. "
					"It enables hi-res colour (rainbow) on the screen and on the border, Gigascreen, Interlaced, ULAplus, Spectra, Timex Video, snow effect, idle bus reading and some other advanced features. "
					"Also enables all the Inves effects.\n"
					"Disabling it, the screen is drawn once per frame (1/50) and the previous effects "
					"are not supported.\n\n"
					"On ZX80/ZX81, enables hi-res display and loading/saving stripes on the screen, and the screen is drawn every scanline.\n"
					"By disabling it, the screen is drawn once per frame, no hi-res display, and only text mode is supported.\n\n"
					"On Z88, display is drawn the same way as disabling it; it is only used when enabling Video out to file.\n\n"
					"Real Video can be enabled on all the video drivers, but on curses, stdout and simpletext (in Spectrum and Z88 machines), the display drawn is the same "
					"as on non-Real Video, but you can have idle bus support on these drivers. "
					"Curses, stdout and simpletext drivers on ZX80/81 machines do have Real Video display."
					);

                if (MACHINE_IS_TSCONF) {
                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_vdac,NULL,"[%c] TSConf VDAC PWM",
                        (tsconf_vdac_with_pwm.v ? 'X' : ' ')     );

                        menu_add_item_menu_tooltip(array_menu_settings_display,"Enables full vdac colour palette or PWM style");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Full vdac colour palette gives you different colour levels for every 5 bit colour component.\n"
					"With PWM mode it gives you 5 bit values different from 0..23, but from 24 to 31 are all set to value 255");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_pal_depth,NULL,
					 "[%d] TSConf palette depth",tsconf_palette_depth);




                }


		if (MACHINE_IS_CPC) {
				if (cpc_forzar_modo_video.v==0)
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[ ] Force Video Mode");
				else
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[%d] Force Video Mode",
						cpc_forzar_modo_video_modo);

				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_double_vsync,NULL,"[%c] Double Vsync",(cpc_send_double_vsync.v==1 ? 'X' : ' ') );
				menu_add_item_menu_tooltip(array_menu_settings_display,"Workaround to avoid hang on some games");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Workaround to avoid hang on some games");
		}


		if (!MACHINE_IS_Z88) {


			if (menu_cond_realvideo() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace,menu_cond_realvideo,"[%c] ~~Interlaced mode", (video_interlaced_mode.v==1 ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'i');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enable interlaced mode");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Interlaced mode draws the screen like the machine on a real TV: "
					"Every odd frame, odd lines on TV are drawn; every even frame, even lines on TV are drawn. It can be used "
					"to emulate twice the vertical resolution of the machine (384) or simulate different colours. "
					"This effect is only emulated with vertical zoom multiple of two: 2,4,6... etc");


				if (video_interlaced_mode.v) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace_scanlines,NULL,"[%c] S~~canlines", (video_interlaced_scanlines.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'c');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable scanlines on interlaced mode");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Scanlines draws odd lines a bit darker than even lines");
				}


				if (!MACHINE_IS_TBBLUE) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_gigascreen,NULL,"[%c] ~~Gigascreen",(gigascreen_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'g');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable gigascreen colours");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Gigascreen enables more than 15 colours by combining pixels "
							"of even and odd frames. The total number of different colours is 102");
				}



				if (MACHINE_IS_SPECTRUM && !MACHINE_IS_ZXEVO && !MACHINE_IS_TBBLUE)  {

					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect,NULL,"[%c] Snow effect support", (snow_effect_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable snow effect on Spectrum");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Snow effect is a bug on some Spectrum models "
						"(models except +2A and +3) that draws corrupted pixels when I register is pointed to "
						"slow RAM.");
						// Even on 48k models it resets the machine after some seconds drawing corrupted pixels");

					if (snow_effect_enabled.v==1) {
						menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect_margin,NULL,"[%d] Snow effect threshold",snow_effect_min_value);
					}
				}


				if (MACHINE_IS_INVES) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_inves_ula_bright_error,NULL,"[%c] Inves bright error",(inves_ula_bright_error.v ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1. Seems it only happens with RF or RGB connection");

				}



			}
		}

		//para stdout

		/*
#ifdef COMPILE_STDOUT
		if (menu_cond_stdout() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"Stdout automatic redraw: %s", (stdout_simpletext_automatic_redraw.v==1 ? "On" : "Off"));
			menu_add_item_menu_tooltip(array_menu_settings_display,"It enables automatic display redraw");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It enables automatic display redraw");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"Send ANSI Control Sequence: %s",(screen_text_accept_ansi==1 ? "On" : "Off"));

		}

#endif

		*/


		if (menu_cond_zx8081_realvideo()) {

		//z80_bit video_zx8081_estabilizador_imagen;

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_estabilizador_imagen,menu_cond_zx8081_realvideo,"[%c] Horizontal stabilization", (video_zx8081_estabilizador_imagen.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Horizontal image stabilization");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"Horizontal image stabilization. Usually enabled.");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_slow_adjust,menu_cond_zx8081_realvideo,"[%c] ~~LNCTR video adjust", (video_zx8081_lnctr_adjust.v==1 ? 'X' : ' '));
			//l repetida con load screen, pero como esa es de spectrum, no coinciden
			menu_add_item_menu_shortcut(array_menu_settings_display,'l');
			menu_add_item_menu_tooltip(array_menu_settings_display,"LNCTR video adjust");
			menu_add_item_menu_ayuda(array_menu_settings_display,"LNCTR video adjust change sprite offset when drawing video images. "
				"If you see your hi-res image is not displayed well, try changing it");




        	        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_x_offset,menu_cond_zx8081_realvideo,"[%d] Video x_offset",offset_zx8081_t_coordx);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video horizontal image offset");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video horizontal image offset, usually you don't need to change this");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_minimo_vsync,menu_cond_zx8081_realvideo,"[%d] Video min. vsync length",minimo_duracion_vsync);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video minimum vsync length in t-states");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video minimum vsync length in t-states");


                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_wrx,NULL,"[%c] Autodetect WRX",(autodetect_wrx.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable WRX mode on ZX80/81");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable WRX. "
                                                "On ZX80/81, it detects the I register on a non-normal value when executing video display. "
						"In some cases, chr$128 and udg modes are detected incorrectly as WRX");


	                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_zx8081_wrx,menu_cond_zx8081_realvideo,"[%c] ~~WRX", (wrx_present.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'w');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables WRX hi-res mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables WRX hi-res mode");



		}

		else {

			if (menu_cond_zx8081() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_fast_zx8081,menu_cond_zx8081_no_realvideo,"[%c] ZX80/81 detect fast mode", (video_fast_mode_emulation.v==1 ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
			}

		}

		if (MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_chroma81,NULL,"[%c] Autodetect Chroma81",(autodetect_chroma81.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect Chroma81");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Detects when Chroma81 video mode is needed and enable it");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_chroma81,NULL,"[%c] Chro~~ma81 support",(chroma81.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'m');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Chroma81 colour video mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Chroma81 colour video mode");

		}


		if (MACHINE_IS_SPECTRUM && !MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ulaplus,NULL,"[%c] ULA~~plus support",(ulaplus_presente.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'p');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ULAplus support");
			menu_add_item_menu_ayuda(array_menu_settings_display,"The following ULAplus modes are supported:\n"
						"Mode 1: Standard 256x192 64 colours\n"
						"Mode 3: Linear mode 128x96, 16 colours per pixel (radastan mode)\n"
						"Mode 5: Linear mode 256x96, 16 colours per pixel (ZEsarUX mode 0)\n"
						"Mode 7: Linear mode 128x192, 16 colours per pixel (ZEsarUX mode 1)\n"
						"Mode 9: Linear mode 256x192, 16 colours per pixel (ZEsarUX mode 2)\n"
			);
		}

		if (MACHINE_IS_PENTAGON) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_16c_mode,NULL,"[%c] 16C mode support",(pentagon_16c_mode_available.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables 16C video mode support");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables 16C video mode support. That brings you mode 256x192x16 colour on Pentagon");
		}

		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video,NULL,"[%c] ~~Timex video support",(timex_video_emulation.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_settings_display,'t');
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Timex Video modes");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"The following Timex Video modes are emulated:\n"
						"Mode 0: Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum)\n"
						"Mode 1: Video data at address 24576 and 8x8 color attributes at address 30720\n"
						"Mode 2: Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576\n"
						"Mode 6: Hi-res mode 512x192, monochrome.");

			if (timex_video_emulation.v && !MACHINE_IS_TBBLUE) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video_512192,NULL,"[%c] Timex Real 512x192",(timex_mode_512192_real.v ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Selects between real 512x192 or scaled 256x192");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Real 512x192 does not support scanline effects (it draws the display at once). "
							"If not enabled real, it draws scaled 256x192 but does support scanline effects");



				if (timex_mode_512192_real.v==0) {

					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_ugly_hack,NULL,"[%c] Ugly hack",(timex_ugly_hack_enabled ? 'X' : ' ') );
					menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
					menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");

					if (timex_ugly_hack_enabled) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_force_line_512192,NULL,"[%d] Force 512x192 at",timex_ugly_hack_last_hires);
					menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
					menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");
					}
				}

			}



			if (!MACHINE_IS_ZXEVO) {

				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_spectra,NULL,"[%c] Sp~~ectra support",(spectra_enabled.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'e');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Spectra video modes");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Spectra video modes. All video modes are fully emulated");


				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_spritechip,NULL,"[%c] ~~ZGX Sprite Chip",(spritechip_enabled.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_settings_display,'z');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ZGX Sprite Chip");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables ZGX Sprite Chip");
			}


		}




		if (MACHINE_IS_SPECTRUM && rainbow_enabled.v==0) {
	                menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081display_spec,menu_display_settings_disp_zx8081_spectrum,"[%c] ZX80/81 Display on Speccy", (simulate_screen_zx8081.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Simulates the resolution of ZX80/81 on the Spectrum");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It makes the resolution of display on Spectrum like a ZX80/81, with no colour. "
					"This mode is not supported with real video enabled");


			if (menu_display_emulate_zx8081_cond() ){
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081_thres,menu_display_emulate_zx8081_cond,"[%d] Pixel threshold",umbral_simulate_screen_zx8081);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
					   "when ZX80/81 Display on Speccy enabled");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
					   "when ZX80/81 Display on Speccy enabled");
			}


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_refresca_sin_colores,NULL,"[%c] Colours enabled",(scr_refresca_sin_colores.v==0 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Disables colours for Spectrum display");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Disables colours for Spectrum display");



		}



		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_CPC) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_length,NULL,"[%d] OSD Adventure KB length",adventure_keyboard_key_length);

			menu_add_item_menu_tooltip(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard, in 1/50 seconds (default 50)");

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_finalspc,NULL,"[%c] OSD Adv. final space",
				(adventure_keyboard_send_final_spc ? 'X' : ' '));
					

			menu_add_item_menu_tooltip(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
			

		}

		menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ocr_23606,NULL,"[%c] OCR Alternate chars", (ocr_settings_not_look_23606.v==0 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_display,"Tells to look for an alternate character set other than the ROM default on OCR functions");
		menu_add_item_menu_ayuda(array_menu_settings_display,"Tells to look for an alternate character set other than the ROM default on OCR functions. "
							"It will look also for another character set which table is set on sysvar 23606/7. It may generate false positives "
							"on some games. It's used on text drivers (curses, stdout, simpletext) but also on OCR function");


		if (menu_display_cursesstdoutsimpletext_cond() || menu_display_aa_cond() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_textdrivers_settings,NULL,"Text driver settings");
		}

		


                menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_display,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_display);

                retorno_menu=menu_dibuja_menu(&settings_display_opcion_seleccionada,&item_seleccionado,array_menu_settings_display,"Display Settings" );

                

		//NOTA: no llamar por numero de opcion dado que hay opciones que ocultamos (relacionadas con real video)

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}




void menu_tbblue_machine_id(MENU_ITEM_PARAMETERS)
{

	menu_warn_message("Changing the machine id may show the Spectrum Next boot logo, which is NOT allowed. "
		"Please read the License file: https://gitlab.com/thesmog358/tbblue/blob/master/LICENSE.md");

        menu_item *array_menu_tbblue_hardware_id;
        menu_item item_seleccionado;
        int retorno_menu;

		menu_add_item_menu_inicial(&array_menu_tbblue_hardware_id,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

                char buffer_texto[40];

                int i;
				int salir=0;
                for (i=0;i<=255 && !salir;i++) {

					z80_byte machine_id=tbblue_machine_id_list[i].id;
					if (machine_id==255) salir=1;
					else {

                  		sprintf (buffer_texto,"%3d %s",machine_id,tbblue_machine_id_list[i].nombre);

                        menu_add_item_menu_format(array_menu_tbblue_hardware_id,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

						//Decir que no es custom 
						menu_add_item_menu_valor_opcion(array_menu_tbblue_hardware_id,0);
					}

				}

				menu_add_item_menu(array_menu_tbblue_hardware_id,"",MENU_OPCION_SEPARADOR,NULL,NULL);

				menu_add_item_menu_format(array_menu_tbblue_hardware_id,MENU_OPCION_NORMAL,NULL,NULL,"Custom");
				//Decir que es custom 
				menu_add_item_menu_valor_opcion(array_menu_tbblue_hardware_id,1);				

                menu_add_item_menu(array_menu_tbblue_hardware_id,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_tbblue_hardware_id,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_tbblue_hardware_id);

                retorno_menu=menu_dibuja_menu(&menu_tbblue_hardware_id_opcion_seleccionada,&item_seleccionado,array_menu_tbblue_hardware_id,"TBBlue machine id" );

                


				if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

					//Si se pulsa Enter
					//Detectar si es la opcion de custom
					if (item_seleccionado.valor_opcion) {
        				char string_valor[4];
						sprintf (string_valor,"%d",tbblue_machine_id);

		                menu_ventana_scanf("ID?",string_valor,4);

        				tbblue_machine_id=parse_string_to_number(string_valor);

					}

					else {
						tbblue_machine_id=tbblue_machine_id_list[menu_tbblue_hardware_id_opcion_seleccionada].id;
					}
											

												
                }

}


void menu_ext_desk_settings_enable(MENU_ITEM_PARAMETERS)
{
	
	debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	screen_ext_desktop_enabled ^=1;

        

	screen_init_pantalla_and_others();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	


}


void menu_ext_desk_settings_custom_width(MENU_ITEM_PARAMETERS)
{


	char string_width[5];

	sprintf (string_width,"%d",screen_ext_desktop_width);


	menu_ventana_scanf("Width",string_width,5);

	int valor=parse_string_to_number(string_width);

	if (valor<128 || valor>9999) {
		debug_printf (VERBOSE_ERR,"Invalid value");
		return;
	}

	screen_ext_desktop_width=valor;




	debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);
       

	screen_init_pantalla_and_others();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	


}

void menu_ext_desk_settings_width(MENU_ITEM_PARAMETERS)
{
	debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	//Cambio ancho
	//screen_ext_desktop_width *=2;
	//if (screen_ext_desktop_width>=2048) screen_ext_desktop_width=128;

	//Incrementos de 128 hasta llegar a 1024
	//Hacerlo multiple de 127 para evitar valores no multiples de custom width

	screen_ext_desktop_width &=(65535-127);


	if (screen_ext_desktop_width>=1024 && screen_ext_desktop_width<2048) screen_ext_desktop_width=2048;
	else if (screen_ext_desktop_width>=2048) screen_ext_desktop_width=128;
	else screen_ext_desktop_width +=128;
        

	screen_init_pantalla_and_others();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	
}

void menu_ext_desk_settings_filltype(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_fill++;
	if (menu_ext_desktop_fill==3) menu_ext_desktop_fill=0;
}

void menu_ext_desk_settings_fillcolor(MENU_ITEM_PARAMETERS)
{
	menu_ext_desktop_fill_solid_color++;
	if (menu_ext_desktop_fill_solid_color==16) menu_ext_desktop_fill_solid_color=0;
}

/*
int menu_ext_desktop_fill=1;
int menu_ext_desktop_fill_solid_color=1;
*/

void menu_ext_desk_settings_placemenu(MENU_ITEM_PARAMETERS)
{
	screen_ext_desktop_place_menu ^=1;	
}

void menu_ext_desktop_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ext_desktop_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_enable,NULL,"[%c] Enabled",(screen_ext_desktop_enabled ? 'X' : ' ' ) );


		if (screen_ext_desktop_enabled) {
			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_width,NULL,"[%4d] Width",screen_ext_desktop_width);
			menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Tells the width of the ZX Desktop space");
			menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Final width is this value in pixels X current horizontal zoom");

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_custom_width,NULL,"Custom Width");

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_filltype,NULL,"[%2d] Fill type",menu_ext_desktop_fill);
			if (menu_ext_desktop_fill==0) {
				menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_fillcolor,NULL,"[%2d] Fill Color",menu_ext_desktop_fill_solid_color);
			}

			menu_add_item_menu_format(array_menu_ext_desktop_settings,MENU_OPCION_NORMAL,menu_ext_desk_settings_placemenu,NULL,"[%c] Open Menu on ZX Desktop",(screen_ext_desktop_place_menu ? 'X' : ' ' ) );
			menu_add_item_menu_tooltip(array_menu_ext_desktop_settings,"Try to place new menu items on the ZX Desktop space");
			menu_add_item_menu_ayuda(array_menu_ext_desktop_settings,"Try to place new menu items on the ZX Desktop space");

		}
		

                menu_add_item_menu(array_menu_ext_desktop_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_ext_desktop_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_ext_desktop_settings);

                retorno_menu=menu_dibuja_menu(&ext_desktop_settings_opcion_seleccionada,&item_seleccionado,array_menu_ext_desktop_settings,"ZX Desktop Settings");

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



void menu_cpu_transaction_log_enable(MENU_ITEM_PARAMETERS)
{
	if (cpu_transaction_log_enabled.v) {
		reset_cpu_core_transaction_log();
	}
	else {

		if (menu_confirm_yesno_texto("May use lot of disk","Sure?")==1)
			set_cpu_core_transaction_log();
	}
}

void menu_cpu_transaction_log_truncate(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Truncate log file")) {
		transaction_log_truncate();
		menu_generic_message("Truncate log file","OK. Log truncated");
	}
}

void menu_cpu_transaction_log_file(MENU_ITEM_PARAMETERS)
{

	if (cpu_transaction_log_enabled.v) reset_cpu_core_transaction_log();

        char *filtros[2];

        filtros[0]="log";
        filtros[1]=0;


        if (menu_filesel("Select Log File",filtros,transaction_log_filename)==1) {
                //Ver si archivo existe y preguntar
		if (si_existe_archivo(transaction_log_filename)) {
                        if (menu_confirm_yesno_texto("File exists","Append?")==0) {
				transaction_log_filename[0]=0;
				return;
			}
                }

        }

	else transaction_log_filename[0]=0;

}


void menu_cpu_transaction_log_enable_address(MENU_ITEM_PARAMETERS)
{
	cpu_transaction_log_store_address.v ^=1;
}

void menu_cpu_transaction_log_enable_datetime(MENU_ITEM_PARAMETERS)
{
        cpu_transaction_log_store_datetime.v ^=1;
}


void menu_cpu_transaction_log_enable_tstates(MENU_ITEM_PARAMETERS)
{
        cpu_transaction_log_store_tstates.v ^=1;
}

void menu_cpu_transaction_log_enable_opcode(MENU_ITEM_PARAMETERS)
{
        cpu_transaction_log_store_opcode.v ^=1;
}

void menu_cpu_transaction_log_enable_registers(MENU_ITEM_PARAMETERS)
{
        cpu_transaction_log_store_registers.v ^=1;
}


void menu_cpu_transaction_log_enable_rotate(MENU_ITEM_PARAMETERS)
{
	cpu_transaction_log_rotate_enabled.v ^=1;	
}

void menu_cpu_transaction_log_rotate_number(MENU_ITEM_PARAMETERS)
{

        char string_number[4];

        sprintf (string_number,"%d",cpu_transaction_log_rotated_files);

        menu_ventana_scanf("Number of files",string_number,4);

        int numero=parse_string_to_number(string_number);

		if (transaction_log_set_rotate_number(numero)) {
			debug_printf (VERBOSE_ERR,"Invalid rotation number");			
		}


}


void menu_cpu_transaction_log_rotate_size(MENU_ITEM_PARAMETERS)
{


        char string_number[5];

        sprintf (string_number,"%d",cpu_transaction_log_rotate_size);

        menu_ventana_scanf("Size in MB (0=no rot)",string_number,5);

        int numero=parse_string_to_number(string_number);

		if (transaction_log_set_rotate_size(numero)) {
			debug_printf (VERBOSE_ERR,"Invalid rotation size");
		}


}

void menu_cpu_transaction_log_rotate_lines(MENU_ITEM_PARAMETERS)
{


        char string_number[11];

        sprintf (string_number,"%d",cpu_transaction_log_rotate_lines);

        menu_ventana_scanf("Lines (0=no rotate)",string_number,11);

        int numero=parse_string_to_number(string_number);

		if (transaction_log_set_rotate_lines(numero)) {
			debug_printf (VERBOSE_ERR,"Invalid rotation lines");
		}


}

void menu_cpu_transaction_log_ignore_rep_halt(MENU_ITEM_PARAMETERS)
{
	cpu_trans_log_ignore_repeated_halt.v ^=1;
}

void menu_cpu_transaction_log(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_cpu_transaction_log;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_transactionlog_shown[18];
                menu_tape_settings_trunc_name(transaction_log_filename,string_transactionlog_shown,18);

                menu_add_item_menu_inicial_format(&array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_file,NULL,"Log file [%s]",string_transactionlog_shown );


                if (transaction_log_filename[0]!=0) {
                        menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable,NULL,"[%c] Transaction log enabled",(cpu_transaction_log_enabled.v ? 'X' : ' ' ) );
						
						menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_rotate,NULL,"[%c] Autorotate files",(cpu_transaction_log_rotate_enabled.v ? 'X' : ' ' ) );
						menu_add_item_menu_tooltip(array_menu_cpu_transaction_log,"Enable automatic rotation of transaction log files");
						menu_add_item_menu_ayuda(array_menu_cpu_transaction_log,"Enable automatic rotation of transaction log files");
						if (cpu_transaction_log_rotate_enabled.v) {
							menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_rotate_number,NULL,"[%d] Rotate files",cpu_transaction_log_rotated_files);
							menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_rotate_size,NULL,"[%d] Rotate size (MB)",cpu_transaction_log_rotate_size);
							menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_rotate_lines,NULL,"[%d] Rotate lines",cpu_transaction_log_rotate_lines);
						}

						menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_truncate,NULL,"    Truncate log file");
                }

		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_ignore_rep_halt,NULL,"[%c] Ignore repeated halt",(cpu_trans_log_ignore_repeated_halt.v ? 'X' : ' '));


		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_datetime,NULL,"[%c] Store Date & Time",(cpu_transaction_log_store_datetime.v ? 'X' : ' '));
		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_tstates,NULL,"[%c] Store T-States",(cpu_transaction_log_store_tstates.v ? 'X' : ' '));
		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_address,NULL,"[%c] Store Address",(cpu_transaction_log_store_address.v ? 'X' : ' '));
		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_opcode,NULL,"[%c] Store Opcode",(cpu_transaction_log_store_opcode.v ? 'X' : ' '));
		menu_add_item_menu_format(array_menu_cpu_transaction_log,MENU_OPCION_NORMAL,menu_cpu_transaction_log_enable_registers,NULL,"[%c] Store Registers",(cpu_transaction_log_store_registers.v ? 'X' : ' '));
		


               menu_add_item_menu(array_menu_cpu_transaction_log,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_cpu_transaction_log);

                retorno_menu=menu_dibuja_menu(&cpu_transaction_log_opcion_seleccionada,&item_seleccionado,array_menu_cpu_transaction_log,"CPU Transaction Log" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



#define SPRITES_X ( menu_origin_x() )
#define SPRITES_Y 0
#define SPRITES_ANCHO 32
#define SPRITES_ALTO 14
#define SPRITES_ALTO_VENTANA (SPRITES_ALTO+10)

menu_z80_moto_int view_sprites_direccion=0;

//ancho en pixeles
z80_int view_sprites_ancho_sprite=8;

//alto en pixeles
z80_int view_sprites_alto_sprite=8*6;


int view_sprites_hardware=0;

z80_bit view_sprites_inverse={0};

//Incremento al moverse al siguiente byte
//Normalmente 1 , pero quiza poner a 2 para sprites que se guardan como:
//byte sprite, byte mascara, byte sprite, byte mascara
//Asi podemos saltar el byte de mascara
int view_sprite_incremento=1;

//z80_byte temp_pagina=0xF7;

//pixeles por cada byte. Puede ser 8, 4, 2 o 1
int view_sprites_ppb=8;
//en 8ppb (1 bpp), primer color rotar 7, luego 6, luego 5, .... 0 (8-(pos actual+1)*bpp). mascara 1
//en 4ppb (2 bpp), primer color rotar 6, luego 4, luego 2, luego 0 (8-(pos actual+1)*bpp). mascara 3
//en 2ppb (4 bpp), primer color rotar 4, luego 0  (8-(pos actual+1)*bpp). mascara 15
//en 1ppb (8 bpp), rotar 0 . mascara 255  (8-(pos actual+1)*bpp). mascara 255


//bits per pixel. Puede ser 1, 2, 4, 8
int view_sprites_bpp=1;


//Paletas:
//0: normal spectrum y en adelante
//1: la que esté mapeada en ulaplus
//2: la mapeada en tsconf
int view_sprites_palette=0;


//Sprite esta organizado como memoria de pantalla spectrum
int view_sprites_scr_sprite=0;

int view_sprites_offset_palette=0;


//Retorna total de colores de una paleta mapeada
int menu_debug_sprites_total_colors_mapped_palette(int paleta)
{


	switch (paleta) {

		//Speccy
		case 0:
			return 16;
		break;

		//ULAPLUS
		case 1:
			return 64;
		break;

		//Spectra
		case 2:
			return 64;
		break;

		//CPC
		case 3:
			return 16;
		break;

		//Prism zero
		case 4:
			return 256;
		break;

		//Prism two
		case 5:
			return 256;
		break;

		//Sam
		case 6:
			return 16;
		break;

		//RGB8 Tbblue
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			return 256;
		break;

		//TSConf
		case 15:
			return 256;
		break;

	}

	return 16;
}

//Retorna maximo valor para una paleta mapeada
int menu_debug_sprites_max_value_mapped_palette(int paleta)
{


	switch (paleta) {

		//Speccy
		case 0:
			return SPECCY_TOTAL_PALETTE_COLOURS;
		break;

		//ULAPLUS
		case 1:
			return ULAPLUS_TOTAL_PALETTE_COLOURS;
		break;

		//Spectra
		case 2:
			return SPECTRA_TOTAL_PALETTE_COLOURS;
		break;

		//CPC
		case 3:
			return CPC_TOTAL_PALETTE_COLOURS;
		break;

		//Prism zero
		case 4:
			return PRISM_TOTAL_PALETTE_COLOURS;
		break;

		//Prism two
		case 5:
			return PRISM_TOTAL_PALETTE_COLOURS;
		break;

		//Sam
		case 6:
			return SAM_TOTAL_PALETTE_COLOURS;
		break;

		//RGB9 Tbblue
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			return RGB9_TOTAL_PALETTE_COLOURS;
		break;

		//TSConf
		case 15:
			return TSCONF_TOTAL_PALETTE_COLOURS;
		break;

	}

	return 16;
}


//Retorna indice color asociado a la paleta indicada
int menu_debug_sprites_return_index_palette(int paleta, z80_byte color)
{


	switch (paleta) {
		case 0:
			//Speccy
			return color;
		break;

		case 1:
			//ULAPlus
			return ulaplus_palette_table[color%64];

		break;

		case 2:
			//Spectra
			return color%64;
		break;

		case 3:
			//CPC
			return cpc_palette_table[color%16];
		break;

		case 4:
			//Prism zero
			return prism_palette_zero[color%256];
		break;

		case 5:
			//Prism two
			return prism_palette_two[color%256];
		break;

		case 6:
			//Sam
			return (  (sam_palette[color&15]) & 127) ;
		break;

		case 7:
			//TBBlue ula paleta 1
			return tbblue_palette_ula_first[color];
		break;

		case 8:
			//TBBlue ula paleta 2
			return tbblue_palette_ula_second[color];
		break;			

		case 9:
			//TBBlue layer2 paleta 1
			return tbblue_palette_layer2_first[color];
		break;

		case 10:
			//TBBlue layer2 paleta 2
			return tbblue_palette_layer2_second[color];
		break;		

		case 11:
			//TBBlue sprites paleta 1
			return tbblue_palette_sprite_first[color];
		break;

		case 12:
			//TBBlue sprites paleta 2
			return tbblue_palette_sprite_second[color];
		break;

		case 13:
			//TBBlue tilemap paleta 1
			return tbblue_palette_tilemap_first[color];
		break;		

		case 14:
			//TBBlue tilemap paleta 2
			return tbblue_palette_tilemap_second[color];
		break;	

		case 15:
			//TSConf
			return tsconf_return_cram_color(color);
		break;

	}

	return color;
}

//Retorna valor de color asociado a la paleta actual mapeada
int menu_debug_sprites_return_color_palette(int paleta, z80_byte color)
{

	int index;

	index=menu_debug_sprites_return_index_palette(paleta, color);

	switch (paleta) {
		case 0:
			return index;
		break;

		case 1:
			return ULAPLUS_INDEX_FIRST_COLOR+index;
		break;

		case 2:
			return SPECTRA_INDEX_FIRST_COLOR+index;
		break;

		case 3:
			return CPC_INDEX_FIRST_COLOR+index;
		break;

		case 4:
			return PRISM_INDEX_FIRST_COLOR+index;
		break;

		case 5:
			return PRISM_INDEX_FIRST_COLOR+index;
		break;

		case 6:
			return SAM_INDEX_FIRST_COLOR+index;
		break;

		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			return RGB9_INDEX_FIRST_COLOR+(index&0x1FF);
		break;

		case 15:
			return TSCONF_INDEX_FIRST_COLOR+index;
		break;

	}

	return color;
}



void menu_debug_sprites_change_palette(void)
{
	view_sprites_palette++;
	if (view_sprites_palette==MENU_TOTAL_MAPPED_PALETTES) view_sprites_palette=0;
}

void menu_debug_sprites_get_palette_name(int paleta, char *s)
{
	switch (paleta) {
		case 0:
			strcpy(s,"Speccy");
		break;

		case 1:
			strcpy(s,"ULAPlus");
		break;

		case 2:
			strcpy(s,"Spectra");
		break;

		case 3:
			strcpy(s,"CPC");
		break;

		case 4:
			strcpy(s,"Prism Zero");
		break;

		case 5:
			strcpy(s,"Prism Two");
		break;

		case 6:
			strcpy(s,"Sam Coupe");
		break;

		case 7:
			strcpy(s,"TBBlue ULA 1");
		break;

		case 8:
			strcpy(s,"TBBlue ULA 2");
		break;	

		case 9:
			strcpy(s,"TBBlue Layer2 1");
		break;

		case 10:
			strcpy(s,"TBBlue Layer2 2");
		break;	

		case 11:
			strcpy(s,"TBBlue Sprites 1");
		break;

		case 12:
			strcpy(s,"TBBlue Sprites 2");
		break;			

		case 13:
			strcpy(s,"TBBlue Tilemap 1");
		break;

		case 14:
			strcpy(s,"TBBlue Tilemap 2");
		break;	

		case 15:
			strcpy(s,"TSConf");
		break;


		default:
			strcpy(s,"UNKNOWN");
		break;



	}
}


void menu_debug_sprites_change_bpp(void)
{
//pixeles por cada byte. Puede ser 8, 4, 2 o 1
//int view_sprites_ppb=8;
//en 8ppb (1 bpp), primer color rotar 7, luego 6, luego 5, .... 0 (8-(pos actual+1)*bpp). mascara 1
//en 4ppb (2 bpp), primer color rotar 6, luego 4, luego 2, luego 0 (8-(pos actual+1)*bpp). mascara 3
//en 2ppb (4 bpp), primer color rotar 4, luego 0  (8-(pos actual+1)*bpp). mascara 15
//en 1ppb (8 bpp), rotar 0 . mascara 255  (8-(pos actual+1)*bpp). mascara 255


//bits per pixel. Puede ser 1, 2, 4, 8
//int view_sprites_bpp=1;
	view_sprites_bpp=view_sprites_bpp <<1;
	view_sprites_ppb=view_sprites_ppb >>1;

	if (view_sprites_bpp>8) {
		view_sprites_bpp=1;
		view_sprites_ppb=8;
	}


	//printf ("bpp: %d ppb: %d\n",view_sprites_bpp,view_sprites_ppb);
}


menu_z80_moto_int menu_debug_draw_sprites_get_pointer_offset(int direccion)
{

	menu_z80_moto_int puntero;

	puntero=direccion; //por defecto

	if (view_sprites_hardware) {

		if (MACHINE_IS_TSCONF) {
			//view_sprites_direccion-> numero sprite
			struct s_tsconf_debug_sprite spriteinfo;
		
			tsconf_get_debug_sprite(view_sprites_direccion,&spriteinfo);

		        int ancho_linea=256; //512 pixeles a 4bpp
			int tnum_x=spriteinfo.tnum_x;
			int tnum_y=spriteinfo.tnum_y;

	                tnum_x *=8;
        	        tnum_y *=8;

                	//a 4bpp
	                tnum_x /=2;



        	        puntero=(tnum_y*ancho_linea)+tnum_x;
	
		}

		if (MACHINE_IS_TBBLUE) {
			puntero=view_sprites_direccion*TBBLUE_SPRITE_SIZE;
		}


	}


	return puntero;

}

zxvision_window *menu_debug_draw_sprites_window;

int view_sprites_bytes_por_linea=1;
int view_sprites_bytes_por_ventana=1;
int view_sprites_increment_cursor_vertical=1;

z80_bit view_sprites_zx81_pseudohires={0}; //Si utiliza puntero a tabla de la rom, como los usados en juegos hires de zx81 (ejemplo rocketman)

z80_byte menu_debug_draw_sprites_get_byte(menu_z80_moto_int puntero)
{

	z80_byte byte_leido;

					puntero=adjust_address_memory_size(puntero);
				byte_leido=menu_debug_get_mapped_byte(puntero);

				

				//Si hay puntero a valores en rom como algunos juegos pseudo hires de zx81
				if (view_sprites_zx81_pseudohires.v) {
					int temp_inverse=0; //si se hace inverse derivado de juegos pseudo hires de zx81
					if (byte_leido&128) temp_inverse=1;

					z80_int temp_dir=reg_i*256+(8*(byte_leido&63));
					byte_leido=peek_byte_no_time(temp_dir);

					if (temp_inverse) byte_leido ^=255;
				}
	

				if (view_sprites_inverse.v) {
					byte_leido ^=255;
				}

	return byte_leido;
}

void menu_debug_draw_sprites(void)
{

	normal_overlay_texto_menu();


	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech	


	//int sx=SPRITES_X+1;
	int sx=1;
	//int sy=SPRITES_Y+3;
	
	//Si es mas ancho, que ventana visible, mover coordenada x 1 posicion atrás
	//if (view_sprites_ancho_sprite/menu_char_width>=SPRITES_ANCHO-2) sx--;

	//Si es mas alto, mover coordenada sprite a 1, asi podemos mostrar sprites de hasta 192 de alto
	//if (view_sprites_alto_sprite>168) sy=1;

	//Si se pasa aun mas
	//if (view_sprites_alto_sprite>184) sy=0;

        int xorigen=sx*menu_char_width;
        
		int yorigen=16; //sy*8;


        int x,y,bit;
	z80_byte byte_leido;

	menu_z80_moto_int puntero;

	//puntero=view_sprites_direccion;

	//Ajustar valor puntero en caso de sprites tsconf por ejemplo
	puntero=menu_debug_draw_sprites_get_pointer_offset(view_sprites_direccion);


	int color;

	int finalx;

	menu_z80_moto_int puntero_inicio_linea;

	//int maximo_visible_x=32*menu_char_width;



		int tamanyo_linea=view_sprites_bytes_por_linea;

		for (y=0;y<view_sprites_alto_sprite;y++) {
			if (view_sprites_scr_sprite && y<192) {
				 puntero=view_sprites_direccion+screen_addr_table[(y<<5)];
			}

			puntero_inicio_linea=puntero;
			finalx=xorigen;
			for (x=0;x<view_sprites_ancho_sprite;) {
				puntero=adjust_address_memory_size(puntero);

				byte_leido=menu_debug_draw_sprites_get_byte(puntero);

				/*byte_leido=menu_debug_get_mapped_byte(puntero);

				

				//Si hay puntero a valores en rom como algunos juegos pseudo hires de zx81
				if (view_sprites_zx81_pseudohires.v) {
					int temp_inverse=0; //si se hace inverse derivado de juegos pseudo hires de zx81
					if (byte_leido&128) temp_inverse=1;

					z80_int temp_dir=reg_i*256+(8*(byte_leido&63));
					byte_leido=peek_byte_no_time(temp_dir);

					if (temp_inverse) byte_leido ^=255;
				}
	

				if (view_sprites_inverse.v) {
					byte_leido ^=255;
				}
				*/

				puntero +=view_sprite_incremento;

				int incx=0;

				for (bit=0;bit<8;bit+=view_sprites_bpp,incx++,finalx++,x++) {


				

					int dis=(8-(incx+1)*view_sprites_bpp);

					//printf ("incx: %d dis: %d\n",incx,dis);

					color=byte_leido >> dis;
					z80_byte mascara;
					switch (view_sprites_bpp) {
						case 1:
							mascara=1;
						break;

						case 2:
							mascara=3;
						break;

						case 4:
							mascara=15;
						break;

						case 8:
							mascara=255;
						break;
					}

					color=color & mascara;

				//en 8ppb (1 bpp), primer color rotar 7, luego 6, luego 5, .... 0 (8-(pos actual+1)*bpp). mascara 1
				//en 4ppb (2 bpp), primer color rotar 6, luego 4, luego 2, luego 0 (8-(pos actual+1)*bpp). mascara 3
				//en 2ppb (4 bpp), primer color rotar 4, luego 0  (8-(pos actual+1)*bpp). mascara 15
				//en 1ppb (8 bpp), rotar 0 . mascara 255  (8-(pos actual+1)*bpp). mascara 255

				//Caso 1 bpp
				if (view_sprites_bpp==1) {
                                        if ( color ==0 ) color=ESTILO_GUI_PAPEL_NORMAL;
                                        else color=ESTILO_GUI_TINTA_NORMAL;
				}
				else {
					color=menu_debug_sprites_return_color_palette(view_sprites_palette,color+view_sprites_offset_palette);

				}

	            
				zxvision_putpixel(menu_debug_draw_sprites_window,finalx,yorigen+y,color);
			   }
			}

			puntero=puntero_inicio_linea;
			puntero +=tamanyo_linea;

		}
	

	zxvision_draw_window_contents(menu_debug_draw_sprites_window);

}

menu_z80_moto_int menu_debug_view_sprites_change_pointer(menu_z80_moto_int p)
{

       //restauramos modo normal de texto de menu, porque sino, tendriamos la ventana
        //del cambio de direccion, y encima los sprites
       set_menu_overlay_function(normal_overlay_texto_menu);


        char string_address[10];



				if (view_sprites_hardware) {
					sprintf(string_address,"%d",p&63);
					menu_ventana_scanf("Sprite?",string_address,3);
				}
				else {
					util_sprintf_address_hex(p,string_address);
        	menu_ventana_scanf("Address?",string_address,10);
				}



        p=parse_string_to_number(string_address);



        set_menu_overlay_function(menu_debug_draw_sprites);


        return p;

}





//Retorna 0 si ok
int menu_debug_view_sprites_save(menu_z80_moto_int direccion,int ancho, int alto, int ppb, int incremento)
{

	char file_save[PATH_MAX];

	char *filtros[3];

						 filtros[0]="pbm";
						 filtros[1]="c";
						 filtros[2]=0;

		int ret;

		 ret=menu_filesel("Select PBM/C File",filtros,file_save);

		 if (ret==1) {

 		 		//Ver si archivo existe y preguntar
			 if (si_existe_archivo(file_save)) {

	 				if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return 0;

 				}


				char string_height[4];
				sprintf(string_height,"%d",alto);
				menu_ventana_scanf("Height?",string_height,4);
				alto=parse_string_to_number(string_height);


				//Asignar buffer temporal
				int longitud=ancho*alto;
				z80_byte *buf_temp=malloc(longitud);
				if (buf_temp==NULL) {
					debug_printf(VERBOSE_ERR,"Error allocating temporary buffer");
				}

				//Copiar de memoria emulador ahi
				int i;
				for (i=0;i<longitud;i++) {
					//buf_temp[i]=peek_byte_z80_moto(direccion);
					buf_temp[i]=menu_debug_draw_sprites_get_byte(direccion);
					direccion +=incremento;
				}


				if (!util_compare_file_extension(file_save,"pbm")) {
					util_write_pbm_file(file_save,ancho,alto,ppb,buf_temp);
				}

				else if (!util_compare_file_extension(file_save,"c")) {
					util_write_sprite_c_file(file_save,ancho,alto,ppb,buf_temp);
				}

				else {
					//debug_printf (VERBOSE_ERR,"Error. Unkown sprite file format");
					return 1;
				}

				free(buf_temp);
		}

	return 0;

}


void menu_debug_sprites_get_parameters_hardware(void)
{
	if (view_sprites_hardware) {
		if (MACHINE_IS_TBBLUE) {
			//Parametros que alteramos segun sprite activo


                	view_sprites_ancho_sprite=16;

	                view_sprites_alto_sprite=16;



			//offset paleta
			view_sprites_offset_palette=0;

			view_sprites_increment_cursor_vertical=1; //saltar de 1 en 1

                        view_sprites_bytes_por_linea=16;

			view_sprites_bytes_por_ventana=8; //saltar de 8 en 8 con pgdn/up



			view_sprites_bpp=8;
			view_sprites_ppb=1;




			//Cambiar a zona memoria 14. TBBlue sprites
			//while (menu_debug_memory_zone!=14) menu_debug_change_memory_zone_non_interactive();

			menu_debug_set_memory_zone(14);

			//paleta 11 tbblue
			//view_sprites_palette=11;


		}

		if (MACHINE_IS_TSCONF) {

			//Parametros que alteramos segun sprite activo
	                struct s_tsconf_debug_sprite spriteinfo;

        	        tsconf_get_debug_sprite(view_sprites_direccion,&spriteinfo);

                	view_sprites_ancho_sprite=spriteinfo.xs;

	                view_sprites_alto_sprite=spriteinfo.ys;

			//offset paleta
			view_sprites_offset_palette=spriteinfo.spal*16;


			view_sprites_increment_cursor_vertical=1; //saltar de 1 en 1

                        view_sprites_bytes_por_linea=256;

			view_sprites_bytes_por_ventana=8; //saltar de 8 en 8 con pgdn/up


			view_sprites_bpp=4;
			view_sprites_ppb=2;




			//Cambiar a zona memoria 15. TSConf sprites
			//while (menu_debug_memory_zone!=15) menu_debug_change_memory_zone_non_interactive();

			menu_debug_set_memory_zone(15);


			//paleta 13 tsconf
			//view_sprites_palette=13;




		}
	}

	else {
		view_sprites_bytes_por_linea=view_sprites_ancho_sprite/view_sprites_ppb;
		view_sprites_bytes_por_ventana=view_sprites_bytes_por_linea*view_sprites_alto_sprite;
		view_sprites_increment_cursor_vertical=view_sprites_bytes_por_linea;
	}
}

void menu_debug_view_sprites_textinfo(zxvision_window *ventana)
{
	int linea=0;
		char buffer_texto[64];

		//Antes de escribir, normalizar zona memoria
		menu_debug_set_memory_zone_attr();

		int restoancho=view_sprites_ancho_sprite % view_sprites_ppb;
		if (view_sprites_ancho_sprite-restoancho>0) view_sprites_ancho_sprite-=restoancho;

		//esto antes que menu_debug_sprites_get_parameters_hardware
		view_sprites_direccion=adjust_address_memory_size(view_sprites_direccion);

		menu_debug_sprites_get_parameters_hardware();



		char texto_memptr[33];

		if (view_sprites_hardware) {
			int max_sprites;
			if (MACHINE_IS_TSCONF) max_sprites=TSCONF_MAX_SPRITES;
			if (MACHINE_IS_TBBLUE) max_sprites=TBBLUE_MAX_PATTERNS;
			sprintf(texto_memptr,"Sprite: %2d",view_sprites_direccion%max_sprites); //dos digitos, tsconf hace 85 y tbblue hace 64. suficiente
		}

		else {
            char buffer_direccion[MAX_LENGTH_ADDRESS_MEMORY_ZONE+1];
            menu_debug_print_address_memory_zone(buffer_direccion,view_sprites_direccion);
			sprintf(texto_memptr,"Memptr:%s",buffer_direccion);
		}


	


		if (CPU_IS_MOTOROLA) sprintf (buffer_texto,"%s Size:%dX%d %dBPP",texto_memptr,view_sprites_ancho_sprite,view_sprites_alto_sprite,view_sprites_bpp);
		else sprintf (buffer_texto,"%s Size:%dX%d %dBPP",texto_memptr,view_sprites_ancho_sprite,view_sprites_alto_sprite,view_sprites_bpp);

		zxvision_print_string_defaults_fillspc(ventana,1,linea++,buffer_texto);



		linea=SPRITES_ALTO+3;

		char buffer_primera_linea[64]; //dar espacio de mas para poder alojar el ~de los atajos
		char buffer_segunda_linea[64];

		char buffer_tercera_linea[64];

		//Forzar a mostrar atajos
		z80_bit antes_menu_writing_inverse_color;
		antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
		menu_writing_inverse_color.v=1;

		char nombre_paleta[33];
		menu_debug_sprites_get_palette_name(view_sprites_palette,nombre_paleta);

		sprintf(buffer_tercera_linea,"Pa~~l.: %s. O~~ff:%d",nombre_paleta,view_sprites_offset_palette);


		char mensaje_texto_hardware[33];

		//por defecto
		mensaje_texto_hardware[0]=0;

		if (MACHINE_IS_TSCONF || MACHINE_IS_TBBLUE) {
			sprintf(mensaje_texto_hardware,"[%c] ~~hardware",(view_sprites_hardware ? 'X' : ' ') );
		}

		char mensaje_texto_zx81_pseudohires[33];
		//por defecto
		mensaje_texto_zx81_pseudohires[0]=0;

		if (MACHINE_IS_ZX8081) {
			sprintf(mensaje_texto_zx81_pseudohires,"[%c] Ps~~eudohires",(view_sprites_zx81_pseudohires.v ? 'X' : ' ') );
		}
		
		sprintf(buffer_primera_linea,"~~memptr In~~c+%d ~~o~~p~~q~~a:Size ~~bpp %s",
		view_sprite_incremento,
		(view_sprites_bpp==1 && !view_sprites_scr_sprite ? "~~save " : ""));

		sprintf(buffer_segunda_linea, "[%c] ~~inv [%c] Sc~~r %s%s",
					(view_sprites_inverse.v ? 'X' : ' '),
					(view_sprites_scr_sprite ? 'X' : ' '),
					mensaje_texto_hardware,mensaje_texto_zx81_pseudohires);


		zxvision_print_string_defaults_fillspc(ventana,1,linea++,buffer_primera_linea);
		zxvision_print_string_defaults_fillspc(ventana,1,linea++,buffer_segunda_linea);
		zxvision_print_string_defaults_fillspc(ventana,1,linea++,buffer_tercera_linea);

		//Mostrar zona memoria

		char textoshow[33];

		char memory_zone_text[64]; //espacio temporal mas grande por si acaso

		if (menu_debug_show_memory_zones==0) {
			sprintf (memory_zone_text,"Mem ~~zone (mapped memory)");
		}

		else {
			//printf ("Info zona %d\n",menu_debug_memory_zone);
			char buffer_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
			//int readwrite;
			machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);
			sprintf (memory_zone_text,"Mem ~~zone (%d %s)",menu_debug_memory_zone,buffer_name);
			//printf ("size: %X\n",menu_debug_memory_zone_size);
			//printf ("Despues zona %d\n",menu_debug_memory_zone);
		}

		//truncar texto a 32 por si acaso
		memory_zone_text[32]=0;


		zxvision_print_string_defaults_fillspc(ventana,1,linea++,memory_zone_text);

		sprintf (textoshow," Size: %d (%d KB)",menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);
		
		zxvision_print_string_defaults_fillspc(ventana,1,linea++,textoshow);

	

		//Restaurar comportamiento atajos
		menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


}


void menu_debug_view_sprites(MENU_ITEM_PARAMETERS)
{

    //Desactivamos interlace - si esta. Con interlace la forma de onda se dibuja encima continuamente, sin borrar
    //z80_bit copia_video_interlaced_mode;
    //copia_video_interlaced_mode.v=video_interlaced_mode.v;
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	if (!MACHINE_IS_TBBLUE & !MACHINE_IS_TSCONF) view_sprites_hardware=0;

	if (!MACHINE_IS_ZX8081) view_sprites_zx81_pseudohires.v=0;

    //disable_interlace();

	zxvision_window ventana;


	int x,y,ancho,alto;

	
    if (!util_find_window_geometry("sprites",&x,&y,&ancho,&alto)) {
        x=SPRITES_X;
        y=SPRITES_Y;
        ancho=SPRITES_ANCHO;
        alto=SPRITES_ALTO_VENTANA;
    }


	zxvision_new_window_nocheck_staticsize(&ventana,x,y,ancho,alto,64,64+2,"Sprites");

	zxvision_draw_window(&ventana);

	z80_byte tecla;


    //Cambiamos funcion overlay de texto de menu
    //Se establece a la de funcion de ver sprites
    set_menu_overlay_function(menu_debug_draw_sprites);

	menu_debug_draw_sprites_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui	

	int redibujar_texto=1;	


    do {


		//Solo redibujar el texto cuando alguna tecla pulsada sea de las validas,
		//y no con mouse moviendo la ventana
		//porque usa mucha cpu y por ejemplo en maquina tsconf se clava si arrastramos ventana
		if (redibujar_texto) {
			//printf ("redibujamos texto\n");
			menu_debug_view_sprites_textinfo(&ventana);

		
			//Si no esta multitarea, mostrar el texto que acabamos de escribir
	    	if (!menu_multitarea) {
				zxvision_draw_window_contents(&ventana);
			}			
		}

	
		tecla=zxvision_common_getkey_refresh();
		//printf ("tecla: %d\n",tecla);

		if (tecla) redibujar_texto=1;

        switch (tecla) {

					case 8:
						//izquierda
						view_sprites_direccion--;
					break;

					case 9:
						//derecha
						view_sprites_direccion++;
					break;

                    case 11:
                        //arriba
                        view_sprites_direccion -=view_sprites_increment_cursor_vertical;
                    break;

                    case 10:
                        //abajo
                        view_sprites_direccion +=view_sprites_increment_cursor_vertical;
                    break;

                    case 24:
                        //PgUp
                        view_sprites_direccion -=view_sprites_bytes_por_ventana;
                    break;

                    case 25:
                        //PgDn
                        view_sprites_direccion +=view_sprites_bytes_por_ventana;
                    break;

                    case 'm':
                        view_sprites_direccion=menu_debug_view_sprites_change_pointer(view_sprites_direccion);
							zxvision_draw_window(&ventana);
                    break;

					case 'b':
						menu_debug_sprites_change_bpp();
					break;

					case 'f':
						view_sprites_offset_palette++;
						if (view_sprites_offset_palette>=256) view_sprites_offset_palette=0;
					break;


					case 'l':
						menu_debug_sprites_change_palette();
					break;

					case 'h':
						if (MACHINE_IS_TBBLUE || MACHINE_IS_TSCONF) view_sprites_hardware ^=1;								
					break;

					case 'e':
						if (MACHINE_IS_ZX8081) view_sprites_zx81_pseudohires.v ^=1;
					break;

					case 'i':
						view_sprites_inverse.v ^=1;
					break;

					case 'z':
							//restauramos modo normal de texto de menu, sino, el selector de zona se vera
								//con el sprite encima
						set_menu_overlay_function(normal_overlay_texto_menu);

						menu_debug_change_memory_zone();

						set_menu_overlay_function(menu_debug_draw_sprites);

						break;

					case 's':


						if (view_sprites_hardware) {

						}

						else {

							//Solo graba sprites de 1bpp (monocromos)
							if (view_sprites_bpp==1 && !view_sprites_scr_sprite) {
								//restauramos modo normal de texto de menu, sino, el selector de archivos se vera
								//con el sprite encima
								set_menu_overlay_function(normal_overlay_texto_menu);


								if (menu_debug_view_sprites_save(view_sprites_direccion,view_sprites_ancho_sprite,view_sprites_alto_sprite,view_sprites_ppb,view_sprite_incremento)) {
									menu_error_message("Unknown file format");
								}

								//cls_menu_overlay();

								//menu_debug_view_sprites_ventana();
								set_menu_overlay_function(menu_debug_draw_sprites);
								zxvision_draw_window(&ventana);
							}

						}
					break;

					case 'o':
						if (view_sprites_ancho_sprite>view_sprites_ppb) view_sprites_ancho_sprite -=view_sprites_ppb;
					break;

					case 'p':

						if (view_sprites_ancho_sprite<512) view_sprites_ancho_sprite +=view_sprites_ppb;
					break;

                    case 'q':
                        if (view_sprites_alto_sprite>1) view_sprites_alto_sprite--;
                    break;

                    case 'a':
						if (view_sprites_alto_sprite<512)  view_sprites_alto_sprite++;
                    break;

					case 'c':
							if (view_sprite_incremento==1) view_sprite_incremento=2;
							else view_sprite_incremento=1;
					break;

					case 'r':
							view_sprites_scr_sprite ^=1;
					break;

					default:
						//no es tecla valida, no redibujar texto
						redibujar_texto=0;
					break;

		}


    } while (tecla!=2);

    //Restauramos modo interlace
    //if (copia_video_interlaced_mode.v) enable_interlace();

    //restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);

    cls_menu_overlay();

    //Grabar geometria ventana
    util_add_window_geometry("sprites",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);	

	zxvision_destroy_window(&ventana);		



}


int menu_breakpoints_cond(void)
{
	return debug_breakpoints_enabled.v;
}



void menu_breakpoints_conditions_set(MENU_ITEM_PARAMETERS)
{
        //printf ("linea: %d\n",breakpoints_opcion_seleccionada);

	//saltamos los breakpoints de registro pc y la primera linea
        //int breakpoint_index=breakpoints_opcion_seleccionada-MAX_BREAKPOINTS-1;

	//saltamos las primeras 2 lineas
	//int breakpoint_index=breakpoints_opcion_seleccionada-2;

	int breakpoint_index=valor_opcion;

  char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];

			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[breakpoint_index],string_texto,MAX_PARSER_TOKENS_NUM);
			
			

  menu_ventana_scanf("Condition",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_breakpoint(breakpoint_index,string_texto);

	//comprobar error
	if (if_pending_error_message) {
		menu_muestra_pending_error_message(); //Si se genera un error derivado del set breakpoint, mostrarlo y salir
		return;
	}


	sprintf (string_texto,"%s",debug_breakpoints_actions_array[breakpoint_index]);

  menu_ventana_scanf("Action? (enter=normal)",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_breakpoint_action(breakpoint_index,string_texto);

}

/*
void menu_breakpoints_condition_evaluate(MENU_ITEM_PARAMETERS)
{

        char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];
	string_texto[0]=0;

        menu_ventana_scanf("Condition",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

        int result=debug_breakpoint_condition_loop(string_texto,1);

        menu_generic_message_format("Result","%s -> %s",string_texto,(result ? "True" : "False " ));
}
*/

void menu_breakpoints_condition_evaluate_new(MENU_ITEM_PARAMETERS)
{

        char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];
	string_texto[0]=0;

        menu_ventana_scanf("Expression",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);


        //menu_generic_message_format("Result","%s -> %s",string_texto,(result ? "True" : "False " ));


	//int exp_par_evaluate_expression(char *entrada,char *salida)
	char buffer_salida[256]; //mas que suficiente
	char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

	int result=exp_par_evaluate_expression(string_texto,buffer_salida,string_detoken);
	if (result==0) {
		menu_generic_message_format("Result","Parsed string: %s\nResult: %s",string_detoken,buffer_salida);		
	}

	else if (result==1) {
		menu_error_message(buffer_salida);
	}

	else {
		menu_generic_message_format("Error","%s parsed string: %s",buffer_salida,string_detoken);
	}

	
}




void menu_breakpoints_enable_disable(MENU_ITEM_PARAMETERS)
{
        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

		breakpoints_enable();
        }


        else {
                debug_breakpoints_enabled.v=0;

		breakpoints_disable();
        }

}


void menu_breakpoints_condition_enable_disable(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_conditions_toggle(valor_opcion);

}




void menu_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_enable_disable,NULL,"~~Breakpoints: %s",
			(debug_breakpoints_enabled.v ? "On" : "Off") );
		menu_add_item_menu_shortcut(array_menu_breakpoints,'b');
		menu_add_item_menu_tooltip(array_menu_breakpoints,"Enable Breakpoints. All breakpoint types depend on this setting");
		menu_add_item_menu_ayuda(array_menu_breakpoints,"Enable Breakpoints. All breakpoint types depend on this setting");

		//char buffer_texto[40];

                int i;




		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_condition_evaluate_new,NULL,"~~Evaluate Expression");
		menu_add_item_menu_shortcut(array_menu_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_breakpoints,"Evaluate expression using parser");
		menu_add_item_menu_ayuda(array_menu_breakpoints,"Evaluate expression using parser. It's the same parser as breakpoint conditions below");

#define MAX_BRK_SHOWN_MENU 10		

		//Para mostrar los tooltip de cada linea, como contenido del breakpoint
		char buffer_temp_breakpoint_array[MAX_BRK_SHOWN_MENU][MAX_BREAKPOINT_CONDITION_LENGTH];		

		//Maximo 10 breakpoints mostrados en pantalla. Para mas, usar ZRCP
        for (i=0;i<MAX_BREAKPOINTS_CONDITIONS && i<MAX_BRK_SHOWN_MENU;i++) {
			char string_condition_shown[23];
			char string_action_shown[7];

			char string_condition_action[33];

			

			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {
			
				//nuevo parser de breakpoints
				char buffer_temp_breakpoint[MAX_BREAKPOINT_CONDITION_LENGTH];
				exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp_breakpoint,MAX_PARSER_TOKENS_NUM);

				//metemos en array para mostrar tooltip
				strcpy(buffer_temp_breakpoint_array[i],buffer_temp_breakpoint);

				menu_tape_settings_trunc_name(buffer_temp_breakpoint,string_condition_shown,23);
				
				//printf ("brkp %d [%s]\n",i,string_condition_shown);

				menu_tape_settings_trunc_name(debug_breakpoints_actions_array[i],string_action_shown,7);
				if (debug_breakpoints_actions_array[i][0]) sprintf (string_condition_action,"%s->%s",string_condition_shown,string_action_shown);

				//Si accion es menu, no escribir, para que quepa bien en pantalla
				//else sprintf (string_condition_action,"%s->menu",string_condition_shown);
				else sprintf (string_condition_action,"%s",string_condition_shown);
			}
			else {
				sprintf(string_condition_action,"None");
				//metemos en array para mostrar tooltip
				buffer_temp_breakpoint_array[i][0]=0;			
			}

			char string_condition_action_shown[23];
			menu_tape_settings_trunc_name(string_condition_action,string_condition_action_shown,23);

																																																										//0123456789012345678901234567890
			if (debug_breakpoints_conditions_enabled[i]==0 || debug_breakpoints_enabled.v==0) {														//Di 12345678901234: 12345678
				menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_conditions_set,menu_breakpoints_cond,"Di %d: %s",i+1,string_condition_action_shown);
			}
            
			else {
				menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_breakpoints_conditions_set,menu_breakpoints_cond,"En %d: %s",i+1,string_condition_action_shown);
			}

			//tooltip dice el breakpoint, si lo hay. si no, breakpoint normal
			if (buffer_temp_breakpoint_array[i][0]) {
				menu_add_item_menu_tooltip(array_menu_breakpoints,buffer_temp_breakpoint_array[i]);
			}
			else menu_add_item_menu_tooltip(array_menu_breakpoints,"Set a condition breakpoint. Press Space to disable or enable. Only 10 shown here. "
						"If you want to use more, connect to ZRCP");

			menu_add_item_menu_espacio(array_menu_breakpoints,menu_breakpoints_condition_enable_disable);

			menu_add_item_menu_valor_opcion(array_menu_breakpoints,i);

			menu_add_item_menu_ayuda(array_menu_breakpoints,"Set a condition breakpoint and its action. Press Space to disable or enable. Only 10 shown here. "
                                                "If you want to use more, connect to ZRCP.\n"
						HELP_MESSAGE_CONDITION_BREAKPOINT
						"\n\n\n"
						HELP_MESSAGE_BREAKPOINT_ACTION

					);

        }

		menu_add_item_menu(array_menu_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints,NULL,"~~Memory breakpoints");
		menu_add_item_menu_shortcut(array_menu_breakpoints,'m');


		menu_add_item_menu_format(array_menu_breakpoints,MENU_OPCION_NORMAL,menu_clear_all_breakpoints,NULL,"Clear all breakpoints");


                menu_add_item_menu(array_menu_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_breakpoints);
                retorno_menu=menu_dibuja_menu(&breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_breakpoints,"Breakpoints" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



//Retorna la pagina mapeada para el segmento



//Si se muestra ram baja de Inves
//z80_bit menu_debug_hex_shows_inves_low_ram={0};

//Vuelca contenido hexa de memoria de spectrum en cadena de texto, finalizando con 0 la cadena de texto
void menu_debug_registers_dump_hex(char *texto,menu_z80_moto_int direccion,int longitud)
{

	z80_byte byte_leido;

	int puntero=0;

	for (;longitud>0;longitud--) {
		//direccion=adjust_address_space_cpu(direccion);
		direccion=adjust_address_memory_size(direccion);

			//byte_leido=peek_byte_z80_moto(direccion);
			byte_leido=menu_debug_get_mapped_byte(direccion);
			//printf ("dump hex: %X\n",direccion);
			direccion++;
		//}

		sprintf (&texto[puntero],"%02X",byte_leido);

		puntero+=2;

	}
}


//Vuelca contenido ascii de memoria de spectrum en cadena de texto
//modoascii: 0: normal. 1:zx80. 2:zx81
void menu_debug_registers_dump_ascii(char *texto,menu_z80_moto_int direccion,int longitud,int modoascii,z80_byte valor_xor)
{

        z80_byte byte_leido;

        int puntero=0;
				//printf ("dir ascii: %d\n",direccion);

        for (;longitud>0;longitud--) {
							//direccion=adjust_address_space_cpu(direccion);
							direccion=adjust_address_memory_size(direccion);

                //Si mostramos RAM oculta de Inves
                //if (MACHINE_IS_INVES && menu_debug_hex_shows_inves_low_ram.v) {
                //        byte_leido=memoria_spectrum[direccion++];
                //}

                //else {
									//byte_leido=peek_byte_z80_moto(direccion);
									byte_leido=menu_debug_get_mapped_byte(direccion) ^ valor_xor;
									direccion++;
								//}



		if (modoascii==0) {
		if (byte_leido<32 || byte_leido>126) byte_leido='.';
		}

		else if (modoascii==1) {
			if (byte_leido>=64) byte_leido='.';
			else byte_leido=da_codigo_zx80_no_artistic(byte_leido);
		}

		else {
			if (byte_leido>=64) byte_leido='.';
                        else byte_leido=da_codigo_zx81_no_artistic(byte_leido);
                }


                sprintf (&texto[puntero],"%c",byte_leido);

                puntero+=1;

        }
}

//Retorna paginas mapeadas (nombres cortos) 
void menu_debug_get_memory_pages(char *s)
{
	debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
        int total_segmentos=debug_get_memory_pages_extended(segmentos);

        int i;
        int longitud;
        int indice=0;

        for (i=0;i<total_segmentos;i++) { 
        	longitud=strlen(segmentos[i].shortname)+1;
        	sprintf(&s[indice],"%s ",segmentos[i].shortname);

        	indice +=longitud;

        }
		
}


//Si muestra:
/*
//1=14 lineas assembler con registros a la derecha
//2=linea assembler, registros cpu, otros registros internos
//3=9 lineas assembler, otros registros internos
//4=14 lineas assembler
//5=9 lineas hexdump, otros registros internos  
//6=14 lineas hexdump   
//7=vista minima con ventana pequeña
*/
//
  
int menu_debug_registers_current_view=1;


//Ultima direccion mostrada en menu_disassemble
menu_z80_moto_int menu_debug_disassemble_last_ptr=0;

//const int menu_debug_num_lineas_full=14;

int get_menu_debug_num_lineas_full(zxvision_window *w)
{
	//return 14;

	//24->14
	int lineas=w->visible_height-10;

	if (lineas<2) lineas=2;

	return lineas;
}


void menu_debug_registers_print_register_aux_moto(zxvision_window *w,char *textoregistros,int *linea,int numero,m68k_register_t registro_direccion,m68k_register_t registro_dato)
{

	sprintf (textoregistros,"A%d: %08X D%d: %08X",numero,m68k_get_reg(NULL, registro_direccion),numero,m68k_get_reg(NULL, registro_dato) );
	//menu_escribe_linea_opcion(*linea,-1,1,textoregistros);
	zxvision_print_string_defaults_fillspc(w,1,*linea,textoregistros);
	(*linea)++;

}

z80_bit menu_debug_follow_pc={1}; //Si puntero de direccion sigue al registro pc
menu_z80_moto_int menu_debug_memory_pointer=0; //Puntero de direccion

//linea en menu debug que tiene el cursor (indicado por >), desde 0 hasta 23 como mucho
int menu_debug_line_cursor=0;

char menu_debug_change_registers_last_reg[30]="";
char menu_debug_change_registers_last_val[30]="";


void menu_debug_change_registers(void)
{
	char string_registervalue[61]; //REG=VALUE

	menu_ventana_scanf("Register?",menu_debug_change_registers_last_reg,30);

	menu_ventana_scanf("Value?",menu_debug_change_registers_last_val,30);

	sprintf (string_registervalue,"%s=%s",menu_debug_change_registers_last_reg,menu_debug_change_registers_last_val);

	if (debug_change_register(string_registervalue)) {
		debug_printf(VERBOSE_ERR,"Error changing register");
        }
}



void menu_debug_registers_change_ptr(void)
{



        char string_address[10];


                                        util_sprintf_address_hex(menu_debug_memory_pointer,string_address);
                menu_ventana_scanf("Address?",string_address,10);

        menu_debug_memory_pointer=parse_string_to_number(string_address);


        return;

}

#define MENU_DEBUG_NUMBER_FLAGS_OBJECTS 7

//Estructura para guardar la parte derecha de la vista de daad, si muestra flag o objeto y cual

struct s_debug_daad_flag_object {
	int tipo; //0=flag, 1=object
	z80_byte indice; //cual
};

struct s_debug_daad_flag_object debug_daad_flag_object[MENU_DEBUG_NUMBER_FLAGS_OBJECTS];

//inicializar la lista de flags/objetos a una por defecto, valida para daad y paws

void menu_debug_daad_init_flagobject(void)
{

	debug_daad_flag_object[0].indice=0;
	debug_daad_flag_object[1].indice=1;
	debug_daad_flag_object[2].indice=33;
	debug_daad_flag_object[3].indice=34;
	debug_daad_flag_object[4].indice=35;
	debug_daad_flag_object[5].indice=38;	
	debug_daad_flag_object[6].indice=51;	

	//todos tipo flag
	int i;
	for (i=0;i<MENU_DEBUG_NUMBER_FLAGS_OBJECTS;i++) 	debug_daad_flag_object[i].tipo=0;

			
}

//comprobamos si algun valor de la tabla se sale del rango admitido. Esto pasa en quill por ejemplo
void menu_debug_daad_check_init_flagobject(void)
{

	//todos tipo flag
	int i;
	for (i=0;i<MENU_DEBUG_NUMBER_FLAGS_OBJECTS;i++) {
		int tipo=debug_daad_flag_object[i].tipo;
		int indice=debug_daad_flag_object[i].indice;

		int limite_max;
		if (tipo==0) limite_max=util_daad_get_limit_flags();
		else limite_max=util_daad_get_limit_objects();

		if (indice>limite_max) debug_daad_flag_object[i].indice=0; //Poner un indice admitido

	}	
			
}


//Retornar el texto si es flag o objeto y valores:
//FXXX XXX o OXXX XXX
void menu_debug_daad_string_flagobject(z80_byte num_linea,char *destino)
{
	z80_byte valor;
	char letra_mostrar;

	z80_byte indice=debug_daad_flag_object[num_linea].indice;

	if (debug_daad_flag_object[num_linea].tipo==0) {
		letra_mostrar='F';
		valor=util_daad_get_flag_value(indice);
	}

	else {
		letra_mostrar='O';
		valor=util_daad_get_object_value(indice);		
	}

	sprintf (destino,"%d.%c%03d %d",num_linea+1,letra_mostrar,indice,valor);
}

                                         //Muestra el registro que le corresponde para esta linea
void menu_debug_show_register_line(int linea,char *textoregistros)
{
	char buffer_flags[32];

	//char textopaginasmem[100];

	//char textopaginasmem_linea1[100];
	//char textopaginasmem_linea2[100];

        debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
        int total_segmentos=debug_get_memory_pages_extended(segmentos);

	int offset_bloque;

	//Por defecto, cadena vacia
	textoregistros[0]=0;

	//En vista daad, mostrar flags de daad
	if (menu_debug_registers_current_view==8) {
		int linea_origen=linea;
		if (linea_origen<0 || linea_origen>MENU_DEBUG_NUMBER_FLAGS_OBJECTS) return;

		//comprobar que no haya watches fuera de rango, como en quill
		menu_debug_daad_check_init_flagobject();

		menu_debug_daad_string_flagobject(linea_origen,textoregistros);

		//sprintf (textoregistros,"F%2d %d",flag_leer,util_daad_get_flag_value(flag_leer));

		return;
	}
	
	//para mostrar vector interrupcion
	char string_vector_int[10]="     ";
	if (im_mode==2) {
	
	
	z80_int temp_i;
z80_int puntero_int;
z80_byte dir_l,dir_h;

							temp_i=reg_i*256+255;
							dir_l=peek_byte_no_time(temp_i++);
							dir_h=peek_byte_no_time(temp_i);
							puntero_int=value_8_to_16(dir_h,dir_l);
							
	sprintf(string_vector_int,"@%04X",puntero_int);
	
	}

	if (CPU_IS_Z80) {

	switch (linea) {
		case 0:
			sprintf (textoregistros,"PC %04X",get_pc_register() );
		break;

		case 1:
			sprintf (textoregistros,"SP %04X",reg_sp);
		break;

		case 2:
			sprintf (textoregistros,"AF %02X%02X'%02X%02X",reg_a,Z80_FLAGS,reg_a_shadow,Z80_FLAGS_SHADOW);
		break;

		case 3:
			sprintf (textoregistros,"%c%c%c%c%c%c%c%c",DEBUG_STRING_FLAGS);
		break;		

		case 4:
			sprintf (textoregistros,"HL %04X'%02X%02X",HL,reg_h_shadow,reg_l_shadow);
		break;

		case 5:
			sprintf (textoregistros,"DE %04X'%02X%02X",DE,reg_d_shadow,reg_e_shadow);
		break;

		case 6:
			sprintf (textoregistros,"BC %04X'%02X%02X",BC,reg_b_shadow,reg_c_shadow);
		break;

		case 7:
			sprintf (textoregistros,"IX %04X",reg_ix);
		break;

		case 8:
			sprintf (textoregistros,"IY %04X",reg_iy);
		break;

		case 9:
			sprintf (textoregistros,"IR %02X%02X%s",reg_i,(reg_r&127)|(reg_r_bit7&128) , string_vector_int);
		break;

		case 10:
			sprintf (textoregistros,"IM%d IFF%c%c",im_mode,DEBUG_STRING_IFF12 );
		break;

		/*case 12:
		case 13:
			menu_debug_get_memory_pages(textopaginasmem);
			menu_util_cut_line_at_spaces(12,textopaginasmem,textopaginasmem_linea1,textopaginasmem_linea2);
			if (linea==12) sprintf (textoregistros,"%s",textopaginasmem_linea1 );
			if (linea==13) sprintf (textoregistros,"%s",textopaginasmem_linea2 );
		break;*/

		case 11:
		case 12:
		case 13:
		case 14:
			//Por defecto, cad
			//Mostrar en una linea, dos bloques de memoria mapeadas
			offset_bloque=linea-11;
			
			offset_bloque *=2; //2 bloques por cada linea
			//primer bloque
			if (offset_bloque<total_segmentos) {
				sprintf (textoregistros,"[%s]",segmentos[offset_bloque].shortname);
				offset_bloque++;

				//Segundo bloque
				if (offset_bloque<total_segmentos) {
					int longitud=strlen(textoregistros);
					sprintf (&textoregistros[longitud],"[%s]",segmentos[offset_bloque].shortname);
				}
			}
		break;
/*
//Retorna paginas mapeadas (nombres cortos)
void menu_debug_get_memory_pages(char *s)
{

        int i;
        int longitud;
        int indice=0;

        for (i=0;i<total_segmentos;i++) {
                longitud=strlen(segmentos[i].shortname)+1;
                sprintf(&s[indice],"%s ",segmentos[i].shortname);

                indice +=longitud;

        }

}
*/

                
		

	}

	}

	if (CPU_IS_SCMP) {
	        switch (linea) {
        	        case 0:
                	        sprintf (textoregistros,"PC %04X",get_pc_register() );
	                break;

        	        case 1:
                	        sprintf (textoregistros,"AC %02X",scmp_m_AC);
	                break;

        	        case 2:
                	        sprintf (textoregistros,"ER %02X",scmp_m_ER);
	                break;

			case 3:
				sprintf (textoregistros,"SR %02X",scmp_m_SR);
			break;

			case 4:
                                scmp_get_flags_letters(scmp_m_SR,buffer_flags);
				sprintf (textoregistros,"%s",buffer_flags);
			break;

			case 5:
				sprintf (textoregistros,"P1 %04X",scmp_m_P1.w.l);
			break;

			case 6:
				sprintf (textoregistros,"P2 %04X",scmp_m_P2.w.l);
			break;

			case 7:
				sprintf (textoregistros,"P3 %04X",scmp_m_P3.w.l);
			break;

		}

	}

	if (CPU_IS_MOTOROLA) {
		switch (linea) {

			case 0:
				 sprintf (textoregistros,"PC %05X",get_pc_register() );
			break;

			case 1:
				 sprintf (textoregistros,"SP %05X",m68k_get_reg(NULL, M68K_REG_SP) );
			break;

			case 2:
				 sprintf (textoregistros,"USP %05X",m68k_get_reg(NULL, M68K_REG_USP) );
			break;

			case 3:
				 sprintf (textoregistros,"SR %04X",m68k_get_reg(NULL, M68K_REG_SR) );
			break;

			case 4:
				motorola_get_flags_string(buffer_flags);
				sprintf (textoregistros,"%s",buffer_flags );
			break;

			case 5:
				 sprintf (textoregistros,"A0 %08X",m68k_get_reg(NULL, M68K_REG_A0) );
			break;

			case 6:
				 sprintf (textoregistros,"A1 %08X",m68k_get_reg(NULL, M68K_REG_A1) );
			break;

			case 7:
				 sprintf (textoregistros,"A2 %08X",m68k_get_reg(NULL, M68K_REG_A2) );
			break;

			case 8:
				 sprintf (textoregistros,"A3 %08X",m68k_get_reg(NULL, M68K_REG_A3) );
			break;

			//No me caben tantos registros en esta vista... meto 4 de A y 4 de D

			case 9:
				sprintf (textoregistros,"D0 %08X",m68k_get_reg(NULL, M68K_REG_D0) );
                        break;

			case 10:
				sprintf (textoregistros,"D1 %08X",m68k_get_reg(NULL, M68K_REG_D1) );
                        break;

			case 11:
				sprintf (textoregistros,"D2 %08X",m68k_get_reg(NULL, M68K_REG_D2) );
                        break;

			case 12:
				sprintf (textoregistros,"D3 %08X",m68k_get_reg(NULL, M68K_REG_D3) );
                        break;

		}
	}
/*
   else if (CPU_IS_MOTOROLA) {
                             
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,0,M68K_REG_A0,M68K_REG_D0);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,1,M68K_REG_A1,M68K_REG_D1);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,2,M68K_REG_A2,M68K_REG_D2);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,3,M68K_REG_A3,M68K_REG_D3);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,4,M68K_REG_A4,M68K_REG_D4);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,5,M68K_REG_A5,M68K_REG_D5);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,6,M68K_REG_A6,M68K_REG_D6);
                                menu_debug_registers_print_register_aux_moto(textoregistros,&linea,7,M68K_REG_A7,M68K_REG_D7);

*/
}

//Longitud que ocupa el ultimo opcode desensamblado
size_t menu_debug_registers_print_registers_longitud_opcode=0;

//Ultima direccion en desemsamblado/vista hexa, para poder hacer pgup/pgdn
menu_z80_moto_int menu_debug_memory_pointer_last=0;


//Direcciones de cada linea en la vista numero 3
//menu_z80_moto_int menu_debug_lines_addresses[24];

//Numero de lineas del listado principal de la vista
int menu_debug_get_main_list_view(zxvision_window *w)
{
	int lineas=1;

    if (menu_debug_registers_current_view==3 || menu_debug_registers_current_view==5) lineas=9;
    if (menu_debug_registers_current_view==1 || menu_debug_registers_current_view==4 || menu_debug_registers_current_view==6) lineas=get_menu_debug_num_lineas_full(w);
	if (menu_debug_registers_current_view==8) lineas=get_menu_debug_num_lineas_full(w)-2;

	return lineas;
}

//Si vista actual tiene desensamblado u otros datos. En el primer de los casos, los movimientos de cursor se gestionan mediante saltos de opcodes
int menu_debug_view_has_disassemly(void)
{
	if (menu_debug_registers_current_view<=4) return 1;

	return 0;
}

menu_z80_moto_int menu_debug_disassemble_subir_veces(menu_z80_moto_int posicion,int veces)
{
        int i;
        for (i=0;i<veces;i++) {
                posicion=menu_debug_disassemble_subir(posicion);
        }
        return posicion;
}


menu_z80_moto_int menu_debug_register_decrement_half(menu_z80_moto_int posicion,zxvision_window *w)
{
	int i;
	for (i=0;i<get_menu_debug_num_lineas_full(w)/2;i++) {
		posicion=menu_debug_disassemble_subir(posicion);
	}
	return posicion;
}

 

int menu_debug_hexdump_change_pointer(int p)
{


        char string_address[10];

        sprintf (string_address,"%XH",p);


        //menu_ventana_scanf("Address? (in hex)",string_address,6);
        menu_ventana_scanf("Address?",string_address,10);

	//p=strtol(string_address, NULL, 16);
	p=parse_string_to_number(string_address);


	return p;

}


//Ajustar cuando se pulsa hacia arriba por debajo de direccion 0.
//Debe poner el puntero hacia el final de la zona de memoria
menu_z80_moto_int menu_debug_hexdump_adjusta_en_negativo(menu_z80_moto_int dir,int linesize)
{
	if (dir>=menu_debug_memory_zone_size) {
		dir=menu_debug_memory_zone_size-linesize;
	}
	//printf ("menu_debug_memory_zone_size %X\n",menu_debug_memory_zone_size);

	return dir;
}


//Si desensamblado en menu view registers muestra:
//0: lo normal. opcodes
//1: hexa
//2: ascii
int menu_debug_registers_subview_type=0;

//Modo ascii. 0 spectrum , 1 zx80, 2 zx81
int menu_debug_hexdump_with_ascii_modo_ascii=0;

void menu_debug_next_dis_show_hexa(void)
{
	menu_debug_registers_subview_type++;

	if (menu_debug_registers_subview_type==4) menu_debug_registers_subview_type=0;
}

void menu_debug_registers_adjust_ptr_on_follow(void)
{
	if (menu_debug_follow_pc.v) {
                menu_debug_memory_pointer=get_pc_register();
                //Si se esta mirando zona copper
                if (menu_debug_memory_zone==MEMORY_ZONE_NUM_TBBLUE_COPPER) {
                        menu_debug_memory_pointer=tbblue_copper_pc;
                }

        }
}


void menu_debug_registros_parte_derecha(int linea,char *buffer_linea,int columna_registros,int mostrar_separador)
{

char buffer_registros[33];
                                        if (menu_debug_registers_subview_type!=3) {

                                                //Quitar el 0 del final
                                                int longitud=strlen(buffer_linea);
                                                buffer_linea[longitud]=32;

                                                //Muestra el registro que le corresponde para esta linea
                                                menu_debug_show_register_line(linea,buffer_registros);


                                                //En QL se pega siempre el opcode con los registros. meter espacio
                                                if (CPU_IS_MOTOROLA) buffer_linea[columna_registros-1]=' ';

                                                //Agregar registro que le corresponda. Columna 19 normalmente. Con el || del separador para quitar el color seleccionado
                                                if (mostrar_separador) sprintf(&buffer_linea[columna_registros],"||%s",buffer_registros);
												else sprintf(&buffer_linea[columna_registros],"%s",buffer_registros);
                                        }
}

int menu_debug_registers_print_registers(zxvision_window *w,int linea)
{
	//printf("linea: %d\n",linea);
	char textoregistros[33];

	char dumpmemoria[33];

	char dumpassembler[65];

	//size_t longitud_opcode;

	//menu_z80_moto_int copia_reg_pc;
	int i;

	menu_z80_moto_int menu_debug_memory_pointer_copia;

	//menu_debug_registers_adjust_ptr_on_follow();


	//Conservamos valor original y usamos uno de copia
	menu_debug_memory_pointer_copia=menu_debug_memory_pointer;

	char buffer_linea[MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH];	



	//Por defecto
	menu_debug_registers_print_registers_longitud_opcode=8; //Esto se hace para que en las vistas de solo hexadecimal, se mueva arriba/abajo de 8 en 8


		if (menu_debug_registers_current_view==7) {
			menu_debug_print_address_memory_zone(dumpassembler,menu_debug_memory_pointer_copia);

			int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

			//metemos espacio en 0 final
			dumpassembler[longitud_direccion]=' ';


			//Assembler
			debugger_disassemble(&dumpassembler[longitud_direccion+1],17,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia);


			//debugger_disassemble(dumpassembler,32,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia );
                        menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia+menu_debug_registers_print_registers_longitud_opcode;

                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
			zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);

			sprintf (textoregistros,"TSTATES: %05d SCANL: %03dX%03d",t_estados,(t_estados % screen_testados_linea),t_scanline_draw);
			//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
		}


		if (menu_debug_registers_current_view==2) {

			debugger_disassemble(dumpassembler,32,&menu_debug_registers_print_registers_longitud_opcode,menu_debug_memory_pointer_copia );
			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia+menu_debug_registers_print_registers_longitud_opcode;

			//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
			zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);


			if (CPU_IS_SCMP) {
				menu_debug_registers_dump_hex(dumpmemoria,get_pc_register(),8);
	     		sprintf (textoregistros,"PC: %04X : %s",get_pc_register(),dumpmemoria);
	     		 //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P1.w.l,8);
				sprintf (textoregistros,"P1: %04X : %s",scmp_m_P1.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P2.w.l,8);
				sprintf (textoregistros,"P2: %04X : %s",scmp_m_P2.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_dump_hex(dumpmemoria,scmp_m_P3.w.l,8);
				sprintf (textoregistros,"P3: %04X : %s",scmp_m_P3.w.l,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"AC: %02X ER: %02XH",scmp_m_AC, scmp_m_ER);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				char buffer_flags[9];
				scmp_get_flags_letters(scmp_m_SR,buffer_flags);

				sprintf (textoregistros,"SR: %02X %s",scmp_m_SR,buffer_flags);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);



			}

			else if (CPU_IS_MOTOROLA) {
				sprintf (textoregistros,"PC: %05X SP: %05X USP: %05X",get_pc_register(),m68k_get_reg(NULL, M68K_REG_SP),m68k_get_reg(NULL, M68K_REG_USP));

				/*
				case M68K_REG_A7:       return cpu->dar[15];
				case M68K_REG_SP:       return cpu->dar[15];
 				case M68K_REG_USP:      return cpu->s_flag ? cpu->sp[0] : cpu->dar[15];

				SP siempre muestra A7
				USP muestra: en modo supervisor, SSP. En modo no supervisor, SP/A7
				*/

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

				char buffer_flags[32];
				motorola_get_flags_string(buffer_flags);
				sprintf (textoregistros,"SR: %04X : %s",registro_sr,buffer_flags);

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,0,M68K_REG_A0,M68K_REG_D0);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,1,M68K_REG_A1,M68K_REG_D1);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,2,M68K_REG_A2,M68K_REG_D2);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,3,M68K_REG_A3,M68K_REG_D3);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,4,M68K_REG_A4,M68K_REG_D4);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,5,M68K_REG_A5,M68K_REG_D5);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,6,M68K_REG_A6,M68K_REG_D6);
				menu_debug_registers_print_register_aux_moto(w,textoregistros,&linea,7,M68K_REG_A7,M68K_REG_D7);



			}

			else {
				//Z80
				menu_debug_registers_dump_hex(dumpmemoria,get_pc_register(),8);

				sprintf (textoregistros,"PC: %04X : %s",get_pc_register(),dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


				menu_debug_registers_dump_hex(dumpmemoria,reg_sp,8);
				sprintf (textoregistros,"SP: %04X : %s",reg_sp,dumpmemoria);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"A: %02X F: %c%c%c%c%c%c%c%c",reg_a,DEBUG_STRING_FLAGS);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"A':%02X F':%c%c%c%c%c%c%c%c",reg_a_shadow,DEBUG_STRING_FLAGS_SHADOW);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"HL: %04X DE: %04X BC: %04X",HL,DE,BC);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"HL':%04X DE':%04X BC':%04X",(reg_h_shadow<<8)|reg_l_shadow,(reg_d_shadow<<8)|reg_e_shadow,(reg_b_shadow<<8)|reg_c_shadow);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				sprintf (textoregistros,"IX: %04X IY: %04X",reg_ix,reg_iy);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

				char texto_nmi[10];
				if (MACHINE_IS_ZX81) {
					sprintf (texto_nmi,"%s",(nmi_generator_active.v ? "NMI:On" : "NMI:Off"));
				}

				else {
					texto_nmi[0]=0;
				}

				sprintf (textoregistros,"R:%02X I:%02X IM%d IFF%c%c %s",
					(reg_r&127)|(reg_r_bit7&128),
					reg_i,
					im_mode,
					DEBUG_STRING_IFF12,
				
					texto_nmi);

				//01234567890123456789012345678901
				// R: 84 I: 1E DI IM1 NMI: Off
				// R: 84 I: 1E IFF1 IFF2 IM1 NMI: Off
				// R:84 I:1E IFF1 IFF2 IM1 NMI:Off

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

			}


		}

		if (menu_debug_registers_current_view==4 || menu_debug_registers_current_view==3) {


			int longitud_op;
			

			int limite=menu_debug_get_main_list_view(w);

			for (i=0;i<limite;i++) {
				menu_debug_dissassemble_una_instruccion(dumpassembler,menu_debug_memory_pointer_copia,&longitud_op);
				//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
				zxvision_print_string_defaults_fillspc(w,1,linea++,dumpassembler);
				menu_debug_memory_pointer_copia +=longitud_op;

				//Almacenar longitud del primer opcode mostrado
				if (i==0) menu_debug_registers_print_registers_longitud_opcode=longitud_op;
			}

			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


		}



		//Linea de condact de daad
		if (menu_debug_registers_current_view==8) {

				int total_lineas_debug=7;

				size_t longitud_op;

				int i;

			

				z80_int direccion_desensamblar=value_8_to_16(reg_b,reg_c);		



				//char buffer_linea[MAX_LINE_CPU_REGISTERS_LENGTH];	


				//Si no esta en zona de parser
				if (!util_daad_is_in_parser() && !util_paws_is_in_parser() ) {
					strcpy(buffer_linea,"Not in condacts");
					//zxvision_print_string_defaults_fillspc(w,1,linea++,"Not in condacts");
				}

				else {				

					char buffer_verbo[6];
					char buffer_nombre[6];		

					z80_byte verbo=util_daad_get_flag_value(33);
					z80_byte nombre=util_daad_get_flag_value(34);

					//printf ("nombre: %d\n",nombre);

					//Por defecto
					strcpy(buffer_verbo,"_");
					strcpy(buffer_nombre,"_");

					//en quill no hay tipos de palabras. los establecemos a 0

					if (verbo!=255) util_daad_paws_locate_word(verbo,0,buffer_verbo);
					if (nombre!=255) {
						z80_byte tipo_palabra=2; 
						if (util_undaad_unpaws_is_quill() ) tipo_palabra=0;
						util_daad_paws_locate_word(nombre,tipo_palabra,buffer_nombre);
					}

					sprintf (buffer_linea,"%s %s",buffer_verbo,buffer_nombre);

					//zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);

				}

				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);

				zxvision_print_string_defaults_fillspc(w,1,linea++,"                    Watches");


/*
Para sacar el verbo + nombre de la entrada:

En el flag 33 está el código del verbo, el 34 el código del nombre. 
Si cualquiera de los dos vale 255 no buscas palabra y en su lugar pones un guion bajo (no-palabra)

Si es otro valor, en 0x8416  está la dirección donde está el vocabulario, si tomas esa direccion irás a una tabla en memoria con bloques de 7 bytes:

5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta)
1 byte para el número de palabra (el flag 33)
1 byte para el tipo de palabra (verbo=0, nombre=2)

Solo tienes que buscar en esa tabla el número de palabra de flag 33, que sea de tipo 0 , y el código del flag 34 que sea de tipo 2
*/

				//linea++;	

				//Posicion fija para la columna de watches
				int columna_watches=20;		

				int terminador=0; //Si se ha llegado a algun terminador de linea						

				for (i=0;i<total_lineas_debug;i++) {

					//Inicializamos linea a mostrar con espacios primero
					int j; 
					for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;

						//Si esta en zona de parser
						if (util_daad_is_in_parser() || util_paws_is_in_parser() ) {

							//$terminatorOpcodes = array(22, 23,103, 116,117,108);  //DONE/OK/NOTDONE/SKIP/RESTART/REDO

							int sera_terminador=0;


							//Si se llega a algun terminador
							if (!terminador) {
								z80_byte opcode=daad_peek(direccion_desensamblar);
								z80_byte opcode_res=opcode & 127;
								if (opcode_res==22 || opcode_res==23 || opcode_res==103 || opcode_res==116 || opcode_res==117 || opcode_res==108) sera_terminador=1;


								//Terminador de final y que no se mostrara
								if (opcode==0xFF) {
									//printf ("Hay terminador FF\n");
									terminador=1;
								}							
							}




							if (!terminador) {
								//Cambiamos temporalmente a zona de memoria de condacts de daad, para que desensamble como si fueran condacts
								int antes_menu_debug_memory_zone=menu_debug_memory_zone;
								if (util_daad_detect()) menu_debug_memory_zone=MEMORY_ZONE_NUM_DAAD_CONDACTS;	
								else menu_debug_memory_zone=MEMORY_ZONE_NUM_PAWS_CONDACTS;
								debugger_disassemble(dumpassembler,32,&longitud_op,direccion_desensamblar);
								menu_debug_memory_zone=antes_menu_debug_memory_zone;

								sprintf(buffer_linea,"%s",dumpassembler);

								terminador=sera_terminador;
							}

						}
						


						//printf ("primero menu_debug_registros_parte_derecha. columna_registros=%d\n",columna_registros);
						menu_debug_registros_parte_derecha(i,buffer_linea,columna_watches,0);
						//printf ("despues\n");

						//printf ("linea: %s\n",buffer_linea);

						zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);


						direccion_desensamblar +=longitud_op;

				
		}

				


		}		

        if (menu_debug_registers_current_view==1) {


				size_t longitud_op;
				int limite=get_menu_debug_num_lineas_full(w);


				//printf ("%d\n",w->visible_width);


				//A partir de que columna aparecen los registros a la derecha
				//dependera del tamaño de la ventana
				int columna_registros;


				columna_registros=w->visible_width-13;   //32-13
				if (CPU_IS_MOTOROLA) columna_registros=w->visible_width-12; //32-12


				//Revisar un minimo y maximo
				if (columna_registros<19) columna_registros=19;

				//20 caracteres dan mas que de sobra para el texto de registros
				if (columna_registros>MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-20) columna_registros=MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH-20;



				//Mi valor ptr
				menu_z80_moto_int puntero_ptr_inicial=menu_debug_memory_pointer_copia;

				//Donde empieza la vista. Subir desde direccion actual, desensamblando "hacia atras" , tantas veces como posicion cursor actual
				menu_debug_memory_pointer_copia=menu_debug_disassemble_subir_veces(puntero_ptr_inicial,menu_debug_line_cursor);
         



				//Comportamiento de 1 caracter de margen a la izquierda en ventana 
				int antes_menu_escribe_linea_startx=menu_escribe_linea_startx;

				menu_escribe_linea_startx=0;
					
				//char buffer_linea[MAX_LINE_CPU_REGISTERS_LENGTH];
                for (i=0;i<limite;i++) {

					//Por si acaso
					//buffer_registros[0]=0;

					//Inicializamos linea a mostrar primero con espacios
					
					int j; 
					for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;

					int opcion_actual=-1;

					int opcion_activada=1;  

					//Si esta linea tiene el cursor
					if (i==menu_debug_line_cursor) {
						opcion_actual=linea;			
						menu_debug_memory_pointer_copia=puntero_ptr_inicial;
						//printf ("draw line is the current. pointer=%04XH\n",menu_debug_memory_pointer_copia);
					}

					menu_z80_moto_int puntero_dir=adjust_address_memory_size(menu_debug_memory_pointer_copia);

					int tiene_brk=0;
					int tiene_pc=0;

					//Si linea tiene breakpoint
					if (debug_return_brk_pc_dir_condition(puntero_dir)>=0) tiene_brk=1;

					//Si linea es donde esta el PC
					if (puntero_dir==get_pc_register() ) tiene_pc=1;

					if (tiene_pc) buffer_linea[0]='>';
					if (tiene_brk) {
						buffer_linea[0]='*';
						opcion_activada=0;
					}

					if (tiene_pc && tiene_brk) buffer_linea[0]='+'; //Cuando coinciden breakpoint y cursor

					

                    debugger_disassemble(dumpassembler,32,&longitud_op,menu_debug_memory_pointer_copia);

/*
//Si desensamblado en menu view registers muestra:
//0: lo normal. opcodes
//1: hexa
//2: ascii
//3: lo normal pero sin mostrar registros a la derecha
int menu_debug_registers_subview_type=0;

*/
//menu_debug_memory_pointer=adjust_address_memory_size(menu_debug_memory_pointer);


					//Si mostramos en vez de desensamblado, volcado hexa o ascii
					if (menu_debug_registers_subview_type==1)	menu_debug_registers_dump_hex(dumpassembler,puntero_dir,longitud_op);
					if (menu_debug_registers_subview_type==2)  menu_debug_registers_dump_ascii(dumpassembler,puntero_dir,longitud_op,menu_debug_hexdump_with_ascii_modo_ascii,0);
					//4 para direccion, fijo
					
					sprintf(&buffer_linea[1],"%04X %s",puntero_dir,dumpassembler);

					//Guardar las direcciones de cada linea
					//menu_debug_lines_addresses[i]=puntero_dir;


					//printf ("segundo menu_debug_registros_parte_derecha. i=%d columna=%d buffer_linea: [%s]\n",i,columna_registros,buffer_linea);
					menu_debug_registros_parte_derecha(i,buffer_linea,columna_registros,1);
					//printf ("despues\n");

					//printf ("buffer_linea: [%s]\n",buffer_linea);


					//zxvision_print_string_defaults_fillspc(w,1,linea,buffer_linea);

					//De los pocos usos de menu_escribe_linea_opcion_zxvision,
					//solo se usa en menus y aqui: para poder mostrar linea activada o en rojo

					menu_escribe_linea_opcion_zxvision(w,linea,opcion_actual,opcion_activada,buffer_linea);

					//menu_escribe_linea_opcion_zxvision(w,linea,opcion_actual,opcion_activada,"0123456789001234567890012345678900123456789001234567890");

					//printf ("despues menu_escribe_linea_opcion_zxvision. i=%d\n",i);

					linea++;


					menu_debug_memory_pointer_copia +=longitud_op;

					//Almacenar longitud del primer opcode mostrado
					if (i==0) menu_debug_registers_print_registers_longitud_opcode=longitud_op;
                }


				menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


				//Vamos a ver si metemos una linea mas de la parte de la derecha extra, siempre que tenga contenido (primer caracter no espacio)
				//Esto sucede por ejemplo en tbblue, pues tiene 8 segmentos de memoria
				//Inicializamos a espacios
				int j;
				for (j=0;j<MAX_ESCR_LINEA_OPCION_ZXVISION_LENGTH;j++) buffer_linea[j]=32;


				//printf ("tercero menu_debug_registros_parte_derecha\n");
				menu_debug_registros_parte_derecha(i,buffer_linea,columna_registros,1);
				//printf ("despues\n");

				//primero borramos esa linea, por si cambiamos de subvista con M y hay "restos" ahi
				zxvision_print_string_defaults_fillspc(w,1,linea,"");

				//Si tiene contenido
				if (buffer_linea[columna_registros]!=' ' && buffer_linea[columna_registros]!=0) {
                                                //Agregamos linea perdiendo la linea en blanco de margen
						//menu_escribe_linea_opcion(linea,-1,1,buffer_linea);
						//zxvision_print_string_defaults_fillspc(w,1,linea,buffer_linea);

					//De los pocos usos de menu_escribe_linea_opcion_zxvision,
					//solo se usa en menus y dos veces en esta funcion
					//en este caso, es para poder procesar los caracteres "||"
					menu_escribe_linea_opcion_zxvision(w,linea,-1,1,buffer_linea);


				}

				linea++;

				menu_escribe_linea_startx=antes_menu_escribe_linea_startx;

			

				//Linea de stack
				//No mostrar stack en caso de scmp
				if (CPU_IS_Z80 || CPU_IS_MOTOROLA) {
					sprintf(buffer_linea,"(SP) ");

					int valores=5;
					if (CPU_IS_MOTOROLA) valores=3;
					debug_get_stack_values(valores,&buffer_linea[5]);
					//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
					zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);
				}

				//Linea de user stack
				if (CPU_IS_MOTOROLA) {
					int valores=5;
					sprintf(buffer_linea,"(USP) ");

					debug_get_user_stack_values(valores,&buffer_linea[5]);
					//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
					zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_linea);
				}

				else {
					//En caso de Z80 o SCMP meter linea vacia
					zxvision_print_string_defaults_fillspc(w,1,linea++,"");
				}


        }

		if (menu_debug_registers_current_view==5 || menu_debug_registers_current_view==6) {

			//Hacer que texto ventana empiece pegado a la izquierda
			menu_escribe_linea_startx=0;

		
			int longitud_linea=8;
			

			int limite=menu_debug_get_main_list_view(w);

			for (i=0;i<limite;i++) {
					menu_debug_hexdump_with_ascii(dumpassembler,menu_debug_memory_pointer_copia,longitud_linea,0);
					//menu_debug_registers_dump_hex(dumpassembler,menu_debug_memory_pointer_copia,longitud_linea);
					//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
					zxvision_print_string_defaults_fillspc(w,0,linea++,dumpassembler);
					menu_debug_memory_pointer_copia +=longitud_linea;
			}

			menu_debug_memory_pointer_last=menu_debug_memory_pointer_copia;


			//Restaurar comportamiento texto ventana
			menu_escribe_linea_startx=1;

		}

		//Aparecen otros registros y valores complementarios
		if (menu_debug_registers_current_view==2 || menu_debug_registers_current_view==3 || menu_debug_registers_current_view==5) {
            //Separador
        	sprintf (textoregistros," ");
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			//
			// MEMPTR y T-Estados
			//
            sprintf (textoregistros,"MEMPTR: %04X TSTATES: %05d",memptr,t_estados);
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			//
			// Mas T-Estados y parcial
			//

			char buffer_estadosparcial[32];
			/*int estadosparcial=debug_t_estados_parcial;
			

			if (estadosparcial>999999999) sprintf (buffer_estadosparcial,"%s","OVERFLOW");
			else sprintf (buffer_estadosparcial,"%09u",estadosparcial);*/

			debug_get_t_estados_parcial(buffer_estadosparcial);

            sprintf (textoregistros,"TSTATL: %03d TSTATP: %s",(t_estados % screen_testados_linea),buffer_estadosparcial );
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);

			//
			// FPS y Scanline
			//

			if (MACHINE_IS_ZX8081) {
	        	sprintf (textoregistros,"SCANLIN: %03d FPS: %03d VPS: %03d",t_scanline_draw,ultimo_fps,last_vsync_per_second);
			}
			else {
	            sprintf (textoregistros,"SCANLINE: %03d FPS: %03d",t_scanline_draw,ultimo_fps);
			}
            //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);



			//
    	    // ULA
			//

			//no hacer autodeteccion de idle bus port, para que no se active por si solo
			z80_bit copia_autodetect_rainbow;
			copia_autodetect_rainbow.v=autodetect_rainbow.v;

			autodetect_rainbow.v=0;



			//
			//Puerto FE, Idle port y flash. cada uno para la maquina que lo soporte
			//Solo para Spectrum O Z88
			//
			if (MACHINE_IS_SPECTRUM || MACHINE_IS_Z88) {
				char feporttext[20];
				if (MACHINE_IS_SPECTRUM) {
					sprintf (feporttext,"FE: %02X ",out_254_original_value);
				}
				else feporttext[0]=0;

            	char flashtext[40];
            	if (MACHINE_IS_SPECTRUM) {
	            	sprintf (flashtext,"FLASH: %d ",estado_parpadeo.v);
    	       	}

        	    else if (MACHINE_IS_Z88) {
            		sprintf (flashtext,"FLASH: %d ",estado_parpadeo.v);
            	}
	
	            else flashtext[0]=0;



				char idleporttext[20];
				if (MACHINE_IS_SPECTRUM) {
					sprintf (idleporttext,"IDLEPORT: %02X",idle_bus_port(255) );
				}
				else idleporttext[0]=0;

	            sprintf (textoregistros,"%s%s%s",feporttext,flashtext,idleporttext );

				autodetect_rainbow.v=copia_autodetect_rainbow.v;
    	        //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);


			}


			//
			// Linea audio 
			//
			if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081) {
                        sprintf (textoregistros,"AUDIO: BEEPER: %03d AY: %03d", (MACHINE_IS_ZX8081 ? da_amplitud_speaker_zx8081() :  value_beeper),da_output_ay() );
                        //menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}						






			//
			// Linea solo de Prism
			//
			if (MACHINE_IS_PRISM) {
				//SI vram aperture prism
				if (prism_ula2_registers[1] & 1) sprintf (textoregistros,"VRAM0 VRAM1 aperture");

				else {
						//       012345678901234567890123456789012
						sprintf (textoregistros,"VRAM0 SRAM10 SRAM11 not apert.");
				}

				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Cosas de Z88
			//

			if (MACHINE_IS_Z88) {
				z80_byte srunsbit=blink_com >> 6;
				sprintf (textoregistros,"SRUN: %01d SBIT: %01d SNZ: %01d COM: %01d",(srunsbit>>1)&1,srunsbit&1,z88_snooze.v,z88_coma.v);
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Copper de TBBlue
			//

			if (MACHINE_IS_TBBLUE) {
				sprintf (textoregistros,"COPPER PC: %04XH CTRL: %02XH",tbblue_copper_pc,tbblue_copper_get_control_bits() );
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}


			//
			// Video zx80/81
			//
			if (MACHINE_IS_ZX8081) {
				sprintf (textoregistros,"LNCTR: %x LCNTR %s ULAV: %s",(video_zx8081_linecntr &7),(video_zx8081_linecntr_enabled.v ? "On" : "Off"),
					(video_zx8081_ula_video_output == 0 ? "+5V" : "0V"));
				//menu_escribe_linea_opcion(linea++,-1,1,textoregistros);
				zxvision_print_string_defaults_fillspc(w,1,linea++,textoregistros);
			}



			//
    		//Paginas memoria
			//
            char textopaginasmem[100];
			menu_debug_get_memory_pages(textopaginasmem);

			int max_longitud=31;
			//limitar a 31 por si acaso

    		//Si paging enabled o no, scr
    		char buffer_paging_state[32];
    		debug_get_paging_screen_state(buffer_paging_state);

    		//Si cabe, se escribe
    		int longitud_texto1=strlen(textopaginasmem);

    		//Lo escribo y ya lo limitará debajo a 31
			sprintf(&textopaginasmem[longitud_texto1]," %s",buffer_paging_state);


			textopaginasmem[max_longitud]=0;
    		//menu_escribe_linea_opcion(linea++,-1,1,textopaginasmem);
			zxvision_print_string_defaults_fillspc(w,1,linea++,textopaginasmem);


		}




	return linea;

}

z80_bit menu_breakpoint_exception_pending_show={0};
int continuous_step=0;



int menu_debug_registers_get_height_ventana_vista(void)
{
	int alto_ventana;

        if (menu_debug_registers_current_view==7) {
                alto_ventana=5;
        }

        else if (menu_debug_registers_current_view==8) {
                alto_ventana=16;
        }		

        else {
                alto_ventana=24;
        }

	return alto_ventana;	
}

void menu_debug_registers_zxvision_ventana_set_height(zxvision_window *w)
{

	int alto_ventana=menu_debug_registers_get_height_ventana_vista();

    

	zxvision_set_visible_height(w,alto_ventana);
}

void menu_debug_registers_set_title(zxvision_window *w)
{
        char titulo[33];

	//En vista daad, meter otro titulo
	if (menu_debug_registers_current_view==8) {
		sprintf(w->window_title,"%s Debug",util_undaad_unpaws_get_parser_name() );
		return;
	}

        //menu_debug_registers_current_view

        //Por defecto
                                   //0123456789012345678901
        sprintf (titulo,"Debug CPU             V");

        if (menu_breakpoint_exception_pending_show.v==1 || menu_breakpoint_exception.v) {
                                           //0123456789012345678901
                sprintf (titulo,"Debug CPU (brk cond)  V");
                //printf ("breakpoint pending show\n");
        }
        else {
                                                                                        //0123456789012345678901
                if (cpu_step_mode.v) sprintf (titulo,"Debug CPU (step)      V");
                //printf ("no breakpoint pending show\n");
        }

        //Poner numero de vista siempre en posicion 23
        sprintf (&titulo[23],"%d",menu_debug_registers_current_view);

	strcpy(w->window_title,titulo);
}

void menu_debug_registers_ventana_common(zxvision_window *ventana)
{
	//Cambiar el alto visible segun la vista actual
	menu_debug_registers_zxvision_ventana_set_height(ventana);

	ventana->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll	
}

void menu_debug_registers_zxvision_ventana(zxvision_window *ventana)
{
/*
	

	*/

	int ancho_ventana;
	int alto_ventana;

	int xorigin,yorigin;


	if (!util_find_window_geometry("debugcpu",&xorigin,&yorigin,&ancho_ventana,&alto_ventana)) {
		xorigin=menu_origin_x();
		yorigin=0;
		ancho_ventana=32;
		alto_ventana=24;
	}


	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window_nocheck_staticsize(ventana,xorigin,yorigin,ancho_ventana,alto_ventana,ancho_ventana,alto_ventana-2,"Debug CPU");


	//Preservar ancho y alto anterior
	//menu_debug_registers_ventana_common(ventana);


	ventana->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll	



}



void menu_debug_registers_gestiona_breakpoint(void)
{
    menu_breakpoint_exception.v=0;
		menu_breakpoint_exception_pending_show.v=1;
    cpu_step_mode.v=1;

    //printf ("Reg pc: %d\n",reg_pc);
		continuous_step=0;

}

void menu_watches_daad(void)
{
		char string_line[10];
		char buffer_titulo[32];

		

		sprintf (buffer_titulo,"Line? (1-%d)",MENU_DEBUG_NUMBER_FLAGS_OBJECTS);
		string_line[0]=0;
        menu_ventana_scanf(buffer_titulo,string_line,2);
		int linea=parse_string_to_number(string_line);		
		if (linea<1 || linea>MENU_DEBUG_NUMBER_FLAGS_OBJECTS) return;
		linea--; //indice empieza en 0



        int tipo=menu_simple_two_choices("Watch type","Type","Flag","Object");
        if (tipo==0) return; //ESC	
		tipo--; //tipo empieza en 0


		string_line[0]=0;
		char ventana_titulo[33];

		char tipo_watch[10];

		int limite_max;

		if (tipo==0) {
			limite_max=util_daad_get_limit_flags();
			strcpy(tipo_watch,"Flag");
		}
		else {
			limite_max=util_daad_get_limit_objects();
			strcpy(tipo_watch,"Object");
		}

		

		sprintf (ventana_titulo,"%s? (max %d)",tipo_watch,limite_max);
		menu_ventana_scanf(ventana_titulo,string_line,4);
		int indice=parse_string_to_number(string_line);
	

		if (indice<0 || indice>limite_max) {
			menu_error_message("Out of range");
			return;
		}


		debug_daad_flag_object[linea].tipo=tipo;
		debug_daad_flag_object[linea].indice=indice;	
}



zxvision_window *menu_watches_overlay_window;

void menu_watches_overlay_mostrar_texto(void)
{
 int linea;

    linea=1; //Empezar justo en cada linea Result

    
  
				char buf_linea[32];

				//char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

				int i;

				for (i=0;i<DEBUG_MAX_WATCHES;i++) {
					
                        int error_code;

                        int resultado=exp_par_evaluate_token(debug_watches_array[i],MAX_PARSER_TOKENS_NUM,&error_code);
                        /* if (error_code) {
                                //printf ("%d\n",tokens[0].tipo);
                                menu_generic_message_format("Error","Error evaluating parsed string: %s\nResult: %d",
                                string_detoken,resultado);
                        }
                        else {
                                menu_generic_message_format("Result","Parsed string: %s\nResult: %d",
                                string_detoken,resultado);
                        }
						*/



	                sprintf (buf_linea,"  Result: %d",resultado); 
					zxvision_print_string_defaults_fillspc(menu_watches_overlay_window,1,linea,buf_linea);

					linea+=2;

								
				}

}



void menu_watches_overlay(void)
{

    normal_overlay_texto_menu();

 	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

 

		menu_watches_overlay_mostrar_texto();
		zxvision_draw_window_contents(menu_watches_overlay_window);

}



void menu_watches_edit(MENU_ITEM_PARAMETERS)
{
        int watch_index=valor_opcion;

  char string_texto[MAX_BREAKPOINT_CONDITION_LENGTH];

    exp_par_tokens_to_exp(debug_watches_array[watch_index],string_texto,MAX_PARSER_TOKENS_NUM);

  menu_ventana_scanf("Watch",string_texto,MAX_BREAKPOINT_CONDITION_LENGTH);

  debug_set_watch(watch_index,string_texto);

  menu_muestra_pending_error_message(); //Si se genera un error derivado del set watch, mostrarlo

}




void menu_watches(void)
{


       //Si es modo debug daad
       if (menu_debug_registers_current_view==8) {
        menu_watches_daad();
               return;
       }

	
	
	//Watches normales

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

    int xventana,yventana;
    int ancho_ventana,alto_ventana;	

	if (!util_find_window_geometry("watches",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {

	 xventana=menu_origin_x();
	 yventana=1;

	 ancho_ventana=32;
	 alto_ventana=22;
	}



	zxvision_window ventana;

	zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"Watches");
	zxvision_draw_window(&ventana);		



    //Cambiamos funcion overlay de texto de menu
    set_menu_overlay_function(menu_watches_overlay);

	menu_watches_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui	

    menu_item *array_menu_watches_settings;
    menu_item item_seleccionado;
    int retorno_menu;						

    do {

		//Valido tanto para cuando multitarea es off y para que nada mas entrar aqui, se vea, sin tener que esperar el medio segundo 
		//que he definido en el overlay para que aparezca
		menu_watches_overlay_mostrar_texto();

        int lin=0;

		
		
		int i;

		char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

		menu_add_item_menu_inicial(&array_menu_watches_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);
		char texto_expresion_shown[27];


		for (i=0;i<DEBUG_MAX_WATCHES;i++) {
			
			//Convertir token de watch a texto 
			if (debug_watches_array[i][0].tipo==TPT_FIN) {
				strcpy(string_detoken,"None");
			}
			else exp_par_tokens_to_exp(debug_watches_array[i],string_detoken,MAX_PARSER_TOKENS_NUM);

			//Limitar a 27 caracteres
			menu_tape_settings_trunc_name(string_detoken,texto_expresion_shown,27);

 			menu_add_item_menu_format(array_menu_watches_settings,MENU_OPCION_NORMAL,menu_watches_edit,NULL,"%2d: %s",i+1,texto_expresion_shown);

			//En que linea va
			menu_add_item_menu_tabulado(array_menu_watches_settings,1,lin);		

			//Indicamos el indice
			menu_add_item_menu_valor_opcion(array_menu_watches_settings,i);
		

			lin+=2;			
		}	
		
				

    retorno_menu=menu_dibuja_menu(&menu_watches_opcion_seleccionada,&item_seleccionado,array_menu_watches_settings,"Watches" );

	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();

				//Nombre de ventana solo aparece en el caso de stdout
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");



									//restauramos modo normal de texto de menu para llamar al editor de watch
                                                                //con el sprite encima
                                    set_menu_overlay_function(normal_overlay_texto_menu);


                                      
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

								set_menu_overlay_function(menu_watches_overlay);
								zxvision_clear_window_contents(&ventana); //limpiar de texto anterior en linea de watch
								zxvision_draw_window(&ventana);


				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);

	   cls_menu_overlay();

	util_add_window_geometry_compact("watches",&ventana);	   

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);			   


}





void menu_debug_registers_set_view(zxvision_window *ventana,int vista)
{

	zxvision_clear_window_contents(ventana);

	if (vista<1 || vista>8) vista=1;

	//Si no es daad, no permite seleccionar vista 8
	if (vista==8 && !util_daad_detect() && !util_paws_detect()) return;

	menu_debug_registers_current_view=vista;

	/*
	Dado que se cambia de vista, podemos estar en vista 7 , por ejemplo, que es pequeña, y el alto total es minimo,
	y si se cambiara a vista 1 por ejemplo, es una vista mayor pero el alto total no variaria y no se veria mas que las primeras 3 lineas
	Entonces, tenemos que destruir la ventana y volverla a crear
	 */

	

    cls_menu_overlay();

	

	int ventana_x=ventana->x;
	int ventana_y=ventana->y;
	int ventana_visible_width=ventana->visible_width;

	//El alto es el que calculamos segun la vista actual. x,y,ancho los dejamos tal cual estaban
	int ventana_visible_height=menu_debug_registers_get_height_ventana_vista();


	zxvision_destroy_window(ventana);

	//Cerrar la ventana y volverla a crear pero cambiando maximo alto

	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window(ventana,ventana_x,ventana_y,ventana_visible_width,ventana_visible_height,ventana_visible_width,ventana_visible_height-2,"Debug CPU");	

	menu_debug_registers_ventana_common(ventana);

}

void menu_debug_registers_splash_memory_zone(void)
{

	menu_debug_set_memory_zone_attr();

	char textofinal[200];
	char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
	int zone=menu_get_current_memory_zone_name_number(zone_name);
	//machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);

	sprintf (textofinal,"Zone number: %d\nName: %s\nSize: %d (%d KB)", zone,zone_name,
		menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);

	menu_generic_message_splash("Memory Zone",textofinal);


}


//Actualmente nadie usa esta funcion. Para que queremos cambiar la zona (en un menu visible) y luego hacer splash?
//antes tenia sentido pues el cambio de zona de memoria no era con menu, simplemente saltaba a la siguiente
void menu_debug_change_memory_zone_splash(void)
{
	menu_debug_change_memory_zone();

	menu_debug_registers_splash_memory_zone();


}

void menu_debug_cpu_step_over(void)
{
  //Si apunta PC a instrucciones RET o JP, hacer un cpu-step
  if (si_cpu_step_over_jpret()) {
          debug_printf(VERBOSE_DEBUG,"Running only cpu-step as current opcode is JP or RET");
	  cpu_core_loop();
          return;
  }


  debug_cpu_step_over();


}


void menu_debug_cursor_up(void)
{


		if (menu_debug_line_cursor>0) {
			menu_debug_line_cursor--;
		}

                                        if (menu_debug_view_has_disassemly() ) { //Si vista con desensamblado
                                                menu_debug_memory_pointer=menu_debug_disassemble_subir(menu_debug_memory_pointer);
                                        }
                                        else {  //Vista solo hexa
                                                menu_debug_memory_pointer -=menu_debug_registers_print_registers_longitud_opcode;
                                        }
}


void menu_debug_cursor_down(zxvision_window *w)
{
		if (menu_debug_line_cursor<get_menu_debug_num_lineas_full(w)-1) {
			menu_debug_line_cursor++;
		}

                                        if (menu_debug_view_has_disassemly() ) { //Si vista con desensamblado
                                                menu_debug_memory_pointer=menu_debug_disassemble_bajar(menu_debug_memory_pointer);
                                        }
                                        else {  //Vista solo hexa
                                                menu_debug_memory_pointer +=menu_debug_registers_print_registers_longitud_opcode;
                                        }

}




void menu_debug_cursor_pgup(zxvision_window *w)
{

                                        int lineas=menu_debug_get_main_list_view(w);


                                        int i;
                                        for (i=0;i<lineas;i++) {
						menu_debug_cursor_up();
                                        }
}


void menu_debug_cursor_pgdn(zxvision_window *w)
{

                                        int lineas=menu_debug_get_main_list_view(w);


                                        int i;
                                        for (i=0;i<lineas;i++) {
                                                menu_debug_cursor_down(w);
                                        }

}

int menu_debug_breakpoint_is_daad(char *texto)
{
	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	if (!strcasecmp(texto,breakpoint_add)) return 1;
	else return 0;
}

int menu_debug_breakpoint_is_daad_runtoparse(char *texto)
{
	char breakpoint_add[64];

	debug_get_daad_runto_parse_string(breakpoint_add);

	if (!strcasecmp(texto,breakpoint_add)) return 1;
	else return 0;
}

//Si estamos haciendo un step to step de daad
z80_bit debug_stepping_daad={0};

//Si estamos haciendo un runto parse daad
z80_bit debug_stepping_daad_runto_parse={0};

//Si hay metido un breakpoint de daad en el interprete y con registro A para el condact ficticio
z80_bit debug_allow_daad_breakpoint={0};

z80_bit debug_daad_breakpoint_runtoparse_fired={0};

void menu_breakpoint_fired(char *s) 
{
/*
//Si mostrar aviso cuando se cumple un breakpoint
int debug_show_fired_breakpoints_type=0;
//0: siempre
//1: solo cuando condicion no es tipo "PC=XXXX"
//2: nunca
*/
	int mostrar=0;

	int es_pc_cond=debug_text_is_pc_condition(s);

	//printf ("es_pc_cond: %d\n",es_pc_cond);

	if (debug_show_fired_breakpoints_type==0) mostrar=1;
	if (debug_show_fired_breakpoints_type==1 && !es_pc_cond) mostrar=1;

	if (mostrar) {
		//Si no era un breakpoint de daad de step-to-step o runtoparse

		int esta_en_parser=0;
		if (util_daad_detect() ) {
			if (reg_pc==util_daad_get_pc_parser()) esta_en_parser=1;
		}

		if (util_paws_detect()){
			if (reg_pc==util_paws_get_pc_parser()) esta_en_parser=1;
		}


		if ( (debug_stepping_daad.v || debug_stepping_daad_runto_parse.v) && esta_en_parser ) {

		}
		else menu_generic_message_format("Breakpoint","Breakpoint fired: %s",catch_breakpoint_message);
	}

	//Forzar follow pc
	menu_debug_follow_pc.v=1;



	//Si breakpoint disparado es el de daad
	if (menu_debug_breakpoint_is_daad(catch_breakpoint_message)) {
		//Accion es decrementar PC e incrementar BC
		debug_printf (VERBOSE_DEBUG,"Catch daad breakpoint. Decrementing PC and incrementing BC");
		reg_pc --;
		BC++;
	}

	//Si breakpoint disparado es el de daad runtoparse
	if (menu_debug_breakpoint_is_daad_runtoparse(catch_breakpoint_message)) {
		//Activamos un flag que se lee desde el menu debug cpu
		debug_printf (VERBOSE_DEBUG,"Catch daad breakpoint runtoparse");
		debug_daad_breakpoint_runtoparse_fired.v=1;
	}

}


void menu_debug_toggle_breakpoint(void)
{
	//Buscar primero direccion que indica el cursor
	menu_z80_moto_int direccion_cursor;

	//direccion_cursor=menu_debug_lines_addresses[menu_debug_line_cursor];
	direccion_cursor=menu_debug_memory_pointer;

	debug_printf (VERBOSE_DEBUG,"Address on cursor: %X",direccion_cursor);

	//Si hay breakpoint ahi, quitarlo
	int posicion=debug_return_brk_pc_dir_condition(direccion_cursor);
	if (posicion>=0) {
		debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
		debug_clear_breakpoint(posicion);

	}

	//Si no, ponerlo
	else {

		char condicion[30];
		sprintf (condicion,"PC=%XH",direccion_cursor);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",condicion);

		debug_add_breakpoint_free(condicion,""); 
	}
}

void menu_debug_runto(void)
{
	//Buscar primero direccion que indica el cursor
	menu_z80_moto_int direccion_cursor;

	//direccion_cursor=menu_debug_lines_addresses[menu_debug_line_cursor];
	direccion_cursor=menu_debug_memory_pointer;

	debug_printf (VERBOSE_DEBUG,"Address on cursor: %X",direccion_cursor);

	//Si no hay breakpoint ahi, ponerlo
	int posicion=debug_return_brk_pc_dir_condition(direccion_cursor);
	if (posicion<0) {

		char condicion[30];
		sprintf (condicion,"PC=%XH",direccion_cursor);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",condicion);

		debug_add_breakpoint_free(condicion,""); 
	}

	//Y salir
}




//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_step_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_step_breakpoint_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}

void menu_debug_daad_step_breakpoint(void)
{


	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_step_breakpoint();	

	char breakpoint_add[64];
	debug_get_daad_step_breakpoint_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

	debug_stepping_daad.v=1;

	//Si no hay breakpoint ahi, ponerlo
	/*int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

		debug_get_daad_step_breakpoint_string(breakpoint_add);

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;
                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}
*/
	//Y salir
}


//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_parse_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_runto_parse_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}

void menu_debug_daad_parse_breakpoint(void)
{

	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_parse_breakpoint();	

	char breakpoint_add[64];
	debug_get_daad_runto_parse_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

}

void menu_debug_daad_runto_parse(void)
{
	menu_debug_daad_parse_breakpoint();
	debug_stepping_daad_runto_parse.v=1;
}


//Quitar todas las apariciones de dicho breakpoint, por si ha quedado alguno desactivado, y al agregar uno, aparecen dos
void menu_debug_delete_daad_special_breakpoint(void)
{

	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	debug_delete_all_repeated_breakpoint(breakpoint_add);

}



void menu_debug_add_daad_special_breakpoint(void)
{

	//Antes quitamos cualquier otra aparicion
	menu_debug_delete_daad_special_breakpoint();

	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	debug_add_breakpoint_ifnot_exists(breakpoint_add);

	//Si no hay breakpoint ahi, ponerlo
	/*int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}*/

	//Y salir
}





/*void menu_debug_toggle_daad_breakpoint(void)
{
	char breakpoint_add[64];

	debug_get_daad_breakpoint_string(breakpoint_add);

	//Si no hay breakpoint ahi, ponerlo
	int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion>=0) {
		debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
		debug_clear_breakpoint(posicion);
	}

	else {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}

	//Y salir
}*/



int menu_debug_registers_show_ptr_text(zxvision_window *w,int linea)
{

	debug_printf (VERBOSE_DEBUG,"Refreshing ptr");


	char buffer_mensaje[64];
                //Forzar a mostrar atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
                menu_writing_inverse_color.v=1;


                                //Mostrar puntero direccion
                                menu_debug_memory_pointer=adjust_address_memory_size(menu_debug_memory_pointer);


				if (menu_debug_registers_current_view!=7 && menu_debug_registers_current_view!=8) {

                                char string_direccion[10];
                                menu_debug_print_address_memory_zone(string_direccion,menu_debug_memory_pointer);

				char maxima_vista='7';

				if (util_daad_detect() || util_paws_detect() ) maxima_vista='8';

                                //sprintf(buffer_mensaje,"P~~tr: %sH ~~FollowPC: %s",
								sprintf(buffer_mensaje,"P~~tr:%sH ~~FlwPC:%s ~~1-~~%c:View",
                                        string_direccion,(menu_debug_follow_pc.v ? "Yes" : "No"),maxima_vista );
                                //menu_escribe_linea_opcion(linea++,-1,1,buffer_mensaje);
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				}

	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

	return linea;
}

void menu_debug_switch_follow_pc(void)
{
	menu_debug_follow_pc.v ^=1;
	
	//if (follow_pc.v==0) menu_debug_memory_pointer=menu_debug_register_decrement_half(menu_debug_memory_pointer);
}


void menu_debug_get_legend_short_long(char *destination_string,int ancho_visible,char *short_string,char *long_string)
{

	int longitud_largo=menu_calcular_ancho_string_item(long_string);

	//Texto mas largo cuando tenemos mas ancho

	//+2 por contar 1 espacio a la izquierda y otro a la derecha
	if (ancho_visible>=longitud_largo+2) strcpy(destination_string,long_string);


	else strcpy(destination_string,short_string);	
}



void menu_debug_get_legend(int linea,char *s,zxvision_window *w)
{

	int ancho_visible=w->visible_width;

	switch (linea) {

		//Primera linea
		case 0:


			if (menu_debug_registers_current_view==8) {
							//01234567890123456789012345678901
							// chReg Brkp. Toggle Runto Watch		

				char step_condact_buffer[32];
				if (!util_daad_is_in_parser() && !util_paws_is_in_parser()) {
					strcpy(step_condact_buffer,"~~E~~n:runTo Condact");
				}
				else {
					strcpy(step_condact_buffer,"~~E~~n:Step Condact");
				}

				if (util_daad_detect()) sprintf(s,"%s [%c] Daadbr~~kpnt",step_condact_buffer,(debug_allow_daad_breakpoint.v ? 'X' : ' '));
				else sprintf(s,"%s",step_condact_buffer);
				return;
			}


			//Modo step mode
			if (cpu_step_mode.v) {
				if (menu_debug_registers_current_view==1) {

					menu_debug_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// StM DAsm En:Stp StOvr CntSt Md	
							"~~StM ~~D~~Asm ~~E~~n:Stp St~~Ovr ~~CntSt ~~Md",
								//          10        20        30        40        50        60
								//012345678901234567890123456789012345678901234567890123456789012
							//     StepMode DisAssemble Enter:Step StepOver ContinuosStep Mode

							"~~StepMode ~~Dis~~Assemble ~~E~~n~~t~~e~~r:Step Step~~Over ~~ContinousStep ~~Mode"
					
					); 

				
				}

				else {

					menu_debug_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// StpM DAsm Ent:Stp Stovr ContSt
							"~~StpM ~~D~~Asm ~~E~~n~~t:Stp St~~Ovr ~~ContSt",
								//          10        20        30        40        50        60
								//012345678901234567890123456789012345678901234567890123456789012
							//     StepMode Disassemble Enter:Step StepOver ContinuosStep 

							"~~StepMode ~~Dis~~Assemble ~~E~~nter:Step Step~~Over ~~ContinousStep"
					
					);

				}
			}

			//Modo NO step mode
			else {
				if (menu_debug_registers_current_view==1) {

					menu_debug_get_legend_short_long(s,ancho_visible,

							//01234567890123456789012345678901
							// Stepmode Disassem Assem Mode
							"~~StepMode ~~Disassem ~~Assem ~~Mode",

							//012345678901234567890123456789012345678901234567890123456789012
							// StepMode Disassemble Assemble Mode
							"~~StepMode ~~Disassemble ~~Assemble ~~Mode"
					);



				}
				else {			
							//01234567890123456789012345678901
							// Stepmode Disassemble Assemble				
					sprintf(s,"~~StepMode ~~Disassemble ~~Assemble");
				}
			}
		break;


		//Segunda linea
		case 1:


			if (menu_debug_registers_current_view==8) {
				//de momento solo el run to parse en daad. en quill o paws no tiene sentido, dado que no usan el condacto "PARSE"
				//solo se usa en psi en paws
				if (util_daad_detect()) sprintf(s,"runto~~Parse ~~Watch Wr~~ite M~~essages");
				else sprintf(s,"~~Watch Wr~~ite M~~essages");
				return;
			}

			if (menu_debug_registers_current_view==1) {

				menu_debug_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// Chrg brkp wtch Toggl Run Runto		
							  "Ch~~rg ~~brkp ~~wtch Togg~~l Ru~~n R~~unto",

							// Changeregisters breakpoints watch Toggle Run Runto	
							//012345678901234567890123456789012345678901234567890123456789012
							  "Change~~registers ~~breakpoints ~~watches Togg~~le Ru~~n R~~unto"
				);
			}

			else {

				menu_debug_get_legend_short_long(s,ancho_visible,
							//01234567890123456789012345678901
							// changeReg Breakpoints Watches					
							  "Change~~reg ~~breakpoints ~~watches",

							// Changeregisters breakpoints watches
							//012345678901234567890123456789012345678901234567890123456789012
							  "Change~~registers ~~breakpoints ~~watches"

				);

			}
		break;


		//Tercera linea
		case 2:

			if (menu_debug_registers_current_view==8) {
				if (util_daad_condact_uses_message() ) sprintf(s,"cond~~Message");
				else sprintf(s,"");
				return;
			}

			char buffer_intermedio_short[128];
			char buffer_intermedio_long[128];

			if (cpu_step_mode.v) {

							//01234567890123456789012345678901
							// ClrTstPart Write VScr MemZn 99	
				sprintf (buffer_intermedio_short,"ClrTst~~Part Wr~~ite ~~VScr Mem~~Zm %d",menu_debug_memory_zone);
							//012345678901234567890123456789012345678901234567890123456789012
							// ClearTstatesPartial Write ViewScreen MemoryZone 99	
				sprintf (buffer_intermedio_long,"ClearTstates~~Partial Wr~~ite ~~ViewScreen Memory~~Zone %d",menu_debug_memory_zone);


				menu_debug_get_legend_short_long(s,ancho_visible,buffer_intermedio_short,buffer_intermedio_long);
			}
			else {
							//01234567890123456789012345678901
							// Clrtstpart Write MemZone 99
				sprintf (buffer_intermedio_short,"ClrTst~~Part Wr~~ite Mem~~Zone %d",menu_debug_memory_zone);

							//012345678901234567890123456789012345678901234567890123456789012
							// ClearTstatesPartial Write MemoryZone 99	
				sprintf (buffer_intermedio_long,"ClearTstates~~Partial Wr~~ite Memory~~Zone %d",menu_debug_memory_zone);

				menu_debug_get_legend_short_long(s,ancho_visible,buffer_intermedio_short,buffer_intermedio_long);

			}
		break;
	}
}

//0= pausa de 0.5
//1= pausa de 0.1
//2= pausa de 0.02
//3= sin pausa
int menu_debug_continuous_speed=0;

//Posicion del indicador para dar sensacion de velocidad. De 0 a 10
int menu_debug_continuous_speed_step=0;

void menu_debug_registers_next_cont_speed(void)
{
	menu_debug_continuous_speed++;
	if (menu_debug_continuous_speed==4) menu_debug_continuous_speed=0;
}

 

//Si borra el menu a cada pulsacion y muestra la pantalla de la maquina emulada debajo
void menu_debug_registers_if_cls(void)
{
                                
	//A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
	if (debug_settings_show_screen.v) {
		cls_menu_overlay();
		menu_refresca_pantalla();

		//Y forzar en este momento a mostrar pantalla
		//scr_refresca_pantalla_solo_driver();
		//printf ("refrescando pantalla\n");
	}


	if (cpu_step_mode.v==1 || menu_multitarea==0) {
		//printf ("Esperamos tecla NO cpu loop\n");
		menu_espera_no_tecla_no_cpu_loop();
	}
	else {
		//printf ("Esperamos tecla SI cpu loop\n");
		menu_espera_no_tecla();
	}


}


void menu_debug_cont_speed_progress(char *s)
{

	int max_position=19;
	//Meter caracteres con .
	int i;
	for (i=0;i<max_position;i++) s[i]='.';
	s[i]=0;

	//Meter tantas franjas > como velocidad
	i=menu_debug_continuous_speed_step;
	int caracteres=menu_debug_continuous_speed+1;

	while (caracteres>0) {
		s[i]='>';
		i++;
		if (i==max_position) i=0; //Si se sale por la derecha
		caracteres--;
	}

	menu_debug_continuous_speed_step++;
	if (menu_debug_continuous_speed_step==max_position) menu_debug_continuous_speed_step=0; //Si se sale por la derecha
}



/*
int screen_generic_getpixel_indexcolour(z80_int *destino,int x,int y,int ancho);
*/

#define ANCHO_SCANLINE_CURSOR 32

//Buffer donde guardamos el contenido anterior del cursor de scanline, antes de meter el cursor
int menu_debug_registers_buffer_precursor[ANCHO_SCANLINE_CURSOR];
int menu_debug_registers_buffer_pre_x=-1; //posicion anterior del cursor
int menu_debug_registers_buffer_pre_y=-1;

void menu_debug_showscan_putpixel(z80_int *destino,int x,int y,int ancho,int color)
{

	screen_generic_putpixel_indexcolour(destino,x,y,ancho,color);	

}

void menu_debug_registers_show_scan_pos_putcursor(int x_inicial,int y)
{

	int ancho,alto;

	ancho=get_total_ancho_rainbow();
	alto=get_total_alto_rainbow();

    //rojo, amarillo, verde, cyan
    int colores_rainbow[]={2+8,6+8,4+8,5+8};

	int x;
    int indice_color=0;

	//printf ("inicial %d,%d\n",x_inicial,y);

	if (x_inicial<0 || y<0) return;

	//TBBlue tiene doble de alto. El ancho ya lo viene multiplicado por 2 al entrar aqui
	if (MACHINE_IS_TBBLUE) y *=2;		

	//Restauramos lo que habia en la posicion anterior del cursor
	if (menu_debug_registers_buffer_pre_x>=0 && menu_debug_registers_buffer_pre_y>=0) {
	        for (x=0;x<ANCHO_SCANLINE_CURSOR;x++) {
	            int x_final=menu_debug_registers_buffer_pre_x+x;


				if (x_final<ancho) {
					int color_antes=menu_debug_registers_buffer_precursor[x];
					menu_debug_showscan_putpixel(rainbow_buffer,x_final,menu_debug_registers_buffer_pre_y,ancho,color_antes);
				}
			}
	}



	menu_debug_registers_buffer_pre_x=x_inicial;
	menu_debug_registers_buffer_pre_y=y;


	if (x_inicial<0) return;

	for (x=0;x<ANCHO_SCANLINE_CURSOR;x++) {
		int x_final=x_inicial+x;


		//Guardamos lo que habia antes de poner el cursor
		if (x_final<ancho) {
			int color_anterior;

			//printf ("%d, %d\n",x_final,y);

			if (y>=0 && y<alto && x>=0 && x<ancho) {

				color_anterior=screen_generic_getpixel_indexcolour(rainbow_buffer,x_final,y,ancho);

				menu_debug_registers_buffer_precursor[x]=color_anterior;

				//Y ponemos pixel
			
	    		menu_debug_showscan_putpixel(rainbow_buffer,x_final,y,ancho,colores_rainbow[indice_color]);
			}
		}



		//Trozos de colores de 4 pixeles de ancho
		if (x>0 && (x%8)==0) {
			indice_color++;
			if (indice_color==4) indice_color=0;
		}


    }
}




void menu_debug_registers_show_scan_position(void)
{

	if (menu_debug_registers_if_showscan.v==0) return;

	if (rainbow_enabled.v) {
		//copiamos contenido linea y border a buffer rainbow
/*
//temp mostrar contenido buffer pixeles y atributos
printf ("pixeles y atributos:\n");
int i;
for (i=0;i<224*2/4;i++) printf ("%02X ",scanline_buffer[i]);
printf ("\n");
*/

		if (MACHINE_IS_SPECTRUM) {
			screen_store_scanline_rainbow_solo_border();
			screen_store_scanline_rainbow_solo_display();
		}

		//Obtener posicion x e y e indicar posicion visualmente

		int si_salta_linea;
		int x,y;
		x=screen_get_x_coordinate_tstates(&si_salta_linea);

		y=screen_get_y_coordinate_tstates();

		//En caso de TBBLUE, doble de ancho

		if (MACHINE_IS_TBBLUE) x*=2;

		menu_debug_registers_show_scan_pos_putcursor(x,y+si_salta_linea);

				

	}
                                
}


int menu_debug_registers_print_legend(zxvision_window *w,int linea)
{



     if (menu_debug_registers_current_view!=7) {
		char buffer_mensaje[128];

				menu_debug_get_legend(0,buffer_mensaje,w);                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				menu_debug_get_legend(1,buffer_mensaje,w);                            
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

				menu_debug_get_legend(2,buffer_mensaje,w);                                
				zxvision_print_string_defaults_fillspc(w,1,linea++,buffer_mensaje);

      }

	return linea;

}




int menu_debug_registers_get_line_legend(zxvision_window *w)
{

	if (menu_debug_registers_current_view!=8) return get_menu_debug_num_lineas_full(w)+5; //19;
	else return 11; //get_menu_debug_num_lineas_full(w)-3; //11;


}	


void menu_debug_daad_edit_flagobject(void)
{
		char string_line[10];
		char buffer_titulo[32];

		
        int tipo=menu_simple_two_choices("Watch type","Type","Flag","Object");
        if (tipo==0) return; //ESC	
		tipo--; //tipo empieza en 0
		
		if (tipo==0) strcpy (buffer_titulo,"Flag to modify?");
		else strcpy (buffer_titulo,"Object to modify?");

		string_line[0]=0;
		menu_ventana_scanf(buffer_titulo,string_line,4);
		int indice=parse_string_to_number(string_line);
		if (indice<0 || indice>255) return;

		string_line[0]=0;
		menu_ventana_scanf("Value to set?",string_line,4);
		int valor=parse_string_to_number(string_line);
		if (valor<0 || valor>255) return;		

		if (tipo==0) {
			util_daad_put_flag_value(indice,valor);
		}

		else {
			util_daad_put_object_value(indice,valor);
		}

}

//Rutina para ver diferentes mensajes de Daad, segun tipo
//0=Objects
//1=User messages
//2=System messages
//3=Locations messages
//4=Compressed messages
//5=Vocabulary
void menu_debug_daad_view_messages(MENU_ITEM_PARAMETERS)
{

	int total_messages;
	char window_title[64];
	void (*funcion_mensajes) (z80_byte index,char *texto);

	char titulo_parser[20];

	strcpy(titulo_parser,util_undaad_unpaws_get_parser_name() );
	//char *entry_message;

	switch (valor_opcion) {
		case 1:
			total_messages=util_daad_get_num_user_messages();
			funcion_mensajes=util_daad_get_user_message;
			sprintf(window_title,"%s User Messages",titulo_parser);
			//entry_message="Message";
		break;

		case 2:
			total_messages=util_daad_get_num_sys_messages();
			funcion_mensajes=util_daad_get_sys_message;
			sprintf(window_title,"%s System Messages",titulo_parser);
			//entry_message="Sys Message";
		break;		

		case 3:
			total_messages=util_daad_get_num_locat_messages();
			funcion_mensajes=util_daad_get_locat_message;
			sprintf(window_title,"%s Locations Messages",titulo_parser);
			//entry_message="Location Message";
		break;		

		case 4:
			total_messages=128;
			funcion_mensajes=util_daad_get_compressed_message;
			sprintf(window_title,"%s Compression Tokens",titulo_parser);
			//entry_message="Compressed Message";
		break;		

		case 5:
			strcpy(window_title,"Vocabulary");
		break;		

		default:
			total_messages=util_daad_get_num_objects_description();
			funcion_mensajes=util_daad_get_object_description;
			sprintf(window_title,"%s Objects",titulo_parser);
			//entry_message="Object";
		break;
	}

	int i;

	char texto[MAX_TEXTO_GENERIC_MESSAGE];
	texto[0]=0;

	int resultado=0;

	if (valor_opcion==5) { 
			if (util_daad_detect() ) util_daad_dump_vocabulary(1,texto,MAX_TEXTO_GENERIC_MESSAGE);
			else util_paws_dump_vocabulary_tostring(1,texto,MAX_TEXTO_GENERIC_MESSAGE);
	}

	else {

		for (i=0;i<total_messages && !resultado;i++) {

			char buffer_temp[256];
			funcion_mensajes(i,buffer_temp); 
			//printf ("object %d: %s\n",i,buffer_temp);

			char buffer_linea[300];
			sprintf(buffer_linea,"%03d: %s\n",i,buffer_temp);

			//Y concatenar a final
			resultado=util_concat_string(texto,buffer_linea,MAX_TEXTO_GENERIC_MESSAGE);

		}
	}

	if (resultado) menu_warn_message("Reached maximum text size. Showing only allowed text");

	menu_generic_message(window_title,texto);
}




void menu_debug_daad_view_messages_ask(void)
{

	menu_item *array_menu_daad_tipo_mensaje;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		

	    menu_add_item_menu_inicial_format(&array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Objects");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'o');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,0);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~User Messages");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'u');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,1);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~System Messages");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'s');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,2);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Locations");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'l');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,3);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Compression Tokens");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'c');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,4);

		menu_add_item_menu_format(array_menu_daad_tipo_mensaje,MENU_OPCION_NORMAL,menu_debug_daad_view_messages,NULL,"~~Vocabulary");
		menu_add_item_menu_shortcut(array_menu_daad_tipo_mensaje,'v');
		menu_add_item_menu_valor_opcion(array_menu_daad_tipo_mensaje,5);


        menu_add_item_menu(array_menu_daad_tipo_mensaje,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_daad_tipo_mensaje);

        retorno_menu=menu_dibuja_menu(&daad_tipo_mensaje_opcion_seleccionada,&item_seleccionado,array_menu_daad_tipo_mensaje,"Message type" );
                

		/*if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			menu_debug_daad_view_messages(daad_tipo_mensaje_opcion_seleccionada);

		}

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);*/




		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);	


}






//Muestra mensaje relacionado con condacto
void menu_debug_daad_get_condact_message(void)
{



	char buffer[256];
	
	util_daad_get_condact_message(buffer);
	menu_generic_message("Message",buffer);


}

void menu_debug_registers_zxvision_save_size(zxvision_window *ventana,int *ventana_ancho_antes,int *ventana_alto_antes)
{

	//Guardar ancho y alto anterior para recrear la ventana si cambia
	*ventana_ancho_antes=ventana->visible_width;
	*ventana_alto_antes=ventana->visible_height;
}
	

void menu_debug_registers(MENU_ITEM_PARAMETERS)
{

	//Si se habia lanzado un runtoparse de daad
	if (debug_daad_breakpoint_runtoparse_fired.v) {
		debug_printf (VERBOSE_DEBUG,"Going back from a daad breakpoint runtoparse. Adding a step to step condact breakpoint and exiting window");
		//Lo quitamos y metemos un breakpoint del step to step
		debug_daad_breakpoint_runtoparse_fired.v=0;
		debug_stepping_daad_runto_parse.v=0;
		menu_debug_delete_daad_parse_breakpoint();
		menu_debug_daad_step_breakpoint();
		salir_todos_menus=1;
		return;
	}


	z80_byte acumulado;

	//ninguna tecla pulsada inicialmente
	acumulado=MENU_PUERTO_TECLADO_NINGUNA;

	int linea=0;

	z80_byte tecla;

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	debug_stepping_daad.v=0;
	debug_stepping_daad_runto_parse.v=0;

	//menu_debug_registers_current_view
	//Si estabamos antes en vista 8, pero ya no hay un programa daad en memoria, resetear a vista 1
	if (menu_debug_registers_current_view==8 && !util_daad_detect() && !util_paws_detect() ) {
		menu_debug_registers_current_view=1;
	}




	//Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();


	//Ver si hemos entrado desde un breakpoint
	if (menu_breakpoint_exception.v) menu_debug_registers_gestiona_breakpoint();

	else menu_espera_no_tecla();


	char buffer_mensaje[64];

	//Si no esta multitarea activa, modo por defecto es step to step
	if (menu_multitarea==0) cpu_step_mode.v=1;


	zxvision_window ventana;
	menu_debug_registers_zxvision_ventana(&ventana);


	//Guardar ancho y alto anterior para recrear la ventana si cambia
	int ventana_ancho_antes;
	int ventana_alto_antes;

	menu_debug_registers_zxvision_save_size(&ventana,&ventana_ancho_antes,&ventana_alto_antes);

	menu_debug_registers_set_title(&ventana);


	do {

		//Ver si ha cambiado tamanyo ventana para recrearla
		//Si no gestionamos esto, zxvision redimensiona por su cuenta el tamaño visible pero el tamaño total
		//requiere logicamente recrearla de nuevo
		if (ventana.visible_width!=ventana_ancho_antes || ventana.visible_height!=ventana_alto_antes) {
			debug_printf (VERBOSE_DEBUG,"Debug CPU window size has changed. Recreate it again");

			//Guardamos la geometria actual
			util_add_window_geometry("debugcpu",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);

			zxvision_destroy_window(&ventana);

			//Al crearla, usara la geometria guardada antes
			menu_debug_registers_zxvision_ventana(&ventana);
			menu_debug_registers_set_title(&ventana);

			//Guardamos valores en variables para saber si cambia
			menu_debug_registers_zxvision_save_size(&ventana,&ventana_ancho_antes,&ventana_alto_antes);
		}


		//Si es la vista 8, siempre esta en cpu step mode, y zona de memoria es la mapped
		if (menu_debug_registers_current_view==8) {
			cpu_step_mode.v=1;
			menu_debug_set_memory_zone_mapped();
		}

		//
		//Si no esta el modo step de la cpu
		//
		if (cpu_step_mode.v==0) {


			//Cuadrarlo cada 1/16 de segundo, justo lo mismo que el flash, asi
			//el valor de flash se ve coordinado
        	        //if ( (contador_segundo%(16*20)) == 0 || menu_multitarea==0) {
			if ( ((contador_segundo%(16*20)) == 0 && valor_contador_segundo_anterior!=contador_segundo ) || menu_multitarea==0) {
				//printf ("Refresco pantalla. contador_segundo=%d\n",contador_segundo);
				valor_contador_segundo_anterior=contador_segundo;


				menu_debug_registers_set_title(&ventana);
				zxvision_draw_window(&ventana);

				menu_debug_registers_adjust_ptr_on_follow();

                linea=0;
                linea=menu_debug_registers_show_ptr_text(&ventana,linea);

                linea++;


                //Forzar a mostrar atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
                menu_writing_inverse_color.v=1;

                        
				linea=menu_debug_registers_print_registers(&ventana,linea);
				//linea=19;


				//En que linea aparece la leyenda
				linea=menu_debug_registers_get_line_legend(&ventana);
				linea=menu_debug_registers_print_legend(&ventana,linea);


				//Restaurar estado mostrar atajos
				menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;


				zxvision_draw_window_contents(&ventana);

                if (menu_multitarea==0) menu_refresca_pantalla();


	        }

        	menu_cpu_core_loop();

			if (menu_breakpoint_exception.v) {
				//Si accion nula o menu o break
				if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
				  menu_debug_registers_gestiona_breakpoint();
				  //Y redibujar ventana para reflejar breakpoint cond
				  //menu_debug_registers_ventana();
				}

				else {
					menu_breakpoint_exception.v=0;
					//Gestion acciones
					debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
				}
			}



            acumulado=menu_da_todas_teclas();

	    	//si no hay multitarea, esperar tecla y salir
        	if (menu_multitarea==0) {
            	menu_espera_tecla();
               	acumulado=0;
	        }

			//Hay tecla pulsada
			if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
				//tecla=zxvision_common_getkey_refresh();
				tecla=zxvision_common_getkey_refresh_noesperanotec();

            	//Aqui suele llegar al mover raton-> se produce un evento pero no se pulsa tecla
                if (tecla==0) {
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

                else {
                    //printf ("tecla: %d\n",tecla);
                    //A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
                    menu_debug_registers_if_cls();
                    //menu_espera_no_tecla_no_cpu_loop();
                }




                if (tecla=='s') {
					cpu_step_mode.v=1;
					menu_debug_follow_pc.v=1; //se sigue pc
				}

				if (tecla=='z') {
					menu_debug_change_memory_zone();
				}


				if (tecla=='d') {
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_disassemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}


				if (tecla=='a') {
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_assemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}				

				if (tecla=='b') {
					menu_breakpoints(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='m' && menu_debug_registers_current_view==1) {
                    menu_debug_next_dis_show_hexa();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				if (tecla=='l' && menu_debug_registers_current_view==1) {
                    menu_debug_toggle_breakpoint();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='u' && menu_debug_registers_current_view==1) {
					menu_debug_runto();
                    tecla=2; //Simular ESC
					salir_todos_menus=1;
                }			

				if (tecla=='n' && menu_debug_registers_current_view==1) {
					//run tal cual. como runto pero sin poner breakpoint
                    tecla=2; //Simular ESC
					salir_todos_menus=1;
                }										

				if (tecla=='w') {
                    menu_watches();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

								
								
				if (tecla=='i') {
					last_debug_poke_dir=menu_debug_memory_pointer;
					if (menu_debug_registers_current_view==8) {
						menu_debug_daad_edit_flagobject();
					}
                    else menu_debug_poke(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }								

                if (tecla=='p') {
					if (menu_debug_registers_current_view==8) {
						//Esto es run hasta Parse Daad
						menu_debug_daad_runto_parse();
                    	tecla=2; //Simular ESC
						salir_todos_menus=1;						
					}
					else {
						debug_t_estados_parcial=0;
                    	//Decimos que no hay tecla pulsada
                    	acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					}
                }

				//Vista. Entre 1 y 6
				if (tecla>='1' && tecla<='8') {
					menu_debug_registers_set_view(&ventana,tecla-'0');
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='f') {
					menu_debug_switch_follow_pc();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				if (tecla=='t') {
					menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_registers_change_ptr();
					//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

               	if (tecla=='r') {
					menu_debug_change_registers();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
            	}

				if (tecla==11) {
                    //arriba
					menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_up();
					//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				if (tecla==10) {
                    //abajo
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_down(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				//24 pgup
                if (tecla==24) {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_pgup(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                }

				//25 pgwn
				if (tecla==25) {
					//PgDn
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					menu_debug_cursor_pgdn(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
				}

				//Si tecla no es ESC, no salir
				if (tecla!=2) acumulado=MENU_PUERTO_TECLADO_NINGUNA;



			}

		}


		//
		//En modo Step mode
		//
		else {

			menu_debug_registers_set_title(&ventana);
			zxvision_draw_window(&ventana);

			menu_breakpoint_exception_pending_show.v=0;

			menu_debug_registers_adjust_ptr_on_follow();
	
   	        linea=0;
	        linea=menu_debug_registers_show_ptr_text(&ventana,linea);

        	linea++;


			int si_ejecuta_una_instruccion=1;

            linea=menu_debug_registers_print_registers(&ventana,linea);

			//linea=19;
			linea=menu_debug_registers_get_line_legend(&ventana);

        	//Forzar a mostrar atajos
	        z80_bit antes_menu_writing_inverse_color;
	        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        	menu_writing_inverse_color.v=1;



			if (continuous_step==0) {
								//      01234567890123456789012345678901
				linea=menu_debug_registers_print_legend(&ventana,linea);
																	// ~~1-~~5 View
			}
			else {
				//Mostrar progreso

				if (menu_debug_registers_current_view!=7) {
					char buffer_progreso[32];
					menu_debug_cont_speed_progress(buffer_progreso);
					sprintf (buffer_mensaje,"~~C: Speed %d %s",menu_debug_continuous_speed,buffer_progreso);
					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_mensaje);

					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Any other key: Stop cont step");
													  //0123456789012345678901234567890

					//si lento, avisar
					if (menu_debug_continuous_speed<=1) {
						zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Note: Make long key presses");
					}
					else {
						zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"                         ");
					}

				}


				//Pausa
				//0= pausa de 0.5
				//1= pausa de 0.1
				//2= pausa de 0.02
				//3= sin pausa

				if (menu_debug_continuous_speed==0) usleep(500000); //0.5 segundo
				else if (menu_debug_continuous_speed==1) usleep(100000); //0.1 segundo
				else if (menu_debug_continuous_speed==2) usleep(20000); //0.02 segundo
			}


			//Restaurar estado mostrar atajos
			menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

			//Actualizamos pantalla
			//zxvision_draw_window(&ventana);
			zxvision_draw_window_contents(&ventana);
			menu_refresca_pantalla();


			//Esperamos tecla
			if (continuous_step==0)
			{ 
				menu_espera_tecla_no_cpu_loop();
					
				//No quiero que se llame a core loop si multitarea esta activo pero aqui estamos en cpu step
				int antes_menu_multitarea=menu_multitarea;
				menu_multitarea=0;
				//tecla=zxvision_common_getkey_refresh();
				tecla=zxvision_common_getkey_refresh_noesperanotec();
				menu_multitarea=antes_menu_multitarea;

				//Aqui suele llegar al mover raton-> se produce un evento pero no se pulsa tecla
				if (tecla==0) {
					acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;
				}

				else {
					//printf ("tecla: %d\n",tecla);

					//A cada pulsacion de tecla, mostramos la pantalla del ordenador emulado
					menu_debug_registers_if_cls();
					//menu_espera_no_tecla_no_cpu_loop();
				}


				if (tecla=='c') {
					continuous_step=1;
				}

                if (tecla=='o') {
                    menu_debug_cpu_step_over();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }

				if (tecla=='d') {
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_disassemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}


				if (tecla=='a') {
					menu_debug_disassemble_last_ptr=menu_debug_memory_pointer;
					menu_debug_assemble(0);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}		


				if (tecla=='z') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_debug_change_memory_zone();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;

				}								

                if (tecla=='b') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_breakpoints(0);

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;


                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;
					
                }

                if (tecla=='w') {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_watches();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;
                }


                if (tecla=='i') {
                	//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;
				
					last_debug_poke_dir=menu_debug_memory_pointer;
					if (menu_debug_registers_current_view==8) {
						menu_debug_daad_edit_flagobject();
					}
                    else menu_debug_poke(0);					

                	//Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;
                }


				if (tecla=='m' && menu_debug_registers_current_view==1) {
		            menu_debug_next_dis_show_hexa();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }


				//Mensaje al que apunta instruccion de condact
				if (tecla=='m' && menu_debug_registers_current_view==8 && util_daad_condact_uses_message() ) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_debug_daad_get_condact_message();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;

                }

				//Lista de todos mensajes
				if (tecla=='e' && menu_debug_registers_current_view==8) {
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_debug_daad_view_messages_ask();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;

                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;

                }				

		        if (tecla=='l' && menu_debug_registers_current_view==1) {
                    menu_debug_toggle_breakpoint();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }
								
				if (tecla=='u' && menu_debug_registers_current_view==1) {
                    menu_debug_runto();
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
					salir_todos_menus=1;
					cpu_step_mode.v=0;
					acumulado=0; //teclas pulsadas
					//Con esto saldremos
                }	

				
				if (tecla=='n' && menu_debug_registers_current_view==1) {
					//run tal cual. como runto pero sin poner breakpoint
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
					salir_todos_menus=1;
					cpu_step_mode.v=0;
					acumulado=0; //teclas pulsadas
					//Con esto saldremos
                }					


                if (tecla=='p') {
					if (menu_debug_registers_current_view==8) {
                    	menu_debug_daad_runto_parse();
                    	//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    	si_ejecuta_una_instruccion=0;
						salir_todos_menus=1;
						cpu_step_mode.v=0;
						acumulado=0; //teclas pulsadas
						//Con esto saldremos						
					}
					else {
						debug_t_estados_parcial=0;
                    	//Decimos que no hay tecla pulsada
                    	acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    	//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    	si_ejecuta_una_instruccion=0;
					}
                }





				//Vista. Entre 1 y 8
				if (tecla>='1' && tecla<='8') {
                	menu_debug_registers_set_view(&ventana,tecla-'0');
				    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}

				if (tecla=='f') {
					menu_debug_switch_follow_pc();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
				}

		        if (tecla=='t') {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
					//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;
                    menu_debug_registers_change_ptr();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                                        
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

                    //Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
                    //de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;
                }

				//Daad breakpoint
		        if (tecla=='k' && menu_debug_registers_current_view==8) {
					if (debug_allow_daad_breakpoint.v) {
						//Quitarlo
						menu_debug_delete_daad_special_breakpoint();
					}
                    else {
						//Ponerlo
						menu_debug_add_daad_special_breakpoint();
					}

					debug_allow_daad_breakpoint.v ^=1;

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }				

                if (tecla=='r') {
                	//Detener multitarea, porque si no, se input ejecutara opcodes de la cpu, al tener que leer el teclado
					int antes_menu_multitarea=menu_multitarea;
					menu_multitarea=0;

                    menu_debug_change_registers();

                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;

					//Restaurar estado multitarea despues de menu_debug_registers_ventana, pues si hay algun error derivado
					//de cambiar registros, se mostraria ventana de error, y se ejecutaria opcodes de la cpu, al tener que leer el teclado
					menu_multitarea=antes_menu_multitarea;
                }


			    if (tecla==11) {
                	//arriba
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_up();
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }

                if (tecla==10) {
                	//abajo
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_down(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                	si_ejecuta_una_instruccion=0;
                }

                //24 pgup
                if (tecla==24) {
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_pgup(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }
                
				//25 pgdn
                if (tecla==25) {
                    //PgDn
                    menu_debug_follow_pc.v=0; //se deja de seguir pc
                    menu_debug_cursor_pgdn(&ventana);
                    //Decimos que no hay tecla pulsada
                    acumulado=MENU_PUERTO_TECLADO_NINGUNA;
                    //decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                }


				if (tecla=='v') {
					menu_espera_no_tecla_no_cpu_loop();
				    //para que no se vea oscuro
				    menu_set_menu_abierto(0);
					menu_cls_refresh_emulated_screen();
				    menu_espera_tecla_no_cpu_loop();
					menu_espera_no_tecla_no_cpu_loop();

					//vuelta a oscuro
				    menu_set_menu_abierto(1);

					menu_cls_refresh_emulated_screen();

					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;

					//Y redibujar ventana
					zxvision_draw_window(&ventana);
				}

				if (tecla=='s') { 
					cpu_step_mode.v=0;
					//Decimos que no hay tecla pulsada
					acumulado=MENU_PUERTO_TECLADO_NINGUNA;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
					si_ejecuta_una_instruccion=0;
				}

				if (tecla==2) { //ESC
					cpu_step_mode.v=0;
					//decirle que despues de pulsar esta tecla no tiene que ejecutar siguiente instruccion
                    si_ejecuta_una_instruccion=0;
                    acumulado=0; //teclas pulsadas
                    //Con esto saldremos

				}

				//Cualquier tecla no enter, no ejecuta instruccion
				if (tecla!=13) si_ejecuta_una_instruccion=0;

			}

			else {
				//Cualquier tecla Detiene el continuous loop excepto C
				//printf ("continuos loop\n");
				acumulado=menu_da_todas_teclas();
				if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA) {

					//tecla=menu_get_pressed_key();
					tecla=zxvision_common_getkey_refresh();

					if (tecla=='c') {
						menu_debug_registers_next_cont_speed();
						tecla=0;
						menu_espera_no_tecla_no_cpu_loop();
					}

					//Si tecla no es 0->0 se suele producir al mover el raton.
					if (tecla!=0) {
						continuous_step=0;
						//printf ("cont step: %d\n",continuous_step);

            			//Decimos que no hay tecla pulsada
            			acumulado=MENU_PUERTO_TECLADO_NINGUNA;

						menu_espera_no_tecla_no_cpu_loop();
					}

				}

			}


			//1 instruccion cpu
			if (si_ejecuta_una_instruccion) {
				//printf ("ejecutando instruccion en step-to-step o continuous\n");
				debug_core_lanzado_inter.v=0;

				screen_force_refresh=1; //Para que no haga frameskip y almacene los pixeles/atributos en buffer rainbow


				//Si vista daad (8)
				if (menu_debug_registers_current_view==8) {
					//Poner breakpoint hasta parser

					menu_debug_daad_step_breakpoint();
                    tecla=2; //Simular ESC
					cpu_step_mode.v=0;
					salir_todos_menus=1;
					acumulado=0;
					
                }					

				else cpu_core_loop();

				//Ver si se ha disparado interrupcion (nmi o maskable)
				//if (debug_core_lanzado_inter.v && debug_core_evitamos_inter.v) {
				if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
					debug_run_until_return_interrupt();
				}


				menu_debug_registers_show_scan_position();
			}

			if (menu_breakpoint_exception.v) {
				//Si accion nula o menu o break
				if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
					menu_debug_registers_gestiona_breakpoint();
				  	//Y redibujar ventana para reflejar breakpoint cond
					//menu_debug_registers_ventana();
				}

				else {
					menu_breakpoint_exception.v=0;
					//Gestion acciones
					debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
				}
			}

		}

	//Hacer mientras step mode este activo o no haya tecla pulsada
	//printf ("acumulado %d cpu_ste_mode: %d\n",acumulado,cpu_step_mode.v);
    } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA || cpu_step_mode.v==1);

	//Si no estamos haciendo stepping de daad, quitar breakpoint del parser
	if (debug_stepping_daad.v==0) {
		menu_debug_delete_daad_step_breakpoint();
	}

	//Si no estamos haciendo runto Parse de daad, quitar breakpoint del parser
	if (debug_stepping_daad_runto_parse.v==0) {
		menu_debug_delete_daad_parse_breakpoint();
	}	

    cls_menu_overlay();

	util_add_window_geometry("debugcpu",ventana.x,ventana.y,ventana.visible_width,ventana.visible_height);


	zxvision_destroy_window(&ventana);

}

/*

Partitura



 --------------------------------

 --------------------------------

 --------------------------------
 
 --------------------------------
 
 --------------------------------



5 lineas de pentagrana -> 4 separaciones

8 pixeles de alto cada separacion -> 32 pixeles cada pentagrama

Si cogemos dos mas para subir hasta el Do, son 8x6=48 pixeles por pentagama

48 * 3 = 144 los 3 pentagramas


Dibujo de la nota:

     012345678901
0 -----------------------------------
1    ..XXX..
2    .X...X.            
3    X.....X
4    X.....X
5    X.....X
6    .X...X.
7    ..XXX..
8 ----------------------------------
*/

#define PENTAGRAMA_NOTA_ALTO 7
#define PENTAGRAMA_NOTA_ANCHO 7
#define PENTAGRAMA_NOTA_LARGO_PALITO 17

//donde empieza el palito
#define PENTAGRAMA_NOTA_OFFSET_PALITO 3

#define PENTAGRAMA_ESPACIO_LINEAS 8

#define PENTAGRAMA_SOST_ANCHO 8
#define PENTAGRAMA_SOST_ALTO 11

#define PENTAGRAMA_ANCHO_NOTA_TOTAL (PENTAGRAMA_SOST_ANCHO+PENTAGRAMA_NOTA_ANCHO+6)

#define PENTAGRAMA_MARGEN_SOSTENIDO (PENTAGRAMA_SOST_ANCHO+2)

#define PENTAGRAMA_TOTAL_ALTO (PENTAGRAMA_ESPACIO_LINEAS*7)

//#define PENTAGRAMA_MARGEN_SUPERIOR 8
//Si hay mas de un chip, meter margen superior
#define PENTAGRAMA_MARGEN_SUPERIOR (total_ay_chips>1 ? 8 : 0)

char *pentagrama_nota_negra[PENTAGRAMA_NOTA_ALTO]={
   //0123456
    "  XXX  ",  
	" XXXXX ",
	"XXXXXXX",
	"XXXXXXX",
	"XXXXXXX",
	" XXXXX ",
	"  XXX  "
};

char *pentagrama_nota_blanca[PENTAGRAMA_NOTA_ALTO]={
   //0123456
    "  XXX  ",  
	" X   X ",
	"X     X",
	"X     X",
	"X     X",
	" X   X ",
	"  XXX  "
};

#define PENTAGRAMA_PUNTILLO_ALTO 2
#define PENTAGRAMA_PUNTILLO_ANCHO 2

/* char *pentagrama_puntillo[PENTAGRAMA_PUNTILLO_ALTO]={
   //0123456
    " X ",  
	"XXX",
	" X "
};*/

char *pentagrama_puntillo[PENTAGRAMA_PUNTILLO_ALTO]={
   //0123456
    "XX",  
	"XX",
};


#define PENTAGRAMA_CLAVE_SOL_ALTO 44
#define PENTAGRAMA_CLAVE_SOL_ANCHO 30

char *pentagrama_clave_sol[PENTAGRAMA_CLAVE_SOL_ALTO]={
/*
 012345678901234567890123456789012345
*/

"                 XXXXX        ",
"               XXXXXXXX       ",
"              XXXXXXXXXX      ",
"             XXXXXXXXXXX      ",
"            XXXXX     XX      ",
"            XXXX      XX      ",
"            XXX       XX      ",
"            XX        XX      ",
"            XX      XXXX      ",
"            XX     XXXXX      ",
"            XX     XXXXX      ",
"            XX  XXXXXXX       ",
"            XXXXXXXXX         ",
"            XXXXXXXXX         ",
"          XXXXXXXXXX          ",
"        XXXXXXXXXX            ",
"       XXXXXXXXX              ",
"     XXXXXXXXXXX              ",
"    XXXXXXXXXX XX             ",
"   XXXXXXXXX   XX             ",
"  XXXXXXXX     XXXXXXX        ",
"  XXXXXXX    XXXXXXXXXXXX     ",
" XXXXX     XXXXXXXXXXXXXXXXX  ",
"XXXXX     XXXXXXXXXXXXXXXXXXX ",
"XXXXXX   XXXXXX   XX   XXXXXXX",
"XXXXXX   XXXXX     XX    XXXXX",
" XXXXX   XXXXX      XX     XXX",
"  XXXX    XXXX      XX     XXX",
"  XXXXX   XXXXX     XX     XX ",
"   XXXXX   XXXX      XX  XXXX ",
"    XXXXXX           XX XXXX  ",
"     XXXXXXX         XXXXX    ",
"          XXXXXXXXXXXXXXX     ",
"             XXXXXXXXXXX      ",
"                      XX      ",
"                      XX      ",
"          XXX         XX      ",
"        XXXXXX        XX      ",
"       XXXXXXXX       XX      ",
"       XXXXXXXX       XX      ",
"        XXXXXX       XX       ",
"          XXX       XX        ",
"          XXXXXXXXXXX          ",
"            XXXXXXXX          ",
};


char *pentagrama_sost[PENTAGRAMA_SOST_ALTO]={
  
//0123456789012
 "  X  X  ",    //0
 "  X  X  ",
 "  X  XXX",
 "  XXXX  ",
 "XXX  X  ",
 "  X  X  ",   //5
 "  X  XXX",
 "  XXXX  ",
 "XXX  X  ",
 "  X  X  ",    
 "  X  X  "     //10

};


//Clave de sol. 56 pixeles alto aprox

#define PIANO_PARTITURA_GRAPHIC_BASE_X (menu_origin_x() )
#define PIANO_PARTITURA_GRAPHIC_BASE_Y 0



#define PIANO_PARTITURA_ANCHO_VENTANA 32
#define PIANO_PARTITURA_ALTO_VENTANA 24

#define MENU_AY_PARTITURA_MAX_COLUMNS 30

zxvision_window *menu_ay_partitura_overlay_window;


//Lo que contiene cada pentagrama, de cada chip, de cada canal

//Chip, canal, columna, string de 4
char menu_ay_partitura_current_state[MAX_AY_CHIPS][3][MENU_AY_PARTITURA_MAX_COLUMNS][4];

//Chip, canal, columna, duracion de cada nota
int menu_ay_partitura_current_state_duraciones[MAX_AY_CHIPS][3][MENU_AY_PARTITURA_MAX_COLUMNS];

//Ultima columna de cada canal usada
int menu_ay_partitura_ultima_columna[3];

//Nota anterior de la ultima columna
//char menu_ay_partitura_last_state[MAX_AY_CHIPS][3][4];


//Hacer putpixel en pantalla de color indexado 16 bits. Usado en watermark para no rainbow
void menu_ay_partitura_putpixel_nota(z80_int *destino GCC_UNUSED,int x,int y,int ancho_destino GCC_UNUSED,int color GCC_UNUSED)
{
	//scr_putpixel(x,y,color);

	//zxvision_putpixel(menu_ay_partitura_overlay_window,x,y,color);
	zxvision_putpixel(menu_ay_partitura_overlay_window,x,y,ESTILO_GUI_TINTA_NORMAL);
}

void menu_ay_partitura_dibujar_sost(int x,int y)
{
	screen_put_asciibitmap_generic(pentagrama_sost,NULL,x,y,PENTAGRAMA_SOST_ANCHO,PENTAGRAMA_SOST_ALTO,0,menu_ay_partitura_putpixel_nota);
}

//duraciones notas
enum aysheet_tipo_nota_duracion {
	AYSHEET_NOTA_SEMIFUSA,
	AYSHEET_NOTA_SEMIFUSA_PUNTO,

	AYSHEET_NOTA_FUSA,
	AYSHEET_NOTA_FUSA_PUNTO,	

	AYSHEET_NOTA_SEMICORCHEA,
	AYSHEET_NOTA_SEMICORCHEA_PUNTO,	

	AYSHEET_NOTA_CORCHEA,
	AYSHEET_NOTA_CORCHEA_PUNTO,	

	AYSHEET_NOTA_NEGRA,
	AYSHEET_NOTA_NEGRA_PUNTO,	

	AYSHEET_NOTA_BLANCA,
	AYSHEET_NOTA_BLANCA_PUNTO,	

	AYSHEET_NOTA_REDONDA,
	AYSHEET_NOTA_REDONDA_PUNTO

};

//retorne el char * de donde esta la nota (blanca,redonda=pentagrama_nota_blanca. resto=pentagrama_nota_negra)
char **aysheet_tipo_nota_bitmap(enum aysheet_tipo_nota_duracion nota)
{
	switch (nota) {
		case AYSHEET_NOTA_BLANCA:
		case AYSHEET_NOTA_BLANCA_PUNTO:
		case AYSHEET_NOTA_REDONDA:
		case AYSHEET_NOTA_REDONDA_PUNTO:
			return pentagrama_nota_blanca;
		break;

		default:
			return pentagrama_nota_negra;
		break;
	}
}


//dice si nota tiene "palito", o sea, todos menos la redonda
int aysheet_tipo_nota_tienepalo(enum aysheet_tipo_nota_duracion nota)
{
	switch (nota) {
		case AYSHEET_NOTA_REDONDA:
		case AYSHEET_NOTA_REDONDA_PUNTO:
			return 0;
		break;

		default:
			return 1;
		break;
	}
}

//dice el numero de diagonales que tiene la nota (corchea, semicorchea, fusa, semifusa)
int aysheet_tipo_nota_diagonales(enum aysheet_tipo_nota_duracion nota)
{
	switch (nota) {
		case AYSHEET_NOTA_CORCHEA:
		case AYSHEET_NOTA_CORCHEA_PUNTO:
			return 1;
		break;

		case AYSHEET_NOTA_SEMICORCHEA:
		case AYSHEET_NOTA_SEMICORCHEA_PUNTO:
			return 2;
		break;

		case AYSHEET_NOTA_FUSA:
		case AYSHEET_NOTA_FUSA_PUNTO:
			return 3;
		break;

		case AYSHEET_NOTA_SEMIFUSA:
		case AYSHEET_NOTA_SEMIFUSA_PUNTO:
			return 4;
		break;				


		default:
			return 0;
		break;
	}
}

//dice si nota tiene puntillo
int aysheet_tipo_nota_tienepuntillo(enum aysheet_tipo_nota_duracion nota)
{
	switch (nota) {
		case AYSHEET_NOTA_SEMIFUSA_PUNTO:
		case AYSHEET_NOTA_FUSA_PUNTO:
		case AYSHEET_NOTA_SEMICORCHEA_PUNTO:
		case AYSHEET_NOTA_CORCHEA_PUNTO:
		case AYSHEET_NOTA_NEGRA_PUNTO:
		case AYSHEET_NOTA_BLANCA_PUNTO:	
		case AYSHEET_NOTA_REDONDA_PUNTO:
			return 1;
		break;

		default:
			return 0;
		break;
	}
}

//Devuelve tipo de nota segun su duracion en 1/50 de segundo
enum aysheet_tipo_nota_duracion menu_aysheet_get_length(int duracion)
{
/*
	Duraciones notas:


	3.125=0.0625 segundos=semifusa
	4.6875=0.09375 segundos=semifusa con punto
	
	6.25=0.125 segundos=fusa
	9.375=0.1875 segundos=fusa con punto

	12.5=0.25 segundos=semicorchea
	18.75=0.375 segundos=semicorchea con punto
	
	25=0.5 segundos=corchea
	37.5=0.75 segundos=corchea con punto

	50=1 segundo=negra
	75=1.5 segundos=negra con punto

	100=2 segundos=blanca
	150=3 segundos=blanca con punto

	200=4 segundos=redonda
	300=6 segundos=redonda con punto

	 */

	//Vemos duraciones segun si es menor o igual. Hacemos redondeos de duraciones: 4.6 es 5
	if (duracion<=3) return AYSHEET_NOTA_SEMIFUSA;
	if (duracion<=5) return AYSHEET_NOTA_SEMIFUSA_PUNTO;
	if (duracion<=6) return AYSHEET_NOTA_FUSA;
	if (duracion<=9) return AYSHEET_NOTA_FUSA_PUNTO;

	if (duracion<=12) return AYSHEET_NOTA_SEMICORCHEA;
	if (duracion<=19) return AYSHEET_NOTA_SEMICORCHEA_PUNTO;
	if (duracion<=25) return AYSHEET_NOTA_CORCHEA;
	if (duracion<=37) return AYSHEET_NOTA_CORCHEA_PUNTO;
	
	if (duracion<=50) return AYSHEET_NOTA_NEGRA;
	if (duracion<=75) return AYSHEET_NOTA_NEGRA_PUNTO;
	if (duracion<=100) return AYSHEET_NOTA_BLANCA;
	if (duracion<=150) return AYSHEET_NOTA_BLANCA_PUNTO;

	if (duracion<=200) return AYSHEET_NOTA_REDONDA;

	//Cualquier otra cosa
	return AYSHEET_NOTA_REDONDA_PUNTO;

}

//incremento_palito: +1 : palito hacia abajo
//incremento_palito: +1 : palito hacia arriba
//duracion en 1/50 de segundos. 50=negra
void menu_ay_partitura_dibujar_nota(int x,int y,int incremento_palito,int duracion)
{


	enum aysheet_tipo_nota_duracion tipo_nota_duracion=menu_aysheet_get_length(duracion);

	char **bitmap_nota=aysheet_tipo_nota_bitmap(tipo_nota_duracion);

	bitmap_nota=aysheet_tipo_nota_bitmap(tipo_nota_duracion);




	screen_put_asciibitmap_generic(bitmap_nota,NULL,x,y,PENTAGRAMA_NOTA_ANCHO,PENTAGRAMA_NOTA_ALTO,0,menu_ay_partitura_putpixel_nota);
	

	//PENTAGRAMA_NOTA_LARGO_PALITO
	if (aysheet_tipo_nota_tienepalo(tipo_nota_duracion)) {
		int yorig=y+PENTAGRAMA_NOTA_OFFSET_PALITO;

		int xorig=x;

		//Si palito hacia arriba
		if (incremento_palito<0) xorig=x+PENTAGRAMA_NOTA_ANCHO-1;

		int alto=PENTAGRAMA_NOTA_LARGO_PALITO;

		for (;alto>0;alto--,yorig +=incremento_palito) {
			zxvision_putpixel(menu_ay_partitura_overlay_window,xorig,yorig,ESTILO_GUI_TINTA_NORMAL); 
		}


		//Diagonales de la nota. corchea, semi, etc
		int diagonales=aysheet_tipo_nota_diagonales(tipo_nota_duracion);
		int i;
		int largo_diagonal=5;

		
		for (i=0;i<diagonales;i++) {
			int l;
			yorig=y+PENTAGRAMA_NOTA_OFFSET_PALITO+((PENTAGRAMA_NOTA_LARGO_PALITO-i*3)*incremento_palito);
			for (l=0;l<largo_diagonal;l++) {
				zxvision_putpixel(menu_ay_partitura_overlay_window,xorig+l,yorig-(l*incremento_palito),ESTILO_GUI_TINTA_NORMAL); 
			}
		}		
	}

	//Si hay que dibujar puntillo
	if (aysheet_tipo_nota_tienepuntillo(tipo_nota_duracion)) {
		screen_put_asciibitmap_generic(pentagrama_puntillo,NULL,x+PENTAGRAMA_NOTA_ANCHO+1,y+PENTAGRAMA_NOTA_ALTO/2+1,
				PENTAGRAMA_PUNTILLO_ANCHO,PENTAGRAMA_PUNTILLO_ALTO,0,menu_ay_partitura_putpixel_nota);
	}




}

void meny_ay_partitura_dibujar_clavesol(int x,int y)
{
	screen_put_asciibitmap_generic(pentagrama_clave_sol,NULL,x,y,PENTAGRAMA_CLAVE_SOL_ANCHO,PENTAGRAMA_CLAVE_SOL_ALTO,0,menu_ay_partitura_putpixel_nota);	
}




void menu_ay_partitura_linea(int x,int y,int ancho)
{

		for (;ancho>0;ancho--,x++) {
			zxvision_putpixel(menu_ay_partitura_overlay_window,x,y,ESTILO_GUI_TINTA_NORMAL);
		}	
}

void menu_ay_partitura_lineas_pentagrama(int x,int y,int ancho,int separacion_alto)
{
	int lineas;

	int y_clavesol=y-7;

	for (lineas=0;lineas<5;lineas++) {
		menu_ay_partitura_linea(x,y,ancho);

		y +=separacion_alto;
	}


	meny_ay_partitura_dibujar_clavesol(x,y_clavesol);
}

//nota puede ser:  do, re, mi, fa, sol, la, si, do, re... si (0...13)
void menu_ay_partitura_nota_pentagrama(int x,int y,int nota,int si_sostenido,int duracion)
{
	//origen y=fa (10)

	int diferencia_nota=10-nota;

	int incremento_alto=diferencia_nota*(PENTAGRAMA_ESPACIO_LINEAS/2);

	//Y el punto inicial y
	int ynota=y+incremento_alto-PENTAGRAMA_NOTA_OFFSET_PALITO;

	//A partir del do, palito para abajo
	int incremento_palito=-1;

	if (nota>=6) incremento_palito=+1; //A partir del Si , palito para abajo


	menu_ay_partitura_dibujar_nota(x,ynota,incremento_palito,duracion);

	//Si hay que poner palito de linea pentagrama (en do (0), la (12), si(12))
	if (nota==0 || nota==12 || nota==13) {
		int ypalito;
		if (nota==0 || nota==12) ypalito=ynota+PENTAGRAMA_NOTA_OFFSET_PALITO;
		else ypalito=ynota+PENTAGRAMA_NOTA_ALTO; //si (12)

		menu_ay_partitura_linea(x-2,ypalito,PENTAGRAMA_NOTA_ANCHO+4);	
	}

	if (si_sostenido) {
		x=x-PENTAGRAMA_MARGEN_SOSTENIDO;
		ynota -=2; //un poquito mas para arriba
		menu_ay_partitura_dibujar_sost(x,ynota);
	}


}

//nota puede ser:  do, re, mi, fa, sol, la, si, do, re... si (0...13)
void menu_ay_partitura_nota_pentagrama_pos(int xorig,int yorig,int columna,int nota,int si_sostenido,int duracion)
{
	int ancho_columna=PENTAGRAMA_ANCHO_NOTA_TOTAL;

	int posx=(columna*ancho_columna)+xorig;

	//Y darle margen por la izquierda pos si hay sostenido
	posx +=PENTAGRAMA_MARGEN_SOSTENIDO;

	//darle margen de la clave de sol
	posx +=PENTAGRAMA_CLAVE_SOL_ANCHO;

	menu_ay_partitura_nota_pentagrama(posx,yorig,nota,si_sostenido,duracion);
}



int menu_ay_partitura_ancho_col_texto(void)
{
	return menu_char_width;
}

int menu_ay_partitura_total_columns(void)
{
	int ancho_columna=menu_ay_partitura_ancho_col_texto();
	int ancho_nota=PENTAGRAMA_ANCHO_NOTA_TOTAL;
	int total_columnas=(((menu_ay_partitura_overlay_window->visible_width)*ancho_columna)-PENTAGRAMA_MARGEN_SOSTENIDO*2)/ancho_nota;

	total_columnas--; //1 menos

	//de la clave de sol
	total_columnas--;


	if (total_columnas>MENU_AY_PARTITURA_MAX_COLUMNS) total_columnas=MENU_AY_PARTITURA_MAX_COLUMNS;

	//control minimos
	if (total_columnas<2) total_columnas=2;

	return total_columnas;	
}




//Scroll de un chip entero
void menu_ay_partitura_scroll(int chip)
{

		//Meter valor actual

	int total_columnas=menu_ay_partitura_total_columns();
	int i;

	for (i=0;i<total_columnas-1;i++) {
		int canal;
		for (canal=0;canal<3;canal++) {
			strcpy(menu_ay_partitura_current_state[chip][canal][i],menu_ay_partitura_current_state[chip][canal][i+1]);
			menu_ay_partitura_current_state_duraciones[chip][canal][i]=menu_ay_partitura_current_state_duraciones[chip][canal][i+1];
		}
	}

	//La ultima columna ponerla a vacia
		int canal;
		for (canal=0;canal<3;canal++) {
			menu_ay_partitura_current_state[chip][canal][i][0]=0;
			menu_ay_partitura_current_state_duraciones[chip][canal][i]=0;
		}	

	//Desplazar indices de ultima columna a la izquierda
		
	for (i=0;i<3;i++) {
		int c=menu_ay_partitura_ultima_columna[i];
		if (c>0) {
			menu_ay_partitura_ultima_columna[i]=c-1;
		}
	}		
}





void menu_ay_partitura_draw_state(int chip,int canal)
{

	int x=0;
	int y=PENTAGRAMA_ESPACIO_LINEAS*2;
	int duracion;


	y +=canal*(PENTAGRAMA_TOTAL_ALTO+1); //+1 para dejar 1 pixelillo de margen


	y +=PENTAGRAMA_MARGEN_SUPERIOR;


	int ancho_columna=menu_ay_partitura_ancho_col_texto();

	//Las lineas de pentagrama que dejen espacio a la izquierda y derecha, de ancho=ancho_columna
	menu_ay_partitura_lineas_pentagrama(x+ancho_columna,y,((menu_ay_partitura_overlay_window->visible_width)-2)*ancho_columna,PENTAGRAMA_ESPACIO_LINEAS);



	int ancho_nota=PENTAGRAMA_ANCHO_NOTA_TOTAL;
	int total_columnas;


	int i;


	total_columnas=menu_ay_partitura_total_columns();
	//printf ("total columnas: %d\n",total_columnas);

	for (i=0;i<total_columnas;i++) {
		char *string_nota;
		string_nota=menu_ay_partitura_current_state[chip][canal][i];
		//if (canal==0) printf ("%d [%s]\n",i,string_nota);

		//Nota leida canal 0

		int nota_final=-1;
		int octava;
		int sostenido;

		get_note_values(string_nota,&nota_final,&sostenido,&octava);
		if (nota_final>=0) {

			//Si octava impar, va hacia arriba
			if (octava & 1) nota_final +=7;

			duracion=menu_ay_partitura_current_state_duraciones[chip][canal][i];

			menu_ay_partitura_nota_pentagrama_pos(x+ancho_nota,y,i,nota_final,sostenido,duracion);
		}
	}



}

int menu_ay_partitura_chip=0;

void menu_ay_partitura_overlay(void)
{


	normal_overlay_texto_menu();


	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech, en el caso que se habilite piano de tipo texto


	//int chip=menu_ay_partitura_chip;


			char nota_a[4];
			char nota_b[4];
			char nota_c[4];




			int freq_a=ay_retorna_frecuencia(0,menu_ay_partitura_chip);
			int freq_b=ay_retorna_frecuencia(1,menu_ay_partitura_chip);
			int freq_c=ay_retorna_frecuencia(2,menu_ay_partitura_chip);


			sprintf(nota_a,"%s",get_note_name(freq_a) );

			
			sprintf(nota_b,"%s",get_note_name(freq_b) );

			
			sprintf(nota_c,"%s",get_note_name(freq_c) );

			//Si canales no suenan como tono, o volumen 0 meter cadena vacia en nota
			if (ay_3_8912_registros[menu_ay_partitura_chip][7]&1 || ay_3_8912_registros[menu_ay_partitura_chip][8]==0) nota_a[0]=0;
			if (ay_3_8912_registros[menu_ay_partitura_chip][7]&2 || ay_3_8912_registros[menu_ay_partitura_chip][9]==0) nota_b[0]=0;
			if (ay_3_8912_registros[menu_ay_partitura_chip][7]&4 || ay_3_8912_registros[menu_ay_partitura_chip][10]==0) nota_c[0]=0;


	


	//Si notas anteriores distintas de las actuales, scroll izquierda


	//printf ("a [%s] [%s]\n",nota_a,menu_ay_partitura_last_state[0][0]);
	//printf ("b [%s] [%s]\n",nota_b,menu_ay_partitura_last_state[0][1]);
	//printf ("c [%s] [%s]\n",nota_c,menu_ay_partitura_last_state[0][2]);

	int columna_estado_anterior;
	//columna_estado_anterior=menu_ay_partitura_total_columns()-1;

	int hayscroll=0;
	int modificado_canal1=0;
	int modificado_canal2=0;
	int modificado_canal3=0;

	//Si alguno de los 3 canales es diferente del estado anterior
	columna_estado_anterior=menu_ay_partitura_ultima_columna[0];
	if (strcasecmp(nota_a,menu_ay_partitura_current_state[menu_ay_partitura_chip][0][columna_estado_anterior])) {
		hayscroll=1;
		modificado_canal1=1;
	}
	else {
		//se mantiene igual. aumentar duracion
		menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][0][columna_estado_anterior]++;
	}


	columna_estado_anterior=menu_ay_partitura_ultima_columna[1];
	if (strcasecmp(nota_b,menu_ay_partitura_current_state[menu_ay_partitura_chip][1][columna_estado_anterior])) {
		hayscroll=1;
		modificado_canal2=1;
	}
	else {
		//se mantiene igual. aumentar duracion
		menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][1][columna_estado_anterior]++;
	}	


	columna_estado_anterior=menu_ay_partitura_ultima_columna[2];
	if (strcasecmp(nota_c,menu_ay_partitura_current_state[menu_ay_partitura_chip][2][columna_estado_anterior])) {
		hayscroll=1;
		modificado_canal3=1;
	}
	else {
		//se mantiene igual. aumentar duracion
		menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][2][columna_estado_anterior]++;
	}	




	if (hayscroll) {
		menu_ay_partitura_scroll(menu_ay_partitura_chip);

		//Meter valor actual los que se han modificado
		if (modificado_canal1) {
			//Y decir que ultima columna es la de mas a la derecha
			columna_estado_anterior=menu_ay_partitura_total_columns()-1;
			menu_ay_partitura_ultima_columna[0]=columna_estado_anterior;

			strcpy(menu_ay_partitura_current_state[menu_ay_partitura_chip][0][columna_estado_anterior],nota_a);
			menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][0][columna_estado_anterior]=1;
		}

		if (modificado_canal2) {
			//Y decir que ultima columna es la de mas a la derecha
			columna_estado_anterior=menu_ay_partitura_total_columns()-1;
			menu_ay_partitura_ultima_columna[1]=columna_estado_anterior;

			strcpy(menu_ay_partitura_current_state[menu_ay_partitura_chip][1][columna_estado_anterior],nota_b);
			menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][1][columna_estado_anterior]=1;
		}

		if (modificado_canal3) {
			//Y decir que ultima columna es la de mas a la derecha
			columna_estado_anterior=menu_ay_partitura_total_columns()-1;
			menu_ay_partitura_ultima_columna[2]=columna_estado_anterior;

			strcpy(menu_ay_partitura_current_state[menu_ay_partitura_chip][2][columna_estado_anterior],nota_c);
			menu_ay_partitura_current_state_duraciones[menu_ay_partitura_chip][2][columna_estado_anterior]=1;
		}				
		


	}

	//Dibujar estado de los 3 canales
	menu_ay_partitura_draw_state(menu_ay_partitura_chip,0);
	menu_ay_partitura_draw_state(menu_ay_partitura_chip,1);
	menu_ay_partitura_draw_state(menu_ay_partitura_chip,2);

	


	zxvision_draw_window_contents(menu_ay_partitura_overlay_window); 

}


void menu_ay_partitura_init_state(void)
{
			

		//Inicializar estado con string "" y duraciones 0

		

		int chip;
		for (chip=0;chip<MAX_AY_CHIPS;chip++) {
			int canal;
			for (canal=0;canal<3;canal++) {
				int col;

				for (col=0;col<MENU_AY_PARTITURA_MAX_COLUMNS;col++) {
					menu_ay_partitura_current_state[chip][canal][col][0]=0;
					menu_ay_partitura_current_state_duraciones[chip][canal][col]=0;

				}
			}
		}


}

void menu_ay_partitura_init_state_last_column(void)
{
			
		//printf ("ultima col %d\n",menu_ay_partitura_total_columns());
		menu_ay_partitura_ultima_columna[0]=menu_ay_partitura_total_columns()-1;
		menu_ay_partitura_ultima_columna[1]=menu_ay_partitura_total_columns()-1;
		menu_ay_partitura_ultima_columna[2]=menu_ay_partitura_total_columns()-1;
}

void menu_aysheet_change_chip(MENU_ITEM_PARAMETERS)
{
	menu_ay_partitura_chip++;
	if (menu_ay_partitura_chip==total_ay_chips) menu_ay_partitura_chip=0;
}

zxvision_window zxvision_window_ay_partitura;


void menu_ay_partitura(MENU_ITEM_PARAMETERS)
{



        menu_espera_no_tecla();

		int xventana,yventana,ancho_ventana,alto_ventana;

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		//Inicializar array de estado
		menu_ay_partitura_init_state();


		if (!util_find_window_geometry("aysheet",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {				
						
						xventana=PIANO_PARTITURA_GRAPHIC_BASE_X;
						yventana=PIANO_PARTITURA_GRAPHIC_BASE_Y;
						ancho_ventana=PIANO_PARTITURA_ANCHO_VENTANA;
						alto_ventana=PIANO_PARTITURA_ALTO_VENTANA;						

		}
				


		char *titulo_ventana="AY Sheet (60 BPM)";
		int ancho_titulo=menu_da_ancho_titulo(titulo_ventana);

		//Para que siempre se lea el titulo de la ventana
		if (ancho_ventana<ancho_titulo) ancho_ventana=ancho_titulo;

		//printf ("ancho %d\n",ancho_ventana);

		zxvision_window *ventana;
		ventana=&zxvision_window_ay_partitura;

		zxvision_new_window_nocheck_staticsize(ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,titulo_ventana);

		zxvision_draw_window(ventana);	


		
		//Comprobacion inicial de que el chip seleccionado no es mayor que los disponibles
		if (menu_ay_partitura_chip>=total_ay_chips) menu_ay_partitura_chip=0;


		//printf ("ancho creada %d\n",ventana->visible_width);	

		menu_ay_partitura_overlay_window=ventana;	

		menu_ay_partitura_init_state_last_column();	


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de piano + texto
        set_menu_overlay_function(menu_ay_partitura_overlay);

	
		
		//Si solo hay 1 chip, no mostrar selector de chip
		if (total_ay_chips==1) zxvision_wait_until_esc(ventana);

		else {
        
        	//Los array de menu_item no tienen porque cambiar el nombre en cada sitio que se usen
        	menu_item *array_menu_nonamed;
        	menu_item item_seleccionado;

        	int nonamed_opcion_seleccionada=0; //Solo 1 item de menu, no tiene sentido guardar posicion
        
        	int retorno_menu;
        	do {

			
            	menu_add_item_menu_inicial_format(&array_menu_nonamed,MENU_OPCION_NORMAL,menu_aysheet_change_chip,NULL,"[%d] Selected ~~Chip",menu_ay_partitura_chip+1);
	        	menu_add_item_menu_shortcut(array_menu_nonamed,'c');

        		//Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
        		//menu_add_item_menu_tooltip(array_menu_nonamed,"Change wave Shape");
        		menu_add_item_menu_ayuda(array_menu_nonamed,"Change selected chip");
            	menu_add_item_menu_tabulado(array_menu_nonamed,1,0);
			


            	//Nombre de ventana solo aparece en el caso de stdout
        		retorno_menu=menu_dibuja_menu(&nonamed_opcion_seleccionada,&item_seleccionado,array_menu_nonamed,"AY Sheet (60 BPM)" );


            	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
            	cls_menu_overlay();
        		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                	//llamamos por valor de funcion
            		if (item_seleccionado.menu_funcion!=NULL) {
                		//printf ("actuamos por funcion\n");
                		item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                		//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	            	}
    	    	}

    		} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);		
		}



				

	//restauramos modo normal de texto de menu
	set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();


	util_add_window_geometry_compact("aysheet",ventana);
	zxvision_destroy_window(ventana);			
	

}







void menu_record_mid_start(MENU_ITEM_PARAMETERS)
{
	
	if (mid_has_been_initialized()) {
		if (!menu_confirm_yesno_texto("Will empty buffer","Sure?")) return;
	}

	mid_initialize_export();
	mid_is_recording.v=1;
}



void menu_record_mid_stop(MENU_ITEM_PARAMETERS)
{
	if (mid_has_been_initialized()) {
		if (!menu_confirm_yesno_texto("Stop recording","Sure?")) return;
	}	
	mid_is_recording.v=0;
}


void menu_record_mid_pause_unpause(MENU_ITEM_PARAMETERS)
{
	mid_is_paused.v ^=1;
}

void menu_record_mid_save(MENU_ITEM_PARAMETERS)
{
        char file_save[PATH_MAX];

        char *filtros[2];

        filtros[0]="mid";
    filtros[1]=0;


    //guardamos directorio actual
    char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);	

	//Obtenemos ultimo directorio visitado
        if (mid_export_file[0]!=0) {
                char directorio[PATH_MAX];
                util_get_dir(mid_export_file,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }	

    int ret;

        ret=menu_filesel("Mid file",filtros,file_save);

        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);		

        if (ret==1) {

                //Ver si archivo existe y preguntar
                if (si_existe_archivo(file_save)) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

       			}

			strcpy(mid_export_file,file_save);
			mid_flush_file();

	        menu_generic_message_splash("Save MID","OK File saved");

 
        }
}

void menu_record_mid_noisetone(MENU_ITEM_PARAMETERS)
{
	mid_record_noisetone.v ^=1;	
}

void menu_record_mid(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_record_mid;
	menu_item item_seleccionado;
	int retorno_menu;

        do {

                    menu_add_item_menu_inicial(&array_menu_record_mid,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

					if (mid_is_recording.v==0) {
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_start,menu_cond_ay_chip,"Start Recording");	
					}

					else {
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_stop,menu_cond_ay_chip,"Stop Recording");	
					}

				




					if (mid_is_recording.v) {

						if (mid_is_paused.v==0) {
							menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_pause_unpause,menu_cond_ay_chip,"Pause Recording");
						}

						else {
							menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_pause_unpause,menu_cond_ay_chip,"Resume Recording");
						}					
					}

					//No dejamos grabar hasta que no se haga stop
					//porque el flush del final mete cabeceras de final de pistas y ya no se puede reaprovechar
					if (mid_has_been_initialized() && mid_notes_recorded && mid_is_recording.v==0) {
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_save,menu_cond_ay_chip,"Save .MID file");
					}


					menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"");
					menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_NORMAL,menu_record_mid_noisetone,NULL,"[%c] Allow tone+noise",
						(mid_record_noisetone.v ? 'X' : ' ') );
					menu_add_item_menu_tooltip(array_menu_record_mid,"Record also channels enabled as tone+noise");
					menu_add_item_menu_ayuda(array_menu_record_mid,"Record also channels enabled as tone+noise");

					if (mid_notes_recorded) {

						int max_buffer=mid_max_buffer();
						

						int max_buffer_perc=(max_buffer*100)/MAX_MID_EXPORT_BUFFER;

						//printf ("%d %d\n",max_buffer,max_buffer_perc);


						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"");
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"Info:");
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"Buffer used: %d%%",max_buffer_perc);
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"Voices: %d",3*mid_chips_al_start);
						menu_add_item_menu_format(array_menu_record_mid,MENU_OPCION_SEPARADOR,NULL,NULL,"Notes recorded: %d",mid_notes_recorded);
						

					}					
		
					



                menu_add_item_menu(array_menu_record_mid,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_record_mid,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_record_mid);

                retorno_menu=menu_dibuja_menu(&record_mid_opcion_seleccionada,&item_seleccionado,array_menu_record_mid,"AY to .mid" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
							
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



//Funcion comun de midi output de alsa y coreaudio
void menu_midi_output_noisetone(MENU_ITEM_PARAMETERS)
{
	midi_output_record_noisetone.v ^=1;
}

//Funcion comun de midi output de alsa y coreaudio
void menu_midi_output_initialize(MENU_ITEM_PARAMETERS)
{
	


	if (audio_midi_output_initialized==0) {
		if (audio_midi_output_init() ) {
			menu_error_message("Error initializing midi device");
		}
	}
	else {
		audio_midi_output_finish();
		audio_midi_output_initialized=0;
	}	
}

int menu_midi_output_initialized_cond(void)
{
	return !audio_midi_output_initialized;
}


void menu_midi_output_client(MENU_ITEM_PARAMETERS)
{
        char string_valor[4];
        int valor;


        sprintf (string_valor,"%d",audio_midi_client);


        menu_ventana_scanf("Client value",string_valor,4);

        valor=parse_string_to_number(string_valor);
	if (valor<0 || valor>255) {
		menu_error_message("Invalid client value");
	}


	audio_midi_client=valor;

}

void menu_midi_output_port(MENU_ITEM_PARAMETERS)
{
        char string_valor[4];
        int valor;


        sprintf (string_valor,"%d",audio_midi_port);


        menu_ventana_scanf("Port value",string_valor,4);

        valor=parse_string_to_number(string_valor);
        if (valor<0 || valor>255) {
                menu_error_message("Invalid client value");
        }


        audio_midi_port=valor;

}



//Listar dispositivos midi. Solo tiene sentido esto en Linux
void menu_direct_alsa_midi_output_list_devices(MENU_ITEM_PARAMETERS) 
{

	char *device_list="/proc/asound/seq/clients";

	if (!si_existe_archivo(device_list)) {
		menu_error_message("Can not find device list");
		return;
	}

	//Abrir archivo y mostrarlo en ventana
	//Usamos esta funcion generica de mostrar archivos de ayuda
	menu_about_read_file("Sequencer devices",device_list);

}




void menu_direct_midi_output(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_direct_midi_output;
	menu_item item_seleccionado;
	int retorno_menu;

        do {


		menu_add_item_menu_inicial(&array_menu_direct_midi_output,"",MENU_OPCION_UNASSIGNED,NULL,NULL);



#ifdef COMPILE_ALSA
		//En Alsa Linux
		menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_direct_alsa_midi_output_list_devices,NULL,"List midi devices");
		menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_client,menu_midi_output_initialized_cond,"Midi client: %d",audio_midi_client);
		menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_port,menu_midi_output_initialized_cond,"Midi port: %d",audio_midi_port);
#endif

#ifdef MINGW
		//en Windows
		menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_port,menu_midi_output_initialized_cond,"Midi port: %d",audio_midi_port);
#endif


	
		if (audio_midi_output_initialized==0) {
			menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_initialize,NULL,"Initialize midi");
		}
		else {
			menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_initialize,NULL,"Disable midi");
		}

		menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,NULL,NULL,"Initialized: %s",
			(audio_midi_output_initialized ? "Yes" : "No" ) );

		//Parece que no funciona la gestion de volumen
		//menu_add_item_menu_format(array_menu_direct_alsa_midi_output,MENU_OPCION_NORMAL,menu_direct_alsa_midi_output_volume,NULL,"Volume: %d%%",alsa_midi_volume);



					menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_SEPARADOR,NULL,NULL,"");
					menu_add_item_menu_format(array_menu_direct_midi_output,MENU_OPCION_NORMAL,menu_midi_output_noisetone,NULL,"[%c] Allow tone+noise",
						(midi_output_record_noisetone.v ? 'X' : ' ') );
					menu_add_item_menu_tooltip(array_menu_direct_midi_output,"Send also channels enabled as tone+noise");
					menu_add_item_menu_ayuda(array_menu_direct_midi_output,"Send also channels enabled as tone+noise");


                menu_add_item_menu(array_menu_direct_midi_output,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_direct_midi_output,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_direct_midi_output);

                retorno_menu=menu_dibuja_menu(&direct_midi_output_opcion_seleccionada,&item_seleccionado,array_menu_direct_midi_output,"AY to MIDI output" );



		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}







//cambia filtro
void menu_ay_mixer_cambia_filtro(MENU_ITEM_PARAMETERS)
{

	int chip=valor_opcion/3;
	int canal=valor_opcion % 3;

	//printf ("chip %d canal %d\n",chip,canal);

	z80_byte valor_filtro=ay_filtros[chip];
	z80_byte mascara=1|8; //bits xxxx1xx1

	z80_byte mascara_ceros=1|8;

	if (canal>0) {
		mascara=mascara<<canal;
		mascara_ceros=mascara_ceros<<canal;
	}

	//aplicar mascara
	valor_filtro &=mascara;

	//Normalizar
	if (canal>0) valor_filtro=valor_filtro>>canal;


	//Valores posibles 0,1,8,9
	if (valor_filtro==0) valor_filtro=1;
	else if (valor_filtro==1) valor_filtro=8;
	else if (valor_filtro==8) valor_filtro=9;
	else valor_filtro=0;

	//Volver a meter donde estaba
	if (canal>0) {
		valor_filtro=valor_filtro<<canal;
	}

	mascara_ceros=255-mascara_ceros;

	//Poner a ceros los que habia
	ay_filtros[chip] &=mascara_ceros;

	//Poner filtro actual
	ay_filtros[chip] |=valor_filtro;
	
}

//Muestra cadena filtro
void menu_ay_mixer_retorna_filtro(int chip,int canal,char *destino)
{
	z80_byte valor_filtro=ay_filtros[chip];
	z80_byte mascara=1|8; //bits xxxx1xx1

	if (canal>0) mascara=mascara<<canal;

	//aplicar mascara
	valor_filtro &=mascara;

	//Normalizar
	if (canal>0) valor_filtro=valor_filtro>>canal;

	//Posibles valores: 0,1,8,9
	switch (valor_filtro) {
		case 0:
			strcpy(destino,"No filter");
		break;

		case 1:
			strcpy(destino,"No tone  ");
		break;

		case 8:
			strcpy(destino,"No noise ");
		break;		

		//case 9:
		default:
			strcpy(destino,"Silence  ");
		break;			

	}

	return;
}


void menu_audio_envelopes(MENU_ITEM_PARAMETERS)
{
	ay_envelopes_enabled.v^=1;
}

void menu_audio_speech(MENU_ITEM_PARAMETERS)
{
        ay_speech_enabled.v^=1;
}

void menu_audio_ay_stereo_custom_A(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) ay3_custom_stereo_A=0;
}

void menu_audio_ay_stereo_custom_B(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_B++;
	if (ay3_custom_stereo_B==3) ay3_custom_stereo_B=0;
}

void menu_audio_ay_stereo_custom_C(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_C++;
	if (ay3_custom_stereo_C==3) ay3_custom_stereo_C=0;
}

char *menu_stereo_positions[]={
	"Left",
	"    Center",
	"          Right"
};

void menu_ay_mixer(MENU_ITEM_PARAMETERS)
{
menu_item *array_menu_ay_mixer;
	menu_item item_seleccionado;
	int retorno_menu;

	char buffer_filtro[33];
	

        do {


		menu_add_item_menu_inicial_format(&array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_envelopes,menu_cond_ay_chip,"[%c] AY ~~Envelopes", (ay_envelopes_enabled.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_ay_mixer,'e');
		menu_add_item_menu_tooltip(array_menu_ay_mixer,"Enable or disable volume envelopes for the AY Chip");
		menu_add_item_menu_ayuda(array_menu_ay_mixer,"Enable or disable volume envelopes for the AY Chip");

		menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_speech,menu_cond_ay_chip,"[%c] AY ~~Speech", (ay_speech_enabled.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_ay_mixer,'s');
		menu_add_item_menu_tooltip(array_menu_ay_mixer,"Enable or disable AY Speech effects");
		menu_add_item_menu_ayuda(array_menu_ay_mixer,"These effects are used, for example, in Chase H.Q.");


		if (MACHINE_IS_SPECTRUM) {


			char ay3_stereo_string[16];
			if (ay3_stereo_mode==1) strcpy(ay3_stereo_string,"ACB");
			else if (ay3_stereo_mode==2) strcpy(ay3_stereo_string,"ABC");
			else if (ay3_stereo_mode==3) strcpy(ay3_stereo_string,"BAC");
			else if (ay3_stereo_mode==4) strcpy(ay3_stereo_string,"Custom");
			else strcpy(ay3_stereo_string,"Mono");

			menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_ay_stereo,menu_cond_ay_chip,"    AY S~~tereo: %s",
				ay3_stereo_string);
			menu_add_item_menu_shortcut(array_menu_ay_mixer,'t');

			if (ay3_stereo_mode==4) {	

				menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_A,menu_cond_ay_chip,
					"    Ch. A: %s",menu_stereo_positions[ay3_custom_stereo_A]);

				menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_B,menu_cond_ay_chip,
					"    Ch. B: %s",menu_stereo_positions[ay3_custom_stereo_B]);

				menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_C,menu_cond_ay_chip,
					"    Ch. C: %s",menu_stereo_positions[ay3_custom_stereo_C]);								


			}

		}		



			int chip;

			for (chip=0;chip<ay_retorna_numero_chips();chip++) {
				int canal;
				menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_SEPARADOR,NULL,NULL,"---Chip %d---",chip+1);

				for (canal=0;canal<3;canal++) {
					menu_ay_mixer_retorna_filtro(chip,canal,buffer_filtro);
					menu_add_item_menu_format(array_menu_ay_mixer,MENU_OPCION_NORMAL,menu_ay_mixer_cambia_filtro,NULL,"[%s] Channel %c",buffer_filtro,'A'+canal);

					menu_add_item_menu_valor_opcion(array_menu_ay_mixer,chip*3+canal);

				}
			}

		


                menu_add_item_menu(array_menu_ay_mixer,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_ay_mixer,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_ay_mixer);

                retorno_menu=menu_dibuja_menu(&ay_mixer_opcion_seleccionada,&item_seleccionado,array_menu_ay_mixer,"AY mixer" );



		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);	
}




void menu_uartbridge_file(MENU_ITEM_PARAMETERS)
{
	uartbridge_disable();

        char *filtros[2];

        filtros[0]="";
        filtros[1]=0;


        if (menu_filesel("Select Device File",filtros,uartbridge_name)==1) {
                if (!si_existe_archivo(uartbridge_name)) {
                        menu_error_message("File does not exist");
                        uartbridge_name[0]=0;
                        return;
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                uartbridge_name[0]=0;


        }

}


void menu_uartbridge_enable(MENU_ITEM_PARAMETERS)
{
	if (uartbridge_enabled.v) uartbridge_disable();
	else uartbridge_enable();
}


int menu_uartbridge_cond(void)
{
	if (uartbridge_name[0]==0) return 0;

	else return 1;

}


int menu_uartbridge_speed_cond(void)
{
	if (uartbridge_enabled.v) return 0;

	else return 1;

}

void menu_uartbridge_speed(MENU_ITEM_PARAMETERS)
{
	if (uartbridge_speed==CHDEV_SPEED_115200) uartbridge_speed=CHDEV_SPEED_DEFAULT;
	else uartbridge_speed++;
}



void menu_uartbridge(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_uartbridge;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_uartbridge_file_shown[13];

			
                        menu_tape_settings_trunc_name(uartbridge_name,string_uartbridge_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_uartbridge,MENU_OPCION_NORMAL,menu_uartbridge_file,NULL,"~~File [%s]",string_uartbridge_file_shown);
                        menu_add_item_menu_shortcut(array_menu_uartbridge,'f');
                        menu_add_item_menu_tooltip(array_menu_uartbridge,"Path to the serial device");
                        menu_add_item_menu_ayuda(array_menu_uartbridge,"Path to the serial device");


						//Lo separamos en dos pues cuando no esta habilitado, tiene que comprobar que el path no sea nulo
						if (uartbridge_enabled.v) {
							menu_add_item_menu_format(array_menu_uartbridge,MENU_OPCION_NORMAL,menu_uartbridge_enable,NULL,"[X] ~~Enabled");
						}
						else {
							menu_add_item_menu_format(array_menu_uartbridge,MENU_OPCION_NORMAL,menu_uartbridge_enable,menu_uartbridge_cond,"[ ] ~~Enabled");
						}	
						menu_add_item_menu_shortcut(array_menu_uartbridge,'e');


#ifndef MINGW

						if (uartbridge_speed==CHDEV_SPEED_DEFAULT) {
							menu_add_item_menu_format(array_menu_uartbridge,MENU_OPCION_NORMAL,menu_uartbridge_speed,menu_uartbridge_speed_cond,"[Default] Speed");
						}
						else {
							menu_add_item_menu_format(array_menu_uartbridge,MENU_OPCION_NORMAL,menu_uartbridge_speed,menu_uartbridge_speed_cond,"[%d] Speed",
							chardevice_getspeed_enum_int(uartbridge_speed));
						}

#endif
					

                        menu_add_item_menu(array_menu_uartbridge,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_uartbridge);

                retorno_menu=menu_dibuja_menu(&uartbridge_opcion_seleccionada,&item_seleccionado,array_menu_uartbridge,"UART Bridge" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



int menu_network_uartbridge_cond(void)
{
	if (MACHINE_IS_ZXUNO || MACHINE_IS_TBBLUE || MACHINE_IS_TSCONF) return 1;
	else return 0;
}



int contador_menu_zeng_connect_print=0;
/*void menu_zeng_connect_print(zxvision_window *w)
{
	char *mensaje="Connecting...";

	int max=strlen(mensaje);
	char mensaje_dest[32];
	strcpy(mensaje_dest,mensaje);

	mensaje_dest[contador_menu_zeng_connect_print]=0;

	zxvision_print_string_defaults_fillspc(w,1,0,mensaje_dest);	
	zxvision_draw_window_contents(w);

	contador_menu_zeng_connect_print++;
	if (contador_menu_zeng_connect_print>max) contador_menu_zeng_connect_print=0;
}
*/

void menu_zeng_connect_print(zxvision_window *w)
{
	char *mensaje="|/-\\";

	int max=strlen(mensaje);
	char mensaje_dest[32];

	int pos=contador_menu_zeng_connect_print % max;

	sprintf(mensaje_dest,"Connecting %c",mensaje[pos]);
	//printf ("pos: %d\n",pos);

	zxvision_print_string_defaults_fillspc(w,1,0,mensaje_dest);	
	zxvision_draw_window_contents(w);

	contador_menu_zeng_connect_print++;

}

int menu_zeng_connect_cond(zxvision_window *w GCC_UNUSED)
{
	return !zeng_enable_thread_running;
}

void menu_zeng_enable_disable(MENU_ITEM_PARAMETERS)
{
	if (zeng_enabled.v) {
		zeng_disable();
	}
	else {

		//Activamos ZRCP, que es lo logico, si es que no esta habilitado ya
		if (remote_protocol_enabled.v==0) enable_and_init_remote_protocol();		

		//menu_footer_clear_bottom_line();
		//menu_putstring_footer(0,2,"Connecting to remote...",WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);
		//all_interlace_scr_refresca_pantalla();

		//Lanzar el thread de activacion
		zeng_enable();
		 
		contador_menu_zeng_connect_print=0;

		zxvision_simple_progress_window("ZENG connection", menu_zeng_connect_cond,menu_zeng_connect_print );


		
		//menu_footer_bottom_line();
	}
}

int menu_zeng_enable_disable_cond(void)
{
	

	
	//Si esta habilitado, opcion siempre disponible para desactivar
	if (zeng_enabled.v) return 1;



	else {
		//Si esta hostname vacio
		if (zeng_remote_hostname[0]==0) return 0;
	}

	return 1;
}


//Si esta habilitado, no se puede cambiar parametro
int menu_zeng_host_cond(void)
{
	if (zeng_enabled.v) return 0;
	else return 1;
}

void menu_zeng_host(MENU_ITEM_PARAMETERS)
{

	menu_ventana_scanf("Remote host",zeng_remote_hostname,MAX_ZENG_HOSTNAME);
	
}


void menu_zeng_port(MENU_ITEM_PARAMETERS)
{


        char string_port[6];


        sprintf (string_port,"%d",zeng_remote_port);


	menu_ventana_scanf("Remote port",string_port,6);
	int numero=parse_string_to_number(string_port);
	if (numero<1 || numero>65535) {
		menu_error_message("Invalid port number");
		return;
	}

	zeng_remote_port=numero;

	
}

void menu_zeng_master(MENU_ITEM_PARAMETERS)
{
	zeng_i_am_master ^=1;	
}


void menu_zeng_snapshot_seconds(MENU_ITEM_PARAMETERS)
{

		char string_seconds[2];

		sprintf (string_seconds,"%d",zeng_segundos_cada_snapshot);


		menu_ventana_scanf("Snapshot seconds?",string_seconds,2);
		int numero=parse_string_to_number(string_seconds);

		if (numero<1 || numero>9) {
			menu_error_message("Invalid interval");
			return;
		}


		zeng_segundos_cada_snapshot=numero;

}


void menu_zeng_send_message(MENU_ITEM_PARAMETERS)
{
	char string_mensaje[AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH];
	string_mensaje[0]=0;

	menu_ventana_scanf("Message?",string_mensaje,AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH);	

	zeng_add_pending_send_message_footer(string_mensaje);
}


int menu_zeng_send_message_cond(void)
{
	//Si hay un mensaje pendiente de enviar, no permitir aun
	//Comprobamos tambien que zeng_enabled.v, esto no se usa en menu pero si en tecla directa F
	if (zeng_enabled.v==0) return 0;
	if (pending_zeng_send_message_footer) return 0;

	return 1;
}

void menu_zeng_cancel_connect(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Still connecting","Cancel?")) {
		//printf ("cancelling zeng connect\n");
		zeng_cancel_connect();
	}
}

void menu_zeng(MENU_ITEM_PARAMETERS)
{
        //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

			menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

			//Si esta thread de conexion ejecutandose, mostrar otra opcion
			if (zeng_enable_thread_running) {
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_cancel_connect,NULL,"Connecting...");
			}

            else {
            	menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_enable_disable,menu_zeng_enable_disable_cond,"[%c] ~~Enabled",(zeng_enabled.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_common,'e');
			}

			char string_host_shown[16]; 
			menu_tape_settings_trunc_name(zeng_remote_hostname,string_host_shown,16);
			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_host,menu_zeng_host_cond,"~~Host [%s]",string_host_shown);
			menu_add_item_menu_shortcut(array_menu_common,'h');


			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_port,menu_zeng_host_cond,"[%d] Remote Port",zeng_remote_port);
			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_master,menu_zeng_host_cond,"[%c] ~~Master",(zeng_i_am_master ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_common,'m');

			if (zeng_i_am_master) {
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_snapshot_seconds,NULL,"[%d] Snapshot seconds",zeng_segundos_cada_snapshot);
			}

			if (zeng_enabled.v) {
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng_send_message,menu_zeng_send_message_cond,"Send message");
			}
                       
						
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu(&zeng_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZENG" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

int menu_online_zx81_letra(char filtro,char letra)
{
	letra=letra_minuscula(letra);
	filtro=letra_minuscula(filtro);
	if (filtro>='a' && filtro<='z') {
		if (letra==filtro) return 1;
		else return 0;
	}
	else {
		//todo lo que no son letras
		if (letra<'a' || letra>'z') return 1;
		else return 0;
	}
}

char online_browse_zx81_ultima_letra='a';

char menu_online_browse_zx81_letter(void)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;
	
	int ancho_ventana=23;
	int alto_ventana=8;
	
	int xventana=menu_center_x()-ancho_ventana/2; 
	int yventana=menu_center_y()-alto_ventana/2; 
	
	char letra_seleccionada=0;

	zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Initial letter");
	zxvision_draw_window(&ventana);		

       
        menu_item *array_menu_osd_adventure_keyboard;
        menu_item item_seleccionado;
        int retorno_menu;
        int salir=0;
        do {

		


        //Como no sabemos cual sera el item inicial, metemos este sin asignar
        	menu_add_item_menu_inicial(&array_menu_osd_adventure_keyboard,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	//if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
	//osd_adv_kbd_defined
		//int i;
		int last_x=4;
		int last_y=0;
		char letra='a';
      int nletra=0;
		
		
		for (;letra<='z'+1;letra++) {
			char letra_mostrar=letra;
			if (letra=='z'+1) letra_mostrar='#';
		menu_add_item_menu_format(array_menu_osd_adventure_keyboard,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard_action,NULL,"%c",letra_mostrar);
        		    menu_add_item_menu_tabulado(array_menu_osd_adventure_keyboard,last_x,last_y);
					menu_add_item_menu_valor_opcion(array_menu_osd_adventure_keyboard,letra_mostrar);
				
menu_add_item_menu_shortcut(array_menu_osd_adventure_keyboard,letra_mostrar);

			last_x +=3;
        nletra++;
			if (nletra==5) {
				last_x=4;
				last_y++; 
           nletra=0;
			}
		}



		//Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu(&online_browse_zx81_letter_opcion_seleccionada,&item_seleccionado,array_menu_osd_adventure_keyboard,"Initial letter" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
				//printf ("Item seleccionado: %d\n",item_seleccionado.valor_opcion);
                                //printf ("actuamos por funcion\n");

	                        

                                letra_seleccionada=item_seleccionado.valor_opcion;
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
				salir=1;
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir);



        cls_menu_overlay();
		

		//printf ("en final de funcion\n");
		zxvision_destroy_window(&ventana);

	return letra_seleccionada;


}

void menu_online_browse_zx81_create_menu(char *mem, char *mem_after_headers,int total_leidos,char letra,char *juego,char *url_juego)
{

	//Por defecto
	url_juego[0]=0;
	juego[0]=0;

	//Dado que es una variable local, siempre podemos usar este nombre array_menu_common
    menu_item *array_menu_common;
    menu_item item_seleccionado;
    int retorno_menu;

	menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	//temp limite
	char texto_final[30000];

	int total_items=0;
		
			int indice_destino=0;
		
			int dif_header=mem_after_headers-mem;
			total_leidos -=dif_header;
			mem=mem_after_headers;
	
			//leer linea a linea 
			char buffer_linea[1024];
			int i=0;
			int salir=0;
			do {
				int leidos;
				char *next_mem;
		
				next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
				total_leidos -=leidos;
		
				if (buffer_linea[0]==0) {
					salir=1;
					//printf ("salir con linea vacia final\n");
					mem=next_mem;
				}
				else {
					//printf ("cabecera %d: %s\n",i,buffer_linea);
					//ver si contine texto de juego
				
					char *existe;
					existe=strstr(buffer_linea,"/files/");
					if (existe!=NULL) {
						if (menu_online_zx81_letra(letra,existe[7])) {
						//if (existe[7]==letra) {
							//quitar desde comilla derecha
							char *comilla;
							comilla=strstr(&existe[7],"\"");
							if (comilla!=NULL) *comilla=0;
							debug_printf (VERBOSE_PARANOID,"Adding raw html line %s",buffer_linea);
							//Todo controlar maximo buffer y maximo que puede mostrar ventana
							sprintf(&texto_final[indice_destino],"%s\n",&existe[7]);
							indice_destino +=strlen(&existe[7])+1;

							menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,&existe[7]);
							debug_printf (VERBOSE_DEBUG,"Adding menu entry %s",&existe[7]);
							total_items++;
						}
					}
					i++;
					mem=next_mem;
				}

				if (total_leidos<=0) salir=1;
		
			} while (!salir);
	
			texto_final[indice_destino]=0;
			
	                      
						
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

			if (total_items) {
				//Si hay resultados con esa letra, normalmente si..
            	retorno_menu=menu_dibuja_menu(&zx81_online_browser_opcion_seleccionada,&item_seleccionado,array_menu_common,"ZX81 Games" );

                
            	if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                	//que juego se ha seleccionado

                	//item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                	//char *juego;
                	strcpy(juego,item_seleccionado.texto_opcion);
                	debug_printf (VERBOSE_INFO,"Selected game: %s",juego);
                
                	sprintf(url_juego,"/files/%s",juego);
				}
			}

			else {
				menu_error_message("No results found");
			}


}

void menu_online_browse_zx81(MENU_ITEM_PARAMETERS)
{


	do {
		//char oldletra=s_online_browse_zx81_letra[0];
	
		//menu_ventana_scanf("Letter",s_online_browse_zx81_letra,2);
	
		//char letra=s_online_browse_zx81_letra[0];
	
		char letra=menu_online_browse_zx81_letter();
		if (letra==0) return;
		
	    stats_total_zx81_browser_queries++;

		//printf ("old letra %c new letra %c\n",online_browse_zx81_ultima_letra,letra);
	
		//si cambia letra, poner cursor arriba
		if (letra!=online_browse_zx81_ultima_letra) zx81_online_browser_opcion_seleccionada=0;

		online_browse_zx81_ultima_letra=letra;
		

		int http_code;
		char *mem;
		char *orig_mem;
		char *mem_after_headers;
		int total_leidos;
		char redirect_url[NETWORK_MAX_URL];
		//int retorno=zsock_http("www.zx81.nl","/files.html",&http_code,&mem,&total_leidos,&mem_after_headers,1,"",0,redirect_url);

		int retorno=menu_zsock_http("www.zx81.nl","/files.html",&http_code,&mem,&total_leidos,&mem_after_headers,1,"",0,redirect_url);
		orig_mem=mem;
	
		//printf("%s\n",mem);

		if (mem_after_headers!=NULL && http_code==200) {
				char url_juego[1024];
				char juego[MAX_TEXTO_OPCION];
			
				menu_online_browse_zx81_create_menu(mem, mem_after_headers,total_leidos,letra,juego,url_juego);

				if (url_juego[0]!=0) {

                	//cargar
                	char archivo_temp[PATH_MAX];
					//sprintf (archivo_temp,"/tmp/%s",juego);
					sprintf (archivo_temp,"%s/%s",get_tmpdir_base(),juego);
		
                	//int ret=util_download_file("www.zx81.nl",url_juego,archivo_temp,0);
					//usamos misma funcion thread de download wos
					int ret=menu_download_wos("www.zx81.nl",url_juego,archivo_temp,0); 

					if (ret==200) {
                                
  						//y cargar
  						strcpy(quickload_file,archivo_temp);
 
						quickfile=quickload_file;
        
						if (quickload(quickload_file)) {
							debug_printf (VERBOSE_ERR,"Unknown file format");
						}

						//Agregar a ultimos archivos usados
						last_filesused_insert(quickload_file);

						//Y salir todos menus
						salir_todos_menus=1;  
					}
			
  					
					else {		
						//debug_printf(VERBOSE_ERR,"Error downloading game. Return code: %d",ret);

						if (ret<0) {	
							//debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",http_code);
							//printf ("Error: %d %s\n",retorno,z_sock_get_error(retorno));
							menu_network_error(ret);
						}
						else {
							debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",ret);
						}

					}	
				}                      
                        
		}
		//Fin resultado http correcto
		else {	
			if (retorno<0) {	
				//debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",http_code);
				debug_printf (VERBOSE_DEBUG,"Error: %d %s",retorno,z_sock_get_error(retorno));
				menu_network_error(retorno);
			}
			else {
				debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",http_code);
			}
		}			
		

		if (orig_mem!=NULL) free(orig_mem);

	} while (!salir_todos_menus);
	//Se saldra al seleccionar juego o al pulsar ESC desde seleccion letra (ahi se sale con return tal cual)
	
}




struct menu_zsock_http_struct
{

	char *host;
	char *url;
	int *http_code;
	char **mem;
	int *t_leidos;
	char **mem_after_headers;
	int skip_headers;
	char *add_headers;
	int use_ssl;
	char *redirect_url;


	int return_code;
    
};

int menu_zsock_http_thread_running=0;

int menu_menu_zsock_http_cond(zxvision_window *w GCC_UNUSED)
{
	return !menu_zsock_http_thread_running;
}



void *menu_menu_zsock_http_thread_function(void *entrada)
{

	//menu_zsock_http_thread_running=1; 

#ifdef USE_PTHREADS

	debug_printf (VERBOSE_DEBUG,"Starting zsock http thread. Host=%s Url=%s",
								((struct menu_zsock_http_struct *)entrada)->host,
								((struct menu_zsock_http_struct *)entrada)->url);

//((struct menu_zsock_http_struct *)entrada)->return_code=-1;


	((struct menu_zsock_http_struct *)entrada)->return_code=
			zsock_http( 
								((struct menu_zsock_http_struct *)entrada)->host,
								((struct menu_zsock_http_struct *)entrada)->url,
								((struct menu_zsock_http_struct *)entrada)->http_code,
								((struct menu_zsock_http_struct *)entrada)->mem,
								((struct menu_zsock_http_struct *)entrada)->t_leidos,
								((struct menu_zsock_http_struct *)entrada)->mem_after_headers,
								((struct menu_zsock_http_struct *)entrada)->skip_headers,
								((struct menu_zsock_http_struct *)entrada)->add_headers,
								((struct menu_zsock_http_struct *)entrada)->use_ssl,
								((struct menu_zsock_http_struct *)entrada)->redirect_url
							); 



	debug_printf (VERBOSE_DEBUG,"Finishing zsock http thread");

#endif
	menu_zsock_http_thread_running=0;

	return 0;

}

#ifdef USE_PTHREADS
pthread_t menu_zsock_http_thread;
#endif

int menu_zsock_http(char *host, char *url,int *http_code,char **mem,int *t_leidos, char **mem_after_headers,int skip_headers,char *add_headers,int use_ssl,char *redirect_url)
{
	
	//int ret=util_download_file(host_final,url_juego_final,archivo_temp,ssl_use); 
	//Lanzar el thread de descarga
	struct menu_zsock_http_struct parametros;

	parametros.host=host;
	parametros.url=url;
	parametros.http_code=http_code;
	parametros.mem=mem;
	parametros.t_leidos=t_leidos;
	parametros.mem_after_headers=mem_after_headers;
	parametros.skip_headers=skip_headers;
	parametros.add_headers=add_headers;
	parametros.use_ssl=use_ssl;
	parametros.redirect_url=redirect_url;

	//de momento not found y error, y mem a null
	parametros.return_code=-1;
	*(parametros.http_code)=404;
	*(parametros.mem)=NULL;
	*(parametros.mem_after_headers)=NULL;
	*(parametros.t_leidos)=0;
	parametros.redirect_url[0]=0;	


#ifdef USE_PTHREADS

	//Inicializar thread
	debug_printf (VERBOSE_DEBUG,"Initializing thread menu_menu_zsock_http_thread_function");
	

	//Antes de lanzarlo, decir que se ejecuta, por si el usuario le da enter rapido a la ventana de progreso y el thread aun no se ha lanzado
	menu_zsock_http_thread_running=1;
	
	if (pthread_create( &menu_zsock_http_thread, NULL, &menu_menu_zsock_http_thread_function, (void *)&parametros) ) {
		debug_printf(VERBOSE_ERR,"Can not create zsock http thread");
		return -1;
	}

#endif

		 
	contador_menu_zeng_connect_print=0;

	//Usamos misma ventana de progreso que zeng. TODO: si se lanzan los dos a la vez (cosa poco probable) se moverian uno con el otro
	zxvision_simple_progress_window("Downloading", menu_menu_zsock_http_cond,menu_zeng_connect_print );

	//TODO Si antes de finalizar la descarga se vuelve atras y se vuelve a realizar otra busqueda, puede dar problemas
	//ya que la variable menu_zsock_http_thread_running es global y única
	
	if (menu_zsock_http_thread_running) menu_warn_message("Download has not ended yet");

	//despues de mostrar el aviso, si la tarea sigue en ejecucion, retornamos error 404
	if (menu_zsock_http_thread_running) return 404;	

	return parametros.return_code;

}


struct download_wos_struct
{
	char *host;
	char *url;
	char *archivo_temp;
	int ssl_use;
	int return_code;
};

int download_wos_thread_running=0;

int menu_download_wos_cond(zxvision_window *w GCC_UNUSED)
{
	return !download_wos_thread_running;
}



void *menu_download_wos_thread_function(void *entrada)
{

	//download_wos_thread_running=1; 

#ifdef USE_PTHREADS

	debug_printf (VERBOSE_DEBUG,"Starting download content thread. Host=%s Url=%s",
	((struct download_wos_struct *)entrada)->host,
								((struct download_wos_struct *)entrada)->url);

	((struct download_wos_struct *)entrada)->return_code=util_download_file( ((struct download_wos_struct *)entrada)->host,
								((struct download_wos_struct *)entrada)->url,
								((struct download_wos_struct *)entrada)->archivo_temp,
								((struct download_wos_struct *)entrada)->ssl_use); 

	debug_printf (VERBOSE_DEBUG,"Finishing download content thread");

#endif
	download_wos_thread_running=0;

	return 0;

}

#ifdef USE_PTHREADS
pthread_t download_wos_thread;
#endif






int menu_download_wos(char *host,char *url,char *archivo_temp,int ssl_use)
{
	
	//int ret=util_download_file(host_final,url_juego_final,archivo_temp,ssl_use); 
	//Lanzar el thread de descarga
	struct download_wos_struct parametros;

	parametros.host=host;
	parametros.url=url;
	parametros.archivo_temp=archivo_temp;
	parametros.ssl_use=ssl_use;

	//de momento not found
	parametros.return_code=404;


#ifdef USE_PTHREADS

	//Inicializar thread

	//Antes de lanzarlo, decir que se ejecuta, por si el usuario le da enter rapido a la ventana de progreso y el thread aun no se ha lanzado
	download_wos_thread_running=1;	
	
	if (pthread_create( &download_wos_thread, NULL, &menu_download_wos_thread_function, (void *)&parametros) ) {
		debug_printf(VERBOSE_ERR,"Can not create download wos thread");
		return -1;
	}

#endif

		 
	contador_menu_zeng_connect_print=0;

	//Usamos misma ventana de progreso que zeng. TODO: si se lanzan los dos a la vez (cosa poco probable) se moverian uno con el otro
	zxvision_simple_progress_window("Downloading software", menu_download_wos_cond,menu_zeng_connect_print );

	//TODO Si antes de finalizar la descarga se vuelve atras y se vuelve a realizar otra busqueda, puede dar problemas
	//ya que la variable download_wos_thread_running es global y única
	
	if (download_wos_thread_running) menu_warn_message("Download has not ended yet");

	//despues de mostrar el aviso, si la tarea sigue en ejecucion, retornamos error 404
	if (download_wos_thread_running) return 404;

	return parametros.return_code;

}


//showindex dice si muestra contenido texto variable index en el item->usado para mostrar el archivo de la url en las diferentes descargas de un mismo juego
void menu_online_browse_zxinfowos_query(char *query_result,char *hostname,char *query_url,char *preffix,char *string_index,char *string_display,
     char *add_headers,int showindex,char *windowtitle,char *error_not_found_message)
{
	
	//Por defecto
	query_result[0]=0;
	
	//Dado que es una variable local, siempre podemos usar este nombre array_menu_common
	menu_item *array_menu_common;
	menu_item item_seleccionado;
	int retorno_menu;

	int zxinfo_wos_opcion_seleccionada=0;
	do {

		menu_add_item_menu_inicial(&array_menu_common,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

		//http://www.zx81.nl/files.html
		int http_code;
		char *mem;
		char *orig_mem;
		char *mem_after_headers;
		int total_leidos;

		
		char redirect_url[NETWORK_MAX_URL];
		//int retorno=zsock_http(hostname,query_url,&http_code,&mem,&total_leidos,&mem_after_headers,1,add_headers,0,redirect_url);
		
		int retorno=menu_zsock_http(hostname,query_url,&http_code,&mem,&total_leidos,&mem_after_headers,1,add_headers,0,redirect_url);



		orig_mem=mem;
	
		if (mem_after_headers!=NULL && http_code==200) {
			
			int dif_header=mem_after_headers-mem;
			total_leidos -=dif_header;
			mem=mem_after_headers;
		
			//leer linea a linea 
			char buffer_linea[1024];
			int i=0;
			int salir=0;

			int existe_id;
			int existe_fulltitle;
			int ultimo_indice_id;
			int ultimo_indice_fulltitle;

			char ultimo_id[1024];
			char ultimo_fulltitle[1024];

			existe_id=0;
			existe_fulltitle=0;
			ultimo_id[0]=0;
			ultimo_fulltitle[0]=0;
			ultimo_indice_id=0;
			ultimo_indice_fulltitle=0;	

			int total_items=0;

			//Leer linea a linea la respuesta http, y meterlo en items de menu
			do {
				int leidos;
				char *next_mem;
			
				next_mem=util_read_line(mem,buffer_linea,total_leidos,1024,&leidos);
				total_leidos -=leidos;
			
				if (buffer_linea[0]==0) {
					salir=1;
					//printf ("salir con linea vacia final\n");
					mem=next_mem;
				}
				else {
					//printf ("cabecera %d: %s\n",i,buffer_linea);
					//ver si contine texto de juego

					/*
					Campos a buscar:

	hits.0._id=0002258
	hits.0.fulltitle=Headcoach

	Pueden salir antes id o antes fulltitle. En bucle leer los dos y cuando estén los dos y tengan mismo .n., agregar a menu
					*/
					
					//filtrar antes los que tienen prefijo
					char *existe_prefijo;
					
					existe_prefijo=strstr(buffer_linea,preffix);
					if (existe_prefijo!=NULL) {
					
						char *existe;
						existe=strstr(buffer_linea,string_index); //"_id=");
						if (existe!=NULL) {
								int pos=strlen(string_index);
								strcpy(ultimo_id,&existe[pos]);
								existe_id=1;
								char *existe_indice;
								existe_indice=strstr(buffer_linea,preffix);
								if (existe_indice!=NULL) {
									//saltar el prefijo para obtener el numero
									int l=strlen(preffix);
									ultimo_indice_id=parse_string_to_number(&existe_indice[l]);
								}
						}

						existe=strstr(buffer_linea,string_display); //"fulltitle=");
						if (existe!=NULL) {
								int pos=strlen(string_display);
								strcpy(ultimo_fulltitle,&existe[pos]);
								existe_fulltitle=1;
								char *existe_indice;
								existe_indice=strstr(buffer_linea,preffix);
								if (existe_indice!=NULL) {
									//saltar el prefijo para obtener el numero
									int l=strlen(preffix);
									ultimo_indice_fulltitle=parse_string_to_number(&existe_indice[l]);
								}						
						}				
							
						if (existe_id && existe_fulltitle) {
							if (ultimo_indice_id==ultimo_indice_fulltitle) {
								
								
								debug_printf (VERBOSE_DEBUG,"Adding menu item [%s] id [%s]",ultimo_fulltitle,ultimo_id);
								
								//meter en entrada linea indice. Realmente para que la queremos?
								//solo la muestro en la busqueda inicial, en la seleccion del formato de archivo ya no
								if (!showindex) {
									//Remodificamos ultimo_fulltitle para meterle el indice delante
									char buf[1024];
									sprintf (buf,"%d %s",ultimo_indice_id,ultimo_fulltitle);
									strcpy(ultimo_fulltitle,buf);
								}
								
								
								//controlar maximo 30 caracteres
								//TODO: si hacemos que se guarde geometria de ventana, teniendo ancho mayor que 32, esta maximo podria ser el ancho 
								//maximo que permite un item de menu (MAX_TEXTO_OPCION)
								ultimo_fulltitle[30]=0;
								
								
								//TODO controlar maximo items en menu. De momento esta limitado por la query a la api (100)
								//Porque? realmente no hay un limite como tal en items de menu, no?

								if (!showindex) {
									menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,ultimo_fulltitle);
									menu_add_item_menu_misc(array_menu_common,ultimo_id);
								}
								else {
									//printf ("%s\n",ultimo_id);
									//obtener archivo sin extension de la descarga
									char nombre_sin_dir[PATH_MAX];
									char nombre_sin_ext[PATH_MAX];

									util_get_file_no_directory(ultimo_id,nombre_sin_dir);

									//TODO Pillamos el nombre sin extension (sin puntos), pero en juegos como "Chase H.Q.tap.zip"
									//quedará: "Chase H"
									util_get_file_without_extension(nombre_sin_dir,nombre_sin_ext);

									//Acortar el nombre por si acaso
									char nombre_shown[28];

									//strcpy(nombre_sin_ext,"01234567890123456789012345678901234567890123456789");

									menu_tape_settings_trunc_name(nombre_sin_ext,nombre_shown,28);
									//printf ("%s\n",nombre_sin_ext);

									menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,NULL,NULL,nombre_shown);
									menu_add_item_menu_misc(array_menu_common,ultimo_id);

									menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL," %s",ultimo_fulltitle);							
								}

								total_items++;
							}

							existe_id=0;
							existe_fulltitle=0;
							ultimo_id[0]=0;
							ultimo_fulltitle[0]=0;
							ultimo_indice_id=0;
							ultimo_indice_fulltitle=0;	

						}
					}
												
					i++;
					mem=next_mem;
				}
			
				if (total_leidos<=0) salir=1;
			
			} while (!salir);
		
			//texto_final[indice_destino]=0;
			if (orig_mem!=NULL) free(orig_mem);
				
						
							
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_ESC_item(array_menu_common);

			if (total_items) {

				retorno_menu=menu_dibuja_menu(&zxinfo_wos_opcion_seleccionada,&item_seleccionado,array_menu_common,windowtitle );

				
				if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

					//printf ("actuamos por funcion\n");
					//item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
					char *juego;
					juego=item_seleccionado.texto_opcion;

					char *url;
					url=item_seleccionado.texto_misc;
					debug_printf (VERBOSE_INFO,"Game [%s] url/id [%s]",juego,url);

					strcpy(query_result,url);
					return;
										
				}
			}

			else {
				//menu_error_message("No results found");
				menu_error_message(error_not_found_message);
				return;
			}
		}  //Aqui cierra mem_after_headers!=NULL && http_code==200

			//Fin resultado http correcto
		else {	
			if (retorno<0) {	
				//debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",http_code);
				printf ("Error: %d %s\n",retorno,z_sock_get_error(retorno));
				menu_network_error(retorno);
				return;
			}
			else {
				debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",http_code);
				return;
			}
		}	
	

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

	

}

void menu_zxinfo_get_final_url(char *url_orig,char *host_final,char *url_final,int *ssl_use)
{
	    /*  Local file links starting with /zxdb/sinclair/ refer to content added afterwards. 
		These files are currently stored at https://spectrumcomputing.co.uk/zxdb/sinclair/  */

		/*
		Local file links starting with /pub/sinclair/ refer to content previously available at the original WorldOfSpectrum archive. 
		These files are currently accessible from Archive.org mirror at 
		https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair/
		Local file links starting with /zxdb/sinclair/ refer to content added afterwards. 
		These files are currently stored at https://spectrumcomputing.co.uk/zxdb/sinclair/


		https://github.com/zxdb/ZXDB/blob/master/README.md
		*/

		
#ifdef COMPILE_SSL
		*ssl_use=1;
		char *pref_wos="/pub/sinclair/";
		//char *pref_zxdb="/zxdb/sinclair/";

		if (strstr(url_orig,pref_wos)!=NULL) {
			debug_printf (VERBOSE_DEBUG,"WOS preffix");

			//Quitar /pub/sinclair
			char url_modif[NETWORK_MAX_URL];
			strcpy(url_modif,url_orig);

			int longitud_pref=strlen(pref_wos);
			//int longitud_url=strlen(url_orig);

			//longitud_url -=longitud_pref;
			//url_modif[longitud_url]=0;

			char *puntero_url;
			puntero_url=&url_modif[longitud_pref];

			//printf ("url modificada primero: %s\n",puntero_url);

			strcpy(host_final,"archive.org");
			sprintf(url_final,"/download/World_of_Spectrum_June_2017_Mirror/World%%20of%%20Spectrum%%20June%%202017%%20Mirror.zip/World%%20of%%20Spectrum%%20June%%202017%%20Mirror/sinclair/%s",puntero_url);
			debug_printf (VERBOSE_DEBUG,"Final URL: %s",url_final);

		}

		else {
			debug_printf (VERBOSE_DEBUG,"Spectrumcomputing preffix");
			//Asumimos que es zxdb
			strcpy(host_final,"spectrumcomputing.co.uk");
			strcpy(url_final,url_orig);
		}
#else
		//Si no tenemos ssl, solo podemos descargar contenido de wos tal cual
		debug_printf (VERBOSE_DEBUG,"Trying to download from WOS using HTTP as we don't have SSL support compiled in");
		*ssl_use=0;
		strcpy(host_final,"www.worldofspectrum.org");
		strcpy(url_final,url_orig);
#endif
		
		
	
}

char zxinfowos_query_search[256]="";

void menu_online_browse_zxinfowos(MENU_ITEM_PARAMETERS)
{

#ifndef COMPILE_SSL
	menu_first_aid("no_ssl_wos");	
#endif
	
	menu_ventana_scanf("Query",zxinfowos_query_search,256);
	if (zxinfowos_query_search[0]==0) return;
	
    stats_total_speccy_browser_queries++;
	
	
	
	//TODO podria pasar que al normalizar ocupe mas de 1024, pero la cadena de entrada tendria que ser muy grande
	char query_search_normalized[1024];
	
	util_normalize_query_http(zxinfowos_query_search,query_search_normalized);
	

	//http://a.zxinfo.dk/api/zxinfo/v2/search?query=head%20over%20heels&mode=compact&sort=rel_desc&size=10&offset=0

	do {
		char query_url[1024];
		//sprintf (query_url,"/api/zxinfo/v2/search?query=%s&mode=compact&sort=rel_desc&size=100&offset=0&contenttype=SOFTWARE&availability=Available",query_search_normalized);
		sprintf (query_url,"/api/zxinfo/v2/search?query=%s&mode=compact&sort=rel_desc&size=100&offset=0&contenttype=SOFTWARE",query_search_normalized);

		char query_id[256];
		menu_online_browse_zxinfowos_query(query_id,"a.zxinfo.dk",query_url,"hits.","_id=","fulltitle=","",0,"Spectrum games","No results found");
		//gestionar resultado vacio
		if (query_id[0]==0) {
			//TODO resultado con ESC
			return;
		}

		debug_printf (VERBOSE_DEBUG,"Entry id result: %s",query_id);
		
		
		//http://a.zxinfo.dk/api/zxinfo/games/0002259?mode=compact
		
		/*
		releases.1.as_title=Foot and Mouth
	releases.1.releaseprice=£7.95
	releases.1.url=/pub/sinclair/games/h/HeadOverHeels.tap.zip
	releases.1.type=Tape image
		*/
		
		sprintf (query_url,"/api/zxinfo/games/%s?mode=compact",query_id);

		
		menu_online_browse_zxinfowos_query(query_id,"a.zxinfo.dk",query_url,"releases.","url=","format=","",1,"Releases",
											"No results found. Maybe there are no releases available or the game is copyright protected");

		//gestionar resultado vacio
		if (query_id[0]==0) {
			//TODO resultado con ESC
			return;
		}	
		
		//gestionar resultado no vacio
		if (query_id[0]!=0) {
			// resultado no ESC
			

			debug_printf (VERBOSE_DEBUG,"Entry url result: %s",query_id);
		
			char url_juego[1024];
			sprintf(url_juego,"%s",query_id);
			//cargar
			char archivo_temp[PATH_MAX];
									
									
			/* Local file links starting with /zxdb/sinclair/ refer to content added afterwards. 
			These files are currently stored at https://spectrumcomputing.co.uk/zxdb/sinclair/  */

			/*
			Local file links starting with /pub/sinclair/ refer to content previously available at the original WorldOfSpectrum archive. 
			These files are currently accessible from Archive.org mirror at 
			https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/sinclair/
			Local file links starting with /zxdb/sinclair/ refer to content added afterwards. 
			These files are currently stored at https://spectrumcomputing.co.uk/zxdb/sinclair/


			https://github.com/zxdb/ZXDB/blob/master/README.md
			*/
			
			
			char juego[PATH_MAX];
			util_get_file_no_directory(query_id,juego);
			util_normalize_name(juego);
			
			char tempdir[PATH_MAX];
			sprintf (tempdir,"%s/download",get_tmpdir_base() );
			menu_filesel_mkdir(tempdir);
			sprintf (archivo_temp,"%s/%s",tempdir,juego);
			

			char url_juego_final[PATH_MAX];
			char host_final[PATH_MAX];

			int ssl_use;

			menu_zxinfo_get_final_url(url_juego,host_final,url_juego_final,&ssl_use);

			debug_printf (VERBOSE_DEBUG,"Downloading file from host %s (SSL=%d) url %s",host_final,ssl_use,url_juego_final);

			int ret=menu_download_wos(host_final,url_juego_final,archivo_temp,ssl_use); 

			if (ret==200) {                    
				//y habrimos menu de smartload
				strcpy(quickload_file,archivo_temp);
	
				quickfile=quickload_file;
				menu_quickload(0);
		
				return;
			}
			else {
				if (ret<0) {	
					menu_network_error(ret);
				}
				else {
					debug_printf(VERBOSE_ERR,"Error downloading game list. Return code: %d",ret);
				}

			}
		} 
	} while (1);
}




void menu_network_http_request(MENU_ITEM_PARAMETERS)
{
	int http_code;
	char *mem;
	char *mem_after_headers;
	char host[100];
	char url[100];
	char s_skip_headers[2];
	char s_add_headers[200];

	host[0]=0;
	url[0]=0;

	strcpy(s_skip_headers,"0");
	//s_skip_headers[0]='0';
	s_add_headers[0]=0;
	
	menu_ventana_scanf("host?",host,100);
	menu_ventana_scanf("url?",url,100);
	menu_ventana_scanf("add headers",s_add_headers,200);
	
	int l=strlen(s_add_headers);
	if (l>0) {
		s_add_headers[l++]='\r';
		s_add_headers[l++]='\n';
		s_add_headers[l++]=0;

	}
	
	menu_ventana_scanf("skip ret headers?(0/1)",s_skip_headers,2);
	int skip_headers=parse_string_to_number(s_skip_headers);
	int total_leidos;
	char redirect_url[NETWORK_MAX_URL];

	int use_ssl=0;

#ifdef COMPILE_SSL
	char s_use_ssl[2];
	strcpy(s_use_ssl,"0");
	menu_ventana_scanf("use ssl? (0/1)",s_use_ssl,2);
	use_ssl=parse_string_to_number(s_use_ssl);
#endif

	//int retorno=zsock_http(host,url,&http_code,&mem,&total_leidos,&mem_after_headers,skip_headers,s_add_headers,0,redirect_url);

	char *mem_mensaje;

	int retorno=menu_zsock_http(host,url,&http_code,&mem,&total_leidos,&mem_after_headers,skip_headers,s_add_headers,use_ssl,redirect_url);
	if (retorno==0 && mem!=NULL) {
		if (skip_headers) {
			if (mem_after_headers) {
				menu_generic_message_format("Http code","%d",http_code);
				mem_mensaje=mem_after_headers;
				//menu_generic_message("Response",mem_after_headers);
			}
		}
		else {
			mem_mensaje=mem;
			//menu_generic_message("Response",mem);
		}
	}

	if (retorno>=0) {

		//Controlar maximo mensaje

		int longitud_mensaje=strlen(mem_mensaje);

		int max_longitud=MAX_TEXTO_GENERIC_MESSAGE-1024;
		//Asumimos el maximo restando 1024, de los posibles altos de linea

		if (longitud_mensaje>max_longitud) {
			//TODO: realmente habria que trocear aqui el mensaje en lineas y ver si el resultado excede el maximo de lineas o el maximo de bytes
			debug_printf (VERBOSE_ERR,"Response too long. Showing only the first %d bytes",max_longitud);
			mem_mensaje[max_longitud]=0;
		}

		menu_generic_message("Response",mem_mensaje);

	}

	else {
		menu_network_error(retorno);
	}



	if (mem!=NULL) free (mem);
}


void menu_network(MENU_ITEM_PARAMETERS)
{
        //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                
            menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_uartbridge,menu_network_uartbridge_cond,"~~UART Bridge emulation");
			menu_add_item_menu_shortcut(array_menu_common,'u');
                        
			menu_add_item_menu_tooltip(array_menu_common,"Bridge from emulated machine uart ports to a local serial uart device");
			menu_add_item_menu_ayuda(array_menu_common,"Bridge from emulated machine uart ports to a local serial uart device\n"
				"It does NOT emulate a full uart device, just links from the emulated machine ports to a physical local device\n"
				"Available for ZX-Uno, TBBlue and ZX Evolution TSConf");
			

#ifdef USE_PTHREADS
			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_zeng,NULL,"Z~~ENG");
			menu_add_item_menu_shortcut(array_menu_common,'e');
			menu_add_item_menu_tooltip(array_menu_common,"Setup ZEsarUX Network Gaming");
			menu_add_item_menu_ayuda(array_menu_common,"ZEsarUX Network Gaming protocol (ZENG) allows you to play to any emulated game, using two ZEsarUX instances, "
			  "located each one on any part of the world or in a local network.\n"
			  "Games doesn't have to be modified, you can use any existing game. "
			  "ZENG works by sending special commands through the ZRCP protocol, so in order to use ZENG you must enable ZRCP protocol on menu settings-debug. "
			  "This protocol listens on tcp port 10000 so you should open your firewall/router to use it. "
			  "One ZEsarUX instance will be the master node and the other instance will be the slave node.\n"
			  "Please do NOT set both nodes as master\n"
			  "When you enable ZENG on both nodes:\n"
			  "-all key/joystick presses will be sent between the two nodes\n"
			  "-every two seconds a snapshot will be sent from the master to the slave node\n\n"
			  "Note about using joystick: real joystick (and cursors on keyboard) are sent to the other node as "
			  "the direction/button (left,right,up,down or fire) but not the type of joystick emulated (kempston, fuller, etc). "
			  "So you must configure same joystick emulation on both nodes. Also, real joystick to key configuration is not sent by ZENG"
			);

                     
             
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_online_browse_zx81,NULL,"~~ZX81 online browser"); 
			menu_add_item_menu_shortcut(array_menu_common,'z'); 
            menu_add_item_menu_tooltip(array_menu_common,"Connects to the www.zx81.nl site to download ZX81 games. Many thanks to ZXwebmaster for allowing it"); 
            menu_add_item_menu_ayuda(array_menu_common,"Connects to the www.zx81.nl site to download ZX81 games. Many thanks to ZXwebmaster for allowing it"); 


			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_online_browse_zxinfowos,NULL,"~~Speccy online browser"); 
			menu_add_item_menu_shortcut(array_menu_common,'s');  

#ifdef COMPILE_SSL
			//Versión con SSL usa zxinfo, spectrum computing y mirror archive.org
			menu_add_item_menu_tooltip(array_menu_common,"It uses zxinfo, spectrum computing and archive.org to download the software. Thanks to Thomas Heckmann and Peter Jones for allowing it");
			menu_add_item_menu_ayuda(array_menu_common,  "It uses zxinfo, spectrum computing and archive.org to download the software. Thanks to Thomas Heckmann and Peter Jones for allowing it");
#else
			//Versión sin SSL usa zxinfo, servidor WOS
			menu_add_item_menu_tooltip(array_menu_common,"It uses zxinfo and WOS to download the software. Thanks to Thomas Heckmann and Lee Fogarty for allowing it");
			menu_add_item_menu_ayuda(array_menu_common,  "It uses zxinfo and WOS to download the software. Thanks to Thomas Heckmann and Lee Fogarty for allowing it");
#endif    


			menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_network_http_request,NULL,"Test Http request"); 

//Fin de condicion si hay pthreads
#endif

						
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu(&network_opcion_seleccionada,&item_seleccionado,array_menu_common,"Network" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_settings_enable_statistics(MENU_ITEM_PARAMETERS)
{
	if (stats_enabled.v) stats_disable();
	else stats_enable();
}

void menu_settings_enable_check_updates(MENU_ITEM_PARAMETERS)
{
	stats_check_updates_enabled.v ^=1;	
}


void menu_settings_statistics(MENU_ITEM_PARAMETERS)
{
        //Dado que es una variable local, siempre podemos usar este nombre array_menu_common
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

			menu_add_item_menu_inicial_format(&array_menu_common,MENU_OPCION_NORMAL,menu_settings_enable_check_updates,NULL,"[%c] Check updates",
					(stats_check_updates_enabled.v ? 'X' : ' ') );


                
            menu_add_item_menu_format(array_menu_common,MENU_OPCION_NORMAL,menu_settings_enable_statistics,NULL,"[%c] Send Statistics",
					(stats_enabled.v ? 'X' : ' ') );
			
                        
			menu_add_item_menu_tooltip(array_menu_common,"Send anonymous statistics to a remote server, every time ZEsarUX starts");
			menu_add_item_menu_ayuda(array_menu_common,"Send anonymous statistics to a remote server, every time ZEsarUX starts");

			if (stats_enabled.v) {
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"The following data is sent:");
				//menu_add_item_menu_tooltip(array_menu_common,"This data is sent every time ZEsarUX starts");
				//menu_add_item_menu_ayuda(array_menu_common,"This data is sent every time ZEsarUX starts");
	
	menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Public IP address");
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    UUID: %s",stats_uuid);
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    System: %s",COMPILATION_SYSTEM);
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Minutes: %d",stats_get_current_total_minutes_use() );
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Speccy queries: %d",stats_total_speccy_browser_queries);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    ZX81 queries: %d",stats_total_zx81_browser_queries);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Emulator version: %s",EMULATOR_VERSION);
				
				menu_add_item_menu_format(array_menu_common,MENU_OPCION_SEPARADOR,NULL,NULL,"    Build Number: %s",BUILDNUMBER);
				

			}

              
						
			menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu(&settings_statistics_opcion_seleccionada,&item_seleccionado,array_menu_common,"Statistics Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}





