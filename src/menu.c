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
#include "settings.h"
#include "datagear.h"
#include "stats.h"
#include "network.h"



#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif

#include "compileoptions.h"

#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
//macro llama a funcion real
	#define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
	#define scrstdout_menu_print_speech_macro(x)
#endif


#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif





//
// Archivo para funciones auxiliares de soporte de menu, excluyendo entradas de menu
// Las entradas de menu estan en menu_items.c
// Aunque aun falta mucho por mover, la mayoria de entradas de menu siguen aqui y habria que moverlas al menu_items.c
//



//si se pulsa tecla mientras se lee el menu
int menu_speech_tecla_pulsada=0;

//indica que hay funcion activa de overlay o no
int menu_overlay_activo=0;

//indica si el menu hace zoom. valores validos: 1 en adelante
int menu_gui_zoom=1;

//Tamanyo del array de char asignado para el browser de file utils
//#define MAX_TEXTO_BROWSER 65536
//Debe ser algo menor que MAX_TEXTO_GENERIC_MESSAGE
#define MAX_TEXTO_BROWSER (MAX_TEXTO_GENERIC_MESSAGE-1024)


//Ancho de caracter de menu
int menu_char_width=8;

int menu_last_cpu_use=0;

defined_f_function defined_f_functions_array[MAX_F_FUNCTIONS]={
	{"Default",F_FUNCION_DEFAULT},
	{"Nothing",F_FUNCION_NOTHING},
	{"Reset",F_FUNCION_RESET},
	{"HardReset",F_FUNCION_HARDRESET},
	{"NMI",F_FUNCION_NMI},
	{"OpenMenu",F_FUNCION_OPENMENU},
	{"OCR",F_FUNCION_OCR},
	{"SmartLoad",F_FUNCION_SMARTLOAD},
	{"Quicksave",F_FUNCION_QUICKSAVE},
	{"LoadBinary",F_FUNCION_LOADBINARY},
	{"SaveBinary",F_FUNCION_SAVEBINARY},
	{"ZengMessage",F_FUNCION_ZENG_SENDMESSAGE},
	{"OSDKeyboard",F_FUNCION_OSDKEYBOARD},
	{"OSDTextKeyboard",F_FUNCION_OSDTEXTKEYBOARD},
	{"SwitchBorder",F_FUNCION_SWITCHBORDER},
	{"SwitchFullScr",F_FUNCION_SWITCHFULLSCREEN},
	{"ReloadMMC",F_FUNCION_RELOADMMC},
	{"ReinsertTape",F_FUNCION_REINSERTTAPE},
	{"DebugCPU",F_FUNCION_DEBUGCPU},
	{"Pause",F_FUNCION_PAUSE},
	{"ExitEmulator",F_FUNCION_EXITEMULATOR}
};

//Funciones de teclas F mapeadas. Desde F1 hasta F15
enum defined_f_function_ids defined_f_functions_keys_array[MAX_F_FUNCTIONS_KEYS]={
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT, //F5
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT, //F10
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT,
	F_FUNCION_DEFAULT //F15
};

//Si el abrir menu (tipica F5 o tecla joystick) esta limitado. De tal manera que para poderlo abrir habra que pulsar 3 veces seguidas en menos de 1 segundo
z80_bit menu_limit_menu_open={0};

//No mostrar subdirectorios en file selector
z80_bit menu_filesel_hide_dirs={0};


//OSD teclado aventura
/*
//numero maximo de entradas
#define MAX_OSD_ADV_KEYB_WORDS 40
//longitud maximo de cada entrada
#define MAX_OSD_ADV_KEYB_TEXT_LENGTH 20
*/


//3 entradas definidas de ejemplo
int osd_adv_kbd_defined=9;
char osd_adv_kbd_list[MAX_OSD_ADV_KEYB_WORDS][MAX_OSD_ADV_KEYB_TEXT_LENGTH]={
	"~~north",
	"~~west",
	"~~east",
	"~~south",
	"loo~~k",  //5
	"e~~xamine",
	"~~help",
	"~~talk",
	"ex~~it"
};


//Definir una tecla a una funcion
//Entrada: tecla: 1...15 F1...15   funcion: string correspondiente a defined_f_functions_array
//Devuelve 0 si ok
int menu_define_key_function(int tecla,char *funcion)
{
	if (tecla<1 || tecla>MAX_F_FUNCTIONS_KEYS) return 1;

	//Buscar en todos los strings de funciones cual es

	int i;

	for (i=0;i<MAX_F_FUNCTIONS;i++) {
		if (!strcasecmp(funcion,defined_f_functions_array[i].texto_funcion)) {
			enum defined_f_function_ids id=defined_f_functions_array[i].id_funcion;
			defined_f_functions_keys_array[tecla-1]=id;
			return 0;
		}
	}

	return 1;
}

//funcion activa de overlay
void (*menu_overlay_function)(void);

//buffer de escritura por pantalla
overlay_screen overlay_screen_array[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];

//buffer de escritura de footer
overlay_screen footer_screen_array[WINDOW_FOOTER_COLUMNS*WINDOW_FOOTER_LINES];


//buffer de texto usado 
int overlay_usado_screen_array[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];

//Indica que hay una segunda capa de texto por encima de menu y por encima del juego incluso
//util para mostrar indicadores de carga de cinta, por ejemplo
//int menu_second_layer=0;

//Footer activado por defecto
int menu_footer=1;

//se activa second layer solo para un tiempo limitado
//int menu_second_layer_counter=0;

//buffer de escritura de segunda capa
//overlay_screen second_overlay_screen_array[32*24];



//Si el menu esta desactivado completamente. Si es asi, cualquier evento que abra el menu, no hará nada
z80_bit menu_desactivado={0};

//Si el menu esta desactivado completamente. Si es asi, cualquier evento que abra el menu, provocará la salida del emulador
z80_bit menu_desactivado_andexit={0};

//Si el menu de file utilities esta deshabilitado
z80_bit menu_desactivado_file_utilities={0};

//indica que el menu aparece en modo multitarea - mientras ejecuta codigo de emulacion de cpu
int menu_multitarea=1;

//Si se oculta la barra vertical en la zona de porcentaje de ventanas de texto o selector de archivos
z80_bit menu_hide_vertical_percentaje_bar={0};

//Si se oculta boton de minimizar ventana
z80_bit menu_hide_minimize_button={0};

//Si se oculta boton de cerrar ventana
z80_bit menu_hide_close_button={0};

//Si se invierte sentido movimiento scroll raton
z80_bit menu_invert_mouse_scroll={0};

//indica que se ha pulsado ESC y por tanto debe aparecer el menu, o gestion de breakpoints, osd, etc
//y tambien, la lectura de puertos de teclado (254) no devuelve nada
int menu_abierto=0;

//Si realmente aparecera el menu
z80_bit menu_event_open_menu={0};

//indica si hay pendiente un mensaje de error por mostrar
int if_pending_error_message=0;

//mensaje de error pendiente a mostrar
char pending_error_message[1024];

//Indica que hay que salir de todos los menus. Esto sucede, por ejemplo, al cargar snapshot
int salir_todos_menus;

//char *welcome_message_key=" Press ESC for menu ";

char *esc_key_message="ESC";
char *openmenu_key_message="F5/Button";


//Gestionar pulsaciones directas de teclado o joystick
//para quickload
z80_bit menu_button_quickload={0};
//para on screen keyboard
z80_bit menu_button_osdkeyboard={0};
z80_bit menu_button_osdkeyboard_return={0};

//Retorno de envio de una tecla
z80_bit menu_button_osd_adv_keyboard_return={0};
//Abrir el menu de Adventure text
z80_bit menu_button_osd_adv_keyboard_openmenu={0};

//Comun para zx8081 y spectrum
//z80_bit menu_button_osdkeyboard_caps={0};
//Solo para spectrum
//z80_bit menu_button_osdkeyboard_symbol={0};
//Solo para zx81
//z80_bit menu_button_osdkeyboard_enter={0};

z80_bit menu_button_exit_emulator={0};

z80_bit menu_event_drag_drop={0};

z80_bit menu_event_new_version_show_changes={0};

z80_bit menu_event_new_update={0};

z80_bit menu_button_f_function={0};

//tecla f pulsada
int menu_button_f_function_index;
//accion de tecla f
int menu_button_f_function_action=0;

//char menu_event_drag_drop_file[PATH_MAX];

//Para evento de entrada en paso a paso desde remote protocol
z80_bit menu_event_remote_protocol_enterstep={0};


//Si menus de confirmacion asumen siempre yes y no preguntan nunca
z80_bit force_confirm_yes={0};

//Si raton no tiene accion sobre el menu
z80_bit mouse_menu_disabled={0};


z80_bit no_close_menu_after_smartload={0};


void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,z80_byte color);
void menu_desactiva_cuadrado(void);
void menu_establece_cuadrado(int x1,int y1,int x2,int y2,z80_byte color);


void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2);


void menu_espera_tecla_timeout_tooltip(void);
z80_byte menu_da_todas_teclas(void);



void menu_textspeech_send_text(char *texto);


int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2);

void menu_file_trd_browser_show(char *filename,char *tipo_imagen);
void menu_file_mmc_browser_show(char *filename,char *tipo_imagen);
void menu_file_viewer_read_file(char *title,char *file_name);
void menu_file_viewer_read_text_file(char *title,char *file_name);
void menu_file_dsk_browser_show(char *filename);



//si hay recuadro activo, y cuales son sus coordenadas y color

int cuadrado_activo=0;
int cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color;

//Y si dicho recuadro tiene marca de redimensionado posible para zxvision
int cuadrado_activo_resize=0;
int ventana_activa_tipo_zxvision=0;

//Si estamos dibujando las ventanas de debajo de la del frente, y por tanto no muestra boton de cerrar por ejemplo
int ventana_es_background=0;

int draw_bateria_contador=0;
int draw_cpu_use=0;
int draw_cpu_temp=0;
int draw_fps=0;

//Portapapeles del menu
z80_byte *menu_clipboard_pointer=NULL;

//tamanyo del portapapeles
int menu_clipboard_size=0;

//Si driver de video soporta lectura de teclas F
int f_functions;

//Si hay que escribir las letras de atajos de menu en inverso. Esto solo sucede cuando salta el tooltip o cuando se pulsa una tecla
//que no es atajo
z80_bit menu_writing_inverse_color={0};

//Si forzar letras en color inverso siempre
z80_bit menu_force_writing_inverse_color={0};

//siempre empieza con 1 espacio de separacion en menu_escribe_linea_opcion excepto algunas ventanas que requieren mas ancho, como hexdump
int menu_escribe_linea_startx=1;


//Si se desactiva parseo caracteres especiales como ~~ o ^^ etc
z80_bit menu_disable_special_chars={0};

//Colores franja speccy
int colores_franja_speccy_brillo[]={2+8,6+8,4+8,5+8};
int colores_franja_speccy_oscuro[]={2,6,4,5};

//Colores franja cpc. Ultima amarillo, porque son 3 barras y queremos que se confunda con el fondo
int colores_franja_cpc_brillo[]={2+8,4+8,1+8,6+8};
int colores_franja_cpc_oscuro[]={2,4,1,6+8};


int estilo_gui_activo=0;

estilos_gui definiciones_estilos_gui[ESTILOS_GUI]={
	{"ZEsarUX",7+8,0,
		0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
		5+8,0, 		//Colores para opcion seleccionada
		7+8,2,7,2, 	//Colores para opcion no disponible
		0,7+8,        	//Colores para el titulo y linea recuadro ventana
		7+8,0,        	//Colores para el titulo y linea recuadro ventana inactiva
		1,		//Color waveform
		7,		//Color para zona no usada en visualmem
		2,7+8,		//Color para opcion marcada
		'*',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
		},
	{"ZXSpectr",1,6,
		1,1,0,0,		//Mostrar cursor >, mostrar recuadro, no mostrar rainbow
		1+8,6,		//Colores para opcion seleccionada
		1,6,1,6,	//Colores para opcion no disponible, iguales que para opcion disponible
		6,1,		//Colores para el titulo y linea recuadro ventana
		1,6,		//Colores para el titulo y linea recuadro ventana inactiva
		6,		//Color waveform
		0,               //Color para zona no usada en visualmem
		2,7+8,		//Color para opcion marcada
		'*',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
		},

        {"ZX80/81",7+8,0,
                1,1,0,1,          //Mostrar cursor >, mostrar recuadro, no mostrar rainbow
                0,7+8,          //Colores para opcion seleccionada
                7+8,0,0,7+8,      //Colores para opcion no disponible
                0,7+8,          //Colores para el titulo y linea recuadro ventana
				7+8,0,          //Colores para el titulo y linea recuadro ventana inactiva
                0,              //Color waveform
                7,               //Color para zona no usada en visualmem
                7,0,		//Color para opcion marcada
		'.',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
                },

//Lo ideal en Z88 seria mismos colores que Z88... Pero habria que revisar para otros drivers, tal como curses o cacalib
//que no tienen esos colores en las fuentes
//Al menos hacemos colores sin brillo
        {"Z88",7,0,
                0,1,0,0,                //No mostrar cursor,mostrar recuadro,no mostrar rainbow
                4,0,          //Colores para opcion seleccionada
                7,2,4,2,      //Colores para opcion no disponible
                0,7,          //Colores para el titulo y linea recuadro ventana
				7,0,          //Colores para el titulo y linea recuadro ventana inactiva
                4,              //Color waveform
                4,               //Color para zona no usada en visualmem
                2,7+8,		//Color para opcion marcada
		'*',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
                },


        {"CPC",1,6+8,
                0,1,1,0,          //No mostrar cursor,mostrar recuadro,mostrar rainbow
                6+8,1,            //Colores para opcion seleccionada
                1,2,6+8,2,        //Colores para opcion no disponible
                6+8,1,            //Colores para el titulo y linea recuadro ventana
				1,6+8,            //Colores para el titulo y linea recuadro ventana inactiva
                6+8,              //Color waveform
                0,               //Color para zona no usada en visualmem
                2,7+8,		//Color para opcion marcada
		'*',
		2, //color de aviso
		colores_franja_cpc_brillo,colores_franja_cpc_oscuro
                },

        {"Sam",7+8,0,
                0,1,1,0,                //No mostrar cursor,mostrar recuadro,mostrar rainbow
                5+8,0,          //Colores para opcion seleccionada
                7+8,2,7,2,      //Colores para opcion no disponible
                0,7+8,          //Colores para el titulo y linea recuadro ventana
				7+8,0,          //Colores para el titulo y linea recuadro ventana inactiva
                1,              //Color waveform
                7,               //Color para zona no usada en visualmem
                2,7+8,		//Color para opcion marcada
		'#',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
                },

						{"ManSoftware",7+8,0,
							0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
							5+8,0, 		//Colores para opcion seleccionada
							7+8,3,7,3, 	//Colores para opcion no disponible
							0,7+8,        	//Colores para el titulo y linea recuadro ventana
							7+8,0,        	//Colores para el titulo y linea recuadro ventana inactiva
							1,		//Color waveform
							7,		//Color para zona no usada en visualmem
							3,7+8,		//Color para opcion marcada
							'#',
		3+8, //color de aviso, en este tema, magenta con brillo
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
							},


			{"QL",7+8,0,
					0,1,0,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
					4+8,0, 		//Colores para opcion seleccionada
					7+8,2,7,2, 	//Colores para opcion no disponible
					2,7+8,        	//Colores para el titulo y linea recuadro ventana
					7+8,2,        	//Colores para el titulo y linea recuadro ventana inactiva
					4,		//Color waveform
					7,		//Color para zona no usada en visualmem
					2,7+8,		//Color para opcion marcada
					'*',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
								},

	{"RetroMac",7,0,
		0,1,0,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
		1+8,7+8, 		//Colores para opcion seleccionada
		7,2,1+8,2, 	//Colores para opcion no disponible
		7+8,0,        	//Colores para el titulo y linea recuadro ventana
		7,0,    	//Colores para el titulo y linea recuadro ventana inactiva
		0,		//Color waveform
		7,		//Color para zona no usada en visualmem
		2,7,		//Color para opcion marcada
		'.',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
		},

{"Borland",1,7+8,
		0,1,1,0, 		//No mostrar cursor,mostrar recuadro,mostrar rainbow
		4,1, 		//Colores para opcion seleccionada
		1,7,7,1, 	//Colores para opcion no disponible
		7+8,0,        	//Colores para el titulo y linea recuadro ventana
		7,0,        	//Colores para el titulo y linea recuadro ventana inactiva
		1,		//Color waveform
		7,		//Color para zona no usada en visualmem
		2,7+8,		//Color para opcion marcada
		'*',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
		},


        {"Clean",7,0,
                0,1,0,0,          //No Mostrar cursor >, mostrar recuadro, no mostrar rainbow
                0,7,          //Colores para opcion seleccionada
		7,2,0,2, 	//Colores para opcion no disponible
                0,7,          //Colores para el titulo y linea recuadro ventana
				7,0,          //Colores para el titulo y linea recuadro ventana inactiva
                0,              //Color waveform
                7,               //Color para zona no usada en visualmem
                7,0,		//Color para opcion marcada
		'X',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
                },

        {"CleanInverse",0,7,
                0,1,0,0,          //No Mostrar cursor >, mostrar recuadro, no mostrar rainbow
                7,0,          //Colores para opcion seleccionada
		0,2,7,2, 	//Colores para opcion no disponible
                7,0,          //Colores para el titulo y linea recuadro ventana
				0,7,          //Colores para el titulo y linea recuadro ventana inactiva
                7,              //Color waveform
                0,               //Color para zona no usada en visualmem
                0,7,		//Color para opcion marcada
		'X',
		2, //color de aviso
		colores_franja_speccy_brillo,colores_franja_speccy_oscuro
                },


};



//valores de la ventana mostrada
 
int current_win_x,current_win_y,current_win_ancho,current_win_alto;

//tipo ventana. normalmente activa. se pone tipo inactiva desde zxvision al pulsar fuera de la ventana
int ventana_tipo_activa=1;


//Elemento que identifica a un archivo en funcion de seleccion
struct s_filesel_item{
	//struct dirent *d;
	char d_name[PATH_MAX];
        unsigned char  d_type;

        //siguiente item
        struct s_filesel_item *next;
};

typedef struct s_filesel_item filesel_item;

int filesel_total_items;
filesel_item *primer_filesel_item;

//linea seleccionada en selector de archivos (relativa al primer archivo 0...20 maximo seguramente)
int filesel_linea_seleccionada;

//numero de archivo seleccionado en selector (0...ultimo archivo en directorio)
int filesel_archivo_seleccionado;

//indica en que zona del selector estamos:
//0: nombre archivo
//1: selector de archivo
//2: zona filtros
int filesel_zona_pantalla;


//nombre completo (nombre+path)del archivo seleccionado
char filesel_nombre_archivo_seleccionado[PATH_MAX];

//Si mostrar en filesel utilidades de archivos
z80_bit menu_filesel_show_utils={0};

//Si no caben todos los archivos en pantalla y por tanto se muestra "*" a la derecha
int filesel_no_cabe_todo;

//Total de archivos en el directorio mostrado
int filesel_total_archivos;

//En que porcentaje esta el indicador
int filesel_porcentaje_visible;


int menu_tooltip_counter;



int menu_window_splash_counter;
int menu_window_splash_counter_ms;

z80_bit tooltip_enabled;

//La primera vez que arranca, dispara evento de startup aid. Se inicializa desde cpu.c
int menu_first_aid_startup=0;


int menu_first_aid_must_show_startup=0;

//El texto a disparar al startup
char *string_config_key_aid_startup;


//Si se refresca en color gris cuando menu abierto y multitask es off
//z80_bit screen_bw_no_multitask_menu={1};


int menu_display_cursesstdout_cond(void);



void menu_filesel_chdir(char *dir);

void menu_change_audio_driver(MENU_ITEM_PARAMETERS);

void menu_chardetection_settings(MENU_ITEM_PARAMETERS);


void menu_tape_settings(MENU_ITEM_PARAMETERS);

void menu_hardware_memory_settings(MENU_ITEM_PARAMETERS);

int menu_tape_settings_cond(void);

void menu_zxuno_spi_flash(MENU_ITEM_PARAMETERS);

void menu_tsconf_layer_settings(MENU_ITEM_PARAMETERS);



int menu_inicio_opcion_seleccionada=0;
int machine_selection_opcion_seleccionada=0;
int machine_selection_por_fabricante_opcion_seleccionada=0;
int display_settings_opcion_seleccionada=0;
int audio_settings_opcion_seleccionada=0;
int hardware_settings_opcion_seleccionada=0;
int keyboard_settings_opcion_seleccionada=0;
int hardware_memory_settings_opcion_seleccionada=0;
int tape_settings_opcion_seleccionada=0;
int snapshot_opcion_seleccionada=0;
int debug_settings_opcion_seleccionada=0;
int chardetection_settings_opcion_seleccionada=0;
int textspeech_opcion_seleccionada=0;
int about_opcion_seleccionada=0;
int interface_settings_opcion_seleccionada=0;
int hotswap_machine_opcion_seleccionada=0;
int hardware_advanced_opcion_seleccionada=0;
int custom_machine_opcion_seleccionada;
int hardware_printers_opcion_seleccionada=0;

int input_file_keyboard_opcion_seleccionada=0;
int change_video_driver_opcion_seleccionada=0;

int hardware_realjoystick_opcion_seleccionada=0;
int hardware_realjoystick_event_opcion_seleccionada=0;
int hardware_realjoystick_keys_opcion_seleccionada=0;



int z88_slots_opcion_seleccionada=0;
int z88_slot_insert_opcion_seleccionada=0;
int z88_eprom_size_opcion_seleccionada=0;
int z88_flash_intel_size_opcion_seleccionada=0;
int find_opcion_seleccionada=0;
int find_bytes_opcion_seleccionada=0;
int find_lives_opcion_seleccionada=0;
int storage_settings_opcion_seleccionada=0;
int external_tools_config_opcion_seleccionada=0;


int timexcart_opcion_seleccionada=0;
int mmc_divmmc_opcion_seleccionada=0;
int ide_divide_opcion_seleccionada=0;
int hardware_redefine_keys_opcion_seleccionada=0;
int hardware_set_f_functions_opcion_seleccionada=0;
int hardware_set_f_func_action_opcion_seleccionada=0;
int ula_settings_opcion_seleccionada=0;

//int debug_configuration_opcion_seleccionada=0;
int dandanator_opcion_seleccionada=0;
int kartusho_opcion_seleccionada=0;
int ifrom_opcion_seleccionada=0;
int superupgrade_opcion_seleccionada=0;
int multiface_opcion_seleccionada=0;
int betadisk_opcion_seleccionada=0;

int settings_opcion_seleccionada=0;

int settings_snapshot_opcion_seleccionada=0;


int settings_storage_opcion_seleccionada=0;
int settings_tape_opcion_seleccionada=0;
int settings_config_file_opcion_seleccionada=0;
int ay_player_opcion_seleccionada=0;
int esxdos_traps_opcion_seleccionada=0;

int colour_settings_opcion_seleccionada=0;
int zxuno_spi_flash_opcion_seleccionada=0;

int menu_recent_files_opcion_seleccionada=0;



int plusthreedisk_opcion_seleccionada=0;


int window_settings_opcion_seleccionada=0;
int osd_settings_opcion_seleccionada=0;


int debug_tsconf_opcion_seleccionada;

int accessibility_settings_opcion_seleccionada=0;

int licenses_opcion_seleccionada=0;


//Indica que esta el splash activo o cualquier otro texto de splash, como el de cambio de modo de video
z80_bit menu_splash_text_active;

//segundos que le faltan para desactivarse
int menu_splash_segundos=0;

void menu_simple_ventana(char *titulo,char *texto);


//filtros activos
char **filesel_filtros;



int menu_avisa_si_extension_no_habitual(char *filtros[],char *archivo);

//filtros iniciales con los que se llama a la funcion
char **filesel_filtros_iniciales;

//filtro de todos archivos
char *filtros_todos_archivos[2];

//cinta seleccionada. tapefile apuntara aqui
char tape_open_file[PATH_MAX];
//cinta seleccionada. tape_out_file apuntara aqui
char tape_out_open_file[PATH_MAX];


//Ultimo directorio al salir con ESC desde fileselector
char menu_filesel_last_directory_seen[PATH_MAX];

char menu_buffer_textspeech_filter_program[PATH_MAX];
char menu_buffer_textspeech_stop_filter_program[PATH_MAX];

//cinta real seleccionada. realtape_name apuntara aqui
char menu_realtape_name[PATH_MAX];






//snapshot load. snapfile apuntara aqui
char snapshot_load_file[PATH_MAX];
char snapshot_save_file[PATH_MAX]="";

char binary_file_load[PATH_MAX]="";
char binary_file_save[PATH_MAX];

//char file_viewer_file_name[PATH_MAX]="";

char file_utils_file_name[PATH_MAX]="";

char *quickfile=NULL;
//quickload seleccionada. quickfile apuntara aqui
char quickload_file[PATH_MAX];

//Ultimos archivos cargados desde smartload
char last_files_used_array[MAX_LAST_FILESUSED][PATH_MAX];

//archivo zxprinter bitmap
char zxprinter_bitmap_filename_buffer[PATH_MAX];
//archivo zxprinter texto ocr
char zxprinter_ocr_filename_buffer[PATH_MAX];


char last_timex_cart[PATH_MAX]="";


void menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor);


//directorio inicial al entrar
char filesel_directorio_inicial[PATH_MAX];





int menu_debug_show_memory_zones=0;
int menu_debug_memory_zone=-1;
menu_z80_moto_int menu_debug_memory_zone_size=65536;


//
// Inicio funciones de gestion de zonas de memoria
//

void menu_debug_set_memory_zone_attr(void)
{

	int readwrite;

	if (menu_debug_show_memory_zones==0) {
		menu_debug_memory_zone_size=65536;
		if (MACHINE_IS_QL) menu_debug_memory_zone_size=QL_MEM_LIMIT+1;
		return;
	}

	//Primero ver si zona actual no esta disponible, fallback a 0 que siempre esta
	 menu_debug_memory_zone_size=machine_get_memory_zone_attrib(menu_debug_memory_zone,&readwrite);
	if (!menu_debug_memory_zone_size) {
		menu_debug_memory_zone=0;
		menu_debug_memory_zone_size=machine_get_memory_zone_attrib(menu_debug_memory_zone,&readwrite);
	}
}

//Muestra byte mapeado de ram normal o de zona de menu mapeada
z80_byte menu_debug_get_mapped_byte(int direccion)
{

	//Mostrar memoria normal
	if (menu_debug_show_memory_zones==0) {
		//printf ("menu_debug_get_mapped_byte dir %04XH result %02XH\n",direccion,peek_byte_z80_moto(direccion));
		return peek_byte_z80_moto(direccion);
	}


	//Mostrar zonas mapeadas
	menu_debug_set_memory_zone_attr();

	direccion=direccion % menu_debug_memory_zone_size;
	return *(machine_get_memory_zone_pointer(menu_debug_memory_zone,direccion));



}


//Interrumpe el core y le dice que hay que abrir el menu
void menu_fire_event_open_menu(void)
{
	//printf ("Ejecutar menu_fire_event_open_menu\n");
	menu_abierto=1;
	menu_event_open_menu.v=1;
}

//Escribe byte mapeado de ram normal o de zona de menu mapeada
void menu_debug_write_mapped_byte(int direccion,z80_byte valor)
{



	//Mostrar memoria normal
	if (menu_debug_show_memory_zones==0) {
		return poke_byte_z80_moto(direccion,valor);
	}


	//Mostrar zonas mapeadas
	menu_debug_set_memory_zone_attr();

	direccion=direccion % menu_debug_memory_zone_size;
	*(machine_get_memory_zone_pointer(menu_debug_memory_zone,direccion))=valor;



}


menu_z80_moto_int adjust_address_memory_size(menu_z80_moto_int direccion)
{

	//Si modo mapeo normal
	if (menu_debug_show_memory_zones==0) {
		return adjust_address_space_cpu(direccion);
	}

	//Si zonas memoria mapeadas
	if (direccion>=menu_debug_memory_zone_size) {
		//printf ("ajustamos direccion %x a %x\n",direccion,menu_debug_memory_zone_size);
		direccion=direccion % menu_debug_memory_zone_size;
		//printf ("resultado ajustado: %x\n",direccion);
	}

	return direccion;
}


void menu_debug_set_memory_zone_mapped(void)
{
		menu_debug_memory_zone=-1;
		menu_debug_show_memory_zones=0;	
}


//Retorna -1 si mapped memory. 0 o en adelante si otros. -2 si ESC
int menu_change_memory_zone_list_title(char *titulo)
{

        menu_item *array_menu_memory_zones;
        menu_item item_seleccionado;
        int retorno_menu;
		int menu_change_memory_zone_list_opcion_seleccionada=0;
        //do {

                char buffer_texto[40];

				

				menu_add_item_menu_inicial_format(&array_menu_memory_zones,MENU_OPCION_NORMAL,NULL,NULL,"Mapped memory");
				menu_add_item_menu_valor_opcion(array_menu_memory_zones,-1);

                int zone=-1;
				int i=1;
                do {

					zone++;
					zone=machine_get_next_available_memory_zone(zone);
					if (zone>=0) {
						machine_get_memory_zone_name(zone,buffer_texto);
						menu_add_item_menu_format(array_menu_memory_zones,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
						menu_add_item_menu_valor_opcion(array_menu_memory_zones,zone);

						if (menu_debug_memory_zone==zone) menu_change_memory_zone_list_opcion_seleccionada=i;

					}
					i++;
				} while (zone>=0);


                menu_add_item_menu(array_menu_memory_zones,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_set_f_func_action,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_memory_zones);

                retorno_menu=menu_dibuja_menu(&menu_change_memory_zone_list_opcion_seleccionada,&item_seleccionado,array_menu_memory_zones,titulo );

                


				if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
						//Cambiamos la zona
						int valor_opcion=item_seleccionado.valor_opcion;
						return valor_opcion;
                    
                }

	return -2;

}

int menu_change_memory_zone_list(void)
{
  return menu_change_memory_zone_list_title("Zones");
 }

void menu_set_memzone(int valor_opcion)
{
if (valor_opcion<0) {
		menu_debug_set_memory_zone_mapped();
	}
	else {
		menu_debug_show_memory_zones=1;
		menu_debug_memory_zone=valor_opcion;
	}	
}

void menu_debug_change_memory_zone(void)
{
	int valor_opcion=menu_change_memory_zone_list();
	if (valor_opcion==-2) return; //Pulsado ESC
	
	menu_set_memzone(valor_opcion);


	/*if (menu_debug_show_memory_zones==0) menu_debug_show_memory_zones=1;

	//Si se ha habilitado en el if anterior, entrara aqui
	if (menu_debug_show_memory_zones) {
		menu_debug_memory_zone++;
		menu_debug_memory_zone=machine_get_next_available_memory_zone(menu_debug_memory_zone);
		if (menu_debug_memory_zone<0)  {
			menu_debug_set_memory_zone_mapped();

		}
	}
	*/
}

void menu_debug_change_memory_zone_non_interactive(void)
{


	if (menu_debug_show_memory_zones==0) menu_debug_show_memory_zones=1;

	//Si se ha habilitado en el if anterior, entrara aqui
	if (menu_debug_show_memory_zones) {
		menu_debug_memory_zone++;
		menu_debug_memory_zone=machine_get_next_available_memory_zone(menu_debug_memory_zone);
		if (menu_debug_memory_zone<0)  {
			menu_debug_set_memory_zone_mapped();

		}
	}
	
}

void menu_debug_set_memory_zone(int zone)
{
	//Cambiar a zona memoria indicada
	int salir=0;

	//int zona_inicial=menu_debug_memory_zone;

	while (menu_debug_memory_zone!=zone && salir<2) {
		menu_debug_change_memory_zone_non_interactive();

		//Si ha pasado dos veces por la zona mapped, es que no existe dicha zona
		if (menu_debug_memory_zone<0) salir++;
	}
}

int menu_get_current_memory_zone_name_number(char *s)
{
	if (menu_debug_show_memory_zones==0) {
		strcpy(s,"Mapped memory");
		return -1;
	}

	machine_get_memory_zone_name(menu_debug_memory_zone,s);
	return menu_debug_memory_zone;
}


//Retorna el numero de digitos para representar un numero en hexadecimal
int menu_debug_get_total_digits_hexa(int valor)
{
	char temp_digitos[20];
	sprintf (temp_digitos,"%X",valor);
	return strlen(temp_digitos);
}

//Retorna el numero de digitos para representar un numero en decimal
int menu_debug_get_total_digits_dec(int valor)
{
	char temp_digitos[20];
	sprintf (temp_digitos,"%d",valor);
	return strlen(temp_digitos);
}

//Escribe una direccion en texto, en hexa, teniendo en cuenta zona memoria (rellenando espacios segun tamanyo zona)
void menu_debug_print_address_memory_zone(char *texto, menu_z80_moto_int address)
{
	//primero meter 6 espacios
	sprintf (texto,"      ");

	address=adjust_address_memory_size(address);
	//int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	//Obtener cuantos digitos hexa se necesitan
	//char temp_digitos[20];
	//sprintf (temp_digitos,"%X",menu_debug_memory_zone_size-1);
	//int digitos=strlen(temp_digitos);

	int digitos=menu_debug_get_total_digits_hexa(menu_debug_memory_zone_size-1);

	//Obtener posicion inicial a escribir direccion. Suponemos maximo 6
	int posicion_inicial_digitos=6-digitos;


	//Escribimos direccion
	sprintf (&texto[posicion_inicial_digitos],"%0*X",digitos,address);
}


//
// Fin funciones de gestion de zonas de memoria
//

//Cambia a directorio donde estan los archivos de instalacion (en share o en ..Resources)

void menu_chdir_sharedfiles(void)
{

	//cambia a los dos directorios. se quedara en el ultimo que exista
	debug_printf(VERBOSE_INFO,"Trying ../Resources");
	menu_filesel_chdir("../Resources");

	char installshare[PATH_MAX];
	sprintf (installshare,"%s/%s",INSTALL_PREFIX,"/share/zesarux/");
	debug_printf(VERBOSE_INFO,"Trying %s",installshare);
	menu_filesel_chdir(installshare);


}


//retorna dentro de un array de N teclas, la tecla pulsada
char menu_get_key_array_n_teclas(z80_byte valor_puerto,char *array_teclas,int teclas)
{

        int i;
        for (i=0;i<teclas;i++) {
                if ((valor_puerto&1)==0) return *array_teclas;
                valor_puerto=valor_puerto >> 1;
                array_teclas++;
        }

        return 0;

}



//retorna dentro de un array de 5 teclas, la tecla pulsada
char menu_get_key_array(z80_byte valor_puerto,char *array_teclas)
{

	return	menu_get_key_array_n_teclas(valor_puerto,array_teclas,5);

}

//funcion que retorna la tecla pulsada, solo tener en cuenta caracteres y numeros, sin modificador (mayus, etc)
//y por tanto solo 1 tecla a la vez

/*
z80_byte puerto_65278=255; //    db    		 255  ; V    C    X    Z    Sh    ;0
z80_byte puerto_65022=255; //    db    		 255  ; G    F    D    S    A     ;1
z80_byte puerto_64510=255; //    db              255  ; T    R    E    W    Q     ;2
z80_byte puerto_63486=255; //    db              255  ; 5    4    3    2    1     ;3
z80_byte puerto_61438=255; //    db              255  ; 6    7    8    9    0     ;4
z80_byte puerto_57342=255; //    db              255  ; Y    U    I    O    P     ;5
z80_byte puerto_49150=255; //    db              255  ; H    J    K    L    Enter ;6
z80_byte puerto_32766=255; //    db              255  ; B    N    M    Simb Space ;7

//puertos especiales no presentes en spectrum
z80_byte puerto_especial1=255; //   ;  .  .  .  . ESC ;
z80_byte puerto_especial2=255; //   F5 F4 F3 F2 F1
z80_byte puerto_especial3=255; //  F10 F9 F8 F7 F6
z80_byte puerto_especial4=255; //  F15 F14 F13 F12 F11
*/

static char menu_array_keys_65022[]="asdfg";
static char menu_array_keys_64510[]="qwert";
static char menu_array_keys_63486[]="12345";
static char menu_array_keys_61438[]="09876";
static char menu_array_keys_57342[]="poiuy";
static char menu_array_keys_49150[]="\x0dlkjh";

//arrays especiales
static char menu_array_keys_65278[]="zxcv";
static char menu_array_keys_32766[]="mnb";


/*
valores de teclas especiales:
2  ESC
3  Tecla de background
8  cursor left
9  cursor right
10 cursor down
11 cursor up
12 Delete o joystick left
13 Enter o joystick fire
15 SYM+MAY(TAB)
24 PgUp
25 PgDn

Joystick izquierda funcionara como Delete, no como cursor left. Resto de direcciones de joystick (up, down, right) se mapean como cursores

*/

/*
Teclas que tienen que retornar estas funciones para todas las maquinas posibles: spectrum, zx80/81, z88, cpc, sam, etc:

Letras y numeros

.  , : / - + < > = ' ( ) "


Hay drivers que retornan otros simbolos adicionales, por ejemplo en Z88 el ;. Esto es porque debe retornar ":" y estos : se obtienen mediante
mayusculas + tecla ";"

*/

//Si permitimos o no ventanas en background al pulsar F6
int menu_allow_background_windows=0;

z80_byte menu_get_pressed_key_no_modifier(void)
{
	z80_byte tecla;

	


	//ESC significa Shift+Space en ZX-Uno y tambien ESC puerto_especial para menu.
	//Por tanto si se pulsa ESC, hay que leer como tal ESC antes que el resto de teclas (Espacio o Shift)
	if ((puerto_especial1&1)==0) return 2;

	if ((puerto_especial3&1)==0 && menu_allow_background_windows) return 3; //Tecla background F6

	tecla=menu_get_key_array(puerto_65022,menu_array_keys_65022);
	if (tecla) return tecla;

	tecla=menu_get_key_array(puerto_64510,menu_array_keys_64510);
	if (tecla) return tecla;

	tecla=menu_get_key_array(puerto_63486,menu_array_keys_63486);
	if (tecla) return tecla;

	tecla=menu_get_key_array(puerto_61438,menu_array_keys_61438);
	if (tecla) return tecla;

	tecla=menu_get_key_array(puerto_57342,menu_array_keys_57342);
	if (tecla) return tecla;

	tecla=menu_get_key_array(puerto_49150,menu_array_keys_49150);
	if (tecla) return tecla;

        tecla=menu_get_key_array_n_teclas(puerto_65278>>1,menu_array_keys_65278,4);
        if (tecla) return tecla;

        tecla=menu_get_key_array_n_teclas(puerto_32766>>2,menu_array_keys_32766,3);
        if (tecla) return tecla;

	//Y espacio
	if ((puerto_32766&1)==0) return ' ';

	//PgUp
	if ((puerto_especial1&2)==0) return 24;

	//PgDn
	if ((puerto_especial1&4)==0) return 25;



	return 0;
}




z80_bit menu_symshift={0};
z80_bit menu_capshift={0};
z80_bit menu_backspace={0};
z80_bit menu_tab={0};

//devuelve tecla pulsada teniendo en cuenta mayus, sym shift
z80_byte menu_get_pressed_key(void)
{

	//Ver tambien eventos de mouse de zxvision
	//int pulsado_boton_cerrar=
	zxvision_handle_mouse_events(zxvision_current_window);

	if (mouse_pressed_close_window) {
		//mouse_pressed_close_window=0;
		return 2; //Como ESC
	}

	z80_byte tecla;

	//primero joystick
	if (puerto_especial_joystick) {
		//z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right
		if ((puerto_especial_joystick&1)) return 9;

		if ((puerto_especial_joystick&2)) return 8;

		//left joystick hace delete en menu.NO
		//if ((puerto_especial_joystick&2)) return 12;

		if ((puerto_especial_joystick&4)) return 10;
		if ((puerto_especial_joystick&8)) return 11;
//8  cursor left
//9  cursor right
//10 cursor down
//11 cursor up

		//Fire igual que enter
		if ((puerto_especial_joystick&16)) return 13;
	}

	

	if (menu_tab.v) {
		//printf ("Pulsado TAB\n");
		return 15;
	}

	if (menu_backspace.v) return 12;


	tecla=menu_get_pressed_key_no_modifier();



	if (tecla==0) return 0;


	//ver si hay algun modificador

	//mayus

//z80_byte puerto_65278=255; //    db              255  ; V    C    X    Z    Sh    ;0
	//if ( (puerto_65278&1)==0) {
	if (menu_capshift.v) {

	

		//si son letras, ponerlas en mayusculas
		if (tecla>='a' && tecla<='z') {
			tecla=tecla-('a'-'A');
			return tecla;
		}
		//seran numeros


		switch (tecla) {
			case '0':
				//delete
				return 12;
			break;

			
		}

	}

	//sym shift
	//else if ( (puerto_32766&2)==0) {
	else if (menu_symshift.v) {
		//ver casos concretos
		switch (tecla) {

			case 'z':
				return ':';
			break;

			case 'x':
				return 96; //Libra
			break;

			case 'c':
				return '?';
			break;

			case 'v':
				return '/';
			break;			

			case 'b':
				return '*';
			break;	

			case 'n':
				return ',';
			break;			

			case 'm':
				return '.';
			break;	

			case 'a':
				return '~'; //Aunque esta sale con ext+symbol
			break;

			case 's':
				return '|'; //Aunque esta sale con ext+symbol
			break;			

			case 'd':
				return '\\'; //Aunque esta sale con ext+symbol
			break;

			case 'f':
				return '{'; //Aunque esta sale con ext+symbol
			break;

			case 'g':
				return '}'; //Aunque esta sale con ext+symbol
			break;		

			case 'h':
				return 94; //Símbolo exponente/circunflejo
			break;						

			case 'j':
				return '-';
			break;

			case 'k':
				return '+';
			break;

			case 'l':
				return '=';
			break;			



			case 'r':
				return '<';
			break;

			case 't':
				return '>';
			break;

			case 'y':
				return '[';
			break;

			case 'u':
				return ']';
			break;	

			//Faltaria el (C) del ext+sym+p. Se podria mapear a sym+I, pero esto genera el codigo 127,
			//Y dicho código en ascii no es imprimible y puede dar problemas en drivers texto, como curses		

			case 'o':
				return ';';
			break;

			case 'p':
				return '"';
			break;	



			case '1':
				return '!';
			break;

			case '2':
				return '@';
			break;

			case '3':
				return '#';
			break;

			case '4':
				return '$';
			break;

			case '5':
				return '%';
			break;

			case '6':
				return '&';
			break;

			case '7':
				return '\'';
			break;

			case '8':
				return '(';
			break;

			case '9':
				return ')';
			break;

			case '0':
				return '_';
			break;




			//no hace falta esta tecla. asi tambien evitamos que alguien la use en nombre de
			//archivo pensando que se puede introducir un filtro tipo *.tap, etc.
			//case 'b':
			//	return '*';
			//break;

		}
	}


	return tecla;

}

//escribe la cadena de texto
void menu_scanf_print_string(char *string,int offset_string,int max_length_shown,int x,int y)
{
	z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
	z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;
	char cadena_buf[2];

	string=&string[offset_string];

	//contar que hay que escribir el cursor
	max_length_shown--;

	//y si offset>0, primer caracter sera '<'
	if (offset_string) {
		menu_escribe_texto(x,y,tinta,papel,"<");
		max_length_shown--;
		x++;
		string++;
	}

	for (;max_length_shown && (*string)!=0;max_length_shown--) {
		cadena_buf[0]=*string;
		cadena_buf[1]=0;
		menu_escribe_texto(x,y,tinta,papel,cadena_buf);
		x++;
		string++;
	}

        //menu_escribe_texto(x,y,tinta,papel,"_");
				putchar_menu_overlay_parpadeo(x,y,'_',tinta,papel,1);
        x++;


        for (;max_length_shown!=0;max_length_shown--) {
                menu_escribe_texto(x,y,tinta,papel," ");
                x++;
        }




}

//funcion que guarda el contenido del texto del menu. Usado por ejemplo en scanf cuando se usa teclado en pantalla
void menu_save_overlay_text_contents(overlay_screen *destination)
{
	int size=sizeof(overlay_screen_array);
	debug_printf(VERBOSE_DEBUG,"Saving overlay text contents. Size=%d bytes",size);

	memcpy(destination,overlay_screen_array,size);
}

//funcion que restaura el contenido del texto del menu. Usado por ejemplo en scanf cuando se usa teclado en pantalla
void menu_restore_overlay_text_contents(overlay_screen *origin)
{
	int size=sizeof(overlay_screen_array);
	debug_printf(VERBOSE_DEBUG,"Restoring overlay text contents. Size=%d bytes",size);

	memcpy(overlay_screen_array,origin,size);
}


//Llamar al teclado en pantalla pero desde algun menu ya, sobreimprimiéndolo
void menu_call_onscreen_keyboard_from_menu(void)
{


	menu_espera_no_tecla();
	menu_button_osdkeyboard.v=0; //Decir que no tecla osd pulsada, por si acaso
	menu_button_f_function.v=0;

	overlay_screen copia_overlay[OVERLAY_SCREEN_MAX_WIDTH*OVERLAY_SCREEN_MAX_HEIGTH];

	//Guardamos contenido de la pantalla
	menu_save_overlay_text_contents(copia_overlay);
	//Guardamos linea cuadrado ventana
	int antes_cuadrado_activo=0;
	int antes_cuadrado_activo_resize=0;
	int antes_cuadrado_x1,antes_cuadrado_y1,antes_cuadrado_x2,antes_cuadrado_y2,antes_cuadrado_color;
	
	antes_cuadrado_activo=cuadrado_activo;
	antes_cuadrado_activo_resize=cuadrado_activo_resize;
	antes_cuadrado_x1=cuadrado_x1;
	antes_cuadrado_y1=cuadrado_y1;
	antes_cuadrado_x2=cuadrado_x2;
	antes_cuadrado_y2=cuadrado_y2;
	antes_cuadrado_color=cuadrado_color;

	//Guardamos tamanyo ventana
	z80_byte antes_ventana_x,antes_ventana_y,antes_ventana_ancho,antes_ventana_alto;
	antes_ventana_x=current_win_x;
	antes_ventana_y=current_win_y;
	antes_ventana_ancho=current_win_ancho;
	antes_ventana_alto=current_win_alto;

	//Comportamiento de 1 caracter de margen a la izquierda en ventana (lo altera hexdump)
	int antes_menu_escribe_linea_startx=menu_escribe_linea_startx;

	menu_escribe_linea_startx=1;
	

	//Conservar setting salir_todos_menus, que lo cambia el osd
	int antes_salir_todos_menus=salir_todos_menus;


	//Cambiar funcion de overlay por la normal


                              //Guardar funcion de texto overlay activo, para menus como el de visual memory por ejemplo, para desactivar  temporalmente
                                        void (*previous_function)(void);

                                        previous_function=menu_overlay_function;

                                       //restauramos modo normal de texto de menu
                                       set_menu_overlay_function(normal_overlay_texto_menu);



       //
         menu_onscreen_keyboard(0);                            


                                        //Restauramos funcion anterior de overlay
                                        set_menu_overlay_function(previous_function);


	

	salir_todos_menus=antes_salir_todos_menus;

	menu_escribe_linea_startx=antes_menu_escribe_linea_startx;

	//Restaurar texto ventana
	menu_restore_overlay_text_contents(copia_overlay);
	
	//Restaurar linea cuadrado ventana
	cuadrado_activo=antes_cuadrado_activo;
	cuadrado_activo_resize=antes_cuadrado_activo_resize;
	cuadrado_x1=antes_cuadrado_x1;
	cuadrado_y1=antes_cuadrado_y1;
	cuadrado_x2=antes_cuadrado_x2;
	cuadrado_y2=antes_cuadrado_y2;
	cuadrado_color=antes_cuadrado_color;

	//Restaurar tamanyo ventana
	current_win_x=antes_ventana_x;
	current_win_y=antes_ventana_y;
	current_win_ancho=antes_ventana_ancho;
	current_win_alto=antes_ventana_alto;

	menu_refresca_pantalla();	

	

			
	


	
}

//Si se ha pulsado tecla (o boton) asignado a osd
int menu_si_pulsada_tecla_osd(void)
{
	if (menu_button_osdkeyboard.v) {
		//debug_printf(VERBOSE_DEBUG,"Pressed OSD default key");
		return 1;
	}

	if (menu_button_f_function.v==0) return 0;

	//debug_printf(VERBOSE_DEBUG,"Pressed F key");

	//Tecla F pulsada, ver si es la asignada a osd
        int indice=menu_button_f_function_index;

        enum defined_f_function_ids accion=defined_f_functions_keys_array[indice];
	if (accion==F_FUNCION_OSDKEYBOARD) {
		//debug_printf(VERBOSE_DEBUG,"Pressed F key mapped to OSD");
		return 1;
	}

	return 0;


}

//devuelve cadena de texto desde teclado
//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//ejemplo, si el array es de 50, se le debe pasar max_length a 50
int menu_scanf(char *string,unsigned int max_length,int max_length_shown,int x,int y)
{

	//Enviar a speech
	char buf_speech[MAX_BUFFER_SPEECH+1];
	sprintf (buf_speech,"Edit box: %s",string);
	menu_textspeech_send_text(buf_speech);


	z80_byte tecla;

	//ajustar offset sobre la cadena de texto visible en pantalla
	int offset_string;

	int j;
	j=strlen(string);
	if (j>max_length_shown-1) offset_string=j-max_length_shown+1;
	else offset_string=0;


	//max_length ancho maximo del texto, sin contar caracter 0
	//por tanto si el array es de 50, se le debe pasar max_length a 50

	max_length--;

	//cursor siempre al final del texto

	do {
		menu_scanf_print_string(string,offset_string,max_length_shown,x,y);

		if (menu_multitarea==0) menu_refresca_pantalla();

		menu_espera_tecla();
		//printf ("Despues de espera tecla\n");
		tecla=menu_get_pressed_key();
		//printf ("tecla leida=%d\n",tecla);
		menu_espera_no_tecla();



		//si tecla normal, agregar:
		if (tecla>31 && tecla<128) {
			if (strlen(string)<max_length) {
				int i;
				i=strlen(string);
				string[i++]=tecla;
				string[i]=0;

				//Enviar a speech letra pulsada
				menu_speech_tecla_pulsada=0;
			        sprintf (buf_speech,"%c",tecla);
        			menu_textspeech_send_text(buf_speech);


				if (i>=max_length_shown) offset_string++;

			}
		}

		//tecla borrar o tecla izquierda
		if (tecla==12 || tecla==8) {
			if (strlen(string)>0) {
                                int i;
                                i=strlen(string)-1;

                                //Enviar a speech letra borrada

				menu_speech_tecla_pulsada=0;
                                sprintf (buf_speech,"%c",string[i]);
                                menu_textspeech_send_text(buf_speech);


                                string[i]=0;
				if (offset_string>0) {
					offset_string--;
					//printf ("offset string: %d\n",offset_string);
				}
			}

		}


	} while (tecla!=13 && tecla!=15 && tecla!=2);

	//if (tecla==13) printf ("salimos con enter\n");
	//if (tecla==15) printf ("salimos con tab\n");

	menu_reset_counters_tecla_repeticion();
	return tecla;

//papel=7+8;
//tinta=0;

}



//funcion para asignar funcion de overlay
void set_menu_overlay_function(void (*funcion)(void) )
{

	menu_overlay_function=funcion;

	//para que al oscurecer la pantalla tambien refresque el border
	modificado_border.v=1;
	menu_overlay_activo=1;

	//Necesario para que al poner la capa de menu, se repinte todo
	clear_putpixel_cache();	

	//Y por si acaso, aunque ya deberia haber buffer de capas activo, asignarlo
	scr_init_layers_menu();
}


//funcion para desactivar funcion de overlay
void reset_menu_overlay_function(void)
{
	//para que al oscurecer la pantalla tambien refresque el border
	modificado_border.v=1;



	menu_overlay_activo=0;

	scr_clear_layer_menu();


}

//funcion para escribir un caracter en el buffer de overlay
//tinta y/o papel pueden tener brillo (color+8)
void putchar_menu_overlay_parpadeo(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel,z80_byte parpadeo)
{

	int xusado=x;

	if (menu_char_width!=8) {
		xusado=(x*menu_char_width)/8;		
	}

	//int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

	//Controlar limite
	if (x<0 || y<0 || x>=scr_get_menu_width() || y>=scr_get_menu_height() ) {
		//printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
		return;
	}

	int pos_array=y*scr_get_menu_width()+x;
	overlay_screen_array[pos_array].tinta=tinta;
	overlay_screen_array[pos_array].papel=papel;
	overlay_screen_array[pos_array].parpadeo=parpadeo;

	if (ESTILO_GUI_SOLO_MAYUSCULAS) overlay_screen_array[pos_array].caracter=letra_mayuscula(caracter);
	else overlay_screen_array[pos_array].caracter=caracter;


	overlay_usado_screen_array[y*scr_get_menu_width()+xusado]=1;

	
}


//funcion para escribir un caracter en el buffer de overlay
//tinta y/o papel pueden tener brillo (color+8)
void putchar_menu_overlay(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel)
{
	putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0); //sin parpadeo
}





void menu_scr_putpixel(int x,int y,int color)
{

	//int margenx_izq,margeny_arr;
	//scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);

	x *=menu_gui_zoom;
	y *=menu_gui_zoom;	

	//Esto ya no hace falta desde el uso de dos layers de menu y maquina
	/*if (rainbow_enabled.v) {
		x+=margenx_izq;
		y+=margeny_arr;
	}*/



	scr_putpixel_gui_zoom(x,y,color,menu_gui_zoom);
}
/*
//Hacer un putpixel en la coordenada indicada pero haciendo tan gordo el pixel como diga zoom_level
void scr_putpixel_gui_zoom(int x,int y,int color,int zoom_level)
{ 
	//Hacer zoom de ese pixel si conviene
	int incx,incy;
	for (incy=0;incy<zoom_level;incy++) {
		for (incx=0;incx<zoom_level;incx++) {
			//printf("putpixel %d,%d\n",x+incx,y+incy);
			if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+incx,y+incy,color);

			else scr_putpixel_zoom(x+incx,y+incy,color);
		}
	}
}
*/

void new_menu_putchar_footer(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel)
{

	putchar_footer_array(x,y,caracter,tinta,papel,0);


}


void old_menu_putchar_footer(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel)
{
	if (!menu_footer) return;

	//Sin interlaced
	if (video_interlaced_mode.v==0) {
		scr_putchar_footer(x,y,caracter,tinta,papel);
		return;
	}


	//Con interlaced
	//Queremos que el footer se vea bien, no haga interlaced y no haga scanlines
	//Cuando se activa interlaced se cambia la funcion de putpixel, por tanto,
	//desactivando aqui interlaced no seria suficiente para que el putpixel saliese bien


	//No queremos que le afecte el scanlines
	z80_bit antes_scanlines;
	antes_scanlines.v=video_interlaced_scanlines.v;
	video_interlaced_scanlines.v=0;

	//Escribe texto pero como hay interlaced, lo hará en una linea de cada 2
	scr_putchar_footer(x,y,caracter,tinta,papel);

	//Dado que hay interlaced, simulamos que estamos en siguiente frame de pantalla para que dibuje la linea par/impar siguiente
	interlaced_numero_frame++;
	scr_putchar_footer(x,y,caracter,tinta,papel);
	interlaced_numero_frame--;

	//restaurar scanlines
	video_interlaced_scanlines.v=antes_scanlines.v;
}

void menu_putstring_footer(int x,int y,char *texto,z80_byte tinta,z80_byte papel)
{
	while ( (*texto)!=0) {
		new_menu_putchar_footer(x++,y,*texto,tinta,papel);
		texto++;
	}

	//Solo en putstring actualizamos el footer. En putchar, no
	redraw_footer();
}


void menu_footer_activity(char *texto)
{

	char buffer_texto[32];
	//Agregar espacio delante y detras
	sprintf (buffer_texto," %s ",texto);

	int inicio_x=32-strlen(buffer_texto);

	menu_putstring_footer(inicio_x,1,buffer_texto,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);

}

void menu_delete_footer_activity(void)
{
	//45678901
	menu_putstring_footer(24,1,"        ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
}

//Escribir info tarjetas memoria Z88
void menu_footer_z88(void)
{

	if (!MACHINE_IS_Z88) return;

	char nombre_tarjeta[20];
	int x=0;

	//menu_putstring_footer(0,0,get_machine_name(current_machine_type),WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

	//borramos esa zona primero

	menu_footer_clear_bottom_line();
	
	int i;
	for (i=1;i<=3;i++) {
		if (z88_memory_slots[i].size==0) sprintf (nombre_tarjeta," Empty ");
		else sprintf (nombre_tarjeta," %s ",z88_memory_types[z88_memory_slots[i].type]);

		//Si ocupa el texto mas de 10, cortar texto
		if (strlen(nombre_tarjeta)>10) {
			nombre_tarjeta[9]=' ';
			nombre_tarjeta[10]=0;
		}

		menu_putstring_footer(x,2,nombre_tarjeta,WINDOW_FOOTER_PAPER,WINDOW_FOOTER_INK);
		x +=10;
	}
}

int menu_si_mostrar_footer_f5_menu(void)
{
	if (!MACHINE_IS_Z88)  {
					//Y si no hay texto por encima de cinta autodetectada
					if (tape_options_set_first_message_counter==0 && tape_options_set_second_message_counter==0) {
							return 1;
					}
	}

	return 0;
}

void menu_footer_f5_menu(void)
{

        //Decir F5 menu en linea de tarjetas de memoria de z88
        //Y si es la primera vez
        if (menu_si_mostrar_footer_f5_menu() ) {
												//Borrar antes con espacios si hay algo               //01234567890123456789012345678901
												//Sucede que al cargar emulador con un tap, se pone abajo primero el nombre de emulador y version,
												//y cuando se quita el splash, se pone este texto. Si no pongo espacios, se mezcla parte del texto de F5 menu etc con la version del emulador

												menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_F5MENU,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
                        char texto_f_menu[32];
                        sprintf(texto_f_menu,"%s Menu",openmenu_key_message);
                        menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_F5MENU,texto_f_menu,WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
				}


}

void menu_footer_zesarux_emulator(void)
{

	if (menu_si_mostrar_footer_f5_menu() ) {
		debug_printf (VERBOSE_DEBUG,"Showing ZEsarUX footer message");
		menu_putstring_footer(0,WINDOW_FOOTER_ELEMENT_Y_ZESARUX_EMULATOR,"ZEsarUX emulator v."EMULATOR_VERSION,WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
	}

}

void menu_clear_footer(void)
{
	if (!menu_footer) return;


	debug_printf (VERBOSE_DEBUG,"Clearing Footer");

        //Borrar footer
        if (si_complete_video_driver() ) {

                int alto=WINDOW_FOOTER_SIZE;
                int ancho=screen_get_window_size_width_no_zoom_border_en();

                int yinicial=screen_get_window_size_height_no_zoom_border_en()-alto;

                int x,y;

                //Para no andar con problemas de putpixel en el caso de realvideo desactivado,
                //usamos putpixel tal cual y calculamos zoom nosotros manualmente

                alto *=zoom_y;
                ancho *=zoom_x;

                yinicial *=zoom_y;

                z80_byte color=WINDOW_FOOTER_PAPER;

                for (y=yinicial;y<yinicial+alto;y++) {
                        //printf ("%d ",y);
                        for (x=0;x<ancho;x++) {
                                scr_putpixel(x,y,color);
                        }
                }


        }

}

void menu_footer_bottom_line(void)
{
	menu_footer_z88();
	menu_footer_zesarux_emulator();
}

void menu_footer_clear_bottom_line(void)
{

	//                         01234567890123456789012345678901
	menu_putstring_footer(0,2,"                                ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

}

//Escribir textos iniciales en el footer
void menu_init_footer(void)
{
	if (!menu_footer) return;


        //int margeny_arr=screen_borde_superior*border_enabled.v;

        if (MACHINE_IS_Z88) {
                //no hay border. estas variables se leen en modo rainbow
                //margeny_arr=0;
        }

	//Si no hay driver video
	if (scr_putpixel==NULL || scr_putpixel_zoom==NULL) return;

	debug_printf (VERBOSE_INFO,"init_footer");

	//Al iniciar emulador, si aun no hay definidas funciones putpixel, volver


	//Borrar footer con pixeles blancos
	menu_clear_footer();

	//Inicializar array footer
	cls_footer();


	//Borrar zona con espacios
	//Tantos espacios como el nombre mas largo de maquina (Microdigital TK90X (Spanish))
	menu_putstring_footer(0,0,"                            ",WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

	//Obtener maquina activa
	menu_putstring_footer(0,0,get_machine_name(current_machine_type),WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);

	autoselect_options_put_footer();

	menu_footer_bottom_line();

	//Si hay lectura de flash activa en ZX-Uno
	//Esto lo hago asi porque al iniciar ZX-Uno, se ha activado el contador de texto "FLASH",
	//y en driver xwindows suele generar un evento ConfigureNotify, que vuelve a llamar a init_footer y borraria dicho texto FLASH
	//y por lo tanto no se veria el texto "FLASH" al arrancar la maquina
	//en otros drivers de video en teoria no haria falta
	//if (MACHINE_IS_ZXUNO) zxuno_footer_print_flash_operating();



}



//funcion para limpiar el buffer de overlay y si hay cuadrado activo
void cls_menu_overlay(void)
{

	int i;

	//Borrar solo el tamanyo de menu activo
	for (i=0;i<scr_get_menu_width()*scr_get_menu_height();i++) {
		overlay_screen_array[i].caracter=0;
		overlay_usado_screen_array[i]=0;
	}

	menu_desactiva_cuadrado();

        //si hay segunda capa, escribir la segunda capa en esta primera
	//copy_second_first_overlay();


	//Si en Z88 y driver grafico, redibujamos la zona inferior
	if (MACHINE_IS_Z88) {
		screen_z88_draw_lower_screen();
	}

	//Si es CPC, entonces aqui el border es variable y por tanto tenemos que redibujarlo, pues quiza el menu esta dentro de zona de border
	modificado_border.v=1;

	scr_clear_layer_menu();


	menu_draw_ext_desktop();

}



//funcion para escribir un caracter en el buffer de footer
//tinta y/o papel pueden tener brillo (color+8)
void putchar_footer_array(int x,int y,z80_byte caracter,z80_byte tinta,z80_byte papel,z80_byte parpadeo)
{

	if (!menu_footer) return;

	//int xfinal=((x*menu_char_width)+menu_char_width-1)/8;

	//Controlar limite
	if (x<0 || y<0 || x>WINDOW_FOOTER_COLUMNS || y>WINDOW_FOOTER_LINES) {
		//printf ("Out of range. X: %d Y: %d Character: %c\n",x,y,caracter);
		return;
	}

	if (ESTILO_GUI_SOLO_MAYUSCULAS) caracter=letra_mayuscula(caracter);

	int pos_array=y*WINDOW_FOOTER_COLUMNS+x;
	footer_screen_array[pos_array].tinta=tinta;
	footer_screen_array[pos_array].papel=papel;
	footer_screen_array[pos_array].parpadeo=parpadeo;
	footer_screen_array[pos_array].caracter=caracter;

	
}


void cls_footer(void)
{
	if (!menu_footer) return;

	int x,y;
	for (y=0;y<WINDOW_FOOTER_LINES;y++) {
		for (x=0;x<WINDOW_FOOTER_COLUMNS;x++) {
			putchar_footer_array(x,y,' ',WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER,0);
		}
	}
	
}

void redraw_footer_continue(void)
{
	if (!menu_footer) return;

	//printf ("redraw footer\n");

	int x,y;
	z80_byte tinta,papel,caracter,parpadeo;
	int pos_array=0;	
	for (y=0;y<WINDOW_FOOTER_LINES;y++) {
		for (x=0;x<WINDOW_FOOTER_COLUMNS;x++,pos_array++) {

			caracter=footer_screen_array[pos_array].caracter;

			tinta=footer_screen_array[pos_array].tinta;
			papel=footer_screen_array[pos_array].papel;
			parpadeo=footer_screen_array[pos_array].parpadeo;

			//Si esta multitask, si es caracter con parpadeo y si el estado del contador del parpadeo indica parpadear
			if (menu_multitarea && parpadeo && estado_parpadeo.v) caracter=' '; //si hay parpadeo y toca, meter espacio tal cual (se oculta)

			scr_putchar_footer(x,y,caracter,tinta,papel);
			
		}
	}

}

void redraw_footer(void)

{


	if (!menu_footer) return;

	//Sin interlaced
	if (video_interlaced_mode.v==0) {
		redraw_footer_continue();
		return;
	}


	//Con interlaced
	//Queremos que el footer se vea bien, no haga interlaced y no haga scanlines
	//Cuando se activa interlaced se cambia la funcion de putpixel, por tanto,
	//desactivando aqui interlaced no seria suficiente para que el putpixel saliese bien


	//No queremos que le afecte el scanlines
	z80_bit antes_scanlines;
	antes_scanlines.v=video_interlaced_scanlines.v;
	video_interlaced_scanlines.v=0;

	//Escribe texto pero como hay interlaced, lo hará en una linea de cada 2
	redraw_footer_continue();

	//Dado que hay interlaced, simulamos que estamos en siguiente frame de pantalla para que dibuje la linea par/impar siguiente
	interlaced_numero_frame++;
	redraw_footer_continue();
	interlaced_numero_frame--;

	//restaurar scanlines
	video_interlaced_scanlines.v=antes_scanlines.v;	

}

//Esta funcion antes se usaba para poner color oscuro o no al abrir el menu
//Actualmente solo cambia el valor de menu_abierto
void menu_set_menu_abierto(int valor)
{

        menu_abierto=valor;
}


//Para meter el logo en zona de extended desktop
void menu_draw_ext_desktop_logo(z80_int *destino GCC_UNUSED,int x,int y,int ancho GCC_UNUSED,int color)
{
	scr_putpixel(x,y,color);
}


//Tipo de rellenado de extended desktop:
//0=color solido
//1=barras diagonales de colores
//2=punteado blanco/negro
int menu_ext_desktop_fill=1;
int menu_ext_desktop_fill_solid_color=0;

void menu_draw_ext_desktop(void)
{

	//Si no escritorio extendido, salir
	if (!screen_ext_desktop_enabled || !scr_driver_can_ext_desktop() ) return;

	
		//Los putpixel que hacemos aqui son sin zoom. Se podrian hacer con zoom, pero habria que
		//usar scr_putpixel_zoom_rainbow y scr_putpixel_zoom dependiendo del caso, y sumar margenes en el caso de rainbow,
		//pero no vale la pena, con una sola funcion scr_putpixel vale para todos los casos
		//Con zoom se habria hecho asi:
		/*
			int margenx_izq;
			int margeny_arr;
			scr_return_margenxy_rainbow(&margenx_izq,&margeny_arr);
			if (rainbow_enabled.v==1) scr_putpixel_zoom_rainbow(x+margenx_izq,y+margenx_der,color);
			else scr_putpixel_zoom(x,y,color);

			Y considerando el espacio de coordenadas x e y con zoom
		*/

		int xinicio=screen_get_emulated_display_width_zoom_border_en();
		int yinicio=0;

		int ancho=screen_get_ext_desktop_width_zoom();
		int alto=screen_get_emulated_display_height_zoom_border_en();

		int x,y;
		


		//Color solido
		if (menu_ext_desktop_fill==0) {

			int color=menu_ext_desktop_fill_solid_color;

			for (y=yinicio;y<yinicio+alto;y++) {
				for (x=xinicio;x<xinicio+ancho;x++) {
					scr_putpixel(x,y,color);
				}
			}

		}			
		
		//Rayas diagonales de colores
		if (menu_ext_desktop_fill==1) {

			int grueso_lineas=8*zoom_x*menu_gui_zoom; //Para que coincida el color con rainbow de titulo de ventanas
 			int total_colores=5;

			int contador_color;

			for (y=yinicio;y<yinicio+alto;y++) {
				contador_color=y; //Para dar un aspecto de rayado

				for (x=xinicio;x<xinicio+ancho;x++) {
					int indice_color=(contador_color/grueso_lineas) % total_colores;
					int color=screen_colores_rainbow_nobrillo[indice_color]; 
					scr_putpixel(x,y,color);

					contador_color++;
				}
			}

		}

		//punteado
		if (menu_ext_desktop_fill==2) {

			int color;

			for (y=yinicio;y<yinicio+alto;y++) {
				for (x=xinicio;x<xinicio+ancho;x++) {

					int suma=x+y;
					color=(suma & 1 ? 0 : 15);

					scr_putpixel(x,y,color);
				}
			}

		}	


	//Agregamos logo ZEsarUX en esquina inferior derecha, con margen
	int xfinal=xinicio+ancho-ZESARUX_ASCII_LOGO_ANCHO-ZESARUX_WATERMARK_LOGO_MARGIN;
	int yfinal=alto-ZESARUX_ASCII_LOGO_ALTO-ZESARUX_WATERMARK_LOGO_MARGIN;

	//El ancho y el puntero dan igual, no los vamos a usar
	screen_put_watermark_generic(NULL,xfinal,yfinal,0, menu_draw_ext_desktop_logo);
	
}

//refresco de pantalla, avisando cambio de border, 
void menu_refresca_pantalla(void)
{

	modificado_border.v=1;
    all_interlace_scr_refresca_pantalla();

	//necesario si hay efectos de darken o grayscale
	//menu_clear_footer();

	//y redibujar todo footer
	redraw_footer();


	menu_draw_ext_desktop();

}

//Borra la pantalla del menu, refresca la pantalla de spectrum
void menu_cls_refresh_emulated_screen()
{

                cls_menu_overlay();

				menu_refresca_pantalla();

}

void enable_footer(void)
{

        menu_footer=1;

        //forzar redibujar algunos contadores
        draw_bateria_contador=0;

}

void disable_footer(void)
{

        menu_footer=0;

        

}





//retornar puntero a campo desde texto, separado por espacios. se permiten multiples espacios entre campos
char *menu_get_cpu_use_idle_value(char *m,int campo)
{

	char c;

	while (campo>1) {
		c=*m;

		if (c==' ') {
			while (*m==' ') m++;
			campo--;
		}

		else m++;
	}

	return m;
}


long menu_cpu_use_seconds_antes=0;
long menu_cpu_use_seconds_ahora=0;
long menu_cpu_use_idle_antes=0;
long menu_cpu_use_idle_ahora=0;

int menu_cpu_use_num_cpus=1;

//devuelve valor idle desde /proc/stat de cpu
//devuelve <0 si error

//long temp_idle;

long menu_get_cpu_use_idle(void)
{

	//printf ("llamando a menu_get_cpu_use_idle\n");

//En Mac OS X, obtenemos consumo cpu de este proceso
#if defined(__APPLE__)

	struct rusage r_usage;

	if (getrusage(RUSAGE_SELF, &r_usage)) {
		return -1;
	    /* ... error handling ... */
	}

	//printf("Total User CPU = %ld.%d\n",        r_usage.ru_utime.tv_sec,        r_usage.ru_utime.tv_usec);

	long cpu_use_mac=r_usage.ru_utime.tv_sec*100+(r_usage.ru_utime.tv_usec/10000);   //el 10000 sale de /1000000*100

	//printf ("Valor retorno: %ld\n",cpu_use_mac);

	return cpu_use_mac;

#endif

//En Linux en cambio obtenemos uso de cpu de todo el sistema
//cat /proc/stat
//cpu  2383406 37572370 7299316 91227807 7207258 18372 473173 0 0 0
//     user    nice    system   idle

	//int max_leer=DEBUG_MAX_MESSAGE_LENGTH-200;

	#define MAX_LEER (DEBUG_MAX_MESSAGE_LENGTH-200)

	//dado que hacemos un debug_printf con este texto,
	//el maximo del debug_printf es DEBUG_MAX_MESSAGE_LENGTH. Quitamos 200 que da margen para poder escribir sin
	//hacer segmentation fault

	//metemos +1 para poder poner el 0 del final
	char procstat_buffer[MAX_LEER+1];

	//char buffer_nulo[100];
	//char buffer_idle[100];
	char *buffer_idle;
	long cpu_use_idle=0;

	char *archivo_cpuuse="/proc/stat";

	if (si_existe_archivo(archivo_cpuuse) ) {
		int leidos=lee_archivo(archivo_cpuuse,procstat_buffer,MAX_LEER);

			if (leidos<1) {
				debug_printf (VERBOSE_DEBUG,"Error reading cpu status on %s",archivo_cpuuse);
	                        return -1;
        	        }

			//leidos es >=1

			//temp
			//printf ("leidos: %d DEBUG_MAX_MESSAGE_LENGTH: %d sizeof: %d\n",leidos,DEBUG_MAX_MESSAGE_LENGTH,sizeof(procstat_buffer) );

			//establecemos final de cadena
			procstat_buffer[leidos]=0;

			debug_printf (VERBOSE_PARANOID,"procstat_buffer: %s",procstat_buffer);

			//miramos numero cpus
			menu_cpu_use_num_cpus=0;

			char *p;
			p=procstat_buffer;

			while (p!=NULL) {
				p=strstr(p,"cpu");
				if (p!=NULL) {
					p++;
					menu_cpu_use_num_cpus++;
				}
			}

			if (menu_cpu_use_num_cpus==0) {
				//como minimo habra 1
				menu_cpu_use_num_cpus=1;
			}

			else {
				//se encuentra cabecera con "cpu" y luego "cpu0, cpu1", etc, por tanto,restar 1
				menu_cpu_use_num_cpus--;
			}

			debug_printf (VERBOSE_DEBUG,"cpu number: %d",menu_cpu_use_num_cpus);

			//parsear valores, usamos scanf
			//fscanf(ptr_procstat,"%s %s %s %s %s",buffer_nulo,buffer_nulo,buffer_nulo,buffer_nulo,buffer_idle);

			//parsear valores, usamos funcion propia
			buffer_idle=menu_get_cpu_use_idle_value(procstat_buffer,5);



			if (buffer_idle!=NULL) {
				//ponemos 0 al final
				int i=0;
				while (buffer_idle[i]!=' ') {
					i++;
				}

				buffer_idle[i]=0;


				debug_printf (VERBOSE_DEBUG,"idle value: %s",buffer_idle);

				cpu_use_idle=atoi(buffer_idle);
			}
	}

	else {
		cpu_use_idle=-1;
	}

	return cpu_use_idle;

}

void menu_get_cpu_use_perc(void)
{

	int usocpu=0;

	struct timeval menu_cpu_use_time;

	gettimeofday(&menu_cpu_use_time, NULL);
	menu_cpu_use_seconds_ahora=menu_cpu_use_time.tv_sec;

	menu_cpu_use_idle_ahora=menu_get_cpu_use_idle();

	if (menu_cpu_use_idle_ahora<0) {
		menu_last_cpu_use=-1;
		return;
	}

	if (menu_cpu_use_seconds_antes!=0) {
		long dif_segundos=menu_cpu_use_seconds_ahora-menu_cpu_use_seconds_antes;
		long dif_cpu_idle=menu_cpu_use_idle_ahora-menu_cpu_use_idle_antes;

		debug_printf (VERBOSE_PARANOID,"sec now: %ld before: %ld cpu now: %ld before: %ld",menu_cpu_use_seconds_ahora,menu_cpu_use_seconds_antes,
			menu_cpu_use_idle_ahora,menu_cpu_use_idle_antes);

		long uso_cpu_idle;

		//proteger division por cero
		if (dif_segundos==0) uso_cpu_idle=100;
		else uso_cpu_idle=dif_cpu_idle/dif_segundos/menu_cpu_use_num_cpus;

#if defined(__APPLE__)
		debug_printf (VERBOSE_PARANOID,"cpu use: %ld",uso_cpu_idle);
		usocpu=uso_cpu_idle;
#else
		debug_printf (VERBOSE_PARANOID,"cpu idle: %ld",uso_cpu_idle);
		//pasamos a int
		usocpu=100-uso_cpu_idle;
#endif
	}

	menu_cpu_use_seconds_antes=menu_cpu_use_seconds_ahora;
	menu_cpu_use_idle_antes=menu_cpu_use_idle_ahora;

	menu_last_cpu_use=usocpu;
}

int cpu_use_total_acumulado=0;
int cpu_use_total_acumulado_medidas=0;

void menu_draw_cpu_use_last(void)
{

	int cpu_use=menu_last_cpu_use;
	debug_printf (VERBOSE_PARANOID,"cpu: %d",cpu_use );

	//error
	if (cpu_use<0) return;

	//control de rango
	if (cpu_use>100) cpu_use=100;
	if (cpu_use<0) cpu_use=0;

	//temp
	//cpu_use=100;

	//printf ("mostrando cpu use\n");

	char buffer_perc[9];
	sprintf (buffer_perc,"%3d%% CPU",cpu_use);

	int x;

	x=WINDOW_FOOTER_ELEMENT_X_CPU_USE;

	int color_tinta=WINDOW_FOOTER_INK;

	//Color en rojo si uso cpu sube
	if (cpu_use>=85) color_tinta=ESTILO_GUI_COLOR_AVISO;

	menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_CPU_USE,buffer_perc,color_tinta,WINDOW_FOOTER_PAPER);

}

void menu_draw_cpu_use(void)
{

        //solo redibujarla de vez en cuando
        if (draw_cpu_use!=0) {
                draw_cpu_use--;
                return;
        }

        //cada 5 segundos
        draw_cpu_use=50*5;

	menu_get_cpu_use_perc();

	int cpu_use=menu_last_cpu_use;
	debug_printf (VERBOSE_PARANOID,"cpu: %d",cpu_use );

	//error
	if (cpu_use<0) return;

	//control de rango
	if (cpu_use>100) cpu_use=100;
	if (cpu_use<0) cpu_use=0;


	cpu_use_total_acumulado +=cpu_use;
	cpu_use_total_acumulado_medidas++;	

	menu_draw_cpu_use_last();

}



//Retorna -1 si hay algun error
int menu_get_cpu_temp(void)
{

	char procstat_buffer[10];

	//sensor generico
	char *posible_archivo_cputemp1="/sys/class/thermal/thermal_zone0/temp";

	//sensor especifico para mi pc linux
	char *posible_archivo_cputemp2="/sys/devices/platform/smsc47b397.1152/hwmon/hwmon0/temp1_input";

	char *archivo_cputemp;

	if (si_existe_archivo(posible_archivo_cputemp1) ) {
		archivo_cputemp=posible_archivo_cputemp1;
	}

	else if (si_existe_archivo(posible_archivo_cputemp2) ) {
		archivo_cputemp=posible_archivo_cputemp2;
	}

	else return -1;


	int leidos=lee_archivo(archivo_cputemp,procstat_buffer,9);

	if (leidos<1) {
        debug_printf (VERBOSE_DEBUG,"Error reading cpu status on %s",archivo_cputemp);
        return -1;
    }

    //establecemos final de cadena
    procstat_buffer[leidos]=0;


	return atoi(procstat_buffer);
	
}

void menu_draw_cpu_temp(void)
{
        //solo redibujarla de vez en cuando
        if (draw_cpu_temp!=0) {
                draw_cpu_temp--;
                return;
        }

        //cada 5 segundos
        draw_cpu_temp=50*5;

        int cpu_temp=menu_get_cpu_temp();
        debug_printf (VERBOSE_DEBUG,"CPU temp: %d",cpu_temp );

	//algun error al leer temperatura
	if (cpu_temp<0) return;

        //control de rango
        if (cpu_temp>99999) cpu_temp=99999;


        //temp forzar
        //cpu_temp=100;

        char buffer_temp[6];

		int grados_entero=cpu_temp/1000; //2 cifras
		int grados_decimal=(cpu_temp%1000)/100; //1 cifra

        sprintf (buffer_temp,"%2d.%dC",grados_entero,grados_decimal );

        //primero liberar esas zonas
        int x;

	int color_tinta=WINDOW_FOOTER_INK;

	//Color en rojo si temperatura alta
	if (grados_entero>=80) color_tinta=ESTILO_GUI_COLOR_AVISO;


        //luego escribimos el texto
        x=WINDOW_FOOTER_ELEMENT_X_CPU_TEMP;


	menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_CPU_TEMP,buffer_temp,color_tinta,WINDOW_FOOTER_PAPER);
}

void menu_draw_last_fps(void)
{


        int fps=ultimo_fps;
        debug_printf (VERBOSE_PARANOID,"FPS: %d",fps);

        //algun error al leer fps
        if (fps<0) return;

        //control de rango
        if (fps>50) fps=50;

	const int ancho_maximo=6;

			//printf ("mostrando fps\n");	

        char buffer_fps[ancho_maximo+1];
        sprintf (buffer_fps,"%02d FPS",fps);

        //primero liberar esas zonas
        int x;


        //luego escribimos el texto
        x=WINDOW_FOOTER_ELEMENT_X_FPS;


	menu_putstring_footer(x,WINDOW_FOOTER_ELEMENT_Y_FPS,buffer_fps,WINDOW_FOOTER_INK,WINDOW_FOOTER_PAPER);
}

void menu_draw_fps(void)
{

	        //solo redibujarla de vez en cuando
        if (draw_fps!=0) {
                draw_fps--;
                return;
        }



        //cada 1 segundo
        draw_fps=50*1;

		menu_draw_last_fps();

}



int menu_get_bateria_perc(void)
{
        //temp forzar
        return 25;

}






//Aqui se llama desde cada driver de video al refrescar la pantalla
//Importante que lo que se muestre en footer se haga cada cierto tiempo y no siempre, sino saturaria la cpu probablemente
void draw_middle_footer(void)
{

	if (menu_footer==0) return;

	//temp forzado
	//menu_draw_cpu_temp();

//Temperatura mostrarla en raspberry y en general en Linux
//#ifdef EMULATE_RASPBERRY
#ifdef __linux__
    menu_draw_cpu_temp();
#endif

	if (screen_show_cpu_usage.v) {
		menu_draw_cpu_use();
	}

	menu_draw_fps();


      

//01234567890123456789012345678901
//50 FPS 100% CPU 99.9C TEMP

}


//0 si no valido
//1 si valido
int si_valid_char(z80_byte caracter)
{
	if (si_complete_video_driver() ) {
		if (caracter<32 || caracter>MAX_CHARSET_GRAPHIC) return 0;
	}

	else {
		if (caracter<32 || caracter>127) return 0;
	}

	return 1;
}


//funcion normal de impresion de overlay de buffer de texto y cuadrado de lineas usado en los menus
void normal_overlay_texto_menu(void)
{

	int x,y;
	z80_byte tinta,papel,caracter,parpadeo;
	int pos_array=0;


	//printf ("normal_overlay_texto_menu\n");
	for (y=0;y<scr_get_menu_height();y++) {
		for (x=0;x<scr_get_menu_width();x++,pos_array++) {
			caracter=overlay_screen_array[pos_array].caracter;
			//si caracter es 0, no mostrar

			//if (overlay_usado_screen_array[pos_array]) {
			if (caracter) {
				//128 y 129 corresponden a franja de menu y a letra enye minuscula
				if (si_valid_char(caracter) ) {
					tinta=overlay_screen_array[pos_array].tinta;
					papel=overlay_screen_array[pos_array].papel;
					parpadeo=overlay_screen_array[pos_array].parpadeo;

					//Si esta multitask, si es caracter con parpadeo y si el estado del contador del parpadeo indica parpadear
					if (menu_multitarea && parpadeo && estado_parpadeo.v) caracter=' '; //si hay parpadeo y toca, meter espacio tal cual (se oculta)

					scr_putchar_menu(x,y,caracter,tinta,papel);
				}

				else if (caracter==255) {
					//Significa no mostrar caracter. Usado en pantalla panic
				}

				//Si caracter no valido, mostrar ?
				else {
					tinta=overlay_screen_array[pos_array].tinta;
					papel=overlay_screen_array[pos_array].papel;
					scr_putchar_menu(x,y,'?',tinta,papel);
				}
			}
		}
	}

	if (cuadrado_activo && ventana_tipo_activa) {
		menu_dibuja_cuadrado(cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color);

		//Y si tiene marca de redimensionado
		//if (cuadrado_activo_resize) menu_dibuja_cuadrado_resize(cuadrado_x1,cuadrado_y1,cuadrado_x2,cuadrado_y2,cuadrado_color);
	}




}


//establece cuadrado activo usado en los menus para xwindows y fbdev
void menu_establece_cuadrado(int x1,int y1,int x2,int y2,z80_byte color)
{

	cuadrado_x1=x1;
	cuadrado_y1=y1;
	cuadrado_x2=x2;
	cuadrado_y2=y2;
	cuadrado_color=color;
	cuadrado_activo=1;

	//Por defecto no se ve marca de resize, para compatibilidad con ventanas no zxvision
	cuadrado_activo_resize=0;
	ventana_activa_tipo_zxvision=0;

}

//desactiva cuadrado  usado en los menus para xwindows y fbdev
void menu_desactiva_cuadrado(void)
{
	cuadrado_activo=0;
	cuadrado_activo_resize=0;
	ventana_activa_tipo_zxvision=0;
}

//Devuelve 1 si hay dos ~~ seguidas en la posicion del indice o ~^ 
//Sino, 0
int menu_escribe_texto_si_inverso(char *texto, int indice)
{

	if (menu_disable_special_chars.v) return 0;

	if (texto[indice++]!='~') return 0;
	if (texto[indice]!='~' && texto[indice]!='^') {
		return 0;
	}

	indice++;

	//Y siguiente caracter no es final de texto
	if (texto[indice]==0) return 0;

	return 1;
}

//Devuelve 1 si hay dos ^^ seguidas en la posicion del indice
//Sino, 0
int menu_escribe_texto_si_parpadeo(char *texto, int indice)
{

	if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='^') return 0;
    if (texto[indice++]!='^') return 0;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;
}


int menu_escribe_texto_si_cambio_tinta(char *texto,int indice)
{
	if (menu_disable_special_chars.v) return 0;

    if (texto[indice++]!='$') return 0;
    if (texto[indice++]!='$') return 0;
	if (texto[indice]<'0' || texto[indice]>'7'+8) return 0; //Soportar colores con brillo
	indice++;

    //Y siguiente caracter no es final de texto
    if (texto[indice]==0) return 0;

    return 1;

}

//Quita simbolos ^^y ~~ y $$X de un texto. Puede que esta funcion este repetida en algun otro sitio
void menu_convierte_texto_sin_modificadores(char *texto,char *texto_destino)
{
	int origen,destino;

	char c;

	for (origen=0,destino=0;texto[origen];origen++,destino++) {
		//printf ("origen: %d destino: %d\n",origen,destino);
		if (menu_escribe_texto_si_inverso(texto,origen) || menu_escribe_texto_si_parpadeo(texto,origen) ) {
			origen +=2;
		}

		else if (menu_escribe_texto_si_cambio_tinta(texto,origen)) {
			origen +=3;
		}

		else {
			c=texto[origen];
			texto_destino[destino]=c;
		}
		//printf ("origen: %d destino: %d\n",origen,destino);
	}

	texto_destino[destino]=0;

}

int menu_es_prefijo_utf(z80_byte caracter)
{
	if (caracter==0xD0 || caracter==0xD1 || caracter==0xC3) return 1;
	else return 0;
}

unsigned char menu_escribe_texto_convert_utf(unsigned char prefijo_utf,unsigned char caracter)
{

	if (prefijo_utf==0xC3) {
		if (caracter==0xB1) {
			//Eñe
			if (si_complete_video_driver()) {
                                return 129; //Eñe
                        }
                        else {
                                return 'n';
                        }
                }

	}

	if (prefijo_utf==0xD0) {
		if (caracter==0x90) return 'A';
		if (caracter==0x9C) return 'M'; //cyrillic capital letter em (U+041C)
		if (caracter==0xA1) return 'C';
		if (caracter==0xA8) { //Ш
			if (si_complete_video_driver()) {
				return 131;
			}
			else {
				return 'W';
			}
		}
		if (caracter==0xB0) return 'a';
		if (caracter==0xB2) return 'B';

		

		if (caracter==0xB3) {
			if (si_complete_video_driver()) {
				return 133; //г
			}
			else {
				return 'g';
			}
		}

		if (caracter==0xB4) {
			if (si_complete_video_driver()) {
				return 135; //д
			}
			else {
				return 'D';
			}
		}

		if (caracter==0xB5) return 'e';
		if (caracter==0xB8) {
			if (si_complete_video_driver()) {
				return 130; //CYRILLIC SMALL LETTER I и
			}
			else {
				return 'i';
			}
		}
		if (caracter==0xBA) return 'k';
		if (caracter==0xBB) { //л
			if (si_complete_video_driver()) {
				return 132;
			}
			else {
				return 'l';
			}
		}		
		if (caracter==0xBC) return 'M';
		if (caracter==0xBD) return 'H';
		if (caracter==0xBE) return 'o';
	}

	if (prefijo_utf==0xD1) {
				if (caracter==0x80) return 'p';
				if (caracter==0x81) return 'c';
                if (caracter==0x82) return 'T';
                if (caracter==0x83) return 'y';
                if (caracter==0x85) return 'x';



				if (caracter==0x87) {
					if (si_complete_video_driver()) {
						return 134; //ч
					}
					else {
						return 'y';
					}
				}

				//я
				if (caracter==0x8F) {
					if (si_complete_video_driver()) {
						return 136; //я
					}
					else {
						return 'a'; //no es lo mismo, sonaria como una "ja" en dutch, pero bueno
					}
				}				

        }

	return '?';


	//Nota: caracteres que generan texto fuera de la tabla normal, considerar si es un driver de texto o grafico, con if (si_complete_video_driver() ) {
}


//escribe una linea de texto
//coordenadas relativas al interior de la pantalla de spectrum (0,0=inicio pantalla)

//Si codigo de color inverso, invertir una letra
//Codigo de color inverso: dos ~ seguidas
void menu_escribe_texto(z80_byte x,z80_byte y,z80_byte tinta,z80_byte papel,char *texto)
{
        unsigned int i;
	z80_byte letra;

	int parpadeo=0;

	int era_utf=0;

    //y luego el texto
    for (i=0;i<strlen(texto);i++) {
		letra=texto[i];

		//Si dos ^ seguidas, invertir estado parpadeo
		if (menu_escribe_texto_si_parpadeo(texto,i)) {
			parpadeo ^=1;
			//y saltamos esos codigos de negado
                        i +=2;
                        letra=texto[i];
		}

		//codigo control color tinta
		if (menu_escribe_texto_si_cambio_tinta(texto,i)) {
			tinta=texto[i+2]-'0';
			i+=3;
			letra=texto[i];
		}

		//ver si dos ~~ seguidas y cuidado al comparar que no nos vayamos mas alla del codigo 0 final
		if (menu_escribe_texto_si_inverso(texto,i)) {
			//y saltamos esos codigos de negado
			i +=2;
			letra=texto[i];

			if (menu_writing_inverse_color.v) putchar_menu_overlay_parpadeo(x,y,letra,papel,tinta,parpadeo);
			else putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
		}

		else {

			//Si estaba prefijo utf activo

			if (era_utf) {
				letra=menu_escribe_texto_convert_utf(era_utf,letra);
				era_utf=0;

				//Caracter final utf
				putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
			}


			//Si no, ver si entra un prefijo utf
			else {
				//printf ("letra: %02XH\n",letra);
				//Prefijo utf
                	        if (menu_es_prefijo_utf(letra)) {
        	                        era_utf=letra;
					//printf ("activado utf\n");
	                        }

				else {
					//Caracter normal
					putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
				}
			}


		}

		//if (x>=32) {
		//	printf ("Escribiendo caracter [%c] en x: %d\n",letra,x);
		//}


		if (!era_utf) x++;
	}

}



//escribe una linea de texto
//coordenadas relativas al interior de la ventana (0,0=inicio zona "blanca")
void menu_escribe_texto_ventana(z80_byte x,z80_byte y,z80_byte tinta,z80_byte papel,char *texto)
{

	menu_escribe_texto(current_win_x+x,current_win_y+y+1,tinta,papel,texto);


}

int menu_if_speech_enabled(void)
{
        if (textspeech_filter_program==NULL) return 0;
        if (textspeech_also_send_menu.v==0) return 0;
        if (menu_speech_tecla_pulsada) return 0;

	return 1;
}

void menu_textspeech_filter_corchetes(char *texto_orig,char *texto)
{
	char texto_active_item[32]=""; //Inicializado a vacio de momento
	int inicio_corchete=0;

	//Buscar si empieza con "Selected item: "
	char *encontrado;

	char *cadena_buscar="Selected item: ";

    encontrado=strstr(texto_orig,cadena_buscar);
    if (encontrado==texto_orig) {
		//Avanzamos el indice a inicio a buscar
		inicio_corchete=strlen(cadena_buscar);
		//Y metemos cadena "prefijo"
		strcpy(texto_active_item,cadena_buscar);

		//printf ("Encontrado texto Selected item en %s\n",texto_orig);
	}

	//char texto[MAX_BUFFER_SPEECH+1];

	//buscar primero si hay [ ] al principio
	
	int cambiado=0;
	//printf ("texto: %s. inicio corchete %d\n",texto_orig,inicio_corchete);
	if (texto_orig[inicio_corchete]=='[') {
		//posible
		int i;
		for (i=inicio_corchete;texto_orig[i]!=0 && !cambiado;i++) { 
			//printf ("%d\n",i);
			if (texto_orig[i]==']') {
				//Hay inicio con [..]. Ponerlo al final en nueva string
				char buf_opcion[MAX_BUFFER_SPEECH+1];
				strcpy(buf_opcion,&texto_orig[inicio_corchete]);

				int longitud_opcion=(i+1)-inicio_corchete;

				//buf_opcion[i+1]=0;  //buf_opcion contiene solo los corchetes y lo de dentro de corchetes
				buf_opcion[longitud_opcion]=0;  //buf_opcion contiene solo los corchetes y lo de dentro de corchetes

				//Y ahora ademas, si la opcion es [ ] dice disabled. Si es [x] dice enabled
				//TODO: solo estamos detectando esto a principio de linea. Creo que no hay ningun menu en que diga [ ] o [X] en otro
				//sitio que no sea principio de linea. Si estuviera en otro sitio, no funcionaria
				if (!strcmp(buf_opcion,"[ ]")) strcpy(buf_opcion,"Disabled");
				else if (!strcmp(buf_opcion,"[X]")) strcpy(buf_opcion,"Enabled");

				sprintf(texto,"%s%s. %s",texto_active_item,&texto_orig[i+1],buf_opcion);
				//printf ("Detected setting at the beginning of the line. Changing speech to menu item and setting: %s\n",texto);
				cambiado=1;
			}
		}
	}

	if (!cambiado) strcpy(texto,texto_orig);
	
	
}

void menu_textspeech_send_text(char *texto_orig)
{

	if (!menu_if_speech_enabled() ) return;


	debug_printf (VERBOSE_DEBUG,"Send text to speech: %s",texto_orig);
	
	
	
	//-si item empieza por [, buscar hasta cierre ]. Y eso se reproduce al final de linea
	//-Si [X], se dice "enabled". Si [ ], se dice "disabled"
	//TODO: aqui se llama tambien al decir directorios por ejemplo. Si hay directorio con [ ] (cosa rara) se interpretaria como una opcion
	//y se diria al final

	//Detectar tambien si al principio se dice "Selected item: "
	//Esto por tanto solo servira cuando el [] esta a principio de linea o bien despues de "Selected item: "
	//Se podria extender a localizar los [] en cualquier sitio pero entonces el problema podria venir por alguna linea
	//que tuviera [] en medio y no fuera de menu, y la moviese al final

	char texto[MAX_BUFFER_SPEECH+1];
	menu_textspeech_filter_corchetes(texto_orig,texto);




	//Eliminamos las ~~ o ^^ del texto. Realmente eliminamos cualquier ~ aunque solo haya una
	//Si hay dos ~~, decir que atajo es al final del texto
	int orig,dest;
	orig=dest=0;
	char buf_speech[MAX_BUFFER_SPEECH+1];

	char letra_atajo=0;

	//printf ("texto: puntero: %d strlen: %d : %s\n",texto,strlen(texto),texto);

	for (;texto[orig]!=0;orig++) {

		if (texto[orig]=='~' && texto[orig+1]=='~') {
			letra_atajo=texto[orig+2];
		}

		//printf ("texto orig : %d\n",texto[orig]);
		//printf ("texto orig char: %c\n",texto[orig]);
		//TODO: saltar cuando hay cambio de color de tinta %%X
		//Si no es ~ ni ^, copiar e incrementar destino
		if (texto[orig]!='~' && texto[orig]!='^') {
			buf_speech[dest]=texto[orig];
			dest++;
		}
	}


	//Si se ha encontrado letra atajo
	if (letra_atajo!=0) {
		buf_speech[dest++]='.';
		buf_speech[dest++]=' ';
		//Parece que en los sistemas de speech la letra mayuscula se lee con mas pausa (al menos testado con festival)
		buf_speech[dest++]=letra_mayuscula(letra_atajo);
		buf_speech[dest++]='.';
	}

	buf_speech[dest]=0;


	//printf ("directorio antes: [%s]\n",buf_speech);

	//Si hay texto <dir> cambiar por directory
	char *p;
	p=strstr(buf_speech,"<dir>");
	if (p) {
		//suponemos que esto va a final de texto
		sprintf (p,"%s","directory");
	}

	//Si directorio es ".."
	if (!strcmp(buf_speech,".. directory") || !strcmp(buf_speech,"..                     directory") ) {
		strcpy(buf_speech,"dot dot directory");
	}

	//Si directorio es ".."
	if (!strcmp(buf_speech,"Selected item: .. directory")) {
		strcpy(buf_speech,"Selected item: dot dot directory");
	}


	//Si es todo espacios sin ningun caracter, no enviar
	int vacio=1;
	for (orig=0;buf_speech[orig]!=0;orig++) {
		if (buf_speech[orig]!=' ') {
			vacio=0;
			break;
		}
	}

	if (vacio==1) {
		debug_printf (VERBOSE_DEBUG,"Empty line, do not send to speech");
		return;
	}





	debug_printf (VERBOSE_DEBUG,"Final sent text to speech after processing filters: %s",buf_speech);
	textspeech_print_speech(buf_speech);
	//printf ("textspeech_print_speech: %s\n",buf_speech);

	//hacemos que el timeout de tooltip se reinicie porque sino cuando se haya leido el menu, acabara saltando el timeout
	//menu_tooltip_counter=0;

	z80_byte acumulado;


	/*
	int contador_refresco=0;
	int max_contador_refresco;

	//Algo mas de un frame de pantalla
	if (menu_multitarea==1) max_contador_refresco=100000;

	//Cada 20 ms
	//Nota: Ver funcion  menu_cpu_core_loop(void) , donde hay usleep(500) en casos de menu_multitarea 0
	else max_contador_refresco=40;
	*/

		//Parece que esto en maquinas lentas (especialmente en mi Windows virtual). Bueno creo que realmente no es un problema de Windows,
		//si no de que la maquina es muy lenta y tarda en refrescar la pantalla mientras esta
		//esperando una tecla y reproduciendo speech. Si quito esto, sucede que
		//si se pulsa el cursor rapido y hay speech, dicho cursor va "con retraso" una posicion
	menu_refresca_pantalla();

        do {
                if (textspeech_finalizado_hijo_speech() ) scrtextspeech_filter_run_pending();

                menu_cpu_core_loop();
                acumulado=menu_da_todas_teclas();


                        //Hay tecla pulsada
                        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
				//printf ("pulsada tecla\n");
                                //int tecla=menu_get_pressed_key();

				//de momento cualquier tecla anula speech
				textspeech_empty_speech_fifo();

				menu_speech_tecla_pulsada=1;

                        }

			//no hay tecla pulsada
			else {
				//Decir que no repeticion de tecla. Si no pusiesemos esto aqui,
				//pasaria que si entramos con repeticion activa, y
				//mientras esperamos a que acabe proceso hijo, no pulsamos una tecla,
				//la repeticion seguiria activa
				menu_reset_counters_tecla_repeticion();
			}

		//Parece que esto en maquinas lentas (especialmente en mi Windows virtual). Bueno creo que realmente no es un problema de Windows,
		//si no de que la maquina es muy lenta y tarda en refrescar la pantalla mientras esta
		//esperando una tecla y reproduciendo speech. Si quito esto, sucede que
		//si se pulsa el cursor rapido y hay speech, dicho cursor va "con retraso" una posicion
		/*contador_refresco++;
		if (contador_refresco==max_contador_refresco) {
			printf ("refrescar\n");
			contador_refresco=0;
			menu_refresca_pantalla();
		}
		*/


        } while (!textspeech_finalizado_hijo_speech() && menu_speech_tecla_pulsada==0);
	//hacemos que el timeout de tooltip se reinicie porque sino cuando se haya leido el menu, acabara saltando el timeout
	menu_tooltip_counter=0;

}

void menu_retorna_colores_linea_opcion(int indice,int opcion_actual,int opcion_activada,z80_byte *papel_orig,z80_byte *tinta_orig)
{
	z80_byte papel,tinta;

	/*
	4 combinaciones:
	opcion seleccionada, disponible (activada)
	opcion seleccionada, no disponible
	opcion no seleccionada, disponible
	opcion no seleccionada, no disponible
	*/

        if (opcion_actual==indice) {
                if (opcion_activada==1) {
                        papel=ESTILO_GUI_PAPEL_SELECCIONADO;
                        tinta=ESTILO_GUI_TINTA_SELECCIONADO;
                }
                else {
                        papel=ESTILO_GUI_PAPEL_SEL_NO_DISPONIBLE;
                        tinta=ESTILO_GUI_TINTA_SEL_NO_DISPONIBLE;
                }
        }

        else {
                if (opcion_activada==1) {
                        papel=ESTILO_GUI_PAPEL_NORMAL;
                        tinta=ESTILO_GUI_TINTA_NORMAL;
                }
                else {
                        papel=ESTILO_GUI_PAPEL_NO_DISPONIBLE;
                        tinta=ESTILO_GUI_TINTA_NO_DISPONIBLE;
                }
        }

	*papel_orig=papel;
	*tinta_orig=tinta;

}

//escribe opcion de linea de texto
//coordenadas "indice" relativa al interior de la ventana (0=inicio)
//opcion_actual indica que numero de linea es la seleccionada
//opcion activada indica a 1 que esa opcion es seleccionable
void menu_escribe_linea_opcion_zxvision(zxvision_window *ventana,int indice,int opcion_actual,int opcion_activada,char *texto_entrada)
{

	char texto[64];

        if (!strcmp(scr_driver_name,"stdout")) {
		printf ("%s\n",texto_entrada);
		scrstdout_menu_print_speech_macro (texto_entrada);
		return;
	}


	z80_byte papel,tinta;
	int i;

	//tinta=0;


	menu_retorna_colores_linea_opcion(indice,opcion_actual,opcion_activada,&papel,&tinta);


	//Obtenemos colores de una opcion sin seleccion y activada, para poder tener texto en ventana con linea en dos colores
	z80_byte papel_normal,tinta_normal;
	menu_retorna_colores_linea_opcion(0,-1,1,&papel_normal,&tinta_normal);

	//Buscamos a ver si en el texto hay el caracter "||" y en ese caso lo eliminamos del texto final
	int encontrado=-1;
	int destino=0;
	for (i=0;texto_entrada[i];i++) {
		if (menu_disable_special_chars.v==0 && texto_entrada[i]=='|' && texto_entrada[i+1]=='|') {
			encontrado=i;
			i ++;
		}
		else {
			texto[destino++]=texto_entrada[i];
		}
	}

	texto[destino]=0;


	//linea entera con espacios
	for (i=0;i<current_win_ancho;i++) {
		zxvision_print_string(ventana,i,indice,0,papel,0," ");
	}

	//y texto propiamente
	int startx=menu_escribe_linea_startx;
	zxvision_print_string(ventana,startx,indice,tinta,papel,0,texto);

	//Si tiene dos colores
	if (encontrado>=0) {
		//menu_escribe_texto_ventana(startx+encontrado,indice,tinta_normal,papel_normal,&texto[encontrado]);
		zxvision_print_string(ventana,startx+encontrado,indice,tinta_normal,papel_normal,0,&texto[encontrado]);
	}

	//si el driver de video no tiene colores o si el estilo de gui lo indica, indicamos opcion activa con un cursor
	if (!scr_tiene_colores || ESTILO_GUI_MUESTRA_CURSOR) {
		if (opcion_actual==indice) {
			if (opcion_activada==1) {
				//menu_escribe_texto_ventana(0,indice,tinta,papel,">");
				zxvision_print_string(ventana,0,indice,tinta,papel,0,">");
			}
			else {
				//menu_escribe_texto_ventana(0,indice,tinta,papel,"x");
				zxvision_print_string(ventana,0,indice,tinta,papel,0,"x");
			}
		}
	}
	if (menu_if_speech_enabled() ) {
    	//printf ("redibujar ventana\n");
        zxvision_draw_window_contents_no_speech(ventana);
        //menu_refresca_pantalla();
    }

	menu_textspeech_send_text(texto);




}


//escribe opcion de linea de texto
//coordenadas "indice" relativa al interior de la ventana (0=inicio)
//opcion_actual indica que numero de linea es la seleccionada
//opcion activada indica a 1 que esa opcion es seleccionable
void menu_escribe_linea_opcion(int indice,int opcion_actual,int opcion_activada,char *texto_entrada)
{

	char texto[64];

        if (!strcmp(scr_driver_name,"stdout")) {
		printf ("%s\n",texto_entrada);
		scrstdout_menu_print_speech_macro (texto_entrada);
		return;
	}


	z80_byte papel,tinta;
	int i;

	//tinta=0;


	menu_retorna_colores_linea_opcion(indice,opcion_actual,opcion_activada,&papel,&tinta);


	//Obtenemos colores de una opcion sin seleccion y activada, para poder tener texto en ventana con linea en dos colores
	z80_byte papel_normal,tinta_normal;
	menu_retorna_colores_linea_opcion(0,-1,1,&papel_normal,&tinta_normal);

	//Buscamos a ver si en el texto hay el caracter "||" y en ese caso lo eliminamos del texto final
	int encontrado=-1;
	int destino=0;
	for (i=0;texto_entrada[i];i++) {
		if (menu_disable_special_chars.v==0 && texto_entrada[i]=='|' && texto_entrada[i+1]=='|') {
			encontrado=i;
			i ++;
		}
		else {
			texto[destino++]=texto_entrada[i];
		}
	}

	texto[destino]=0;


	//linea entera con espacios
	for (i=0;i<current_win_ancho;i++) menu_escribe_texto_ventana(i,indice,0,papel," ");

	//y texto propiamente
	int startx=menu_escribe_linea_startx;
        menu_escribe_texto_ventana(startx,indice,tinta,papel,texto);

	//Si tiene dos colores
	if (encontrado>=0) {
		menu_escribe_texto_ventana(startx+encontrado,indice,tinta_normal,papel_normal,&texto[encontrado]);
	}

	//si el driver de video no tiene colores o si el estilo de gui lo indica, indicamos opcion activa con un cursor
	if (!scr_tiene_colores || ESTILO_GUI_MUESTRA_CURSOR) {
		if (opcion_actual==indice) {
			if (opcion_activada==1) menu_escribe_texto_ventana(0,indice,tinta,papel,">");
			else menu_escribe_texto_ventana(0,indice,tinta,papel,"x");
		}
	}
	menu_textspeech_send_text(texto);

}




//escribe opcion de texto tabulado
//coordenadas "indice" relativa al interior de la ventana (0=inicio)
//opcion_actual indica que numero de linea es la seleccionada
//opcion activada indica a 1 que esa opcion es seleccionable
void menu_escribe_linea_opcion_tabulado_zxvision(zxvision_window *ventana,int indice,int opcion_actual,int opcion_activada,char *texto,int x,int y)
{

        if (!strcmp(scr_driver_name,"stdout")) {
                printf ("%s\n",texto);
                scrstdout_menu_print_speech_macro (texto);
                return;
        }


        z80_byte papel,tinta;


        menu_retorna_colores_linea_opcion(indice,opcion_actual,opcion_activada,&papel,&tinta);


		zxvision_print_string(ventana,x,y,tinta,papel,0,texto);
		//printf ("Escribiendo texto tabulado %s en %d,%d\n",texto,x,y);

        menu_textspeech_send_text(texto);

}

void menu_retorna_margenes_border(int *miz, int *mar)
{
	//margenes de zona interior de pantalla. para modo rainbow
	int margenx_izq=screen_total_borde_izquierdo*border_enabled.v;
	int margeny_arr=screen_borde_superior*border_enabled.v;

if (MACHINE_IS_Z88) {
//margenes para realvideo
margenx_izq=margeny_arr=0;
}


	else if (MACHINE_IS_CPC) {
//margenes para realvideo
margenx_izq=CPC_LEFT_BORDER_NO_ZOOM*border_enabled.v;
					margeny_arr=CPC_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_PRISM) {
//margenes para realvideo
margenx_izq=PRISM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
					margeny_arr=PRISM_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_TBBLUE) {
//margenes para realvideo
margenx_izq=TBBLUE_LEFT_BORDER_NO_ZOOM*border_enabled.v;
					margeny_arr=TBBLUE_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}	

	else if (MACHINE_IS_SAM) {
					//margenes para realvideo
					margenx_izq=SAM_LEFT_BORDER_NO_ZOOM*border_enabled.v;
					margeny_arr=SAM_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_QL) {
					//margenes para realvideo
					margenx_izq=QL_LEFT_BORDER_NO_ZOOM*border_enabled.v;
					margeny_arr=QL_TOP_BORDER_NO_ZOOM*border_enabled.v;
	}

	else if (MACHINE_IS_TSCONF) {
		margenx_izq=margeny_arr=0;
	}

	*miz=margenx_izq;
	*mar=margeny_arr;

}


//dibuja cuadrado (4 lineas) usado en los menus para xwindows y fbdev
//Entrada: x1,y1 punto superior izquierda,x2,y2 punto inferior derecha en resolucion de zx spectrum. Color
//nota: realmente no es un cuadrado porque el titulo ya hace de franja superior
void menu_dibuja_cuadrado(int x1,int y1,int x2,int y2,z80_byte color)
{

	if (!ESTILO_GUI_MUESTRA_RECUADRO) return;


	int x,y;



	//Para poner una marca en la ventana indicando si es de tipo zxvision
	int centro_marca_zxvison_x=x2-3-6;
	int centro_marca_zxvison_y=y1+3+2;
		
	//int longitud_marca_zxvision=3;
	//int mitad_long_marca_zxvision=longitud_marca_zxvision/2;
	int color_marca_zxvision=ESTILO_GUI_PAPEL_NORMAL;


	//printf ("Cuadrado %d,%d - %d,%d\n",x1,y1,x2,y2);


	//solo hacerlo en el caso de drivers completos
	if (si_complete_video_driver() ) {

		//if (rainbow_enabled.v) {
		if (1) {

			//parte inferior
			for (x=x1;x<=x2;x++) {
				if (mouse_is_dragging && (x%2)==0) continue; //punteado cuando se mueve o redimensiona
				scr_putpixel_gui_zoom(x*menu_gui_zoom,y2*menu_gui_zoom,color,menu_gui_zoom);
			}


			//izquierda
			for (y=y1;y<=y2;y++) {
				if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
				scr_putpixel_gui_zoom(x1*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
			}

			

			//derecha
			for (y=y1;y<=y2;y++) {
				if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
				scr_putpixel_gui_zoom(x2*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
			}


                      

			//Marca redimensionado
			if (cuadrado_activo_resize) {
				//marca de redimensionado
				//		  *
				//		 **
				//		***	


				//Arriba del todo
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color,menu_gui_zoom);	

				//Medio
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color,menu_gui_zoom);
				scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color,menu_gui_zoom);				

				//Abajo del todo
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);
				scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);
				scr_putpixel_gui_zoom((x2-3)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);
			}

			if (!ventana_activa_tipo_zxvision) {
				//Poner un pixel avisando que ventana no es zxvision
				scr_putpixel_gui_zoom((centro_marca_zxvison_x)*menu_gui_zoom,(centro_marca_zxvison_y)*menu_gui_zoom,color_marca_zxvision,menu_gui_zoom);					
			}				


		}

		/*else {

		
 	               //parte inferior
        	        for (x=x1;x<=x2;x++) {
						if (mouse_is_dragging && (x%2)==0) continue; //punteado cuando se mueve o redimensiona
						scr_putpixel_gui_zoom(x*menu_gui_zoom,y2*menu_gui_zoom,color,menu_gui_zoom);
					}



	                //izquierda
        	        for (y=y1;y<=y2;y++) {
						if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
						scr_putpixel_gui_zoom(x1*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
					}

					

	                //derecha
        	        for (y=y1;y<=y2;y++) {
						if (mouse_is_dragging && (y%2)==0) continue; //punteado cuando se mueve o redimensiona
						scr_putpixel_gui_zoom(x2*menu_gui_zoom,y*menu_gui_zoom,color,menu_gui_zoom);
					}
                               


			//Marca redimensionado
			if (cuadrado_activo_resize) {
				//marca de redimensionado
				//		  *
				//		 **
				//		***	

				//Arriba del todo
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-3)*menu_gui_zoom,color,menu_gui_zoom);

				//Medio
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color,menu_gui_zoom);		
				scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-2)*menu_gui_zoom,color,menu_gui_zoom);	

				//Abajo del todo
				scr_putpixel_gui_zoom((x2-1)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);		
				scr_putpixel_gui_zoom((x2-2)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);	
				scr_putpixel_gui_zoom((x2-3)*menu_gui_zoom,(y2-1)*menu_gui_zoom,color,menu_gui_zoom);							
			}


			if (!ventana_activa_tipo_zxvision) {
				

				//Poner solo un pixel
				scr_putpixel_gui_zoom((centro_marca_zxvison_x)*menu_gui_zoom,(centro_marca_zxvison_y)*menu_gui_zoom,color_marca_zxvision,menu_gui_zoom);					
			}	

		}
		*/
	}


}

void menu_muestra_pending_error_message(void)
{
	if (if_pending_error_message) {
		if_pending_error_message=0;
		debug_printf (VERBOSE_INFO,"Showing pending error message on menu");
		//menu_generic_message("ERROR",pending_error_message);
		menu_error_message(pending_error_message);
	}
}


//x,y origen ventana, ancho ventana
void menu_dibuja_ventana_franja_arcoiris_oscuro(int x, int y, int ancho,int indice)
{

	if (!ventana_tipo_activa) return;

	//int cr[]={2,6,4,5};

	int cr[4]; 
	//Copiar del estilo actual aqui, pues internamente lo modificamos
	int i;
	int *temp_ptr;
	temp_ptr=ESTILO_GUI_FRANJAS_OSCURAS;
	for (i=0;i<4;i++) {
		cr[i]=temp_ptr[i];
	}
	//int *cr;
	//cr=ESTILO_GUI_FRANJAS_OSCURAS;

	//int indice=4-franjas;

	if (indice>=0 && indice<=3) {
		//cr[indice]+=8;
		//Coger color de las normales brillantes
		int *temp_ptr_brillo;
		temp_ptr_brillo=ESTILO_GUI_FRANJAS_NORMALES;
		cr[indice]=temp_ptr_brillo[indice];
	}


	if (ESTILO_GUI_MUESTRA_RAINBOW) {

		if (si_complete_video_driver() ) {
		                	putchar_menu_overlay(x+ancho-6,y,128,cr[0],ESTILO_GUI_PAPEL_TITULO);
        	        	putchar_menu_overlay(x+ancho-5,y,128,cr[1],cr[0]);
                		putchar_menu_overlay(x+ancho-4,y,128,cr[2],cr[1]);
	                	putchar_menu_overlay(x+ancho-3,y,128,cr[3],cr[2]);
        	        	putchar_menu_overlay(x+ancho-2,y,128,ESTILO_GUI_PAPEL_TITULO,cr[3]);
		}

             //en caso de curses o caca, hacerlo con lineas de colores
                if (!strcmp(scr_driver_name,"curses") || !strcmp(scr_driver_name,"caca") ) {


                                putchar_menu_overlay(x+ancho-5,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                                putchar_menu_overlay(x+ancho-4,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                                putchar_menu_overlay(x+ancho-3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
                                putchar_menu_overlay(x+ancho-2,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
		}

	}
}

//x,y origen ventana, ancho ventana
void menu_dibuja_ventana_franja_arcoiris_trozo(int x, int y, int ancho,int franjas)
{

	if (!ventana_tipo_activa) return;

	//int cr[]={2+8,6+8,4+8,5+8};
	int *cr;
	cr=ESTILO_GUI_FRANJAS_NORMALES;

	if (ESTILO_GUI_MUESTRA_RAINBOW) {
		//en el caso de drivers completos, hacerlo real
		if (si_complete_video_driver() ) {
			//5 espacios negro primero
			int i;
			for (i=6;i>=2;i--) putchar_menu_overlay(x+ancho-i,y,' ',ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_PAPEL_TITULO);
	                if (franjas==4) {
	                	putchar_menu_overlay(x+ancho-6,y,128,cr[0],ESTILO_GUI_PAPEL_TITULO);
        	        	putchar_menu_overlay(x+ancho-5,y,128,cr[1],cr[0]);
                		putchar_menu_overlay(x+ancho-4,y,128,cr[2],cr[1]);
	                	putchar_menu_overlay(x+ancho-3,y,128,cr[3],cr[2]);
        	        	putchar_menu_overlay(x+ancho-2,y,128,ESTILO_GUI_PAPEL_TITULO,cr[3]);
        	        }

        	     	if (franjas==3) {
        	        	putchar_menu_overlay(x+ancho-5,y,128,cr[1],ESTILO_GUI_PAPEL_TITULO);
                		putchar_menu_overlay(x+ancho-4,y,128,cr[2],cr[1]);
	                	putchar_menu_overlay(x+ancho-3,y,128,cr[3],cr[2]);
        	        	putchar_menu_overlay(x+ancho-2,y,128,ESTILO_GUI_PAPEL_TITULO,cr[3]);
        	        }


        	        if (franjas==2) {
                		putchar_menu_overlay(x+ancho-4,y,128,cr[2],ESTILO_GUI_PAPEL_TITULO);
	                	putchar_menu_overlay(x+ancho-3,y,128,cr[3],cr[2]);
        	        	putchar_menu_overlay(x+ancho-2,y,128,ESTILO_GUI_PAPEL_TITULO,cr[3]);
        	        }

        	        if (franjas==1) {
	                	putchar_menu_overlay(x+ancho-3,y,128,cr[3],ESTILO_GUI_PAPEL_TITULO);
        	        	putchar_menu_overlay(x+ancho-2,y,128,ESTILO_GUI_PAPEL_TITULO,cr[3]);
        	        }

        	      
	        }

		//en caso de curses o caca, hacerlo con lineas de colores
	        if (!strcmp(scr_driver_name,"curses") || !strcmp(scr_driver_name,"caca") ) {
        	        //putchar_menu_overlay(x+ancho-6,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
                	//putchar_menu_overlay(x+ancho-5,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
	                //putchar_menu_overlay(x+ancho-4,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
        	        //putchar_menu_overlay(x+ancho-3,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);


        	        //5 espacios negro primero
			int i;
			for (i=6;i>=2;i--) putchar_menu_overlay(x+ancho-i,y,' ',ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_PAPEL_TITULO);
	                if (franjas==4) {
	                	putchar_menu_overlay(x+ancho-5,y,'/',cr[0],ESTILO_GUI_PAPEL_TITULO);
        	        	putchar_menu_overlay(x+ancho-4,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                		putchar_menu_overlay(x+ancho-3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
	                	putchar_menu_overlay(x+ancho-2,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        	        }

        	     	if (franjas==3) {
        	        	putchar_menu_overlay(x+ancho-4,y,'/',cr[1],ESTILO_GUI_PAPEL_TITULO);
                		putchar_menu_overlay(x+ancho-3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
	                	putchar_menu_overlay(x+ancho-2,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        	        }


        	        if (franjas==2) {
                		putchar_menu_overlay(x+ancho-3,y,'/',cr[2],ESTILO_GUI_PAPEL_TITULO);
	                	putchar_menu_overlay(x+ancho-2,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        	        }

        	        if (franjas==1) {
	                	putchar_menu_overlay(x+ancho-2,y,'/',cr[3],ESTILO_GUI_PAPEL_TITULO);
        	        }
	        }
	}
}


void menu_dibuja_ventana_franja_arcoiris(int x, int y, int ancho)
{
	menu_dibuja_ventana_franja_arcoiris_trozo(x,y,ancho,4);
}

void menu_dibuja_ventana_franja_arcoiris_trozo_current(int trozos)
{

	menu_dibuja_ventana_franja_arcoiris_trozo(current_win_x,current_win_y,current_win_ancho,trozos);
}

void menu_dibuja_ventana_franja_arcoiris_oscuro_current(int indice)
{

	menu_dibuja_ventana_franja_arcoiris_oscuro(current_win_x,current_win_y,current_win_ancho,indice);
}

//Da ancho titulo de ventana segun el texto titulo, boton cerrado si/no, y franjas de color
int menu_da_ancho_titulo(char *titulo)
{
		int ancho_boton_cerrar=2;

        if (menu_hide_close_button.v) ancho_boton_cerrar=0;

		int ancho_franjas_color=MENU_ANCHO_FRANJAS_TITULO;

		if (!ESTILO_GUI_MUESTRA_RAINBOW) ancho_franjas_color=0;

		int margen_adicional=2; //1 para que no se pegue el titulo a la derecha, otro mas para el caracter de minimizar

		int ancho_total=strlen(titulo)+ancho_boton_cerrar+ancho_franjas_color+margen_adicional; //+1 de margen, para que no se pegue el titulo

		return ancho_total;
}



int menu_dibuja_ventana_ret_ancho_titulo(int ancho,char *titulo)
{
	int ancho_mostrar_titulo=menu_da_ancho_titulo(titulo);

	int ancho_disponible_titulo=ancho;

	if (ancho_disponible_titulo<ancho_mostrar_titulo) ancho_mostrar_titulo=ancho_disponible_titulo;

	return ancho_mostrar_titulo;
}

z80_byte menu_retorna_caracter_minimizar(zxvision_window *w)
{
	z80_byte caracter_mostrar='-';
	if (w->is_minimized) caracter_mostrar='=';

	return caracter_mostrar;
}

void menu_dibuja_ventana_botones(void)
{

	int x=current_win_x;
	int y=current_win_y;
	int ancho=current_win_ancho;
	//int alto=ventana_alto;

		//Boton de minimizar
		if (ventana_activa_tipo_zxvision) {
			if (ventana_tipo_activa) {
				if (cuadrado_activo_resize) {
					z80_byte caracter_mostrar=menu_retorna_caracter_minimizar(zxvision_current_window);
					if (menu_hide_minimize_button.v) caracter_mostrar=' ';
					//Si no mostrar, meter solo espacio. es importante esto, si no hay boton, y no escribieramos espacio,
					//se veria el texto de titulo en caso de que ancho de ventana la hagamos pequeña

					//if (zxvision_current_window->is_minimized) caracter_mostrar='+';
					putchar_menu_overlay(x+ancho-1,y,caracter_mostrar,ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);
				}
			}
		}	



		//putchar_menu_overlay(x+ancho-1,y,'-',ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);
}

//si no mostramos mensajes de error pendientes
int no_dibuja_ventana_muestra_pending_error_message=0;

//dibuja ventana de menu, con:
//titulo
//contenido blanco
//recuadro de lineas
//Entrada: x,y posicion inicial. ancho, alto. Todo coordenadas en caracteres 0..31 y 0..23
void menu_dibuja_ventana(int x,int y,int ancho,int alto,char *titulo)
{


	//Para draw below windows, no mostrar error pendiente cuando esta dibujando ventanas de debajo
	if (!no_dibuja_ventana_muestra_pending_error_message) menu_muestra_pending_error_message();

	//En el caso de stdout, solo escribimos el texto
        if (!strcmp(scr_driver_name,"stdout")) {
                printf ("%s\n",titulo);
		scrstdout_menu_print_speech_macro(titulo);
		printf ("------------------------\n\n");
		//paso de curses a stdout deja stdout que no hace flush nunca. forzar
		fflush(stdout);
                return;
        }

	//printf ("valor menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);

	//valores en pixeles
	int xpixel,ypixel,anchopixel,altopixel;
	int i,j;

	//guardamos valores globales de la ventana mostrada
	current_win_x=x;
	current_win_y=y;
	current_win_ancho=ancho;
	current_win_alto=alto;

	xpixel=x*menu_char_width;
	ypixel=y*8;
	anchopixel=ancho*menu_char_width;
	altopixel=alto*8;

	int xderecha=xpixel+anchopixel-1;
	//printf ("x derecha: %d\n",xderecha);

	//if (menu_char_width!=8) xderecha++; //?????

	//contenido en blanco normalmente en estilo ZEsarUX
	for (i=0;i<alto-1;i++) {
		for (j=0;j<ancho;j++) {
			//putchar_menu_overlay(x+j,y+i+1,' ',0,7+8);
			putchar_menu_overlay(x+j,y+i+1,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
		}
	}

	//recuadro
	//menu_establece_cuadrado(xpixel,ypixel,xpixel+anchopixel-1,ypixel+altopixel-1,0);
	//printf ("cuadrado: %d,%d %dX%d\n",xpixel,ypixel,xpixel+anchopixel-1,ypixel+altopixel-1);
	
	menu_establece_cuadrado(xpixel,ypixel,xderecha,ypixel+altopixel-1,ESTILO_GUI_PAPEL_TITULO);


	int color_tinta_titulo;
	int color_papel_titulo;


	if (ventana_tipo_activa) {
		color_tinta_titulo=ESTILO_GUI_TINTA_TITULO;
		color_papel_titulo=ESTILO_GUI_PAPEL_TITULO;
	}

	else {
		color_tinta_titulo=ESTILO_GUI_TINTA_TITULO_INACTIVA;
		color_papel_titulo=ESTILO_GUI_PAPEL_TITULO_INACTIVA;		
	}


        //titulo
        //primero franja toda negra normalmente en estilo ZEsarUX
        for (i=0;i<ancho;i++) {
			putchar_menu_overlay(x+i,y,' ',color_tinta_titulo,color_papel_titulo);
		}


		int ancho_mostrar_titulo=menu_dibuja_ventana_ret_ancho_titulo(ancho,titulo);

		char titulo_mostrar[64];
		char caracter_cerrar=ESTILO_GUI_BOTON_CERRAR;

		if (menu_hide_close_button.v || ventana_es_background ) strcpy(titulo_mostrar,titulo);
		else sprintf (titulo_mostrar,"%c %s",caracter_cerrar,titulo);


        //y luego el texto. titulo mostrar solo lo que cabe de ancho


	//Boton de cerrado



        for (i=0;i<ancho_mostrar_titulo && titulo_mostrar[i];i++) {
			char caracter_mostrar=titulo_mostrar[i];
			
			putchar_menu_overlay(x+i,y,caracter_mostrar,color_tinta_titulo,color_papel_titulo);
		}



        //y las franjas de color
	if (ESTILO_GUI_MUESTRA_RAINBOW && ventana_tipo_activa) {
		//en el caso de drivers completos, hacerlo real
		menu_dibuja_ventana_franja_arcoiris(x,y,ancho);
	
	}

		menu_dibuja_ventana_botones();


        char buffer_titulo[100];
        sprintf (buffer_titulo,"Window: %s",titulo);
        menu_textspeech_send_text(buffer_titulo);

        //Forzar que siempre suene
        //menu_speech_tecla_pulsada=0;



}

int last_mouse_x,last_mouse_y;
int mouse_movido=0;

int menu_mouse_x=0;
int menu_mouse_y=0;

zxvision_window *zxvision_current_window=NULL;

//Decir que con una ventana zxvision visible, las pulsaciones de teclas no se envian a maquina emulada
int zxvision_keys_event_not_send_to_machine=1;

void zxvision_set_draw_window_parameters(zxvision_window *w)
{
	ventana_activa_tipo_zxvision=1;

	cuadrado_activo_resize=w->can_be_resized;

}

void zxvision_draw_below_windows_nospeech(zxvision_window *w)
{
	//Redibujar las de debajo
	//printf ("antes draw below\n");

	int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
	//No enviar a speech las ventanas por debajo
	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
	
	zxvision_draw_below_windows(w);

	menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
	//printf ("despues draw below\n");
}

//Controlar rangos excepto tamaño ventana en estatico
void zxvision_new_window_check_range(int *x,int *y,int *visible_width,int *visible_height)
{

	//Controlar rangos. Cualquier valor que se salga de rango, hacemos ventana maximo 32x24

	//Rango xy es el total de ventana. Rango ancho y alto es 32x24, aunque luego se pueda hacer mas grande

	if (

	 (*x<0               || *x>ZXVISION_MAX_X_VENTANA) ||
	 (*y<0               || *y>ZXVISION_MAX_Y_VENTANA) ||

	 //Rangos estaticos de ventana
	 (*visible_width<=0) ||
	 (*visible_height<=0) ||

	//Rangos de final de ventana. ZXVISION_MAX_X_VENTANA normalmente vale 31. ZXVISION_MAX_Y_VENTANA normalmente vale 23. Si esta en ancho 31 y le suma 1, es ok. Si suma 2, es error
	 ((*x)+(*visible_width)>ZXVISION_MAX_X_VENTANA+1) ||
	 ((*y)+(*visible_height)>ZXVISION_MAX_Y_VENTANA+1) 

	)
		{
                debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d. Returning fixed safe values",*x,*y,*visible_width,*visible_height);
				//printf ("zxvision_new_window: window out of range: %d,%d %dx%d\n",*x,*y,*visible_width,*visible_height);
                *x=0;
                *y=0;
                *visible_width=ZXVISION_MAX_ANCHO_VENTANA;
                *visible_height=ZXVISION_MAX_ALTO_VENTANA;

		}
}

//Comprobar que el alto y ancho no pase de un fijo estatico (32x24 normalmente),
//para tener ventanas que normalmente no excedan ese 32x24 al crearse
//Nota: menu_filesel no hace este check
void zxvision_new_window_check_static_size_range(int *x,int *y,int *visible_width,int *visible_height)
{


	if (


	 //Rangos estaticos de ancho ventana
	 (*visible_width>ZXVISION_MAX_ANCHO_VENTANA) ||
	 (*visible_height>ZXVISION_MAX_ALTO_VENTANA) 


	)
		{
                debug_printf (VERBOSE_INFO,"zxvision_new_window: window out of range: %d,%d %dx%d",*x,*y,*visible_width,*visible_height);
				//printf ("zxvision_new_window: window out of range: %d,%d %dx%d\n",*x,*y,*visible_width,*visible_height);
                *x=0;
                *y=0;
                *visible_width=ZXVISION_MAX_ANCHO_VENTANA;
                *visible_height=ZXVISION_MAX_ALTO_VENTANA;

		}
}

void zxvision_new_window_no_check_range(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

	//Alto visible se reduce en 1 - por el titulo de ventana
	
	w->x=x;
	w->y=y;
	w->visible_width=visible_width;
	w->visible_height=visible_height;

	w->total_width=total_width;
	w->total_height=total_height;

	w->offset_x=0;
	w->offset_y=0;

	//Establecer titulo ventana
	strcpy(w->window_title,title);	

	int buffer_size=total_width*total_height;
	w->memory=malloc(buffer_size*sizeof(overlay_screen));

	if (w->memory==NULL) cpu_panic ("Can not allocate memory for window");

	//Inicializarlo todo con texto blanco
	//putchar_menu_overlay(x+j,y+i+1,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);

	int i;
	overlay_screen *p;
	p=w->memory;

	for (i=0;i<buffer_size;i++) {
		p->tinta=ESTILO_GUI_TINTA_NORMAL;
		p->papel=ESTILO_GUI_PAPEL_NORMAL;
		p->parpadeo=0;
		p->caracter=' ';

		p++;
	}

	//Ventana anterior
	w->previous_window=zxvision_current_window;
	//printf ("Previous window: %p\n",w->previous_window);
	w->next_window=NULL;

	//Ventana siguiente, decir a la anterior, es la actual
	if (zxvision_current_window!=NULL) {
		//printf ("Decimos que siguiente ventana es: %p\n",zxvision_current_window);
		zxvision_current_window->next_window=w;
	}

	//Ventana actual
	zxvision_current_window=w;


	//Decir que al abrir la ventana, las pulsaciones de teclas no se envian por defecto a maquina emulada
	zxvision_keys_event_not_send_to_machine=1;

	ventana_tipo_activa=1;

	

	//Decimos que se puede redimensionar
	w->can_be_resized=1;

	w->is_minimized=0;
	w->is_maximized=0;
	w->height_before_max_min_imize=visible_height;	
	w->width_before_max_min_imize=visible_width;	

	w->x_before_max_min_imize=x;	
	w->y_before_max_min_imize=y;	

	w->can_use_all_width=0;
	//w->applied_can_use_all_width=0;

	w->visible_cursor=0;
	w->cursor_line=0;

	w->upper_margin=0;
	w->lower_margin=0;

	//Y textos margen nulos
	//w->text_margin[0]=NULL;


	//Funcion de overlay inicializada a NULL
	w->overlay_function=NULL;


	zxvision_set_draw_window_parameters(w);

	//Redibujar las de debajo
	zxvision_draw_below_windows_nospeech(w);


}


void zxvision_new_window(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

	zxvision_new_window_check_range(&x,&y,&visible_width,&visible_height);
	zxvision_new_window_check_static_size_range(&x,&y,&visible_width,&visible_height);
	zxvision_new_window_no_check_range(w,x,y,visible_width,visible_height,total_width,total_height,title);
}

void zxvision_new_window_nocheck_staticsize(zxvision_window *w,int x,int y,int visible_width,int visible_height,int total_width,int total_height,char *title)
{

	zxvision_new_window_check_range(&x,&y,&visible_width,&visible_height);
	zxvision_new_window_no_check_range(w,x,y,visible_width,visible_height,total_width,total_height,title);
}



//Borrar contenido ventana con espacios
void zxvision_clear_window_contents(zxvision_window *w)
{

	int y;

	for (y=0;y<w->total_height;y++) {
		zxvision_fill_width_spaces(w,y);
	}

}

void zxvision_destroy_window(zxvision_window *w)
{
	zxvision_current_window=w->previous_window;


	//printf ("Setting current window to %p\n",zxvision_current_window);

	//printf ("Next window was %p\n",w->next_window);

	
	ventana_tipo_activa=1;
	zxvision_keys_event_not_send_to_machine=1;

	if (zxvision_current_window!=NULL) {
		//Dibujar las de detras
		//printf ("Dibujando ventanas por detras\n");

		//printf ("proxima ventana antes de dibujar: %p\n",w->next_window);
		zxvision_draw_below_windows_nospeech(w);

		
		zxvision_set_draw_window_parameters(zxvision_current_window);

		//Dibujar ventana que habia debajo
		zxvision_draw_window(zxvision_current_window);
		zxvision_draw_window_contents(zxvision_current_window);
		//printf ("Dibujando ventana de debajo que ahora es de frente\n");
	}

	//Liberar memoria cuando ya no se use para nada
	free(w->memory);

	//Decir que esta ventana no tiene siguiente. 
	//TODO: solo podemos hacer destroy de la ultima ventana creada,
	//habria que tener metodo para poder destruir ventana de en medio
	//TODO2: si hacemos esto justo despues de zxvision_current_window=w->previous_window; acaba provocando segfault al redibujar. por que?
	if (zxvision_current_window!=NULL) zxvision_current_window->next_window=NULL;

	//para poder hacer destroy de ventana de en medio seria tan simple como hacer que zxvision_current_window->next_window= fuera el next que habia al principio

}


z80_byte zxvision_read_keyboard(void)
{

	//printf ("antes menu_get_pressed_key\n");
    z80_byte tecla;
	
	if (!mouse_pressed_close_window) {
		tecla=menu_get_pressed_key();


		//Si ventana inactiva y se ha pulsado tecla, excepto ESC, no leer dicha tecla
		if (tecla!=0 && tecla!=2 && zxvision_keys_event_not_send_to_machine==0) {
			//printf ("no leemos tecla en ventana pues esta inactiva\n");
			tecla=0; 
		}
	}

	//Si pulsado boton cerrar ventana, enviar ESC
	if (mouse_pressed_close_window) {
		//printf ("Retornamos ESC pues se ha pulsado boton de cerrar ventana\n");
		//mouse_pressed_close_window=0;
		return 2;
	}

	return tecla;
}

void zxvision_wait_until_esc(zxvision_window *w)
{
	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();
		zxvision_handle_cursors_pgupdn(w,tecla);
	} while (tecla!=2);

}


//escribe la cadena de texto
void zxvision_scanf_print_string(zxvision_window *ventana,char *string,int offset_string,int max_length_shown,int x,int y,int pos_cursor_x)
{

	char cadena_buf[2];

	string=&string[offset_string];


	int rel_x=0;

	//y si offset>0, primer caracter sera '<'
	if (offset_string) {
		zxvision_print_string_defaults(ventana,x,y,"<");
		max_length_shown--;
		x++;
		rel_x++;
		string++;
	}

	for (;max_length_shown;max_length_shown--) {

		if (rel_x==pos_cursor_x) {
			//printf ("Escribir cursor en medio o final en %d %d\n",x,y);
			zxvision_print_string(ventana,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,1,"_");
		}

		else {
			if ( (*string) !=0) {
				cadena_buf[0]=*string;
				cadena_buf[1]=0;
		
				zxvision_print_string_defaults(ventana,x,y,cadena_buf);
				string++;
			}

			else {
				//meter espacios
				zxvision_print_string_defaults(ventana,x,y," ");
			}
		}
		x++;
		rel_x++;

		
	}

	zxvision_draw_window_contents(ventana);


}

void menu_scanf_cursor_izquierda(int *offset_string,int *pos_cursor_x)
{
                        
				//Desplazar siempre offset que se pueda
				if ((*offset_string)>0) {
					(*offset_string)--;
					//printf ("offset string: %d\n",*offset_string);
				}

				else if ((*pos_cursor_x)>0) (*pos_cursor_x)--;
}

void menu_scanf_cursor_derecha(char *texto,int *pos_cursor_x,int *offset_string,int max_length_shown)
{

			int i;
			i=strlen(texto);

			int pos_final=(*offset_string)+(*pos_cursor_x);


			if (pos_final<i) {  

				if ((*pos_cursor_x)<max_length_shown-1) {
						(*pos_cursor_x)++;
						//printf ("mover cursor\n");
				}
					//Si no mueve cursor, puede que haya que desplazar offset del inicio
				
				else if (i>=max_length_shown) {
					//printf ("Scroll\n");
					(*offset_string)++;
				}
			}
}

//devuelve cadena de texto desde teclado
//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
//ejemplo, si el array es de 50, se le debe pasar max_length a 50
int zxvision_scanf(zxvision_window *ventana,char *string,unsigned int max_length,int max_length_shown,int x,int y)
{


	//Al menos 2 de maximo a mostrar. Si no, salir
	if (max_length_shown<2) {
		debug_printf (VERBOSE_ERR,"Edit field size too small. Returning null string");
		string[0]=0;
		return 2; //Devolvemos escape

	}	

	//Enviar a speech
	char buf_speech[MAX_BUFFER_SPEECH+1];
	sprintf (buf_speech,"Edit box: %s",string);
	menu_textspeech_send_text(buf_speech);


	z80_byte tecla;

	//ajustar offset sobre la cadena de texto visible en pantalla
	int offset_string;

	int j;
	j=strlen(string);
	if (j>max_length_shown-1) offset_string=j-max_length_shown+1;
	else offset_string=0;

	int pos_cursor_x; //Donde esta el cursor

	pos_cursor_x=j;
	if (pos_cursor_x>max_length_shown-1) pos_cursor_x=max_length_shown;


	//max_length ancho maximo del texto, sin contar caracter 0
	//por tanto si el array es de 50, se le debe pasar max_length a 50

	max_length--;

	//cursor siempre al final del texto

	do { 
		zxvision_scanf_print_string(ventana,string,offset_string,max_length_shown,x,y,pos_cursor_x);

		if (menu_multitarea==0) menu_refresca_pantalla();

		menu_espera_tecla();
		//printf ("Despues de espera tecla\n");
		tecla=zxvision_common_getkey_refresh();	
		//printf ("tecla leida=%d\n",tecla);
		menu_espera_no_tecla();



		//si tecla normal, agregar en la posicion del cursor:
		if (tecla>31 && tecla<128) {
			if (strlen(string)<max_length) {
			
				int i;
				i=strlen(string);

				int pos_agregar=pos_cursor_x+offset_string;
				//printf ("agregar letra en %d\n",pos_agregar);
				util_str_add_char(string,pos_agregar,tecla);


				//Enviar a speech letra pulsada
				menu_speech_tecla_pulsada=0;
			    sprintf (buf_speech,"%c",tecla);
        		menu_textspeech_send_text(buf_speech);

				//Y mover cursor a la derecha
				menu_scanf_cursor_derecha(string,&pos_cursor_x,&offset_string,max_length_shown);

				//printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);

			}
		}

		//tecla derecha
		if (tecla==9) {
				menu_scanf_cursor_derecha(string,&pos_cursor_x,&offset_string,max_length_shown);
				//printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);
		}			

		//tecla borrar
		if (tecla==12) {
			//Si longitud texto no es 0
			if (strlen(string)>0) {

				int pos_eliminar=pos_cursor_x+offset_string-1;

				//no borrar si cursor a la izquierda del todo
				if (pos_eliminar>=0) {

					//printf ("borrar\n");
					
								
                    //Enviar a speech letra borrada

					menu_speech_tecla_pulsada=0;
                    sprintf (buf_speech,"%c",string[pos_eliminar]);
                    menu_textspeech_send_text(buf_speech);
                           
					//Eliminar ese caracter
					util_str_del_char(string,pos_eliminar);

					//Y mover cursor a la izquierda
					menu_scanf_cursor_izquierda(&offset_string,&pos_cursor_x);	
					
				}
			}

		}

		//tecla izquierda
		if (tecla==8) {
				menu_scanf_cursor_izquierda(&offset_string,&pos_cursor_x);
				//printf ("offset_string %d pos_cursor %d\n",offset_string,pos_cursor_x);
		}				

		//tecla abajo. borrar todo
		if (tecla==10) {
			//Enviar a speech decir borrar todo
			menu_speech_tecla_pulsada=0;
            strcpy (buf_speech,"delete all");
            menu_textspeech_send_text(buf_speech);

            string[0]=0;
			offset_string=0;
			pos_cursor_x=0;
	
		}


	} while (tecla!=13 && tecla!=15 && tecla!=2);

	//if (tecla==13) printf ("salimos con enter\n");
	//if (tecla==15) printf ("salimos con tab\n");

	menu_reset_counters_tecla_repeticion();
	return tecla;

//papel=7+8;
//tinta=0;

}


int zxvision_generic_message_cursor_down(zxvision_window *ventana)
{

	//int linea_retornar;

	if (ventana->visible_cursor) {

		//Movemos el cursor si es que es posible
		if (ventana->cursor_line<ventana->total_height-1) {
			//printf ("Incrementamos linea cursor\n");
			ventana->cursor_line++;
		}
		else {
			
			return ventana->cursor_line;
		}

		//Ver en que offset estamos
		int offset_y=ventana->offset_y;
		//Y donde esta el cursor
		int cursor=ventana->cursor_line;

		//Y si cursor no esta visible, lo ponemos para que este abajo del todo (hemos de suponer que estaba abajo y ha bajado 1 mas)
		if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
			ventana->cursor_line=offset_y+ventana->visible_height-2;
			zxvision_send_scroll_down(ventana);
			//printf ("Bajamos linea cursor y bajamos offset\n");
		}
		else {
			//Redibujamos contenido
			//printf ("Solo redibujamos\n");
			zxvision_draw_window_contents(ventana);
			//zxvision_draw_scroll_bars(w);
		}

		return ventana->cursor_line;
	}

	else {	
		zxvision_send_scroll_down(ventana);
		return (ventana->offset_y + ventana->visible_height-3);
	}



}

int zxvision_generic_message_cursor_up(zxvision_window *ventana)
{
	if (ventana->visible_cursor) {

		//Movemos el cursor si es que es posible
		if (ventana->cursor_line>0) {
			//printf ("Decrementamos linea cursor\n");
			ventana->cursor_line--;
		}
		else return ventana->cursor_line;

		//Ver en que offset estamos
		int offset_y=ventana->offset_y;
		//Y donde esta el cursor
		int cursor=ventana->cursor_line;

		//Y si cursor no esta visible, lo ponemos para que este arriba del todo (hemos de suponer que estaba arriba i ha subido 1 mas)
		if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
			if (offset_y>0) ventana->cursor_line=offset_y-1;
			zxvision_send_scroll_up(ventana);
			//printf ("Subimos linea cursor y subimos offset\n");
		}
		else {
			//Redibujamos contenido
			//printf ("Solo redibujamos\n");
			zxvision_draw_window_contents(ventana);
			//zxvision_draw_scroll_bars(w);
		}

		return ventana->cursor_line;
	}

	else {	
		zxvision_send_scroll_up(ventana);
		return ventana->offset_y;
	}



}		


int menu_get_origin_x_zxdesktop_aux(int divisor)
{
	//Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
	int ancho_total=scr_get_menu_width();

	//Quitamos el tamaño maximo ventana (normalmente 32), entre 2
	//int pos_x=ancho_total-ZXVISION_MAX_ANCHO_VENTANA/2;
	int restar=screen_ext_desktop_width/menu_char_width/menu_gui_zoom;
	//printf ("restar: %d\n",restar);
	//al menos 32 de ancho para zona de menu
	if (restar<32) restar=32;
	int pos_x=ancho_total-restar/divisor;

	//Por si acaso
	if (pos_x<0) pos_x=0;		
	return pos_x;
}

int menu_origin_x(void)
{


	if (screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop()) {
		//Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
		return menu_get_origin_x_zxdesktop_aux(1);
	}

	return 0;
}


int menu_center_x(void)
{


	if (screen_ext_desktop_place_menu && screen_ext_desktop_enabled*scr_driver_can_ext_desktop()) {
		//Esta zxdesktop. Intentamos mantener ventanas localizadas ahi por defecto, si hay esa opcion activada
		return menu_get_origin_x_zxdesktop_aux(2);
	}

	return scr_get_menu_width()/2;
}

int menu_center_y(void)
{
	return scr_get_menu_height()/2;
}

//Funcion generica para preguntar por un archivo de texto a grabar, con un unico filtro de texto
//Retorna 0 si se cancela
int menu_ask_file_to_save(char *titulo_ventana,char *filtro,char *file_save)
{
	//char file_save[PATH_MAX];

	char *filtros[2];

	filtros[0]=filtro;
    filtros[1]=0;

    int ret;

	ret=menu_filesel(titulo_ventana,filtros,file_save);

	if (ret==1) {

		//Ver si archivo existe y preguntar
		if (si_existe_archivo(file_save)) {

			if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return 0;

        }

		return 1;

	}

	return 0;
}


//Muestra un mensaje en ventana troceando el texto en varias lineas de texto con estilo zxvision
//volver_timeout: si vale 1, significa timeout normal como ventanas splash. Si vale 2, no finaliza, muestra franjas de color continuamente
//return_after_print_text, si no es 0, se usa para que vuelva a la funcion que llama justo despues de escribir texto,
//usado en opciones de mostrar First Aid y luego agregarle opciones de menu tabladas,
//por lo que agrega cierta altura a la ventana. Se agregan tantas lineas como diga el parametro return_after_print_text
void zxvision_generic_message_tooltip(char *titulo, int return_after_print_text,int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, int resizable, const char * texto_format , ...)
{

	//Buffer de entrada

        char texto[MAX_TEXTO_GENERIC_MESSAGE];
        va_list args;
        va_start (args, texto_format);
        vsprintf (texto,texto_format, args);
	va_end (args);

	//printf ("input text: %s\n",texto);

	if (volver_timeout) {
		menu_window_splash_counter=0;
		menu_window_splash_counter_ms=0;
	}
	//linea cursor en el caso que se muestre cursor
	//int linea_cursor=0;

	//En caso de stdout, es mas simple, mostrar texto y esperar tecla
        if (!strcmp(scr_driver_name,"stdout")) {
		//printf ("%d\n",strlen(texto));


		printf ("-%s-\n",titulo);
		printf ("\n");
		printf ("%s\n",texto);

		scrstdout_menu_print_speech_macro(titulo);
		scrstdout_menu_print_speech_macro(texto);

		menu_espera_no_tecla();
		menu_espera_tecla();

		return;
        }

	//En caso de simpletext, solo mostrar texto sin esperar tecla
	if (!strcmp(scr_driver_name,"simpletext")) {
                printf ("-%s-\n",titulo);
                printf ("\n");
                printf ("%s\n",texto);

		return;
	}


	int tecla;


	//texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
	char buffer_lineas[MAX_LINEAS_TOTAL_GENERIC_MESSAGE][MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

	const int max_ancho_texto=30;

	//Primera linea que mostramos en la ventana
	//int primera_linea=0;

	int indice_linea=0;  //Numero total de lineas??
	int indice_texto=0;
	int ultimo_indice_texto=0;
	int longitud=strlen(texto);


	//Copia del texto de entrada (ya formateado con vsprintf) que se leera solo al copiar clipboard
	//Al pulsar tecla de copy a cliboard, se lee el texto que haya aqui,
	//y no el contenido en el char *texto, pues ese se ha alterado quitando saltos de linea y otros caracteres
	char *menu_generic_message_tooltip_text_initial;


	debug_printf(VERBOSE_INFO,"Allocating %d bytes to initial text",longitud+1);
	menu_generic_message_tooltip_text_initial=malloc(longitud+1);
	if (menu_generic_message_tooltip_text_initial==NULL) {
		debug_printf(VERBOSE_ERR,"Can not allocate buffer for initial text");
	}


	//En caso que se haya podido asignar el buffer de clonado
	if (menu_generic_message_tooltip_text_initial!=NULL) {
		strcpy(menu_generic_message_tooltip_text_initial,texto);
	}

	int ultima_linea_buscada=-1;
	char buffer_texto_buscado[33];

	//int indice_segunda_linea;

	//int texto_no_cabe=0;

	do {
		indice_texto+=max_ancho_texto;

		//temp
		//printf ("indice_linea: %d\n",indice_linea);

		//Controlar final de texto
		if (indice_texto>=longitud) indice_texto=longitud;

		//Si no, miramos si hay que separar por espacios
		else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

		//Separamos por salto de linea, filtramos caracteres extranyos
		indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

		//copiar texto
		int longitud_texto=indice_texto-ultimo_indice_texto;


		//snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);


		menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
		buffer_lineas[indice_linea++][longitud_texto]=0;
		//printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


		//printf ("texto: %s\n",buffer_lineas[indice_linea-1]);

		if (indice_linea==MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
                        //cpu_panic("Max lines on menu_generic_message reached");
			debug_printf(VERBOSE_INFO,"Max lines on menu_generic_message reached (%d)",MAX_LINEAS_TOTAL_GENERIC_MESSAGE);
			//finalizamos bucle
			indice_texto=longitud;
		}

		ultimo_indice_texto=indice_texto;
		//printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

	} while (indice_texto<longitud);


	//printf ("\ntext after converting to lines: %s\n",texto);


	debug_printf (VERBOSE_INFO,"Read %d lines (word wrapped)",indice_linea);
	//int primera_linea_a_speech=0;
	//int ultima_linea_a_speech=0;
	int linea_a_speech=0;
	int enviar_linea_a_speech=0;

	//do {

	//printf ("primera_linea: %d\n",primera_linea);




	int alto_ventana=indice_linea+2;

	int alto_total_ventana=indice_linea;

	if (return_after_print_text) {
		//Darle mas altura	
		alto_ventana +=return_after_print_text;
		alto_total_ventana +=return_after_print_text;
	}

	if (alto_ventana-2>MAX_LINEAS_VENTANA_GENERIC_MESSAGE) {
		alto_ventana=MAX_LINEAS_VENTANA_GENERIC_MESSAGE+2;
		//texto_no_cabe=1;
	}


	//printf ("alto ventana: %d\n",alto_ventana);
	//int ancho_ventana=max_ancho_texto+2;
	int ancho_ventana=max_ancho_texto+2;

	int xventana=menu_center_x()-ancho_ventana/2;
	int yventana=menu_center_y()-alto_ventana/2;

	if (tooltip_enabled==0) {
		menu_espera_no_tecla_con_repeticion();
		cls_menu_overlay();
	}

	zxvision_window *ventana;

	if (return_after_print_text) {
		//Dado que vamos a volver con la ventana activa que se crea aquí, hay que asignar la estructura en memoria global
		ventana=malloc(sizeof(zxvision_window));
		//printf ("tamanyo memoria ventana %d\n",sizeof(zxvision_window));
		if (ventana==NULL) cpu_panic("Can not allocate memory for zxvision window");
	}

	else {
		zxvision_window mi_ventana;
		ventana=&mi_ventana;
	}
	//printf ("antes de zxvision_new_window\n");		


	zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_total_ventana,titulo);	

	//printf ("despues de zxvision_new_window\n");							

	if (!resizable) zxvision_set_not_resizable(ventana);	

	if (mostrar_cursor) ventana->visible_cursor=1;	

	zxvision_draw_window(ventana);

	//printf ("despues de zxvision_draw_window\n");

				//Decir que se ha pulsado tecla asi no se lee todo cuando el cursor esta visible
				if (ventana->visible_cursor) menu_speech_tecla_pulsada=1;
	int i;
	/*for (i=0;i<indice_linea-primera_linea && i<MAX_LINEAS_VENTANA_GENERIC_MESSAGE;i++) {
		if (mostrar_cursor) {
			menu_escribe_linea_opcion(i,linea_cursor,1,buffer_lineas[i+primera_linea]);
		}
        	else menu_escribe_linea_opcion(i,-1,1,buffer_lineas[i+primera_linea]);
		//printf ("i: %d linea_cursor: %d primera_linea: %d\n",i,linea_cursor,primera_linea);
		//printf ("Linea seleccionada: %d (%s)\n",linea_cursor+primera_linea,buffer_lineas[linea_cursor+primera_linea]);
	}*/

	for (i=0;i<indice_linea;i++) {
		zxvision_print_string(ventana,1,i,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,buffer_lineas[i]);
	}

	zxvision_draw_window_contents(ventana);

	if (return_after_print_text) return;

	do {

		//printf ("entrada bucle do\n");



	//Enviar primera linea o ultima a speech

	//La primera linea puede estar oculta por .., aunque para speech mejor que diga esa primera linea oculta
	//debug_printf (VERBOSE_DEBUG,"First line: %s",buffer_lineas[primera_linea]);
	//debug_printf (VERBOSE_DEBUG,"Last line: %s",buffer_lineas[i+primera_linea-1]);

	//printf ("Line to speech: %s\n",buffer_lineas[linea_a_speech]);

	if (enviar_linea_a_speech) {
		menu_speech_tecla_pulsada=0;
		enviar_linea_a_speech=0;
		menu_textspeech_send_text(buffer_lineas[linea_a_speech]);
	}



	menu_speech_tecla_pulsada=1;
	enviar_linea_a_speech=0;





        if (!menu_multitarea) {
			//printf ("refresca pantalla\n");
			menu_refresca_pantalla();
		}

		/*else {
			menu_cpu_core_loop();
		}*/

		if (volver_timeout) {
			//printf ("antes de tooltip\n");
			zxvision_espera_tecla_timeout_window_splash(volver_timeout);
			if (volver_timeout==2) menu_espera_no_tecla();
			//printf ("despues de tooltip\n");
		}
		else {
			//printf ("Antes espera tecla\n");
			menu_cpu_core_loop();
        	menu_espera_tecla();


		}

//printf ("antes de leer tecla\n");
				tecla=zxvision_read_keyboard();

//printf ("despues de leer tecla\n");             

				//Si se pulsa boton mouse, al final aparece como enter y no es lo que quiero
				//if (tecla==13 && mouse_left && zxvision_keys_event_not_send_to_machine && !mouse_is_dragging) {
				if (tecla==13 && mouse_left) {	
					tecla=0;
				}


		if (volver_timeout) tecla=13;
						
							

		if (tooltip_enabled==0 && tecla) {
			//printf ("Esperamos no tecla\n");
			menu_espera_no_tecla_con_repeticion();
		}


		int contador_pgdnup;

        switch (tecla) {

						//Nota: No llamamos a funcion generica zxvision_handle_cursors_pgupdn en caso de arriba,abajo, pg, pgdn,
						//dado que se comporta distinto cuando cursor esta visible

                        //abajo
                        case 10:
						//primera_linea=menu_generic_message_cursor_abajo_mostrar_cursor(primera_linea,alto_ventana,indice_linea,mostrar_cursor,&linea_cursor);
						linea_a_speech=zxvision_generic_message_cursor_down(ventana);
						//zxvision_send_scroll_down(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
						enviar_linea_a_speech=1;
                        break;

                        //arriba
                        case 11:
						//primera_linea=menu_generic_message_cursor_arriba_mostrar_cursor(primera_linea,mostrar_cursor,&linea_cursor);
						//zxvision_send_scroll_up(ventana);
						linea_a_speech=zxvision_generic_message_cursor_up(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
						//primera_linea_a_speech=1;
						enviar_linea_a_speech=1;
                        break;


                        //izquierda
                        case 8:
						/*zxvision_send_scroll_left(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;*/
						zxvision_handle_cursors_pgupdn(ventana,tecla);
						//ultima_linea_a_speech=1;
                        break;

                        //derecha
                        case 9:
						/*zxvision_send_scroll_right(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;*/
						zxvision_handle_cursors_pgupdn(ventana,tecla);
						//primera_linea_a_speech=1;
                        break;						

						//PgUp
						case 24:
							for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2;contador_pgdnup++) {
								zxvision_generic_message_cursor_up(ventana);
							}
							//Decir que no se ha pulsado tecla para que se relea
							menu_speech_tecla_pulsada=0;

							//Y recargar ventana para que la relea
							zxvision_draw_window_contents(ventana);
						break;

                    	//PgDn
                    	case 25:
                    		for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2;contador_pgdnup++) {
								zxvision_generic_message_cursor_down(ventana);
                        	}

							//Decir que no se ha pulsado tecla para que se relea
							menu_speech_tecla_pulsada=0;

							//Y recargar ventana para que la relea
							zxvision_draw_window_contents(ventana);
                    	break;
						
                                        case 'c':
                                        	menu_copy_clipboard(menu_generic_message_tooltip_text_initial);
                                        	menu_generic_message_splash("Clipboard","Text copied to ZEsarUX clipboard. Go to file utils and press P to paste to a file");

											zxvision_draw_window(ventana);
											zxvision_draw_window_contents(ventana);
                                        break;

						
						/*
						Desactivado para evitar confusiones. Mejor hay que hacer antes copy y paste en file utls
                         case 's':
						 	menu_save_text_to_file(menu_generic_message_tooltip_text_initial,"Save Text");
                 											zxvision_draw_window(ventana);
											zxvision_draw_window_contents(ventana);
                        break;
						*/


						
					//Buscar texto
					case 'f':
					case 'n':

						if (tecla=='f' || ultima_linea_buscada==-1) {

							buffer_texto_buscado[0]=0;
			        			menu_ventana_scanf("Text to find",buffer_texto_buscado,33);

							//ultima_linea_buscada=0; //Si lo pusiera a 0, no encontraria nada en primera linea
							//pues no se cumpliria la condicion de mas abajo de i>ultima_linea_buscada

							ultima_linea_buscada=-1;

						}

						int i;
						char *encontrado=NULL;
						for (i=0;i<indice_linea;i++) {
							debug_printf(VERBOSE_DEBUG,"Searching text on line %d: %s",i,buffer_lineas[i]);
							encontrado=util_strcasestr(buffer_lineas[i], buffer_texto_buscado);
							if (encontrado && i>ultima_linea_buscada) {
								break;
							}
						}

						if (encontrado) {
							ultima_linea_buscada=i;
							//mover cursor hasta ahi
							//primera_linea=0;
							//linea_cursor=0;

							//printf ("mover cursor hasta linea: %d\n",ultima_linea_buscada);

							//Mostramos cursor para poder indicar en que linea se ha encontrado el texto
							mostrar_cursor=1;

							ventana->visible_cursor=1;

							ventana->cursor_line=i;

							//Si no esta visible, cambiamos offset
							zxvision_set_offset_y_visible(ventana,i);
							//if (i<ventana->offset_y || i>=ventana->offset_y+ventana->visible_height-2) zxvision_set_offset_y(ventana,i);

							/*int contador;
							for (contador=0;contador<ultima_linea_buscada;contador++) {
									primera_linea=menu_generic_message_cursor_abajo_mostrar_cursor(primera_linea,alto_ventana,indice_linea,mostrar_cursor,&linea_cursor);
							}*/

							//menu_speech_tecla_pulsada=0;
							//menu_textspeech_send_text(buffer_lineas[ultima_linea_buscada]);
						}

						else {
							menu_speech_tecla_pulsada=0; //para decir que siempre se escuchara el mensaje
							menu_warn_message("Text not found");
						}

						zxvision_draw_window(ventana);
						zxvision_draw_window_contents(ventana);


					break;

					//Movimiento y redimensionado ventana con teclado
					                        //derecha
                    case 'Q':
					case 'A':
					case 'O':
					case 'P':
                    case 'W':
					case 'S':
					case 'K':
					case 'L':
						zxvision_handle_cursors_pgupdn(ventana,tecla);
                    break;		
					
				}

	//Salir con Enter o ESC o fin de tooltip
	} while (tecla!=13 && tecla!=2 && tooltip_enabled==0);

	if (retorno!=NULL) {
		int linea_final;

		//printf ("mostrar cursor %d cursor_line %d ventana->offset_x %d\n",mostrar_cursor,ventana->cursor_line,ventana->offset_x);

		if (mostrar_cursor) linea_final=ventana->cursor_line;
		else linea_final=ventana->offset_x;


		strcpy(retorno->texto_seleccionado,buffer_lineas[linea_final]);
		retorno->linea_seleccionada=linea_final;

		// int estado_retorno; //Retorna 1 si sale con enter. Retorna 0 si sale con ESC
		if (tecla==2) retorno->estado_retorno=0;
		else retorno->estado_retorno=1;

		//printf ("\n\nLinea seleccionada: %d (%s)\n",linea_final,buffer_lineas[linea_final]);

	}

    cls_menu_overlay();
	zxvision_destroy_window(ventana);



        if (tooltip_enabled==0) menu_espera_no_tecla_con_repeticion();

	


	if (menu_generic_message_tooltip_text_initial!=NULL) {
		debug_printf(VERBOSE_INFO,"Freeing previous buffer for initial text");
		free(menu_generic_message_tooltip_text_initial);
	}
	//if (tooltip_enabled==0)



}

//Retorna 1 si se debe perder 1 de ancho visible por la linea de scroll vertical (lo habitual)
//Retorna 0 si no
int zxvision_get_minus_width_byscrollvbar(zxvision_window *w)
{
	if (w->can_use_all_width==0) return 1;
	else return 0;
}

int zxvision_get_effective_width(zxvision_window *w)
{
	//Ancho del contenido es 1 menos, por la columna a la derecha de margen
	return w->visible_width-zxvision_get_minus_width_byscrollvbar(w);
}

int zxvision_get_effective_height(zxvision_window *w)
{
	//Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
	return w->visible_height-2;
}

int zxvision_if_vertical_scroll_bar(zxvision_window *w)
{
	if (w->can_use_all_width==1) {
		//w->applied_can_use_all_width=1;
		return 0;
	}
	int effective_height=zxvision_get_effective_height(w);
	if (w->total_height>effective_height && w->visible_height>=6) return 1;

	return 0;
}

int zxvision_if_horizontal_scroll_bar(zxvision_window *w)
{
	int effective_width=zxvision_get_effective_width(w);
	if (w->total_width>effective_width && w->visible_width>=6) return 1;

	return 0;
}

void zxvision_draw_vertical_scroll_bar(zxvision_window *w,int estilo_invertido)
{

	int effective_height=zxvision_get_effective_height(w);
		//Dibujar barra vertical
		int valor_parcial=w->offset_y+effective_height;
		if (valor_parcial<0) valor_parcial=0;

		//Caso especial arriba del todo cero, valor_parcial es 0 y no contemplamos el alto visible
		//if (w->offset_y==0) valor_parcial=0;		

		int valor_total=w->total_height;
		if (valor_total<=0) valor_total=1; //Evitar divisiones por cero o negativos


		int porcentaje=(valor_parcial*100)/(1+valor_total);  

		//Caso especial arriba del todo
		if (w->offset_y==0) {
			//printf ("Scroll vertical cursor is at the minimum\n");
			porcentaje=0;
		}		

		//Caso especial abajo del todo
		if (w->offset_y+(w->visible_height)-2==w->total_height) { //-2 de perder linea titulo y linea scroll
			//printf ("Scroll vertical cursor is at the maximum\n");
			porcentaje=100;
		}

		menu_ventana_draw_vertical_perc_bar(w->x,w->y,w->visible_width,w->visible_height-1,porcentaje,estilo_invertido);	
}

void zxvision_draw_horizontal_scroll_bar(zxvision_window *w,int estilo_invertido)
{

	int effective_width=w->visible_width-1;
	//Dibujar barra horizontal
		int valor_parcial=w->offset_x+effective_width;
		if (valor_parcial<0) valor_parcial=0;

		//Si offset es cero, valor_parcial es 0 y no contemplamos el ancho visible
		//if (w->offset_x==0) valor_parcial=0;

		int valor_total=w->total_width;
		if (valor_total<=0) valor_total=1; //Evitar divisiones por cero o negativos


		int porcentaje=(valor_parcial*100)/(1+valor_total);  

		//Caso especial izquierda del todo
		if (w->offset_x==0) {
			//printf ("Scroll horizontal cursor is at the minimum\n");
			porcentaje=0;
		}		

		//Caso especial derecha del todo
		if (w->offset_x+(w->visible_width)-1==w->total_width) { //-1 de perder linea scroll
			//printf ("Scroll horizontal cursor is at the maximum\n");
			porcentaje=100;
		}

		menu_ventana_draw_horizontal_perc_bar(w->x,w->y,effective_width,w->visible_height,porcentaje,estilo_invertido);
}

void zxvision_draw_scroll_bars(zxvision_window *w)
{
	//Barras de desplazamiento
	//Si hay que dibujar barra derecha de desplazamiento vertical
	//int effective_height=zxvision_get_effective_height(w);
	//int effective_width=zxvision_get_effective_width(w);
	//int effective_width=w->visible_width-1;

	if (zxvision_if_vertical_scroll_bar(w)) {
		zxvision_draw_vertical_scroll_bar(w,0);
	}

	if (zxvision_if_horizontal_scroll_bar(w)) {
		zxvision_draw_horizontal_scroll_bar(w,0);
	}	
}

void zxvision_draw_window(zxvision_window *w)
{
	menu_dibuja_ventana(w->x,w->y,w->visible_width,w->visible_height,w->window_title);


	//Ver si se puede redimensionar
	//Dado que cada vez que se dibuja ventana, la marca de resize se establece por defecto a desactivada
	cuadrado_activo_resize=w->can_be_resized;
	ventana_activa_tipo_zxvision=1;


	//Si no hay barras de desplazamiento, alterar scroll horiz o vertical segun corresponda
	if (!zxvision_if_horizontal_scroll_bar(w)) {
		//printf ("no hay barra scroll horizontal y por eso ponemos offset x a 0\n");
		w->offset_x=0;
	}
	if (!zxvision_if_vertical_scroll_bar(w)) {
		//printf ("no hay barra scroll vertical y por eso ponemos offset y a 0\n");
		w->offset_y=0;
	}

	zxvision_draw_scroll_bars(w);

	//Mostrar boton de minimizar
	menu_dibuja_ventana_botones();



}

void zxvision_set_not_resizable(zxvision_window *w)
{
	//Decimos que no se puede redimensionar
	//printf ("set not resizable\n");
	cuadrado_activo_resize=0;

	w->can_be_resized=0;
}

void zxvision_set_resizable(zxvision_window *w)
{
	//Decimos que se puede redimensionar
	//printf ("set resizable\n");
	cuadrado_activo_resize=1;

	w->can_be_resized=1;
}

void zxvision_set_offset_x(zxvision_window *w,int offset_x)
{
	//Si se pasa por la izquierda
	if (offset_x<0) return;

	//Si se pasa por la derecha
	if (offset_x+w->visible_width-1>w->total_width) return; //-1 porque se pierde 1 a la derecha con la linea scroll

	w->offset_x=offset_x;	

	zxvision_draw_window_contents(w);
	zxvision_draw_scroll_bars(w);
}

int zxvision_maximum_offset_y(zxvision_window *w)
{
	return w->total_height+2-w->visible_height;
}

void zxvision_set_offset_y(zxvision_window *w,int offset_y)
{
	//Si se pasa por arriba
	if (offset_y<0) return;

	//Si se pasa por abajo

	//if (offset_y+w->visible_height-2 > (w->total_height ) ) return; //-2 porque se pierde 2 linea scroll y la linea titulo
	int maximum_offset=zxvision_maximum_offset_y(w);

	if (offset_y>maximum_offset) {
		//printf ("Maximum offset y reached\n");
		return;
	}

	w->offset_y=offset_y;	

	zxvision_draw_window_contents(w);
	zxvision_draw_scroll_bars(w);
}

void zxvision_set_offset_y_or_maximum(zxvision_window *w,int offset_y)
{
        int maximum_offset=zxvision_maximum_offset_y(w);

        if (offset_y>maximum_offset) {
                //printf ("Maximum offset y reached. Setting maximum\n");
		offset_y=maximum_offset;
        }

	zxvision_set_offset_y(w,offset_y);
}



//Si no esta visible, cambiamos offset
void zxvision_set_offset_y_visible(zxvision_window *w,int y)
{

	int linea_final;

	//El cursor esta por arriba. Decimos que este lo mas arriba posible
	if (y<w->offset_y) {
		linea_final=y;
		//printf ("adjust verticall scroll por arriba to %d\n",linea_final);
		
	}

	//El cursor esta por abajo. decimos que el cursor este lo mas abajo posible
	else if (y>=w->offset_y+w->visible_height-2) {
		linea_final=y-(w->visible_height-2)+1;
		//Ejemplo
		//total height 12
		//visble 10->efectivos son 8
		//establecemos a linea 7
		//linea_final=7-(10-2)+1 = 7-8+1=0

		//printf ("adjust verticall scroll por abajo to %d\n",linea_final);
	}

	else return;

	int ultima_linea_scroll=w->total_height-(w->visible_height-2);
	
	/*
	Ejemplo: visible_height 10-> efectivos son 8
	total_height 12
	podremos hacer 4 veces scroll
	12-(10-2)=12-8=4
	*/

	if (ultima_linea_scroll<0) ultima_linea_scroll=0;
	if (linea_final>ultima_linea_scroll) linea_final=ultima_linea_scroll;

	//printf ("final scroll %d\n",linea_final);

	zxvision_set_offset_y(w,linea_final);


}


void zxvision_send_scroll_down(zxvision_window *w)
{
	if (w->offset_y<(w->total_height-1)) {
						zxvision_set_offset_y(w,w->offset_y+1);
	}	
}


void zxvision_send_scroll_up(zxvision_window *w)
{
	if (w->offset_y>0) {
		zxvision_set_offset_y(w,w->offset_y-1);
	}
}


void zxvision_send_scroll_left(zxvision_window *w)
{
	if (w->offset_x>0) {
		zxvision_set_offset_x(w,w->offset_x-1);
	}
}

void zxvision_send_scroll_right(zxvision_window *w)
{
	if (w->offset_x<(w->total_width-1)) {
		zxvision_set_offset_x(w,w->offset_x+1);
	}
}

int zxvision_cursor_out_view(zxvision_window *ventana)
{

        //int linea_retornar;

        if (ventana->visible_cursor) {

                //Ver en que offset estamos
                int offset_y=ventana->offset_y;
                //Y donde esta el cursor
                int cursor=ventana->cursor_line;

                //Y si cursor no esta visible, lo ponemos para que este abajo del todo (hemos de suponer que estaba abajo y ha bajado 1 mas)
                if (cursor<offset_y || cursor>=offset_y+ventana->visible_height-2) {
			return 1;
                }
        }

        return 0;

}


//Retorna 1 si ha reajustado el cursor
int zxvision_adjust_cursor_bottom(zxvision_window *ventana)
{

	//int linea_retornar;

	if (zxvision_cursor_out_view(ventana)) {

		//Ver en que offset estamos
		int offset_y=ventana->offset_y;
		//Y donde esta el cursor
		//int cursor=ventana->cursor_line;

			//printf ("Reajustar cursor\n");
			ventana->cursor_line=offset_y+ventana->visible_height-2-ventana->upper_margin-ventana->lower_margin;
			return 1;
	}

	return 0;

}

//Retorna 1 si ha reajustado el cursor
int zxvision_adjust_cursor_top(zxvision_window *ventana)
{

	if (zxvision_cursor_out_view(ventana)) {

		//Ver en que offset estamos
		int offset_y=ventana->offset_y;
		//Y donde esta el cursor
		//int cursor=ventana->cursor_line;

			if (offset_y>0) {
				//printf ("Reajustar cursor\n");
				ventana->cursor_line=offset_y-1;
				return 1;
			}

	}

	return 0;


}

int zxvision_out_bonds(int x,int y,int ancho,int alto)
{
	if (x<0 || y<0) return 1;

	if (x+ancho>scr_get_menu_width() || y+alto>scr_get_menu_height()) return 1;

	return 0;
}

//Duibujar todas las ventanas que hay debajo de esta en cascada, desde la mas antigua hasta arriba
void zxvision_draw_below_windows(zxvision_window *w)
{
	//Primero ir a buscar la de abajo del todo
	zxvision_window *pointer_window;

	//printf ("original window: %p\n",w);
        //printf ("\noriginal window: %p. Title: %s\n",w,w->window_title);



	pointer_window=w;

	while (pointer_window->previous_window!=NULL) {
		//printf ("zxvision_draw_below_windows. current window %p below window: %p title below: %s\n",pointer_window,pointer_window->previous_window,pointer_window->previous_window->window_title);
		pointer_window=pointer_window->previous_window;
	}

	//printf ("after while pointer_window->previous_window!=NULL\n");

	int antes_ventana_tipo_activa=ventana_tipo_activa;
	ventana_tipo_activa=0; //Redibujar las de debajo como inactivas

	//Redibujar diciendo que estan por debajo
	ventana_es_background=1;

	//Y ahora de ahi hacia arriba
	//Si puntero es NULL, es porque se ha borrado alguna ventana de debajo. Salir
	//esto puede suceder haciendo esto:
	//entrar a debug cpu-breakpoints. activarlo y dejar que salte el tooltip
	//ir a ZRCP. Meter breakpoint que de error, ejemplo: "sb 1 pc=kkkk("
	//ir a menu. enter y enter. Se provoca esta situacion. Por que? Probablemente porque se ha llamado a destroy window y
	//se ha generado una ventana de error cuando habia un tooltip abierto
	//Ver comentarios en zxvision_destroy_window

	//if (pointer_window==NULL) {
	//	printf ("Pointer was null before loop redrawing below windows\n");
	//}	

	//printf ("\nStart loop redrawing below windows\n");

	//no mostrar mensajes de error pendientes
	//si eso se hiciera, aparece en medio de la lista de ventanas una que apunta a null y de ahi la condicion pointer_window!=NULL
	//asi entonces dicha condicion pointer_window!=NULL ya no seria necesaria pero la dejamos por si acaso...
	int antes_no_dibuja_ventana_muestra_pending_error_message=no_dibuja_ventana_muestra_pending_error_message;
	no_dibuja_ventana_muestra_pending_error_message=1;

	while (pointer_window!=w && pointer_window!=NULL) {
		//printf ("window from bottom to top %p\n",pointer_window);
		//printf ("window from bottom to top %p. name: %s\n",pointer_window,pointer_window->window_title);
		
		zxvision_draw_window(pointer_window);
	        zxvision_draw_window_contents(pointer_window);

		pointer_window=pointer_window->next_window;
	}


	no_dibuja_ventana_muestra_pending_error_message=antes_no_dibuja_ventana_muestra_pending_error_message;

	//printf ("Stop loop redrawing below windows\n\n");	

	//if (pointer_window==NULL) {
	//	printf ("Pointer was null redrawing below windows\n");
	//}

	ventana_es_background=0;
	ventana_tipo_activa=antes_ventana_tipo_activa;
}


int zxvision_drawing_in_background=0;

//Dibujar todas las ventanas que hay debajo de esta en cascada, desde la mas antigua hasta arriba, pero llamando solo las que tienen overlay
void zxvision_draw_below_windows_with_overlay(zxvision_window *w)
{

	//zxvision_draw_below_windows(w);
	//return;

        //Primero ir a buscar la de abajo del todo
        zxvision_window *pointer_window;

	//zxvision_window *actual_current_window=zxvision_current_window;

        if (w!=NULL) printf ("\nDraw with overlay. original window: %p. Title: %s\n",w,w->window_title);

        pointer_window=w;

        while (pointer_window->previous_window!=NULL) {
                printf ("zxvision_draw_below_windows_with_overlay below window: %p\n",pointer_window->previous_window);
                pointer_window=pointer_window->previous_window;
        }

        int antes_ventana_tipo_activa=ventana_tipo_activa;
        ventana_tipo_activa=0; //Redibujar las de debajo como inactivas

        //Redibujar diciendo que estan por debajo
        ventana_es_background=1;

        //Y ahora de ahi hacia arriba, incluido la ultima


	printf ("\n");

		zxvision_drawing_in_background=1;

        while (pointer_window!=NULL) {
        //while (pointer_window!=w) {
                printf ("window from bottom to top %p. next: %p nombre: %s\n",pointer_window,pointer_window->next_window,pointer_window->window_title);


		void (*overlay_function)(void);
		overlay_function=pointer_window->overlay_function;

		printf ("Funcion overlay: %p\n",overlay_function);


		//zxvision_current_window=pointer_window;


                zxvision_draw_window(pointer_window);

		//Dibujamos contenido anterior, ya que draw_window la borra con espacios
		zxvision_draw_window_contents(pointer_window);
		//Esto pasa en ventanas que por ejemplo actualizan no a cada frame, al menos refrescar aqui con ultimo valor				

		if (overlay_function!=NULL) {
			printf ("llamando a funcion overlay %p\n",overlay_function);
			
			overlay_function(); //llamar a funcion overlay
		}


		//else zxvision_draw_window_contents(pointer_window);

                pointer_window=pointer_window->next_window;
        }

	//zxvision_current_window=actual_current_window;

		zxvision_drawing_in_background=0;

        ventana_es_background=0;
        ventana_tipo_activa=antes_ventana_tipo_activa;

}


void zxvision_redraw_window_on_move(zxvision_window *w)
{
	cls_menu_overlay();
	zxvision_draw_below_windows_nospeech(w);
	zxvision_draw_window(w);
	zxvision_draw_window_contents(w);
}

void zxvision_set_x_position(zxvision_window *w,int x)
{
	if (zxvision_out_bonds(x,w->y,w->visible_width,w->visible_height)) return;

	w->x=x;
	zxvision_redraw_window_on_move(w);

}

void zxvision_set_y_position(zxvision_window *w,int y)
{
	if (zxvision_out_bonds(w->x,y,w->visible_width,w->visible_height)) return;

	w->y=y;
	zxvision_redraw_window_on_move(w);

}


void zxvision_set_visible_width(zxvision_window *w,int visible_width)
{
	if (zxvision_out_bonds(w->x,w->y,visible_width,w->visible_height)) {
		//printf ("Window out of bounds trying to set width\n");
		return;
	}

	if (visible_width<1) return;

	w->visible_width=visible_width;
	zxvision_redraw_window_on_move(w);

}

void zxvision_set_visible_height(zxvision_window *w,int visible_height)
{
	if (zxvision_out_bonds(w->x,w->y,w->visible_width,visible_height)) return;

	if (visible_height<1) return;

	w->visible_height=visible_height;
	zxvision_redraw_window_on_move(w);

}

/*char *zxvision_get_text_margin(zxvision_window *w,int linea)
{
	int i;
	char *text_margin;
	for (i=0;i<linea;i++) {
		text_margin=w->text_margin[linea];
		if (text_margin==NULL) return NULL;
	}

	return text_margin;

}*/

void zxvision_draw_window_contents_stdout(zxvision_window *w)
{
	//Simple. Mostrarlo todo
	int y,x;

	
	//Simple. Mostrar todas lineas
	char buffer_linea[MAX_BUFFER_SPEECH+1];


	for (y=0;y<w->total_height;y++) {
		int offset_caracter=y*w->total_width;



		for (x=0;x<w->total_width && x<MAX_BUFFER_SPEECH;x++) {

                                overlay_screen *caracter;
                                caracter=w->memory;
                                caracter=&caracter[offset_caracter];

                                z80_byte caracter_escribir=caracter->caracter;

			buffer_linea[x]=caracter_escribir;
			offset_caracter++;
		}

		buffer_linea[x]=0;


                printf ("%s\n",buffer_linea);

                scrstdout_menu_print_speech_macro(buffer_linea);

        }


	//menu_espera_no_tecla();
	//menu_espera_tecla();
}

void zxvision_draw_window_contents(zxvision_window *w)
{

	if (!strcmp(scr_driver_name,"stdout")) {
		zxvision_draw_window_contents_stdout(w);
		return;
	}

	//menu_textspeech_send_text(texto);

	//Buffer para speech
	char buffer_linea[MAX_BUFFER_SPEECH+1];

	int width,height;

	width=zxvision_get_effective_width(w);

	//Alto del contenido es 2 menos, por el titulo de ventana y la linea por debajo de margen
	height=zxvision_get_effective_height(w);

	int x,y;

	for (y=0;y<height;y++) {
		int indice_speech=0;
		for (x=0;x<width;x++) {

			//printf ("x %d y %d\n",x,y);
		
			int xdestination=w->x+x;
			int ydestination=(w->y)+1+y; //y +1 porque empezamos a escribir debajo del titulo

			//obtener caracter
			int out_of_bonds=0;

			int offset_x_final=x+w->offset_x;
			if (offset_x_final>=w->total_width) out_of_bonds=1;

			int offset_y_final=y+w->offset_y;

			int lower_margin_starts_at=height-(w->lower_margin);

			//printf ("sonda 1\n");
				
				//Texto leyenda parte superior
				if (y<w->upper_margin) {
					offset_y_final=y;
				}
				//Texto leyenda parte inferior
				else if (y>=lower_margin_starts_at) {
					int effective_height=height-w->upper_margin-w->lower_margin;
					int final_y=y-effective_height;
					offset_y_final=final_y;
				}
				else {
					offset_y_final +=w->lower_margin; //Dado que ya hemos pasado la parte superior, saltar la inferior
				}
			//printf ("sonda 2\n");

			if (offset_y_final>=w->total_height) out_of_bonds=1;

			if (!out_of_bonds) {

				//Origen de donde obtener el texto
				int offset_caracter;
				
				offset_caracter=((offset_y_final)*w->total_width)+offset_x_final;

				overlay_screen *caracter;
				caracter=w->memory;
				caracter=&caracter[offset_caracter];

				z80_byte caracter_escribir=caracter->caracter;

				int tinta=caracter->tinta;
				int papel=caracter->papel;

				//Si esta linea cursor visible
				int linea_cursor=w->cursor_line;
				//tener en cuenta desplazamiento de margenes superior e inferior
				linea_cursor +=w->lower_margin;
				linea_cursor +=w->upper_margin;
				if (w->visible_cursor && linea_cursor==offset_y_final) {
					tinta=ESTILO_GUI_TINTA_SELECCIONADO;
					papel=ESTILO_GUI_PAPEL_SELECCIONADO;
				} 
			
				//printf ("antes de putchar\n");
				putchar_menu_overlay_parpadeo(xdestination,ydestination,
					caracter_escribir,tinta,papel,caracter->parpadeo);

					//printf ("despues de putchar\n");

				if (indice_speech<MAX_BUFFER_SPEECH) {
					buffer_linea[indice_speech++]=caracter_escribir;
				}
			}

			//Fuera de rango. Metemos espacio
			else {
				putchar_menu_overlay_parpadeo(xdestination,ydestination,
				' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0);
			}
			//printf ("sonda 3\n");

		}

		buffer_linea[indice_speech]=0;
		menu_textspeech_send_text(buffer_linea);

		//printf ("sonda 4\n");
	}


}


void zxvision_draw_window_contents_no_speech(zxvision_window *ventana)
{
                //No queremos que el speech vuelva a leer la ventana, solo cargar ventana
		int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
                menu_speech_tecla_pulsada=1;
                zxvision_draw_window_contents(ventana);

		menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;

}


//Escribir caracter en la memoria de la ventana
void zxvision_print_char(zxvision_window *w,int x,int y,overlay_screen *caracter)
{
	//Comprobar limites
	if (x>=w->total_width || x<0 || y>=w->total_height || y<0) return;

	//Sacamos offset
	int offset=(y*w->total_width)+x;

	/*struct s_overlay_screen {
	z80_byte tinta,papel,parpadeo;
	z80_byte caracter;
};	*/

	//Puntero
	overlay_screen *p;

	p=w->memory; //Puntero inicial

	p=&p[offset]; //Puntero con offset

	p->tinta=caracter->tinta;
	p->papel=caracter->papel;
	p->parpadeo=caracter->parpadeo;
	p->caracter=caracter->caracter;

	
}

void zxvision_print_char_simple(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,z80_byte caracter)
{
	overlay_screen caracter_aux;
	caracter_aux.caracter=caracter;
	caracter_aux.tinta=tinta;
	caracter_aux.papel=papel;
	caracter_aux.parpadeo=parpadeo;		

	zxvision_print_char(w,x,y,&caracter_aux);
}

void zxvision_print_string(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto)
{


	int inverso_letra=0;
	int minuscula_letra=1;
	int era_utf=0;

	while (*texto) {

		overlay_screen caracter_aux;
		caracter_aux.caracter=*texto;

		//TODO: gestion caracteres de control
//Si dos ^ seguidas, invertir estado parpadeo
		if (menu_escribe_texto_si_parpadeo(texto,0)) {
			parpadeo ^=1;
			//y saltamos esos codigos de negado
                        texto +=2;
                        caracter_aux.caracter=*texto;
		}

		//Codigo control color tinta
		if (menu_escribe_texto_si_cambio_tinta(texto,0)) {
			tinta=texto[2]-'0';
			texto+=3;
			caracter_aux.caracter=*texto;
		}

		//ver si dos ~~ o ~^ seguidas y cuidado al comparar que no nos vayamos mas alla del codigo 0 final
		if (menu_escribe_texto_si_inverso(texto,0)) {
			minuscula_letra=1;
			//y saltamos esos codigos de negado. Ver si era ~^, con lo que indica que no hay que bajar a minusculas
			texto++;
			if (*texto=='^') minuscula_letra=0;
			texto++;
			caracter_aux.caracter=*texto;

			if (menu_writing_inverse_color.v) inverso_letra=1;
			else inverso_letra=0;

		}

		//else {

			//Si estaba prefijo utf activo

			if (era_utf) {
				caracter_aux.caracter=menu_escribe_texto_convert_utf(era_utf,*texto);
				era_utf=0;

				//Caracter final utf
				//putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
			}


			//Si no, ver si entra un prefijo utf
			else {
				//printf ("letra: %02XH\n",letra);
				//Prefijo utf
                if (menu_es_prefijo_utf(*texto)) {
        	        era_utf=*texto;
					//printf ("activado utf\n");
	            }

				/*else {
					//Caracter normal
					putchar_menu_overlay_parpadeo(x,y,letra,tinta,papel,parpadeo);
				}*/
			}


		//}


		if (!inverso_letra) {
			caracter_aux.tinta=tinta;
			caracter_aux.papel=papel;
		}
		else {
			caracter_aux.tinta=papel;
			caracter_aux.papel=tinta;			
			//Los hotkeys de menu siempre apareceran en minusculas para ser coherentes
			//De la misma manera, no se soportan hotkeys en menus que sean minusculas
			if (minuscula_letra) caracter_aux.caracter=letra_minuscula(caracter_aux.caracter);			
		}

		inverso_letra=0;


		caracter_aux.parpadeo=parpadeo;


		zxvision_print_char(w,x,y,&caracter_aux);
		if (!era_utf) x++;
		texto++;
	}	
}

void zxvision_print_string_defaults(zxvision_window *w,int x,int y,char *texto)
{

	zxvision_print_string(w,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,texto);

}

void zxvision_fill_width_spaces(zxvision_window *w,int y)
{
	overlay_screen caracter_aux;
	caracter_aux.caracter=' ';
	caracter_aux.tinta=ESTILO_GUI_TINTA_NORMAL;
	caracter_aux.papel=ESTILO_GUI_PAPEL_NORMAL;
	caracter_aux.parpadeo=0;		

	int i;
	for (i=0;i<w->total_width;i++) {
		zxvision_print_char(w,i,y,&caracter_aux);
	}
}

//Igual que la anterior pero antes borra la linea con espacios
void zxvision_print_string_defaults_fillspc(zxvision_window *w,int x,int y,char *texto)
{

	/*overlay_screen caracter_aux;
	caracter_aux.caracter=' ';
	caracter_aux.tinta=ESTILO_GUI_TINTA_NORMAL;
	caracter_aux.papel=ESTILO_GUI_PAPEL_NORMAL;
	caracter_aux.parpadeo=0;		

	int i;
	for (i=0;i<w->total_width;i++) {
		zxvision_print_char(w,i,y,&caracter_aux);
	}*/
	zxvision_fill_width_spaces(w,y);

	zxvision_print_string(w,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,texto);

}

void zxvision_putpixel(zxvision_window *w,int x,int y,int color)
{

	//int final_x,final_y;

	/*

-Como puede ser que al redimensionar ay sheet la ventana tenga más tamaño total... se crea de nuevo al redimensionar?
->no, porque dibuja pixeles con overlay y eso no comprueba si sale del limite virtual de la ventana
Creo que el putpixel en overlay no controla ancho total sino ancho visible. Exacto

Efectivamente. Se usa tamaño visible
Hacerlo constar en zxvision_putpixel como un TODO. Aunque si no hubiera este “fallo”, al redimensionar ay sheet no se vería el tamaño adicional , habría que cerrar la ventana y volverla a abrir ya con el tamaño total nuevo (ya que guarda geometría)
Es lo que pasa con otras ventanas de texto, que no se amplía el ancho total al no recrearse la ventana , y hay que salir y volver a entrar. Ejemplos??


*/
	/*
	    int offsetx=PIANO_GRAPHIC_BASE_X*menu_char_width+12;
    int offsety=piano_graphic_base_y*scale_y_chip(8)+18;

*/
	//Obtener coordenadas en pixeles de zona ventana dibujable
	int window_pixel_start_x=(w->x)*menu_char_width;
	int window_pixel_start_y=((w->y)+1)*8;
	int window_pixel_final_x=window_pixel_start_x+((w->visible_width)-zxvision_get_minus_width_byscrollvbar(w))*menu_char_width;
	int window_pixel_final_y=window_pixel_start_y+((w->visible_height)-2)*8;

	//Obtener coordenada x,y final donde va a parar
	int xfinal=x+window_pixel_start_x-(w->offset_x)*menu_char_width;
	int yfinal=y+window_pixel_start_y-(w->offset_y)*8;

	//Ver si esta dentro de rango
	if (xfinal>=window_pixel_start_x && xfinal<window_pixel_final_x && yfinal>=window_pixel_start_y && yfinal<window_pixel_final_y) {
		menu_scr_putpixel(xfinal,yfinal,color);
	}
	else {
		//printf ("pixel out of window %d %d\n",x,y);
	}
}

int mouse_is_dragging=0;
int window_is_being_moved=0;
int window_is_being_resized=0;
int window_mouse_x_before_move=0;
int window_mouse_y_before_move=0;

int last_x_mouse_clicked=0;
int last_y_mouse_clicked=0;
int mouse_is_clicking=0;
int menu_mouse_left_double_click_counter=0;
int menu_mouse_left_double_click_counter_initial=0;

int mouse_is_double_clicking=0;


void zxvision_handle_mouse_move_aux(zxvision_window *w)
{
				int movimiento_x=menu_mouse_x-window_mouse_x_before_move;
				int movimiento_y=menu_mouse_y-window_mouse_y_before_move;

				//printf ("Windows has been moved. menu_mouse_x: %d (%d) menu_mouse_y: %d (%d)\n",menu_mouse_x,movimiento_x,menu_mouse_y,movimiento_y);
				


				//Actualizar posicion
				int new_x=w->x+movimiento_x;
				int new_y=w->y+movimiento_y;


				zxvision_set_x_position(w,new_x);
				zxvision_set_y_position(w,new_y);
}

void zxvision_handle_mouse_resize_aux(zxvision_window *w)
{
				int incremento_ancho=menu_mouse_x-(w->visible_width)+1;
				int incremento_alto=menu_mouse_y-(w->visible_height)+1;

				//printf ("Incremento %d x %d\n",incremento_ancho,incremento_alto);

				int ancho_final=(w->visible_width)+incremento_ancho;
				int alto_final=(w->visible_height)+incremento_alto;

				//Evitar ventana de ancho pequeño, aunque se puede hacer, pero los colores se van por la izquierda
				if (ancho_final>5) {
					zxvision_set_visible_width(w,ancho_final);
				}

				//Evitar ventana de alto 1, aunque se puede hacer, pero luego no habria zona de redimensionado
				if (alto_final>1) {
					zxvision_set_visible_height(w,alto_final);
				}
}

int zxvision_mouse_in_bottom_right(zxvision_window *w)
{
	if (menu_mouse_x==(w->visible_width)-1 && menu_mouse_y==w->visible_height-1) return 1;

	return 0;
}


void zxvision_handle_minimize(zxvision_window *w)
{

	if (w->can_be_resized) {

		//Para cualquiera de los dos casos, la ponemos como minimizada
		//Luego en restaurar, restauramos valores originales
		//Se hace asi para que se pueda partir de un tamaño minimo y poder restaurar a su tamaño original
		//Si no, las funciones de establecer x,y, ancho, alto, podrian detectar fuera de rango de pantalla y no restaurar

		//Cambiar alto
		zxvision_set_visible_height(w,2);

		//Cambiar ancho
		//primero poner ancho inicial y luego reducir a ancho minimo para que quepa el titulo
		zxvision_set_visible_width(w,w->width_before_max_min_imize);
							
		int ancho_ventana_final=menu_dibuja_ventana_ret_ancho_titulo(w->visible_width,w->window_title);

		//printf ("ancho final: %d\n",ancho_ventana_final);
		zxvision_set_visible_width(w,ancho_ventana_final);

		//Al minimizar/restaurar, desactivamos maximizado
		w->is_maximized=0;

		if (w->is_minimized) {
			//Des-minimizar. Dejar posicion y tamaño original
			//printf ("Unminimize window\n");
			zxvision_set_x_position(w,w->x_before_max_min_imize);
			zxvision_set_y_position(w,w->y_before_max_min_imize);
			zxvision_set_visible_height(w,w->height_before_max_min_imize);
			zxvision_set_visible_width(w,w->width_before_max_min_imize);
			w->is_minimized=0;
		}
		
		else {
			//Ya la hemos minimizado antes. solo indicarlo
			//printf ("Minimize window\n");
			w->is_minimized=1;
		}

		zxvision_draw_window(w);
		zxvision_draw_window_contents(w);
	}

}


void zxvision_handle_maximize(zxvision_window *w)
{

	if (w->can_be_resized) {

		//Para cualquiera de los dos casos, la ponemos como minimizada
		//Luego en restaurar, restauramos valores originales
		//Se hace asi para que se pueda partir de un tamaño minimo y poder restaurar a su tamaño original
		//Si no, las funciones de establecer x,y, ancho, alto, podrian detectar fuera de rango de pantalla y no restaurar

		//Cambiar alto
		zxvision_set_visible_height(w,2);

		//Cambiar ancho
		//primero poner ancho inicial y luego reducir a ancho minimo para que quepa el titulo
		zxvision_set_visible_width(w,w->width_before_max_min_imize);	

		int ancho_ventana_final=menu_dibuja_ventana_ret_ancho_titulo(w->visible_width,w->window_title);

		//printf ("ancho final: %d\n",ancho_ventana_final);
		zxvision_set_visible_width(w,ancho_ventana_final);



		//Al maximizar/restaurar, desactivamos minimizado
		w->is_minimized=0;

		if (w->is_maximized) {
			//Des-minimizar. Dejar posicion y tamaño original
			debug_printf (VERBOSE_DEBUG,"Unmaximize window");
			zxvision_set_x_position(w,w->x_before_max_min_imize);
			zxvision_set_y_position(w,w->y_before_max_min_imize);
			zxvision_set_visible_height(w,w->height_before_max_min_imize);
			zxvision_set_visible_width(w,w->width_before_max_min_imize);

			w->is_maximized=0;
		}
		
		else {
			debug_printf (VERBOSE_DEBUG,"Maximize window");
			zxvision_set_x_position(w,0);
			zxvision_set_y_position(w,0);
			int max_width=scr_get_menu_width();
			int max_height=scr_get_menu_height();
			//printf ("visible width %d\n",max_width);
			zxvision_set_visible_width(w,max_width);
			zxvision_set_visible_height(w,max_height);
			
			w->is_maximized=1;
		}

		zxvision_draw_window(w);
		zxvision_draw_window_contents(w);
	}

}

void zxvision_send_scroll_right_and_draw(zxvision_window *w)
{
						//printf ("Pulsado en scroll derecha\n");
						zxvision_send_scroll_right(w);

						//Redibujar botones scroll. Esto es necesario solo en el caso que,
						//al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
						//se quedaria el color del boton invertido
						zxvision_draw_horizontal_scroll_bar(w,0);	
}

void zxvision_send_scroll_left_and_draw(zxvision_window *w)
{
						//printf ("Pulsado en scroll izquierda\n");
						zxvision_send_scroll_left(w);

						//Redibujar botones scroll. Esto es necesario solo en el caso que,
						//al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
						//se quedaria el color del boton invertido
						zxvision_draw_horizontal_scroll_bar(w,0);
}

void zxvision_send_scroll_up_and_draw(zxvision_window *w)
{
						//printf ("Pulsado en scroll arriba\n");
						zxvision_send_scroll_up(w);

						//Redibujar botones scroll. Esto es necesario solo en el caso que,
						//al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
						//se quedaria el color del boton invertido
						zxvision_draw_vertical_scroll_bar(w,0);
}

void zxvision_send_scroll_down_and_draw(zxvision_window *w)
{
						//printf ("Pulsado en scroll abajo\n");
						zxvision_send_scroll_down(w);

						//Redibujar botones scroll. Esto es necesario solo en el caso que,
						//al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
						//se quedaria el color del boton invertido
						zxvision_draw_vertical_scroll_bar(w,0);	
}

//int zxvision_mouse_events_counter=0;
//int tempconta;
//Retorna 1 si pulsado boton de cerrar ventana
void zxvision_handle_mouse_events(zxvision_window *w)
{

	if (w==NULL) return; // 0; 

	if (!si_menu_mouse_activado()) return; // 0;

	//printf ("zxvision_handle_mouse_events %d\n",tempconta++);
	//int pulsado_boton_cerrar=0;

	menu_calculate_mouse_xy();

	if (mouse_left && !mouse_is_dragging) {
		//Si se pulsa dentro de ventana y no esta arrastrando
	 	if (si_menu_mouse_en_ventana() && !zxvision_keys_event_not_send_to_machine) {
			//printf ("Clicked inside window. Events are not sent to emulated machine\n");
			zxvision_keys_event_not_send_to_machine=1;
			ventana_tipo_activa=1;
			zxvision_draw_window(w);
			zxvision_draw_window_contents(w);
		}

		if (!si_menu_mouse_en_ventana() && zxvision_keys_event_not_send_to_machine) {
			//Si se pulsa fuera de ventana
			//printf ("Clicked outside window. Events are sent to emulated machine\n");
			zxvision_keys_event_not_send_to_machine=0;
			ventana_tipo_activa=0;
			zxvision_draw_window(w);
			zxvision_draw_window_contents(w);
		}
	}


	if (mouse_movido) {
		//printf ("mouse movido\n");
		if (si_menu_mouse_en_ventana() ) {
				//if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) {
					//printf ("dentro ventana\n");
					if (menu_mouse_y==0) {
						//printf ("En barra titulo\n");
					}
					//Descartar linea titulo y ultima linea
		}

	}

	if (mouse_left) {
		//printf ("Pulsado boton izquierdo\n");

		if (!mouse_movido) {
			if (!mouse_is_clicking) {
				//printf ("Mouse started clicking\n");
				mouse_is_clicking=1;
				last_x_mouse_clicked=menu_mouse_x;
				last_y_mouse_clicked=menu_mouse_y;


				//Gestion doble click
				if (menu_multitarea) {
					if (menu_mouse_left_double_click_counter-menu_mouse_left_double_click_counter_initial<25) {
						//printf ("-IT is DOBLE click\n");
						mouse_is_double_clicking=1;
					}
					else {
						menu_mouse_left_double_click_counter_initial=menu_mouse_left_double_click_counter;
						mouse_is_double_clicking=0;
					}
				}

				else {
					//Sin multitarea nunca hay doble click
					mouse_is_double_clicking=0;
				}
			}
		}
	}

	//Si empieza a pulsar botón izquierdo
	if (mouse_left && mouse_is_clicking) {

		if (si_menu_mouse_en_ventana()) {
			//Pulsado en barra titulo
			if (last_y_mouse_clicked==0) {
				if (!mouse_is_double_clicking) {
						//Si pulsa boton cerrar ventana
					if (last_x_mouse_clicked==0 && menu_hide_close_button.v==0) {
						//printf ("pulsado boton cerrar\n");
						//pulsado_boton_cerrar=1;
						mouse_pressed_close_window=1;
						//Mostrar boton cerrar pulsado
						putchar_menu_overlay(w->x,w->y,ESTILO_GUI_BOTON_CERRAR,ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
					}

					//Si se pulsa en boton minimizar, indicar que se esta pulsando
					if (last_x_mouse_clicked==w->visible_width-1 && menu_hide_minimize_button.v==0 && w->can_be_resized) {
						putchar_menu_overlay(w->x+w->visible_width-1,w->y,menu_retorna_caracter_minimizar(w),ESTILO_GUI_PAPEL_TITULO,ESTILO_GUI_TINTA_TITULO);
					}
				}
			}

			//Pulsado en botones Scroll horizontal
			if (zxvision_if_horizontal_scroll_bar(w)) {
				if (last_y_mouse_clicked==w->visible_height-1) {
					//Linea scroll horizontal
					int posicion_flecha_izquierda=1;
					int posicion_flecha_derecha=w->visible_width-2;

					//Flecha izquierda
					if (last_x_mouse_clicked==posicion_flecha_izquierda) {
						//printf ("Pulsado en scroll izquierda\n");
						//putchar_menu_overlay(w->x+posicion_flecha_izquierda,w->y+w->visible_height-1,'<',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
						zxvision_draw_horizontal_scroll_bar(w,1);

					}
					//Flecha derecha
					if (last_x_mouse_clicked==posicion_flecha_derecha) {
						//printf ("Pulsado en scroll derecha\n");
						//putchar_menu_overlay(w->x+posicion_flecha_derecha,w->y+w->visible_height-1,'>',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
						zxvision_draw_horizontal_scroll_bar(w,2);
					
					}

					//Zona porcentaje
					if (last_x_mouse_clicked>posicion_flecha_izquierda && last_x_mouse_clicked<posicion_flecha_derecha) {
						//printf ("Pulsado en zona scroll horizontal\n");
						zxvision_draw_horizontal_scroll_bar(w,3);
					}

				}
			}

			//Pulsado en botones Scroll vertical
			if (zxvision_if_vertical_scroll_bar(w)) {
				if (last_x_mouse_clicked==w->visible_width-1) {
					//Linea scroll vertical
					int posicion_flecha_arriba=1;
					int posicion_flecha_abajo=w->visible_height-2;

					//Flecha arriba
					if (last_y_mouse_clicked==posicion_flecha_arriba) {
						//printf ("Pulsado en scroll arriba\n");
						//putchar_menu_overlay(w->x+w->visible_width-1,w->y+posicion_flecha_arriba,'^',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
						zxvision_draw_vertical_scroll_bar(w,1);
					}

					//Flecha abajo
					if (last_y_mouse_clicked==posicion_flecha_abajo) {
						//printf ("Pulsado en scroll abajo\n");
						//putchar_menu_overlay(w->x+w->visible_width-1,w->y+posicion_flecha_abajo,'v',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
						zxvision_draw_vertical_scroll_bar(w,2);
					}

					if (last_y_mouse_clicked>posicion_flecha_arriba && last_y_mouse_clicked<posicion_flecha_abajo) {
						//printf ("Pulsado en zona scroll vertical\n");
						zxvision_draw_vertical_scroll_bar(w,3);
					}

				}
			}

		}
	}

	//Liberación boton izquierdo
	if (!mouse_left && mouse_is_clicking && !mouse_is_dragging) {
			//printf ("Mouse stopped clicking mouse_is_dragging %d\n",mouse_is_dragging);
			mouse_is_clicking=0;
			//Pulsacion en sitios de ventana
			//Si en barra titulo
			if (si_menu_mouse_en_ventana() && last_y_mouse_clicked==0) {
				//printf ("Clicked on title\n");
				//Y si ha sido doble click
				if (mouse_is_double_clicking) {
					debug_printf (VERBOSE_DEBUG,"Double clicked on title");

					zxvision_handle_maximize(w);
					
					
				}
				else {
					//Simple click
					//Si pulsa zona minimizar
					if (last_x_mouse_clicked==w->visible_width-1 && menu_hide_minimize_button.v==0) {
						//Mostrar boton minimizar pulsado
						//printf ("minimizar\n");
						//putchar_menu_overlay(w->x+w->visible_width-2,w->y,'X',ESTILO_GUI_TINTA_TITULO,ESTILO_GUI_PAPEL_TITULO);
						//menu_refresca_pantalla();
						zxvision_handle_minimize(w);
					}
					//Si pulsa boton cerrar ventana
					/*if (last_x_mouse_clicked==0 && menu_hide_close_button.v==0) {
						printf ("pulsado boton cerrar\n");
						//pulsado_boton_cerrar=1;
						mouse_pressed_close_window=1;
					}*/

				}

				
			}

			//Scroll horizontal
			if (zxvision_if_horizontal_scroll_bar(w)) {
				if (last_y_mouse_clicked==w->visible_height-1) {
					//Linea scroll horizontal
					int posicion_flecha_izquierda=1;
					int posicion_flecha_derecha=w->visible_width-2;

					//Flecha izquierda
					if (last_x_mouse_clicked==posicion_flecha_izquierda) {
						zxvision_send_scroll_left_and_draw(w);			
					}

					//Flecha derecha
					if (last_x_mouse_clicked==posicion_flecha_derecha) {
						zxvision_send_scroll_right_and_draw(w);			
					}

					if (last_x_mouse_clicked>posicion_flecha_izquierda && last_x_mouse_clicked<posicion_flecha_derecha) {
						//printf ("Pulsado en zona scroll horizontal\n");
						//Sacamos porcentaje
						int total_ancho=posicion_flecha_derecha-posicion_flecha_izquierda;
						if (total_ancho==0) total_ancho=1; //Evitar dividir por cero

						int parcial_ancho=last_x_mouse_clicked-posicion_flecha_izquierda;

						int porcentaje=(parcial_ancho*100)/total_ancho;

						//printf ("Porcentaje: %d\n",porcentaje);

						int offset_to_mult=w->total_width-w->visible_width+1; //+1 porque se pierde linea derecha por scroll
						//printf ("Multiplicando sobre %d\n",offset_to_mult);

						//Establecemos offset horizontal
						int offset=((offset_to_mult)*porcentaje)/100;

						//printf ("set offset: %d\n",offset);

						//Casos especiales de izquierda del todo y derecha del todo
						if (last_x_mouse_clicked==posicion_flecha_izquierda+1) {
							//printf ("Special case: clicked on the top left. Set offset 0\n");
							offset=0;
						}

						if (last_x_mouse_clicked==posicion_flecha_derecha-1) {
							//printf ("Special case: clicked on the top right. Set offset to maximum\n");
							offset=w->total_width-w->visible_width+1;
						}

						zxvision_set_offset_x(w,offset);


						//Redibujar botones scroll. Esto es necesario solo en el caso que,
                                                //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                                                //se quedaria el color del boton invertido
                                                zxvision_draw_horizontal_scroll_bar(w,0);

					}
				}
			} 


			//Scroll vertical
			if (zxvision_if_vertical_scroll_bar(w)) {
				if (last_x_mouse_clicked==w->visible_width-1) {
					//Linea scroll vertical
					int posicion_flecha_arriba=1;
					int posicion_flecha_abajo=w->visible_height-2;

					//Flecha arriba
					if (last_y_mouse_clicked==posicion_flecha_arriba) {
						zxvision_send_scroll_up_and_draw(w);
					}

					//Flecha abajo
					if (last_y_mouse_clicked==posicion_flecha_abajo) {
						zxvision_send_scroll_down_and_draw(w);					
					}

					if (last_y_mouse_clicked>posicion_flecha_arriba && last_y_mouse_clicked<posicion_flecha_abajo) {
						//printf ("Pulsado en zona scroll vertical\n");
						//Sacamos porcentaje
						int total_alto=posicion_flecha_abajo-posicion_flecha_arriba;
						if (total_alto==0) total_alto=1; //Evitar dividir por cero

						int parcial_alto=last_y_mouse_clicked-posicion_flecha_arriba;

						int porcentaje=(parcial_alto*100)/total_alto;

						//printf ("Porcentaje: %d\n",porcentaje);


						int offset_to_mult=w->total_height-w->visible_height+2; //+2 porque se pierde linea abajo de scroll y titulo
						//printf ("Multiplicando sobre %d\n",offset_to_mult);

						//Establecemos offset vertical
						int offset=((offset_to_mult)*porcentaje)/100;

						//printf ("set offset: %d\n",offset);

						//Casos especiales de arriba del todo y abajo del todo
						if (last_y_mouse_clicked==posicion_flecha_arriba+1) {
							//printf ("Special case: clicked on the top. Set offset 0\n");
							offset=0;
						}

						if (last_y_mouse_clicked==posicion_flecha_abajo-1) {
							//printf ("Special case: clicked on the bottom. Set offset to maximum\n");
							offset=w->total_height-w->visible_height+2;
						}

						zxvision_set_offset_y(w,offset);

						//Redibujar botones scroll. Esto es necesario solo en el caso que,
                                                //al empezar a pulsar boton, este se invierte el color, y si está el scroll en el limite y no actua,
                                                //se quedaria el color del boton invertido
                                                zxvision_draw_vertical_scroll_bar(w,0);

					}
				}
			} 


	}

	if (mouse_wheel_vertical && zxvision_if_vertical_scroll_bar(w)) {
		int leido_mouse_wheel_vertical=mouse_wheel_vertical;
		//printf ("Read mouse vertical wheel from zxvision_handle_mouse_events : %d\n",leido_mouse_wheel_vertical);

		//Si invertir movimiento
		if (menu_invert_mouse_scroll.v) leido_mouse_wheel_vertical=-leido_mouse_wheel_vertical;

		while (leido_mouse_wheel_vertical<0) {
			zxvision_send_scroll_down_and_draw(w);
			leido_mouse_wheel_vertical++;
		}

		while (leido_mouse_wheel_vertical>0) {
			zxvision_send_scroll_up_and_draw(w);
			leido_mouse_wheel_vertical--;
		}
		
		//Y resetear a 0. importante
		mouse_wheel_vertical=0;
	}

	if (mouse_wheel_horizontal && zxvision_if_horizontal_scroll_bar(w)) {
		int leido_mouse_wheel_horizontal=mouse_wheel_horizontal;
		//printf ("Read mouse horizontal wheel from zxvision_handle_mouse_events : %d\n",leido_mouse_wheel_horizontal);
	

		//Si invertir movimiento
		if (menu_invert_mouse_scroll.v) leido_mouse_wheel_horizontal=-leido_mouse_wheel_horizontal;		


		while (leido_mouse_wheel_horizontal<0) {
			zxvision_send_scroll_right_and_draw(w);
			leido_mouse_wheel_horizontal++;
		}

		while (leido_mouse_wheel_horizontal>0) {
			zxvision_send_scroll_left_and_draw(w);
			leido_mouse_wheel_horizontal--;
		}
		
		//Y resetear a 0. importante
		mouse_wheel_horizontal=0;
	}	

	if (!mouse_is_dragging) {
		if (mouse_left && mouse_movido) {
			//printf ("Mouse has begun to drag\n");
			mouse_is_dragging=1;

			//Si estaba en titulo
			if (si_menu_mouse_en_ventana()) {
				if (menu_mouse_y==0) {
					//printf ("Arrastrando ventana\n");
					window_is_being_moved=1;
					window_mouse_x_before_move=menu_mouse_x;
					window_mouse_y_before_move=menu_mouse_y;
				}

				//Si esta en esquina inferior derecha (donde se puede redimensionar) y se permite resize
				if (zxvision_mouse_in_bottom_right(w) && w->can_be_resized) {
					//printf ("Mouse dragging in bottom right\n");

					window_is_being_resized=1;
					window_mouse_x_before_move=menu_mouse_x;
					window_mouse_y_before_move=menu_mouse_y;					
				}				
			}


		}
	}

	if (mouse_is_dragging) {
		//printf ("mouse is dragging\n");
		if (!mouse_left) { 
			//printf ("Mouse has stopped to drag\n");
			mouse_is_dragging=0;
			mouse_is_clicking=0; //Cuando se deja de arrastrar decir que se deja de pulsar tambien
			if (window_is_being_moved) {

				//printf ("Handle moved window\n");
				zxvision_handle_mouse_move_aux(w);
				window_is_being_moved=0;

			}

			if (window_is_being_resized) {

				//printf ("Handle resized window\n");
				zxvision_handle_mouse_resize_aux(w);
				

				window_is_being_resized=0;
			}
		}

		else {
			if (window_is_being_moved) {
				//Si se ha movido un poco
				if (menu_mouse_y!=window_mouse_y_before_move || menu_mouse_x!=window_mouse_x_before_move) {
					//printf ("Handle moved window\n");
					zxvision_handle_mouse_move_aux(w);
				
					//Hay que recalcular menu_mouse_x y menu_mouse_y dado que son relativos a la ventana que justo se ha movido
					//menu_mouse_y siempre sera 0 dado que el titulo de la ventana, donde se puede arrastrar para mover, es posicion relativa 0
					menu_calculate_mouse_xy();

					window_mouse_y_before_move=menu_mouse_y;
					window_mouse_x_before_move=menu_mouse_x;
									
				}
			}

			if (window_is_being_resized) {
				//Si se ha redimensionado un poco
				if (menu_mouse_y!=window_mouse_y_before_move || menu_mouse_x!=window_mouse_x_before_move) {
					//printf ("Handle resized window\n");
					zxvision_handle_mouse_resize_aux(w);

					window_mouse_y_before_move=menu_mouse_y;
					window_mouse_x_before_move=menu_mouse_x;					
				}
			}
		}
	}

	//if (mouse_left && mouse_movido) printf ("Mouse is dragging\n");
	//return pulsado_boton_cerrar;
}

//Funcion comun que usan algunas ventanas para movimiento de cursores y pgup/dn
void zxvision_handle_cursors_pgupdn(zxvision_window *ventana,z80_byte tecla)
{
	int contador_pgdnup;
	int tecla_valida=1;
					switch (tecla) {

		                //abajo
                        case 10:
						zxvision_send_scroll_down(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
                        break;

                        //arriba
                        case 11:
						zxvision_send_scroll_up(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
                        break;

                        //izquierda
                        case 8:
						zxvision_send_scroll_left(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
                        break;

                        //derecha
                        case 9:
						zxvision_send_scroll_right(ventana);

						//Decir que se ha pulsado tecla para que no se relea
						menu_speech_tecla_pulsada=1;
                        break;						

						//PgUp
						case 24:
							for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2;contador_pgdnup++) {
								zxvision_send_scroll_up(ventana);
							}
							//Decir que no se ha pulsado tecla para que se relea
							menu_speech_tecla_pulsada=0;
						break;

                    	//PgDn
                    	case 25:
                    		for (contador_pgdnup=0;contador_pgdnup<ventana->visible_height-2;contador_pgdnup++) {
								zxvision_send_scroll_down(ventana);
                        	}

							//Decir que no se ha pulsado tecla para que se relea
							menu_speech_tecla_pulsada=0;
                    	break;

						//Mover ventana 
						case 'Q':
							zxvision_set_y_position(ventana,ventana->y-1);
						break;

						case 'A':
							zxvision_set_y_position(ventana,ventana->y+1);
						break;

						case 'O':
							zxvision_set_x_position(ventana,ventana->x-1);
						break;						

						case 'P':
							zxvision_set_x_position(ventana,ventana->x+1);
						break;						

						//Redimensionar ventana
						//Mover ventana 
						case 'W':
							if (ventana->visible_height-1>1) zxvision_set_visible_height(ventana,ventana->visible_height-1);
						break;		

						case 'S':
							zxvision_set_visible_height(ventana,ventana->visible_height+1);
						break;	

						case 'K':
							if (ventana->visible_width-1>5) zxvision_set_visible_width(ventana,ventana->visible_width-1);
						break;									

						case 'L':
							zxvision_set_visible_width(ventana,ventana->visible_width+1);
						break;											
					
						default:
							tecla_valida=0;
						break;

				}

	if (tecla_valida) {
		//Refrescamos pantalla para reflejar esto, util con multitask off
		if (!menu_multitarea) {
			//printf ("refresca pantalla\n");
			menu_refresca_pantalla();
		}		
	}

}

/*
Funcion comun usados en algunas ventanas que:
-refrescan pantalla
-ejecutan core loop si multitask activo
-leen tecla y esperan a liberar dicha tecla
*/
z80_byte zxvision_common_getkey_refresh(void)
{
	z80_byte tecla;

	     if (!menu_multitarea) {
			//printf ("refresca pantalla\n");
			menu_refresca_pantalla();
		}					

		
	            menu_cpu_core_loop();


				menu_espera_tecla();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13 && mouse_left) {	
					tecla=0;
				}

		if (tecla) {
			//printf ("Esperamos no tecla\n");
			menu_espera_no_tecla_con_repeticion();
		}	

	return tecla;
}


//Igual que zxvision_common_getkey_refresh pero sin esperar a no tecla
z80_byte zxvision_common_getkey_refresh_noesperanotec(void)
{
	z80_byte tecla;

	     if (!menu_multitarea) {
			//printf ("refresca pantalla\n");
			menu_refresca_pantalla();
		}					

		
	            menu_cpu_core_loop();


				menu_espera_tecla();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13 && mouse_left) {	
					tecla=0;
				}

	return tecla;
}

z80_byte zxvision_common_getkey_refresh_noesperatecla(void)
//Igual que zxvision_common_getkey_refresh pero sin esperar tecla cuando multitarea activa
{

	z80_byte tecla;

                menu_cpu_core_loop();

            	//si no hay multitarea, refrescar pantalla para mostrar contenido ventana rellenada antes, esperar tecla, 
                if (menu_multitarea==0) {
						menu_refresca_pantalla();
                        menu_espera_tecla();
                        //acumulado=0;
                }				

				tecla=zxvision_read_keyboard();

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)

				if (tecla==13 && mouse_left) {	
					tecla=0;
				}					


		if (tecla) {
			//printf ("Esperamos no tecla\n");
			menu_espera_no_tecla_con_repeticion();
		}	

	return tecla;
}

//Retorna 1 si la tecla no se tiene que enviar a la maquina emulada
//esto es , cuando el menu esta abierto y la ventana tiene el foco
//En cambio retorna 0 (la tecla se va a enviar a la maquina emulada), cuando el menu esta cerrado o la ventana no tiene el foco
int zxvision_key_not_sent_emulated_mach(void)
{
	if (menu_abierto==1 && zxvision_keys_event_not_send_to_machine) return 1;
	else return 0;
}



//Crea ventana simple de 1 de alto con funcion para condicion de salida, y funcion de print. 
void zxvision_simple_progress_window(char *titulo, int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) )
{
	    zxvision_window ventana;

		int alto_ventana=4;
		int ancho_ventana=28;


        int x_ventana=menu_center_x()-ancho_ventana/2; 
        int y_ventana=menu_center_y()-alto_ventana/2; 

        zxvision_new_window(&ventana,x_ventana,y_ventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,titulo);

        zxvision_draw_window(&ventana);


        zxvision_draw_window_contents(&ventana);

             
		zxvision_espera_tecla_condicion_progreso(&ventana,funcioncond,funcionprint);


        cls_menu_overlay();

        zxvision_destroy_window(&ventana);
}

//Retorna el item i
menu_item *menu_retorna_item(menu_item *m,int i)
{

	menu_item *item_next;

        while (i>0)
        {
        	//printf ("m: %p i: %d\n",m,i);
        	item_next=m->next;
        	if (item_next==NULL) return m;  //Controlar si final

                m=item_next;
		i--;
        }

	return m;


}


//Retorna el item i
menu_item *menu_retorna_item_tabulado_xy(menu_item *m,int x,int y,int *linea_buscada)
{

	menu_item *item_next;
	int indice=0;
	int encontrado=0;

	//printf ("buscar item x: %d y: %d\n",x,y);

        while (!encontrado)
        {

        	//Ver si coincide y. x tiene que estar en el rango del texto
        	int longitud_texto=menu_calcular_ancho_string_item(m->texto_opcion);
        	if (y==m->menu_tabulado_y && 
        	    x>=m->menu_tabulado_x && x<m->menu_tabulado_x+longitud_texto) 
        	{
        		encontrado=1;
        	}

        	else {

        		//printf ("m: %p i: %d\n",m,i);
        		item_next=m->next;
	        	if (item_next==NULL) return NULL;  //Controlar si final

                	m=item_next;
			//i--;
			indice++;
		}
        }

        if (encontrado) {
        	*linea_buscada=indice;
		return m;
	}

	else return NULL;


}

void menu_cpu_core_loop(void)
{
                if (menu_multitarea==1) cpu_core_loop();
                else {
                        scr_actualiza_tablas_teclado();
			realjoystick_main();

                        //0.5 ms
                        usleep(MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK);


			//printf ("en menu_cpu_core_loop\n");
                }

}



int si_menu_mouse_en_ventana(void)
{
	if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<current_win_ancho && menu_mouse_y<current_win_alto ) return 1;
	return 0;
}


int menu_allows_mouse(void)
{
	//Primero, fbdev no permite raton
	if (!strcmp(scr_driver_name,"fbdev")) return 0;

	//Luego, el resto de los drivers completos (xwindows, sdl, cocoa, ...)

	return si_complete_video_driver();
}

void menu_calculate_mouse_xy(void)
{
	int x,y;
	if (menu_allows_mouse() ) {

		int mouse_en_emulador=0;
		//printf ("x: %04d y: %04d\n",mouse_x,mouse_y);

		int ancho=screen_get_window_size_width_zoom_border_en();

		ancho +=screen_get_ext_desktop_width_zoom();

		if (mouse_x>=0 && mouse_y>=0
			&& mouse_x<=ancho && mouse_y<=screen_get_window_size_height_zoom_border_en() ) {
				//Si mouse esta dentro de la ventana del emulador
				mouse_en_emulador=1;
		}

		if (  (mouse_x!=last_mouse_x || mouse_y !=last_mouse_y) && mouse_en_emulador) {
			mouse_movido=1;
		}
		else mouse_movido=0;

		last_mouse_x=mouse_x;
		last_mouse_y=mouse_y;

		//printf ("x: %04d y: %04d movido=%d\n",mouse_x,mouse_y,mouse_movido);

		//Quitarle el zoom
		x=mouse_x/zoom_x;
		y=mouse_y/zoom_y;

		//Considerar borde pantalla

		//Todo lo que sea negativo o exceda border, nada.

		//printf ("x: %04d y: %04d\n",x,y);



        //margenes de zona interior de pantalla. para modo rainbow
				int margenx_izq;
				int margeny_arr;
				menu_retorna_margenes_border(&margenx_izq,&margeny_arr);

	//Ya no hace falta restar margenes
	margenx_izq=margeny_arr=0;

	x -=margenx_izq;
	y -=margeny_arr;

	//printf ("x: %04d y: %04d\n",x,y);

	//Aqui puede dar negativo, en caso que cursor este en el border
	//si esta justo en los ultimos 8 pixeles, dara entre -7 y -1. al dividir entre 8, retornaria 0, diciendo erroneamente que estamos dentro de ventana

	if (x<0) x-=(menu_char_width*menu_gui_zoom); //posicion entre -7 y -1 y demas, cuenta como -1, -2 al dividir entre 8
	if (y<0) y-=(8*menu_gui_zoom);

	x /=menu_char_width;
	y /=8;

	x /= menu_gui_zoom;
	y /= menu_gui_zoom;

	x -=current_win_x;
	y -=current_win_y;

	menu_mouse_x=x;
	menu_mouse_y=y;

	//if (x<=0 || y<=0) printf ("x: %04d y: %04d final\n",x,y);

	//printf ("ventana_x %d margen_izq %d\n",ventana_x,margenx_izq);

	//Coordenadas menu_mouse_x y tienen como origen 0,0 en zona superior izquierda de ventana (titulo ventana)
	//Y en coordenadas de linea (y=0 primera linea, y=1 segunda linea, etc)

	}
}



//No dejar aparecer el osd keyboard dentro del mismo osd keyboard
int osd_kb_no_mostrar_desde_menu=0;
int timer_osd_keyboard_menu=0;

z80_byte menu_da_todas_teclas(void)
{

	//Ver tambien eventos de mouse de zxvision
    //int pulsado_boton_cerrar=
	zxvision_handle_mouse_events(zxvision_current_window);

    //On screen keyboard desde el propio menu. Necesita multitask
    if (menu_si_pulsada_tecla_osd() && !osd_kb_no_mostrar_desde_menu && !timer_osd_keyboard_menu && menu_multitarea) {
			debug_printf(VERBOSE_INFO,"Calling osd keyboard from menu keyboard read routine");

			osd_kb_no_mostrar_desde_menu=1;
                        menu_call_onscreen_keyboard_from_menu();
                        //TODO: si se pulsa CS o SS, no lo detecta como tecla pulsada (en parte logico)
                        //pero esto hara que al pulsar una de esas teclas no se abra el menu osd de nuevo hasta que se pulse otra
                        //tecla distinta
                        //printf ("despues de haber leido tecla de osd\n");
			osd_kb_no_mostrar_desde_menu=0;

			//Esperar 1 segundo hasta poder abrir menu osd. La pulsacion de teclas desde osd se hace por medio segundo,
			//con lo que al retornar a 1 segundo ya es correcto
			timer_osd_keyboard_menu=50;
    }



	z80_byte acumulado;

	acumulado=255;

	//symbol i shift no cuentan por separado
	acumulado=acumulado & (puerto_65278 | 1) & puerto_65022 & puerto_64510 & puerto_63486 & puerto_61438 & puerto_57342 & puerto_49150 & (puerto_32766 |2) & puerto_especial1 & puerto_especial2 & puerto_especial3 & puerto_especial4;


	//Boton cerrar de ventana
	if (mouse_pressed_close_window) {
		acumulado |=1;
		//mouse_pressed_close_window=0;
	}


	//no ignorar disparo
	z80_byte valor_joystick=(puerto_especial_joystick&31)^255;
	acumulado=acumulado & valor_joystick;

	//contar tambien botones mouse
	if (si_menu_mouse_activado()) {
		//menu_calculate_mouse_xy(); //Ya no hacemos esto pues se ha calculado ya arriba en zxvision_handle_mouse_events
		//quiza pareceria que no hay problema en leerlo dos veces, el problema es con la variable mouse_leido,
		//que al llamarla aqui la segunda vez, siempre dira que el mouse no se ha movido

		z80_byte valor_botones_mouse=(mouse_left | mouse_right | mouse_movido)^255;
		acumulado=acumulado & valor_botones_mouse;
	}

	//Contar también algunas teclas solo menu:
	z80_byte valor_teclas_menus=(menu_backspace.v|menu_tab.v)^255;
	acumulado=acumulado & valor_teclas_menus;



  

	if ( (acumulado&MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return acumulado;

	

	return acumulado;


}


//Para forzar desde remote command a salir de la funcion, sin haber pulsado tecla realmente
//z80_bit menu_espera_tecla_no_cpu_loop_flag_salir={0};

//Esperar a pulsar una tecla sin ejecutar cpu
void menu_espera_tecla_no_cpu_loop(void)
{ 

	z80_byte acumulado;

        do {

                scr_actualiza_tablas_teclado();
		realjoystick_main();

                //0.5 ms
                usleep(500);

                acumulado=menu_da_todas_teclas();

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA
				//&&	menu_espera_tecla_no_cpu_loop_flag_salir.v==0
							);


	//menu_espera_tecla_no_cpu_loop_flag_salir.v=0;

}



void menu_espera_no_tecla_no_cpu_loop(void)
{

        //Esperar a liberar teclas
        z80_byte acumulado;

        do {

                scr_actualiza_tablas_teclado();
		realjoystick_main();

                //0.5 ms
                usleep(500);

                acumulado=menu_da_todas_teclas();

        //printf ("menu_espera_no_tecla_no_cpu_loop acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA);

}


void menu_espera_tecla_timeout_tooltip(void)
{

        //Esperar a pulsar una tecla o timeout de tooltip
        z80_byte acumulado;

        acumulado=menu_da_todas_teclas();

        int resetear_contadores=0;

        //Si se entra y no hay tecla pulsada, resetear contadores
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA) {
        	resetear_contadores=1;
        }

        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

		//printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && menu_tooltip_counter<TOOLTIP_SECONDS);

	if (resetear_contadores) {
        	menu_reset_counters_tecla_repeticion();
	}

}

/*
void menu_espera_tecla_timeout_window_splash(void)
{
	//printf ("espera splash\n");

        //Esperar a pulsar una tecla o timeout de window splash
        z80_byte acumulado;

        int contador_antes=menu_window_splash_counter_ms;
        int trozos=4;
        //WINDOW_SPLASH_SECONDS. 
        //5 pasos. total de WINDOW_SPLASH_SECONDS
        int tiempototal=1000*WINDOW_SPLASH_SECONDS;
        //Quitamos 1 segundo
        tiempototal-=1000;

        //Intervalo de cambio
        int intervalo=tiempototal/5; //5 pasos
        //printf ("intervalo: %d\n",intervalo);



        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

                //Cada 400 ms
                if (menu_window_splash_counter_ms-contador_antes>intervalo) {
                	trozos--;
                	contador_antes=menu_window_splash_counter_ms;
                	//printf ("dibujar franjas trozos: %d\n",trozos);
                	if (trozos>=0) {		
                		menu_dibuja_ventana_franja_arcoiris_trozo_current(trozos);
                	}
                }


		//printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);
		//printf ("contador splash: %d\n",menu_window_splash_counter);
		

	} while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && menu_window_splash_counter<WINDOW_SPLASH_SECONDS);

}*/

//tipo: 1 volver timeout normal como ventanas splash. 2. no finaliza, franjas continuamente moviendose
void zxvision_espera_tecla_timeout_window_splash(int tipo)
{

	z80_byte tecla;
	//printf ("espera splash\n");
	do {

        //Esperar a pulsar una tecla o timeout de window splash
        //z80_byte acumulado;

        int contador_antes=menu_window_splash_counter_ms;
        int trozos=4;
        //WINDOW_SPLASH_SECONDS. 
        //5 pasos. total de WINDOW_SPLASH_SECONDS
        int tiempototal=1000*WINDOW_SPLASH_SECONDS;
        //Quitamos 1 segundo
        tiempototal-=1000;

        //Intervalo de cambio
        int intervalo=tiempototal/5; //5 pasos
        //printf ("intervalo: %d\n",intervalo);

		int indice_apagado=0;


	

    do {
                menu_cpu_core_loop();


                //acumulado=menu_da_todas_teclas();
				tecla=zxvision_read_keyboard();

				//con boton izquierdo no salimos
				if (tecla==13 && mouse_left) {	
					tecla=0;
				}				

                //Cada 400 ms
                if (menu_window_splash_counter_ms-contador_antes>intervalo) {
                	trozos--;
                	contador_antes=menu_window_splash_counter_ms;
                	//printf ("dibujar franjas trozos: %d\n",trozos);
                	if (trozos>=0) {		
                		if (tipo==1) menu_dibuja_ventana_franja_arcoiris_trozo_current(trozos);
                	}

					if (tipo==2) menu_dibuja_ventana_franja_arcoiris_oscuro_current(indice_apagado);
					indice_apagado++;
                }


		//printf ("menu_espera_tecla_timeout_tooltip acumulado: %d\n",acumulado);
		//printf ("contador splash: %d\n",menu_window_splash_counter);
		

	} while (tecla==0 && menu_window_splash_counter<WINDOW_SPLASH_SECONDS);

	menu_window_splash_counter=0;
	menu_window_splash_counter_ms=0;

	} while (tipo==2 && tecla==0);

}

//Esperar a tecla ESC, o que la condicion de funcion sea diferente de 0
//Cada medio segundo se llama la condicion y tambien la funcion de print
//Poner alguna a NULL si no se quiere llamar
//Funciones de condicion y progreso tambien funcionan aun sin multitarea
void zxvision_espera_tecla_condicion_progreso(zxvision_window *w,int (*funcioncond) (zxvision_window *),void (*funcionprint) (zxvision_window *) )
{

	z80_byte tecla;
	int condicion=0;
	int contador_antes=menu_window_splash_counter_ms;
	int intervalo=20*12; //12 frames de pantalla

	//contador en us
	int contador_no_multitask=0;


	//printf ("espera splash\n");
	do {

                menu_cpu_core_loop();
				int pasado_cuarto_segundo=0;

				//TODO: se puede dar el caso que se llame aqui pero el thread aun no se haya creado, lo que provoca
				//que dice que el thread no esta en ejecucion aun y por tanto cree que esta finalizado, diciendo que la condicion de salida es verdadera
				//y salga cuando aun no ha finalizado
				//Seria raro, porque el intervalo de comprobacion es cada 1/4 de segundo, y en ese tiempo se tiene que haber lanzado el thread de sobra

	 			if (!menu_multitarea) {
					contador_no_multitask+=MENU_CPU_CORE_LOOP_SLEEP_NO_MULTITASK;

					//Cuando se llega a 1/4 segundo ms
					if (contador_no_multitask>=intervalo*1000) {
						//printf ("Pasado medio segundo %d\n",contador_no_multitask);
						contador_no_multitask=0;
						pasado_cuarto_segundo=1;

						//printf ("refresca pantalla\n");
						menu_refresca_pantalla();	
				
					}
				}

                //acumulado=menu_da_todas_teclas();
				tecla=zxvision_read_keyboard();

				//con boton izquierdo no salimos
				if (tecla==13 && mouse_left) {	
					tecla=0;
				}				

				if (menu_window_splash_counter_ms-contador_antes>intervalo) pasado_cuarto_segundo=1;

                //Cada 224 ms
                if (pasado_cuarto_segundo) {
                	//trozos--;
                	contador_antes=menu_window_splash_counter_ms;
                	//printf ("dibujar franjas trozos: %d\n",trozos);
                	//llamar a la condicion
					if (funcioncond!=NULL) condicion=funcioncond(w);

					//llamar a funcion print
					if (funcionprint!=NULL) funcionprint(w);
						
                }
		

	} while (tecla==0 && !condicion);


}



void menu_espera_tecla(void)
{

        //Esperar a pulsar una tecla
        z80_byte acumulado;

	//Si al entrar aqui ya hay tecla pulsada, volver
        acumulado=menu_da_todas_teclas();
        if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA) return;


	do {
		menu_cpu_core_loop();


		acumulado=menu_da_todas_teclas();


	} while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA);

	//Al salir del bucle, reseteamos contadores de repeticion
	menu_reset_counters_tecla_repeticion();

}

void menu_espera_tecla_o_joystick(void)
{
        //Esperar a pulsar una tecla o joystick
        z80_byte acumulado;

        do {
                menu_cpu_core_loop();


                acumulado=menu_da_todas_teclas();

		//printf ("menu_espera_tecla_o_joystick acumulado: %d\n",acumulado);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && !realjoystick_hit() );

}




void menu_espera_no_tecla(void)
{

        //Esperar a liberar teclas. No ejecutar ni una instruccion cpu si la tecla esta liberada
	//con eso evitamos que cuando salte un breakpoint, que llama aqui, no se ejecute una instruccion y el registro PC apunte a la siguiente instruccion
        z80_byte acumulado;
	int salir=0;

        do {
		acumulado=menu_da_todas_teclas();
		if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) {
			salir=1;
		}

		else {
			menu_cpu_core_loop();
		}

	//printf ("menu_espera_no_tecla acumulado: %d\n",acumulado);

	} while (!salir);

}

//#define CONTADOR_HASTA_REPETICION (MACHINE_IS_Z88 ? 75 : 25)
//#define CONTADOR_ENTRE_REPETICION (MACHINE_IS_Z88 ? 3 : 1)

#define CONTADOR_HASTA_REPETICION (25)
#define CONTADOR_ENTRE_REPETICION (1)


//0.5 segundos para empezar repeticion (25 frames)
int menu_contador_teclas_repeticion;

int menu_segundo_contador_teclas_repeticion;


void menu_reset_counters_tecla_repeticion(void)
{
	//printf ("menu_reset_counters_tecla_repeticion\n");
                        menu_contador_teclas_repeticion=CONTADOR_HASTA_REPETICION;
                        menu_segundo_contador_teclas_repeticion=CONTADOR_ENTRE_REPETICION;
}

void menu_espera_no_tecla_con_repeticion(void)
{

        //Esperar a liberar teclas, pero si se deja pulsada una tecla el tiempo suficiente, se retorna
        z80_byte acumulado;

        //printf ("menu_espera_no_tecla_con_repeticion %d\n",menu_contador_teclas_repeticion);

	//x frames de segundo entre repeticion
	menu_segundo_contador_teclas_repeticion=CONTADOR_ENTRE_REPETICION;

        do {
                menu_cpu_core_loop();

                acumulado=menu_da_todas_teclas();

        	//printf ("menu_espera_no_tecla_con_repeticion acumulado: %d\n",acumulado);

		//si no hay tecla pulsada, restablecer contadores
		if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) == MENU_PUERTO_TECLADO_NINGUNA) menu_reset_counters_tecla_repeticion();

		//printf ("contadores: 1 %d  2 %d\n",menu_contador_teclas_repeticion,menu_segundo_contador_teclas_repeticion);

        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) != MENU_PUERTO_TECLADO_NINGUNA && menu_segundo_contador_teclas_repeticion!=0);


}



void menu_view_screen(MENU_ITEM_PARAMETERS)
{
        menu_espera_no_tecla();

	//para que no se vea oscuro
	menu_set_menu_abierto(0);
	//modificado_border.v=1;

	menu_cls_refresh_emulated_screen();

	//menu_refresca_pantalla();
        menu_espera_tecla();
	menu_set_menu_abierto(1);
	menu_espera_no_tecla();
	modificado_border.v=1;
}


//Quita de la linea los caracteres de atajo ~~ o ^^ o $$X
void menu_dibuja_menu_stdout_texto_sin_atajo(char *origen, char *destino)
{

	int indice_orig,indice_dest;

	indice_orig=indice_dest=0;

	while (origen[indice_orig]) {
		if (menu_escribe_texto_si_inverso(origen,indice_orig)) {
			indice_orig +=2;
		}

		if (menu_escribe_texto_si_parpadeo(origen,indice_orig)) {
			indice_orig +=2;
		}

		if (menu_escribe_texto_si_cambio_tinta(origen,indice_orig)) {
			indice_orig +=3;
		}


		destino[indice_dest++]=origen[indice_orig++];
	}

	destino[indice_dest]=0;

}





int menu_dibuja_menu_stdout(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{
	int linea_seleccionada=*opcion_inicial;
	char texto_linea_sin_shortcut[64];

	menu_item *aux;

	//Titulo
	printf ("\n");
        printf ("%s\n",titulo);
	printf ("------------------------\n\n");
	scrstdout_menu_print_speech_macro(titulo);

	aux=m;

        int max_opciones=0;

	int tecla=13;

	//para speech stdout. asumir no tecla pulsada. si se pulsa tecla, para leer menu
	menu_speech_tecla_pulsada=0;

        do {

		//scrstdout_menu_kbhit_macro();
                max_opciones++;

		if (aux->tipo_opcion!=MENU_OPCION_SEPARADOR) {

			//Ver si esta activa
                        t_menu_funcion_activo menu_funcion_activo;

                        menu_funcion_activo=aux->menu_funcion_activo;

                        if ( menu_funcion_activo!=NULL) {
				if (menu_funcion_activo()!=0) {
					printf ("%2d)",max_opciones);
					char buf[10];
					sprintf (buf,"%d",max_opciones);
					if (!menu_speech_tecla_pulsada) {
						scrstdout_menu_print_speech_macro(buf);
					}
				}

				else {
					printf ("x  ");
					if (!menu_speech_tecla_pulsada) {
						scrstdout_menu_print_speech_macro("Unavailable option: ");
					}
				}
			}

			else {
				printf ("%2d)",max_opciones);
					char buf[10];
					sprintf (buf,"%d",max_opciones);
					if (!menu_speech_tecla_pulsada) {
						scrstdout_menu_print_speech_macro(buf);
					}
			}

			//Imprimir linea menu pero descartando los ~~ de los atajo de teclado o ^^
			menu_dibuja_menu_stdout_texto_sin_atajo(aux->texto_opcion,texto_linea_sin_shortcut);


			printf ( "%s",texto_linea_sin_shortcut);
			if (!menu_speech_tecla_pulsada) {
				scrstdout_menu_print_speech_macro (texto_linea_sin_shortcut);
			}

		}


		printf ("\n");

                aux=aux->next;
        } while (aux!=NULL);

	printf ("\n");

	char texto_opcion[256];

	int salir_opcion;


	do {

		salir_opcion=1;
		printf("Option number? (prepend the option with h for help, t for tooltip). Write ESC to go back. ");
		//printf ("menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);
		if (!menu_speech_tecla_pulsada) {
			scrstdout_menu_print_speech_macro("Option number? (prepend the option with h for help, t for tooltip). Write ESC to go back. ");
		}

		//paso de curses a stdout deja stdout que no hace flush nunca. forzar
		fflush(stdout);
		scanf("%s",texto_opcion);

		if (!strcasecmp(texto_opcion,"ESC")) {
			tecla=MENU_RETORNO_ESC;
		}

		else if (texto_opcion[0]=='h' || texto_opcion[0]=='t') {
				salir_opcion=0;
                                char *texto_ayuda;
				linea_seleccionada=atoi(&texto_opcion[1]);
				linea_seleccionada--;
				if (linea_seleccionada>=0 && linea_seleccionada<max_opciones) {

					char *titulo_texto;

					if (texto_opcion[0]=='h') {
		                                texto_ayuda=menu_retorna_item(m,linea_seleccionada)->texto_ayuda;
						titulo_texto="Help";
					}

					else {
						texto_ayuda=menu_retorna_item(m,linea_seleccionada)->texto_tooltip;
						titulo_texto="Tooltip";
					}


        	                        if (texto_ayuda!=NULL) {
                	                        menu_generic_message(titulo_texto,texto_ayuda);
					}
					else {
						printf ("Item has no %s\n",titulo_texto);
					}
                                }
				else {
					printf ("Invalid option number\n");
				}
		}

		else {
			linea_seleccionada=atoi(texto_opcion);

			if (linea_seleccionada<1 || linea_seleccionada>max_opciones) {
				printf ("Incorrect option number\n");
				salir_opcion=0;
			}

			//Numero correcto
			else {
				linea_seleccionada--;

				//Ver si item no es separador
				menu_item *item_seleccionado;
				item_seleccionado=menu_retorna_item(m,linea_seleccionada);

				if (item_seleccionado->tipo_opcion==MENU_OPCION_SEPARADOR) {
					salir_opcion=0;
					printf ("Item is a separator\n");
				}

				else {


					//Ver si item esta activo
        	                	t_menu_funcion_activo menu_funcion_activo;
	        	                menu_funcion_activo=item_seleccionado->menu_funcion_activo;

	                	        if ( menu_funcion_activo!=NULL) {

        	                	        if (menu_funcion_activo()==0) {
							//opcion inactiva
							salir_opcion=0;
							printf ("Item is disabled\n");
						}
					}
				}
                        }



		}

	} while (salir_opcion==0);

        menu_item *menu_sel;
        menu_sel=menu_retorna_item(m,linea_seleccionada);

        item_seleccionado->menu_funcion=menu_sel->menu_funcion;
        item_seleccionado->tipo_opcion=menu_sel->tipo_opcion;
	item_seleccionado->valor_opcion=menu_sel->valor_opcion;
	
		strcpy(item_seleccionado->texto_opcion,menu_sel->texto_opcion);
		strcpy(item_seleccionado->texto_misc,menu_sel->texto_misc);


        //Liberar memoria del menu
        aux=m;
        menu_item *nextfree;

        do {
                //printf ("Liberando %x\n",aux);
                nextfree=aux->next;
                free(aux);
                aux=nextfree;
        } while (aux!=NULL);

	*opcion_inicial=linea_seleccionada;


	if (tecla==MENU_RETORNO_ESC) return MENU_RETORNO_ESC;

	return MENU_RETORNO_NORMAL;
}


//devuelve a que numero de opcion corresponde el atajo pulsado
//-1 si a ninguno
//NULL si a ninguno
int menu_retorna_atajo(menu_item *m,z80_byte tecla)
{

	//Si letra en mayusculas, bajar a minusculas
	if (tecla>='A' && tecla<='Z') tecla +=('a'-'A');

	int linea=0;

	while (m!=NULL) {
		if (m->atajo_tecla==tecla) {
			debug_printf (VERBOSE_DEBUG,"Shortcut found at entry number: %d",linea);
			return linea;
		}
		m=m->next;
		linea++;
	}

	//no encontrado atajo. escribir entradas de menu con atajos para informar al usuario
	menu_writing_inverse_color.v=1;
	return -1;

}

int menu_active_item_primera_vez=1;



void menu_escribe_opciones_zxvision(zxvision_window *ventana,menu_item *aux,int linea_seleccionada,int max_opciones)
{

        int i;

		//Opcion esta permitida seleccionarla (no esta en rojo)
        int opcion_activada;

		int menu_tabulado=0;
		if (aux->es_menu_tabulado) menu_tabulado=1;


		//Opcion de donde esta el cursor
		char texto_opcion_seleccionada[100];
		//Asumimos por si acaso que no hay ninguna activa
		texto_opcion_seleccionada[0]=0;
		//La opcion donde esta el cursor, si esta activada o no. Asumimos que si, por si acaso
		int opcion_seleccionada_activada=1;
		



        for (i=0;i<max_opciones;i++) {

			//si la opcion seleccionada es un separador, el cursor saltara a la siguiente
			//Nota: el separador no puede ser final de menu
			//if (linea_seleccionada==i && aux->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada++;

			t_menu_funcion_activo menu_funcion_activo;

			menu_funcion_activo=aux->menu_funcion_activo;

			if (menu_funcion_activo!=NULL) {
				opcion_activada=menu_funcion_activo();
			}

			else {
				opcion_activada=1;
			}

			//Al listar opciones de menu, decir si la opcion está desabilitada
			if (!opcion_activada) menu_textspeech_send_text("Unavailable option: ");

			//Cuando haya opcion_activa, nos la apuntamos para decirla al final en speech.
			//Y si es la primera vez en ese menu, dice "Selected item". Sino, solo dice el nombre de la opcion
			if (linea_seleccionada==i) {
				if (menu_active_item_primera_vez) {
					sprintf (texto_opcion_seleccionada,"Selected item: %s",aux->texto_opcion);
					menu_active_item_primera_vez=0;
				}

				else {
					sprintf (texto_opcion_seleccionada,"%s",aux->texto_opcion);
				}

				opcion_seleccionada_activada=opcion_activada;
			}

			if (menu_tabulado) {
				menu_escribe_linea_opcion_tabulado_zxvision(ventana,i,linea_seleccionada,opcion_activada,aux->texto_opcion,aux->menu_tabulado_x,aux->menu_tabulado_y);
			}
            
			
			else {
				int y_destino=i;
				int linea_seleccionada_destino=linea_seleccionada;

				if (y_destino>=0) {
				
						menu_escribe_linea_opcion_zxvision(ventana,y_destino,linea_seleccionada_destino,opcion_activada,aux->texto_opcion);
					
				}
				
		
			}
            
			
			aux=aux->next;

        }



		if (texto_opcion_seleccionada[0]!=0) {
			//Selected item siempre quiero que se escuche

			//Guardamos estado actual
			int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
			menu_speech_tecla_pulsada=0;


			//Al decir la linea seleccionada de menu, decir si la opcion está desabilitada
			if (!opcion_seleccionada_activada) menu_textspeech_send_text("Unavailable option: ");

			menu_textspeech_send_text(texto_opcion_seleccionada);



			//Restauro estado
			//Pero si se ha pulsado tecla, no restaurar estado
			//Esto sino provocaria que , por ejemplo, en la ventana de confirmar yes/no,
			//se entra con menu_speech_tecla_pulsada=0, se pulsa tecla mientras se esta leyendo el item activo,
			//y luego al salir de aqui, se pierde el valor que se habia metido (1) y se vuelve a poner el 0 del principio
			//provocando que cada vez que se mueve el cursor, se relea la ventana entera
			if (menu_speech_tecla_pulsada==0) menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
		}
}



int menu_dibuja_menu_cursor_arriba(int linea_seleccionada,int max_opciones,menu_item *m)
{
	if (linea_seleccionada==0) linea_seleccionada=max_opciones-1;
	else {
		linea_seleccionada--;
		//ver si la linea seleccionada es un separador
		int salir=0;
		while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR && !salir) {
			linea_seleccionada--;
			//si primera linea es separador, nos iremos a -1
			if (linea_seleccionada==-1) {
				linea_seleccionada=max_opciones-1;
				salir=1;
			}
		}
	}
	//si linea resultante es separador, decrementar
	while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada--;

	//Decir que se ha pulsado tecla
	menu_speech_tecla_pulsada=1;

	return linea_seleccionada;
}

int menu_dibuja_menu_cursor_abajo(int linea_seleccionada,int max_opciones,menu_item *m)
{
	if (linea_seleccionada==max_opciones-1) linea_seleccionada=0;
	else {
		linea_seleccionada++;
		//ver si la linea seleccionada es un separador
		int salir=0;
		while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR && !salir) {
			linea_seleccionada++;
			//si ultima linea es separador, nos salimos de rango
			if (linea_seleccionada==max_opciones) {
				linea_seleccionada=0;
				salir=0;
			}
		}
	}
	//si linea resultante es separador, incrementar
	while (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR) linea_seleccionada++;

	//Decir que se ha pulsado tecla
	menu_speech_tecla_pulsada=1;

	return linea_seleccionada;
}


int menu_dibuja_menu_cursor_abajo_tabulado(int linea_seleccionada,int max_opciones,menu_item *m)
{

	if (linea_seleccionada==max_opciones-1) linea_seleccionada=0;

		else {

		//Ubicarnos primero en el item de menu seleccionado
		menu_item *m_aux=menu_retorna_item(m,linea_seleccionada);

		//Su coordenada y original
		int orig_tabulado_y=m_aux->menu_tabulado_y;
		int orig_tabulado_x=m_aux->menu_tabulado_x;


		//Y vamos hacia abajo hasta que coordenada y sea diferente
		do {
			//printf ("antes vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
			linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
			m_aux=menu_retorna_item(m,linea_seleccionada);
			//printf ("despues vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
		} while (m_aux->menu_tabulado_y==orig_tabulado_y);

		int posible_posicion=linea_seleccionada;
		int final_y=m_aux->menu_tabulado_y;

		//Y ahora buscar la que tenga misma coordenada x o mas a la derecha, si la hubiera
		while (m_aux->menu_tabulado_y==final_y && m_aux->menu_tabulado_x<orig_tabulado_x) {
			posible_posicion=linea_seleccionada;
			//printf ("antes horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
			linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
			m_aux=menu_retorna_item(m,linea_seleccionada);
			//printf ("despues horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
		};

		//Si no estamos en misma posicion y, volver a posicion
		if (m_aux->menu_tabulado_y!=final_y) linea_seleccionada=posible_posicion;
	}

	//Decir que se ha pulsado tecla
	menu_speech_tecla_pulsada=1;

	return linea_seleccionada;
}

int menu_dibuja_menu_cursor_arriba_tabulado(int linea_seleccionada,int max_opciones,menu_item *m)
{

	if (linea_seleccionada==0) linea_seleccionada=max_opciones-1;

	else {

		//Ubicarnos primero en el item de menu seleccionado
		menu_item *m_aux=menu_retorna_item(m,linea_seleccionada);

		//Su coordenada y original
		int orig_tabulado_y=m_aux->menu_tabulado_y;
		int orig_tabulado_x=m_aux->menu_tabulado_x;


		//Y vamos hacia arriba hasta que coordenada y sea diferente
		do {
			//printf ("antes vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
			linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
			m_aux=menu_retorna_item(m,linea_seleccionada);
			//printf ("despues vert orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
		} while (m_aux->menu_tabulado_y==orig_tabulado_y);

		int posible_posicion=linea_seleccionada;
		int final_y=m_aux->menu_tabulado_y;

		//Y ahora buscar la que tenga misma coordenada x o mas a la derecha, si la hubiera
		while (m_aux->menu_tabulado_y==final_y && m_aux->menu_tabulado_x>orig_tabulado_x) {
			posible_posicion=linea_seleccionada;
			//printf ("antes horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
			linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
			m_aux=menu_retorna_item(m,linea_seleccionada);
			//printf ("despues horiz orig y: %d y: %d linea_seleccionada: %d texto: %s\n",orig_tabulado_y,m_aux->menu_tabulado_y,linea_seleccionada,m_aux->texto_opcion);
		};

		//Si no estamos en misma posicion y, volver a posicion
		if (m_aux->menu_tabulado_y!=final_y) linea_seleccionada=posible_posicion;

	}

	//Decir que se ha pulsado tecla
	menu_speech_tecla_pulsada=1;

	return linea_seleccionada;
}


int menu_dibuja_menu_cursor_abajo_common(int linea_seleccionada,int max_opciones,menu_item *m)
{
	//Mover abajo
			
	if (m->es_menu_tabulado==0) linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
	else linea_seleccionada=menu_dibuja_menu_cursor_abajo_tabulado(linea_seleccionada,max_opciones,m);
			
	return linea_seleccionada;
}


int menu_dibuja_menu_cursor_arriba_common(int linea_seleccionada,int max_opciones,menu_item *m)
{
	//Mover arriba
			
	if (m->es_menu_tabulado==0) linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
	else linea_seleccionada=menu_dibuja_menu_cursor_arriba_tabulado(linea_seleccionada,max_opciones,m);

	return linea_seleccionada;

}



void menu_dibuja_menu_help_tooltip(char *texto, int si_tooltip)
{



                                        //Guardar funcion de texto overlay activo, para menus como el de visual memory por ejemplo, para desactivar temporalmente
                                        void (*previous_function)(void);

                                        previous_function=menu_overlay_function;

                                       //restauramos modo normal de texto de menu
                                       set_menu_overlay_function(normal_overlay_texto_menu);



        if (si_tooltip) {
			//menu_generic_message_tooltip("Tooltip",0,1,0,NULL,"%s",texto);
			//printf ("justo antes de message tooltip\n");
			zxvision_generic_message_tooltip("Tooltip" , 0 ,0,1,0,NULL,0,"%s",texto);
		}
	
		else menu_generic_message("Help",texto);

        //Restauramos funcion anterior de overlay
        set_menu_overlay_function(previous_function);

		if (zxvision_current_window!=NULL) {
			zxvision_draw_window(zxvision_current_window);
			//printf ("antes draw windows contents\n");
			zxvision_draw_window_contents(zxvision_current_window);
		}

		//printf ("antes refrescar pantalla\n");
        menu_refresca_pantalla();

		

}


//Indica que el menu permite repeticiones de teclas. Solo valido al pulsar hotkeys
int menu_dibuja_menu_permite_repeticiones_hotk=0;

void menu_dibuja_menu_espera_no_tecla(void)
{
	if (menu_dibuja_menu_permite_repeticiones_hotk) menu_espera_no_tecla_con_repeticion();
	else menu_espera_no_tecla();
}

int menu_calcular_ancho_string_item(char *texto)
{
	//Devuelve longitud de texto teniendo en cuenta de no sumar caracteres ~~ o ^^ o $$X
	unsigned int l;
	int ancho_calculado=strlen(texto);

	for (l=0;l<strlen(texto);l++) {
			if (menu_escribe_texto_si_inverso(texto,l)) ancho_calculado-=2;
			if (menu_escribe_texto_si_parpadeo(texto,l)) ancho_calculado-=2;
			if (menu_escribe_texto_si_cambio_tinta(texto,l)) ancho_calculado-=3;
	}

	return ancho_calculado;
}

//Decir si usamos hasta la ultima columna, pues no se muestra barra scroll,
//o bien se muestra barra scroll y no usamos hasta ultima columna
//Si hemos cambiado la ventana, retornar no 0
int menu_dibuja_menu_adjust_last_column(zxvision_window *w,int ancho,int alto)
{
			//Si no hay barra scroll vertical, usamos hasta la ultima columna
		int incremento_por_columna=0;
		//printf ("visible height: %d ancho %d alto %d\n",w->visible_height,ancho,alto);
		if (w->visible_height>=alto) {
			incremento_por_columna=1;
		}							

		if (incremento_por_columna) {
			if (w->can_use_all_width==0) {
				//printf ("Usamos hasta la ultima columna\n");
				w->can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll
				w->total_width=ancho-1+1;
				return 1;
			}
		}
		else {
			if (w->can_use_all_width) {
				//printf ("NO usamos hasta la ultima columna\n");
				w->can_use_all_width=0; 
				w->total_width=ancho-1;
				return 1;
			}
		}

		/*printf ("total width: %d ancho: %d\n",ventana_menu.total_width,ancho);
		ventana_menu.total_width=10;
		printf ("total width: %d ancho: %d\n",ventana_menu.total_width,ancho);*/
		//ventana_menu.total_width=
		//printf ("total width: %d visible width %d\n",ventana_menu.total_width,ventana_menu.visible_width);
		//ventana_menu.can_use_all_width=1;  //Esto falla porque en algun momento despues se pierde este parametro

	return 0;

}

z80_int menu_mouse_frame_counter=0;
z80_int menu_mouse_frame_counter_anterior=0;

//Funcion de gestion de menu
//Entrada: opcion_inicial: puntero a opcion inicial seleccionada
//m: estructura de menu (estructura en forma de lista con punteros)
//titulo: titulo de la ventana del menu
//Nota: x,y, ancho, alto de la ventana se calculan segun el contenido de la misma

//Retorno
//valor retorno: tecla pulsada: 0 normal (ENTER), 1 ESCAPE,... MENU_RETORNO_XXXX

//opcion_inicial contiene la opcion seleccionada.
//asigna en item_seleccionado valores de: tipo_opcion, menu_funcion (debe ser una estructura ya asignada)

 

int menu_dibuja_menu(int *opcion_inicial,menu_item *item_seleccionado,menu_item *m,char *titulo)
{


	//no escribir letras de atajos de teclado al entrar en un menu
	menu_writing_inverse_color.v=0;

	//Si se fuerza siempre que aparezcan letras de atajos
	if (menu_force_writing_inverse_color.v) menu_writing_inverse_color.v=1;

	//Primera vez decir selected item. Luego solo el nombre del item
	menu_active_item_primera_vez=1;

        if (!strcmp(scr_driver_name,"stdout") ) {
                //se abre menu con driver stdout. Llamar a menu alternativo

		//si hay menu tabulado, agregamos ESC (pues no se incluye nunca)
		if (m->es_menu_tabulado) menu_add_ESC_item(m);

                return menu_dibuja_menu_stdout(opcion_inicial,item_seleccionado,m,titulo);
        }
/*
        if (if_pending_error_message) {
                if_pending_error_message=0;
                menu_generic_message("ERROR",pending_error_message);
        }
*/


	//esto lo haremos ligeramente despues menu_speech_tecla_pulsada=0;

	if (!menu_dibuja_menu_permite_repeticiones_hotk) {
		//printf ("llamar a menu_reset_counters_tecla_repeticion desde menu_dibuja_menu al inicio\n");
		menu_reset_counters_tecla_repeticion();
	}


	//nota: parece que scr_actualiza_tablas_teclado se debe llamar en el caso de xwindows para que refresque la pantalla->seguramente viene por un evento


	int max_opciones;
	int linea_seleccionada=*opcion_inicial;

	//si la anterior opcion era la final (ESC), establecemos el cursor a 0
	//if (linea_seleccionada<0) linea_seleccionada=0;

	int x,y,ancho,alto;

	menu_item *aux;

	aux=m;

	//contar el numero de opciones totales
	//calcular ancho maximo de la ventana
	int ancho_calculado=0;

	//Para permitir menus mas grandes verticalmente de lo que cabe en ventana.
	//int scroll_opciones=0;


	ancho=menu_dibuja_ventana_ret_ancho_titulo(ZXVISION_MAX_ANCHO_VENTANA,titulo);






	max_opciones=0;
	do {
		


		ancho_calculado=menu_calcular_ancho_string_item(aux->texto_opcion)+2; //+2 espacios


		if (ancho_calculado>ancho) ancho=ancho_calculado;
		//printf ("%s\n",aux->texto);
		aux=aux->next;
		max_opciones++;
	} while (aux!=NULL);

	//printf ("Opciones totales: %d\n",max_opciones);

	alto=max_opciones+2;

	x=menu_center_x()-ancho/2;
	y=menu_center_y()-alto/2;

	int ancho_visible=ancho;
	int alto_visible=alto;

	if (x<0 || y<0 || x+ancho>scr_get_menu_width() || y+alto>scr_get_menu_height()) {
		//char window_error_message[100];
		//sprintf(window_error_message,"Window out of bounds: x: %d y: %d ancho: %d alto: %d",x,y,ancho,alto);
		//cpu_panic(window_error_message);

		//Ajustar limites
		if (x<0) x=0;
		if (y<0) y=0;
		if (x+ancho>scr_get_menu_width()) ancho_visible=scr_get_menu_width()-x;
		if (y+alto>scr_get_menu_height()) alto_visible=scr_get_menu_height()-y;
	}

	int redibuja_ventana;
	int tecla;

	//Apuntamos a ventana usada. Si no es menu tabulado, creamos una nosotros
	//Si es tabulado, usamos current_window (pues ya alguien la ha creado antes)
	zxvision_window *ventana;
	zxvision_window ventana_menu;



	if (m->es_menu_tabulado==0) {
		



		//zxvision_new_window(&ventana_menu,x,y,ancho_visible,alto_visible,
		//					ancho-1,alto-2,titulo);		 //hacer de momento igual de ancho que ancho visible para poder usar ultima columna


		//Hacer 1 mas de ancho total para poder usar columna derecha
		zxvision_new_window(&ventana_menu,x,y,ancho_visible,alto_visible,
							ancho-1+1,alto-2,titulo);		 //hacer de momento igual de ancho que ancho visible para poder usar ultima columna


		//Si no hay barra scroll vertical, usamos hasta la ultima columna
		menu_dibuja_menu_adjust_last_column(&ventana_menu,ancho,alto);



		ventana=&ventana_menu;

	}

	else {
		ventana=zxvision_current_window;
	}


	zxvision_draw_window(ventana);	

	

	//Entrar aqui cada vez que se dibuje otra subventana aparte, como tooltip o ayuda
	do {
		redibuja_ventana=0;

		//printf ("Entrada desde subventana aparte, como tooltip o ayuda\n");


		menu_tooltip_counter=0;


		tecla=0;

        //si la opcion seleccionada es mayor que el total de opciones, seleccionamos linea 0
        //esto pasa por ejemplo cuando activamos realvideo, dejamos el cursor por debajo, y cambiamos a zxspectrum
        //printf ("linea %d max %d\n",linea_seleccionada,max_opciones);
        if (linea_seleccionada>=max_opciones) {
                debug_printf(VERBOSE_INFO,"Selected Option beyond limits. Set option to 0");
                linea_seleccionada=0;
        }


	//menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR
	//si opcion activa es un separador (que esto pasa por ejemplo cuando activamos realvideo, dejamos el cursor por debajo, y cambiamos a zxspectrum)
	//en ese caso, seleccionamos linea 0
	if (menu_retorna_item(m,linea_seleccionada)->tipo_opcion==MENU_OPCION_SEPARADOR) {
		debug_printf(VERBOSE_INFO,"Selected Option is a separator. Set option to 0");
		linea_seleccionada=0;
	}


	while (tecla!=13 && tecla!=32 && tecla!=MENU_RETORNO_ESC && tecla!=MENU_RETORNO_F1 && tecla!=MENU_RETORNO_F2 && tecla!=MENU_RETORNO_F10 && redibuja_ventana==0 && menu_tooltip_counter<TOOLTIP_SECONDS) {

		//printf ("tecla desde bucle: %d\n",tecla);
		//Ajustar scroll
		//scroll_opciones=0;


		//desactivado en zxvision , tiene su propio scroll
	


		//Si menu tabulado, ajustamos scroll de zxvision
		if (m->es_menu_tabulado) {
			int linea_cursor=menu_retorna_item(m,linea_seleccionada)->menu_tabulado_y;
			//printf ("ajustar scroll a %d\n",linea_cursor);
			zxvision_set_offset_y_visible(ventana,linea_cursor);
		}

		else {
			zxvision_set_offset_y_visible(ventana,linea_seleccionada);
		}





		//escribir todas opciones
		//printf ("Escribiendo de nuevo las opciones\n");
		menu_escribe_opciones_zxvision(ventana,m,linea_seleccionada,max_opciones);


	
		//printf ("Linea seleccionada: %d\n",linea_seleccionada);
		//No queremos que el speech vuelva a leer la ventana
		//menu_speech_tecla_pulsada=1;
		zxvision_draw_window_contents_no_speech(ventana);


        menu_refresca_pantalla();

		tecla=0;

		//la inicializamos a 0. aunque parece que no haga falta, podria ser que el bucle siguiente
		//no se entrase (porque menu_tooltip_counter<TOOLTIP_SECONDS) y entonces tecla_leida tendria valor indefinido
		int tecla_leida=0;


		//Si se estaba escuchando speech y se pulsa una tecla, esa tecla debe entrar aqui tal cual y por tanto, no hacemos espera_no_tecla
		//temp menu_espera_no_tecla();
		if (menu_speech_tecla_pulsada==0) {
			//menu_espera_no_tecla();
			menu_dibuja_menu_espera_no_tecla();
		}
		menu_speech_tecla_pulsada=0;

		while (tecla==0 && redibuja_ventana==0 && menu_tooltip_counter<TOOLTIP_SECONDS) {


			//Si no hay barra scroll vertical, usamos hasta la ultima columna, solo para menus no tabulados
			if (m->es_menu_tabulado==0) {
				if (menu_dibuja_menu_adjust_last_column(ventana,ancho,alto)) {
					//printf ("Redibujar ventana pues hay cambio en columna final de scroll\n");

					//Es conveniente llamar antes a zxvision_draw_window pues este establece parametros de ventana_ancho y alto,
					//que se leen luego en menu_escribe_opciones_zxvision
					//sin embargo, al llamar a menu_escribe_opciones_zxvision, el cursor sigue apareciendo como mas pequeño hasta que
					//no se pulsa tecla
					//printf ("ventana ancho antes: %d\n",ventana_ancho);
					zxvision_draw_window(ventana);
					//printf ("ventana ancho despues: %d\n",ventana_ancho);

					//borrar contenido ventana despues de redimensionarla con espacios
					int i;
					for (i=0;i<ventana->total_height;i++) zxvision_print_string_defaults_fillspc(ventana,0,i,"");

					menu_escribe_opciones_zxvision(ventana,m,linea_seleccionada,max_opciones);
					
					zxvision_draw_window_contents(ventana);
				}
			}

			//Si no hubera este menu_refresca_pantalla cuando multitask esta a off,
			//no se moverian las ventanas con refresco al mover raton
			//el resto de cosas funcionaria bien
             if (!menu_multitarea) {
                        menu_refresca_pantalla();
                }


			menu_espera_tecla_timeout_tooltip();

			//Guardamos valor de mouse_movido pues se perdera el valor al leer el teclado de nuevo
			int antes_mouse_movido=mouse_movido;

			tecla_leida=zxvision_read_keyboard();

			//printf ("Despues tecla leida: %d\n",tecla_leida);

			mouse_movido=antes_mouse_movido;

			//Para poder usar repeticiones
			if (tecla_leida==0) {
				//printf ("llamar a menu_reset_counters_tecla_repeticion desde menu_dibuja_menu cuando tecla=0\n");
				menu_reset_counters_tecla_repeticion();
			}

			else {
				//printf ("no reset counter tecla %d\n",tecla);
			}




		
			//printf ("mouse_movido: %d\n",mouse_movido);


			//printf ("tecla_leida: %d\n",tecla_leida);
			if (mouse_movido) {
				//printf ("mouse x: %d y: %d menu mouse x: %d y: %d\n",mouse_x,mouse_y,menu_mouse_x,menu_mouse_y);
				//printf ("ventana x %d y %d ancho %d alto %d\n",ventana_x,ventana_y,ventana_ancho,ventana_alto);
				if (si_menu_mouse_en_ventana() ) {
				//if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) {
					//printf ("dentro ventana\n");
					//Descartar linea titulo y ultima linea

					if (menu_mouse_y>0 && menu_mouse_y<current_win_alto-1) {
						//printf ("dentro espacio efectivo ventana\n");
						//Ver si hay que subir o bajar cursor
						int posicion_raton_y=menu_mouse_y-1;

						//tener en cuenta scroll
						posicion_raton_y +=ventana->offset_y;

						//Si no se selecciona separador. Menu no tabulado
						if (m->es_menu_tabulado==0) {
							if (menu_retorna_item(m,posicion_raton_y)->tipo_opcion!=MENU_OPCION_SEPARADOR) {
								linea_seleccionada=posicion_raton_y;
								redibuja_ventana=1;
								menu_tooltip_counter=0;
							}
						}
						else {
							menu_item *buscar_tabulado;
							int linea_buscada;
							int posicion_raton_x=menu_mouse_x;
							buscar_tabulado=menu_retorna_item_tabulado_xy(m,posicion_raton_x,posicion_raton_y,&linea_buscada);

							if (buscar_tabulado!=NULL) {
								//Buscar por coincidencia de coordenada x,y
								if (buscar_tabulado->tipo_opcion!=MENU_OPCION_SEPARADOR) {
									linea_seleccionada=linea_buscada;
									redibuja_ventana=1;
									menu_tooltip_counter=0;
								}
							}
							else {
								//printf ("item no encontrado\n");
							}
						}

					}
					//else {
					//	printf ("En espacio ventana no usable\n");
					//}
				}
				//else {
				//	printf ("fuera ventana\n");
				//}
			}

			//mouse boton izquierdo es como enter
			int mouse_en_zona_opciones=1;

			//if (menu_mouse_x>=0 && menu_mouse_y>=0 && menu_mouse_x<ventana_ancho && menu_mouse_y<ventana_alto ) return 1;

			//Mouse en columna ultima de la derecha,
			//o mouse en primera linea
			//o mouse en ultima linea
			// no enviamos enter si pulsamos boton
			if (menu_mouse_x==current_win_ancho-1 || menu_mouse_y==0 || menu_mouse_y==current_win_alto-1) mouse_en_zona_opciones=0;

			//printf ("Despues tecla leida2: %d\n",tecla_leida);

			if (si_menu_mouse_en_ventana() && mouse_left && mouse_en_zona_opciones && !mouse_is_dragging) {
				//printf ("Enviamos enter\n");
				tecla=13;
			}					


			else if (tecla_leida==11) tecla='7';
			else if (tecla_leida==10) tecla='6';
			else if (tecla_leida==13) tecla=13;
			else if (tecla_leida==24) tecla=24;
			else if (tecla_leida==25) tecla=25;


			//Teclas para menus tabulados
			else if (tecla_leida==8) tecla='5';	
			else if (tecla_leida==9) tecla='8';	

			else if (tecla_leida==2) {
				//tecla=2; //ESC que viene de cerrar ventana al pulsar con raton boton de cerrar en titulo
				tecla=MENU_RETORNO_ESC;
				//printf ("tecla final es ESC\n");
			}


			else if ((puerto_especial1 & 1)==0) {
				//Enter
				//printf ("Leido ESC\n");
				tecla=MENU_RETORNO_ESC;
			}



			//En principio ya no volvemos mas con F1, dado que este se usa para ayuda contextual de cada funcion

			//F1 (ayuda) o h en drivers que no soportan F
            else if ((puerto_especial2 & 1)==0 || (tecla_leida=='h' && f_functions==0) ) {
                                //F1
				char *texto_ayuda;
				texto_ayuda=menu_retorna_item(m,linea_seleccionada)->texto_ayuda;
				if (texto_ayuda!=NULL) {
					//Forzar que siempre suene
					//Esperamos antes a liberar tecla, sino lo que hara sera que esa misma tecla F1 cancelara el speech texto de ayuda
					menu_espera_no_tecla();
					menu_speech_tecla_pulsada=0;


					menu_dibuja_menu_help_tooltip(texto_ayuda,0);


					redibuja_ventana=1;
					menu_tooltip_counter=0;
					//Y volver a decir "selected item"
					menu_active_item_primera_vez=1;

				}
                        }


                        else if ((puerto_especial2 & 2)==0) {
                                //F2
                                tecla=MENU_RETORNO_F2;
                        }

                        else if ((puerto_especial3 & 16)==0) {
                                //F10
                                tecla=MENU_RETORNO_F10;
                        }


			//teclas de atajos. De momento solo admitido entre a y z
			else if ( (tecla_leida>='a' && tecla_leida<='z') || (tecla_leida>='A' && tecla_leida<='Z')) {
				debug_printf (VERBOSE_DEBUG,"Read key: %c. Possibly shortcut",tecla_leida);
				tecla=tecla_leida;
			}

			//tecla espacio. acciones adicionales. Ejemplo en breakpoints para desactivar
			else if (tecla_leida==32) {
				debug_printf (VERBOSE_DEBUG,"Pressed key space");
				tecla=32;
            }

				


			else {
				//printf ("Final ponemos tecla a 0. Era %d\n",tecla);
				tecla=0;
			}


			//printf ("menu tecla: %d\n",tecla);
		}

		//Si no se ha pulsado tecla de atajo:
		if (!((tecla_leida>='a' && tecla_leida<='z') || (tecla_leida>='A' && tecla_leida<='Z')) ) {
			menu_espera_no_tecla();

		

		}



        t_menu_funcion_activo sel_activo;

		t_menu_funcion funcion_espacio;

		if (tecla!=0) menu_tooltip_counter=0;

		int lineas_mover_pgup_dn;
		int conta_mover_pgup_dn;

		//printf ("tecla: %d\n",tecla);

		switch (tecla) {
			case 13:
				//ver si la opcion seleccionada esta activa

				sel_activo=menu_retorna_item(m,linea_seleccionada)->menu_funcion_activo;

				if (sel_activo!=NULL) {
		                	if ( sel_activo()==0 ) tecla=0;  //desactivamos seleccion
				}
                        break;


			//Mover Izquierda, solo en tabulados
            case '5':
            	//en menus tabulados, misma funcion que arriba para un no tabulado
                if (m->es_menu_tabulado==0) break;

                //Si es tabulado, seguira hasta la opcion '7'
				linea_seleccionada=menu_dibuja_menu_cursor_arriba(linea_seleccionada,max_opciones,m);
			break; 


			//Mover Derecha, solo en tabulados
			case '8':
				//en menus tabulados, misma funcion que abajo para un no tabulado
				if (m->es_menu_tabulado==0) break;

				linea_seleccionada=menu_dibuja_menu_cursor_abajo(linea_seleccionada,max_opciones,m);
			break;

			//Mover abajo
			case '6':
				linea_seleccionada=menu_dibuja_menu_cursor_abajo_common(linea_seleccionada,max_opciones,m);
			break;

			//Mover arriba
			case '7':
				linea_seleccionada=menu_dibuja_menu_cursor_arriba_common(linea_seleccionada,max_opciones,m);
			break;			

			//PgUp
			case 24:
				lineas_mover_pgup_dn=ventana->visible_height-3;
				//Ver si al limite de arriba
				if (linea_seleccionada-lineas_mover_pgup_dn<0) {
					lineas_mover_pgup_dn=linea_seleccionada-1; //el -1 final es por tener en cuenta el separador de siempre
				}

				//TODO esto movera el cursor tantas lineas como lineas visibles tiene el menu,
				//si hay algun item como separador, se lo saltara, moviendo el cursor mas lineas de lo deseado
				//printf ("lineas mover: %d\n",lineas_mover_pgup_dn);
				for (conta_mover_pgup_dn=0;conta_mover_pgup_dn<lineas_mover_pgup_dn;conta_mover_pgup_dn++) linea_seleccionada=menu_dibuja_menu_cursor_arriba_common(linea_seleccionada,max_opciones,m);
				
			break;

			//PgUp
			case 25:
				lineas_mover_pgup_dn=ventana->visible_height-3;
				//Ver si al limite de abajo
				if (linea_seleccionada+lineas_mover_pgup_dn>=max_opciones) {
					lineas_mover_pgup_dn=max_opciones-linea_seleccionada-1-1; //el -1 final es por tener en cuenta el separador de siempre
				}

				//TODO esto movera el cursor tantas lineas como lineas visibles tiene el menu,
				//si hay algun item como separador, se lo saltara, moviendo el cursor mas lineas de lo deseado
				//printf ("lineas mover: %d\n",lineas_mover_pgup_dn);
				//int i;
				for (conta_mover_pgup_dn=0;conta_mover_pgup_dn<lineas_mover_pgup_dn;conta_mover_pgup_dn++) linea_seleccionada=menu_dibuja_menu_cursor_abajo_common(linea_seleccionada,max_opciones,m);
				
			break;



			case 32:
				//Accion para tecla espacio
				//printf ("Pulsado espacio\n");
                                //decimos que se ha pulsado Enter
                                //tecla=13;

				//Ver si tecla asociada a espacio
				funcion_espacio=menu_retorna_item(m,linea_seleccionada)->menu_funcion_espacio;

				if (funcion_espacio==NULL) {
					debug_printf (VERBOSE_DEBUG,"No space key function associated to this menu item");
					tecla=0;
				}

				else {

					debug_printf (VERBOSE_DEBUG,"Found space key function associated to this menu item");

	                                //ver si la opcion seleccionada esta activa

        	                        sel_activo=menu_retorna_item(m,linea_seleccionada)->menu_funcion_activo;

                	                if (sel_activo!=NULL) {
                        	                if ( sel_activo()==0 ) {
							tecla=0;  //desactivamos seleccion
							debug_printf (VERBOSE_DEBUG,"Menu item is disabled");
						}
                                	}

				}

			break;



		}

		//teclas de atajos. De momento solo admitido entre a y z
		if ( (tecla>='a' && tecla<='z') || (tecla>='A' && tecla<='Z')) {
			//printf ("buscamos atajo\n");

			int entrada_atajo;
			entrada_atajo=menu_retorna_atajo(m,tecla);


			//Encontrado atajo
			if (entrada_atajo!=-1) {
				linea_seleccionada=entrada_atajo;

				//Mostrar por un momento opciones y letras
				menu_writing_inverse_color.v=1;
				menu_escribe_opciones_zxvision(ventana,m,entrada_atajo,max_opciones);
				menu_refresca_pantalla();
				//menu_espera_no_tecla();
				menu_dibuja_menu_espera_no_tecla();

				//decimos que se ha pulsado Enter
				tecla=13;

	                        //Ver si esa opcion esta habilitada o no
        	                t_menu_funcion_activo sel_activo;
                	        sel_activo=menu_retorna_item(m,linea_seleccionada)->menu_funcion_activo;
                        	if (sel_activo!=NULL) {
	                                //opcion no habilitada
        	                        if ( sel_activo()==0 ) {
                	                        debug_printf (VERBOSE_DEBUG,"Shortcut found at entry number %d but entry disabled",linea_seleccionada);
						tecla=0;
                                	}
	                        }


			}

			else {
				debug_printf (VERBOSE_DEBUG,"No shortcut found for read key: %c",tecla);
				tecla=0;
				menu_espera_no_tecla();
			}
		}



	}

	//NOTA: contador de tooltip se incrementa desde bucle de timer, ejecutado desde cpu loop
	//Si no hay multitask de menu, NO se incrementa contador y por tanto no hay tooltip

	if (menu_tooltip_counter>=TOOLTIP_SECONDS) {

        redibuja_ventana=1;

		//Por defecto asumimos que no saltara tooltip y por tanto que no queremos que vuelva a enviar a speech la ventana
		//Aunque si que volvera a decir el "Selected item: ..." en casos que se este en una opcion sin tooltip,
		//no aparecera el tooltip pero vendra aqui con el timeout y esto hara redibujar la ventana por redibuja_ventana=1
		//si quitase ese redibujado, lo que pasaria es que no aparecerian los atajos de teclado para cada opcion
		//Entonces tal y como esta ahora:
		//Si la opcion seleccionada tiene tooltip, salta el tooltip
		//Si no tiene tooltip, no salta tooltip, pero vuelve a decir "Selected item: ..."
		menu_speech_tecla_pulsada=1;

		//Si ventana no esta activa, no mostrar tooltips,
		//porque esto hace que, por ejemplo, si el foco está en la máquina emulada, al saltar el tooltip, cambiaria el foco a la ventana de menu
		if (tooltip_enabled.v && ventana_tipo_activa) {
			char *texto_tooltip;
			texto_tooltip=menu_retorna_item(m,linea_seleccionada)->texto_tooltip;
			if (texto_tooltip!=NULL) {
				//printf ("mostramos tooltip\n");
				//Forzar que siempre suene
				menu_speech_tecla_pulsada=0;


				menu_dibuja_menu_help_tooltip(texto_tooltip,1);

				//printf ("despues de mostrar tooltip\n");


				//Esperar no tecla
				menu_espera_no_tecla();


				//Y volver a decir "Selected item"
				menu_active_item_primera_vez=1;


				//Y reactivar parametros ventana usados en menu_dibuja_ventana
				//zxvision_set_draw_window_parameters(ventana);

	        }

			else {
				//printf ("no hay tooltip\n");

				//No queremos que se vuelva a leer cuando tooltip es inexistente. si no, estaria todo el rato releyendo la linea
				//TODO: esto no tiene efecto, sigue releyendo cuando estas sobre item que no tiene tooltip
				//menu_speech_tecla_pulsada=1;	
	
			}

		}

		//else printf ("No mostrar tooltip\n");

		//Hay que dibujar las letras correspondientes en texto inverso
		menu_writing_inverse_color.v=1;

		menu_tooltip_counter=0;
	}

	} while (redibuja_ventana==1);

	*opcion_inicial=linea_seleccionada;

	//nos apuntamos valor de retorno

	menu_item *menu_sel;
	menu_sel=menu_retorna_item(m,linea_seleccionada);

	//Si tecla espacio
	if (tecla==32) {
		item_seleccionado->menu_funcion=menu_sel->menu_funcion_espacio;
		tecla=13;
	}
	else item_seleccionado->menu_funcion=menu_sel->menu_funcion;

	item_seleccionado->tipo_opcion=menu_sel->tipo_opcion;
	item_seleccionado->valor_opcion=menu_sel->valor_opcion;
	strcpy(item_seleccionado->texto_opcion,menu_sel->texto_opcion);
	strcpy(item_seleccionado->texto_misc,menu_sel->texto_misc);

	//printf ("misc selected: %s %s\n",item_seleccionado->texto_misc,menu_sel->texto_misc);


	//Liberar memoria del menu
        aux=m;
	menu_item *nextfree;

        do {
		//printf ("Liberando %x\n",aux);
		nextfree=aux->next;
		free(aux);
                aux=nextfree;
        } while (aux!=NULL);


	//Salir del menu diciendo que no se ha pulsado tecla
	menu_speech_tecla_pulsada=0;


	//En caso de menus tabulados, es responsabilidad de este de borrar con cls y liberar ventana 
	if (m->es_menu_tabulado==0) {
		cls_menu_overlay();
		zxvision_destroy_window(ventana);
	}

	//printf ("tecla: %d\n",tecla);

	if (tecla==MENU_RETORNO_ESC) return MENU_RETORNO_ESC;
	else if (tecla==MENU_RETORNO_F1) return MENU_RETORNO_F1;
	else if (tecla==MENU_RETORNO_F2) return MENU_RETORNO_F2;
	else if (tecla==MENU_RETORNO_F10) return MENU_RETORNO_F10;

	else return MENU_RETORNO_NORMAL;

}

















//Agregar el item inicial del menu
//Parametros: puntero al puntero de menu_item inicial. texto
void menu_add_item_menu_inicial(menu_item **p,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo)
{

	menu_item *m;

	m=malloc(sizeof(menu_item));

	//printf ("%d\n",sizeof(menu_item));

        if (m==NULL) cpu_panic("Cannot allocate initial menu item");

	//comprobacion de maximo
	if (strlen(texto)>MAX_TEXTO_OPCION) cpu_panic ("Text item greater than maximum");

	//m->texto=texto;
	strcpy(m->texto_opcion,texto);





	m->tipo_opcion=tipo_opcion;
	m->menu_funcion=menu_funcion;
	m->menu_funcion_activo=menu_funcion_activo;
	m->texto_ayuda=NULL;
	m->texto_tooltip=NULL;

	//Por defecto inicializado a ""
	m->texto_misc[0]=0;

	m->atajo_tecla=0;
	m->menu_funcion_espacio=NULL;


	m->es_menu_tabulado=0; //por defecto no es menu tabulado. esta opcion se hereda en cada item, desde el primero


	m->next=NULL;


	*p=m;
}

//Agregar un item al menu
//Parametros: puntero de menu_item inicial. texto
void menu_add_item_menu(menu_item *m,char *texto,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo)
{
	//busca el ultimo item i le añade el indicado. O hasta que encuentre uno con MENU_OPCION_UNASSIGNED, que tendera a ser el ultimo

	while (m->next!=NULL && m->tipo_opcion!=MENU_OPCION_UNASSIGNED)
	{
		m=m->next;
	}

	menu_item *next;




	if (m->tipo_opcion==MENU_OPCION_UNASSIGNED) {
		debug_printf (VERBOSE_DEBUG,"Overwrite last item menu because it was MENU_OPCION_UNASSIGNED");
		next=m;
	}

	else {

		next=malloc(sizeof(menu_item));
		//printf ("%d\n",sizeof(menu_item));

		if (next==NULL) cpu_panic("Cannot allocate menu item");

		m->next=next;
	}


	//Si era menu tabulado. Heredamos la opcion. Aunque se debe establecer la x,y luego para cada item, lo mantenemos asi para que cada item,
	//tengan ese parametro
	int es_menu_tabulado;
	es_menu_tabulado=m->es_menu_tabulado;

	//comprobacion de maximo
	if (strlen(texto)>MAX_TEXTO_OPCION) cpu_panic ("Text item greater than maximum");

	//next->texto=texto;
	strcpy(next->texto_opcion,texto);



	next->tipo_opcion=tipo_opcion;
	next->menu_funcion=menu_funcion;
	next->menu_funcion_activo=menu_funcion_activo;
	next->texto_ayuda=NULL;
	next->texto_tooltip=NULL;

	//Por defecto inicializado a ""
	next->texto_misc[0]=0;

	next->atajo_tecla=0;
	next->menu_funcion_espacio=NULL;
	next->es_menu_tabulado=es_menu_tabulado;
	next->next=NULL;
}

//Agregar ayuda al ultimo item de menu
void menu_add_item_menu_ayuda(menu_item *m,char *texto_ayuda)
{
       //busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

	m->texto_ayuda=texto_ayuda;
}

//Agregar tooltip al ultimo item de menu
void menu_add_item_menu_tooltip(menu_item *m,char *texto_tooltip)
{
       //busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

        m->texto_tooltip=texto_tooltip;
}

//Agregar atajo de tecla al ultimo item de menu
void menu_add_item_menu_shortcut(menu_item *m,z80_byte tecla)
{
       //busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

        m->atajo_tecla=tecla;
}


//Agregar funcion de gestion de tecla espacio
void menu_add_item_menu_espacio(menu_item *m,t_menu_funcion menu_funcion_espacio)
{
//busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

        m->menu_funcion_espacio=menu_funcion_espacio;
}


//Indicar que es menu tabulado. Se hace para todos los items, dado que establece coordenada x,y
void menu_add_item_menu_tabulado(menu_item *m,int x,int y)
{
//busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

        m->es_menu_tabulado=1;
	m->menu_tabulado_x=x;
	m->menu_tabulado_y=y;
}


//Agregar un valor como opcion al ultimo item de menu
//Esto sirve, por ejemplo, para que cuando esta en el menu de z88, insertar slot,
//se pueda saber que slot se ha seleccionado
void menu_add_item_menu_valor_opcion(menu_item *m,int valor_opcion)
{
       //busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

	//printf ("temp. agregar valor opcion %d\n",valor_opcion);

        m->valor_opcion=valor_opcion;
}


//Agregar texto misc al ultimo item de menu
//Esto sirve, por ejemplo, para guardar url en navegador online
void menu_add_item_menu_misc(menu_item *m,char *texto_misc)
{
       //busca el ultimo item i le añade el indicado

        while (m->next!=NULL)
        {
                m=m->next;
        }

		

        strcpy(m->texto_misc,texto_misc);

		//printf ("agregado texto misc %s\n",m->texto_misc);
}


//Agregar un item al menu
//Parametros: puntero de menu_item inicial. texto con formato
void menu_add_item_menu_format(menu_item *m,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...)
{
	char buffer[100];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	va_end (args);

	menu_add_item_menu(m,buffer,tipo_opcion,menu_funcion,menu_funcion_activo);
}


//Agregar el item inicial del menu
//Parametros: puntero al puntero de menu_item inicial. texto con formato
void menu_add_item_menu_inicial_format(menu_item **p,int tipo_opcion,t_menu_funcion menu_funcion,t_menu_funcion_activo menu_funcion_activo,const char * format , ...)
{
        char buffer[100];
        va_list args;
        va_start (args, format);
        vsprintf (buffer,format, args);
	va_end (args);

        menu_add_item_menu_inicial(p,buffer,tipo_opcion,menu_funcion,menu_funcion_activo);

}

char *string_esc_go_back="ESC always go back to the previous menu, or return back to the emulated machine if you are in main menu";

//Agrega item de ESC normalmente.  En caso de aalib y consola es con tecla TAB
void menu_add_ESC_item(menu_item *array_menu_item)
{

        char mensaje_esc_back[32];

        sprintf (mensaje_esc_back,"%s Back",esc_key_message);

        menu_add_item_menu(array_menu_item,mensaje_esc_back,MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_item_menu_tooltip(array_menu_item,string_esc_go_back);
		menu_add_item_menu_ayuda(array_menu_item,string_esc_go_back);

}







int menu_cond_zx81(void)
{
        if (MACHINE_IS_ZX81) return 1;
        return 0;
}

int menu_cond_zx81_realvideo(void)
{
        if (menu_cond_zx81()==0) return 0;
        return rainbow_enabled.v;

}

int menu_cond_realvideo(void)
{
	return rainbow_enabled.v;

}




int menu_cond_zx8081(void)
{
	if (MACHINE_IS_ZX8081) return 1;
	return 0;
}

int menu_cond_zx8081_realvideo(void)
{
	if (menu_cond_zx8081()==0) return 0;
	return rainbow_enabled.v;
}

int menu_cond_zx8081_wrx(void)
{
        if (menu_cond_zx8081()==0) return 0;
        return wrx_present.v;
}

int menu_cond_zx8081_wrx_no_stabilization(void)
{
	if (menu_cond_zx8081_wrx()==0) return 0;
	return !video_zx8081_estabilizador_imagen.v;
}

int menu_cond_zx8081_no_realvideo(void)
{
        if (menu_cond_zx8081()==0) return 0;
        return !rainbow_enabled.v;
}

int menu_cond_curses(void)
{
	if (!strcmp(scr_driver_name,"curses")) return 1;
	return 0;
}

int menu_cond_stdout(void)
{
        if (!strcmp(scr_driver_name,"stdout")) return 1;

        return 0;
}

int menu_cond_simpletext(void)
{
        if (!strcmp(scr_driver_name,"simpletext")) return 1;

        return 0;
}



/*
int menu_cond_no_stdout(void)
{
        //esto solo se permite en drivers xwindows, caca, aa, curses. NO en stdout
        if (!strcmp(scr_driver_name,"stdout")) return 0;
        return 1;
}
*/

int menu_cond_no_curses_no_stdout(void)
{
        //esto solo se permite en drivers xwindows, caca, aa. NO en curses ni stdout
        if (!strcmp(scr_driver_name,"curses")) return 0;
        if (!strcmp(scr_driver_name,"stdout")) return 0;
	return 1;
}




int menu_cond_zx8081_no_curses_no_stdout(void)
{
	if (!menu_cond_zx8081()) return 0;
	return menu_cond_no_curses_no_stdout();

}

/*int menu_cond_spectrum(void)
{
	return (MACHINE_IS_SPECTRUM);
	//return !menu_cond_zx8081();
}*/







//			//Hacer decaer el volumen
//			if (menu_waveform_previous_volume>menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume--;

//Funcion usada por los vu-meters para hacer el efecto de "decae" del maximo
//Retorna valor de variable de decae, segun el ultimo valor del volumen
int menu_decae_dec_valor_volumen(int valor_decae,int valor_volumen)
{
	//Hacer decaer el volumen
	if (valor_decae>valor_volumen) valor_decae--;	

	return valor_decae;
}


//	//Volume. Mostrarlo siempre, no solo dos veces por segundo, para que se actualice mas frecuentemente
//	if (menu_waveform_previous_volume<menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume=menu_audio_draw_sound_wave_volumen_escalado;

//Funcion usada por los vu-meters cada vez que se quieren mostrar, ver si valor de variable decae es menor que volumen, 
//entonces modificarla

int menu_decae_ajusta_valor_volumen(int valor_decae,int valor_volumen)
{
	if (valor_decae<valor_volumen) valor_decae=valor_volumen;

	return valor_decae;

}

//llena el string con el valor del volumen - para chip de sonido
//mete tambien caracter de "decae" si conviene (si >=0 y <=15)
void menu_string_volumen(char *texto,z80_byte registro_volumen,int indice_decae)
{
	if ( (registro_volumen & 16)!=0) sprintf (texto,"ENV            ");
	else {
		registro_volumen=registro_volumen & 15;
		int i;
		int destino;
		int indicado_rojo=0;

		

		for (i=0,destino=0;i<registro_volumen;i++) {
			texto[destino++]='=';


			//Codigo control color tinta. 
			if (i==11) { 
				texto[destino++]='$';
				texto[destino++]='$';
				texto[destino++]='0'+ESTILO_GUI_COLOR_AVISO; //'2';
				indicado_rojo=1;
			}
		}

        for (;i<15;i++) {
        	texto[destino++]=' ';
        }

		texto[destino]=0;

		//Si indice es menor que volumen, forzar a valor que volumen
		if (indice_decae<registro_volumen) indice_decae=registro_volumen;

		if (indice_decae>=0 && indice_decae<=14 && indice_decae>=registro_volumen) texto[indice_decae+indicado_rojo*3]='>';

		//printf ("registro volumen: %d indice decae: %d pos decae: %d\n",registro_volumen,indice_decae,indice_decae+indicado_rojo*3);
	}
}




void menu_debug_ioports(MENU_ITEM_PARAMETERS)
{

	char stats_buffer[MAX_TEXTO_GENERIC_MESSAGE];


	debug_get_ioports(stats_buffer);

  menu_generic_message("IO Ports",stats_buffer);

}






menu_z80_moto_int menu_debug_disassemble_bajar(menu_z80_moto_int dir_inicial)
{
	//Bajar 1 opcode en el listado
	        char buffer[32];
        size_t longitud_opcode;

	debugger_disassemble(buffer,30,&longitud_opcode,dir_inicial);

	dir_inicial +=longitud_opcode;

	return dir_inicial;
}



menu_z80_moto_int menu_debug_disassemble_subir(menu_z80_moto_int dir_inicial)
{
	//Subir 1 opcode en el listado

	//Metodo:
	//Empezamos en direccion-10 (en QL: direccion-30)
	//inicializamos un puntero ficticio de direccion a 0, mientras que mantenemos la posicion de memoria de lectura inicial en direccion-10/30
	//Vamos leyendo opcodes. Cuando el puntero ficticio este >=10 (o 30), nuestra direccion final será la inicial - longitud opcode anterior

	char buffer[32];
	size_t longitud_opcode;

	menu_z80_moto_int dir;

	int decremento=10;

	if (CPU_IS_MOTOROLA) decremento=30; //En el caso de motorola mejor empezar antes


	dir=dir_inicial-decremento;

	dir=menu_debug_hexdump_adjusta_en_negativo(dir,1);

	//menu_z80_moto_int dir_anterior=dir;

	int puntero_ficticio=0;


	do {

		//dir_anterior=dir;

		debugger_disassemble(buffer,30,&longitud_opcode,dir);
		
		dir+=longitud_opcode;
		dir=adjust_address_memory_size(dir);
		puntero_ficticio+=longitud_opcode;

		//printf ("dir %X dir_anterior %X puntero_ficticio %d\n",dir,dir_anterior,puntero_ficticio);

		if (puntero_ficticio>=decremento) {
			return menu_debug_hexdump_adjusta_en_negativo(dir_inicial-longitud_opcode,1);
		}
		

	} while (1);


}

//Desensamblando usando un maximo de 64 caracteres
void menu_debug_dissassemble_una_inst_sino_hexa(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode,int sino_hexa,int full_hexa_dump_motorola)
{

	char buf_temp_dir[65];
	char buf_temp_hexa[65];
	char buf_temp_opcode[65];

	size_t longitud_opcode;

	//int full_hexa_dump_motorola=1;

	//Direccion

	dir=adjust_address_memory_size(dir);


	//Texto direccion
	menu_debug_print_address_memory_zone(buf_temp_dir,dir);

	int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	//metemos espacio en 0 final
	dumpassembler[longitud_direccion]=' ';

	int max_longitud_volcado_hexa=8;

	//Hasta instrucciones de 8 bytes si se indica full dump
	//Si no, como maximo mostrara 4 bytes (longitud hexa=8)
	//El full dump solo aparece en menu disassemble, pero no en debug cpu
	if (CPU_IS_MOTOROLA && full_hexa_dump_motorola) max_longitud_volcado_hexa=16;


	//Texto opcode
	debugger_disassemble(buf_temp_opcode,64,&longitud_opcode,dir);


	//Texto volcado hexa
	//Primero meter espacios hasta limite 64
	int i;
	for (i=0;i<64;i++) {
		buf_temp_hexa[i]=' ';
	}

	buf_temp_hexa[i]=0;

	menu_debug_registers_dump_hex(buf_temp_hexa,dir,longitud_opcode);
	int longitud_texto_hex=longitud_opcode*2;
	//quitar el 0 final
	buf_temp_hexa[longitud_texto_hex]=' ';


	//agregar un espacio final para poder meter "+" en caso necesario, esto solo sucede en Motorola
	if (CPU_IS_MOTOROLA) {
		buf_temp_hexa[max_longitud_volcado_hexa]=' ';
		buf_temp_hexa[max_longitud_volcado_hexa+1]=0;
	}

	else {
		//Meter el 0 final donde diga el limite de volcado
		buf_temp_hexa[max_longitud_volcado_hexa]=0;
	}

	//Si meter +
	if (longitud_texto_hex>max_longitud_volcado_hexa) {
		buf_temp_hexa[max_longitud_volcado_hexa]='+';
	}


	//Montar todo
	if (sino_hexa) {
		sprintf(dumpassembler,"%s %s %s",buf_temp_dir,buf_temp_hexa,buf_temp_opcode);
	}

	else {
		sprintf(dumpassembler,"%s %s",buf_temp_dir,buf_temp_opcode);
	}

	*longitud_final_opcode=longitud_opcode;

}


void menu_debug_dissassemble_una_instruccion(char *dumpassembler,menu_z80_moto_int dir,int *longitud_final_opcode)
{
	menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,longitud_final_opcode,1,0);
}







void menu_linea_zxvision(zxvision_window *ventana,int x,int y1,int y2,int color)
{

	int yorigen;
	int ydestino;


	//empezamos de menos a mas
	if (y1<y2) {
		yorigen=y1;
		ydestino=y2;
	}

	else {
		yorigen=y2;
		ydestino=y1;
	}


	for (;yorigen<=ydestino;yorigen++) {
		zxvision_putpixel(ventana,x,yorigen,color);
	}
}



void menu_ay_pianokeyboard_insert_inverse(char *origen_orig, int indice)
{
	char cadena_temporal[40];

	char *destino;

	destino=cadena_temporal;

	char *origen;
	origen=origen_orig;

	int i;

	for (i=0;*origen;origen++,i++) {
			if (i==indice) {
				*destino++='~';
				*destino++='~';
			}

			*destino++=*origen;
	}

	*destino=0;

	//copiar a cadena original
	strcpy(origen_orig,cadena_temporal);
}

//#define PIANO_GRAPHIC_BASE_X 9
#define PIANO_GRAPHIC_BASE_X (menu_center_x()-7)

int piano_graphic_base_y=0;

#define PIANO_ZOOM_X ( menu_char_width>=7 ? 3 : 2 )
#define PIANO_ZOOM_Y 3

#define AY_PIANO_ANCHO_VENTANA ( menu_char_width==8 || menu_char_width==6 ? 14 : 15 )

z80_bit menu_ay_piano_drawing_wavepiano={0};

//Escala alto en vertical teclado piano segun si ay chip>2, para que el teclado sea mas pequeñito
int scale_y_chip(int y)
{
	if (ay_retorna_numero_chips()<3) return y;

	//Si venimos de un Wave Piano, no hay que hacerlo pequeño , aunque tengamos 3 chips de audio, a ese menu no le tiene que afectar
	if (menu_ay_piano_drawing_wavepiano.v) return y;

	//Casos:
	//3,4,7,8
	if (y==3) y=1;
	else if (y==4) y=2;
	else if (y==7) y=5;
	else if (y==8) y=6;

	//temp
	return y;
}




void menu_ay_pianokeyboard_draw_graphical_piano_draw_pixel_zoom(int x,int y,int color)
{
	//#define PIANO_ZOOM 3

	int offsetx=12;
	int offsety=scale_y_chip(8)+0;

	x=offsetx+x*PIANO_ZOOM_X;
	y=offsety+y*PIANO_ZOOM_Y;

	int xorig=x;
	int zx=0;
	int zy=0;

	for (zy=0;zy<PIANO_ZOOM_Y;zy++) {
		x=xorig;
		for (zx=0;zx<PIANO_ZOOM_X;zx++) {
			//No deberia ser null , pero por si acaso
			if (zxvision_current_window!=NULL) zxvision_putpixel(zxvision_current_window,x,y,color);

			x++;

		}
		y++;
	}

}



//Basandome en coordenadas basicas sin zoom
void menu_ay_pianokeyboard_draw_graphical_piano_draw_line(int x, int y, int stepx, int stepy, int length, int color)
{

	for (;length>0;length--) {
			menu_ay_pianokeyboard_draw_graphical_piano_draw_pixel_zoom(x,y,color);
			x +=stepx;
			y +=stepy;
	}

}

void menu_ay_piano_graph_dibujar_negra(int x, int y,int color)
{
 int alto=4;

	for (alto=0;alto<4;alto++) {
		menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, y, +1, 0, 3, color);
		y++;
	}
}


//Como C, F
void menu_ay_piano_graph_dibujar_blanca_izquierda(int x, int y,int color)
{
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, y, 0, +1, scale_y_chip(7), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+1, y, 0, +1, scale_y_chip(7), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+2, y+4, 0, +1, scale_y_chip(3), color);
}

//Como D, G, A
void menu_ay_piano_graph_dibujar_blanca_media(int x, int y,int color)
{

	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, y+4, 0, +1, scale_y_chip(3), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+1, y, 0, +1, scale_y_chip(7), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+2, y+4, 0, +1, scale_y_chip(3), color);
}


//Como E, B
void menu_ay_piano_graph_dibujar_blanca_derecha(int x, int y,int color)
{
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, y+4, 0, +1, scale_y_chip(3), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+1, y, 0, +1, scale_y_chip(7), color);
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x+2, y, 0, +1, scale_y_chip(7), color);
}

void menu_ay_pianokeyboard_draw_graphical_piano(int linea GCC_UNUSED,int canal,char *note)
{
	/*
	Teclado:
	0123456789012345678901234567890

	xxxxxxxxxxxxxxxxxxxxxxxxxxxxx         0
	x  xxx xxx  x  xxx xxx xxx  x         1
	x  xxx xxx  x  xxx xxx xxx  x         2
	x  xxx xxx  x  xxx xxx xxx  x         3
	x  xxx xxx  x  xxx xxx xxx  x         4
	x   x   x   x   x   x   x   x         5
	x   x   x   x   x   x   x   x         6
	x   x   x   x   x   x   x   x         7

	0123456789012345678901234567890

    C   D   E   F   G   A   B
Altura, para 2 chips de sonido (6 canales), tenemos maximo 192/6=32
32 de alto maximo, podemos hacer zoom x3 del esquema basico, por tanto tendriamos 8x3x6=144 de alto con 2 chips de sonido


	*/

	//scr_putpixel_zoom(x,y,ESTILO_GUI_TINTA_NORMAL);

	int ybase=0; //TODO: depende de linea de entrada

	//printf ("linea: %d\n",linea);

	//temp
	ybase +=scale_y_chip(8)*canal;

	//Recuadro en blanco
	int x,y;
	for (x=0;x<29;x++) {
		for (y=ybase;y<ybase+scale_y_chip(8);y++) {
			menu_ay_pianokeyboard_draw_graphical_piano_draw_pixel_zoom(x,y,7);
		}
	}

	//Linea superior
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(0, ybase+0, +1, 0, 29, 0);

	//Linea vertical izquierda
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(0, ybase+0, 0, +1, scale_y_chip(8), 0);

	//Linea vertical derecha
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(28, ybase+0, 0, +1, scale_y_chip(8), 0);

	//6 separaciones verticales pequeñas
	int i;
	x=4;
	for (i=0;i<6;i++) {
		menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, ybase+5, 0, +1, scale_y_chip(3), 0);
		x+=4;
	}

	//Linea vertical central
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(12, ybase+0, 0, +1, scale_y_chip(8), 0);

	//Y ahora las 5 negras
	x=3;
	for (i=0;i<5;i++) {
		menu_ay_piano_graph_dibujar_negra(x,ybase+1,0);
		/*
		for (y=1;y<=4;y++) {
			menu_ay_pianokeyboard_draw_graphical_piano_draw_line(x, y, +1, 0, 3, 0);
		}
		*/

		x+=4;
		if (i==1) x+=4;  //Saltar posicion donde iria la "tercera" negra
	}


	//Dibujar la linea inferior. Realmente la linea inferior es siempre la linea superior del siguiente canal, excepto en el ultimo canal
	menu_ay_pianokeyboard_draw_graphical_piano_draw_line(0, ybase+scale_y_chip(8), +1, 0, 29, 0);

	//Y ahora destacar la que se pulsa
	char letra_nota=note[0];
	int es_negra=0;
	if (note[1]=='#') es_negra=1;

	if (es_negra) {

		//determinar posicion x
		switch (letra_nota)
		{
			case 'C':
				x=3;
			break;

			case 'D':
				x=7;
			break;

			case 'F':
				x=15;
			break;

			case 'G':
				x=19;
			break;

			case 'A':
				x=23;
			break;

			default:
				//por si acaso
				x=-1;
			break;
		}
		if (x!=-1) menu_ay_piano_graph_dibujar_negra(x,ybase+1,1); //Color 1 para probar
	}

	else {
		//blancas
		switch (letra_nota)
		{
			case 'C':
			 menu_ay_piano_graph_dibujar_blanca_izquierda(1, ybase+1,1);
			break;

			case 'D':
			//Como D, G, A
			 menu_ay_piano_graph_dibujar_blanca_media(5, ybase+1,1);
			break;

			case 'E':
				menu_ay_piano_graph_dibujar_blanca_derecha(9, ybase+1,1);
			break;

			case 'F':
			 menu_ay_piano_graph_dibujar_blanca_izquierda(13, ybase+1,1);
			break;

			case 'G':
			//Como D, G, A
			 menu_ay_piano_graph_dibujar_blanca_media(17, ybase+1,1);
			break;

			case 'A':
			//Como D, G, A
			 menu_ay_piano_graph_dibujar_blanca_media(21, ybase+1,1);
			break;

			case 'B':
				menu_ay_piano_graph_dibujar_blanca_derecha(25, ybase+1,1);
			break;
		}

	}


}

void menu_ay_pianokeyboard_draw_text_piano(int linea,int canal GCC_UNUSED,char *note)
{

	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;

	menu_writing_inverse_color.v=1;

	char linea_negras[40];
	char linea_blancas[40];
												//012345678901
	sprintf (linea_negras, " # #  # # #");
	sprintf (linea_blancas,"C D EF G A B");

	if (note==NULL || note[0]==0) {
	}

	else {
		//Marcar tecla piano pulsada con ~~
		//Interpretar Nota viene como C#4 o C4 por ejemplo
		char letra_nota=note[0];
		int es_negra=0;
		if (note[1]=='#') es_negra=1;

		//TODO: mostramos la octava?

		//Linea negras
		if (es_negra) {
				int indice_negra_marcar=0;
				switch (letra_nota)
				{
					case 'C':
						indice_negra_marcar=1;
					break;

					case 'D':
						indice_negra_marcar=3;
					break;

					case 'F':
						indice_negra_marcar=6;
					break;

					case 'G':
						indice_negra_marcar=8;
					break;

					case 'A':
						indice_negra_marcar=10;
					break;
				}

				//Reconstruimos la cadena introduciendo ~~donde indique el indice
				menu_ay_pianokeyboard_insert_inverse(linea_negras,indice_negra_marcar);
		}

		//Linea blancas
		else {
			int indice_blanca_marcar=0;
			//												//012345678901
			//	sprintf (linea_negras, " # #  # # #");
			//	sprintf (linea_blancas,"C D EF G A B");
			switch (letra_nota)
			{
			  case 'C':
			    indice_blanca_marcar=0;
			  break;

			  case 'D':
			    indice_blanca_marcar=2;
			  break;

			  case 'E':
			    indice_blanca_marcar=4;
			  break;

			  case 'F':
			    indice_blanca_marcar=5;
			  break;

			  case 'G':
			    indice_blanca_marcar=7;
			  break;

			  case 'A':
			    indice_blanca_marcar=9;
			  break;

			  case 'B':
			    indice_blanca_marcar=11;
			  break;
			}

			//Reconstruimos la cadena introduciendo ~~donde indique el indice
			menu_ay_pianokeyboard_insert_inverse(linea_blancas,indice_blanca_marcar);
		}
	}

	//menu_escribe_linea_opcion(linea++,-1,1,linea_negras);
	//menu_escribe_linea_opcion(linea++,-1,1,linea_blancas);

	zxvision_print_string_defaults(zxvision_current_window,1,linea++,linea_negras);
	zxvision_print_string_defaults(zxvision_current_window,1,linea++,linea_blancas);


	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

}





//Dice si se muestra piano grafico o de texto.
//Si es un driver de solo texto, mostrar texto
//Si es un driver grafico y setting dice que lo mostremos en texto, mostrar texto
//Si nada de lo demas, mostrar grafico
int si_mostrar_ay_piano_grafico(void)
{
	if (!si_complete_video_driver()) return 0;

	if (!setting_mostrar_ay_piano_grafico.v) return 0;

	return 1;

}

void menu_ay_pianokeyboard_draw_piano(int linea,int canal,char *note)
{
	if (!si_mostrar_ay_piano_grafico()) {
		menu_ay_pianokeyboard_draw_text_piano(linea,canal,note);
	}
	else {
		menu_ay_pianokeyboard_draw_graphical_piano(linea,canal,note);
	}
}


zxvision_window *menu_ay_pianokeyboard_overlay_window;


void menu_ay_pianokeyboard_overlay(void)
{

	//printf ("overlay de menu_ay_pianokeyboard_overlay\n");

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	//workaround_pentagon_clear_putpixel_cache();

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech, en el caso que se habilite piano de tipo texto

	//char volumen[16],textovolumen[32],textotono[32];

	int  total_chips=ay_retorna_numero_chips();
	//Max 3 ay chips
	if (total_chips>3) total_chips=3;



	//if (total_chips>2) total_chips=2;

	int chip;

	int linea=1;

	int canal=0;

	for (chip=0;chip<total_chips;chip++) {


			int freq_a=ay_retorna_frecuencia(0,chip);
			int freq_b=ay_retorna_frecuencia(1,chip);
			int freq_c=ay_retorna_frecuencia(2,chip);

			char nota_a[4];
			sprintf(nota_a,"%s",get_note_name(freq_a) );

			char nota_b[4];
			sprintf(nota_b,"%s",get_note_name(freq_b) );

			char nota_c[4];
			sprintf(nota_c,"%s",get_note_name(freq_c) );

			//Si canales no suenan como tono, o volumen 0 meter cadena vacia en nota
			if (ay_3_8912_registros[chip][7]&1 || ay_3_8912_registros[chip][8]==0) nota_a[0]=0;
			if (ay_3_8912_registros[chip][7]&2 || ay_3_8912_registros[chip][9]==0) nota_b[0]=0;
			if (ay_3_8912_registros[chip][7]&4 || ay_3_8912_registros[chip][10]==0) nota_c[0]=0;

			int incremento_linea=3;

			if (!si_mostrar_ay_piano_grafico()) {
				//Dibujar ay piano con texto. Comprimir el texto (quitar linea de entre medio) cuando hay 3 chips
				if (total_chips>2) incremento_linea=2;
			}


			menu_ay_pianokeyboard_draw_piano(linea,canal,nota_a);
			linea+=incremento_linea;
			canal++;

			menu_ay_pianokeyboard_draw_piano(linea,canal,nota_b);
			linea+=incremento_linea;
			canal++;

			menu_ay_pianokeyboard_draw_piano(linea,canal,nota_c);
			linea+=incremento_linea;
			canal++;

	}

	zxvision_draw_window_contents(menu_ay_pianokeyboard_overlay_window); 

}


zxvision_window zxvision_window_ay_piano;


void menu_ay_pianokeyboard(MENU_ITEM_PARAMETERS)
{
        menu_espera_no_tecla();

		int xventana,yventana,ancho_ventana,alto_ventana;

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		int  total_chips=ay_retorna_numero_chips();
		//Max 3 ay chips
		if (total_chips>3) total_chips=3;

		if (!util_find_window_geometry("aypiano",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {				

				if (!si_mostrar_ay_piano_grafico()) {

					ancho_ventana=14;

					if (total_chips==1) {
						xventana=9;
						yventana=7;					
					}
          			else if (total_chips==2) {
						xventana=9;
						yventana=2;						  					  
					}

					else {
						xventana=9;
						yventana=1;					
					}

				}

				else {
					//Dibujar ay piano con grafico. Ajustar segun ancho de caracter (de ahi que use AY_PIANO_ANCHO_VENTANA en vez de valor fijo 14)
					if (total_chips==1) {
						xventana=PIANO_GRAPHIC_BASE_X;
						yventana=piano_graphic_base_y;
						ancho_ventana=AY_PIANO_ANCHO_VENTANA;					
					}
					else if (total_chips==2) {
						xventana=PIANO_GRAPHIC_BASE_X;
						yventana=piano_graphic_base_y;
						ancho_ventana=AY_PIANO_ANCHO_VENTANA;							
					}

					else {
						xventana=PIANO_GRAPHIC_BASE_X;
						yventana=piano_graphic_base_y;
						ancho_ventana=AY_PIANO_ANCHO_VENTANA;						
					}
				}

			}

		//El alto ventana siempre lo recalculamos segun el numero de chips
				if (!si_mostrar_ay_piano_grafico()) {

					if (total_chips==1) {			
						alto_ventana=11;
					}
          			else if (total_chips==2) {
						alto_ventana=20;						  						  
					}

					else {
						alto_ventana=22;						
					}

				}

				else {
					//Dibujar ay piano con grafico. Ajustar segun ancho de caracter (de ahi que use AY_PIANO_ANCHO_VENTANA en vez de valor fijo 14)
					if (total_chips==1) {
						piano_graphic_base_y=5;						
						alto_ventana=13;						
					}
					else if (total_chips==2) {
						piano_graphic_base_y=1;
						alto_ventana=22;							
					}

					else {
						piano_graphic_base_y=0;						
						alto_ventana=24;							
					}
				}


		char *titulo_ventana="AY Piano";
		int ancho_titulo=menu_da_ancho_titulo(titulo_ventana);

		//Para que siempre se lea el titulo de la ventana
		if (ancho_ventana<ancho_titulo) ancho_ventana=ancho_titulo;

		zxvision_window *ventana;
		ventana=&zxvision_window_ay_piano;

		zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,titulo_ventana);

		zxvision_draw_window(ventana);						

        //z80_byte acumulado;

			menu_ay_pianokeyboard_overlay_window=ventana;


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de piano + texto
        set_menu_overlay_function(menu_ay_pianokeyboard_overlay);





				int valor_contador_segundo_anterior;

valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla=0;

   do {

                //esto hara ejecutar esto 2 veces por segundo
                //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
								if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
										valor_contador_segundo_anterior=contador_segundo;

										//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
				if (menu_multitarea==0) menu_refresca_pantalla();


                }


				menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla!=2 && tecla!=3);				





       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	if (tecla==3) {
                //zxvision_ay_registers_overlay
                ventana->overlay_function=menu_ay_pianokeyboard_overlay;
                printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
				menu_generic_message("Background task","OK. Window put in background");
	}

	else {
		util_add_window_geometry_compact("aypiano",ventana);
		zxvision_destroy_window(ventana);			
	}


	//workaround_pentagon_clear_putpixel_cache();	

	menu_espera_no_tecla();

	/* Nota:
	Creo que este es de los pocos casos en que llamamos a menu_espera_no_tecla al salir, por dos razones:

	1) si no se hiciera, cuando hay text-to-speech+also send menu, la tecla ESC se suele escalar hacia abajo, probablemente porque activa
	variable menu_speech_tecla_pulsada=1
	Probablemente habria que llamar siempre a menu_espera_no_tecla(); al finalizar ventanas que no estan gestionadas por menu_dibuja_menu

	
	2) si no se hiciera, saldria con menu_speech_tecla_pulsada=1, y la tecla pulsada utilizada para salir de esta ventana (ESC),
	acaba pasando al menu anterior, cerrando el menu directamente. Esto solo pasa si no hay text-to-speech+also send menu
	No se muy bien porque solo sucede en este caso, quiza es porque estamos mostrando texto directamente en la funcion overlay y 
	de manera muy rapida. Esto es lo mismo que en el menu de Wave Piano
	

	*/			

}


zxvision_window *menu_beeper_pianokeyboard_overlay_window;


void menu_beeper_pianokeyboard_overlay(void)
{
    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	//workaround_pentagon_clear_putpixel_cache();

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	//char volumen[16],textovolumen[32],textotono[32];


	int linea=1;

	int canal=0;

        audiobuffer_stats audiostats;
        audio_get_audiobuffer_stats(&audiostats);


        int frecuencia=audiostats.frecuencia;

		//printf ("frecuencia %d\n",frecuencia);



			int freq_a=frecuencia;


			char nota_a[4];
			sprintf(nota_a,"%s",get_note_name(freq_a) );

			//Si no hay sonido, suele dar frecuencia 5 o menos
			if (frecuencia<=5) nota_a[0]=0;

			//Indicar que no hay que reducir el tamaño del piano segun el numero de chips (esto va bien en ay piano, pero no aqui)
			menu_ay_piano_drawing_wavepiano.v=1;
			menu_ay_pianokeyboard_draw_piano(linea,canal,nota_a);
			//Restauramos comportamiento por defecto
			menu_ay_piano_drawing_wavepiano.v=0;

			//workaround_pentagon_clear_putpixel_cache();

			char buffer_texto[40];
			
			if (nota_a[0]!=0) {
                        sprintf (buffer_texto,"%d Hz (%s) ",frecuencia,nota_a);
			}

			else strcpy (buffer_texto,"             ");

				
            
			//menu_escribe_linea_opcion(5,-1,1,buffer_texto);
			zxvision_print_string_defaults(menu_beeper_pianokeyboard_overlay_window,1,5,buffer_texto);
			//printf ("menu_speech_tecla_pulsada despues de enviar texto: %d\n",menu_speech_tecla_pulsada);


	zxvision_draw_window_contents(menu_beeper_pianokeyboard_overlay_window); 

}



zxvision_window zxvision_menu_beeper_pianokeyboard;

void menu_beeper_pianokeyboard(MENU_ITEM_PARAMETERS)
{
        menu_espera_no_tecla();

				if (!menu_multitarea) {
					menu_warn_message("This menu item needs multitask enabled");
					return;
				}

				//Como si fuera 1 solo chip


				int xventana,yventana,ancho_ventana,alto_ventana;
		if (!util_find_window_geometry("wavepiano",&xventana,&yventana,&ancho_ventana,&alto_ventana)) {
				if (!si_mostrar_ay_piano_grafico()) {

					xventana=7;
					yventana=7;
					ancho_ventana=19;
					alto_ventana=11;

				}

				else {
					//Dibujar ay piano con grafico. Ajustar segun ancho de caracter (de ahi que use AY_PIANO_ANCHO_VENTANA en vez de valor fijo 14)
					xventana=PIANO_GRAPHIC_BASE_X-2;
					yventana=piano_graphic_base_y;
					ancho_ventana=AY_PIANO_ANCHO_VENTANA;
					alto_ventana=8;

				}
		}


		if (si_mostrar_ay_piano_grafico()) {
			piano_graphic_base_y=8;
		}		

		char *titulo_ventana="Wave Piano";
		int ancho_titulo=menu_da_ancho_titulo(titulo_ventana);

		if (ancho_ventana<ancho_titulo) ancho_ventana=ancho_titulo;				

		zxvision_window *ventana;	
		ventana=&zxvision_menu_beeper_pianokeyboard;

		zxvision_new_window(ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,titulo_ventana);

		zxvision_draw_window(ventana);						

        //z80_byte acumulado;


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de piano + texto
        set_menu_overlay_function(menu_beeper_pianokeyboard_overlay);

		menu_beeper_pianokeyboard_overlay_window=ventana; 


				int valor_contador_segundo_anterior;

valor_contador_segundo_anterior=contador_segundo;

z80_byte tecla=0;

   do {

                //esto hara ejecutar esto 2 veces por segundo
                //if ( (contador_segundo%500) == 0 || menu_multitarea==0) {
								if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
										valor_contador_segundo_anterior=contador_segundo;

										//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);



			if (menu_multitarea==0) menu_refresca_pantalla();

                }

                menu_cpu_core_loop();
                //acumulado=menu_da_todas_teclas();

                //si no hay multitarea, esperar tecla y salir
                if (menu_multitarea==0) {
                        menu_espera_tecla();

                        //acumulado=0;
                }

				//tecla=menu_get_pressed_key();
				tecla=zxvision_read_keyboard();

				//con enter no salimos. TODO: esto se hace porque el mouse esta enviando enter al pulsar boton izquierdo, y lo hace tambien al hacer dragging
				//lo ideal seria que mouse no enviase enter al pulsar boton izquierdo y entonces podemos hacer que se salga tambien con enter
				if (tecla==13) tecla=0;

        //} while (  (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA && tecla==0)  ;

		} while (tecla!=2 && tecla!=3);										

      


       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();


        if (tecla==3) {
                //zxvision_ay_registers_overlay
                ventana->overlay_function=menu_beeper_pianokeyboard_overlay;
                printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
				menu_generic_message("Background task","OK. Window put in background");
        }

        else {
		util_add_window_geometry_compact("wavepiano",ventana);
		zxvision_destroy_window(ventana);

	}

	menu_espera_no_tecla();


	/* Nota:
	Creo que este es de los pocos casos en que llamamos a menu_espera_no_tecla al salir,
	si no se hiciera, saldria con menu_speech_tecla_pulsada=1, y la tecla pulsada utilizada para salir de esta ventana (ESC),
	acaba pasando al menu anterior, cerrando el menu directamente. Esto solo pasa si no hay text-to-speech+also send menu
	No se muy bien porque solo sucede en este caso, quiza es porque estamos mostrando texto directamente en la funcion overlay y 
	de manera muy rapida
	*/	

}


int menu_cond_allow_write_rom(void)
{

	if (superupgrade_enabled.v) return 0;
	if (dandanator_enabled.v) return 0;
	if (kartusho_enabled.v) return 0;
	if (ifrom_enabled.v) return 0;
	if (betadisk_enabled.v) return 0;

	if (MACHINE_IS_INVES) return 0;
	if (MACHINE_IS_SPECTRUM_16_48) return 1;
	if (MACHINE_IS_ZX8081) return 1;
	if (MACHINE_IS_ACE) return 1;
	if (MACHINE_IS_SAM) return 1;

	return 0;

}

void menu_hardware_allow_write_rom(MENU_ITEM_PARAMETERS)
{
	if (allow_write_rom.v) {
		reset_poke_byte_function_writerom();
                allow_write_rom.v=0;

	}

	else {
		set_poke_byte_function_writerom();
		allow_write_rom.v=1;
	}
}





//menu audio settings
void menu_audio_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_audio_settings;
	menu_item item_seleccionado;
	int retorno_menu;

        do {


					menu_add_item_menu_inicial(&array_menu_audio_settings,"AY ~~Registers",MENU_OPCION_NORMAL,menu_ay_registers,menu_cond_ay_chip);
					menu_add_item_menu_shortcut(array_menu_audio_settings,'r');


					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_ay_mixer,menu_cond_ay_chip,"AY Mi~~xer");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'x');

					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_ay_pianokeyboard,menu_cond_ay_chip,"AY P~~iano");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'i');
					menu_add_item_menu_tooltip(array_menu_audio_settings,"Shows a piano keyboard with the notes being played on the AY Chip");
                	menu_add_item_menu_ayuda(array_menu_audio_settings,"Shows a piano keyboard with the notes being played on the AY Chip");


		if (si_complete_video_driver() ) {
					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_ay_partitura,menu_cond_ay_chip,"AY ~~Sheet");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'s');

		}
					

					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_audio_new_ayplayer,NULL,"AY ~~Player");
					menu_add_item_menu_tooltip(array_menu_audio_settings,"Opens the .ay file player menu");
					menu_add_item_menu_ayuda(array_menu_audio_settings,"Opens the .ay file player menu");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'p');


					
					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_record_mid,menu_cond_ay_chip,"AY to .~~mid");
					menu_add_item_menu_tooltip(array_menu_audio_settings,"Saves music from the AY Chip to a .mid file");
					menu_add_item_menu_ayuda(array_menu_audio_settings,"Saves music from the AY Chip to a .mid file");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'m');

	


					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_beeper_pianokeyboard,NULL,"W~~ave Piano");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'a');		
					menu_add_item_menu_tooltip(array_menu_audio_settings,"Shows a piano keyboard with the note being played through the output speakers");
                	menu_add_item_menu_ayuda(array_menu_audio_settings,"Shows a piano keyboard with the note being played through the output speakers. "
						"In case you don't have AY sound or DAC audio, that note is the one that is played through the beeper. "
						"It can be inaccurate with short notes");


					

					menu_add_item_menu_format(array_menu_audio_settings,MENU_OPCION_NORMAL,menu_audio_new_waveform,NULL,"~~Waveform");
					menu_add_item_menu_tooltip(array_menu_audio_settings,"Shows the waveform being played through the output speakers");
					menu_add_item_menu_ayuda(array_menu_audio_settings,"Shows the waveform being played through the output speakers");
					menu_add_item_menu_shortcut(array_menu_audio_settings,'w');






                menu_add_item_menu(array_menu_audio_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_audio_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_audio_settings);

                retorno_menu=menu_dibuja_menu(&audio_settings_opcion_seleccionada,&item_seleccionado,array_menu_audio_settings,"Audio" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
							
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int menu_insert_slot_number;
int menu_insert_slot_ram_size;
int menu_insert_slot_type;
//char menu_insert_slot_rom_name[PATH_MAX];
char menu_insert_slot_eprom_name[PATH_MAX];
char menu_insert_slot_flash_intel_name[PATH_MAX];

void menu_z88_slot_insert_internal_ram(MENU_ITEM_PARAMETERS)
{

	//RAM interna. Siempre entre 32 y 512 K
        if (menu_insert_slot_ram_size==512*1024) menu_insert_slot_ram_size=32*1024;
        else menu_insert_slot_ram_size *=2;
}


void menu_z88_slot_insert_ram(MENU_ITEM_PARAMETERS)
{

	//RAM siempre entre 32 y 1024 K
	if (menu_insert_slot_ram_size==0) menu_insert_slot_ram_size=32768;
	else if (menu_insert_slot_ram_size==1024*1024) menu_insert_slot_ram_size=0;
	else menu_insert_slot_ram_size *=2;
}





int menu_z88_eprom_size(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_z88_eprom_size;
        menu_item item_seleccionado;
        int retorno_menu;

        do {
               //32, 128, 256

                menu_add_item_menu_inicial_format(&array_menu_z88_eprom_size,MENU_OPCION_NORMAL,NULL,NULL,"32 Kb");

                menu_add_item_menu_format(array_menu_z88_eprom_size,MENU_OPCION_NORMAL,NULL,NULL,"128 Kb");

		menu_add_item_menu_format(array_menu_z88_eprom_size,MENU_OPCION_NORMAL,NULL,NULL,"256 Kb");


                menu_add_item_menu(array_menu_z88_eprom_size,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_z88_eprom_size);

                retorno_menu=menu_dibuja_menu(&z88_eprom_size_opcion_seleccionada,&item_seleccionado,array_menu_z88_eprom_size,"Eprom Size" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }

			//Devolver tamanyo eprom
			if (z88_eprom_size_opcion_seleccionada==0) return 32*1024;
			if (z88_eprom_size_opcion_seleccionada==1) return 128*1024;
			if (z88_eprom_size_opcion_seleccionada==2) return 256*1024;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);

	//Salimos con ESC, devolver 0
	return 0;
}


void menu_z88_slot_insert_eprom(MENU_ITEM_PARAMETERS)
{
        char *filtros[4];

        filtros[0]="eprom";
        filtros[1]="epr";
        filtros[2]="63";
        filtros[3]=0;

	//guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de rom
        //si no hay directorio, vamos a rutas predefinidas
        if (menu_insert_slot_eprom_name[0]==0) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(menu_insert_slot_eprom_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }



        int ret;

        ret=menu_filesel("Select existing or new",filtros,menu_insert_slot_eprom_name);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

       		//Ver si archivo existe y preguntar si inicializar o no
                struct stat buf_stat;

                if (stat(menu_insert_slot_eprom_name, &buf_stat)!=0) {

			if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
				menu_insert_slot_eprom_name[0]=0;
				return;
			}

			//Preguntar tamanyo EPROM
			int size=menu_z88_eprom_size(0);

			if (size==0) {
				//ha salido con ESC
				menu_insert_slot_eprom_name[0]=0;
				return;
			}


			if (z88_create_blank_eprom_flash_file(menu_insert_slot_eprom_name,size)) {
				menu_insert_slot_eprom_name[0]=0;
			}

			return;
                }
        }

        else menu_insert_slot_eprom_name[0]=0;
}


void menu_z88_slot_insert_hybrid_eprom(MENU_ITEM_PARAMETERS)
{
        char *filtros[4];

        filtros[0]="eprom";
        filtros[1]="epr";
        //filtros[2]="63"; .63 extensions no admitidas para eprom que son hybridas
        filtros[2]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de rom
        //si no hay directorio, vamos a rutas predefinidas
        if (menu_insert_slot_eprom_name[0]==0) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(menu_insert_slot_eprom_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }



        int ret;

        ret=menu_filesel("Select existing or new",filtros,menu_insert_slot_eprom_name);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

  if (ret==1) {

                //Ver si archivo existe y preguntar si inicializar o no
                struct stat buf_stat;

                if (stat(menu_insert_slot_eprom_name, &buf_stat)!=0) {

                        if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                menu_insert_slot_eprom_name[0]=0;
                                return;
                        }

                        //Tamanyo de la parte EPROM es de 512kb
                        int size=512*1024;


                        if (z88_create_blank_eprom_flash_file(menu_insert_slot_eprom_name,size)) {
                                menu_insert_slot_eprom_name[0]=0;
                        }

                        return;
                }
        }

        else menu_insert_slot_eprom_name[0]=0;
}



int menu_z88_flash_intel_size(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_z88_flash_intel_size;
        menu_item item_seleccionado;
        int retorno_menu;

        do {
               //512,1024

                menu_add_item_menu_inicial_format(&array_menu_z88_flash_intel_size,MENU_OPCION_NORMAL,NULL,NULL,"512 Kb");

                menu_add_item_menu_format(array_menu_z88_flash_intel_size,MENU_OPCION_NORMAL,NULL,NULL,"1 Mb");


                menu_add_item_menu(array_menu_z88_flash_intel_size,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_z88_flash_intel_size);

                retorno_menu=menu_dibuja_menu(&z88_flash_intel_size_opcion_seleccionada,&item_seleccionado,array_menu_z88_flash_intel_size,"Flash Size" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }

                        //Devolver tamanyo flash_intel
                        if (z88_flash_intel_size_opcion_seleccionada==0) return 512*1024;
                        if (z88_flash_intel_size_opcion_seleccionada==1) return 1024*1024;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);

        //Salimos con ESC, devolver 0
        return 0;
}


void menu_z88_slot_insert_flash_intel(MENU_ITEM_PARAMETERS)
{
        char *filtros[2];

        filtros[0]="flash";
        filtros[1]=0;

        int ret;

        ret=menu_filesel("Select existing or new",filtros,menu_insert_slot_flash_intel_name);

        if (ret==1) {

                //Ver si archivo existe y preguntar si inicializar o no
                struct stat buf_stat;

                if (stat(menu_insert_slot_flash_intel_name, &buf_stat)!=0) {

                        if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                menu_insert_slot_flash_intel_name[0]=0;
                                return;
                        }

                        //Preguntar tamanyo Flash
                        int size=menu_z88_flash_intel_size(0);

                        if (size==0) {
                                //ha salido con ESC
                                menu_insert_slot_flash_intel_name[0]=0;
                                return;
                        }


                        if (z88_create_blank_eprom_flash_file(menu_insert_slot_flash_intel_name,size)) {
				menu_insert_slot_flash_intel_name[0]=0;
			}

                        return;
                }
        }

        else menu_insert_slot_flash_intel_name[0]=0;
}




//extern void z88_insert_ram_card(int size,int slot);

//Si se han aplicado cambios (insertar tarjeta memoria) para volver a menu anterior
z80_bit menu_z88_slot_insert_applied_changes={0};

void menu_z88_slot_insert_apply(MENU_ITEM_PARAMETERS)
{

	if (menu_insert_slot_number==0) {
		//Memoria interna

		if (menu_confirm_yesno("Need Hard Reset")==1) {
			z88_change_internal_ram(menu_insert_slot_ram_size);
			menu_z88_slot_insert_applied_changes.v=1;
		}
		return;
	}


	if (menu_insert_slot_type==0) {
		//RAM
		z88_insert_ram_card(menu_insert_slot_ram_size,menu_insert_slot_number);
		menu_z88_slot_insert_applied_changes.v=1;
	}


	/*
	if (menu_insert_slot_type==1) {
                //ROM
		if (menu_insert_slot_rom_name[0]==0) debug_printf (VERBOSE_ERR,"Empty ROM name");

		else {
			z88_load_rom_card(menu_insert_slot_rom_name,menu_insert_slot_number);
			menu_z88_slot_insert_applied_changes.v=1;
		}
	}
	*/

        if (menu_insert_slot_type==2) {
                //EPROM
                if (menu_insert_slot_eprom_name[0]==0) debug_printf (VERBOSE_ERR,"Empty EPROM name");

                else {
			z88_load_eprom_card(menu_insert_slot_eprom_name,menu_insert_slot_number);
			menu_z88_slot_insert_applied_changes.v=1;
		}
        }

        if (menu_insert_slot_type==3) {
                //Intel Flash
                if (menu_insert_slot_flash_intel_name[0]==0) debug_printf (VERBOSE_ERR,"Empty Flash name");

                else {
			z88_load_flash_intel_card(menu_insert_slot_flash_intel_name,menu_insert_slot_number);
			menu_z88_slot_insert_applied_changes.v=1;
		}
        }


        if (menu_insert_slot_type==4) {
                //Hybrida RAM+EPROM
                if (menu_insert_slot_eprom_name[0]==0) debug_printf (VERBOSE_ERR,"Empty EPROM name");

                else {
                        z88_load_hybrid_eprom_card(menu_insert_slot_eprom_name,menu_insert_slot_number);
                        menu_z88_slot_insert_applied_changes.v=1;
                }
        }





}

void menu_z88_slot_insert_change_type(MENU_ITEM_PARAMETERS)
{
	if (menu_insert_slot_ram_size==0) {
		//si no habia, activamos ram con 32kb
		menu_insert_slot_type=0;
		menu_insert_slot_ram_size=32768;
	}

	else if (menu_insert_slot_type==0) {
		//de ram pasamos a eprom
		menu_insert_slot_type=2;
	}

/*	else if (menu_insert_slot_type==1) {
		//de rom pasamos a eprom
		menu_insert_slot_type=2;
	}
*/

        else if (menu_insert_slot_type==2) {
                //de eprom pasamos a flash
                menu_insert_slot_type=3;
        }

        else if (menu_insert_slot_type==3) {
                //de flash pasamos a hybrida ram+eprom
                menu_insert_slot_type=4;
        }




	else {
		//y de flash pasamos a vacio - ram con 0 kb
                menu_insert_slot_type=0;
                menu_insert_slot_ram_size=0;
	}
}


//valor_opcion le viene dado desde la propia funcion de gestion de menu
void menu_z88_slot_insert(MENU_ITEM_PARAMETERS)
{
	//menu_insert_slot_number=z88_slots_opcion_seleccionada;


	menu_insert_slot_number=valor_opcion;

	debug_printf (VERBOSE_DEBUG,"Slot selected on menu: %d",menu_insert_slot_number);

	if (menu_insert_slot_number<0 || menu_insert_slot_number>3) cpu_panic ("Invalid slot number");

	menu_z88_slot_insert_applied_changes.v=0;


	if (menu_insert_slot_number>0) {
		//Memoria externa
		//guardamos tamanyo +1
		menu_insert_slot_ram_size=z88_memory_slots[menu_insert_slot_number].size+1;

		if (menu_insert_slot_ram_size==1) menu_insert_slot_ram_size=0;

		menu_insert_slot_type=z88_memory_slots[menu_insert_slot_number].type;
	}

	else {
		//Memoria interna
		//guardamos tamanyo +1

		menu_insert_slot_ram_size=z88_internal_ram_size+1;
	}



	//reseteamos nombre rom
	//menu_insert_slot_rom_name[0]=0;

	//copiamos nombre eprom/flash a las dos variables
	strcpy(menu_insert_slot_eprom_name,z88_memory_slots[menu_insert_slot_number].eprom_flash_nombre_archivo);

	strcpy(menu_insert_slot_flash_intel_name,z88_memory_slots[menu_insert_slot_number].eprom_flash_nombre_archivo);




        menu_item *array_menu_z88_slot_insert;
        menu_item item_seleccionado;
        int retorno_menu;
        //char string_slot_name_shown[20];
	char string_slot_eprom_name_shown[20],string_slot_flash_intel_name_shown[20];
	char string_memory_type[20];

        do {

		//menu_tape_settings_trunc_name(menu_insert_slot_rom_name,string_slot_name_shown,20);

		menu_tape_settings_trunc_name(menu_insert_slot_eprom_name,string_slot_eprom_name_shown,20);
		menu_tape_settings_trunc_name(menu_insert_slot_flash_intel_name,string_slot_flash_intel_name_shown,20);


                //int slot;


		if (menu_insert_slot_ram_size==0) sprintf (string_memory_type,"%s","Empty");
		else sprintf (string_memory_type,"%s",z88_memory_types[menu_insert_slot_type]);

		//Memorias externas

		if (menu_insert_slot_number>0) {

			menu_add_item_menu_inicial_format(&array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_change_type,NULL,"Memory type: %s",string_memory_type);
        	        menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Type of memory card if present");
                	menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Type of memory card if present");

			if (menu_insert_slot_type==0) {
				//RAM
				menu_add_item_menu_format(array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_ram,NULL,"Size: %d Kb",(menu_insert_slot_ram_size)/1024);
		                menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Size of RAM card");
        		        menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Size of RAM card");

			}

			if (menu_insert_slot_type==1) {
				cpu_panic("ROM cards do not exist on Z88");
				
			}

			if (menu_insert_slot_type==2) {
                	        //EPROM
                        	menu_add_item_menu_format(array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_eprom,NULL,"Name: %s",string_slot_eprom_name_shown);
		                menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"EPROM file to use");
        		        menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"EPROM file to use. Valid formats are .eprom. Select existing or new");

                	}

                        if (menu_insert_slot_type==3) {
                                //Intel Flash
                                menu_add_item_menu_format(array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_flash_intel,NULL,"Name: %s",string_slot_flash_intel_name_shown);
                                menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Intel Flash file to use");
                                menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Intel Flash file to use. Valid formats are .flash. Select existing or new");

                        }

			if (menu_insert_slot_type==4) {
                                //Hybrida RAM+EPROM
                                menu_add_item_menu_format(array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_hybrid_eprom,NULL,"Name: %s",string_slot_eprom_name_shown);
                                menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Hybrid RAM+EPROM file to use");
                                menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Hybrid RAM+EPROM file to use. Valid formats are .eprom. Select existing or new");

                        }



		}

		else {
			//Memoria interna
			menu_add_item_menu_inicial_format(&array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_internal_ram,NULL,"RAM Size: %d Kb",(menu_insert_slot_ram_size)/1024);
			menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Size of RAM card");
                        menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Size of RAM card");
		}




		menu_add_item_menu_format(array_menu_z88_slot_insert,MENU_OPCION_NORMAL,menu_z88_slot_insert_apply,NULL,"Apply changes");
                menu_add_item_menu_tooltip(array_menu_z88_slot_insert,"Apply slot changes");
                menu_add_item_menu_ayuda(array_menu_z88_slot_insert,"Apply slot changes");





                menu_add_item_menu(array_menu_z88_slot_insert,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                menu_add_ESC_item(array_menu_z88_slot_insert);

                //retorno_menu=menu_dibuja_menu(&z88_slot_insert_opcion_seleccionada,&item_seleccionado,array_menu_z88_slot_insert,"Z88 Memory Slots" );

		//Titulo de la ventana variable segun el nombre del slot
		char texto_titulo[32];
		sprintf (texto_titulo,"Z88 Memory Slot %d",menu_insert_slot_number);

                retorno_menu=menu_dibuja_menu(&z88_slot_insert_opcion_seleccionada,&item_seleccionado,array_menu_z88_slot_insert,texto_titulo);

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !menu_z88_slot_insert_applied_changes.v );


}

void menu_z88_slot_erase_eprom_flash(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno("Erase Card")==1) {
		z88_erase_eprom_flash();
		menu_generic_message("Erase Card","OK. Card erased");
        }
}

void menu_z88_eprom_flash_reclaim_free_space(MENU_ITEM_PARAMETERS)
{
       if (menu_confirm_yesno("Reclaim Free Space")==1) {
                z80_long_int liberados=z88_eprom_flash_reclaim_free_space();
                menu_generic_message_format("Reclaim Free Space","OK. %d bytes reclaimed",liberados);
        }

}

void menu_z88_eprom_flash_undelete_files(MENU_ITEM_PARAMETERS)
{
       if (menu_confirm_yesno("Undelete Files")==1) {
                z80_long_int undeleted=z88_eprom_flash_recover_deleted();
                menu_generic_message_format("Undelete Files","OK. %d files undeleted",undeleted);
        }

}

char copy_to_eprom_nombrearchivo[PATH_MAX]="";

void menu_z88_slot_copy_to_eprom_flash(MENU_ITEM_PARAMETERS)
{

	//printf ("%d\n",sizeof(copy_to_eprom_nombrearchivo));

        char *filtros[2];

        filtros[0]="";
        filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        int ret;


        //Obtenemos ultimo directorio visitado
        if (copy_to_eprom_nombrearchivo[0]!=0) {
                char directorio[PATH_MAX];
                util_get_dir(copy_to_eprom_nombrearchivo,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }




        ret=menu_filesel("Select file to copy",filtros,copy_to_eprom_nombrearchivo);

        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {
		//Cargamos archivo en memoria. Maximo 128 KB
		z80_byte *datos;
		const z80_long_int max_load=131072;
		datos=malloc(max_load);

		FILE *ptr_load;
		ptr_load=fopen(copy_to_eprom_nombrearchivo,"rb");

		if (!ptr_load) {
			debug_printf (VERBOSE_ERR,"Unable to open file %s",copy_to_eprom_nombrearchivo);
			return;
		}


		z80_long_int leidos=fread(datos,1,max_load,ptr_load);
		if (leidos==max_load) {
			debug_printf (VERBOSE_ERR,"Can not load more than %d bytes",max_load);
			return;
		}


		if (leidos<1) {
			debug_printf (VERBOSE_ERR,"No bytes read");
			return;
		}


		//esto puede retornar algun error mediante debug_printf
		//nombre en eprom, quitando carpetas
		char nombre_eprom[256];
		util_get_file_no_directory(copy_to_eprom_nombrearchivo,nombre_eprom);


		//Preguntar nombre en destino. Truncar a maximo de nombre en tarjeta memoria
		nombre_eprom[Z88_MAX_CARD_FILENAME]=0;
		menu_ventana_scanf("Target file name?",nombre_eprom,Z88_MAX_CARD_FILENAME+1);


		if (z88_eprom_flash_fwrite(nombre_eprom,datos,leidos)==0) {
			menu_generic_message("Copy File","OK. File copied to Card");
		}

		free(datos);

	}

}


//Funcion de card browser pero las funciones usan punteros de memoria en vez de variables z88_dir
void menu_z88_new_ptr_card_browser(char *archivo)
{

		//printf ("Z88 card browser\n");

        //Asignar 1 mb

        
        int bytes_to_load=1024*1024;

        z80_byte *flash_file_memory;
        flash_file_memory=malloc(bytes_to_load);
        if (flash_file_memory==NULL) {
                debug_printf(VERBOSE_ERR,"Unable to assign memory");
                return;
        }
        
        //Leemos cabecera archivo 
        FILE *ptr_file_flash_browser;
        ptr_file_flash_browser=fopen(archivo,"rb");

        if (!ptr_file_flash_browser) {
                debug_printf(VERBOSE_ERR,"Unable to open file");
                free(flash_file_memory);
                return;
        }


        int leidos=fread(flash_file_memory,1,bytes_to_load,ptr_file_flash_browser);

        if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_flash_browser);

        //El segundo byte tiene que ser 0 (archivo borrado) o '/'
        if (flash_file_memory[1]!=0 && flash_file_memory[1]!='/') {
			//printf ("Calling read text file\n");
        	menu_file_viewer_read_text_file("Flash file",archivo);
        	free(flash_file_memory);
        	return;
        }


	#define MAX_TEXTO 4096
        char texto_buffer[MAX_TEXTO];

        int max_texto=MAX_TEXTO;


        //z88_dir dir;
        z88_eprom_flash_file file;

        //z88_eprom_flash_find_init(&dir,slot);

        z80_byte *dir;

        dir=flash_file_memory;

        char buffer_nombre[Z88_MAX_CARD_FILENAME+1];

        int retorno;
        int longitud;


        int indice_buffer=0;

        do {
                retorno=z88_eprom_new_ptr_flash_find_next(&dir,&file);
                if (retorno) {
                        z88_eprom_flash_get_file_name(&file,buffer_nombre);

			if (buffer_nombre[0]=='.') buffer_nombre[0]='D'; //archivo borrado

			//printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
                        longitud=strlen(buffer_nombre)+1; //Agregar salto de linea
                        if (indice_buffer+longitud>max_texto-1) retorno=0;
                        else {
                                sprintf (&texto_buffer[indice_buffer],"%s\n",buffer_nombre);
                                indice_buffer +=longitud;
                        }

                }
        } while (retorno!=0);

        texto_buffer[indice_buffer]=0;

	//menu_generic_message_tooltip("Z88 Card Browser", 0, 0, 1, NULL, "%s", texto_buffer);
	zxvision_generic_message_tooltip("Z88 Card Browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_buffer);

        free(flash_file_memory);

}


void menu_z88_slot_card_browser_aux(int slot,char *texto_buffer,int max_texto)
{
        z88_dir dir;
        z88_eprom_flash_file file;

        z88_eprom_flash_find_init(&dir,slot);

        char buffer_nombre[Z88_MAX_CARD_FILENAME+1];

        int retorno;
        int longitud;


        int indice_buffer=0;

        do {
                retorno=z88_eprom_flash_find_next(&dir,&file);
                if (retorno) {
                        z88_eprom_flash_get_file_name(&file,buffer_nombre);

			if (buffer_nombre[0]=='.') buffer_nombre[0]='D'; //archivo borrado

			//printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
                        longitud=strlen(buffer_nombre)+1; //Agregar salto de linea
                        if (indice_buffer+longitud>max_texto-1) retorno=0;
                        else {
                                sprintf (&texto_buffer[indice_buffer],"%s\n",buffer_nombre);
                                indice_buffer +=longitud;
                        }

                }
        } while (retorno!=0);

        texto_buffer[indice_buffer]=0;

}

void menu_z88_slot_card_browser(MENU_ITEM_PARAMETERS)
{
#define MAX_TEXTO 4096
        char texto_buffer[MAX_TEXTO];

        int slot=valor_opcion;

        menu_z88_slot_card_browser_aux(slot,texto_buffer,MAX_TEXTO);


	//Si esta vacio
        if (texto_buffer[0]==0) {
                debug_printf(VERBOSE_ERR,"Card is empty");
                return;
        }

        //menu_generic_message_tooltip("Card browser", 0, 0, 1, NULL, "%s", texto_buffer);

	zxvision_generic_message_tooltip("Card Browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_buffer);		

}


void menu_z88_slot_copy_from_eprom(MENU_ITEM_PARAMETERS)
{
#define MAX_TEXTO 4096
	char texto_buffer[MAX_TEXTO];

	int slot=valor_opcion;

	menu_z88_slot_card_browser_aux(slot,texto_buffer,MAX_TEXTO);

	//Si esta vacio
	if (texto_buffer[0]==0) {
		debug_printf(VERBOSE_ERR,"Card is empty");
                return;
        }



	//printf ("archivos: %s\n",texto_buffer);
	generic_message_tooltip_return retorno_archivo;
	//menu_generic_message_tooltip("Select file", 0, 0, 1, &retorno_archivo, "%s", texto_buffer);
	zxvision_generic_message_tooltip("Select file" , 0 , 0, 0, 1, &retorno_archivo, 1,"%s", texto_buffer);

	//Si se sale con ESC
	if (retorno_archivo.estado_retorno==0) return;

       //        strcpy(retorno->texto_seleccionado,buffer_lineas[linea_final];
       //         retorno->linea_seleccionada=linea_final;
	//printf ("Seleccionado: (%s) linea: %d\n",retorno_archivo.texto_seleccionado,retorno_archivo.linea_seleccionada);
	//nombre archivo viene con espacio al final


	int longitud_nombre=strlen(retorno_archivo.texto_seleccionado);
	if (longitud_nombre) {
		if (retorno_archivo.texto_seleccionado[longitud_nombre-1] == ' ') {
			retorno_archivo.texto_seleccionado[longitud_nombre-1]=0;
		}
	}



        z88_dir dir;
        z88_eprom_flash_file file;


	z88_find_eprom_flash_file (&dir,&file,retorno_archivo.texto_seleccionado, slot);


	//con el puntero a dir, retornamos file

	//printf ("dir: %x bank: %x\n",dir.dir,dir.bank);

	//z88_return_eprom_flash_file (&dir,&file);

	//Si no es archivo valido
	if (file.namelength==0 || file.namelength==255) {
		debug_printf (VERBOSE_ERR,"File not found");
		return;
	}


	//Grabar archivo en disco. Supuestamente le damos el mismo nombre evitando la / inicial

	//Con selector de archivos
	char nombredestino[PATH_MAX];

	char *filtros[2];

        filtros[0]="";
        filtros[1]=0;


        if (menu_filesel("Select Target File",filtros,nombredestino)==0) return;


	//Ver si archivo existe
	if (si_existe_archivo(nombredestino)) {
		if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;
	}




	//Hacer que dir apunte a los datos
	dir.bank=file.datos.bank;
	dir.dir=file.datos.dir;

	z80_byte byte_leido;

                                FILE *ptr_file_save;
                                  ptr_file_save=fopen(nombredestino,"wb");
                                  if (!ptr_file_save)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",nombredestino);

                                  }
                                else {
					unsigned int i;
					z80_long_int size=file.size[0]+(file.size[1]*256)+(file.size[2]*65536)+(file.size[3]*16777216);

					for (i=0;i<size;i++) {
						byte_leido=peek_byte_no_time_z88_bank_no_check_low(dir.dir,dir.bank);
						z88_increment_pointer(&dir);
						fwrite(&byte_leido,1,1,ptr_file_save);
					}


                                  fclose(ptr_file_save);
				}
	menu_generic_message("Copy File","OK. File copied from Card to Disk");




}


//menu z88 slots
void menu_z88_slots(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_z88_slots;
        menu_item item_seleccionado;
        int retorno_menu;

        do {

		menu_add_item_menu_inicial_format(&array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_insert,NULL,"ROM: %d Kb RAM: %d Kb",(z88_internal_rom_size+1)/1024,(z88_internal_ram_size+1)/1024);
                menu_add_item_menu_tooltip(array_menu_z88_slots,"Internal ROM and RAM");
                menu_add_item_menu_ayuda(array_menu_z88_slots,"Internal RAM can be maximum 512 KB. Internal ROM can be changed from \n"
					"Machine Menu->Custom Machine, and can also be maximum 512 KB");

                //establecemos numero slot como opcion de ese item de menu
                menu_add_item_menu_valor_opcion(array_menu_z88_slots,0);



		int slot;

		//int eprom_flash_invalida_en_slot_3=0;

		for (slot=1;slot<=3;slot++) {

			int eprom_flash_valida=0;
			int tipo_tarjeta=-1;
			char *tipos_tarjeta[]={"APP","FIL","MIX"};
			int type;

			//Si no hay slot insertado
			if (z88_memory_slots[slot].size==0) {
				menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_insert,NULL,"Empty");
			}

			else {
				type=z88_memory_slots[slot].type;

				//Si es flash/eprom en slot de escritura(3), indicar a que archivo hace referencia
				if (slot==3 && (type==2 || type==3 || type==4) ) {
					char string_writable_card_shown[18];
					menu_tape_settings_trunc_name(z88_memory_slots[slot].eprom_flash_nombre_archivo,string_writable_card_shown,18);
					menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_insert,NULL,"%s: %s",z88_memory_types[type],string_writable_card_shown);
				}
				else {
					menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_insert,NULL,"%s",z88_memory_types[type]);
				}
			}

			//establecemos numero slot como opcion de ese item de menu
			menu_add_item_menu_valor_opcion(array_menu_z88_slots,slot);
	                menu_add_item_menu_tooltip(array_menu_z88_slots,"Type of memory card if present and file name in case of slot 3 and EPROM/Flash cards");
        	        menu_add_item_menu_ayuda(array_menu_z88_slots,"Type of memory card if present and file name in case of slot 3 and EPROM/Flash cards.\n"
						"Slot 1, 2 or 3 can contain "
						"EPROM or Intel Flash cards. But EPROMS and Flash cards can only be written on slot 3\n"
						"EPROM/Flash cards files on slots 1,2 are only used at insert time and loaded on Z88 memory\n"
						"EPROM/Flash cards files on slot 3 are loaded on Z88 memory at insert time but are written everytime \n"
						"a change is made on this Z88 memory, so, they are always used when they are inserted\n"
						"\n"
						"Flash card files and eprom files are internally compatible, but eprom size maximum is 256 Kb "
						"and flash minimum size is 512 Kb, so you can not load an eprom file as a flash or viceversa\n"
						);
						//"Note: if you use OZ rom 4.3.1 or higher, flash cards are recognized well; if you use a lower "
						//"version, flash cards are recognized as eprom cards");

			if (z88_memory_slots[slot].size!=0) {

				int size=(z88_memory_slots[slot].size+1)/1024;

				//Si es EPROM o Flash, decir espacio libre, detectando si hay una tarjeta con filesystem
				char string_info_tarjeta[40];
				if (type==2 || type==3 || type==4) {
					z80_long_int total_eprom,used_eprom, free_eprom;
					tipo_tarjeta=z88_return_card_type(slot);
					debug_printf (VERBOSE_DEBUG,"Card type: %d",tipo_tarjeta);

					if (tipo_tarjeta>0) {

						//No buscar filesystem en caso de tarjeta hibrida
						if (type==4) {
							sprintf (string_info_tarjeta," (%s) %d kb Free Unkn",tipos_tarjeta[tipo_tarjeta],size);
                                                }

						else {

							z88_eprom_flash_free(&total_eprom,&used_eprom, &free_eprom,slot);

							//Controlar si eprom corrupta, tamanyos logicos
							if (free_eprom>1024*1024) {
								sprintf (string_info_tarjeta," (%s) %d kb Free Unkn",tipos_tarjeta[tipo_tarjeta],size);
							}

							else {
								sprintf (string_info_tarjeta," (%s) %d K Free %d K",tipos_tarjeta[tipo_tarjeta],size,free_eprom/1024);
								eprom_flash_valida=1;
							}
						}
					}

					else {
						//0 o -1
						if (tipo_tarjeta==0) {
							sprintf (string_info_tarjeta," (%s) %d kb",tipos_tarjeta[tipo_tarjeta],size);
						}
						else {
							sprintf (string_info_tarjeta," (Unk) %d kb",size);
						}
					}
				}

				else {
					sprintf (string_info_tarjeta," %d K",size);
				}

				menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,NULL,NULL,"%s",string_info_tarjeta);
				menu_add_item_menu_tooltip(array_menu_z88_slots,"Card Information");
				menu_add_item_menu_ayuda(array_menu_z88_slots,"Size of Card, and in case of EPROM/Flash cards: \n"
							"-Type of Card: Applications, Files or Unknown type\n"
							"-Space Available, in case of Files Card\n");


			}

			//Si hay una eprom en slot 3, dar opcion de borrar
			if (slot==3 && z88_memory_slots[3].size!=0 && (z88_memory_slots[3].type==2 || z88_memory_slots[3].type==3) ) {
				menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_erase_eprom_flash,NULL," Erase Card");
	                        menu_add_item_menu_tooltip(array_menu_z88_slots,"Card can only be erased on slot 3");
        	                menu_add_item_menu_ayuda(array_menu_z88_slots,"Card can only be erased on slot 3");
			}



			if (eprom_flash_valida && tipo_tarjeta>0) {

				if (slot==3) {

					menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_eprom_flash_reclaim_free_space,NULL," Reclaim Free Space");
					menu_add_item_menu_tooltip(array_menu_z88_slots,"It reclaims the space used by deleted files");
					menu_add_item_menu_ayuda(array_menu_z88_slots,"It reclaims the space used by deleted files");

					menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_eprom_flash_undelete_files,NULL," Undelete Files");
					menu_add_item_menu_tooltip(array_menu_z88_slots,"Undelete deleted files");
					menu_add_item_menu_ayuda(array_menu_z88_slots,"Undelete deleted files");


					menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_copy_to_eprom_flash,NULL," Copy to Card");
					menu_add_item_menu_tooltip(array_menu_z88_slots,"Copy files from your hard drive to Card");
					menu_add_item_menu_ayuda(array_menu_z88_slots,"Copy files from your hard drive to Card. "
						"Card card must be initialized before you can copy files to it, "
						"I mean, if it's a new card, it must be erased and Z88 Filer must know it; usually it is needed to copy "
						"at least one file from Filer");
				}




				menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_card_browser,NULL," Card browser");
				menu_add_item_menu_tooltip(array_menu_z88_slots,"Browse card");
				menu_add_item_menu_ayuda(array_menu_z88_slots,"Browse card");
	                        //establecemos numero slot como opcion de ese item de menu
        	                menu_add_item_menu_valor_opcion(array_menu_z88_slots,slot);

				menu_add_item_menu_format(array_menu_z88_slots,MENU_OPCION_NORMAL,menu_z88_slot_copy_from_eprom,NULL," Copy from Card");
				menu_add_item_menu_tooltip(array_menu_z88_slots,"Copy files from Card to your hard drive");
				menu_add_item_menu_ayuda(array_menu_z88_slots,"Copy files from Card to your hard drive");
	                        //establecemos numero slot como opcion de ese item de menu
        	                menu_add_item_menu_valor_opcion(array_menu_z88_slots,slot);


			}

		}


                menu_add_item_menu(array_menu_z88_slots,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_ESC_item(array_menu_z88_slots);

                retorno_menu=menu_dibuja_menu(&z88_slots_opcion_seleccionada,&item_seleccionado,array_menu_z88_slots,"Z88 Memory Slots" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);


}




//Retorna 0 si ok
//Retorna -1 si fuera de rango
//Modifica valor de variable
int menu_hardware_advanced_input_value(int minimum,int maximum,char *texto,int *variable)
{

	int valor;

        char string_value[4];

        sprintf (string_value,"%d",*variable);


        menu_ventana_scanf(texto,string_value,4);

        valor=parse_string_to_number(string_value);

	if (valor<minimum || valor>maximum) {
		debug_printf (VERBOSE_ERR,"Value out of range. Minimum: %d Maximum: %d",minimum,maximum);
		return -1;
	}

	*variable=valor;
	return 0;


}

void menu_hardware_advanced_reload_display(void)
{

		screen_testados_linea=screen_total_borde_izquierdo/2+128+screen_total_borde_derecho/2+screen_invisible_borde_derecho/2;

	        //Recalcular algunos valores cacheados
	        recalcular_get_total_ancho_rainbow();
        	recalcular_get_total_alto_rainbow();

                screen_set_video_params_indices();
                inicializa_tabla_contend();

                init_rainbow();
                init_cache_putpixel();
}



void menu_hardware_advanced_hidden_top_border(MENU_ITEM_PARAMETERS)
{
	int max,min;
	min=7;
	max=16;
	if (MACHINE_IS_PRISM) {
		min=32;
		max=45;
	}

	if (menu_hardware_advanced_input_value(min,max,"Hidden top Border",&screen_invisible_borde_superior)==0) {
		menu_hardware_advanced_reload_display();
	}
}

void menu_hardware_advanced_visible_top_border(MENU_ITEM_PARAMETERS)
{

	int max,min;
	min=32;
	max=56;
	if (MACHINE_IS_PRISM) {
		min=32;
		max=48;
	}

        if (menu_hardware_advanced_input_value(min,max,"Visible top Border",&screen_borde_superior)==0) {
                menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_visible_bottom_border(MENU_ITEM_PARAMETERS)
{
	int max,min;
	min=48;
	max=56;
	if (MACHINE_IS_PRISM) {
		min=32;
                max=48;
        }



        if (menu_hardware_advanced_input_value(min,max,"Visible bottom Border",&screen_total_borde_inferior)==0) {
                menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_borde_izquierdo(MENU_ITEM_PARAMETERS)
{

	int valor_pixeles;

	valor_pixeles=screen_total_borde_izquierdo/2;

        int max,min;
	min=12;
	max=24;
	if (MACHINE_IS_PRISM) {
                min=20;
		max=32;
	}

	if (menu_hardware_advanced_input_value(min,max,"Left Border TLength",&valor_pixeles)==0) {
		screen_total_borde_izquierdo=valor_pixeles*2;
		 menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_borde_derecho(MENU_ITEM_PARAMETERS)
{

        int valor_pixeles;

	valor_pixeles=screen_total_borde_derecho/2;

        int max,min;
	min=12;
        max=24;
	if (MACHINE_IS_PRISM) {
                min=20;
                max=32;
        }

        if (menu_hardware_advanced_input_value(min,max,"Right Border TLength",&valor_pixeles)==0) {
                screen_total_borde_derecho=valor_pixeles*2;
                 menu_hardware_advanced_reload_display();
        }
}

void menu_hardware_advanced_hidden_borde_derecho(MENU_ITEM_PARAMETERS)
{

        int valor_pixeles;

	valor_pixeles=screen_invisible_borde_derecho/2;

        int max,min;
	min=24;
	max=52;

	if (MACHINE_IS_PRISM) {
                min=60;
                max=79;
        }

        if (menu_hardware_advanced_input_value(min,max,"Right Hidden B.TLength",&valor_pixeles)==0) {
                screen_invisible_borde_derecho=valor_pixeles*2;
                 menu_hardware_advanced_reload_display();
        }
}


void menu_ula_advanced(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_advanced;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
                menu_add_item_menu_inicial_format(&array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_hidden_top_border,NULL,"[%2d] Hidden Top Border",screen_invisible_borde_superior);

                menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_visible_top_border,NULL,"[%d] Visible Top Border",screen_borde_superior);
                menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_visible_bottom_border,NULL,"[%d] Visible Bottom Border",screen_total_borde_inferior);

		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_borde_izquierdo,NULL,"[%d] Left Border TLength",screen_total_borde_izquierdo/2);
		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_borde_derecho,NULL,"[%d] Right Border TLength",screen_total_borde_derecho/2);

		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,menu_hardware_advanced_hidden_borde_derecho,NULL,"[%d] Right Hidden B. TLength",screen_invisible_borde_derecho/2);


                menu_add_item_menu(array_menu_hardware_advanced,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Info:");
		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total line TLength: %d",screen_testados_linea);
		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total scanlines: %d",screen_scanlines);
		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total T-states: %d",screen_testados_total);
		menu_add_item_menu_format(array_menu_hardware_advanced,MENU_OPCION_NORMAL,NULL,NULL,"Total Hz: %d",screen_testados_total*50);

                menu_add_item_menu(array_menu_hardware_advanced,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_advanced,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_advanced);

                retorno_menu=menu_dibuja_menu(&hardware_advanced_opcion_seleccionada,&item_seleccionado,array_menu_hardware_advanced,"Advanced Timing Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

void menu_hardware_printers_zxprinter_enable(MENU_ITEM_PARAMETERS)
{
	zxprinter_enabled.v ^=1;

	if (zxprinter_enabled.v==0) {
	        close_zxprinter_bitmap_file();
        	close_zxprinter_ocr_file();
	}

}

int menu_hardware_zxprinter_cond(void)
{
	return zxprinter_enabled.v;
}

void menu_hardware_zxprinter_bitmapfile(MENU_ITEM_PARAMETERS)
{


	close_zxprinter_bitmap_file();

        char *filtros[3];

        filtros[0]="txt";
        filtros[1]="pbm";
        filtros[2]=0;


        if (menu_filesel("Select Bitmap File",filtros,zxprinter_bitmap_filename_buffer)==1) {
                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(zxprinter_bitmap_filename_buffer, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }

                zxprinter_bitmap_filename=zxprinter_bitmap_filename_buffer;

		zxprinter_file_bitmap_init();


        }

//        else {
//		close_zxprinter_file();
//        }
}

void menu_hardware_zxprinter_ocrfile(MENU_ITEM_PARAMETERS)
{


        close_zxprinter_ocr_file();

        char *filtros[2];

        filtros[0]="txt";
        filtros[1]=0;


        if (menu_filesel("Select OCR File",filtros,zxprinter_ocr_filename_buffer)==1) {
                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(zxprinter_ocr_filename_buffer, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }

                zxprinter_ocr_filename=zxprinter_ocr_filename_buffer;

                zxprinter_file_ocr_init();


        }

//        else {
//              close_zxprinter_file();
//        }
}


void menu_hardware_zxprinter_copy(MENU_ITEM_PARAMETERS)
{
        push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);

	if (MACHINE_IS_SPECTRUM) {
	        reg_pc=0x0eac;
	}

	if (MACHINE_IS_ZX81) {
		reg_pc=0x0869;
	}


	if (menu_multitarea) menu_generic_message("COPY","OK. COPY executed");
	else menu_generic_message("COPY","Register PC set to the COPY routine. Return to the emulator to let the COPY routine to be run");


}



void menu_hardware_printers(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_printers;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
                menu_add_item_menu_inicial_format(&array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_printers_zxprinter_enable,NULL,"[%c] ZX Printer",(zxprinter_enabled.v==1 ? 'X' : ' ' ));
		menu_add_item_menu_tooltip(array_menu_hardware_printers,"Enables or disables ZX Printer emulation");
		menu_add_item_menu_ayuda(array_menu_hardware_printers,"You must set it to off when finishing printing to close generated files");

                char string_bitmapfile_shown[16];
                menu_tape_settings_trunc_name(zxprinter_bitmap_filename,string_bitmapfile_shown,16);

                menu_add_item_menu_format(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_bitmapfile,menu_hardware_zxprinter_cond,"Bitmap file [%s]",string_bitmapfile_shown);

                menu_add_item_menu_tooltip(array_menu_hardware_printers,"Sends printer output to image file");
                menu_add_item_menu_ayuda(array_menu_hardware_printers,"Printer output is saved to a image file. Supports pbm file format, and "
					"also supports text file, "
					"where every pixel is a character on text. "
					"It is recommended to close the image file when finishing printing, so its header is updated");


		char string_ocrfile_shown[19];
		menu_tape_settings_trunc_name(zxprinter_ocr_filename,string_ocrfile_shown,19);

                menu_add_item_menu_format(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_ocrfile,menu_hardware_zxprinter_cond,"OCR file [%s]",string_ocrfile_shown);

                menu_add_item_menu_tooltip(array_menu_hardware_printers,"Sends printer output to text file using OCR method");
                menu_add_item_menu_ayuda(array_menu_hardware_printers,"Printer output is saved to a text file using OCR method to guess text. "
					"If you cancel a printing with SHIFT+SPACE on Basic, you have to re-select the ocr file to reset some "
					"internal counters. If you don't do that, OCR will not work");


		menu_add_item_menu_format(array_menu_hardware_printers,MENU_OPCION_NORMAL,menu_hardware_zxprinter_copy,menu_hardware_zxprinter_cond,"Run COPY routine");

                menu_add_item_menu_tooltip(array_menu_hardware_printers,"Runs ROM COPY routine");
		menu_add_item_menu_ayuda(array_menu_hardware_printers,"It calls ROM copy routine on Spectrum and ZX-81, like the COPY command on BASIC. \n"
					"I did not guarantee that the call will always work, because this function will probably "
					"use some structures and variables needed in BASIC and if you are running some game, maybe it "
					"has not these variables correct");



                menu_add_item_menu(array_menu_hardware_printers,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_printers,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_printers);

                retorno_menu=menu_dibuja_menu(&hardware_printers_opcion_seleccionada,&item_seleccionado,array_menu_hardware_printers,"Printing emulation" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_hardware_keyboard_issue(MENU_ITEM_PARAMETERS)
{
	keyboard_issue2.v^=1;
}

void menu_hardware_azerty(MENU_ITEM_PARAMETERS)
{
	azerty_keyboard.v ^=1;
}


void menu_chloe_keyboard(MENU_ITEM_PARAMETERS)
{
	chloe_keyboard.v ^=1;
	scr_z88_cpc_load_keymap();
}

void menu_hardware_redefine_keys_set_keys(MENU_ITEM_PARAMETERS)
{


        z80_byte tecla_original,tecla_redefinida;
	tecla_original=lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original;
	tecla_redefinida=lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_redefinida;

        char buffer_caracter_original[2];
	char buffer_caracter_redefinida[2];

	if (tecla_original==0) {
		buffer_caracter_original[0]=0;
		buffer_caracter_redefinida[0]=0;
	}


	else {
		buffer_caracter_original[0]=(tecla_original>=32 && tecla_original <=127 ? tecla_original : '?');
		buffer_caracter_redefinida[0]=(tecla_redefinida>=32 && tecla_redefinida <=127 ? tecla_redefinida : '?');
	}

        buffer_caracter_original[1]=0;
        buffer_caracter_redefinida[1]=0;




        menu_ventana_scanf("Original key",buffer_caracter_original,2);
        tecla_original=buffer_caracter_original[0];

	if (tecla_original==0) {
		lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=0;
		return;
	}


        menu_ventana_scanf("Destination key",buffer_caracter_redefinida,2);
        tecla_redefinida=buffer_caracter_redefinida[0];

	if (tecla_redefinida==0) {
                lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=0;
                return;
        }


	lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_original=tecla_original;
	lista_teclas_redefinidas[hardware_redefine_keys_opcion_seleccionada].tecla_redefinida=tecla_redefinida;

}




void menu_hardware_redefine_keys(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_redefine_keys;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_TECLAS_REDEFINIDAS;i++) {
				z80_byte tecla_original=lista_teclas_redefinidas[i].tecla_original;
				z80_byte tecla_redefinida=lista_teclas_redefinidas[i].tecla_redefinida;
                        if (tecla_original) {
					sprintf(buffer_texto,"Key %c to %c",(tecla_original>=32 && tecla_original <=127 ? tecla_original : '?'),
					(tecla_redefinida>=32 && tecla_redefinida <=127 ? tecla_redefinida : '?') );

								}

                        else {
                                sprintf(buffer_texto,"Unused entry");
                        }



                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_redefine_keys,MENU_OPCION_NORMAL,menu_hardware_redefine_keys_set_keys,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_redefine_keys,MENU_OPCION_NORMAL,menu_hardware_redefine_keys_set_keys,NULL,buffer_texto);


                        menu_add_item_menu_tooltip(array_menu_hardware_redefine_keys,"Redefine the key");
                        menu_add_item_menu_ayuda(array_menu_hardware_redefine_keys,"Indicates which key on the Spectrum keyboard is sent when "
                                                "pressed the original key");
                }



                menu_add_item_menu(array_menu_hardware_redefine_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_redefine_keys,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_redefine_keys);

                retorno_menu=menu_dibuja_menu(&hardware_redefine_keys_opcion_seleccionada,&item_seleccionado,array_menu_hardware_redefine_keys,"Redefine keys" );

                


if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_hardware_inves_poke(MENU_ITEM_PARAMETERS)
{


	char string_poke[4];

	sprintf (string_poke,"%d",last_inves_low_ram_poke_menu);


	menu_ventana_scanf("Poke low RAM with",string_poke,4);

	last_inves_low_ram_poke_menu=parse_string_to_number(string_poke);

	modificado_border.v=1;

	poke_inves_rom(last_inves_low_ram_poke_menu);

}

int menu_inves_cond(void)
{
        if (MACHINE_IS_INVES) return 1;
        else return 0;
}

int menu_inves_cond_realvideo(void)
{
	if (!menu_inves_cond()) return 0;
	return rainbow_enabled.v;

}


void menu_hardware_ace_ramtop(MENU_ITEM_PARAMETERS)
{


        char string_ramtop[3];

        //int valor_antes_ramtop=((ramtop_ace-16383)/1024)+3;
				int valor_antes_ramtop=get_ram_ace();

	//printf ("ramtop: %d\n",valor_antes_ramtop);

        sprintf (string_ramtop,"%d",valor_antes_ramtop);


        menu_ventana_scanf("RAM? (3, 19 or 35)",string_ramtop,3);

        int valor_leido_ramtop=parse_string_to_number(string_ramtop);

        //si mismo valor volver
        if (valor_leido_ramtop==valor_antes_ramtop) return;

	if (valor_leido_ramtop!=3 && valor_leido_ramtop!=19 && valor_leido_ramtop!=35) {
		debug_printf (VERBOSE_ERR,"Invalid RAM value");
		return;
	}


	set_ace_ramtop(valor_leido_ramtop);
        reset_cpu();

}



void menu_hardware_zx8081_ramtop(MENU_ITEM_PARAMETERS)
{


        char string_ramtop[3];

	//int valor_antes_ramtop=(ramtop_zx8081-16383)/1024;
	int valor_antes_ramtop=zx8081_get_standard_ram();

        sprintf (string_ramtop,"%d",valor_antes_ramtop);


        menu_ventana_scanf("Standard RAM",string_ramtop,3);

        int valor_leido_ramtop=parse_string_to_number(string_ramtop);

	//si mismo valor volver
	if (valor_leido_ramtop==valor_antes_ramtop) return;


	if (valor_leido_ramtop>0 && valor_leido_ramtop<17) {
		set_zx8081_ramtop(valor_leido_ramtop);
		//ramtop_zx8081=16383+1024*valor_leido_ramtop;
		reset_cpu();
	}

}

void menu_hardware_zx8081_ram_in_8192(MENU_ITEM_PARAMETERS)
{
	ram_in_8192.v ^=1;
}

void menu_hardware_zx8081_ram_in_49152(MENU_ITEM_PARAMETERS)
{
        if (ram_in_49152.v==0) enable_ram_in_49152();
        else ram_in_49152.v=0;

}

void menu_hardware_zx8081_ram_in_32768(MENU_ITEM_PARAMETERS)
{

	if (ram_in_32768.v==0) enable_ram_in_32768();
	else {
		ram_in_32768.v=0;
		ram_in_49152.v=0;
	}

}





//OLD: Solo permitimos autofire para Kempston, Fuller ,Zebra y mikrogen, para evitar que con cursor o con sinclair se este mandando una tecla y dificulte moverse por el menu
int menu_hardware_autofire_cond(void)
{
	//if (joystick_emulation==JOYSTICK_FULLER || joystick_emulation==JOYSTICK_KEMPSTON || joystick_emulation==JOYSTICK_ZEBRA || joystick_emulation==JOYSTICK_MIKROGEN) return 1;
	//return 0;

	return 1;
}


void menu_hardware_realjoystick_event_button(MENU_ITEM_PARAMETERS)
{
        menu_simple_ventana("Redefine event","Please press the button/axis");
	menu_refresca_pantalla();

	//Para xwindows hace falta esto, sino no refresca
	 scr_actualiza_tablas_teclado();

        //redefinir evento
        if (!realjoystick_redefine_event(hardware_realjoystick_event_opcion_seleccionada)) {
		//se ha salido con tecla. ver si es ESC
		if ((puerto_especial1&1)==0) {
			//desasignar evento
			realjoystick_events_array[hardware_realjoystick_event_opcion_seleccionada].asignado.v=0;
		}
	}

        cls_menu_overlay();
}


//Retorna <0 si salir con ESC
int menu_joystick_event_list(void)
{
      
        menu_item *array_menu_joystick_event_list;
        menu_item item_seleccionado;
        int retorno_menu;

        int joystick_event_list_opcion_seleccionada=0;
       

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {

                  //enum defined_f_function_ids accion=defined_f_functions_keys_array[i];

                  sprintf (buffer_texto,"%s",realjoystick_event_names[i]);


                    if (i==0) menu_add_item_menu_inicial_format(&array_menu_joystick_event_list,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
                     else menu_add_item_menu_format(array_menu_joystick_event_list,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

				}



                menu_add_item_menu(array_menu_joystick_event_list,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_joystick_event_list);

                retorno_menu=menu_dibuja_menu(&joystick_event_list_opcion_seleccionada,&item_seleccionado,array_menu_joystick_event_list,"Select event" );

                


								if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

												//Si se pulsa Enter
												return joystick_event_list_opcion_seleccionada;
			
                                }

                                else return -1;

}



void menu_hardware_realjoystick_keys_button(MENU_ITEM_PARAMETERS)
{


	//int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2)
	int tipo=menu_simple_two_choices("Selection type","You want to set by","Button","Event");

	if (tipo==0) return; //ESC


        z80_byte caracter;

        char buffer_caracter[2];
        buffer_caracter[0]=0;

        menu_ventana_scanf("Please write the key",buffer_caracter,2);


        caracter=buffer_caracter[0];

        if (caracter==0) {
		//desasignamos
		realjoystick_keys_array[hardware_realjoystick_keys_opcion_seleccionada].asignado.v=0;
		return;
	}



	cls_menu_overlay();

	if (tipo==1) { //Definir por boton


        	menu_simple_ventana("Redefine key","Please press the button/axis");
        	menu_refresca_pantalla();

        	//Para xwindows hace falta esto, sino no refresca
        	scr_actualiza_tablas_teclado();

        	//redefinir boton a tecla
        	if (!realjoystick_redefine_key(hardware_realjoystick_keys_opcion_seleccionada,caracter)) {
			//se ha salido con tecla. ver si es ESC
        	        if ((puerto_especial1&1)==0) {
                	        //desasignar evento
				realjoystick_keys_array[hardware_realjoystick_keys_opcion_seleccionada].asignado.v=0;
                	}
        	}
        }

        if (tipo==2) { //Definir por evento
        	int evento=menu_joystick_event_list();
        	 realjoystick_copy_event_button_key(evento,hardware_realjoystick_keys_opcion_seleccionada,caracter);
        	//printf ("evento: %d\n",evento);
        }



}

void menu_print_text_axis(char *buffer,int button_type,int button_number)
{

	char buffer_axis[2];

	if (button_type==0) sprintf (buffer_axis,"%s","");
                                        //este sprintf se hace asi para evitar warnings al compilar

	if (button_type<0) sprintf (buffer_axis,"-");
	if (button_type>0) sprintf (buffer_axis,"+");

	sprintf(buffer,"%s%d",buffer_axis,button_number);

}


void menu_hardware_realjoystick_clear_keys(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno_texto("Clear list","Sure?")==1) {
                realjoystick_clear_keys_array();
        }
}


void menu_hardware_realjoystick_keys(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_realjoystick_keys;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];
                char buffer_texto_boton[10];

                int i;
                for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
                        if (realjoystick_keys_array[i].asignado.v) {
				menu_print_text_axis(buffer_texto_boton,realjoystick_keys_array[i].button_type,realjoystick_keys_array[i].button);

				z80_byte c=realjoystick_keys_array[i].caracter;
				if (c>=32 && c<=126) sprintf (buffer_texto,"Button %s sends [%c]",buffer_texto_boton,c);
				else sprintf (buffer_texto,"Button %s sends [(%d)]",buffer_texto_boton,c);
                        }

                        else {
                                sprintf(buffer_texto,"Unused entry");
			}



                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys_button,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys_button,NULL,buffer_texto);


                        menu_add_item_menu_tooltip(array_menu_hardware_realjoystick_keys,"Redefine the button");
                        menu_add_item_menu_ayuda(array_menu_hardware_realjoystick_keys,"Indicates which key on the Spectrum keyboard is sent when "
						"pressed the button/axis on the real joystick");
                }

                menu_add_item_menu(array_menu_hardware_realjoystick_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_item_menu_format(array_menu_hardware_realjoystick_keys,MENU_OPCION_NORMAL,menu_hardware_realjoystick_clear_keys,NULL,"Clear list");


                menu_add_item_menu(array_menu_hardware_realjoystick_keys,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_realjoystick_keys,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_realjoystick_keys);

                retorno_menu=menu_dibuja_menu(&hardware_realjoystick_keys_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick_keys,"Joystick to key" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_hardware_realjoystick_clear_events(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Clear list","Sure?")==1) {
		realjoystick_clear_events_array();
	}
}


void menu_hardware_realjoystick_event(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_realjoystick_event;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];
                char buffer_texto_boton[10];

                int i;
                for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
                        if (realjoystick_events_array[i].asignado.v) {
				menu_print_text_axis(buffer_texto_boton,realjoystick_events_array[i].button_type,realjoystick_events_array[i].button);
                        }

                        else {
                                sprintf(buffer_texto_boton,"None");
                        }

                        sprintf (buffer_texto,"Button for %s [%s]",realjoystick_event_names[i],buffer_texto_boton);


                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event_button,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event_button,NULL,buffer_texto);


                        menu_add_item_menu_tooltip(array_menu_hardware_realjoystick_event,"Redefine the action");
                        menu_add_item_menu_ayuda(array_menu_hardware_realjoystick_event,"Redefine the action");
                }

                menu_add_item_menu(array_menu_hardware_realjoystick_event,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_item_menu_format(array_menu_hardware_realjoystick_event,MENU_OPCION_NORMAL,menu_hardware_realjoystick_clear_events,NULL,"Clear list");


                menu_add_item_menu(array_menu_hardware_realjoystick_event,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_realjoystick_event,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_realjoystick_event);

                retorno_menu=menu_dibuja_menu(&hardware_realjoystick_event_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick_event,"Joystick to event" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


#define REALJOYSTICK_TEST_X 1
#define REALJOYSTICK_TEST_Y 8
#define REALJOYSTICK_TEST_ANCHO 30
#define REALJOYSTICK_TEST_ALTO 6


void menu_hardware_realjoystick_test_reset_last_values(void)
{
	realjoystick_last_button=realjoystick_last_type=realjoystick_last_value=realjoystick_last_index=-1;
}


//Llena string de texto con barras =====|===== segun si valor es entre -32767  y 32768
//Valor 0:      -----|-----
//Valor 32767:  -----|=====
//Valor -32767: =====|-----
//limite_barras dice cuantas barras muestra hacia la derecha o izquierda
void menu_hardware_realjoystick_test_fill_bars(int valor,char *string,int limite_barras)
{
	//Limitar valor entre -32767 y 32767
	if (valor>32767) valor=32767;
	if (valor<-32767) valor=-32767;

	//Cuantas barras hay que hacer
	int barras=(valor*limite_barras)/32767;

	if (barras<0) barras=-barras;

	//String inicial
	int i;
	for (i=0;i<limite_barras;i++) {
		string[i]='-';
		string[i+limite_barras+1]='-';
	}

	//Medio
	string[i]='|';

	//Final
	string[i+limite_barras+1]=0;


	//Y ahora llenar hacia la izquierda o derecha
	int signo=+1;
	if (valor<0) signo=-1;
	int indice=limite_barras+signo; //Nos posicionamos a la derecha o izquierda de la barra central
	for (;barras;barras--) {
		string[indice]='=';
		indice +=signo;
	}

}

void menu_hardware_realjoystick_test(MENU_ITEM_PARAMETERS)
{

        menu_espera_no_tecla();
    

	zxvision_window ventana;

	zxvision_new_window(&ventana,REALJOYSTICK_TEST_X,REALJOYSTICK_TEST_Y-1,REALJOYSTICK_TEST_ANCHO,REALJOYSTICK_TEST_ALTO+3,
							REALJOYSTICK_TEST_ANCHO-1,REALJOYSTICK_TEST_ALTO+3-2,"Joystick test");
	zxvision_draw_window(&ventana);			



        z80_byte acumulado;



                               int valor_contador_segundo_anterior;

                                valor_contador_segundo_anterior=contador_segundo;

	menu_hardware_realjoystick_test_reset_last_values();

	int salir_por_boton=0;


        do {

		menu_cpu_core_loop();
                acumulado=menu_da_todas_teclas();

		//si no hay multitarea, pausar
		if (menu_multitarea==0) {
			usleep(20000); //20 ms
		}


		//Si es evento de salir, forzar el mostrar la info y luego salir
		if (realjoystick_last_button>=0 && realjoystick_last_index==REALJOYSTICK_EVENT_ESC_MENU) {
			//printf ("Salir por boton\n");
			salir_por_boton=1;
		}

        if ( ((contador_segundo%100) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0 || salir_por_boton) {
                                                                        valor_contador_segundo_anterior=contador_segundo;
                                                                        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
                       if (menu_multitarea==0) menu_refresca_pantalla();






                        char buffer_texto_medio[40];

			int linea=0;
			//int realjoystick_last_button,realjoystick_last_type,realjoystick_last_value,realjoystick_last_index;
				//menu_escribe_linea_opcion(linea++,-1,1,"Last joystick button/axis:");
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Last joystick button/axis:");
				linea++;





			if (realjoystick_last_button>=0) {

				char buffer_type[40];
#define LONGITUD_BARRAS 6
				char fill_bars[(LONGITUD_BARRAS*2)+2];
				fill_bars[0]=0;
				if (realjoystick_last_type==JS_EVENT_BUTTON) {
					strcpy(buffer_type,"Button");
				}
				else if (realjoystick_last_type==JS_EVENT_AXIS) {
					strcpy(buffer_type,"Axis");
					menu_hardware_realjoystick_test_fill_bars(realjoystick_last_value,fill_bars,LONGITUD_BARRAS);
				}
				else strcpy(buffer_type,"Unknown");

				sprintf (buffer_texto_medio,"Button: %d",realjoystick_last_button);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_texto_medio);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);

				sprintf (buffer_texto_medio,"Type: %d (%s)",realjoystick_last_type,buffer_type);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_texto_medio);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);


				//type JS_EVENT_BUTTON, JS_EVENT_AXIS


				char buffer_event[40];
				if (realjoystick_last_index>=0 && realjoystick_last_index<MAX_EVENTS_JOYSTICK) {
					strcpy(buffer_event,realjoystick_event_names[realjoystick_last_index]);
				}
				else {
					strcpy(buffer_event,"None");
				}

	
				sprintf (buffer_texto_medio,"Value: %6d %s",realjoystick_last_value,fill_bars);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_texto_medio);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);

				sprintf (buffer_texto_medio,"Index: %d Event: %s",realjoystick_last_index,buffer_event);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_texto_medio);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_texto_medio);


				//realjoystick_ultimo_indice=-1;
				menu_hardware_realjoystick_test_reset_last_values();

				zxvision_draw_window_contents(&ventana);


			}
        }

		

        //Hay tecla pulsada
            if ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) !=MENU_PUERTO_TECLADO_NINGUNA ) {
                                int tecla=menu_get_pressed_key();

                                                                                                                                
				//Si tecla no es ESC, no salir
                                
				if (tecla!=2) {
					acumulado = MENU_PUERTO_TECLADO_NINGUNA;
				}


				//Si ha salido por boton de joystick, esperar evento
				if (salir_por_boton) {
                       			if (menu_multitarea==0) menu_refresca_pantalla();
					menu_espera_no_tecla();
				}
				
                                                                        
			}


        } while ( (acumulado & MENU_PUERTO_TECLADO_NINGUNA) ==MENU_PUERTO_TECLADO_NINGUNA);


	cls_menu_overlay();
	zxvision_destroy_window(&ventana);

}


void menu_hardware_realjoystick(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_realjoystick;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                menu_add_item_menu_inicial_format(&array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_event,NULL,"Joystick to events");

                menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Define which events generate every button/movement of the joystick");
                menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Define which events generate every button/movement of the joystick");



		menu_add_item_menu_format(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_keys,NULL,"Joystick to keys");

                menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Define which press key generate every button/movement of the joystick");
                menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Define which press key generate every button/movement of the joystick");


		menu_add_item_menu_format(array_menu_hardware_realjoystick,MENU_OPCION_NORMAL,menu_hardware_realjoystick_test,NULL,"Test joystick");
		menu_add_item_menu_tooltip(array_menu_hardware_realjoystick,"Test joystick buttons");
		menu_add_item_menu_ayuda(array_menu_hardware_realjoystick,"Test joystick buttons");




                menu_add_item_menu(array_menu_hardware_realjoystick,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_realjoystick,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_realjoystick);

                retorno_menu=menu_dibuja_menu(&hardware_realjoystick_opcion_seleccionada,&item_seleccionado,array_menu_hardware_realjoystick,"Real joystick emulation" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

void menu_hardware_joystick(MENU_ITEM_PARAMETERS)
{
	joystick_cycle_next_type();
}

void menu_hardware_gunstick(MENU_ITEM_PARAMETERS)
{
        if (gunstick_emulation==GUNSTICK_TOTAL) gunstick_emulation=0;
	else gunstick_emulation++;
}

void menu_hardware_gunstick_range_x(MENU_ITEM_PARAMETERS)
{
	if (gunstick_range_x==256) gunstick_range_x=1;
	else gunstick_range_x *=2;
}

void menu_hardware_gunstick_range_y(MENU_ITEM_PARAMETERS)
{
        if (gunstick_range_y==64) gunstick_range_y=1;
        else gunstick_range_y *=2;
}

void menu_hardware_gunstick_y_offset(MENU_ITEM_PARAMETERS)
{
	if (gunstick_y_offset==32) gunstick_y_offset=0;
	else gunstick_y_offset +=4;
}

void menu_hardware_gunstick_solo_brillo(MENU_ITEM_PARAMETERS)
{
	gunstick_solo_brillo ^=1;
}

int menu_hardware_gunstick_aychip_cond(void)
{
	if (gunstick_emulation==GUNSTICK_AYCHIP) return 1;
	else return 0;
}

void menu_ula_contend(MENU_ITEM_PARAMETERS)
{
	contend_enabled.v ^=1;

	inicializa_tabla_contend();

}


void menu_ula_late_timings(MENU_ITEM_PARAMETERS)
{
	ula_late_timings.v ^=1;
	inicializa_tabla_contend();
}


void menu_ula_im2_slow(MENU_ITEM_PARAMETERS)
{
        ula_im2_slow.v ^=1;
}


void menu_ula_pentagon_timing(MENU_ITEM_PARAMETERS)
{
	if (pentagon_timing.v) {
		contend_enabled.v=1;
		ula_disable_pentagon_timing();
	}

	else {
		contend_enabled.v=0;
		ula_enable_pentagon_timing();
	}


}


void menu_hardware_autofire(MENU_ITEM_PARAMETERS)
{
	if (joystick_autofire_frequency==0) joystick_autofire_frequency=1;
	else if (joystick_autofire_frequency==1) joystick_autofire_frequency=2;
	else if (joystick_autofire_frequency==2) joystick_autofire_frequency=5;
	else if (joystick_autofire_frequency==5) joystick_autofire_frequency=10;
	else if (joystick_autofire_frequency==10) joystick_autofire_frequency=25;
	else if (joystick_autofire_frequency==25) joystick_autofire_frequency=50;

	else joystick_autofire_frequency=0;
}

void menu_hardware_kempston_mouse(MENU_ITEM_PARAMETERS)
{
	kempston_mouse_emulation.v ^=1;
}


int menu_hardware_realjoystick_cond(void)
{
	return realjoystick_present.v;
}

int menu_hardware_keyboard_issue_cond(void)
{
	if (MACHINE_IS_SPECTRUM) return 1;
	return 0;
}

void menu_hardware_keymap_z88_cpc(MENU_ITEM_PARAMETERS) {
	//solo hay dos tipos
	z88_cpc_keymap_type ^=1;
	scr_z88_cpc_load_keymap();
}


void menu_hardware_memory_refresh(MENU_ITEM_PARAMETERS)
{
	if (machine_emulate_memory_refresh==0) {
		set_peek_byte_function_ram_refresh();
		machine_emulate_memory_refresh=1;
	}

	else {
		reset_peek_byte_function_ram_refresh();
                machine_emulate_memory_refresh=0;
	}
}



void menu_zxuno_spi_persistent_writes(MENU_ITEM_PARAMETERS)
{
	if (zxuno_flash_persistent_writes.v==0) {
           if (menu_confirm_yesno_texto("Will flush prev. changes","Enable?")==1) zxuno_flash_persistent_writes.v=1;

	}

	else zxuno_flash_persistent_writes.v=0;
}

void menu_zxuno_spi_flash_file(MENU_ITEM_PARAMETERS)
{
	char *filtros[2];

        filtros[0]="flash";
        filtros[1]=0;


        if (menu_filesel("Select Flash File",filtros,zxuno_flash_spi_name)==1) {

                //Ver si archivo existe y preguntar
                /*if (si_existe_archivo(zxuno_flash_spi_name) ) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }
		*/

		if (si_existe_archivo(zxuno_flash_spi_name) ) {

			if (menu_confirm_yesno_texto("File exists","Reload SPI Flash from file?")) {

				//Decir que no hay que hacer flush anteriores
				zxuno_flash_must_flush_to_disk=0;

				//Y sobreescribir ram spi flash con lo que tiene el archivo de disco
				zxuno_load_spi_flash();
			}

		}

		else {

			//Si archivo nuevo,
			//volcar contenido de la memoria flash en ram aqui
			//Suponemos que permisos de escritura estan activos
			zxuno_flash_must_flush_to_disk=1;
		}

        }

	//Sale con ESC
        else {
		//dejar archivo por defecto
		zxuno_flash_spi_name[0]=0;

		//Y por defecto solo lectura
		zxuno_flash_persistent_writes.v=0;

		if (menu_confirm_yesno_texto("Default SPI Flash file","Reload SPI Flash from file?")) {
			zxuno_load_spi_flash();
                }

        }
}

void menu_storage_mmc_emulation(MENU_ITEM_PARAMETERS)
{
	if (mmc_enabled.v) mmc_disable();
	else mmc_enable();
}


int menu_storage_mmc_emulation_cond(void)
{
        if (mmc_file_name[0]==0) return 0;
        else return 1;
}

int menu_storage_mmc_if_enabled_cond(void)
{
	return mmc_enabled.v;
}

void menu_storage_zxmmc_emulation(MENU_ITEM_PARAMETERS)
{
	zxmmc_emulation.v ^=1;
}

void menu_storage_divmmc_mmc_ports_emulation(MENU_ITEM_PARAMETERS)
{
        if (divmmc_mmc_ports_enabled.v) divmmc_mmc_ports_disable();
        else divmmc_mmc_ports_enable();
}

void menu_storage_divmmc_diviface(MENU_ITEM_PARAMETERS)
{
	if (divmmc_diviface_enabled.v) divmmc_diviface_disable();
	else {
		divmmc_diviface_enable();
                //Tambien activamos puertos si esta mmc activado. Luego si quiere el usuario que los desactive
                if (mmc_enabled.v) divmmc_mmc_ports_enable();
	}
}


void menu_storage_mmc_file(MENU_ITEM_PARAMETERS)
{

	mmc_disable();

        char *filtros[4];

        filtros[0]="mmc";
		filtros[1]="mmcide";
		filtros[2]="hdf";
        filtros[3]=0;


        if (menu_filesel("Select MMC File",filtros,mmc_file_name)==1) {
		if (!si_existe_archivo(mmc_file_name)) {
			if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                mmc_file_name[0]=0;
                                return;
                        }

			//Preguntar tamanyo en MB
			char string_tamanyo[5];
			sprintf (string_tamanyo,"32");
			menu_ventana_scanf("Size (in MB)",string_tamanyo,5);
			int size=parse_string_to_number(string_tamanyo);
			if (size<1) {
				debug_printf (VERBOSE_ERR,"Invalid file size");
				mmc_file_name[0]=0;
                                return;
			}

			if (size>=1024) {
				menu_warn_message("Using MMC bigger than 1 GB can be very slow");
			}


			//Crear archivo vacio
		        FILE *ptr_mmcfile;
			ptr_mmcfile=fopen(mmc_file_name,"wb");

		        long int totalsize=size;
			totalsize=totalsize*1024*1024;
			z80_byte valor_grabar=0;

		        if (ptr_mmcfile!=NULL) {
				while (totalsize) {
					fwrite(&valor_grabar,1,1,ptr_mmcfile);
					totalsize--;
				}
		                fclose(ptr_mmcfile);
		        }

		}

		else {
			//Comprobar aqui tambien el tamanyo
			long int size=get_file_size(mmc_file_name);
			if (size>1073741824L) {
				menu_warn_message("Using MMC bigger than 1 GB can be very slow");
                        }
		}


        }
        //Sale con ESC
        else {
                //Quitar nombre
                mmc_file_name[0]=0;


        }
}

void menu_storage_zxpand_enable(MENU_ITEM_PARAMETERS)
{
	if (zxpand_enabled.v) zxpand_disable();
	else zxpand_enable();
}

void menu_storage_zxpand_root_dir(MENU_ITEM_PARAMETERS)
{

	int ret;
	ret=menu_storage_string_root_dir(zxpand_root_dir);

	//Si sale con ESC
	if (ret==0) {
       	//directorio zxpand vacio
        zxpand_cwd[0]=0;
	}		

}


void menu_timexcart_load(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="dck";

        filtros[1]=0;



        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de ultimo archivo
        //si no hay directorio, vamos a rutas predefinidas
        if (last_timex_cart[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(last_timex_cart,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }


        int ret;

        ret=menu_filesel("Select Cartridge",filtros,last_timex_cart);
        //volvemos a directorio inicial
		menu_filesel_chdir(directorio_actual);


        if (ret==1) {
		//                sprintf (last_timex_cart,"%s",timexcart_load_file);

                //sin overlay de texto, que queremos ver las franjas de carga con el color normal (no apagado)
                reset_menu_overlay_function();


                        timex_insert_dck_cartridge(last_timex_cart);

                //restauramos modo normal de texto de menu
                set_menu_overlay_function(normal_overlay_texto_menu);

                //Y salimos de todos los menus
                salir_todos_menus=1;
        }


}


void menu_timexcart_eject(MENU_ITEM_PARAMETERS)
{
	timex_empty_dock_space();
	menu_generic_message("Eject Cartridge","OK. Cartridge ejected");
}

void menu_dandanator_rom_file(MENU_ITEM_PARAMETERS)
{
	dandanator_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select dandanator File",filtros,dandanator_rom_file_name)==1) {
                if (!si_existe_archivo(dandanator_rom_file_name)) {
                        menu_error_message("File does not exist");
                        dandanator_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(dandanator_rom_file_name);
                        if (size!=DANDANATOR_SIZE) {
                                menu_error_message("ROM file must be 512 KB length");
                                dandanator_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                dandanator_rom_file_name[0]=0;


        }

}

int menu_storage_dandanator_emulation_cond(void)
{
	if (dandanator_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_dandanator_press_button_cond(void)
{
	return dandanator_enabled.v;
}


void menu_storage_dandanator_emulation(MENU_ITEM_PARAMETERS)
{
	if (dandanator_enabled.v) dandanator_disable();
	else dandanator_enable();
}

void menu_storage_dandanator_press_button(MENU_ITEM_PARAMETERS)
{
	dandanator_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_dandanator(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_dandanator;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_dandanator_file_shown[13];


                        menu_tape_settings_trunc_name(dandanator_rom_file_name,string_dandanator_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_dandanator,MENU_OPCION_NORMAL,menu_dandanator_rom_file,NULL,"~~ROM File [%s]",string_dandanator_file_shown);
                        menu_add_item_menu_shortcut(array_menu_dandanator,'r');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"ROM Emulation file");


                        menu_add_item_menu_format(array_menu_dandanator,MENU_OPCION_NORMAL,menu_storage_dandanator_emulation,menu_storage_dandanator_emulation_cond,"[%c] Dandanator ~~Enabled", (dandanator_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_dandanator,'e');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"Enable dandanator");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"Enable dandanator");


			menu_add_item_menu_format(array_menu_dandanator,MENU_OPCION_NORMAL,menu_storage_dandanator_press_button,menu_storage_dandanator_press_button_cond,"~~Press button");
			menu_add_item_menu_shortcut(array_menu_dandanator,'p');
                        menu_add_item_menu_tooltip(array_menu_dandanator,"Press button");
                        menu_add_item_menu_ayuda(array_menu_dandanator,"Press button");


                                menu_add_item_menu(array_menu_dandanator,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_dandanator);

				char titulo_menu[32];
				if (MACHINE_IS_SPECTRUM) strcpy(titulo_menu,"ZX Dandanator");
				else strcpy(titulo_menu,"CPC Dandanator");

                retorno_menu=menu_dibuja_menu(&dandanator_opcion_seleccionada,&item_seleccionado,array_menu_dandanator,titulo_menu);

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_kartusho_rom_file(MENU_ITEM_PARAMETERS)
{
	kartusho_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select kartusho File",filtros,kartusho_rom_file_name)==1) {
                if (!si_existe_archivo(kartusho_rom_file_name)) {
                        menu_error_message("File does not exist");
                        kartusho_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(kartusho_rom_file_name);
                        if (size!=KARTUSHO_SIZE) {
                                menu_error_message("ROM file must be 512 KB length");
                                kartusho_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                kartusho_rom_file_name[0]=0;


        }

}

int menu_storage_kartusho_emulation_cond(void)
{
	if (kartusho_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_kartusho_press_button_cond(void)
{
	return kartusho_enabled.v;
}


void menu_storage_kartusho_emulation(MENU_ITEM_PARAMETERS)
{
	if (kartusho_enabled.v) kartusho_disable();
	else kartusho_enable();
}

void menu_storage_kartusho_press_button(MENU_ITEM_PARAMETERS)
{
	kartusho_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_kartusho(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_kartusho;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_kartusho_file_shown[13];


                        menu_tape_settings_trunc_name(kartusho_rom_file_name,string_kartusho_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_kartusho,MENU_OPCION_NORMAL,menu_kartusho_rom_file,NULL,"~~ROM File [%s]",string_kartusho_file_shown);
                        menu_add_item_menu_shortcut(array_menu_kartusho,'r');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"ROM Emulation file");


                        			menu_add_item_menu_format(array_menu_kartusho,MENU_OPCION_NORMAL,menu_storage_kartusho_emulation,menu_storage_kartusho_emulation_cond,"[%c] ~~Kartusho Enabled", (kartusho_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_kartusho,'k');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"Enable kartusho");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"Enable kartusho");


			menu_add_item_menu_format(array_menu_kartusho,MENU_OPCION_NORMAL,menu_storage_kartusho_press_button,menu_storage_kartusho_press_button_cond,"~~Press button");
			menu_add_item_menu_shortcut(array_menu_kartusho,'p');
                        menu_add_item_menu_tooltip(array_menu_kartusho,"Press button");
                        menu_add_item_menu_ayuda(array_menu_kartusho,"Press button");


                                menu_add_item_menu(array_menu_kartusho,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_kartusho);

                retorno_menu=menu_dibuja_menu(&kartusho_opcion_seleccionada,&item_seleccionado,array_menu_kartusho,"Kartusho" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_ifrom_rom_file(MENU_ITEM_PARAMETERS)
{
	ifrom_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select ifrom File",filtros,ifrom_rom_file_name)==1) {
                if (!si_existe_archivo(ifrom_rom_file_name)) {
                        menu_error_message("File does not exist");
                        ifrom_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(ifrom_rom_file_name);
                        if (size!=IFROM_SIZE) {
                                menu_error_message("ROM file must be 512 KB length");
                                ifrom_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                ifrom_rom_file_name[0]=0;


        }

}

int menu_storage_ifrom_emulation_cond(void)
{
	if (ifrom_rom_file_name[0]==0) return 0;
        return 1;
}

int menu_storage_ifrom_press_button_cond(void)
{
	return ifrom_enabled.v;
}


void menu_storage_ifrom_emulation(MENU_ITEM_PARAMETERS)
{
	if (ifrom_enabled.v) ifrom_disable();
	else ifrom_enable();
}

void menu_storage_ifrom_press_button(MENU_ITEM_PARAMETERS)
{
	ifrom_press_button();
	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_ifrom(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ifrom;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_ifrom_file_shown[13];


                        menu_tape_settings_trunc_name(ifrom_rom_file_name,string_ifrom_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_ifrom,MENU_OPCION_NORMAL,menu_ifrom_rom_file,NULL,"~~ROM File [%s]",string_ifrom_file_shown);
                        menu_add_item_menu_shortcut(array_menu_ifrom,'r');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"ROM Emulation file");


                        			menu_add_item_menu_format(array_menu_ifrom,MENU_OPCION_NORMAL,menu_storage_ifrom_emulation,menu_storage_ifrom_emulation_cond,"[%c] ~~iFrom Enabled", (ifrom_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_ifrom,'k');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"Enable ifrom");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"Enable ifrom");


			menu_add_item_menu_format(array_menu_ifrom,MENU_OPCION_NORMAL,menu_storage_ifrom_press_button,menu_storage_ifrom_press_button_cond,"~~Press button");
			menu_add_item_menu_shortcut(array_menu_ifrom,'p');
                        menu_add_item_menu_tooltip(array_menu_ifrom,"Press button");
                        menu_add_item_menu_ayuda(array_menu_ifrom,"Press button");


                                menu_add_item_menu(array_menu_ifrom,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_ifrom);

                retorno_menu=menu_dibuja_menu(&ifrom_opcion_seleccionada,&item_seleccionado,array_menu_ifrom,"iFrom" );


                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);

                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_storage_betadisk_emulation(MENU_ITEM_PARAMETERS)
{
	if (betadisk_enabled.v) betadisk_disable();
	else betadisk_enable();
}

void menu_storage_betadisk_allow_boot(MENU_ITEM_PARAMETERS)
{
	betadisk_allow_boot_48k.v ^=1;
}


void menu_storage_trd_emulation(MENU_ITEM_PARAMETERS)
{
	if (trd_enabled.v) trd_disable();
	else trd_enable();
}

void menu_storage_trd_write_protect(MENU_ITEM_PARAMETERS)
{
	trd_write_protection.v ^=1;
}

int menu_storage_trd_emulation_cond(void)
{
        if (trd_file_name[0]==0) return 0;
        else return 1;
}


void menu_storage_trd_file(MENU_ITEM_PARAMETERS)
{

	trd_disable();

        char *filtros[2];

        filtros[0]="trd";
        filtros[1]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

              //Obtenemos directorio de trd
        //si no hay directorio, vamos a rutas predefinidas
        if (trd_file_name[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(trd_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }


        int ret=menu_filesel("Select TRD File",filtros,trd_file_name);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {
		if (!si_existe_archivo(trd_file_name)) {
			if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                trd_file_name[0]=0;
                                return;
                        }


			//Crear archivo vacio
		        FILE *ptr_trdfile;
			ptr_trdfile=fopen(trd_file_name,"wb");

		        long int totalsize=640*1024;
			
			z80_byte valor_grabar=0;

		        if (ptr_trdfile!=NULL) {
				while (totalsize) {
					fwrite(&valor_grabar,1,1,ptr_trdfile);
					totalsize--;
				}
		                fclose(ptr_trdfile);
		        }

		}

		trd_enable();
		


        }
        //Sale con ESC
        else {
                //Quitar nombre
                trd_file_name[0]=0;


        }
}


void menu_storage_trd_browser(MENU_ITEM_PARAMETERS)
{
	//menu_file_trd_browser_show(trd_file_name,"TRD");
	menu_file_viewer_read_file("TRD file browser",trd_file_name);
}


void menu_storage_trd_persistent_writes(MENU_ITEM_PARAMETERS)
{
	trd_persistent_writes.v ^=1;
}


void menu_betadisk(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_betadisk;
        menu_item item_seleccionado;
        int retorno_menu;
        do {




        	char string_trd_file_shown[17];
						

menu_tape_settings_trunc_name(trd_file_name,string_trd_file_shown,17);
                        menu_add_item_menu_inicial_format(&array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_file,NULL,"~~TRD File [%s]",string_trd_file_shown);
                        menu_add_item_menu_shortcut(array_menu_betadisk,'t');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Emulation file");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Emulation file");


                        menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_emulation,menu_storage_trd_emulation_cond,"[%c] TRD ~~Emulation", (trd_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_betadisk,'e');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Emulation");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Emulation");


			menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_write_protect,NULL,"[%c] ~~Write protect", (trd_write_protection.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_betadisk,'w');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"If TRD disk is write protected");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"If TRD disk is write protected");


                        menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_persistent_writes,NULL,"[%c] Persistent Writes",(trd_persistent_writes.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_betadisk,"Tells if TRD writes are saved to disk");
			menu_add_item_menu_ayuda(array_menu_betadisk,"Tells if TRD writes are saved to disk. "
			"Note: all writing operations to TRD are always saved to internal memory (unless you disable write permission), but this setting "
			"tells if these changes are written to disk or not."
			);



                        menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_trd_browser,menu_storage_trd_emulation_cond,"TRD ~~Browser");
                        menu_add_item_menu_shortcut(array_menu_betadisk,'b');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"TRD Browser");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"TRD Browser");



                        menu_add_item_menu(array_menu_betadisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                        menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_betadisk_emulation,NULL,"[%c] Betadis~~k Enabled", (betadisk_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_betadisk,'k');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"Enable betadisk");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"Enable betadisk");


			menu_add_item_menu_format(array_menu_betadisk,MENU_OPCION_NORMAL,menu_storage_betadisk_allow_boot,NULL,"[%c] ~~Allow Boot", (betadisk_allow_boot_48k.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_betadisk,'a');
                        menu_add_item_menu_tooltip(array_menu_betadisk,"Allow autoboot on 48k machines");
                        menu_add_item_menu_ayuda(array_menu_betadisk,"Allow autoboot on 48k machines");

                        


                        
                                menu_add_item_menu(array_menu_betadisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_betadisk);

                retorno_menu=menu_dibuja_menu(&betadisk_opcion_seleccionada,&item_seleccionado,array_menu_betadisk,"Betadisk" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_superupgrade_rom_file(MENU_ITEM_PARAMETERS)
{
        superupgrade_disable();

        char *filtros[2];

        filtros[0]="flash";
        filtros[1]=0;


        if (menu_filesel("Select Superupgrade File",filtros,superupgrade_rom_file_name)==1) {
                if (!si_existe_archivo(superupgrade_rom_file_name)) {
                        menu_error_message("File does not exist");
                        superupgrade_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(superupgrade_rom_file_name);
                        if (size!=SUPERUPGRADE_ROM_SIZE) {
                                menu_error_message("Flash file must be 512 KB length");
                                superupgrade_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                superupgrade_rom_file_name[0]=0;

        }

}

int menu_storage_superupgrade_emulation_cond(void)
{
        if (superupgrade_rom_file_name[0]==0) return 0;
        return 1;
}


void menu_storage_superupgrade_emulation(MENU_ITEM_PARAMETERS)
{
        if (superupgrade_enabled.v) superupgrade_disable();
        else superupgrade_enable(1);
}

void menu_storage_superupgrade_internal_rom(MENU_ITEM_PARAMETERS)
{
		//superupgrade_puerto_43b ^=0x20;
		//if ( (superupgrade_puerto_43b & (32+64))==32) return 1;

		superupgrade_puerto_43b &=(255-32-64);
		superupgrade_puerto_43b |=32;
}



void menu_superupgrade(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_superupgrade;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_superupgrade_file_shown[13];


                        menu_tape_settings_trunc_name(superupgrade_rom_file_name,string_superupgrade_file_shown,13);
                        menu_add_item_menu_inicial_format(&array_menu_superupgrade,MENU_OPCION_NORMAL,menu_superupgrade_rom_file,NULL,"~~Flash File [%s]",string_superupgrade_file_shown);
                        menu_add_item_menu_shortcut(array_menu_superupgrade,'f');
                        menu_add_item_menu_tooltip(array_menu_superupgrade,"Flash Emulation file");
                        menu_add_item_menu_ayuda(array_menu_superupgrade,"Flash Emulation file");


                        menu_add_item_menu_format(array_menu_superupgrade,MENU_OPCION_NORMAL,menu_storage_superupgrade_emulation,menu_storage_superupgrade_emulation_cond,"[%c] ~~Superupgrade Enabled", (superupgrade_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_superupgrade,'s');
                        menu_add_item_menu_tooltip(array_menu_superupgrade,"Enable superupgrade");
                        menu_add_item_menu_ayuda(array_menu_superupgrade,"Enable superupgrade");


												menu_add_item_menu_format(array_menu_superupgrade,MENU_OPCION_NORMAL,menu_storage_superupgrade_internal_rom,menu_storage_superupgrade_emulation_cond,"[%c] Show ~~internal ROM", (si_superupgrade_muestra_rom_interna() ? 'X' : ' '));
												menu_add_item_menu_shortcut(array_menu_superupgrade,'i');
												menu_add_item_menu_tooltip(array_menu_superupgrade,"Show internal ROM instead of Superupgrade flash");
												menu_add_item_menu_ayuda(array_menu_superupgrade,"Show internal ROM instead of Superupgrade flash");



                                menu_add_item_menu(array_menu_superupgrade,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_superupgrade);


retorno_menu=menu_dibuja_menu(&superupgrade_opcion_seleccionada,&item_seleccionado,array_menu_superupgrade,"Superupgrade" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}




void menu_if1_settings(MENU_ITEM_PARAMETERS)
{
	if (if1_enabled.v==0) enable_if1();
	else disable_if1();
}

void menu_storage_divmmc_diviface_total_ram(MENU_ITEM_PARAMETERS)
{
	diviface_current_ram_memory_bits++;
	if (diviface_current_ram_memory_bits==7) diviface_current_ram_memory_bits=2;
}


void menu_timexcart(MENU_ITEM_PARAMETERS)
{

        menu_item *array_menu_timexcart;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial(&array_menu_timexcart,"~~Load Cartridge",MENU_OPCION_NORMAL,menu_timexcart_load,NULL);
                menu_add_item_menu_shortcut(array_menu_timexcart,'l');
                menu_add_item_menu_tooltip(array_menu_timexcart,"Load Timex Cartridge");
                menu_add_item_menu_ayuda(array_menu_timexcart,"Supported timex cartridge formats on load:\n"
                                        "DCK");

                menu_add_item_menu(array_menu_timexcart,"~~Eject Cartridge",MENU_OPCION_NORMAL,menu_timexcart_eject,NULL);
                menu_add_item_menu_shortcut(array_menu_timexcart,'e');
                menu_add_item_menu_tooltip(array_menu_timexcart,"Eject Cartridge");
                menu_add_item_menu_ayuda(array_menu_timexcart,"Eject Cartridge");


     				menu_add_item_menu(array_menu_timexcart,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_timexcart);

                retorno_menu=menu_dibuja_menu(&timexcart_opcion_seleccionada,&item_seleccionado,array_menu_timexcart,"Timex Cartridge" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_storage_mmc_reload(MENU_ITEM_PARAMETERS)
{
	if (mmc_read_file_to_memory()==0) {
		menu_generic_message_splash("Reload MMC","OK. MMC file reloaded");
	}
}

void menu_divmmc_rom_file(MENU_ITEM_PARAMETERS)
{


	//desactivamos diviface divmmc. Así obligamos que el usuario tenga que activarlo de nuevo, recargando del firmware
	divmmc_diviface_disable();


        char *filtros[3];

        filtros[0]="rom";
				filtros[1]="bin";
        filtros[2]=0;


        if (menu_filesel("Select ROM File",filtros, divmmc_rom_name)==1) {
				//Nada

        }
        //Sale con ESC
        else {
                //Quitar nombre
                divmmc_rom_name[0]=0;


        }

				menu_generic_message("Change DIVMMC ROM","OK. Remember to enable DIVMMC paging to load the firmware");
}

void menu_storage_diviface_eprom_write_jumper(MENU_ITEM_PARAMETERS)
{
	diviface_eprom_write_jumper.v ^=1;
}

void menu_storage_mmc_write_protect(MENU_ITEM_PARAMETERS)
{
	mmc_write_protection.v ^=1;
}


void menu_storage_mmc_persistent_writes(MENU_ITEM_PARAMETERS)
{
	mmc_persistent_writes.v ^=1;
}

void menu_storage_mmc_browser(MENU_ITEM_PARAMETERS)
{
	//menu_file_mmc_browser_show(mmc_file_name,"MMC");
	menu_file_viewer_read_file("MMC file browser",mmc_file_name);
}

//menu MMC/Divmmc
void menu_mmc_divmmc(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_mmc_divmmc;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_mmc_file_shown[17];
								char string_divmmc_rom_file_shown[10];


                        menu_tape_settings_trunc_name(mmc_file_name,string_mmc_file_shown,17);
                        menu_add_item_menu_inicial_format(&array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_file,NULL,"~~MMC File [%s]",string_mmc_file_shown);
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'m');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"MMC Emulation file");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"MMC Emulation file");


                        menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_emulation,menu_storage_mmc_emulation_cond,"[%c] MMC ~~Emulation", (mmc_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'e');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"MMC Emulation");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"MMC Emulation");




                        menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_write_protect,NULL,"[%c] ~~Write protect", (mmc_write_protection.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'w');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"If MMC disk is write protected");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"If MMC disk is write protected");


			menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_persistent_writes,NULL,"[%c] Persistent Writes",(mmc_persistent_writes.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Tells if MMC writes are saved to disk");
			menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Tells if MMC writes are saved to disk. "
			"Note: all writing operations to MMC are always saved to internal memory (unless you disable write permission), but this setting "
			"tells if these changes are written to disk or not."
			);

  			menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_browser,menu_storage_mmc_emulation_cond,"MMC ~~Browser");
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'b');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"MMC Browser");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"MMC Browser");


			if (mmc_enabled.v) {
				menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_mmc_reload,NULL,"Reload MMC file");
				menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Reload MMC contents from MMC file to emulator memory");
				menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Reload MMC contents from MMC file to emulator memory. You can modify the MMC file "
																								"outside the emulator, and reload its contents without having to disable and enable MM.");
			}


			menu_add_item_menu(array_menu_mmc_divmmc,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_divmmc_diviface,NULL,"[%c] ~~DIVMMC paging",(divmmc_diviface_enabled.v ? 'X' : ' ') );
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'d');
			menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Enables DIVMMC paging and firmware, and DIVMMC access ports if MMC emulation is enabled");
			menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Enables DIVMMC paging and firmware, and DIVMMC access ports if MMC emulation is enabled");

			if (divmmc_diviface_enabled.v) {
				menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_divmmc_diviface_total_ram,NULL,"DIVMMC RAM [%d KB]",get_diviface_total_ram() );
				menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Changes DIVMMC RAM");
				menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Changes DIVMMC RAM");


			}

			if (divmmc_rom_name[0]==0) sprintf (string_divmmc_rom_file_shown,"Default");
			else menu_tape_settings_trunc_name(divmmc_rom_name, string_divmmc_rom_file_shown,10);
			menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_divmmc_rom_file,NULL,"DIVMMC EPROM File [%s]", string_divmmc_rom_file_shown);

			menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Changes DIVMMC firmware eprom file");
			menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Changes DIVMMC firmware eprom file");


			if (divmmc_diviface_enabled.v) {
				menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_diviface_eprom_write_jumper,NULL,"[%c] Firmware writeable",
				(diviface_eprom_write_jumper.v ? 'X' : ' ') );
				menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Allows writing to DivIDE/DivMMC eprom");
				menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Allows writing to DivIDE/DivMMC eprom. Changes are lost when you exit the emulator");
			}

                        menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_divmmc_mmc_ports_emulation,menu_storage_mmc_if_enabled_cond,"[%c] DIVMMC ~~ports",(divmmc_mmc_ports_enabled.v ? 'X' : ' ') );
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'p');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Enables DIVMMC access ports");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Enables DIVMMC access ports. Requires enabling MMC Emulation");


                        menu_add_item_menu(array_menu_mmc_divmmc,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                        menu_add_item_menu_format(array_menu_mmc_divmmc,MENU_OPCION_NORMAL,menu_storage_zxmmc_emulation,menu_storage_mmc_if_enabled_cond,"[%c] ~~ZXMMC Enabled",(zxmmc_emulation.v ? 'X' : ' ') );
                        menu_add_item_menu_shortcut(array_menu_mmc_divmmc,'z');
                        menu_add_item_menu_tooltip(array_menu_mmc_divmmc,"Access MMC using ZXMMC");
                        menu_add_item_menu_ayuda(array_menu_mmc_divmmc,"Enables ZXMMC ports to access MMC");



				menu_add_item_menu(array_menu_mmc_divmmc,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_mmc_divmmc,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_mmc_divmmc);

                retorno_menu=menu_dibuja_menu(&mmc_divmmc_opcion_seleccionada,&item_seleccionado,array_menu_mmc_divmmc,"MMC" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_tape_autoloadtape(MENU_ITEM_PARAMETERS)
{
        noautoload.v ^=1;
}

void menu_tape_autoselectfileopt(MENU_ITEM_PARAMETERS)
{
        autoselect_snaptape_options.v ^=1;
}




void menu_storage_ide_emulation(MENU_ITEM_PARAMETERS)
{
        if (ide_enabled.v) ide_disable();
        else ide_enable();
}


int menu_storage_ide_emulation_cond(void)
{
        if (ide_file_name[0]==0) return 0;
        else return 1;
}

/*
void menu_storage_divide_emulation(MENU_ITEM_PARAMETERS)
{
        if (divide_enabled.v) divide_disable();
        else divide_enable();
}
*/


void menu_storage_divide_ide_ports_emulation(MENU_ITEM_PARAMETERS)
{
        if (divide_ide_ports_enabled.v) divide_ide_ports_disable();
        else divide_ide_ports_enable();
}

void menu_storage_divide_diviface(MENU_ITEM_PARAMETERS)
{
        if (divide_diviface_enabled.v) divide_diviface_disable();
        else {
                divide_diviface_enable();
                //Tambien activamos puertos si esta ide activado. Luego si quiere el usuario que los desactive
		if (ide_enabled.v) divide_ide_ports_enable();
        }
}



void menu_storage_ide_file(MENU_ITEM_PARAMETERS)
{

        ide_disable();

        char *filtros[3];

        filtros[0]="ide";
		filtros[1]="mmcide";
        filtros[2]=0;


        if (menu_filesel("Select IDE File",filtros,ide_file_name)==1) {
                if (!si_existe_archivo(ide_file_name)) {
                        if (menu_confirm_yesno_texto("File does not exist","Create?")==0) {
                                ide_file_name[0]=0;
                                return;
                        }

                        //Preguntar tamanyo en MB
                        char string_tamanyo[5];
                        sprintf (string_tamanyo,"32");
                        menu_ventana_scanf("Size (in MB)",string_tamanyo,5);
                        int size=parse_string_to_number(string_tamanyo);
                        if (size<1) {
                                debug_printf (VERBOSE_ERR,"Invalid file size");
                                ide_file_name[0]=0;
                                return;
                        }

                        if (size>=1024) {
                                menu_warn_message("Using IDE bigger than 1 GB can be very slow");
                        }


                        //Crear archivo vacio
                        FILE *ptr_idefile;
                        ptr_idefile=fopen(ide_file_name,"wb");

   long int totalsize=size;
                        totalsize=totalsize*1024*1024;
                        z80_byte valor_grabar=0;

                        if (ptr_idefile!=NULL) {
                                while (totalsize) {
                                        fwrite(&valor_grabar,1,1,ptr_idefile);
                                        totalsize--;
                                }
                                fclose(ptr_idefile);
                        }

                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(ide_file_name);
                        if (size>1073741824L) {
                                menu_warn_message("Using IDE bigger than 1 GB can be very slow");
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                ide_file_name[0]=0;


        }
}


int menu_storage_ide_if_enabled_cond(void)
{
	return ide_enabled.v;
}

void menu_eightbitsimple_enable(MENU_ITEM_PARAMETERS)
{
	if (eight_bit_simple_ide_enabled.v) eight_bit_simple_ide_disable();
	else eight_bit_simple_ide_enable();
}

void menu_atomlite_enable(MENU_ITEM_PARAMETERS)
{
        int reset=0;

        if (atomlite_enabled.v) {

                reset=menu_confirm_yesno_texto("Confirm reset","Load normal rom and reset?");

                atomlite_enabled.v=0;
        }

        else {
                reset=menu_confirm_yesno_texto("Confirm reset","Load atomlite rom and reset?");
                atomlite_enabled.v=1;
        }

        if (reset) {
                set_machine(NULL);
                cold_start_cpu_registers();
                reset_cpu();
		salir_todos_menus=1;
        }

}

void menu_storage_ide_reload(MENU_ITEM_PARAMETERS)
{
	if (ide_read_file_to_memory()==0) {
		menu_generic_message_splash("Reload IDE","OK. IDE file reloaded");
	}
}


void menu_divide_rom_file(MENU_ITEM_PARAMETERS)
{


	//desactivamos diviface divide. Así obligamos que el usuario tenga que activarlo de nuevo, recargando del firmware
	divide_diviface_disable();


        char *filtros[3];

        filtros[0]="rom";
				filtros[1]="bin";
        filtros[2]=0;


        if (menu_filesel("Select ROM File",filtros, divide_rom_name)==1) {
				//Nada

        }
        //Sale con ESC
        else {
                //Quitar nombre
                divide_rom_name[0]=0;


        }

				menu_generic_message("Change DIVIDE ROM","OK. Remember to enable DIVIDE paging to load the firmware");
}

void menu_storage_ide_write_protect(MENU_ITEM_PARAMETERS)
{
	ide_write_protection.v ^=1;
}

void menu_storage_ide_persistent_writes(MENU_ITEM_PARAMETERS)
{
	ide_persistent_writes.v ^=1;
}


void menu_storage_ide_browser(MENU_ITEM_PARAMETERS)
{
	//menu_file_mmc_browser_show(ide_file_name,"IDE");
	menu_file_viewer_read_file("IDE file browser",ide_file_name);
}

//menu IDE/Divide
void menu_ide_divide(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ide_divide;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_ide_file_shown[17];
								char string_divide_rom_file_shown[10];





                        menu_tape_settings_trunc_name(ide_file_name,string_ide_file_shown,17);
                        menu_add_item_menu_inicial_format(&array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_file,NULL,"~~IDE File [%s]",string_ide_file_shown);
                        menu_add_item_menu_shortcut(array_menu_ide_divide,'i');
                        menu_add_item_menu_tooltip(array_menu_ide_divide,"IDE Emulation file");
                        menu_add_item_menu_ayuda(array_menu_ide_divide,"IDE Emulation file");


                        menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_emulation,menu_storage_ide_emulation_cond,"[%c] IDE ~~Emulation", (ide_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_ide_divide,'e');
                        menu_add_item_menu_tooltip(array_menu_ide_divide,"IDE Emulation");
                        menu_add_item_menu_ayuda(array_menu_ide_divide,"IDE Emulation");


                        menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_write_protect,NULL,"[%c] ~~Write protect", (ide_write_protection.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_ide_divide,'w');
                        menu_add_item_menu_tooltip(array_menu_ide_divide,"If IDE disk is write protected");
                        menu_add_item_menu_ayuda(array_menu_ide_divide,"If IDE disk is write protected");


			menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_persistent_writes,NULL,"[%c] Persistent Writes",(ide_persistent_writes.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_ide_divide,"Tells if IDE writes are saved to disk");
			menu_add_item_menu_ayuda(array_menu_ide_divide,"Tells if IDE writes are saved to disk. "
			"Note: all writing operations to IDE are always saved to internal memory (unless you disable write permission), but this setting "
			"tells if these changes are written to disk or not."
			);


			menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_browser,menu_storage_ide_emulation_cond,"IDE ~~Browser");
                        menu_add_item_menu_shortcut(array_menu_ide_divide,'b');
                        menu_add_item_menu_tooltip(array_menu_ide_divide,"IDE Browser");
                        menu_add_item_menu_ayuda(array_menu_ide_divide,"IDE Browser");


												if (ide_enabled.v) {
												menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_ide_reload,NULL,"Reload IDE file");
												menu_add_item_menu_tooltip(array_menu_ide_divide,"Reload IDE contents from IDE file to emulator memory");
												menu_add_item_menu_ayuda(array_menu_ide_divide,"Reload IDE contents from IDE file to emulator memory. You can modify the IDE file "
												                        "outside the emulator, and reload its contents without having to disable and enable IDE");
												}

			


			if (MACHINE_IS_SPECTRUM) {

				menu_add_item_menu(array_menu_ide_divide,"",MENU_OPCION_SEPARADOR,NULL,NULL);

	                       menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_divide_diviface,NULL,"[%c] ~~DIVIDE paging",(divide_diviface_enabled.v ? 'X' : ' ') );
        	                menu_add_item_menu_shortcut(array_menu_ide_divide,'d');
        	                menu_add_item_menu_tooltip(array_menu_ide_divide,"Enables DIVIDE paging and firmware, and DIVIDE access ports if IDE emulation is enabled");
        	                menu_add_item_menu_ayuda(array_menu_ide_divide,"Enables DIVIDE paging and firmware, and DIVIDE access ports if IDE emulation is enabled");

	                        if (divide_diviface_enabled.v) {
	                                menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_divmmc_diviface_total_ram,NULL,"DIVIDE RAM [%d KB]",get_diviface_total_ram() );
        	                        menu_add_item_menu_tooltip(array_menu_ide_divide,"Changes DIVIDE RAM");
                	                menu_add_item_menu_ayuda(array_menu_ide_divide,"Changes DIVIDE RAM");



               		         }
													 if (divide_rom_name[0]==0) sprintf (string_divide_rom_file_shown,"Default");
													 else menu_tape_settings_trunc_name(divide_rom_name, string_divide_rom_file_shown,10);
													 menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_divide_rom_file,NULL,"DIVIDE EPROM File [%s]", string_divide_rom_file_shown);

													 menu_add_item_menu_tooltip(array_menu_ide_divide,"Changes DIVIDE firmware eprom file");
													 menu_add_item_menu_ayuda(array_menu_ide_divide,"Changes DIVIDE firmware eprom file");

													 if (divide_diviface_enabled.v) {
										 				menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_diviface_eprom_write_jumper,NULL,"[%c] Firmware writeable",
										 				(diviface_eprom_write_jumper.v ? 'X' : ' ') );
										 				menu_add_item_menu_tooltip(array_menu_ide_divide,"Allows writing to DivIDE/DivMMC eprom");
										 				menu_add_item_menu_ayuda(array_menu_ide_divide,"Allows writing to DivIDE/DivMMC eprom. Changes are lost when you exit the emulator");
										 			}



	                        menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_storage_divide_ide_ports_emulation,menu_storage_ide_if_enabled_cond,"[%c] DIVIDE ~~ports",(divide_ide_ports_enabled.v ? 'X' : ' ') );
        	                menu_add_item_menu_shortcut(array_menu_ide_divide,'p');
                	        menu_add_item_menu_tooltip(array_menu_ide_divide,"Enables DIVIDE access ports");
                        	menu_add_item_menu_ayuda(array_menu_ide_divide,"Enables DIVIDE access ports. Requires enabling IDE Emulation");


			}

			if (MACHINE_IS_SPECTRUM) {
				menu_add_item_menu(array_menu_ide_divide,"",MENU_OPCION_SEPARADOR,NULL,NULL);
				
				menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_eightbitsimple_enable,menu_storage_ide_if_enabled_cond,"[%c] 8-bit simple IDE",(eight_bit_simple_ide_enabled.v ? 'X' : ' ') );
			}


        if (MACHINE_IS_SAM) {
        		menu_add_item_menu(array_menu_ide_divide,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                        menu_add_item_menu_format(array_menu_ide_divide,MENU_OPCION_NORMAL,menu_atomlite_enable,NULL,"[%c] ~~Atom Lite",(atomlite_enabled.v ? 'X' : ' ' ) );
                        menu_add_item_menu_shortcut(array_menu_ide_divide,'a');
                        menu_add_item_menu_tooltip(array_menu_ide_divide,"Enable Atom Lite");
                        menu_add_item_menu_ayuda(array_menu_ide_divide,"Enable Atom Lite");
                }



                                menu_add_item_menu(array_menu_ide_divide,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_ide_divide,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_ide_divide);

                retorno_menu=menu_dibuja_menu(&ide_divide_opcion_seleccionada,&item_seleccionado,array_menu_ide_divide,"IDE" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

//Funcion para seleccionar un directorio con filesel
//Solo cambia string_root_dir si se sale de filesel con ESC
//Devuelve mismo valor que retorna menu_filesel
int menu_storage_string_root_dir(char *string_root_dir)
{

        char *filtros[2];

        filtros[0]="nofiles";
        filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        int ret;


	char nada[PATH_MAX];

        //Obtenemos ultimo directorio visitado
	menu_filesel_chdir(string_root_dir);


        ret=menu_filesel("Enter dir & press ESC",filtros,nada);


	//Si sale con ESC
	if (ret==0) {
		//Directorio root
		sprintf (string_root_dir,"%s",menu_filesel_last_directory_seen);
		debug_printf (VERBOSE_DEBUG,"Selected directory: %s",string_root_dir);

	}

    //volvemos a directorio inicial
    menu_filesel_chdir(directorio_actual);

	return ret;


}


void menu_ql_microdrive_floppy(MENU_ITEM_PARAMETERS)
{
	ql_microdrive_floppy_emulation ^=1;
}

void menu_ql_mdv1(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(ql_mdv1_root_dir);
}

void menu_ql_mdv2(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(ql_mdv2_root_dir);
}

void menu_ql_flp1(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(ql_flp1_root_dir);
}


void menu_storage_esxdos_traps_emulation(MENU_ITEM_PARAMETERS)
{



	if (esxdos_handler_enabled.v) esxdos_handler_disable();
	else {
		//Si no hay paging, avisar
		if (diviface_enabled.v==0) {
			if (menu_confirm_yesno_texto("No divide/mmc paging","Sure enable?")==0) return;
		}
		esxdos_handler_enable();
	}
}

void menu_esxdos_traps_root_dir(MENU_ITEM_PARAMETERS)
{


	int ret;
	ret=menu_storage_string_root_dir(esxdos_handler_root_dir);

	//Si sale con ESC
	if (ret==0) {
        //directorio esxdos vacio
	    esxdos_handler_cwd[0]=0;
	}		

}


void menu_esxdos_traps(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_esxdos_traps;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_esxdos_traps_root_dir_shown[18];


                menu_add_item_menu_inicial_format(&array_menu_esxdos_traps,MENU_OPCION_NORMAL,menu_storage_esxdos_traps_emulation,NULL,"[%c] ~~Enabled", (esxdos_handler_enabled.v ? 'X' : ' ' ));
          menu_add_item_menu_shortcut(array_menu_esxdos_traps,'e');
          menu_add_item_menu_tooltip(array_menu_esxdos_traps,"Enable ESXDOS handler");
          menu_add_item_menu_ayuda(array_menu_esxdos_traps,"Enable ESXDOS handler");

						if (esxdos_handler_enabled.v) {
                        menu_tape_settings_trunc_name(esxdos_handler_root_dir,string_esxdos_traps_root_dir_shown,18);
                        menu_add_item_menu_format(array_menu_esxdos_traps,MENU_OPCION_NORMAL,menu_esxdos_traps_root_dir,NULL,"~~Root dir: %s",string_esxdos_traps_root_dir_shown);
                        menu_add_item_menu_shortcut(array_menu_esxdos_traps,'r');

												menu_add_item_menu_tooltip(array_menu_esxdos_traps,"Sets the root directory for ESXDOS filesystem");
												menu_add_item_menu_ayuda(array_menu_esxdos_traps,"Sets the root directory for ESXDOS filesystem. "
													"Only file and folder names valid for ESXDOS will be shown:\n"
													"-Maximum 8 characters for name and 3 for extension\n"
													"-Files and folders will be shown always in uppercase. Folders which are not uppercase, are shown but can not be accessed\n"
													);


						}






                                menu_add_item_menu(array_menu_esxdos_traps,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_esxdos_traps);

                retorno_menu=menu_dibuja_menu(&esxdos_traps_opcion_seleccionada,&item_seleccionado,array_menu_esxdos_traps,"ESXDOS handler" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_storage_dskplusthree_file(MENU_ITEM_PARAMETERS)
{

	dskplusthree_disable();

        char *filtros[2];

        filtros[0]="dsk";
        filtros[1]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

              //Obtenemos directorio de dskplusthree
        //si no hay directorio, vamos a rutas predefinidas
        if (dskplusthree_file_name[0]==0) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(dskplusthree_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

	char dskfile[PATH_MAX];
	dskfile[0]=0;

        int ret=menu_filesel("Select DSK File",filtros,dskfile);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

		if (!si_existe_archivo(dskfile)) {

			menu_warn_message("File does not exist");
			return;


			//Crear archivo vacio
			/*
		        FILE *ptr_dskplusthreefile;
			ptr_dskplusthreefile=fopen(dskplusthree_file_name,"wb");

		        long int totalsize=640*1024;
			
			z80_byte valor_grabar=0;

		        if (ptr_dskplusthreefile!=NULL) {
				while (totalsize) {
					fwrite(&valor_grabar,1,1,ptr_dskplusthreefile);
					totalsize--;
				}
		                fclose(ptr_dskplusthreefile);
		        }
				*/

		}
		dsk_insert_disk(dskfile);

		dskplusthree_enable();
		pd765_enable();
		plus3dos_traps.v=1;


        }
        //Sale con ESC
        else {
                //Quitar nombre
                dskplusthree_file_name[0]=0;


        }
}

void menu_storage_plusthreedisk_traps(MENU_ITEM_PARAMETERS)
{
	plus3dos_traps.v ^=1;
}


void menu_plusthreedisk_pd765(MENU_ITEM_PARAMETERS)
{
        if (pd765_enabled.v) pd765_disable();
	else pd765_enable();
}

int menu_storage_dskplusthree_emulation_cond(void)
{
        if (dskplusthree_file_name[0]==0) return 0;
        else return 1;
}


void menu_storage_dskplusthree_emulation(MENU_ITEM_PARAMETERS)
{
	if (dskplusthree_emulation.v==0) {
		dskplusthree_enable();
		pd765_enable();
		plus3dos_traps.v=1;
	}

	else dskplusthree_disable();
}


void menu_storage_dskplusthree_browser(MENU_ITEM_PARAMETERS)
{
	menu_file_dsk_browser_show(dskplusthree_file_name);
}

void menu_storage_dsk_write_protect(MENU_ITEM_PARAMETERS)
{	
	dskplusthree_write_protection.v ^=1;
}	


void menu_storage_dskplusthree_persistent_writes(MENU_ITEM_PARAMETERS)
{
	dskplusthree_persistent_writes.v ^=1;
}

void menu_plusthreedisk(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_plusthreedisk;
        menu_item item_seleccionado;
        int retorno_menu;
        do {




        	char string_dskplusthree_file_shown[17];
						

			menu_tape_settings_trunc_name(dskplusthree_file_name,string_dskplusthree_file_shown,17);
                        menu_add_item_menu_inicial_format(&array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_file,NULL,"~~DSK File: %s",string_dskplusthree_file_shown);
                        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'d');
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"DSK Emulation file");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"DSK Emulation file");


                        menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_emulation,
                        	menu_storage_dskplusthree_emulation_cond,"DSK ~~Emulation: %s", (dskplusthree_emulation.v ? "Yes" : "No"));
                        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'e');
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"DSK Emulation");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"DSK Emulation");


			menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dsk_write_protect,NULL,"~~Write protect: %s", (dskplusthree_write_protection.v ? "Yes" : "No"));
                        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'w');
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"If DSK disk is write protected");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"If DSK disk is write protected");


			menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_persistent_writes,NULL,"Persistent Writes: %s",(dskplusthree_persistent_writes.v ? "Yes" : "No") );
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Tells if DSK writes are saved to disk");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Tells if DSK writes are saved to disk. "
                        "Note: all writing operations to TRD are always saved to internal memory (unless you disable write permission), but this setting "
                        "tells if these changes are written to disk or not."
                        );


                        menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_dskplusthree_browser,menu_storage_dskplusthree_emulation_cond,"DSK ~~Browser");
                        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'b');
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"DSK Browser");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"DSK Browser");

                               
			menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                        menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_storage_plusthreedisk_traps,NULL,"~~PLUS3DOS Traps: %s", (plus3dos_traps.v ? "Yes" : "No"));
                        menu_add_item_menu_shortcut(array_menu_plusthreedisk,'k');
                        menu_add_item_menu_tooltip(array_menu_plusthreedisk,"Enable plusthreedisk");
                        menu_add_item_menu_ayuda(array_menu_plusthreedisk,"Enable plusthreedisk");


                        menu_add_item_menu_format(array_menu_plusthreedisk,MENU_OPCION_NORMAL,menu_plusthreedisk_pd765,NULL,"PD765 enabled: %s",(pd765_enabled.v ? "Yes" : "No") );


                        
                                menu_add_item_menu(array_menu_plusthreedisk,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_plusthreedisk);

                retorno_menu=menu_dibuja_menu(&plusthreedisk_opcion_seleccionada,&item_seleccionado,array_menu_plusthreedisk,"+3 Disk" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



//menu storage settings
void menu_storage_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_storage_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                //char string_spi_flash_file_shown[13]; //,string_mmc_file_shown[13];

		 menu_add_item_menu_inicial(&array_menu_storage_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


		if (MACHINE_IS_Z88) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_z88_slots,NULL,"Z88 Memory ~~Slots");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'s');

                        menu_add_item_menu_tooltip(array_menu_storage_settings,"Z88 Memory Slots");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"Selects Memory Slots to use on Z88");

		}

		else if (MACHINE_IS_QL) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ql_microdrive_floppy,NULL,"Microdrive&Floppy: %s",
				(ql_microdrive_floppy_emulation ? "Yes" : "No") );

				if (ql_microdrive_floppy_emulation) {
					char string_ql_mdv1_root_dir_shown[13];
					char string_ql_mdv2_root_dir_shown[13];
					char string_ql_flp1_root_dir_shown[13];
					menu_tape_settings_trunc_name(ql_mdv1_root_dir,string_ql_mdv1_root_dir_shown,13);
					menu_tape_settings_trunc_name(ql_mdv2_root_dir,string_ql_mdv2_root_dir_shown,13);
					menu_tape_settings_trunc_name(ql_flp1_root_dir,string_ql_flp1_root_dir_shown,13);

					menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ql_mdv1,NULL,"Mdv1 root dir: %s",string_ql_mdv1_root_dir_shown);
					menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ql_mdv2,NULL,"Mdv2 root dir: %s",string_ql_mdv2_root_dir_shown);
					menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ql_flp1,NULL,"Flp1 root dir: %s",string_ql_flp1_root_dir_shown);
				}

		}


		else if (!MACHINE_IS_CHLOE) {
	            	menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_tape_settings,menu_tape_settings_cond,"~~Tape");
                	menu_add_item_menu_shortcut(array_menu_storage_settings,'t');
                	menu_add_item_menu_tooltip(array_menu_storage_settings,"Select tape and options");
                	menu_add_item_menu_ayuda(array_menu_storage_settings,"Select tape for input (read) or output (write). \n"
                                                              "You use them as real tapes, with BASIC functions like LOAD and SAVE\n"
                                                              "Also select different options to change tape behaviour");
		}


		if (MACHINE_IS_SPECTRUM_P2A_P3) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_plusthreedisk,NULL,"+3 ~~Disk");
			menu_add_item_menu_shortcut(array_menu_storage_settings,'d');
			menu_add_item_menu_tooltip(array_menu_storage_settings,"+3 Disk emulation");
			menu_add_item_menu_ayuda(array_menu_storage_settings,"+3 Disk emulation");
		}


		if (MACHINE_IS_TIMEX_TS2068) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_timexcart,NULL,"Timex ~~Cartridge");
			menu_add_item_menu_shortcut(array_menu_storage_settings,'c');
			menu_add_item_menu_tooltip(array_menu_storage_settings,"Timex Cartridge Settings");
			menu_add_item_menu_ayuda(array_menu_storage_settings,"Timex Cartridge Settings");
		}



		if (MACHINE_IS_ZX8081) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_storage_zxpand_enable,NULL,"ZX~~pand emulation: %s",(zxpand_enabled.v ? "Yes" : "No"));
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'p');
			menu_add_item_menu_tooltip(array_menu_storage_settings,"Enable ZXpand emulation");
			menu_add_item_menu_ayuda(array_menu_storage_settings,"Enable ZXpand emulation");


			if (zxpand_enabled.v) {
				char string_zxpand_root_folder_shown[20];
				menu_tape_settings_trunc_name(zxpand_root_dir,string_zxpand_root_folder_shown,20);
				menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_storage_zxpand_root_dir,NULL,"~~Root dir: %s",string_zxpand_root_folder_shown);
                        	menu_add_item_menu_shortcut(array_menu_storage_settings,'r');
				menu_add_item_menu_tooltip(array_menu_storage_settings,"Sets the root directory for ZXpand filesystem");
				menu_add_item_menu_ayuda(array_menu_storage_settings,"Sets the root directory for ZXpand filesystem. "
					"Only file and folder names valid for zxpand will be shown:\n"
					"-Maximum 8 characters for name and 3 for extension\n"
					"-Files and folders will be shown always in uppercase. Folders which are not uppercase, are shown but can not be accessed\n"
					);

			}
		}

		if (MACHINE_IS_SPECTRUM) {
			//menu_tape_settings_trunc_name(mmc_file_name,string_mmc_file_shown,13);
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_mmc_divmmc,NULL,"~~MMC");
			menu_add_item_menu_shortcut(array_menu_storage_settings,'m');
			menu_add_item_menu_tooltip(array_menu_storage_settings,"MMC, DivMMC and ZXMMC settings");
			menu_add_item_menu_ayuda(array_menu_storage_settings,"MMC, DivMMC and ZXMMC settings");
		}



		if (MACHINE_IS_SPECTRUM || MACHINE_IS_SAM) {

                        menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ide_divide,NULL,"~~IDE");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'i');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"IDE, DivIDE and 8-bit simple settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"IDE, DivIDE and 8-bit simple settings");


		}




		if (MACHINE_IS_SPECTRUM) {
                        menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_betadisk,NULL,"~~Betadisk");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'b');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"Betadisk settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"Betadisk settings");
		}


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_esxdos_traps,NULL,"~~ESXDOS Handler");
			menu_add_item_menu_shortcut(array_menu_storage_settings,'e');
			menu_add_item_menu_tooltip(array_menu_storage_settings,"Enables emulator to handle ESXDOS calls");
			menu_add_item_menu_ayuda(array_menu_storage_settings,"Enables emulator to handle ESXDOS calls and "
				"use local files from your computer instead of using from inside the mmc/ide virtual file.\n"
				"You can choose to have also "
				"enabled mmc/ide virtual file or not, you can use one of the settings or both. In case of using both, supported esxdos handler "
				"functions are managed with it; if not, they will be handled by the usual mmc/ide virtual file firmware (usually esxdos).\n"
				"Use with caution, "
				"only some ESXDOS calls are handled, the rest are handled from the mmc/ide virtual file firmware.\n"
				"The list of supported calls are: \n"
				"DISK_INFO, DISK_STATUS, F_CHDIR, F_CLOSE, F_FSTAT, F_GETCWD, F_MOUNT, F_OPEN, F_OPENDIR, F_READ, F_READDIR, F_RENAME, F_REWINDDIR, F_SEEK, F_SEEKDIR, F_STAT, F_SYNC, F_TELLDIR, F_UNLINK, F_WRITE, M_DRIVEINFO, M_GETSETDRV"
				"\n"
				"Note: you can also enable and disable ESXDOS handler whenever you want, for example in programs that use unsupported functions for handler."


			);
		}


		if (MACHINE_IS_ZXUNO) {
			menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_zxuno_spi_flash,NULL,"~~ZX-Uno Flash");
			menu_add_item_menu_shortcut(array_menu_storage_settings,'z');
		}
							


     		if ( (MACHINE_IS_SPECTRUM || MACHINE_IS_CPC) && !MACHINE_IS_ZXUNO) {

	                    menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_dandanator,NULL,
							"%s D~~andanator",(MACHINE_IS_SPECTRUM ? "ZX" : "CPC")  );
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'a');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"Dandanator settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"Dandanator settings");

                }

		if (superupgrade_supported_machine() ) {
			    menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_superupgrade,NULL,"Speccy Superup~~grade");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'g');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"Superupgrade settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"Superupgrade settings");

		}


		if (MACHINE_IS_SPECTRUM || MACHINE_IS_CPC) {
                       menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_kartusho,NULL,"~~Kartusho");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'k');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"Kartusho settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"Kartusho settings");
		}


		if (MACHINE_IS_SPECTRUM || MACHINE_IS_CPC) {
                       menu_add_item_menu_format(array_menu_storage_settings,MENU_OPCION_NORMAL,menu_ifrom,NULL,"i~~From");
                        menu_add_item_menu_shortcut(array_menu_storage_settings,'f');
                        menu_add_item_menu_tooltip(array_menu_storage_settings,"iFrom settings");
                        menu_add_item_menu_ayuda(array_menu_storage_settings,"iFrom settings");
		}

              



                menu_add_item_menu(array_menu_storage_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_storage_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_storage_settings);

                retorno_menu=menu_dibuja_menu(&storage_settings_opcion_seleccionada,&item_seleccionado,array_menu_storage_settings,"Storage" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}

void menu_ula_disable_rom_paging(MENU_ITEM_PARAMETERS)
{
	ula_disabled_rom_paging.v ^=1;
}

void menu_ula_disable_ram_paging(MENU_ITEM_PARAMETERS)
{
        ula_disabled_ram_paging.v ^=1;
}




//menu ula settings
void menu_ula_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_ula_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial(&array_menu_ula_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


#ifdef EMULATE_CONTEND

                if (MACHINE_IS_SPECTRUM) {
                        menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_late_timings,NULL,"ULA ~~timing [%s]",(ula_late_timings.v ? "Late" : "Early"));
                        menu_add_item_menu_shortcut(array_menu_ula_settings,'t');
                        menu_add_item_menu_tooltip(array_menu_ula_settings,"Use ULA early or late timings");
                        menu_add_item_menu_ayuda(array_menu_ula_settings,"Late timings have the contended memory table start one t-state later");

                        menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_contend,NULL,"[%c] ~~Contended memory", (contend_enabled.v==1 ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_ula_settings,'c');
                        menu_add_item_menu_tooltip(array_menu_ula_settings,"Enable contended memory & ports emulation");
                        menu_add_item_menu_ayuda(array_menu_ula_settings,"Contended memory & ports is the native way of some of the emulated machines");



		
		
		}

#endif

			if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_im2_slow,NULL,"[%c] ULA IM2 ~~slow",(ula_im2_slow.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_ula_settings,'s');
			menu_add_item_menu_tooltip(array_menu_ula_settings,"Add one t-state when an IM2 is fired");
			menu_add_item_menu_ayuda(array_menu_ula_settings,"It improves visualization on some demos, like overscan, ula128 and scroll2017");
                }




		if (MACHINE_IS_SPECTRUM) {

                        menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_pentagon_timing,NULL,"[%c] ~~Pentagon timing",(pentagon_timing.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_ula_settings,'p');
                        menu_add_item_menu_tooltip(array_menu_ula_settings,"Enable Pentagon timings");
                        menu_add_item_menu_ayuda(array_menu_ula_settings,"Pentagon does not have contended memory/ports and have different display timings");

		}


		if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
			menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_disable_rom_paging,NULL,"[%c] ROM Paging",(ula_disabled_rom_paging.v==0 ? 'X' : ' '));
			menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_disable_ram_paging,NULL,"[%c] RAM Paging",(ula_disabled_ram_paging.v==0 ? 'X' : ' '));
		}

                menu_add_item_menu(array_menu_ula_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_format(array_menu_ula_settings,MENU_OPCION_NORMAL,menu_ula_advanced,menu_cond_realvideo,"~~Advanced timing settings");
                menu_add_item_menu_shortcut(array_menu_ula_settings,'a');
                menu_add_item_menu_tooltip(array_menu_ula_settings,"Advanced timing settings. Requires realvideo");
                menu_add_item_menu_ayuda(array_menu_ula_settings,"Change and view some timings for the machine. Requires realvideo");


                menu_add_item_menu(array_menu_ula_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_ula_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_ula_settings);

                retorno_menu=menu_dibuja_menu(&ula_settings_opcion_seleccionada,&item_seleccionado,array_menu_ula_settings,"ULA Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



#define OSD_KEYBOARD_ANCHO_VENTANA 26
#define OSD_KEYBOARD_ALTO_VENTANA 12
#define OSD_KEYBOARD_X_VENTANA (menu_center_x()-OSD_KEYBOARD_ANCHO_VENTANA/2)
#define OSD_KEYBOARD_Y_VENTANA (menu_center_y()-OSD_KEYBOARD_ALTO_VENTANA/2)

        struct s_osd_teclas {
        char tecla[5]; //4 de longitud mas 0 final
        int x; //posicion X en pantalla. Normalmente todos van separados dos caracteres excepto Symbol Shift, Enter etc que ocupan mas
        int ancho_tecla; //1 para todas, excepto enter, cap shift, etc
        };



        typedef struct s_osd_teclas osd_teclas;
/*

01234567890123456789012345

1 2 3 4 5 6 7 8 9 0
 Q W E R T Y U I O P
 A S D F G H J K L ENT
CS Z X C V B N M SS SP
*/

        //Primeras 10 teclas son de la primera linea, siguientes 10, siguiente linea, etc
        osd_teclas teclas_osd[40]={
        {"1",0,1}, {"2",2,1}, {"3",4,1}, {"4",6,1}, {"5",8,1}, {"6",10,1}, {"7",12,1}, {"8",14,1}, {"9",16,1}, {"0",18,1},
 {"Q",1,1}, {"W",3,1}, {"E",5,1}, {"R",7,1}, {"T",9,1}, {"Y",11,1}, {"U",13,1}, {"I",15,1}, {"O",17,1}, {"P",19,1},
 {"A",1,1}, {"S",3,1}, {"D",5,1}, {"F",7,1}, {"G",9,1}, {"H",11,1}, {"J",13,1}, {"K",15,1}, {"L",17,1}, {"ENT",19,3},
{"CS",0,2}, {"Z",3,1}, {"X",5,1}, {"C",7,1}, {"V",9,1}, {"B",11,1}, {"N",13,1}, {"M",15,1}, {"SS",17,2}, {"SP",20,2}
        };


int osd_keyboard_cursor_x=0; //Vale 0..9
int osd_keyboard_cursor_y=0; //Vale 0..3

int menu_onscreen_keyboard_return_index_cursor(void)
{
	return osd_keyboard_cursor_y*10+osd_keyboard_cursor_x;
}


//Comun para dibujar cursor y senyalar teclas activas
void menu_onscreen_keyboard_dibuja_cursor_aux(zxvision_window *ventana,char *s,int x,int y,int escursor)
{
	char *textocursor;
	char textospeech[32];
	textocursor=s;

        //Parchecillo para ZX80/81
        if (MACHINE_IS_ZX8081) {
		if (!strcmp(s,"SS")) textocursor=".";
		if (!strcmp(s,"ENT")) textocursor="NL";
        }


        //Si es teclas activas, texto inverso. Si es cursor, texto papel de color seleccion no disponible (rojo por defecto)
        if (escursor) {
		//menu_escribe_texto_ventana(x,y,ESTILO_GUI_TINTA_SELECCIONADO,ESTILO_GUI_PAPEL_SELECCIONADO,textocursor);	
		//void zxvision_print_string(zxvision_window *w,int x,int y,int tinta,int papel,int parpadeo,char *texto)
		zxvision_print_string(ventana,x,y,ESTILO_GUI_TINTA_SELECCIONADO,ESTILO_GUI_PAPEL_SELECCIONADO,0,textocursor);
	}

	else {
		//menu_escribe_texto_ventana(x,y,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,textocursor);	
		zxvision_print_string(ventana,x,y,ESTILO_GUI_TINTA_OPCION_MARCADA,ESTILO_GUI_PAPEL_OPCION_MARCADA,0,textocursor);
	}

	

	//Enviar a speech
	string_a_minusculas(textocursor,textospeech);

	//Casos especiales para speech
	if (!strcmp(textospeech,".")) strcpy (textospeech,"dot");
	else if (!strcmp(textospeech,"cs")) strcpy (textospeech,"caps shift");
	else if (!strcmp(textospeech,"ss")) strcpy (textospeech,"symbol shift");
	else if (!strcmp(textospeech,"ent")) strcpy (textospeech,"enter");
	else if (!strcmp(textospeech,"sp")) strcpy (textospeech,"space");

	//Forzar que siempre suene en speech
	menu_speech_tecla_pulsada=0;


	menu_textspeech_send_text(textospeech);
}

void menu_onscreen_keyboard_dibuja_cursor(zxvision_window *ventana)
{


	int offset_x=1;
	int offset_y=1;

	//Calcular posicion del cursor
	int x,y;
	y=osd_keyboard_cursor_y*2;
	//la x hay que buscarla en el array



	int indice=menu_onscreen_keyboard_return_index_cursor();

	//Si en stick
	if (osd_keyboard_cursor_y==4) {
		if (indice==40) menu_onscreen_keyboard_dibuja_cursor_aux(ventana,"Stick",offset_x,offset_y+y,1);
		else menu_onscreen_keyboard_dibuja_cursor_aux(ventana,"Send",offset_x+6,offset_y+y,1);
		return;
	}

	x=teclas_osd[indice].x;

	menu_onscreen_keyboard_dibuja_cursor_aux(ventana,teclas_osd[indice].tecla,offset_x+x,offset_y+y,1);

	



}


//Que teclas se estan pulsando. A 0 no tecla, a 1 tecla
z80_byte menu_osd_teclas_pulsadas[40];

int menu_onscreen_keyboard_sticky=0;


void menu_onscreen_keyboard_dibuja_teclas_activas(zxvision_window *ventana)
{
	int i,x,y;

	int offset_x=1;
	int offset_y=1;


	for (i=0;i<40;i++) {
		y=(i/10)*2;
		if (menu_osd_teclas_pulsadas[i]) {
			x=teclas_osd[i].x;
			menu_onscreen_keyboard_dibuja_cursor_aux(ventana,teclas_osd[i].tecla,offset_x+x,offset_y+y,0);
		}
	}

	if (menu_onscreen_keyboard_sticky) menu_onscreen_keyboard_dibuja_cursor_aux(ventana,"Stick",offset_x,offset_y+(40/10)*2,0);
}

void menu_onscreen_keyboard_reset_pressed_keys(void)
{
	int i;
	for (i=0;i<40;i++) menu_osd_teclas_pulsadas[i]=0;
}

void menu_osd_send_key_text(char *texto_tecla)
{
	//Enviar tecla ascii tal cual, excepto Enter, CS, SS, SP
		if (strlen(texto_tecla)==1) {
			z80_byte tecla=letra_minuscula(texto_tecla[0]);
			debug_printf (VERBOSE_DEBUG,"Sending Ascii key: %c",tecla);
			convert_numeros_letras_puerto_teclado_continue(tecla,1);
		}
		else {
/*
 A S D F G H J K L ENT
CS Z X C V B N M SS SP
*/
			if (!strcmp(texto_tecla,"ENT")) util_set_reset_key(UTIL_KEY_ENTER,1);

			//CS y SS los enviamos asi porque en algunos teclados, util_set_reset_key los gestiona diferente y no queremos

			else if (!strcmp(texto_tecla,"SS")) {
				if (MACHINE_IS_ZX8081) {
					puerto_32766 &=255-2;
				}
				else util_set_reset_key(UTIL_KEY_ALT_L,1);
			}


			else if (!strcmp(texto_tecla,"SP")) util_set_reset_key(UTIL_KEY_SPACE,1);


			//Si estan los flags de CS o SS, activarlos
			else if (!strcmp(texto_tecla,"CS")) util_set_reset_key(UTIL_KEY_SHIFT_L,1);
		//if (menu_button_osdkeyboard_caps.v) puerto_65278  &=255-1;
		//if (menu_button_osdkeyboard_symbol.v) puerto_32766 &=255-2;



		}
}

int menu_onscreen_send_enter_check_exit(z80_byte tecla)
{


	if (tecla==2) {
		//printf ("exit with ESC\n");

		return 1; //Salir con ESC
	}

	int indice;
	int salir=0;

	indice=menu_onscreen_keyboard_return_index_cursor();
	if (indice==40) {
					//En sticky
					menu_onscreen_keyboard_sticky ^=1;

					if (!menu_onscreen_keyboard_sticky) menu_onscreen_keyboard_reset_pressed_keys();
	}

	else {


		//Si esta en modo sticky, agregar tecla a la lista y enviar todas
		//Si no, solo enviar una tecla
		if (!menu_onscreen_keyboard_sticky) {
			menu_onscreen_keyboard_reset_pressed_keys();
		}

		//Agregar esa tecla al array, siempre que no sea orden "send"
	        indice=menu_onscreen_keyboard_return_index_cursor();

	        if (indice!=41) {
			menu_osd_teclas_pulsadas[indice] ^=1;
			if (menu_osd_teclas_pulsadas[indice]) debug_printf (VERBOSE_DEBUG,"Adding key %s",teclas_osd[indice].tecla);
			else debug_printf (VERBOSE_DEBUG,"Clearing key %s",teclas_osd[indice].tecla);
		}

		//Si es modo stick, solo enviar cuando pulsar "Send"
		//Si no modo stick, enviar la que haya
		//int enviar=1;
		salir=1;
		if (menu_onscreen_keyboard_sticky && indice!=41) {
			salir=0;
			//enviar=0;
		}

	}


	return salir;

}

void menu_onscreen_keyboard(MENU_ITEM_PARAMETERS)
{
	//Si maquina no es Spectrum o zx80/81, volver
	if (!MACHINE_IS_SPECTRUM && !MACHINE_IS_ZX8081) return;

	
	//Evitar que se pueda llamar al mismo osd desde aqui dentro
	int antes_osd_kb_no_mostrar_desde_menu=osd_kb_no_mostrar_desde_menu;
	osd_kb_no_mostrar_desde_menu=1;


	if (!menu_onscreen_keyboard_sticky) menu_onscreen_keyboard_reset_pressed_keys();


	zxvision_window ventana;

	



        zxvision_new_window(&ventana,OSD_KEYBOARD_X_VENTANA,OSD_KEYBOARD_Y_VENTANA,OSD_KEYBOARD_ANCHO_VENTANA,OSD_KEYBOARD_ALTO_VENTANA,
				OSD_KEYBOARD_ANCHO_VENTANA-1,OSD_KEYBOARD_ALTO_VENTANA-2,"On Screen Keyboard");

        zxvision_draw_window(&ventana);

	z80_byte tecla;

	int salir=0;

	//int indice;

	do {

	        int linea=1;

				//01234567890123456789012345678901
        	char textoventana[32];

		int fila_tecla;
		int pos_tecla_en_fila;

		int indice_tecla=0;

		for (fila_tecla=0;fila_tecla<4;fila_tecla++) {
		//Inicializar texto linea con 31 espacios
					  //1234567890123456789012345678901
			sprintf (textoventana,"%s","                               ");
			for (pos_tecla_en_fila=0;pos_tecla_en_fila<10;pos_tecla_en_fila++) {
				//Copiar texto tecla a buffer linea
				int len=teclas_osd[indice_tecla].ancho_tecla;
				int tecla_x=teclas_osd[indice_tecla].x;
				int i;

				//parchecillo para ZX80/81. Si es Symbol Shift, Realmente es el .
				if (MACHINE_IS_ZX8081 && !strcmp(teclas_osd[indice_tecla].tecla,"SS")) {
					textoventana[tecla_x+0]='.';
					textoventana[tecla_x+1]=' ';  //Como ancho de tecla realmente es dos, meter un espacio
				}

				//parchecillo para ZX80/81. Si es ENT, Realmente es NL
				else if (MACHINE_IS_ZX8081 && !strcmp(teclas_osd[indice_tecla].tecla,"ENT")) {
					textoventana[tecla_x+0]='N';
					textoventana[tecla_x+1]='L';
					textoventana[tecla_x+2]=' ';  //Como ancho de tecla realmente es tres, meter un espacio
				}

				else for (i=0;i<len;i++) {
					textoventana[tecla_x+i+0]=teclas_osd[indice_tecla].tecla[i];
				}

				indice_tecla++;
			}

			//Meter final de cadena
			textoventana[OSD_KEYBOARD_ANCHO_VENTANA-2]=0;


			//No queremos que se envie cada linea a speech
			z80_bit old_textspeech_also_send_menu;
			old_textspeech_also_send_menu.v=textspeech_also_send_menu.v;
			textspeech_also_send_menu.v=0;

	        	//menu_escribe_linea_opcion(linea++,-1,1,textoventana);
			zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoventana);


			//Restaurar parametro speech
			textspeech_also_send_menu.v=old_textspeech_also_send_menu.v;

			linea++;
		}

		//No quiero que envie ni "Stick" ni "Send" a speech
		z80_bit old_textspeech_also_send_menu;
		old_textspeech_also_send_menu.v=textspeech_also_send_menu.v;
		textspeech_also_send_menu.v=0;

		//menu_escribe_linea_opcion(linea++,-1,1,"Stick Send");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"Stick Send");

		//Restaurar parametro speech
		textspeech_also_send_menu.v=old_textspeech_also_send_menu.v;


		menu_onscreen_keyboard_dibuja_teclas_activas(&ventana);

		menu_onscreen_keyboard_dibuja_cursor(&ventana);


		zxvision_draw_window_contents(&ventana);



       		/*if (menu_multitarea==0) menu_refresca_pantalla();
		menu_espera_tecla();

		tecla=menu_get_pressed_key();

		menu_espera_no_tecla_con_repeticion();*/

		tecla=zxvision_common_getkey_refresh();

		//tambien permitir mover con joystick aunque no este mapeado a cursor key
		switch (tecla) {
			case 11:
				//arriba
				if (osd_keyboard_cursor_y>0) osd_keyboard_cursor_y--;
			break;

			case 10:
				//abajo
				if (osd_keyboard_cursor_y<4) osd_keyboard_cursor_y++;
				if (osd_keyboard_cursor_y==4) osd_keyboard_cursor_x=0;  //Cursor en sticky
			break;

			case 8:
				//izquierda
				if (osd_keyboard_cursor_x>0) osd_keyboard_cursor_x--;
			break;

			case 9:
				//derecha
				if (osd_keyboard_cursor_y==4) {  //Si en linea inferior
					if (osd_keyboard_cursor_x<1) osd_keyboard_cursor_x++;
				}
				else if (osd_keyboard_cursor_x<9) osd_keyboard_cursor_x++;
			break;

			case 2: //ESC
			case 13: //Enter
				salir=menu_onscreen_send_enter_check_exit(tecla);
				
			break;
		}

	} while (salir==0);

	menu_espera_no_tecla();

	//Si salido con Enter o Fire joystick
	if (tecla==13) {
	
			//Liberar otras teclas, por si acaso
			reset_keyboard_ports();
			int i;
			for (i=0;i<40;i++) {
				if (menu_osd_teclas_pulsadas[i]) {
					//printf ("Sending key %s\n",teclas_osd[i].tecla);
					menu_osd_send_key_text(teclas_osd[i].tecla);
				}
			}


			//printf ("Exiting and sending\n");

			salir_todos_menus=1;
			timer_on_screen_key=25; //durante medio segundo
		

	}

	cls_menu_overlay();
        zxvision_destroy_window(&ventana);

	//Si no se ha salido con escape, hacer que vuelva y quitar pulsaciones de caps y symbol
	if (tecla!=2) {
		menu_button_osdkeyboard_return.v=1;
	}
	else {
		//se sale con esc, quitar pulsaciones de caps y symbol
		menu_onscreen_keyboard_reset_pressed_keys();
		menu_onscreen_keyboard_sticky=0;

	}

	osd_kb_no_mostrar_desde_menu=antes_osd_kb_no_mostrar_desde_menu;

}





void menu_hardware_sam_ram(MENU_ITEM_PARAMETERS)
{
	if (sam_memoria_total_mascara==15) sam_memoria_total_mascara=31;
	else sam_memoria_total_mascara=15;
}






void menu_cpu_speed(MENU_ITEM_PARAMETERS)
{

        char string_speed[5];

        sprintf (string_speed,"%d",porcentaje_velocidad_emulador);

        menu_ventana_scanf("Emulator Speed (%)",string_speed,5);

        porcentaje_velocidad_emulador=parse_string_to_number(string_speed);
        if (porcentaje_velocidad_emulador<1 || porcentaje_velocidad_emulador>9999) porcentaje_velocidad_emulador=100;

	set_emulator_speed();

}





void menu_multiface_rom_file(MENU_ITEM_PARAMETERS)
{
	multiface_disable();

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select multiface File",filtros,multiface_rom_file_name)==1) {
                if (!si_existe_archivo(multiface_rom_file_name)) {
                        menu_error_message("File does not exist");
                        multiface_rom_file_name[0]=0;
                        return;



                }

                else {
                        //Comprobar aqui tambien el tamanyo
                        long int size=get_file_size(multiface_rom_file_name);
                        if (size!=8192) {
                                menu_error_message("ROM file must be 8 KB length");
                                multiface_rom_file_name[0]=0;
                                return;
                        }
                }


        }
        //Sale con ESC
        else {
                //Quitar nombre
                multiface_rom_file_name[0]=0;


        }

}


void menu_hardware_multiface_enable(MENU_ITEM_PARAMETERS)
{
	if (multiface_enabled.v) multiface_disable();
	else multiface_enable();
}

void menu_hardware_multiface_type(MENU_ITEM_PARAMETERS)
{
  multiface_type++;
  if (multiface_type==MULTIFACE_TOTAL_TYPES)  multiface_type=0;
}

int menu_hardware_multiface_type_cond(void)
{
	//En tbblue no se puede cambiar el tipo
	if (MACHINE_IS_TBBLUE) return 0;

	return !multiface_enabled.v;
}

void menu_multiface(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_multiface;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_multiface_file_shown[13];

				//Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
                menu_add_item_menu_inicial(&array_menu_multiface,"",MENU_OPCION_UNASSIGNED,NULL,NULL);
					//En tbblue, la rom no es seleccionable, la carga el mismo en la sdram

					if (!MACHINE_IS_TBBLUE) {
                        menu_tape_settings_trunc_name(multiface_rom_file_name,string_multiface_file_shown,13);
                        menu_add_item_menu_format(array_menu_multiface,MENU_OPCION_NORMAL,menu_multiface_rom_file,NULL,"~~ROM File [%s]",string_multiface_file_shown);
                        menu_add_item_menu_shortcut(array_menu_multiface,'r');
                        menu_add_item_menu_tooltip(array_menu_multiface,"ROM Emulation file");
                        menu_add_item_menu_ayuda(array_menu_multiface,"ROM Emulation file");
					}


                        menu_add_item_menu_format(array_menu_multiface,MENU_OPCION_NORMAL,menu_hardware_multiface_type,menu_hardware_multiface_type_cond,"~~Type [%s]",multiface_types_string[multiface_type]);
                  menu_add_item_menu_shortcut(array_menu_multiface,'t');
                  menu_add_item_menu_tooltip(array_menu_multiface,"Multiface type. You must first disable it if you want to change type");
                  menu_add_item_menu_ayuda(array_menu_multiface,"Multiface type. You must first disable it if you want to change type");


                        			menu_add_item_menu_format(array_menu_multiface,MENU_OPCION_NORMAL,menu_hardware_multiface_enable,NULL,"[%c] ~~Multiface Enabled", (multiface_enabled.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_multiface,'m');
                        menu_add_item_menu_tooltip(array_menu_multiface,"Enable multiface");
                        menu_add_item_menu_ayuda(array_menu_multiface,"Enable multiface");





                                menu_add_item_menu(array_menu_multiface,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_multiface);

                retorno_menu=menu_dibuja_menu(&multiface_opcion_seleccionada,&item_seleccionado,array_menu_multiface,"Multiface settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_hardware_set_f_func_action(MENU_ITEM_PARAMETERS)
{
        hardware_set_f_func_action_opcion_seleccionada=defined_f_functions_keys_array[valor_opcion];


        menu_item *array_menu_hardware_set_f_func_action;
        menu_item item_seleccionado;
        int retorno_menu;
        //do {

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_F_FUNCTIONS;i++) {

                  //enum defined_f_function_ids accion=defined_f_functions_keys_array[i];

                  sprintf (buffer_texto,"%s",defined_f_functions_array[i].texto_funcion);


                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_set_f_func_action,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_set_f_func_action,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

												}


                menu_add_item_menu(array_menu_hardware_set_f_func_action,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_set_f_func_action,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_set_f_func_action);

                retorno_menu=menu_dibuja_menu(&hardware_set_f_func_action_opcion_seleccionada,&item_seleccionado,array_menu_hardware_set_f_func_action,"Set F keys" );

                


								if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

												//Si se pulsa Enter
												int indice=hardware_set_f_func_action_opcion_seleccionada;
												defined_f_functions_keys_array[valor_opcion]=indice;

												
                }

}



void menu_hardware_set_f_functions(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_set_f_functions;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char buffer_texto[40];

                int i;
                for (i=0;i<MAX_F_FUNCTIONS_KEYS;i++) {

                  enum defined_f_function_ids accion=defined_f_functions_keys_array[i];

					//tabulado todo a misma columna, agregamos un espacio con F entre 1 y 9
                  sprintf (buffer_texto,"Key F%d %s[%s]",i+1,(i+1<=9 ? " " : ""),defined_f_functions_array[accion].texto_funcion);




                        if (i==0) menu_add_item_menu_inicial_format(&array_menu_hardware_set_f_functions,MENU_OPCION_NORMAL,menu_hardware_set_f_func_action,NULL,buffer_texto);
                        else menu_add_item_menu_format(array_menu_hardware_set_f_functions,MENU_OPCION_NORMAL,menu_hardware_set_f_func_action,NULL,buffer_texto);

												menu_add_item_menu_valor_opcion(array_menu_hardware_set_f_functions,i);

                        //menu_add_item_menu_tooltip(array_menu_hardware_set_f_functions,"Redefine the key");
                        //menu_add_item_menu_ayuda(array_menu_hardware_set_f_functions,"Indicates which key on the Spectrum keyboard is sent when "
                          //                      "pressed the original key");
												}



                menu_add_item_menu(array_menu_hardware_set_f_functions,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_set_f_functions,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_hardware_set_f_functions);

                retorno_menu=menu_dibuja_menu(&hardware_set_f_functions_opcion_seleccionada,&item_seleccionado,array_menu_hardware_set_f_functions,"Set F keys" );

                


								if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



void menu_hardware_spectrum_keyboard_matrix_error(MENU_ITEM_PARAMETERS)
{
	keyboard_matrix_error.v ^=1;
}

void menu_hardware_recreated_keyboard(MENU_ITEM_PARAMETERS)
{
	recreated_zx_keyboard_support.v ^=1;
}

void menu_hardware_sdl_raw_read(MENU_ITEM_PARAMETERS)
{
	sdl_raw_keyboard_read.v ^=1;
}

//menu keyboard settings
void menu_keyboard_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_keyboard_settings;
	menu_item item_seleccionado;
	int retorno_menu;
        do {

		menu_add_item_menu_inicial(&array_menu_keyboard_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);	




		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keyboard_issue,menu_hardware_keyboard_issue_cond,"[%c] Keyboard ~~Issue", (keyboard_issue2.v==1 ? '2' : '3'));
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'i');
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Type of Spectrum keyboard emulated");
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Changes the way the Spectrum keyboard port returns its value: Issue 3 returns bit 6 off, and Issue 2 has bit 6 on");
		}


		//Soporte para Azerty keyboard

		if (!strcmp(scr_driver_name,"xwindows")) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_azerty,NULL,"[%c] ~~Azerty keyboard",(azerty_keyboard.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'a');
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables azerty keyboard");
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Only used on xwindows driver by now. Enables to use numeric keys on Azerty keyboard, without having "
						"to press Shift. Note we are referring to the numeric keys (up to letter A, Z, etc) and not to the numeric keypad.");
		}


		menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_recreated_keyboard,NULL,"[%c] ZX Recreated support",
			(recreated_zx_keyboard_support.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables ZX Recreated support. Press F1 to see details");
		menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Enables ZX Recreated support. You have to consider the following:\n"
						"- It supports Game Mode/Layer A on ZX Recreated. QWERTY mode/Layer B works like a normal keyboard\n"
						"- You must use the ZX Recreated only on the machine emulated, not on the menu\n"
						"- You must use your normal PC keyboard on the menu\n"
						"- I can't distinguish between normal keyboard and ZX Recreated keyboard key press. "
						"So if you enable ZX Recreated support and press keys on your normal PC keyboard, out of the menu, will produce strange combination of keys. "
						"If you press keys on the ZX Recreated on the menu, will produce strange combination of keys too. "
						"On the other hand, if you have ZX Recreated support disabled, and press keys on the ZX Recreated, will produce strange combination of keys too.\n"
						"- If you use Mac OS X, you are probably using the Cocoa driver, so it will work only by enabling this setting\n"
						"- If you use Linux, you should use the SDL1 or SDL2 video driver, and also enable the SDL Raw keyboard setting. "
						"It won't work well using other video drivers (last row of keys will fail)\n"
						"- If you use Windows, you are probably using the SDL1 or SDL2 video driver, so same behaviour as Linux: you must also enable the SDL Raw keyboard setting"
		);

#ifdef COMPILE_SDL
		if (!strcmp(scr_driver_name,"sdl")) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_sdl_raw_read,NULL,"[%c] SDL Raw keyboard",
				(sdl_raw_keyboard_read.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Read the keyboard using raw mode. Needed for ZX Recreated to work");
				menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Read the keyboard using raw mode. Needed for ZX Recreated to work");
		}
#endif


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_chloe_keyboard,NULL,"[%c] Chloe Keyboard",(chloe_keyboard.v ? 'X' : ' ') );
		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_spectrum_keyboard_matrix_error,NULL,"[%c] Speccy keyb. ~~ghosting",
					(keyboard_matrix_error.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Enables real keyboard emulation, even with the keyboard matrix error");
			menu_add_item_menu_shortcut(array_menu_keyboard_settings,'g');
			menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Enables real keyboard emulation, even with the keyboard matrix error.\n"
						"This is the error/feature that returns more key pressed than the real situation, for example, "
						"pressing keys ASQ, will return ASQW. Using a pc keyboard is difficult to test that effect, because "
						"that most of them can return more than two or three keys pressed at a time. But using the on-screen keyboard "
						"and also the Recreated Keyboard, you can test it");

		}

		if (MACHINE_IS_Z88 || MACHINE_IS_CPC || chloe_keyboard.v || MACHINE_IS_SAM || MACHINE_IS_QL)  {
			//keymap solo hace falta con xwindows y sdl. fbdev y cocoa siempre leen en raw como teclado english
			if (!strcmp(scr_driver_name,"xwindows")  || !strcmp(scr_driver_name,"sdl") ) {
				if (MACHINE_IS_Z88) menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"Z88 K~~eymap [%s]",(z88_cpc_keymap_type == 1 ? "Spanish" : "Default" ));
				else if (MACHINE_IS_CPC) menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"CPC K~~eymap [%s]",(z88_cpc_keymap_type == 1 ? "Spanish" : "Default" ));
				else if (MACHINE_IS_SAM) menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"SAM K~~eymap [%s]",(z88_cpc_keymap_type == 1 ? "Spanish" : "Default" ));
				else if (MACHINE_IS_QL) menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"QL K~~eymap [%s]",(z88_cpc_keymap_type == 1 ? "Spanish" : "Default" ));

				else menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_keymap_z88_cpc,NULL,"Chloe K~~eymap [%s]",(z88_cpc_keymap_type == 1 ? "Spanish" : "Default" ));
				menu_add_item_menu_shortcut(array_menu_keyboard_settings,'e');
				menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Keyboard Layout");
				menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Used on Z88, CPC, Sam and Chloe machines, needed to map symbol keys. "
						"You must indicate here which kind of physical keyboard you have. The keyboard will "
						"be mapped always to a Z88/CPC/Sam/Chloe English keyboard, to the absolute positions of the keys. "
						"You have two physical keyboard choices: Default (English) and Spanish");

			}
		}


                menu_add_item_menu(array_menu_keyboard_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

        	//Redefine keys
		menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_redefine_keys,NULL,"~~Redefine keys");
		menu_add_item_menu_shortcut(array_menu_keyboard_settings,'r');
		menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Redefine one key to another");
		menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Redefine one key to another");


		//Set F keys functions
		menu_add_item_menu_format(array_menu_keyboard_settings,MENU_OPCION_NORMAL,menu_hardware_set_f_functions,NULL,"Set ~~F keys functions");
		menu_add_item_menu_shortcut(array_menu_keyboard_settings,'f');
		menu_add_item_menu_tooltip(array_menu_keyboard_settings,"Assign actions to F keys");
		menu_add_item_menu_ayuda(array_menu_keyboard_settings,"Assign actions to F keys");







                menu_add_item_menu(array_menu_keyboard_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_keyboard_settings);

                retorno_menu=menu_dibuja_menu(&keyboard_settings_opcion_seleccionada,&item_seleccionado,array_menu_keyboard_settings,"Keyboard Settings" );

                
		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
	                if (item_seleccionado.menu_funcion!=NULL) {
        	                //printf ("actuamos por funcion\n");
                	        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
	                }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_hardware_datagear_dma(MENU_ITEM_PARAMETERS)
{
	if (datagear_dma_emulation.v) datagear_dma_disable();
	else datagear_dma_enable();
}

void menu_hardware_kempston_mouse_sensibilidad(MENU_ITEM_PARAMETERS)
{
	//kempston_mouse_factor_sensibilidad++;
	//if (kempston_mouse_factor_sensibilidad==6) kempston_mouse_factor_sensibilidad=1;
	char titulo_ventana[33];
	sprintf (titulo_ventana,"Sensitivity (1-%d)",MAX_KMOUSE_SENSITIVITY);

	menu_hardware_advanced_input_value(1,MAX_KMOUSE_SENSITIVITY,titulo_ventana,&kempston_mouse_factor_sensibilidad);
}

void menu_tbblue_fast_boot_mode(MENU_ITEM_PARAMETERS)
{
	tbblue_fast_boot_mode.v ^=1;
}

//menu hardware settings
void menu_hardware_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_settings;
	menu_item item_seleccionado;
	int retorno_menu;
        do {

			menu_add_item_menu_inicial(&array_menu_hardware_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);





		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_SAM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_joystick,NULL,"~~Joystick type [%s]",joystick_texto[joystick_emulation]);
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'j');
        	        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Decide which joystick type is emulated");
                	menu_add_item_menu_ayuda(array_menu_hardware_settings,"Joystick is emulated with:\n"
					"-A real joystick connected to an USB port\n"
					"-Cursor keys on the keyboard for the directions and Home key for fire"
			);


	                if (joystick_autofire_frequency==0) menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_autofire,menu_hardware_autofire_cond,"[ ] ~~Autofire");
        	        else {
                	        menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_autofire,NULL,"[%d Hz] ~~Autofire",50/joystick_autofire_frequency);
	                }

			menu_add_item_menu_shortcut(array_menu_hardware_settings,'a');

        	        menu_add_item_menu_tooltip(array_menu_hardware_settings,"Frequency for the joystick autofire");
                	menu_add_item_menu_ayuda(array_menu_hardware_settings,"Times per second (Hz) the joystick fire is auto-switched from pressed to not pressed and viceversa. "
                                        "Autofire can only be enabled on Kempston, Fuller, Zebra and Mikrogen; Sinclair, Cursor, and OPQA can not have "
                                        "autofire because this function can interfiere with the menu (it might think a key is pressed)");


		
		}


		if (MACHINE_IS_SPECTRUM) {

			if (gunstick_emulation==0) menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick,NULL,"[ ] ~~Lightgun");
			else menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick,NULL,"[%s] ~~Lightgun",gunstick_texto[gunstick_emulation]);
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'l');
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Decide which kind of lightgun is emulated with the mouse");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Lightgun emulation supports the following two models:\n\n"
					"Gunstick from MHT Ingenieros S.L: all types except AYChip\n\n"
					"Magnum Light Phaser (experimental): supported by AYChip type");


			if (menu_hardware_gunstick_aychip_cond()) {
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_range_x,NULL," X Range: %d",gunstick_range_x);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_range_y,NULL," Y Range: %d",gunstick_range_y);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_y_offset,NULL," Y Offset: %s%d",(gunstick_y_offset ? "-" : "" ), gunstick_y_offset);
				menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_gunstick_solo_brillo,NULL," Detect only white bright: %s",(gunstick_solo_brillo ? "On" : "Off"));
		}


			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_kempston_mouse,NULL,"[%c] Kempston Mou~~se emulation",(kempston_mouse_emulation.v==1 ? 'X' : ' '));

			menu_add_item_menu_shortcut(array_menu_hardware_settings,'s');

			if (kempston_mouse_emulation.v) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_kempston_mouse_sensibilidad,NULL,"    Mouse Sensitivity: %d",kempston_mouse_factor_sensibilidad);
			}

		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_datagear_dma,NULL,"[%c] Datagear DMA emulation",(datagear_dma_emulation.v==1 ? 'X' : ' '));
		}		


  		if (MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_tbblue_fast_boot_mode,NULL,"[%c] TBBlue fast boot mode",
			(tbblue_fast_boot_mode.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Boots tbblue directly to a 48 rom but with all the Next features enabled (except divmmc)");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Boots tbblue directly to a 48 rom but with all the Next features enabled (except divmmc)");

			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_tbblue_machine_id,NULL,"[%d] TBBlue machine id",tbblue_machine_id); 

		}

		menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_cpu_speed,NULL,"Emulator Spee~~d [%d%%]",porcentaje_velocidad_emulador);
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'d');
		menu_add_item_menu_tooltip(array_menu_hardware_settings,"Change the emulator Speed");
		menu_add_item_menu_ayuda(array_menu_hardware_settings,"Changes all the emulator speed by setting a different interval between display frames. "
		"Also changes audio frequency");		

		menu_add_item_menu(array_menu_hardware_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		//Keyboard settings
		menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_keyboard_settings,NULL,"~~Keyboard settings");
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'k');
		menu_add_item_menu_tooltip(array_menu_hardware_settings,"Hardware settings");
		menu_add_item_menu_ayuda(array_menu_hardware_settings,"Hardware settings");		



	if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_SAM) {
	
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_realjoystick,menu_hardware_realjoystick_cond,"~~Real joystick emulation");
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'r');
			menu_add_item_menu_tooltip(array_menu_hardware_settings,"Settings for the real joystick");
			menu_add_item_menu_ayuda(array_menu_hardware_settings,"Settings for the real joystick");

		}




		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX81) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_printers,NULL,"~~Printing emulation");
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'p');
		}

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_multiface,NULL,"M~~ultiface emulation"  );
			menu_add_item_menu_shortcut(array_menu_hardware_settings,'u');
		}

		


		menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_hardware_memory_settings,NULL,"~~Memory Settings");
		menu_add_item_menu_shortcut(array_menu_hardware_settings,'m');




		/* De momento esto desactivado
		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_hardware_settings,MENU_OPCION_NORMAL,menu_if1_settings,NULL,"Interface 1: %s",(if1_enabled.v ? "Yes" : "No") );
		}
		*/




                menu_add_item_menu(array_menu_hardware_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_hardware_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_hardware_settings);

                retorno_menu=menu_dibuja_menu(&hardware_settings_opcion_seleccionada,&item_seleccionado,array_menu_hardware_settings,"Hardware Settings" );

                
		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
	                if (item_seleccionado.menu_funcion!=NULL) {
        	                //printf ("actuamos por funcion\n");
                	        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
	                }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_hardware_memory_128k_multiplier(MENU_ITEM_PARAMETERS)
{

	z80_byte valor=mem128_multiplicador;

	if (valor==8) valor=1;
	else valor <<=1;

	mem_set_multiplicador_128(valor);
}

void menu_hardware_tbblue_ram(MENU_ITEM_PARAMETERS)
{
	if (tbblue_extra_512kb_blocks==3) tbblue_extra_512kb_blocks=0;
	else tbblue_extra_512kb_blocks++;
}



//menu hardware settings
void menu_hardware_memory_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_hardware_memory_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_allow_write_rom,menu_cond_allow_write_rom,"[%c] Allow ~~write in ROM",
			(allow_write_rom.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_hardware_memory_settings,'w');
		menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Allow write in ROM");
		menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Allow write in ROM. Only allowed on Spectrum 48k/16k models, ZX80, ZX81, Sam Coupe and Jupiter Ace (and not on Inves)");

		if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_memory_128k_multiplier,NULL,"RAM size [%4d KB]",128*mem128_multiplicador);
			menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Allows setting more than 128k RAM on a 128k type machine");
			menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Allows setting more than 128k RAM on a 128k type machine");
		}

		if (MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_tbblue_ram,NULL,"RAM size [%d KB]",tbblue_get_current_ram() );
		}

		if (menu_cond_zx8081() ) {

                        //int ram_zx8081=(ramtop_zx8081-16383)/1024;
												int ram_zx8081=zx8081_get_standard_ram();
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ramtop,menu_cond_zx8081,"ZX80/81 Standard RAM [%d KB]",ram_zx8081);
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Standard RAM for the ZX80/81");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"Standard RAM for the ZX80/81");


                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_8192,menu_cond_zx8081,"[%c] ZX80/81 8K RAM in 2000H", (ram_in_8192.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"8KB RAM at address 2000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"8KB RAM at address 2000H. Used on some wrx games");

                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_32768,menu_cond_zx8081,"[%c] ZX80/81 16K RAM in 8000H", (ram_in_32768.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"16KB RAM at address 8000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"16KB RAM at address 8000H");
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_zx8081_ram_in_49152,menu_cond_zx8081,"[%c] ZX80/81 16K RAM in C000H", (ram_in_49152.v ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"16KB RAM at address C000H");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"16KB RAM at address C000H. It requires the previous RAM at 8000H");


                }


		if (MACHINE_IS_SAM) {
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_sam_ram,NULL,"Sam Coupe RAM [%d KB]",(sam_memoria_total_mascara==15 ? 256 : 512) );
		}


		if (MACHINE_IS_ACE) {
			//int ram_ace=((ramtop_ace-16383)/1024)+3;
			int ram_ace=get_ram_ace();
			menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_ace_ramtop,NULL,"Jupiter Ace RAM [%d KB]",ram_ace);
		}



      if (MACHINE_IS_SPECTRUM_48) {
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_memory_refresh,NULL,"[%c] RAM Refresh emulation", (machine_emulate_memory_refresh==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Enable RAM R~~efresh emulation");
                        menu_add_item_menu_shortcut(array_menu_hardware_memory_settings,'e');
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"RAM Refresh emulation consists, in a real Spectrum 48k, "
                                        "to refresh the upper 32kb RAM using the R register. On a real Spectrum 48k, if you modify "
                                        "the R register very fast, you can lose RAM contents.\n"
                                        "This option emulates this behaviour, and sure you don't need to enable it on the 99.99 percent of the "
                                        "situations ;) ");
                }


	  if (MACHINE_IS_INVES) {
                        menu_add_item_menu_format(array_menu_hardware_memory_settings,MENU_OPCION_NORMAL,menu_hardware_inves_poke,menu_inves_cond,"Poke Inves Low RAM");
                        menu_add_item_menu_tooltip(array_menu_hardware_memory_settings,"Poke Inves low RAM");
                        menu_add_item_menu_ayuda(array_menu_hardware_memory_settings,"You can alter the way Inves work with ULA port (sound & border). "
                                        "You change here the contents of the low (hidden) RAM of the Inves (addresses 0-16383). Choosing this option "
                                        "is the same as poke at the whole low RAM addresses (0 until 16383). I suggest to poke with value 15 or 23 "
                                        "on games that you can not hear well the music: Lemmings, ATV, Batman caped crusader...");

                }




		menu_add_item_menu(array_menu_hardware_memory_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_hardware_memory_settings);

                retorno_menu=menu_dibuja_menu(&hardware_memory_settings_opcion_seleccionada,&item_seleccionado,array_menu_hardware_memory_settings,"Memory Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_tape_any_flag(MENU_ITEM_PARAMETERS)
{
	tape_any_flag_loading.v^=1;
}

int menu_tape_input_insert_cond(void)
{
	if (tapefile==NULL) return 0;
	else return 1;
}

void menu_tape_input_insert(MENU_ITEM_PARAMETERS)
{

	if (tapefile==NULL) return;

	if ((tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) {
		tap_open();
	}

	else {
		tap_close();
	}
}

//Esto de momento solo se llama desde una tecla F. Lo pongo aqui para que este cerca de las condiciones y acciones de insert
void menu_reinsert_tape(void)
{

	debug_printf(VERBOSE_DEBUG,"Running reinsert tape");

	if (!menu_tape_input_insert_cond() ) {
		debug_printf(VERBOSE_DEBUG,"No tape inserted to reinsert");
		return;
	}

	//Si esta insertada, expulsar
	if ((tape_loadsave_inserted & TAPE_LOAD_INSERTED)) {
		debug_printf(VERBOSE_DEBUG,"Ejecting tape");
		tap_close();
	}

	//E insertar
	debug_printf(VERBOSE_DEBUG,"Inserting tape");
	tap_open();
}

//truncar el nombre del archivo a un maximo
//si origen es NULL, poner en destino cadena vacia
//Si es 0 o menor que 0, devuelve siempre cadena vacia acabada en 0 (usa 1 byte)
//Esto se hace en algunos casos para menu_filesel en que se resta sobre el tamaño visible y puede ser <=0
void menu_tape_settings_trunc_name(char *orig,char *dest,int max)
{
	//printf ("max: %d\n",max);
	if (max<=0) {
		dest[0]=0;
		return;
	}
	//en maximo se incluye caracter 0 del final
	max--;

                if (orig!=0) {

                        int longitud=strlen(orig);
                        int indice=longitud-max;

			if (indice<0) indice=0;

                        strncpy(dest,&orig[indice],max);


			//printf ("copiamos %d max caracteres\n",max);

                        //si cadena es mayor, acabar con 0

			//en teoria max siempre es mayor de cero, pero por si acaso...
			if (max>0) dest[max]=0;

			//else printf ("max=%d\n",max);


			if (indice>0) dest[0]='<';

                }

             else {
                        strcpy(dest,"");
                }

}


int menu_tape_output_insert_cond(void)
{
        if (tape_out_file==NULL) return 0;
        else return 1;
}

void menu_tape_output_insert(MENU_ITEM_PARAMETERS)
{

        if (tape_out_file==NULL) return;

	if ((tape_loadsave_inserted & TAPE_SAVE_INSERTED)==0) {
                tap_out_open();
        }

        else {
                tap_out_close();
        }
}


//devuelve 1 si el directorio cumple el filtro
//realmente lo que hacemos aqui es ocultar/mostrar carpetas que empiezan con .
int menu_file_filter_dir(const char *name,char *filtros[])
{

        int i;
        //char extension[1024];

        //directorio ".." siempre se muestra
        if (!strcmp(name,"..")) return 1;

        char *f;


        //Bucle por cada filtro
        for (i=0;filtros[i];i++) {
                //si filtro es "", significa todo (*)
                //supuestamente si hay filtro "" no habrian mas filtros pasados en el array...

 	       f=filtros[i];
                if (f[0]==0) return 1;

                //si filtro no es *, ocultamos los que empiezan por "."
                if (name[0]=='.') return 0;

        }


	//y finalmente mostramos el directorio
        return 1;

}


#define MENU_LAST_DIR_FILE_NAME "zesarux_last_dir.txt"


//devuelve 1 si el archivo cumple el filtro
int menu_file_filter(const char *name,char *filtros[])
{

	int i;
	char extension[NAME_MAX];

	/*
	//obtener extension del nombre
	//buscar ultimo punto

	int j;
	j=strlen(name);
	if (j==0) extension[0]=0;
	else {
		for (;j>=0 && name[j]!='.';j--);

		if (j>=0) strcpy(extension,&name[j+1]);
		else extension[0]=0;
	}

	//printf ("Extension: %s\n",extension);

	*/

	//El archivo MENU_LAST_DIR_FILE_NAME zesarux_last_dir.txt usado para abrir archivos comprimidos, no lo mostrare nunca
	if (!strcmp(name,MENU_LAST_DIR_FILE_NAME)) return 0;

	util_get_file_extension((char *) name,extension);

	char *f;

	//Si filtro[0]=="nofiles" no muestra ningun archivo
	if (!strcasecmp(filtros[0],"nofiles")) return 0;

	//Bucle por cada filtro
	for (i=0;filtros[i];i++) {
		//si filtro es "", significa todo (*)
		//supuestamente si hay filtro "" no habrian mas filtros pasados en el array...

		f=filtros[i];
		//printf ("f: %d\n",f);
		if (f[0]==0) return 1;

		//si filtro no es *, ocultamos los que empiezan por "."
		//Aparentemente esto no tiene mucho sentido, con esto ocultariamos archivo de nombre tipo ".xxxx.tap" por ejemplo
		//Pero bueno, para volumenes que vienen de mac os x, los metadatos se guardan en archivos tipo:
		//._0186.tap
		if (name[0]=='.') return 0;


		//comparamos extension
		if (!strcasecmp(extension,f)) return 1;
	}

	//Si es zip, tambien lo soportamos
	if (!strcasecmp(extension,"zip")) return 1;

	//Si es gz, tambien lo soportamos
	if (!strcasecmp(extension,"gz")) return 1;

	//Si es tar, tambien lo soportamos
	if (!strcasecmp(extension,"tar")) return 1;

	//Si es rar, tambien lo soportamos
	if (!strcasecmp(extension,"rar")) return 1;

	//Si es mdv, tambien lo soportamos
	if (!strcasecmp(extension,"mdv")) return 1;

	//Si es hdf, tambien lo soportamos
	if (!strcasecmp(extension,"hdf")) return 1;

	//Si es dsk, tambien lo soportamos
	if (!strcasecmp(extension,"dsk")) return 1;

	//Si es tap, tambien lo soportamos
	if (!strcasecmp(extension,"tap")) return 1;	

	//Si es tzx, tambien lo soportamos
	if (!strcasecmp(extension,"tzx")) return 1;	

	//Si es trd, tambien lo soportamos
	if (!strcasecmp(extension,"trd")) return 1;		

	//Si es scl, tambien lo soportamos
	if (!strcasecmp(extension,"scl")) return 1;			

	//Si es epr, eprom o flash, tambien lo soportamos
	if (!strcasecmp(extension,"epr")) return 1;
	if (!strcasecmp(extension,"eprom")) return 1;
	if (!strcasecmp(extension,"flash")) return 1;		

	//NOTA: Aqui agregamos todas las extensiones que en principio pueden generar muchos diferentes tipos de archivos,
	//ya sea porque son archivos comprimidos (p.ej. zip) o porque son archivos que se pueden expandir (p.j. tap)
	//Hay algunos que se pueden expandir y directamente los excluyo (como .P o .O) por ser su uso muy limitado 
	//(solo generan .baszx80 y .baszx81 en este caso)

	return 0;

}

int menu_filesel_filter_func(const struct dirent *d)
{

	int tipo_archivo=get_file_type(d->d_type,(char *)d->d_name);


	//si es directorio, ver si empieza con . y segun el filtro activo
	//Y si setting no mostrar directorios, no mostrar
	if (tipo_archivo == 2) {
		if (menu_filesel_hide_dirs.v) return 0;
		if (menu_file_filter_dir(d->d_name,filesel_filtros)==1) return 1;
		return 0;
	}

	//Si no es archivo ni link, no ok

	if (tipo_archivo  == 0) {
		debug_printf (VERBOSE_DEBUG,"Item is not a directory, file or link. Type: %d",d->d_type);
		return 0;
	}

	//es un archivo. ver el nombre

	if (menu_file_filter(d->d_name,filesel_filtros)==1) return 1;


	return 0;
}

int menu_filesel_alphasort(const struct dirent **d1, const struct dirent **d2)
{

	//printf ("menu_filesel_alphasort %s %s\n",(*d1)->d_name,(*d2)->d_name );

	//compara nombre
	return (strcasecmp((*d1)->d_name,(*d2)->d_name));
}

int menu_filesel_readdir(void)
{

/*
       lowing macro constants for the value returned in d_type:

       DT_BLK      This is a block device.

       DT_CHR      This is a character device.

       DT_DIR      This is a directory.

       DT_FIFO     This is a named pipe (FIFO).

       DT_LNK      This is a symbolic link.

       DT_REG      This is a regular file.

       DT_SOCK     This is a UNIX domain socket.

       DT_UNKNOWN  The file type is unknown.

*/

debug_printf(VERBOSE_DEBUG,"Reading directory");

filesel_total_items=0;
primer_filesel_item=NULL;


    struct dirent **namelist;

	struct dirent *nombreactual;

    int n;
//printf ("usando scandir\n");

	filesel_item *item;
	filesel_item *itemanterior;


#ifndef MINGW
	n = scandir(".", &namelist, menu_filesel_filter_func, menu_filesel_alphasort);
#else
	//alternativa scandir, creada por mi
	n = scandir_mingw(".", &namelist, menu_filesel_filter_func, menu_filesel_alphasort);
#endif

    if (n < 0) {
		debug_printf (VERBOSE_ERR,"Error reading directory contents: %s",strerror(errno));
		return 1;
	}

    else {
        int i;

	//printf("total elementos directorio: %d\n",n);

        for (i=0;i<n;i++) {
		nombreactual=namelist[i];
            //printf("%s\n", nombreactual->d_name);
            //printf("%d\n", nombreactual->d_type);


		item=malloc(sizeof(filesel_item));
		if (item==NULL) cpu_panic("Error allocating file item");

		strcpy(item->d_name,nombreactual->d_name);

		item->d_type=nombreactual->d_type;
		item->next=NULL;

		//primer item
		if (primer_filesel_item==NULL) {
			primer_filesel_item=item;
		}

		//siguientes items
		else {
			itemanterior->next=item;
		}

		itemanterior=item;
		free(namelist[i]);


		filesel_total_items++;
        }

		free(namelist);

    }

	return 0;
	//free(namelist);

}


//Retorna 1 si ok
//Retorna 0 si no ok
int menu_avisa_si_extension_no_habitual(char *filtros[],char *archivo)
{

	int i;

	for (i=0;filtros[i];i++) {
		if (!util_compare_file_extension(archivo,filtros[i])) return 1;

		//si filtro es "", significa todo (*)
		if (!strcmp(filtros[i],"")) return 1;

	}


	//no es extension habitual. Avisar
	return menu_confirm_yesno_texto("Unusual file extension","Do you want to use this file?");
}


void menu_quickload(MENU_ITEM_PARAMETERS)
{

	menu_first_aid("smartload");

        char *filtros[31];

        filtros[0]="zx";
        filtros[1]="sp";
        filtros[2]="z80";
        filtros[3]="sna";

        filtros[4]="o";
        filtros[5]="p";
        filtros[6]="80";
        filtros[7]="81";
	filtros[8]="z81";

        filtros[9]="tzx";
        filtros[10]="tap";

	filtros[11]="rwa";
	filtros[12]="smp";
	filtros[13]="wav";

	filtros[14]="epr";
	filtros[15]="63";
	filtros[16]="eprom";
	filtros[17]="flash";

	filtros[18]="ace";

	filtros[19]="dck";

	filtros[20]="cdt";

	filtros[21]="ay";

	filtros[22]="scr";

	filtros[23]="rzx";

	filtros[24]="zsf";

	filtros[25]="spg";

	filtros[26]="trd";

	filtros[27]="nex";
	
	filtros[28]="dsk";

	filtros[29]="pzx";

	filtros[30]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de snap
        //si no hay directorio, vamos a rutas predefinidas
        if (quickfile==NULL) menu_chdir_sharedfiles();
		else {
                char directorio[PATH_MAX];
                util_get_dir(quickfile,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }

				util_get_file_no_directory(quickfile,menu_filesel_posicionar_archivo_nombre);

				menu_filesel_posicionar_archivo.v=1;

        }






        int ret;

        ret=menu_filesel("Select File",filtros,quickload_file);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

			quickfile=quickload_file;

            //sin overlay de texto, que queremos ver las franjas de carga con el color normal (no apagado)
            reset_menu_overlay_function();


			if (quickload(quickload_file)) {
				debug_printf (VERBOSE_ERR,"Unknown file format");
			}

                //restauramos modo normal de texto de menu
                set_menu_overlay_function(normal_overlay_texto_menu);

                //Y salimos de todos los menus si conviene
                if (no_close_menu_after_smartload.v==0) salir_todos_menus=1;
        }

	//printf ("tapefile: %p %s\n",tapefile,tapefile);

}


void menu_tape_out_open(MENU_ITEM_PARAMETERS)
{

        char *filtros[5];
	char mensaje_existe[20];

        if (MACHINE_IS_ZX8081) {

			if (MACHINE_IS_ZX80) filtros[0]="o";
			else filtros[0]="p";

			filtros[1]=0;

			strcpy(mensaje_existe,"Overwrite?");
		}

		else {
			filtros[0]="tzx";
			filtros[1]="tap";
			filtros[2]="pzx";
			filtros[3]=0;
			strcpy(mensaje_existe,"Append?");
		}


        if (menu_filesel("Select Output Tape",filtros,tape_out_open_file)==1) {

		//Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(tape_out_open_file, &buf_stat)==0) {

                	if (MACHINE_IS_ZX8081) {
                        	if (menu_confirm_yesno_texto("File exists",mensaje_existe)==0) {
					tape_out_file=NULL;
					tap_out_close();
					return;
				}
			}

			else {
				int opcion=menu_ask_no_append_truncate_texto("File exists","What do you want?");
				//printf ("opcion: %d\n",opcion);

				//Cancel
				if (opcion==0) {
					tape_out_file=NULL;
					tap_out_close();
					return;
				}

				//Truncate
				if (opcion==2) {
					util_truncate_file(tape_out_open_file);
				}
			}

                }

                tape_out_file=tape_out_open_file;
                tape_out_init();
        }


        else {
                tape_out_file=NULL;
			tap_out_close();
        }



}


void menu_tape_open(MENU_ITEM_PARAMETERS)
{

        char *filtros[7];

	if (MACHINE_IS_ZX80) {
		filtros[0]="80";
        	filtros[1]="o";
        	filtros[2]="rwa";
        	filtros[3]="smp";
        	filtros[4]="wav";
        	filtros[5]="z81";
        	filtros[6]=0;
	}

	else if (MACHINE_IS_ZX81) {
                filtros[0]="p";
                filtros[1]="81";
                filtros[2]="rwa";
                filtros[3]="smp";
                filtros[4]="z81";
                filtros[5]="wav";
                filtros[6]=0;
        }

	else {
        filtros[0]="tzx";
        filtros[1]="tap";
        filtros[2]="rwa";
        filtros[3]="smp";
        filtros[4]="wav";
        filtros[5]=0;
	}


	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de cinta
	//si no hay directorio, vamos a rutas predefinidas
	if (tapefile==NULL) menu_chdir_sharedfiles();

	else {
	        char directorio[PATH_MAX];
	        util_get_dir(tapefile,directorio);
	        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

		//cambiamos a ese directorio, siempre que no sea nulo
		if (directorio[0]!=0) {
			debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
			menu_filesel_chdir(directorio);
		}
	}



        int ret;

        ret=menu_filesel("Select Input Tape",filtros,tape_open_file);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


	if (ret==1) {
		tapefile=tape_open_file;
		tape_init();
	}


}


void menu_tape_simulate_real_load(MENU_ITEM_PARAMETERS)
{
	tape_loading_simulate.v ^=1;

	//Al activar carga real, tambien activamos realvideo
	if (tape_loading_simulate.v==1) rainbow_enabled.v=1;
}

void menu_tape_simulate_real_load_fast(MENU_ITEM_PARAMETERS)
{
        tape_loading_simulate_fast.v ^=1;
}


int menu_tape_simulate_real_load_cond(void)
{
	return tape_loading_simulate.v==1;
}


void menu_realtape_insert(MENU_ITEM_PARAMETERS)
{
	if (realtape_inserted.v==0) realtape_insert();
	else realtape_eject();
}

void menu_realtape_play(MENU_ITEM_PARAMETERS)
{
	if (realtape_playing.v) realtape_stop_playing();
	else realtape_start_playing();
}

void menu_realtape_volumen(MENU_ITEM_PARAMETERS)
{
	realtape_volumen++;
	if (realtape_volumen==16) realtape_volumen=0;
}

int menu_realtape_cond(void)
{
	if (realtape_name==NULL) return 0;
	else return 1;
}

int menu_realtape_inserted_cond(void)
{
	if (menu_realtape_cond()==0) return 0;
	return realtape_inserted.v;
}

void menu_realtape_open(MENU_ITEM_PARAMETERS)
{

        char *filtros[10];

        filtros[0]="smp";
        filtros[1]="rwa";
        filtros[2]="wav";
        filtros[3]="tzx";
        filtros[4]="p";
        filtros[5]="o";
        filtros[6]="tap";
        filtros[7]="cdt";
		filtros[8]="pzx";
        filtros[9]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de cinta
        //si no hay directorio, vamos a rutas predefinidas
        if (realtape_name==NULL) menu_chdir_sharedfiles();

        else {
                char directorio[PATH_MAX];
                util_get_dir(realtape_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

     		//cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }



        int ret;

        ret=menu_filesel("Select Input Tape",filtros,menu_realtape_name);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


        if (ret==1) {
                realtape_name=menu_realtape_name;
        	realtape_insert();
	}


}

void menu_realtape_wave_offset(MENU_ITEM_PARAMETERS)
{
        int valor_offset;

        char string_offset[5];


        sprintf (string_offset,"%d",realtape_wave_offset);

        menu_ventana_scanf("Offset",string_offset,5);

        valor_offset=parse_string_to_number(string_offset);

	if (valor_offset<-128 || valor_offset>127) {
		debug_printf (VERBOSE_ERR,"Invalid offset");
		return;
	}

	realtape_wave_offset=valor_offset;

}



void menu_standard_to_real_tape_fallback(MENU_ITEM_PARAMETERS)
{
	standard_to_real_tape_fallback.v ^=1;

}

void menu_realtape_accelerate_loaders(MENU_ITEM_PARAMETERS)
{
	accelerate_loaders.v ^=1;

}

void menu_realtape_loading_sound(MENU_ITEM_PARAMETERS)
{
	realtape_loading_sound.v ^=1;
}

void menu_file_p_browser_show(char *filename)
{
	
	//Leemos cabecera archivo p
        FILE *ptr_file_p_browser;
        ptr_file_p_browser=fopen(filename,"rb");

        if (!ptr_file_p_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer 128 bytes de la cabecera. Nota: archivos .P no tienen cabecera como tal
	z80_byte p_header[128];

        int leidos=fread(p_header,1,128,ptr_file_p_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_p_browser);


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: ZX-81");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	

        z80_int p_pc_reg=0x207;
        sprintf(buffer_texto,"PC Register: %04XH",p_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("P file browser", 0, 0, 1, NULL, "%s", texto_browser);
  zxvision_generic_message_tooltip("P file Browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}


void menu_file_o_browser_show(char *filename)
{
	
	//Leemos cabecera archivo p
        FILE *ptr_file_o_browser;
        ptr_file_o_browser=fopen(filename,"rb");

        if (!ptr_file_o_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer 128 bytes de la cabecera. Nota: archivos .O no tienen cabecera como tal
	z80_byte o_header[128];

        int leidos=fread(o_header,1,128,ptr_file_o_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_o_browser);


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: ZX-80");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	

        z80_int o_pc_reg=0x283;
        sprintf(buffer_texto,"PC Register: %04XH",o_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("O file browser", 0, 0, 1, NULL, "%s", texto_browser);
  zxvision_generic_message_tooltip("O file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tao_get_info(z80_byte *tape,char *texto)


}



void menu_file_sp_browser_show(char *filename)
{
	
	//Leemos cabecera archivo sp
        FILE *ptr_file_sp_browser;
        ptr_file_sp_browser=fopen(filename,"rb");

        if (!ptr_file_sp_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer 38 bytes de la cabecera
	z80_byte sp_header[38];

        int leidos=fread(sp_header,1,38,ptr_file_sp_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_sp_browser);

        //Testear cabecera "SP" en los primeros bytes
        if (sp_header[0]!='S' || sp_header[1]!='P') {
        	debug_printf(VERBOSE_ERR,"Invalid .SP file");
        	return;
        }

	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	sprintf(buffer_texto,"Machine: Spectrum 48k");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	

        z80_int sp_pc_reg=value_8_to_16(sp_header[31],sp_header[30]);
        sprintf(buffer_texto,"PC Register: %04XH",sp_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SP file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SP file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}


void menu_file_mmc_browser_show_file(z80_byte *origen,char *destino,int sipuntoextension,int longitud)
{
	int i;
	int salir=0;
	for (i=0;i<longitud && !salir;i++) {
		char caracter;
		caracter=*origen;

		
			origen++;
			if (caracter<32 || caracter>127) {
				//Si detectamos final de texto y siempre que no este en primer caracter
				if (i) salir=1;
				else caracter='?';
			}

			if (!salir) {
				*destino=caracter;
				destino++;
			
				if (sipuntoextension && i==7) {
					*destino='.';
					destino++;
				}
			}
		
	}

	*destino=0;
}

void menu_file_mmc_browser_show(char *filename,char *tipo_imagen)
{

	/*
	Ejemplo directorio en fat para imagenes mmc de 32 MB:
00110200  54 42 42 4c 55 45 20 20  46 57 20 20 00 5d 10 95  |TBBLUE  FW  .]..|
00110210  54 49 58 4b 00 00 6f 9d  58 4b 04 00 00 7e 01 00  |TIXK..o.XK...~..|
00110220  54 4d 50 20 20 20 20 20  20 20 20 10 00 5d 10 95  |TMP        ..]..|
00110230  54 49 54 49 00 00 10 95  54 49 5d 00 00 00 00 00  |TITI....TI].....|
00110240  42 49 4e 20 20 20 20 20  20 20 20 10 00 79 4f 9e  |BIN        ..yO.|
00110250  58 4b 58 4b 00 00 4f 9e  58 4b 03 00 00 00 00 00  |XKXK..O.XK......|
00110260  42 49 54 4d 41 50 53 20  20 20 20 10 00 2d 77 9e  |BITMAPS    ..-w.|
00110270  58 4b 58 4b 00 00 77 9e  58 4b 79 00 00 00 00 00  |XKXK..w.XKy.....|
00110280  52 54 43 20 20 20 20 20  20 20 20 10 00 27 88 9e  |RTC        ..'..|
00110290  58 4b 58 4b 00 00 88 9e  58 4b 7d 00 00 00 00 00  |XKXK....XK}.....|
001102a0  53 59 53 20 20 20 20 20  20 20 20 10 00 89 a5 9e  |SYS        .....|
001102b0  58 4b 58 4b 00 00 a5 9e  58 4b 5c 01 00 00 00 00  |XKXK....XK\.....|
001102c0  54 42 42 4c 55 45 20 20  20 20 20 10 00 0f c5 9e  |TBBLUE     .....|
001102d0  58 4b 58 4b 00 00 c5 9e  58 4b f5 01 00 00 00 00  |XKXK....XK......|
001102e0  53 50 52 49 54 45 53 20  20 20 20 10 00 92 95 9e  |SPRITES    .....|
001102f0  58 4b 58 4b 00 00 95 9e  58 4b 09 01 00 00 00 00  |XKXK....XK......|
00110300  e5 45 53 54 49 4e 4f 20  20 20 20 20 08 6c 53 9d  |.ESTINO     .lS.|
	*/

	int tamanyo_vfat_entry=32;
	int tamanyo_plus3_entry=32;

	int max_entradas_vfat=16;

	//Asignamos para 16 entradas
	int bytes_to_load=0x110200+tamanyo_vfat_entry*max_entradas_vfat;

	z80_byte *mmc_file_memory;
	mmc_file_memory=malloc(bytes_to_load);
	if (mmc_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos cabecera archivo mmc
        FILE *ptr_file_mmc_browser;
        ptr_file_mmc_browser=fopen(filename,"rb");

        if (!ptr_file_mmc_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(mmc_file_memory);
		return;
	}


        int leidos=fread(mmc_file_memory,1,bytes_to_load,ptr_file_mmc_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_mmc_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"RAW disk image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00100020  00 00 00 00 80 00 29 bb  4f 74 10 4e 4f 20 4e 41  |......).Ot.NO NA|
00100030  4d 45 20 20 20 20 46 41  54 31 36 20 20 20 0e 1f  |ME    FAT16   ..|
*/

/*
00000000  50 4c 55 53 49 44 45 44  4f 53 20 20 20 20 20 20  |PLUSIDEDOS      |
00000010  01 00 00 00 00 00 00 7f  00 00 00 00 00 00 00 00  |................|
00000020  02 01 02 80 00 01 04 00  38 38 00 00 00 00 00 00  |........88......|
00000030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000040  64 61 74 6f 73 20 20 20  20 20 20 20 20 20 20 20  |datos           |
00000050  03 00 00 01 80 00 00 ff  7f 00 00 00 00 00 00 00  |................|
00000060  00 02 06 3f 03 f7 07 ff  01 c0 00 00 80 00 00 02  |...?............|
00000070  03 00 ff 80 00 00 02 00  00 00 00 00 43 00 00 00  |............C...|
00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000090  ff 80 00 01 01 01 01 7f  81 00 00 00 00 00 00 00  |................|
000000a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00010000  00 53 50 45 43 54 34 38  4b 52 4f 4d 00 00 00 80  |.SPECT48KROM....|
00010010  02 00 03 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010020  00 42 49 4f 53 20 20 20  20 52 4f 4d 00 00 00 80  |.BIOS    ROM....|
00010030  04 00 05 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010040  00 49 4e 56 45 53 20 20  20 52 4f 4d 00 00 00 80  |.INVES   ROM....|
00010050  06 00 07 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010060  00 54 4b 39 30 58 20 20  20 52 4f 4d 00 00 00 80  |.TK90X   ROM....|
00010070  08 00 09 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00010080  00 50 33 33 45 4d 4d 43  30 52 4f 4d 00 00 00 80  |.P33EMMC0ROM....|
*/

	char filesystem[32];
	memcpy(filesystem,&mmc_file_memory[0x100036],5);

	filesystem[5]=0;
	if (!strcmp(filesystem,"FAT16")) {


	        sprintf(buffer_texto,"Filesystem: FAT16");
        	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		char vfat_label[8+3+1];
		menu_file_mmc_browser_show_file(&mmc_file_memory[0x10002b],vfat_label,0,11);
		sprintf(buffer_texto,"FAT Label: %s",vfat_label);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		sprintf(buffer_texto,"First FAT entries:");
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

		int puntero,i;

		puntero=0x110200;
/*


https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html


Bytes   Content
0-10    File name (8 bytes) with extension (3 bytes)
11      Attribute - a bitvector. Bit 0: read only. Bit 1: hidden.
        Bit 2: system file. Bit 3: volume label. Bit 4: subdirectory.
        Bit 5: archive. Bits 6-7: unused.
12-21   Reserved (see below)
22-23   Time (5/6/5 bits, for hour/minutes/doubleseconds)
24-25   Date (7/4/5 bits, for year-since-1980/month/day)
26-27   Starting cluster (0 for an empty file)
28-31   Filesize in bytes
*/

		for (i=0;i<max_entradas_vfat;i++) {
			z80_byte file_attribute=mmc_file_memory[puntero+11];
			menu_file_mmc_browser_show_file(&mmc_file_memory[puntero],buffer_texto,1,11);
			if (buffer_texto[0]!='?') {
				//Es directorio?
				if (file_attribute&16) {
					//Agregamos texto <DIR>
					int l=strlen(buffer_texto);
					strcpy(&buffer_texto[l]," <DIR>");
				}

				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
			}

			puntero +=tamanyo_vfat_entry;	
		}
	}

	memcpy(filesystem,&mmc_file_memory[0],10);

	filesystem[10]=0;
	if (!strcmp(filesystem,"PLUSIDEDOS")) {

		sprintf(buffer_texto,"Filesystem: PLUSIDEDOS");
        	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		char plus3_label[16+1];
		menu_file_mmc_browser_show_file(&mmc_file_memory[0x40],plus3_label,0,11);
		sprintf(buffer_texto,"Label: %s",plus3_label);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


		sprintf(buffer_texto,"First PLUS3 entries:");
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

		int puntero,i;

		puntero=0x10000+1;

		for (i=0;i<max_entradas_vfat;i++) {
			menu_file_mmc_browser_show_file(&mmc_file_memory[puntero],buffer_texto,1,11);
			if (buffer_texto[0]!='?') {
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
			}

			puntero +=tamanyo_plus3_entry;	
		}
	}
	

	texto_browser[indice_buffer]=0;
	char titulo_ventana[32];
	sprintf(titulo_ventana,"%s file browser",tipo_imagen);
	//menu_generic_message_tooltip(titulo_ventana, 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip(titulo_ventana , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)

	free(mmc_file_memory);

}


void menu_file_trd_browser_show(char *filename,char *tipo_imagen)
{


	int tamanyo_trd_entry=16;

	int max_entradas_trd=16;

	//Asignamos para 16 entradas
	//int bytes_to_load=tamanyo_trd_entry*max_entradas_trd;

	//Leemos 4kb. esto permite leer el directorio y el label
	int bytes_to_load=4096;

	z80_byte *trd_file_memory;
	trd_file_memory=malloc(bytes_to_load);
	if (trd_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos cabecera archivo trd
        FILE *ptr_file_trd_browser;
        ptr_file_trd_browser=fopen(filename,"rb");

        if (!ptr_file_trd_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(trd_file_memory);
		return;
	}


        int leidos=fread(trd_file_memory,1,bytes_to_load,ptr_file_trd_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_trd_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"TRD disk image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00000000  43 43 31 30 41 59 20 20  42 4a 00 4a 00 31 00 01  |CC10AY  BJ.J.1..|
00000010  74 69 74 6c 65 20 20 20  43 20 20 00 1b 1b 01 04  |title   C  .....|
00000020  73 6b 69 6e 20 20 20 20  43 20 20 00 1b 1b 0c 05  |skin    C  .....|
00000030  20 4e 69 67 68 74 49 6e  6d 20 20 b9 0a 0b 07 07  | NightInm  .....|
00000040  20 45 76 61 31 20 20 20  70 74 33 26 0c 0d 02 08  | Eva1   pt3&....|
00000050  20 45 76 61 32 20 20 20  70 74 33 c4 1e 1f 0f 08  | Eva2   pt3.....|
00000060  20 52 6f 73 65 31 20 20  70 74 33 ef 03 04 0e 0a  | Rose1  pt3.....|
00000070  20 52 6f 73 65 32 20 20  70 74 33 a2 04 05 02 0b  | Rose2  pt3.....|
00000080  20 53 74 72 6f 6c 6c 6e  70 74 33 a6 19 1a 07 0b  | Strollnpt3.....|
00000090  20 53 77 6f 6f 6e 43 56  6d 20 20 7f 0f 10 01 0d  | SwoonCVm  .....|
000000a0  20 53 6f 70 68 69 65 45  6d 20 20 76 11 12 01 0e  | SophieEm  v....|
000000b0  20 73 75 6d 6d 65 72 31  70 74 33 e0 06 07 03 0f  | summer1pt3.....|
000000c0  20 73 75 6d 6d 65 72 32  70 74 33 8d 05 06 0a 0f  | summer2pt3.....|
000000d0  20 62 74 74 66 31 20 20  70 74 33 60 0b 0c 00 10  | bttf1  pt3`....|
000000e0  20 62 74 74 66 32 20 20  70 74 33 41 04 05 0c 10  | bttf2  pt3A....|
000000f0  20 54 6f 52 69 73 6b 54  6d 20 20 e3 28 29 01 11  | ToRiskTm  .()..|
*/

	//La extension es de 1 byte


	sprintf(buffer_texto,"Filesystem: TRDOS");
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int start_track_8=256*8;


        char trd_label[8+1];
        menu_file_mmc_browser_show_file(&trd_file_memory[0x8f5],trd_label,0,8);
        sprintf(buffer_texto,"TRD Label: %s",trd_label);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	sprintf(buffer_texto,"Free sectors on disk: %d",trd_file_memory[start_track_8+229]+256*trd_file_memory[start_track_8+230]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"First free sector %dt:%ds",trd_file_memory[start_track_8+226],trd_file_memory[start_track_8+225]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	char *trd_disk_types[]={
	"80 tracks, double side",
	"40 tracks, double side",
	"80 tracks, single side",
	"40 tracks, single side"};

	char buffer_trd_disk_type[32];
	z80_byte trd_disk_type=trd_file_memory[start_track_8+227];

	if (trd_disk_type>=0x16 && trd_disk_type<=0x19) {
		strcpy(buffer_trd_disk_type,trd_disk_types[trd_disk_type-0x16]);
	}
	else strcpy(buffer_trd_disk_type,"Unknown");

	sprintf(buffer_texto,"Disk type: %04XH (%s)",trd_disk_type,buffer_trd_disk_type);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Files on disk: %d",trd_file_memory[start_track_8+228]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Deleted files on disk: %d",trd_file_memory[start_track_8+244]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	sprintf(buffer_texto,"First file entries:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int puntero,i;

	puntero=0;

	for (i=0;i<max_entradas_trd;i++) {
		menu_file_mmc_browser_show_file(&trd_file_memory[puntero],buffer_texto,1,9);
		if (buffer_texto[0]!='?') {
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
		}
		z80_byte start_sector=trd_file_memory[puntero+14];
		z80_byte start_track=trd_file_memory[puntero+15];
		debug_printf (VERBOSE_DEBUG,"File %s starts at track %d sector %d",buffer_texto,start_track,start_sector);

		puntero +=tamanyo_trd_entry;	
	}
	


	texto_browser[indice_buffer]=0;
	char titulo_ventana[32];
	sprintf(titulo_ventana,"%s file browser",tipo_imagen);
	//menu_generic_message_tooltip(titulo_ventana, 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip(titulo_ventana , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)

	free(trd_file_memory);

}


//Retorna el offset al dsk segun la pista y sector dados (ambos desde 0...)
//-1 si no se encuentra
int menu_dsk_getoff_track_sector(z80_byte *dsk_memoria,int total_pistas,int pista_buscar,int sector_buscar)
{

/*
sectores van alternados:
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|

1,6,2,7,3,8


0 1 2 3 4 5 6 7 8  
0,5,1,6,2,7,3,8,4

*/

	int pista;
	int sector;

	int iniciopista_orig=256;

	//Buscamos en todo el archivo dsk
	for (pista=0;pista<total_pistas;pista++) {

		int sectores_en_pista=dsk_memoria[iniciopista_orig+0x15];
		//debug_printf(VERBOSE_DEBUG,"Iniciopista: %XH (%d). Sectores en pista %d: %d. IDS pista:  ",iniciopista_orig,iniciopista_orig,pista,sectores_en_pista);

		//int iniciopista_orig=traps_plus3dos_getoff_start_trackinfo(pista);
		int iniciopista=iniciopista_orig;
		//saltar 0x18
		iniciopista +=0x18;

		for (sector=0;sector<sectores_en_pista;sector++) {
			int offset_tabla_sector=sector*8; 
			z80_byte pista_id=dsk_memoria[iniciopista+offset_tabla_sector]; //Leemos pista id
			z80_byte sector_id=dsk_memoria[iniciopista+offset_tabla_sector+2]; //Leemos c1, c2, etc

			//debug_printf(VERBOSE_DEBUG,"%02X ",sector_id);

			sector_id &=0xF;

			sector_id--;  //empiezan en 1...

			if (pista_id==pista_buscar && sector_id==sector_buscar) {
				//printf("Found sector %d/%d at %d/%d",pista_buscar,sector_buscar,pista,sector);
		                //int offset=traps_plus3dos_getoff_start_track(pista);
		                int offset=iniciopista_orig+256;

                		//int iniciopista=traps_plus3dos_getoff_start_track(pista);
		                return offset+512*sector;
			}

		}

		//debug_printf(VERBOSE_DEBUG,"");

		iniciopista_orig +=256;
		iniciopista_orig +=512*sectores_en_pista;
	}

	debug_printf(VERBOSE_DEBUG,"Not found sector %d/%d",pista_buscar,sector_buscar);	
	
	//retornamos offset fuera de rango
	return -1;


}


void menu_dsk_getoff_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *offset1,int *offset2)
{

			int total_pistas=longitud_dsk/4864;
			int pista;
			int sector_en_pista;

			int sector_total;

			sector_total=bloque*2; //cada bloque es de 2 sectores

			//tenemos sector total en variable bloque
			//sacar pista
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;

			//printf ("pista: %d sector en pista: %d\n",pista,sector_en_pista);

			//offset a los datos dentro del dsk
			//int offset=pista*4864+sector_en_pista*512;



			*offset1=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista);

			sector_total++;
			pista=sector_total/9; //9 sectores por pista
			sector_en_pista=sector_total % 9;			

			*offset2=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,pista,sector_en_pista);

}


int menu_dsk_get_start_filesystem(z80_byte *dsk_file_memory,int longitud_dsk)
{

                        int total_pistas=longitud_dsk/4864;

                        //tenemos sector total en variable bloque
                        //sacar pista

                        //printf ("pista: %d sector en pista: %d\n",pista,sector_en_pista);

                        //offset a los datos dentro del dsk
                        //int offset=pista*4864+sector_en_pista*512;



                        return menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,0,0);

}


void menu_file_dsk_browser_show(char *filename)
{


	int tamanyo_dsk_entry=32;

	int max_entradas_dsk=16;

	//Asignamos para 16 entradas
	//int bytes_to_load=tamanyo_dsk_entry*max_entradas_dsk;

	//Leemos 300kb. aunque esto excede con creces el tamanyo para leer el directorio, podria pasar que la pista 0 donde esta
	//el directorio estuviera ordenada al final del archivo
	int bytes_to_load=300000;  //temp. 4096

	z80_byte *dsk_file_memory;
	dsk_file_memory=malloc(bytes_to_load);
	if (dsk_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos archivo dsk
        FILE *ptr_file_dsk_browser;
        ptr_file_dsk_browser=fopen(filename,"rb");

        if (!ptr_file_dsk_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(dsk_file_memory);
		return;
	}


        int leidos=fread(dsk_file_memory,1,bytes_to_load,ptr_file_dsk_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_dsk_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"DSK disk image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00000000  45 58 54 45 4e 44 45 44  20 43 50 43 20 44 53 4b  |EXTENDED CPC DSK|
00000010  20 46 69 6c 65 0d 0a 44  69 73 6b 2d 49 6e 66 6f  | File..Disk-Info|
00000020  0d 0a 43 50 44 52 65 61  64 20 76 33 2e 32 34 00  |..CPDRead v3.24.|
00000030  2d 01 00 00 13 13 13 13  13 13 13 13 13 13 13 13  |-...............|
00000040  13 13 13 13 13 13 13 13  13 13 13 13 13 13 13 13  |................|
00000050  13 13 13 13 13 13 13 13  13 13 13 13 00 00 00 00  |................|
00000060  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000100  54 72 61 63 6b 2d 49 6e  66 6f 0d 0a 00 00 00 00  |Track-Info......|
00000110  00 00 00 00 02 09 4e e5  00 00 c1 02 00 00 00 02  |......N.........|
00000120  00 00 c6 02 00 00 00 02  00 00 c2 02 00 00 00 02  |................|
00000130  00 00 c7 02 00 00 00 02  00 00 c3 02 00 00 00 02  |................|
00000140  00 00 c8 02 00 00 00 02  00 00 c4 02 00 00 00 02  |................|
00000150  00 00 c9 02 00 00 00 02  00 00 c5 02 00 00 00 02  |................|
00000160  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000200  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 00 00 00 80  |.COMPILER.IN....|
00000210  02 03 04 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11  |................|
00000220  00 43 4f 4d 50 49 4c 45  52 c2 49 4e 01 00 00 59  |.COMPILER.IN...Y|
00000230  12 13 14 15 16 17 18 19  1a 1b 1c 1d 00 00 00 00  |................|
00000240  00 4b 49 54 31 32 38 4c  44 c2 49 4e 00 00 00 03  |.KIT128LD.IN....|
00000250  1e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000260  00 4b 49 54 31 32 38 20  20 c2 49 4e 00 00 00 80  |.KIT128  .IN....|
00000270  1f 20 21 22 23 24 25 26  27 28 29 2a 2b 2c 2d 2e  |. !"#$%&'()*+,-.|
00000280  00 4b 49 54 31 32 38 20  20 c2 49 4e 01 00 00 59  |.KIT128  .IN...Y|
*/

	//La extension es de 1 byte


	/*
        sprintf(buffer_texto,"DSK Label: %s",dsk_label);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	sprintf(buffer_texto,"Free sectors on disk: %d",dsk_file_memory[start_track_8+229]+256*dsk_file_memory[start_track_8+230]);
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);*/

 	sprintf(buffer_texto,"Disk information:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	util_binary_to_ascii(&dsk_file_memory[0], buffer_texto, 34, 34);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


 	sprintf(buffer_texto,"\nCreator:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	util_binary_to_ascii(&dsk_file_memory[0x22], buffer_texto, 14, 14);

	sprintf(buffer_texto,"\nTracks: %d",dsk_file_memory[0x30]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Sides: %d",dsk_file_memory[0x31]);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);	


        sprintf(buffer_texto,"\nFirst PLUS3 entries:");
        indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);



	int puntero,i;
	//puntero=0x201;

	puntero=menu_dsk_get_start_filesystem(dsk_file_memory,bytes_to_load);
/*
en teoria , el directorio empieza en pista 0 sector 0, aunque esta info dice otra cosa:

                TRACK 0          TRACK 1             TRACK 2

SPECTRUM +3    Reserved          Directory             -
                               (sectors 0-3)


Me encuentro con algunos discos en que empiezan en pista 1 y otros en pista 0 ??

*/

	if (puntero==-1) {
		//printf ("Filesystem track/sector not found. Guessing it\n");
		//no encontrado. probar con lo habitual
		puntero=0x200;
	}

	//else {
		//Si contiene e5 en el nombre, nos vamos a pista 1
		if (dsk_file_memory[puntero+1]==0xe5) {
			//printf ("Filesystem doesnt seem to be at track 0. Trying with track 1\n");
            int total_pistas=bytes_to_load/4864;

            puntero=menu_dsk_getoff_track_sector(dsk_file_memory,total_pistas,1,0);

			if (puntero==-1) {
		                //printf ("Filesystem track/sector not found. Guessing it\n");
		                //no encontrado. probar con lo habitual
	                	puntero=0x200;
			}
			//else 	printf ("Filesystem found at offset %XH\n",puntero);
		}
		//else printf ("Filesystem found at offset %XH\n",puntero);
	//}
	
	puntero++; //Saltar el primer byte en la entrada de filesystem



	for (i=0;i<max_entradas_dsk;i++) {

		menu_file_mmc_browser_show_file(&dsk_file_memory[puntero],buffer_texto,1,11);
		if (buffer_texto[0]!='?') {
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
		}

		puntero +=tamanyo_dsk_entry;	


	}
	


	texto_browser[indice_buffer]=0;


	//  menu_generic_message_tooltip("DSK file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("DSK file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


	free(dsk_file_memory);

}



void menu_file_zsf_browser_show(char *filename)
{


	long int bytes_to_load=get_file_size(filename);

	z80_byte *zsf_file_memory;
	zsf_file_memory=malloc(bytes_to_load);
	if (zsf_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos archivo zsf
        FILE *ptr_file_zsf_browser;
        ptr_file_zsf_browser=fopen(filename,"rb");

        if (!ptr_file_zsf_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(zsf_file_memory);
		return;
	}


        int leidos=fread(zsf_file_memory,1,bytes_to_load,ptr_file_zsf_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_zsf_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"ZSF ZEsarUX Snapshot");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


int indice_zsf=0;


//Verificar que la cabecera inicial coincide
  //zsf_magic_header

  char buffer_magic_header[256];

  int longitud_magic=strlen(zsf_magic_header);


  if (leidos<longitud_magic) {
    debug_printf(VERBOSE_ERR,"Invalid ZSF file, small magic header");
    return;
  }

  //Comparar texto
  memcpy(buffer_magic_header,zsf_file_memory,longitud_magic);
  buffer_magic_header[longitud_magic]=0;

  if (strcmp(buffer_magic_header,zsf_magic_header)) {
    debug_printf(VERBOSE_ERR,"Invalid ZSF file, invalid magic header");
    return;
  }

  indice_zsf+=longitud_magic;
  bytes_to_load -=longitud_magic;


/*
Format ZSF:
* All numbers are LSB

Every block is defined with a header:

2 bytes - 16 bit: block ID
4 bytes - 32 bit: block Lenght
After these 6 bytes, the data for the block comes.
*/
	

	while (bytes_to_load>0) {
		    z80_int block_id;
    			block_id=value_8_to_16(zsf_file_memory[indice_zsf+1],zsf_file_memory[indice_zsf+0]);
    			unsigned int block_lenght=zsf_file_memory[indice_zsf+2]+(zsf_file_memory[indice_zsf+3]*256)+(zsf_file_memory[indice_zsf+4]*65536)+(zsf_file_memory[indice_zsf+5]*16777216);

    			debug_printf (VERBOSE_INFO,"Block id: %u Size: %u",block_id,block_lenght);

    			sprintf(buffer_texto,"Id: %u (%s) Size: %u",block_id,zsf_get_block_id_name(block_id),block_lenght);
    			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

    			bytes_to_load -=6;
    			bytes_to_load -=block_lenght;


    			indice_zsf +=6;
    			indice_zsf +=block_lenght;

	}

	
	


	texto_browser[indice_buffer]=0;

	//  menu_generic_message_tooltip("ZSF file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZSF file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)
	free(zsf_file_memory);

}

void menu_file_zxuno_flash_browser_show(char *filename)
{


	//Asignar 4 mb

	
	int bytes_to_load=4*1024*1024;

	z80_byte *zxuno_flash_file_memory;
	zxuno_flash_file_memory=malloc(bytes_to_load);
	if (zxuno_flash_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos cabecera archivo zxuno_flash
        FILE *ptr_file_zxuno_flash_browser;
        ptr_file_zxuno_flash_browser=fopen(filename,"rb");

        if (!ptr_file_zxuno_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(zxuno_flash_file_memory);
		return;
	}


        int leidos=fread(zxuno_flash_file_memory,1,bytes_to_load,ptr_file_zxuno_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_zxuno_flash_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"ZX-Uno Flash image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*
00006000  00 01 3c 3c 00 00 00 00  fd 5e 00 00 00 00 00 00  |..<<.....^......|
00006010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006020  5a 58 20 53 70 65 63 74  72 75 6d 20 34 38 4b 20  |ZX Spectrum 48K |
00006030  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |
00006040  01 04 3d 00 00 00 00 00  be eb f9 91 e7 97 39 98  |..=...........9.|
00006050  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00006060  5a 58 20 53 70 65 63 74  72 75 6d 20 2b 32 41 20  |ZX Spectrum +2A |
00006070  45 4e 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |EN              |
00006080  05 02 3d 28 00 00 00 00  dc ec ef fc 00 00 00 00  |..=(............|
00006090  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000060a0  5a 58 20 53 70 65 63 74  72 75 6d 20 31 32 38 4b  |ZX Spectrum 128K|
000060b0  20 45 4e 20 20 20 20 20  20 20 20 20 20 20 20 20  | EN             |
000060c0  07 01 3c 3c 00 00 00 00  8e f2 00 00 00 00 00 00  |..<<............|
000060d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000060e0  49 6e 76 65 73 20 53 70  65 63 74 72 75 6d 2b 20  |Inves Spectrum+ |
000060f0  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |

*/


 	sprintf(buffer_texto,"\nROMS:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_roms_show=32;
	int i;
	for (i=0;i<total_roms_show;i++) {
		util_binary_to_ascii(&zxuno_flash_file_memory[0x6020+i*64],buffer_texto,30,30);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}


/*
Bitstreams
00007100  53 61 6d 20 43 6f 75 70  65 20 20 20 20 20 20 20  |Sam Coupe       |
00007110  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007120  4a 75 70 69 74 65 72 20  41 43 45 20 20 20 20 20  |Jupiter ACE     |
00007130  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007140  4d 61 73 74 65 72 20 53  79 73 74 65 6d 20 20 20  |Master System   |
00007150  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007160  42 42 43 20 4d 69 63 72  6f 20 20 20 20 20 20 20  |BBC Micro       |
00007170  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
00007180  50 61 63 20 4d 61 6e 20  20 20 20 20 20 20 20 20  |Pac Man         |
00007190  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071a0  4f 72 69 63 20 41 74 6d  6f 73 20 20 20 20 20 20  |Oric Atmos      |
000071b0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071c0  41 70 70 6c 65 20 32 20  28 56 47 41 29 20 20 20  |Apple 2 (VGA)   |
000071d0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |
000071e0  4e 45 53 20 28 56 47 41  29 20 20 20 20 20 20 20  |NES (VGA)       |
000071f0  20 20 20 20 20 20 00 20  20 20 20 20 20 20 20 20  |      .         |

*/
        

	sprintf(buffer_texto,"\nBitstreams:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_bitstream_show=8;

	for (i=0;i<total_bitstream_show;i++) {
		util_binary_to_ascii(&zxuno_flash_file_memory[0x7100+i*32],buffer_texto,30,30);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}


	//  menu_generic_message_tooltip("ZX-Uno Flash browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZX-Uno Flash browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)

	free(zxuno_flash_file_memory);

}


void menu_file_superupgrade_flash_browser_show(char *filename)
{


	//Asignar 512 kb

	
	int bytes_to_load=4*1024*1024;

	z80_byte *superupgrade_flash_file_memory;
	superupgrade_flash_file_memory=malloc(bytes_to_load);
	if (superupgrade_flash_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos cabecera archivo superupgrade_flash
        FILE *ptr_file_superupgrade_flash_browser;
        ptr_file_superupgrade_flash_browser=fopen(filename,"rb");

        if (!ptr_file_superupgrade_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(superupgrade_flash_file_memory);
		return;
	}


        int leidos=fread(superupgrade_flash_file_memory,1,bytes_to_load,ptr_file_superupgrade_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_superupgrade_flash_browser);



	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	


 	sprintf(buffer_texto,"Superupgrade Flash image");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


/*

00000300  0f 30 2d 53 50 45 43 54  52 55 4d 20 20 20 20 20  |.0-SPECTRUM     |
00000310  0f 31 2d 49 4e 56 45 53  20 20 20 20 20 20 20 20  |.1-INVES        |
00000320  0f 32 2d 54 4b 2d 39 30  58 20 20 20 20 20 20 20  |.2-TK-90X       |
00000330  0f 33 2d 53 50 45 43 2b  33 45 20 4d 4d 43 20 20  |.3-SPEC+3E MMC  |
00000340  0f 34 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.4----------01  |
00000350  0f 35 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 32 20 20  |.5----------02  |
00000360  0f 36 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 33 20 20  |.6----------03  |
00000370  0f 37 2d 53 50 45 43 54  52 55 4d 2b 32 20 20 20  |.7-SPECTRUM+2   |
00000380  0f 38 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.8----------01  |
00000390  0f 39 2d 53 50 45 43 54  2e 20 20 31 32 38 20 20  |.9-SPECT.  128  |
000003a0  0f 41 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.A----------01  |
000003b0  0f 42 2d 53 50 45 43 54  52 55 4d 2b 33 45 20 20  |.B-SPECTRUM+3E  |
000003c0  0f 43 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.C----------01  |
000003d0  0f 44 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 32 20 20  |.D----------02  |
000003e0  0f 45 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 33 20 20  |.E----------03  |
000003f0  0f 46 2d 4a 55 50 49 54  45 52 20 41 43 45 20 20  |.F-JUPITER ACE  |
00000400  0f 47 2d 54 4b 2d 39 35  20 20 20 20 20 20 20 20  |.G-TK-95        |
00000410  0f 48 2d 54 45 53 54 20  4d 43 4c 45 4f 44 20 20  |.H-TEST MCLEOD  |
00000420  0f 49 2d 2d 2d 2d 2d 2d  2d 2d 2d 2d 30 31 20 20  |.I----------01  |
00000430  0f 4a 2d 5a 58 20 54 45  53 54 20 52 4f 4d 20 20  |.J-ZX TEST ROM  |
00000440  0f 4b 2d 53 45 20 42 41  53 49 43 20 20 20 20 20  |.K-SE BASIC     |
00000450  0f 4c 2d 4d 41 4e 49 43  20 4d 49 4e 45 52 20 20  |.L-MANIC MINER  |
00000460  0f 4d 2d 4a 45 54 20 53  45 54 20 57 49 4c 4c 59  |.M-JET SET WILLY|
00000470  0f 4e 2d 53 54 41 52 20  57 41 52 53 20 20 20 20  |.N-STAR WARS    |
00000480  0f 4f 2d 44 45 41 54 48  20 43 48 41 53 45 20 20  |.O-DEATH CHASE  |
00000490  0f 50 2d 4d 49 53 43 4f  20 4a 4f 4e 45 53 20 20  |.P-MISCO JONES  |
000004a0  0f 51 2d 4c 41 4c 41 20  50 52 4f 4c 4f 47 55 45  |.Q-LALA PROLOGUE|
000004b0  0f 52 2d 53 50 41 43 45  20 52 41 49 44 45 52 53  |.R-SPACE RAIDERS|
000004c0  0f 53 2d 43 48 45 53 53  20 20 20 20 20 20 20 20  |.S-CHESS        |
000004d0  0f 54 2d 51 42 45 52 54  20 20 20 20 20 20 20 20  |.T-QBERT        |
000004e0  0f 55 2d 50 4f 50 45 59  45 20 20 20 20 20 20 20  |.U-POPEYE       |
000004f0  0f 56 2d 48 45 52 52 41  4d 49 45 4e 54 41 53 20  |.V-HERRAMIENTAS |
00000500  e0 61 62 63 04 05 06 07  48 49 4a 4b 0c 0d 0e 0f  |.abc....HIJK....|



*/


 	sprintf(buffer_texto,"\nROMS:");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	int total_roms_show=32;
	int i;
	for (i=0;i<total_roms_show;i++) {
		util_binary_to_ascii(&superupgrade_flash_file_memory[0x301+i*16],buffer_texto,15,15);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}



	//menu_generic_message_tooltip("Superupgrade Flash browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Superupgrade Flash browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);


	free(superupgrade_flash_file_memory);

}




void menu_file_flash_browser_show(char *filename)
{


	
	//Leemos cabecera archivo flash
	z80_byte flash_cabecera[256];

        FILE *ptr_file_flash_browser;
        ptr_file_flash_browser=fopen(filename,"rb");

        if (!ptr_file_flash_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}


        int leidos=fread(flash_cabecera,1,256,ptr_file_flash_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_flash_browser);


        if (get_file_size(filename)==4*1024*1024
        	&& flash_cabecera[0]==0xff
        	&& flash_cabecera[1]==0xff
        	&& flash_cabecera[2]==0xff
        	&& flash_cabecera[3]==0xff
        	) {
        	menu_file_zxuno_flash_browser_show(filename);
       	}


       	else if (
       		   flash_cabecera[0]==1
        	&& flash_cabecera[1]==0
        	&& flash_cabecera[2]==0
        	&& flash_cabecera[3]==0
        	&& flash_cabecera[4]==0
        	&& flash_cabecera[5]==0
       	 	) { 
       		//adivinar que es z88 flash
       		//01 00 00 00 00 00

       		menu_z88_new_ptr_card_browser(filename);
       	}

       	else if (
       		   flash_cabecera[0]==0x01
        	&& flash_cabecera[1]==0xfd
        	&& flash_cabecera[2]==0x7f
		) {
       		//adivinar superupgrade
       		//00000000  01 fd 7f 3e c5 ed 79 01
       		menu_file_superupgrade_flash_browser_show(filename);
       	}


       	else {
       		//binario
       		 menu_file_viewer_read_text_file("Flash file",filename);
       	}

}

void menu_file_hexdump_browser_show(char *filename)
{


	//Cargamos maximo 4 kb
#define MAX_HEXDUMP_FILE 4096
	int bytes_to_load=MAX_HEXDUMP_FILE;

	z80_byte *hexdump_file_memory;
	hexdump_file_memory=malloc(bytes_to_load);
	if (hexdump_file_memory==NULL) {
		debug_printf(VERBOSE_ERR,"Unable to assign memory");
		return;
	}
	
	//Leemos cabecera archivo hexdump
        FILE *ptr_file_hexdump_browser;
        ptr_file_hexdump_browser=fopen(filename,"rb");

        if (!ptr_file_hexdump_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		free(hexdump_file_memory);
		return;
	}


        int leidos=fread(hexdump_file_memory,1,bytes_to_load,ptr_file_hexdump_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_hexdump_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;
#define MAX_TEXTO_BROWSER_HEX (MAX_HEXDUMP_FILE*5)
//Por cada linea de texto de 33 bytes, se muestran 8 bytes del fichero. Por tanto, podemos aproximar por lo alto,
//que el texto ocupa 5 veces los bytes. Ejemplo 8*5=40 

	char texto_browser[MAX_TEXTO_BROWSER_HEX];
	int indice_buffer=0;

	
 	sprintf(buffer_texto,"Hexadecimal view");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	long int tamanyo=get_file_size(filename);
 	sprintf(buffer_texto,"File size: %ld bytes",tamanyo);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	if (tamanyo>bytes_to_load) {
		leidos=bytes_to_load;
		sprintf(buffer_texto,"Showing first %d bytes",leidos);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}

	int i;
	//char buffer_indice[5];
	char buffer_hex[8*2+1];
	char buffer_ascii[8+1];

	for (i=0;i<leidos;i+=8) {
		util_binary_to_hex(&hexdump_file_memory[i],buffer_hex,8,leidos-i);
		util_binary_to_ascii(&hexdump_file_memory[i],buffer_ascii,8,leidos-i);
		sprintf(buffer_texto,"%04X %s %s",i,buffer_hex,buffer_ascii);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
	}



	texto_browser[indice_buffer]=0;

	//menu_generic_message_tooltip("Hex viewer", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Hex viewer" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(hexdump_file_memory);

}




void menu_file_sna_browser_show(char *filename)
{
	
	//Leemos cabecera archivo sna
        FILE *ptr_file_sna_browser;
        ptr_file_sna_browser=fopen(filename,"rb");

        if (!ptr_file_sna_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer 27 bytes de la cabecera
	z80_byte sna_header[27];

        int leidos=fread(sna_header,1,27,ptr_file_sna_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_sna_browser);


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	

        //z80_int sna_pc_reg=value_8_to_16(sna_header[31],sna_header[30]);
        //sprintf(buffer_texto,"PC Register: %04XH",sna_pc_reg);
 	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	//si 49179, snapshot de 48k
 	long int tamanyo=get_file_size(filename);
 	if (tamanyo==49179) {
 		sprintf(buffer_texto,"Machine: Spectrum 48k");
 	}
 	else if (tamanyo==131103 || tamanyo==147487) {
 		sprintf(buffer_texto,"Machine: Spectrum 128k");
 	}

 	else {
 		debug_printf(VERBOSE_ERR,"Invalid .SNA file");
 		return;
 	}


	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);



	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SNA file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SNA file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}

z80_byte *last_bas_browser_memory;
int last_bas_browser_memory_size=1;

z80_byte menu_file_bas_browser_show_peek(z80_int dir)
{
	return last_bas_browser_memory[dir % last_bas_browser_memory_size]; //con % para no salirnos de la memoria
}


void menu_file_basic_browser_show(char *filename)
{
	
	//Leemos archivo .bas
        FILE *ptr_file_bas_browser;
        ptr_file_bas_browser=fopen(filename,"rb");

        if (!ptr_file_bas_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	int tamanyo=get_file_size(filename);
	z80_byte *memoria;
	memoria=malloc(tamanyo);
	if (memoria==NULL) cpu_panic ("Can not allocate memory for bas read");

	last_bas_browser_memory_size=tamanyo;

	last_bas_browser_memory=memoria;

	//Leer archivo


    int leidos=fread(memoria,1,tamanyo,ptr_file_bas_browser);

	if (leidos!=tamanyo) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
    }

    fclose(ptr_file_bas_browser);

	char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

	char titulo_ventana[33];
	strcpy(titulo_ventana,"View Basic");

  	char **dir_tokens;
  	int inicio_tokens;	

	int dir_inicio_linea=0;

	int tipo=0; //Asumimos spectrum

	//Si es basic zx81
	if (!util_compare_file_extension(filename,"baszx81")) {

                //ZX81
                //dir_inicio_linea=116; //16509-0x4009
				dir_inicio_linea=0;

                //D_FILE
                //final_basic=peek_byte_no_time(0x400C)+256*peek_byte_no_time(0x400D);

                dir_tokens=zx81_rom_tokens;

                inicio_tokens=192;

				tipo=2;

				strcpy(titulo_ventana,"View ZX81 Basic");

	}

	else if (!util_compare_file_extension(filename,"baszx80")) {

                //ZX80
            //ZX80
                  //dir_inicio_linea=40; //16424-16384
				  dir_inicio_linea=0;


                  //VARS
                  //final_basic=peek_byte_no_time(0x4008)+256*peek_byte_no_time(0x4009);

                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;

                tipo=1;

		strcpy(titulo_ventana,"View ZX80 Basic");				

	}	

	//Si extension z88 o si firma de final de archivo parece ser .z88
	else if (!util_compare_file_extension(filename,"basz88") || file_is_z88_basic(filename)) {
		dir_inicio_linea=0;
		debug_view_z88_basic_from_memory(results_buffer,dir_inicio_linea,tamanyo,menu_file_bas_browser_show_peek);

  		menu_generic_message_format("View Z88 Basic","%s",results_buffer);
		free(memoria);
	//void debug_view_z88_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,
	//z80_byte (*lee_byte_function)(z80_int dir) )	
		//Este finaliza aqui
		return;	
	}

	else {
		//O es texto tal cual o es tokens de spectrum
		//.bas , .b

		//Deducimos si es un simple .bas de texto normal, o es de basic spectrum
		//Comprobacion facil, primeros dos bytes contiene numero de linea. Asumimos que si los dos son caracteres ascii imprimibles, son de texto
		//Y siempre que extension no sea .B (en este caso es spectrum con tokens)
		if (leidos>2 && util_compare_file_extension(filename,"b") ) {
			z80_byte caracter1=memoria[0];
			z80_byte caracter2=memoria[1]; 
			if (caracter1>=32 && caracter1<=127 && caracter2>=32 && caracter2<=127) {
				//Es ascii. abrir visor ascii
				debug_printf(VERBOSE_INFO,".bas file type is guessed as simple text");
				free(memoria);
				menu_file_viewer_read_text_file("Basic file",filename);
				return;
			}
			else {
				debug_printf(VERBOSE_INFO,".bas file type is guessed as Spectrum/ZX80/ZX81");
				strcpy(titulo_ventana,"View ZX Spectrum Basic");
			}
		}




		//basic spectrum
  			dir_tokens=spectrum_rom_tokens;

  			inicio_tokens=163;
	}


	debug_view_basic_from_memory(results_buffer,dir_inicio_linea,tamanyo,dir_tokens,inicio_tokens,menu_file_bas_browser_show_peek,tipo);

  	menu_generic_message_format(titulo_ventana,"%s",results_buffer);
	free(memoria);

}


void menu_file_spg_browser_show(char *filename)
{
	
	//Leemos cabecera archivo spg
        FILE *ptr_file_spg_browser;
        ptr_file_spg_browser=fopen(filename,"rb");

        if (!ptr_file_spg_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer  bytes de la cabecera
	int tamanyo_cabecera=sizeof(struct hdrSPG1_0);
	struct hdrSPG1_0 spg_header;

        int leidos=fread(&spg_header,1,tamanyo_cabecera,ptr_file_spg_browser);

	if (leidos!=tamanyo_cabecera) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }

        fclose(ptr_file_spg_browser);






        


        


	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;


	if (memcmp(&spg_header.sign, "SpectrumProg", 12)) {
    		debug_printf(VERBOSE_ERR,"Unknown snapshot signature");
    		return;
    	}




	

        //z80_int spg_pc_reg=value_8_to_16(spg_header[31],spg_header[30]);
        //sprintf(buffer_texto,"PC Register: %04XH",spg_pc_reg);
 	//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	sprintf(buffer_texto,"Machine: ZX-Evolution TS-Conf");
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


    	z80_byte snap_type = spg_header.ver;

        if ((snap_type != 0) && (snap_type != 1) && (snap_type != 2) && (snap_type != 0x10)) {
      		debug_printf(VERBOSE_ERR,"Unknown snapshot type: %02XH",snap_type);
      		return;
      	}


	sprintf(buffer_texto,"Snapshot type: %02XH",snap_type);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Author/Name: %s",spg_header.author);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Creator: %s",spg_header.creator);
	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	sprintf(buffer_texto,"Saved on: %d/%02d/%02d %02d:%02d:%02d ",
			2000+spg_header.year,spg_header.month,spg_header.day,spg_header.hour,spg_header.minute,spg_header.second);

	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	z80_int zx_pc_reg=spg_header.pc;
        sprintf(buffer_texto,"PC Register: %04XH",zx_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("SPG file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("SPG file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}


void menu_file_zx_browser_show(char *filename)
{
	
	//Leemos cabecera archivo zx
        FILE *ptr_file_zx_browser;
        ptr_file_zx_browser=fopen(filename,"rb");

        if (!ptr_file_zx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leer 201 bytes de la cabecera
	z80_byte zx_header[201];

        int leidos=fread(zx_header,1,201,ptr_file_zx_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        fclose(ptr_file_zx_browser);

        //Testear cabecera "ZX" en los primeros bytes
        if (zx_header[0]!='Z' || zx_header[1]!='X') {
        	debug_printf(VERBOSE_ERR,"Invalid .ZX file");
        	return;
        }

	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	
     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
	z80_byte zx_version=zx_header[38];
	sprintf(buffer_texto,"ZX File version: %d",zx_version);
	//longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
	//sprintf (&texto_browser[indice_buffer],"%s\n",buffer_texto);
 	//indice_buffer +=longitud_texto+1;

 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	if (zx_version>=3) {
 		z80_byte zx_machine=zx_header[71];
 		if (zx_machine<=24) {
 			sprintf(buffer_texto,"Machine: %s",zxfile_machines_id[zx_machine]);
 			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 		}
 	}


 	if (zx_version>=4) {
		sprintf(buffer_texto,"Saved on: %d/%02d/%02d %02d:%02d ",
			value_8_to_16(zx_header[78],zx_header[77]),zx_header[76],zx_header[75],zx_header[79],zx_header[80]);

		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
        }


        z80_int zx_pc_reg=value_8_to_16(zx_header[31],zx_header[30]);
        sprintf(buffer_texto,"PC Register: %04XH",zx_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("ZEsarUX ZX file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("ZEsarUX ZX file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}

void menu_file_z80_browser_show(char *filename)
{
	
	//Leemos cabecera archivo z80
        FILE *ptr_file_z80_browser;
        ptr_file_z80_browser=fopen(filename,"rb");

        if (!ptr_file_z80_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

	//Leemos primeros 30 bytes de la cabecera
	z80_byte z80_header[87];

        int leidos=fread(z80_header,1,30,ptr_file_z80_browser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading file");
                return;
        }


        

        //Ver si version 2 o 3
        z80_byte z80_version=1;

        z80_int z80_pc_reg=value_8_to_16(z80_header[7],z80_header[6]);

        if (z80_pc_reg==0) {
        	z80_version=2; //minimo 2

        	//Leer cabecera adicional 
        	fread(&z80_header[30],1,57,ptr_file_z80_browser);

        	z80_pc_reg=value_8_to_16(z80_header[33],z80_header[32]);

        	if (z80_header[30]!=23) z80_version=3;
        }


        fclose(ptr_file_z80_browser);

       
	char buffer_texto[64]; //2 lineas, por si acaso

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	sprintf(buffer_texto,"Z80 File version: %d",z80_version);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

 	//Maquina
 	if (z80_version==1) {
 		sprintf(buffer_texto,"Machine: Spectrum 48k");
 		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 	}
 	else {
 		z80_byte z80_machine=z80_header[34];
 		if (z80_machine<7) {
 			if (z80_version==2) {
 				if (z80_machine==3 || z80_machine==4) {
 					z80_machine++;
 				}
 			}
 			sprintf(buffer_texto,"Machine: %s",z80file_machines_id[z80_machine]);
 		}

 		else if (z80_machine>=7 && z80_machine<=15) {
 			sprintf(buffer_texto,"Machine: %s",z80file_machines_id[z80_machine]);
 		}

 		else if (z80_machine==128) {
 			sprintf(buffer_texto,"Machine: TS2068");
 		}

 		else sprintf(buffer_texto,"Machine: Unknown");
 		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
 	}

 	sprintf(buffer_texto,"PC Register: %04XH",z80_pc_reg);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("Z80 file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Z80 file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)


}


void menu_file_tzx_browser_show(char *filename)
{

	int filesize=get_file_size(filename);
	z80_byte *tzx_file_mem;

	tzx_file_mem=malloc(filesize);
	if (tzx_file_mem==NULL) {
		debug_printf(VERBOSE_ERR,"Can not allocate memory for tzx browser");
		return;
	}

	
	//Leemos cabecera archivo tzx
        FILE *ptr_file_tzx_browser;
        ptr_file_tzx_browser=fopen(filename,"rb");

        if (!ptr_file_tzx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

        int leidos=fread(tzx_file_mem,1,filesize,ptr_file_tzx_browser);

	if (leidos!=filesize) {
                debug_printf(VERBOSE_ERR,"Error reading file");
		free(tzx_file_mem);
                return;
        }


        fclose(ptr_file_tzx_browser);

        //Testear cabecera "ZXTape!" en los primeros bytes
	char signature[8];
	memcpy(signature,tzx_file_mem,7);
	signature[7]=0;
        if (strcmp(signature,"ZXTape!")) {
        	debug_printf(VERBOSE_ERR,"Invalid .TZX file");
		free(tzx_file_mem);
        	return;
        }

	char buffer_texto[300]; //Para poder contener info de tzx extensa

	//int longitud_bloque;

	//int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	//TODO. controlar que no se salga del maximo de texto_browser
	
	z80_byte tzx_version_major=tzx_file_mem[8];
	z80_byte tzx_version_minor=tzx_file_mem[9];
	sprintf(buffer_texto,"TZX File version: %d.%d",tzx_version_major,tzx_version_minor);
 	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);




	int puntero=10;
	int salir=0;

	for (;puntero<filesize && !salir;) {
		z80_byte tzx_id=tzx_file_mem[puntero++];
		z80_int longitud_bloque,longitud_sub_bloque;
		int longitud_larga;
		char buffer_bloque[256];

		//printf ("ID %02XH puntero: %d indice_buffer: %d\n",tzx_id,puntero,indice_buffer);
		//Aproximacion a llenado de buffer
		if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
			debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
			salir=1;
			break;
		}
		//int subsalir;

		switch (tzx_id) {

			case 0x10:

				//Es un poco molesto mostrar el texto cada vez
				//sprintf(buffer_texto,"ID 10 Std. Speed Data Block");
				//indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


				puntero+=2;
			

			        longitud_sub_bloque=util_tape_tap_get_info(&tzx_file_mem[puntero],buffer_bloque);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

					
				puntero +=longitud_sub_bloque;
				


			break;


			case 0x11:

				
				sprintf(buffer_texto,"ID 11 - Turbo Data Block:");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

				longitud_larga=tzx_file_mem[puntero+15]+256*tzx_file_mem[puntero+16]+65536*tzx_file_mem[puntero+17];
				//printf("longitud larga: %d\n",longitud_larga);

				//Longitud en este tipo de bloque es de 3 bytes, saltamos el primero para que la rutina
				//de obtener cabecera de tap siga funcionando
				puntero+=18;

				//Lo escribimos con espacio
				buffer_bloque[0]=' ';
			        //longitud_sub_bloque=util_tape_tap_get_info(&tzx_file_mem[puntero],&buffer_bloque[1]);
				util_tape_get_info_tapeblock(&tzx_file_mem[puntero+3],tzx_file_mem[puntero+2],longitud_larga,&buffer_bloque[1]);

				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

				puntero +=longitud_larga;
				


			break;



			case 0x20:

				
				sprintf(buffer_texto,"ID 20 - Pause");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);


				puntero+=2;
				


			break;

			case 0x30:
				sprintf(buffer_texto,"ID 30 Text description:");
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

				longitud_bloque=tzx_file_mem[puntero];
				//printf ("puntero: %d longitud: %d\n",puntero,longitud_bloque);
				util_binary_to_ascii(&tzx_file_mem[puntero+1],buffer_bloque,longitud_bloque,longitud_bloque);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

				puntero+=1;
				puntero+=longitud_bloque;
				//printf ("puntero: %d\n",puntero);
			break;

                        case 0x31:
                                sprintf(buffer_texto,"ID 31 Message block:");
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=tzx_file_mem[puntero+1];
                                util_binary_to_ascii(&tzx_file_mem[puntero+2],buffer_bloque,longitud_bloque,longitud_bloque);
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

                                puntero+=2;
                                puntero+=longitud_bloque;
                        break;

                        case 0x32:
                                sprintf(buffer_texto,"ID 32 Archive info:");
                                indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                longitud_bloque=tzx_file_mem[puntero]+256*tzx_file_mem[puntero+1];
                                puntero+=2;

                                z80_byte nstrings=tzx_file_mem[puntero++];

                                char buffer_text_id[256];
                                char buffer_text_text[256];

                                for (;nstrings;nstrings--) {
                                	z80_byte text_type=tzx_file_mem[puntero++];
                                	z80_byte longitud_texto=tzx_file_mem[puntero++];
                                	tape_tzx_get_archive_info(text_type,buffer_text_id);
                                	util_binary_to_ascii(&tzx_file_mem[puntero],buffer_text_text,longitud_texto,longitud_texto);
                                	sprintf (buffer_texto,"%s: %s",buffer_text_id,buffer_text_text);

                                	indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);

                                	puntero +=longitud_texto;
                                }

                        break;

			default:
				sprintf(buffer_texto,"Unhandled TZX ID %02XH",tzx_id);
				indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_texto);
				salir=1;
			break;
		}
	}

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("TZX file browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("TZX file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(tzx_file_mem);

}




void menu_file_pzx_browser_show(char *filename)
{

	int filesize=get_file_size(filename);
	z80_byte *pzx_file_mem;

	pzx_file_mem=malloc(filesize);
	if (pzx_file_mem==NULL) {
		debug_printf(VERBOSE_ERR,"Can not allocate memory for pzx browser");
		return;
	}


	//Leemos cabecera archivo pzx
        FILE *ptr_file_pzx_browser;
        ptr_file_pzx_browser=fopen(filename,"rb");

    if (!ptr_file_pzx_browser) {
		debug_printf(VERBOSE_ERR,"Unable to open file");
		return;
	}

        int leidos=fread(pzx_file_mem,1,filesize,ptr_file_pzx_browser);

	if (leidos!=filesize) {
                debug_printf(VERBOSE_ERR,"Error reading file");
		free(pzx_file_mem);
                return;
        }


        fclose(ptr_file_pzx_browser);

      
	//char buffer_texto[300]; //Para poder contener info de pzx extensa

 

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	//Ir leyendo hasta llegar al final del archivo
	//z80_long_int puntero_lectura=0;
	long int puntero_lectura=0;

	char buffer_bloque[512];
	//char buffer_bloque2[512];
	//char buffer_bloque3[512];

	int salir=0;

	while (puntero_lectura<filesize && !salir) {

		if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
			debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
			//printf ("bucle inicial %d %d %d\n",indice_buffer,MAX_TEXTO_BROWSER,MAX_TEXTO_GENERIC_MESSAGE);
			salir=1;
			break;
		}

		/*
		Leer datos identificador de bloque
		offset type     name   meaning
		0      u32      tag    unique identifier for the block type.
		4      u32      size   size of the block in bytes, excluding the tag and size fields themselves.
		8      u8[size] data   arbitrary amount of block data.
		*/

		char tag_name[5];
		tag_name[0]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[1]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[2]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[3]=util_return_valid_ascii_char(pzx_file_mem[puntero_lectura++]);
		tag_name[4]=0;

		z80_long_int block_size;
		
		

		block_size=pzx_file_mem[puntero_lectura]+
					(pzx_file_mem[puntero_lectura+1]*256)+
					(pzx_file_mem[puntero_lectura+2]*65536)+
					(pzx_file_mem[puntero_lectura+3]*16777216);   
		puntero_lectura +=4;                     

		//printf ("Block tag name: [%s] size: [%u]\n",tag_name,block_size);

		sprintf(buffer_bloque,"%s Block",tag_name);
		indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


		//Tratar cada tag
		if (!strcmp(tag_name,"PZXT")) {
			z80_byte pzx_version_major=pzx_file_mem[puntero_lectura];
        	z80_byte pzx_version_minor=pzx_file_mem[puntero_lectura+1];

        	sprintf(buffer_bloque," file version: %d.%d",pzx_version_major,pzx_version_minor);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

			z80_byte *memoria;
			memoria=&pzx_file_mem[puntero_lectura];			


			int block_size_tag=block_size;
			block_size_tag -=2;
			memoria +=2;

			//Los strings los vamos guardando en un array de char separado. Asumimos que ninguno ocupa mas de 1024. Si es asi, esto petara...

			char text_string[1024];
			int index_string=0;

			while (block_size_tag>0) {
					char caracter=*memoria;

					if (caracter==0) {
							text_string[index_string++]=0;
							//fin de cadena
							sprintf(buffer_bloque," %s",text_string);
							indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);
							index_string=0;
					}

					else {
							text_string[index_string++]=util_return_valid_ascii_char(caracter);
					}

					memoria++;
					block_size_tag--;

			}

			//Final puede haber acabado con byte 0 o no. Lo metemos por si acaso
			if (index_string!=0) {
					text_string[index_string++]=0;
					sprintf(buffer_bloque," %s",text_string);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);
			}



		}

		else if (!strcmp(tag_name,"PULS")) {
				//convert_pzx_to_rwa_tag_puls(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

				z80_byte *memoria;
				memoria=&pzx_file_mem[puntero_lectura];

			z80_int count;
			int duration; 

			//int valor_pulso_inicial=0;
			//int t_estado_actual=*p_t_estado_actual;

			int block_size_tag=block_size;

			while (block_size_tag>0 && !salir) {

					count = 1 ;

					//duration = fetch_u16() ;
					duration = (*memoria)|((memoria[1])<<8);
					memoria +=2;
					block_size_tag -=2;

					if ( duration > 0x8000 ) {
							count = duration & 0x7FFF ;
							//duration = fetch_u16() ;
							duration = (*memoria)|((memoria[1])<<8);
							memoria +=2;
							block_size_tag -=2;
					}
					if ( duration >= 0x8000 ) {
							duration &= 0x7FFF ;
							duration <<= 16 ;
							//duration |= fetch_u16() ;
							duration |= (*memoria)|((memoria[1])<<8);
							memoria +=2;
							block_size_tag -=2;
					}

					//printf ("count: %d duration: %d\n",count,duration);
					//printf("size: %d count: %d duration: %d\n",block_size_tag,count,duration);

					sprintf(buffer_bloque," count: %d duration: %d",count,duration);
					indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

					//Proteccion aqui tambien porque pueden generarse muchos bloques en este bucle
					if (indice_buffer>=MAX_TEXTO_BROWSER-1024) {
							debug_printf(VERBOSE_ERR,"Too many entries. Showing only what is allowed on memory");
							salir=1;
							break;
					}					

					
			}



		}

		else if (!strcmp(tag_name,"DATA")) {
				//convert_pzx_to_rwa_tag_data(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

				z80_byte *memoria;
				memoria=&pzx_file_mem[puntero_lectura];

  				int initial_pulse;

        z80_long_int count;   

        //int t_estado_actual=*p_t_estado_actual;


        count=memoria[0]+
                (memoria[1]*256)+
                (memoria[2]*65536)+
                ((memoria[3]&127)*16777216);

        initial_pulse=(memoria[3]&128)>>7;

        memoria +=4;

        z80_int tail=memoria[0]+
                (memoria[1]*256);

        memoria +=2;
        
        z80_byte num_pulses_zero=*memoria;
        memoria++;

        z80_byte num_pulses_one=*memoria;
        memoria++;

        //Secuencias que identifican a un cero y un uno
        z80_int seq_pulses_zero[256];
        z80_int seq_pulses_one[256];

        //Metemos las secuencias de 0 y 1 en array
        int i;
        for (i=0;i<num_pulses_zero;i++) {
               seq_pulses_zero[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }


        for (i=0;i<num_pulses_one;i++) {
               seq_pulses_one[i]=memoria[0]+(memoria[1]*256);

                memoria +=2;
        }

					   

    
        //Procesar el total de bits
        //int bit_number=7;
        //z80_byte processing_byte;

        //z80_int *sequence_bit;
        //int longitud_sequence_bit;

        //z80_long_int total_bits_read; 

	     


			//Saltamos flag
			z80_byte flag=*memoria;
			memoria++;

			//Metemos un espacio delante
			buffer_bloque[0]=' ';
			util_tape_get_info_tapeblock(memoria,flag,count/8,&buffer_bloque[1]);	
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);	


			//Y debug del bloque
		    sprintf(buffer_bloque," Length: %d (%d bits)",count/8,count);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);

        	sprintf(buffer_bloque," count: %d initial_pulse: %d tail: %d num_pulses_0: %d num_pulses_1: %d",
            		count,initial_pulse,tail,num_pulses_zero,num_pulses_one); 
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);		             


		}                

		else if (!strcmp(tag_name,"PAUS")) {
				//convert_pzx_to_rwa_tag_paus(&pzx_file_mem[puntero_lectura],block_size,ptr_destino,&estado_actual);

			int initial_pulse;

			z80_long_int count;   

							z80_byte *memoria;
					memoria=&pzx_file_mem[puntero_lectura];

	


			count=memoria[0]+
					(memoria[1]*256)+
					(memoria[2]*65536)+
					((memoria[3]&127)*16777216);

			initial_pulse=(memoria[3]&128)>>7;

			memoria +=4;

			sprintf(buffer_bloque," count: %d",count);
			indice_buffer +=util_add_string_newline(&texto_browser[indice_buffer],buffer_bloque);


		}   

		else {
			//debug_printf (VERBOSE_DEBUG,"PZX: Unknown block type: %02XH %02XH %02XH %02XH. Skipping it",
			//    tag_name[0],tag_name[1],tag_name[2],tag_name[3]);
		}             


		//Y saltar al siguiente bloque
		puntero_lectura +=block_size;
		
	}


	texto_browser[indice_buffer]=0;
	zxvision_generic_message_tooltip("PZX file browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	free(pzx_file_mem);

}





void menu_tape_browser_show(char *filename)
{

	//Si tzx o cdt
	if (!util_compare_file_extension(filename,"tzx") ||
	    !util_compare_file_extension(filename,"cdt")
		) {
		menu_file_tzx_browser_show(filename);
		return;
	}

	//Si pzx
	if (!util_compare_file_extension(filename,"pzx") 
		) {
		menu_file_pzx_browser_show(filename);
		return;
	}	

	//tapefile
	if (util_compare_file_extension(filename,"tap")!=0) {
		debug_printf(VERBOSE_ERR,"Tape browser not supported for this tape type");
		return;
	}

	//Leemos cinta en memoria
	int total_mem=get_file_size(filename);

	z80_byte *taperead;



        FILE *ptr_tapebrowser;
        ptr_tapebrowser=fopen(filename,"rb");

        if (!ptr_tapebrowser) {
		debug_printf(VERBOSE_ERR,"Unable to open tape");
		return;
	}

	taperead=malloc(total_mem);
	if (taperead==NULL) cpu_panic("Error allocating memory for tape browser");

	z80_byte *puntero_lectura;
	puntero_lectura=taperead;


        int leidos=fread(taperead,1,total_mem,ptr_tapebrowser);

	if (leidos==0) {
                debug_printf(VERBOSE_ERR,"Error reading tape");
		free(taperead);
                return;
        }


        fclose(ptr_tapebrowser);

	char buffer_texto[40];

	int longitud_bloque;

	int longitud_texto;

	char texto_browser[MAX_TEXTO_BROWSER];
	int indice_buffer=0;

	while(total_mem>0) {
		longitud_bloque=util_tape_tap_get_info(puntero_lectura,buffer_texto);
		total_mem-=longitud_bloque;
		puntero_lectura +=longitud_bloque;
		debug_printf (VERBOSE_DEBUG,"Tape browser. Block: %s",buffer_texto);


     //printf ("nombre: %s c1: %d\n",buffer_nombre,buffer_nombre[0]);
                        longitud_texto=strlen(buffer_texto)+1; //Agregar salto de linea
                        if (indice_buffer+longitud_texto>MAX_TEXTO_BROWSER-1) {
				debug_printf (VERBOSE_ERR,"Too much headers. Showing only the allowed in memory");
				total_mem=0; //Finalizar bloque
			}

                        else {
                                sprintf (&texto_browser[indice_buffer],"%s\n",buffer_texto);
                                indice_buffer +=longitud_texto;
                        }

	}

	texto_browser[indice_buffer]=0;
	//menu_generic_message_tooltip("Tape browser", 0, 0, 1, NULL, "%s", texto_browser);
	zxvision_generic_message_tooltip("Tape browser" , 0 , 0, 0, 1, NULL, 1, "%s", texto_browser);

	//int util_tape_tap_get_info(z80_byte *tape,char *texto)

	free(taperead);

}



void menu_tape_browser(MENU_ITEM_PARAMETERS)
{
	menu_tape_browser_show(tapefile);
}

void menu_tape_browser_output(MENU_ITEM_PARAMETERS)
{
	menu_tape_browser_show(tape_out_file);
}

void menu_tape_browser_real(MENU_ITEM_PARAMETERS)
{
	menu_tape_browser_show(realtape_name);
}

//menu tape settings
void menu_tape_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_tape_settings;
	menu_item item_seleccionado;
	int retorno_menu;

        do {
                char string_tape_load_shown[20],string_tape_load_inserted[50],string_tape_save_shown[20],string_tape_save_inserted[50];
		char string_realtape_shown[23];

		menu_add_item_menu_inicial_format(&array_menu_tape_settings,MENU_OPCION_NORMAL,NULL,NULL,"--Standard Tape--");
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Select Standard tape for Input and Output");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Standard tapes are those handled by ROM routines and "
					"have normal speed (no turbo). These tapes are handled by the emulator and loaded or saved "
					"very quickly. The input formats supported are: o, p, tap, tzx (binary only), rwa, wav and smp; "
					"output formats are: o, p, tap and tzx. Those input audio formats (rwa, wav or smp) "
					"are converted by the emulator to bytes and loaded on the machine memory. For every other non standard "
					"tapes (turbo or handled by non-ROM routines like loading stripes on different colours) you must use "
					"Real Input tape for load, and Audio output to file for saving");


		menu_tape_settings_trunc_name(tapefile,string_tape_load_shown,20);
		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_open,NULL,"~~Input [%s]",string_tape_load_shown);
		menu_add_item_menu_shortcut(array_menu_tape_settings,'i');


		sprintf (string_tape_load_inserted,"[%c] Input tape inserted",((tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0 ? 'X' : ' '));
		menu_add_item_menu(array_menu_tape_settings,string_tape_load_inserted,MENU_OPCION_NORMAL,menu_tape_input_insert,menu_tape_input_insert_cond);

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser,menu_tape_input_insert_cond,"Tape ~~Browser");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'b');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Input tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Input tape");


		menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                menu_tape_settings_trunc_name(tape_out_file,string_tape_save_shown,20);
		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_out_open,NULL,"~~Output [%s]",string_tape_save_shown);
		menu_add_item_menu_shortcut(array_menu_tape_settings,'o');

                sprintf (string_tape_save_inserted,"[%c] Output tape inserted",((tape_loadsave_inserted & TAPE_SAVE_INSERTED)!=0 ? 'X' : ' '));
                menu_add_item_menu(array_menu_tape_settings,string_tape_save_inserted,MENU_OPCION_NORMAL,menu_tape_output_insert,menu_tape_output_insert_cond);

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser_output,menu_tape_output_insert_cond,"Tape B~~rowser");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'r');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Output tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Output tape");				


        	        menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,NULL,NULL,"--Input Real Tape--");
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Input Real Tape at normal loading Speed");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"You may select any input valid tape: o, p, tap, tzx, rwa, wav, smp. "
					"This tape is handled the same way as the real machine does, at normal loading speed, and may "
					"select tapes with different loading methods instead of the ROM: turbo loading, alkatraz, etc...\n"
					"When inserted real tape, realvideo is enabled, only to show real loading stripes on screen, but it is "
					"not necessary, you may disable realvideo if you want");


                menu_tape_settings_trunc_name(realtape_name,string_realtape_shown,23);
                menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_open,NULL,"~~File [%s]",string_realtape_shown);
		menu_add_item_menu_shortcut(array_menu_tape_settings,'f');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Audio file to use as the input audio");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Audio file to use as the input audio");


		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_insert,menu_realtape_cond,"[%c] Inserted", (realtape_inserted.v==1 ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Insert the audio file");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Insert the audio file");



		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_realtape_play,menu_realtape_inserted_cond,"[%c] ~~Playing", (realtape_playing.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_tape_settings,'p');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Start playing the audio tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Start playing the audio tape");

		menu_add_item_menu_format(array_menu_tape_settings,MENU_OPCION_NORMAL,menu_tape_browser_real,menu_realtape_cond,"Tape Bro~~wser");
		menu_add_item_menu_shortcut(array_menu_tape_settings,'w');
		menu_add_item_menu_tooltip(array_menu_tape_settings,"Browse Real tape");
		menu_add_item_menu_ayuda(array_menu_tape_settings,"Browse Real tape");				



                menu_add_item_menu(array_menu_tape_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);




                //menu_add_item_menu(array_menu_tape_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_tape_settings);

                retorno_menu=menu_dibuja_menu(&tape_settings_opcion_seleccionada,&item_seleccionado,array_menu_tape_settings,"Tape" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);


}


void menu_snapshot_load(MENU_ITEM_PARAMETERS)
{

        char *filtros[15];

        filtros[0]="zx";
        filtros[1]="sp";
        filtros[2]="z80";
        filtros[3]="sna";
        filtros[4]="o";
        filtros[5]="p";

        filtros[6]="80";
        filtros[7]="81";
        filtros[8]="z81";
        filtros[9]="ace";
		filtros[10]="rzx";
		filtros[11]="zsf";
		filtros[12]="spg";
		filtros[13]="nex";
        filtros[14]=0;



        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de snap
        //si no hay directorio, vamos a rutas predefinidas
        if (snapfile==NULL) menu_chdir_sharedfiles();

	else {
	        char directorio[PATH_MAX];
	        util_get_dir(snapfile,directorio);
	        //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

	        //cambiamos a ese directorio, siempre que no sea nulo
	        if (directorio[0]!=0) {
	                debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
	                menu_filesel_chdir(directorio);
        	}
	}


	int ret;

	ret=menu_filesel("Select Snapshot",filtros,snapshot_load_file);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


        if (ret==1) {
		snapfile=snapshot_load_file;

		//sin overlay de texto, que queremos ver las franjas de carga con el color normal (no apagado)
		reset_menu_overlay_function();


			snapshot_load();

		//restauramos modo normal de texto de menu
		set_menu_overlay_function(normal_overlay_texto_menu);

		//Y salimos de todos los menus
		salir_todos_menus=1;
        }


}

void menu_snapshot_save(MENU_ITEM_PARAMETERS)
{

    char *filtros[6];

  	if (MACHINE_IS_ZX8081) {
		filtros[0]="zx";

		if (MACHINE_IS_ZX80) filtros[1]="o";
		else filtros[1]="p";

		filtros[2]="zsf";
		filtros[3]=0;
}

	else if (MACHINE_IS_Z88) {
		filtros[0]="zx";
		filtros[1]="zsf";
		filtros[2]=0;
	}

	else if (MACHINE_IS_SPECTRUM_16_48) {
		filtros[0]="zx";
		filtros[1]="z80";
		filtros[2]="sp";
		filtros[3]="zsf";
		filtros[4]="sna";
		filtros[5]=0;
	}

	else if (MACHINE_IS_ACE) {
		filtros[0]="zx";
		filtros[1]="ace";
		filtros[2]="zsf";
		filtros[3]=0;
	}

	else if (MACHINE_IS_CPC) {
		filtros[0]="zx";
		filtros[1]="zsf";
		filtros[2]=0;
    }


	else {
		filtros[0]="zx";
		filtros[1]="z80";
		filtros[2]="zsf";
		filtros[3]="sna";
		filtros[4]=0;
	}


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);


 		//Obtenemos directorio de snap save
        //si no hay directorio, vamos a rutas predefinidas
        if (snapshot_save_file[0]==0) menu_chdir_sharedfiles();

		else {
			char directorio[PATH_MAX];
			util_get_dir(snapshot_save_file,directorio);
			//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

			//cambiamos a ese directorio, siempre que no sea nulo
			if (directorio[0]!=0) {
					debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
					menu_filesel_chdir(directorio);
			}
		}



		int ret;

		ret=menu_filesel("Snapshot file",filtros,snapshot_save_file);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

			//Ver si archivo existe y preguntar
			struct stat buf_stat;

			if (stat(snapshot_save_file, &buf_stat)==0) {

			if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

		}


		snapshot_save(snapshot_save_file);

		//Si ha ido bien la grabacion
		if (!if_pending_error_message) menu_generic_message_splash("Save Snapshot","OK. Snapshot saved");

			//Y salimos de todos los menus
			salir_todos_menus=1;

        }




}


void menu_snapshot_save_version(MENU_ITEM_PARAMETERS)
{
	snap_zx_version_save++;
	if (snap_zx_version_save>CURRENT_ZX_VERSION) snap_zx_version_save=1;
}

void menu_snapshot_permitir_versiones_desconocidas(MENU_ITEM_PARAMETERS)
{
	snap_zx_permitir_versiones_desconocidas.v ^=1;
}

void menu_snapshot_autosave_at_interval(MENU_ITEM_PARAMETERS)
{
	snapshot_contautosave_interval_enabled.v ^=1;

	//resetear contador
	snapshot_autosave_interval_current_counter=0;
}

void menu_snapshot_autosave_at_interval_seconds(MENU_ITEM_PARAMETERS)
{
	char string_segundos[3];

sprintf (string_segundos,"%d",snapshot_autosave_interval_seconds);

	menu_ventana_scanf("Seconds: ",string_segundos,3);

	int valor_leido=parse_string_to_number(string_segundos);

	if (valor_leido<0 || valor_leido>999) debug_printf(VERBOSE_ERR,"Invalid interval");

	else snapshot_autosave_interval_seconds=valor_leido;
}

void menu_snapshot_autosave_at_interval_prefix(MENU_ITEM_PARAMETERS)
{
	char string_prefix[30];
	//Aunque el limite real es PATH_MAX, lo limito a 30

	sprintf (string_prefix,"%s",snapshot_autosave_interval_quicksave_name);

	menu_ventana_scanf("Name prefix: ",string_prefix,30);

	if (string_prefix[0]==0) return;

	strcpy(snapshot_autosave_interval_quicksave_name,string_prefix);
}


void menu_snapshot_autosave_at_interval_directory(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(snapshot_autosave_interval_quicksave_directory);
}


void menu_snapshot_stop_rzx_play(MENU_ITEM_PARAMETERS)
{
	eject_rzx_file();
}

void menu_snapshot_quicksave(MENU_ITEM_PARAMETERS)
{
	char final_name[PATH_MAX];
	snapshot_quick_save(final_name);


	menu_generic_message_format("Quicksave","OK. Snapshot name: %s",final_name);

}

void menu_snapshot_save_game_config(MENU_ITEM_PARAMETERS)
{

        char *filtros[29];

        filtros[0]="zx";
        filtros[1]="sp";
        filtros[2]="z80";
        filtros[3]="sna";

        filtros[4]="o";
        filtros[5]="p";
        filtros[6]="80";
        filtros[7]="81";
        filtros[8]="z81";

        filtros[9]="tzx";
        filtros[10]="tap";

        filtros[11]="rwa";
        filtros[12]="smp";
        filtros[13]="wav";

        filtros[14]="epr";
        filtros[15]="63";
        filtros[16]="eprom";
        filtros[17]="flash";

        filtros[18]="ace";

        filtros[19]="dck";

        filtros[20]="cdt";

        filtros[21]="ay";

        filtros[22]="scr";

        filtros[23]="rzx";

        filtros[24]="zsf";

        filtros[25]="spg";

        filtros[26]="trd";

        filtros[27]="config";

        filtros[28]=0;

	char source_file[PATH_MAX];
	char game_config_file[PATH_MAX];


	//por defecto
	source_file[0]=0;

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

	int ret;

        //Obtenemos directorio de quickload, para generar el .config en la ultima ruta que se haya hecho smartload
        //si no hay directorio, vamos a rutas predefinidas
        if (quickfile!=NULL)  {
			debug_printf (VERBOSE_INFO,"Last smartload file: %s",quickfile);

			char nombre[PATH_MAX];
			util_get_file_no_directory(quickfile,nombre);


			int usar_nombre_autodetectado;
		
     

			//Si nombre vacio, no usar nombre autodetectado
			if (nombre[0]==0) {
				usar_nombre_autodetectado=0;
			}

			else {
				//Nombre previo. El usuario quiere usarlo?
				char nombre_shown[25];
				menu_tape_settings_trunc_name(nombre,nombre_shown,25);
				usar_nombre_autodetectado=menu_confirm_yesno_texto("Generate config for",nombre_shown);

			}

			if (usar_nombre_autodetectado) {
				strcpy(source_file,quickfile);
				ret=1;
			}


			else {
				//No seleccionamos ultimo quickfile como source. Cambiar a directorio quickfile
	              char directorio[PATH_MAX];
        	        util_get_dir(quickfile,directorio);
                	//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

	                //cambiamos a ese directorio, siempre que no sea nulo
        	        if (directorio[0]!=0) {
                	        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        	menu_filesel_chdir(directorio);
	        }


		}

    }

	//Si no se selecciona source_file como ultimo archivo cargado
	if (source_file[0]==0) {

			ret=menu_filesel("Source or dest file",filtros,source_file);
			//volvemos a directorio inicial
			menu_filesel_chdir(directorio_actual);

	}

	debug_printf (VERBOSE_INFO,"Source file: %s",source_file);

    if (ret) {

		//Archivo final agregar .config, si es que no es ya el .config el que hemos seleccionado
		if (!util_compare_file_extension(source_file,"config")) strcpy(game_config_file,source_file);
		else sprintf (game_config_file,"%s.config",source_file);


		debug_printf (VERBOSE_INFO,"Destination file will be %s",game_config_file);

		 //Ver si archivo existe y preguntar
                if (si_existe_archivo(game_config_file) ) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }



		util_save_game_config(game_config_file);
		menu_generic_message_splash("Save autoconfig","OK. File saved");


    }
                
}

void menu_snapshot(MENU_ITEM_PARAMETERS)
{

        menu_item *array_menu_snapshot;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial(&array_menu_snapshot,"~~Load snapshot",MENU_OPCION_NORMAL,menu_snapshot_load,NULL);
		menu_add_item_menu_shortcut(array_menu_snapshot,'l');
		menu_add_item_menu_tooltip(array_menu_snapshot,"Load snapshot");
		menu_add_item_menu_ayuda(array_menu_snapshot,"Supported snapshot formats on load:\n"
					"Z80, ZX, SP, SNA, O, 80, P, 81, Z81");

                menu_add_item_menu(array_menu_snapshot,"~~Save snapshot",MENU_OPCION_NORMAL,menu_snapshot_save,NULL);
		menu_add_item_menu_shortcut(array_menu_snapshot,'s');
		menu_add_item_menu_tooltip(array_menu_snapshot,"Save snapshot of the current machine state");
		menu_add_item_menu_ayuda(array_menu_snapshot,"Supported snapshot formats on save:\n"
					"Z80, ZX, SP, P, O\n"
					"You must write the file name with the extension");

					menu_add_item_menu(array_menu_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);

					if (rzx_reproduciendo) {
						menu_add_item_menu(array_menu_snapshot,"Stop RZX Play",MENU_OPCION_NORMAL,menu_snapshot_stop_rzx_play,NULL);
						menu_add_item_menu(array_menu_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);
					}


					menu_add_item_menu_format(array_menu_snapshot,MENU_OPCION_NORMAL,menu_snapshot_quicksave,NULL,"~~Quicksave");
					menu_add_item_menu_shortcut(array_menu_snapshot,'q');
					menu_add_item_menu_tooltip(array_menu_snapshot,"Save a snapshot quickly");
					menu_add_item_menu_ayuda(array_menu_snapshot,"Save a snapshot quickly. Name prefix and directory to save are configured on settings->Snapshot");


				menu_add_item_menu(array_menu_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);


				menu_add_item_menu_format(array_menu_snapshot,MENU_OPCION_NORMAL,menu_snapshot_save_game_config,NULL,"Save a~~utoconfig file");
				menu_add_item_menu_shortcut(array_menu_snapshot,'u');
				menu_add_item_menu_tooltip(array_menu_snapshot,"Generate .config file with common settings");
				menu_add_item_menu_ayuda(array_menu_snapshot,"Generate .config file with common settings. Used to define custom settings for games, "
					"by default it asks to generate a .config file for the last smartloaded game");




                menu_add_item_menu(array_menu_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_snapshot);

                retorno_menu=menu_dibuja_menu(&snapshot_opcion_seleccionada,&item_seleccionado,array_menu_snapshot,"Snapshot" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}



void menu_debug_reset(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Reset CPU")==1) {
		reset_cpu();
                //Y salimos de todos los menus
                salir_todos_menus=1;
	}

}


/*void menu_debug_prism_usr0(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Set PC=0?")==1) {
		reg_pc=0;
		menu_generic_message("Prism Set PC=0","OK. Set PC=0");
		salir_todos_menus=1;
	}
}
*/


void menu_debug_prism_failsafe(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Reset to failsafe?")==1) {
                reset_cpu();

		prism_failsafe_mode.v=1;
		//Para aplicar cambio de pagina rom
		prism_set_memory_pages();

                //Y salimos de todos los menus
                salir_todos_menus=1;
	}
}

void menu_debug_hard_reset(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno("Hard Reset CPU")==1) {
                hard_reset_cpu();
                //Y salimos de todos los menus
                salir_todos_menus=1;
        }

}


void menu_debug_nmi(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno("Generate NMI")==1) {

		generate_nmi();

                //Y salimos de todos los menus
                salir_todos_menus=1;
        }

}

void menu_debug_nmi_multiface_tbblue(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno("Generate Multiface NMI")==1) {

		generate_nmi_multiface_tbblue();

                //Y salimos de todos los menus
                salir_todos_menus=1;
        }

}

void menu_debug_special_nmi(MENU_ITEM_PARAMETERS)
{
        if (menu_confirm_yesno("Generate Special NMI")==1) {

		char string_nmi[4];
		int valor_nmi;

		sprintf (string_nmi,"0");

		//Pedir valor de NMIEVENT
	        menu_ventana_scanf("NMIEVENT value (0-255)",string_nmi,4);

        	valor_nmi=parse_string_to_number(string_nmi);

	        if (valor_nmi<0 || valor_nmi>255) {
        	        debug_printf (VERBOSE_ERR,"Invalid value %d",valor_nmi);
                	return;
	        }


		generate_nmi();

		//meter BOOTM a 1 (bit 0)
		zxuno_ports[0] |=1;

		zxuno_set_memory_pages();

		//Mapear sram 13
		//zxuno_memory_paged_new(13);
		zxuno_memory_paged_brandnew[3*2]=zxuno_sram_mem_table_new[13];
		zxuno_memory_paged_brandnew[3*2+1]=zxuno_sram_mem_table_new[13]+8192;

		//Valor nmievent
		zxuno_ports[8]=valor_nmi;

                //Y salimos de todos los menus
                salir_todos_menus=1;
        }

}





void menu_run_mantransfer(MENU_ITEM_PARAMETERS)
{
	//Cargar mantransfer.bin
	char *mantransfefilename="mantransfev3.bin";
                FILE *ptr_mantransfebin;
                int leidos;

                open_sharedfile(mantransfefilename,&ptr_mantransfebin);
                if (!ptr_mantransfebin)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open mantransfer binary file %s",mantransfefilename);
                        return;
                }

	#define MAX_MANTRANSFE_BIN 1024
	z80_byte buffer[MAX_MANTRANSFE_BIN];

	leidos=fread(buffer,1,MAX_MANTRANSFE_BIN,ptr_mantransfebin);

	//pasarlo a memoria
	int i;

	for (i=0;i<leidos;i++) {
		poke_byte_no_time(16384+i,buffer[i]);
	}

	fclose(ptr_mantransfebin);

	//Establecer modo im1 o im2
	//Al inicio hay
	//regsp defw 0
	//im 1  (0xed,0x56)

	//im2 seria 0xed, 0x5e

	int offset_im_opcode=16384+2+1;
	z80_byte opcode;

	if (im_mode==1 || im_mode==0) opcode=0x56;
	//im2
	else opcode=0x5e;

	poke_byte_no_time(offset_im_opcode,opcode);

	debug_printf (VERBOSE_INFO,"Running mantransfer saving routine");

	//y saltar a la rutina de grabacion de mantransfe
	//Si se cambia la rutina, hay que cambiar este salto tambien
        push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);
        reg_pc=16384+0x32;

        //Y salimos de todos los menus
        salir_todos_menus=1;

}




menu_z80_moto_int load_binary_last_address=0;
menu_z80_moto_int load_binary_last_length=0;
menu_z80_moto_int save_binary_last_address=0;
menu_z80_moto_int save_binary_last_length=1;



//Retorna 0 si ok
//1 si archivo no encontrado
//2 si error leyendo
//Tiene en cuenta zonas de memoria
int menu_load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud)
{
  int returncode=0;

  if (!si_existe_archivo(binary_file_load)) return 1;

  if (valor_leido_longitud==0) valor_leido_longitud=4194304; //4 MB max

                /*if (MACHINE_IS_SPECTRUM) {
                  if (valor_leido_longitud==0 || valor_leido_longitud>65536) valor_leido_longitud=65536;

                  //maximo hasta direccion 65535
                  if (valor_leido_direccion+valor_leido_longitud > 65535) valor_leido_longitud=65536-valor_leido_direccion;
                }

                else { //MOTOROLA
                  if (valor_leido_longitud==0) valor_leido_longitud=QL_MEM_LIMIT+1;
                }*/

  		menu_debug_set_memory_zone_attr();


  		char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
        	menu_get_current_memory_zone_name_number(zone_name);

                debug_printf(VERBOSE_INFO,"Loading %s file at %d address at zone %s with maximum %d bytes",binary_file_load,valor_leido_direccion,zone_name,valor_leido_longitud);



                                FILE *ptr_binaryfile_load;
                                  ptr_binaryfile_load=fopen(binary_file_load,"rb");
                                  if (!ptr_binaryfile_load)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",binary_file_load);
                                      returncode=2;

                                  }
                                else {

                                                int leidos=1;
                                                z80_byte byte_leido;
                                                while (valor_leido_longitud>0 && leidos>0) {
                                                        leidos=fread(&byte_leido,1,1,ptr_binaryfile_load);
                                                        if (leidos>0) {
                                                                //poke_byte_no_time(valor_leido_direccion,byte_leido);
                                                                //poke_byte_z80_moto(valor_leido_direccion,byte_leido);
                                                                menu_debug_write_mapped_byte(valor_leido_direccion,byte_leido);


                                                                valor_leido_direccion++;
                                                                valor_leido_longitud--;
                                                        }
                                                }


                                  fclose(ptr_binaryfile_load);

                                }

    return returncode;

}


void menu_debug_load_binary(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

	filtros[0]="";
	filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

	int ret;

        //Obtenemos ultimo directorio visitado
        if (binary_file_load[0]!=0) {
                char directorio[PATH_MAX];
                util_get_dir(binary_file_load,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

        ret=menu_filesel("Select File to Load",filtros,binary_file_load);

        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

		cls_menu_overlay();

		menu_debug_set_memory_zone_attr();


		menu_debug_change_memory_zone();

  		

        	char string_direccion[10];

		sprintf (string_direccion,"%XH",load_binary_last_address);

	        menu_ventana_scanf("Address: ",string_direccion,10);

	        int valor_leido_direccion=parse_string_to_number(string_direccion);

					//printf ("valor: %d\n",valor_leido_direccion);

		/*if (valor_leido_direccion>65535 && MACHINE_IS_SPECTRUM) {
			debug_printf (VERBOSE_ERR,"Invalid address %d",valor_leido_direccion);
			//menu_generic_message ("Error","Invalid address");
			return;
		}*/

		load_binary_last_address=valor_leido_direccion;

		cls_menu_overlay();

                char string_longitud[8];

		sprintf (string_longitud,"%d",load_binary_last_length);

                menu_ventana_scanf("Length: 0 - all",string_longitud,8);

                int valor_leido_longitud=parse_string_to_number(string_longitud);

		load_binary_last_length=valor_leido_longitud;

		menu_load_binary_file(binary_file_load,valor_leido_direccion,valor_leido_longitud);


		//Y salimos de todos los menus
		salir_todos_menus=1;


        }




}

void menu_debug_save_binary(MENU_ITEM_PARAMETERS)
{

     char *filtros[2];

                filtros[0]="";
                filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

	int ret;

        //Obtenemos ultimo directorio visitado
        if (binary_file_save[0]!=0) {
                char directorio[PATH_MAX];
                util_get_dir(binary_file_save,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

        ret=menu_filesel("Select File to Save",filtros,binary_file_save);

        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


    if (ret==1) {

		//Ver si archivo existe y preguntar
	        struct stat buf_stat;

      if (stat(binary_file_save, &buf_stat)==0) {

				if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

			}

                cls_menu_overlay();

                menu_debug_set_memory_zone_attr();


				menu_debug_change_memory_zone();



                char string_direccion[10];

                sprintf (string_direccion,"%XH",save_binary_last_address);

                menu_ventana_scanf("Address: ",string_direccion,10);

                int valor_leido_direccion=parse_string_to_number(string_direccion);

                /*if (MACHINE_IS_SPECTRUM && valor_leido_direccion>65535) {
                        debug_printf (VERBOSE_ERR,"Invalid address %d",valor_leido_direccion);
			//menu_generic_message ("Error","Invalid address");
                        return;
                }*/

		save_binary_last_address=valor_leido_direccion;

                cls_menu_overlay();

                char string_longitud[8];

                sprintf (string_longitud,"%d",save_binary_last_length);

                menu_ventana_scanf("Length: 0 - all",string_longitud,8);

                int valor_leido_longitud=parse_string_to_number(string_longitud);

		save_binary_last_length=valor_leido_longitud;						


			menu_debug_set_memory_zone_attr();

			

		if (valor_leido_longitud==0) valor_leido_longitud=menu_debug_memory_zone_size;			

 

		


		char zone_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
        	menu_get_current_memory_zone_name_number(zone_name);


		debug_printf(VERBOSE_INFO,"Saving %s file at %d address at zone %s with %d bytes",binary_file_save,valor_leido_direccion,zone_name,valor_leido_longitud);

                                FILE *ptr_binaryfile_save;
                                  ptr_binaryfile_save=fopen(binary_file_save,"wb");
                                  if (!ptr_binaryfile_save)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Binary file %s",binary_file_save);
					//menu_generic_message ("Error","Unable to open Binary file %s",binary_file_save);

                                  }
                                else {



                                                int escritos=1;
                                                z80_byte byte_leido;
                                                while (valor_leido_longitud>0 && escritos>0) {
							//byte_leido=peek_byte_no_time(valor_leido_direccion);
							//byte_leido=peek_byte_z80_moto(valor_leido_direccion);
							byte_leido=menu_debug_get_mapped_byte(valor_leido_direccion);

							escritos=fwrite(&byte_leido,1,1,ptr_binaryfile_save);
                                                        valor_leido_direccion++;
                                                        valor_leido_longitud--;
                                                }


                                  fclose(ptr_binaryfile_save);

		                //Y salimos de todos los menus
                		salir_todos_menus=1;

                                }




        }




}



void menu_debug_lost_vsync(MENU_ITEM_PARAMETERS)
{
	simulate_lost_vsync.v ^=1;
}

void menu_input_file_keyboard_insert(MENU_ITEM_PARAMETERS)
{

        if (input_file_keyboard_inserted.v==0) {
                input_file_keyboard_init();
        }

        else if (input_file_keyboard_inserted.v==1) {
                input_file_keyboard_close();
        }

}



int menu_input_file_keyboard_cond(void)
{
        if (input_file_keyboard_name!=NULL) return 1;
        else return 0;
}

void menu_input_file_keyboard_delay(MENU_ITEM_PARAMETERS)
{
        if (input_file_keyboard_delay==1) input_file_keyboard_delay=2;
        else if (input_file_keyboard_delay==2) input_file_keyboard_delay=5;
        else if (input_file_keyboard_delay==5) input_file_keyboard_delay=10;
        else if (input_file_keyboard_delay==10) input_file_keyboard_delay=25;
        else if (input_file_keyboard_delay==25) input_file_keyboard_delay=50;

        else input_file_keyboard_delay=1;
}

void menu_input_file_keyboard(MENU_ITEM_PARAMETERS)
{

        input_file_keyboard_inserted.v=0;
		input_file_keyboard_playing.v=0;


        char *filtros[2];

        filtros[0]="txt";
        filtros[1]=0;


        if (menu_filesel("Select Spool File",filtros,input_file_keyboard_name_buffer)==1) {
		input_file_keyboard_name=input_file_keyboard_name_buffer;

        }

        else {
		input_file_keyboard_name=NULL;
		eject_input_file_keyboard();
        }
}


void menu_input_file_keyboard_send_pause(MENU_ITEM_PARAMETERS)
{
	input_file_keyboard_send_pause.v ^=1;
}


void menu_input_file_keyboard_turbo(MENU_ITEM_PARAMETERS)
{

	if (input_file_keyboard_turbo.v) {
		reset_peek_byte_function_spoolturbo();
		input_file_keyboard_turbo.v=0;
	}

	else {
		set_peek_byte_function_spoolturbo();
		input_file_keyboard_turbo.v=1;
	}
}


int menu_input_file_keyboard_turbo_cond(void)
{
	if (MACHINE_IS_SPECTRUM) return 1;
	return 0;
}


int get_efectivo_tamanyo_find_buffer(void)
{
	if (MACHINE_IS_QL) return QL_MEM_LIMIT+1;
	return 65536;
}

//int menu_find_bytes_total_items=0;


//Indica si esta vacio o no; esto se usa para saber si la busqueda se hace sobre la busqueda anterior o no
int menu_find_bytes_empty=1;

unsigned char *menu_find_bytes_mem_pointer=NULL;
/* Estructura del array de busqueda:
65536 items del array. Cada item es un unsigned char
El valor indica:
0: en esa posicion de la memoria no se encuentra el byte
1: en esa posicion de la memoria si se encuentra el byte
otros valores de momento no tienen significado
*/


void menu_find_bytes_clear_results_process(void)
{
        //inicializamos array
        int i;

        for (i=0;i<get_efectivo_tamanyo_find_buffer();i++) menu_find_bytes_mem_pointer[i]=0;

        menu_find_bytes_empty=1;

}

void menu_find_bytes_clear_results(MENU_ITEM_PARAMETERS)
{
        menu_find_bytes_clear_results_process();

        menu_generic_message("Clear Results","OK. Results cleared");
}

void menu_find_bytes_view_results(MENU_ITEM_PARAMETERS)
{

        int index_find,index_buffer;

        char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

        //margen suficiente para que quepa una linea
        //direccion+salto linea+codigo 0
        char buf_linea[9];

        index_buffer=0;

        int encontrados=0;

        int salir=0;

        for (index_find=0;index_find<get_efectivo_tamanyo_find_buffer() && salir==0;index_find++) {
                if (menu_find_bytes_mem_pointer[index_find]) {
                        sprintf (buf_linea,"%d\n",index_find);
                        sprintf (&results_buffer[index_buffer],"%s\n",buf_linea);
                        index_buffer +=strlen(buf_linea);
                        encontrados++;
                }

                //controlar maximo
                //20 bytes de margen
                if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-20) {
                        debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first %d",encontrados);
                        //forzar salir
                        salir=1;
                }

        }

        results_buffer[index_buffer]=0;

        menu_generic_message("View Results",results_buffer);
}

//Busqueda desde direccion indicada
int menu_find_bytes_process_from(z80_byte byte_to_find,int inicio)
{
	int dir;
	int total_items_found=0;
	int final_find=get_efectivo_tamanyo_find_buffer();

	//Busqueda con array no inicializado
	if (menu_find_bytes_empty) {

					debug_printf (VERBOSE_INFO,"Starting Search with no previous results");


					//asumimos que no va a encontrar nada
					menu_find_bytes_empty=1;

					for (dir=inicio;dir<final_find;dir++) {
									if (peek_byte_z80_moto(dir)==byte_to_find) {
													menu_find_bytes_mem_pointer[dir]=1;

													//al menos hay un resultado
													menu_find_bytes_empty=0;

													//incrementamos contador de resultados para mostrar al final
													total_items_found++;
									}
					}

	}

	else {
					//Busqueda con array ya con contenido
					//examinar solo las direcciones que indique el array

					debug_printf (VERBOSE_INFO,"Starting Search using previous results");

					//asumimos que no va a encontrar nada
					menu_find_bytes_empty=1;

					int i;
					for (i=0;i<final_find;i++) {
									if (menu_find_bytes_mem_pointer[i]) {
													//Ver el contenido de esa direccion
													if (peek_byte_z80_moto(i)==byte_to_find) {
																	//al menos hay un resultado
																	menu_find_bytes_empty=0;
																	//incrementamos contador de resultados para mostrar al final
																	total_items_found++;
													}
													else {
																	//el byte ya no esta en esa direccion
																	menu_find_bytes_mem_pointer[i]=0;
													}
									}
					}
	}

	return total_items_found;

}

//Busqueda desde direccion 0
int menu_find_bytes_process(z80_byte byte_to_find)
{
	return menu_find_bytes_process_from(byte_to_find,0);
}

void menu_find_bytes_find(MENU_ITEM_PARAMETERS)
{

        //Buscar en la memoria direccionable (0...65535) si se encuentra el byte
        z80_byte byte_to_find;


        char string_find[4];

        sprintf (string_find,"0");

        menu_ventana_scanf("Find byte",string_find,4);

        int valor_find=parse_string_to_number(string_find);

        if (valor_find<0 || valor_find>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",valor_find);
                return;
        }


        byte_to_find=valor_find;


        int total_items_found;

				total_items_found=menu_find_bytes_process(byte_to_find);

        menu_generic_message_format("Find","Total addresses found: %d",total_items_found);


}



//int total_tamanyo_find_buffer=0;

void menu_find_bytes_alloc_if_needed(void)
{
	//Si puntero memoria no esta asignado, asignarlo, o si hemos cambiado de tipo de maquina
	if (menu_find_bytes_mem_pointer==NULL) {

					//65536 elementos del array, cada uno de tamanyo unsigned char
					//total_tamanyo_find_buffer=get_total_tamanyo_find_buffer();

					menu_find_bytes_mem_pointer=malloc(QL_MEM_LIMIT+1); //Asignamos el maximo (maquina QL)
					if (menu_find_bytes_mem_pointer==NULL) cpu_panic("Error allocating find array");

					//inicializamos array
					int i;
					for (i=0;i<get_efectivo_tamanyo_find_buffer();i++) menu_find_bytes_mem_pointer[i]=0;
	}
}

void menu_find_bytes(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_find_bytes;
        menu_item item_seleccionado;
        int retorno_menu;

        //Si puntero memoria no esta asignado, asignarlo, o si hemos cambiado de tipo de maquina
        menu_find_bytes_alloc_if_needed();


        do {

                menu_add_item_menu_inicial_format(&array_menu_find_bytes,MENU_OPCION_NORMAL,menu_find_bytes_find,NULL,"Find byte");
                menu_add_item_menu_tooltip(array_menu_find_bytes,"Find one byte on memory");
                menu_add_item_menu_ayuda(array_menu_find_bytes,"Find one byte on the 64 KB of mapped memory, considering the last address found (if any).\n"
                        "It can be used to find POKEs, it's very easy: \n"
                        "I first recommend to disable Multitasking menu, to avoid losing lives where in the menu.\n"
                        "As an example, you are in a game with 4 lives. Enter find byte "
                        "with value 4. It will find a lot of addresses, don't panic.\n"
                        "Then, you must lose one live, you have now 3. Then you must search for byte with value 3; "
                        "the search will be made considering the last search results. Normally here you will find only "
                        "one address with value 3. At this moment you know the address where the lives are stored.\n"
                        "The following to find, for example, infinite lives, is to search where this life value is decremented. "
                        "You can find it by making a MRA breakpoint to the address where the lives are stored, or a condition breakpoint "
			"(NN)=value, setting NN to the address where lives are stored, and value to the desired number of lives, for example: (51308)=2.\n"
                        "When the breakpoint is caught, you will probably have the section of code where the lives are decremented ;) ");

                menu_add_item_menu_format(array_menu_find_bytes,MENU_OPCION_NORMAL,menu_find_bytes_view_results,NULL,"View results");
                menu_add_item_menu_tooltip(array_menu_find_bytes,"View results");
                menu_add_item_menu_ayuda(array_menu_find_bytes,"View results");

                menu_add_item_menu_format(array_menu_find_bytes,MENU_OPCION_NORMAL,menu_find_bytes_clear_results,NULL,"Clear results");
                menu_add_item_menu_tooltip(array_menu_find_bytes,"Clear results");
                menu_add_item_menu_ayuda(array_menu_find_bytes,"Clear results");




                menu_add_item_menu(array_menu_find_bytes,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                menu_add_ESC_item(array_menu_find_bytes);

                retorno_menu=menu_dibuja_menu(&find_bytes_opcion_seleccionada,&item_seleccionado,array_menu_find_bytes,"Find bytes" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);
}



void menu_input_file_keyboard_play(MENU_ITEM_PARAMETERS)
{
	input_file_keyboard_playing.v ^=1;
}




void menu_debug_input_file_keyboard(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_input_file_keyboard;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                char string_input_file_keyboard_shown[16];
                menu_tape_settings_trunc_name(input_file_keyboard_name,string_input_file_keyboard_shown,16);
                menu_add_item_menu_inicial_format(&array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard,NULL,"Spool file [%s]",string_input_file_keyboard_shown);


                menu_add_item_menu_format(array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard_insert,menu_input_file_keyboard_cond,"[%c] Spool file inserted",(input_file_keyboard_inserted.v ? 'X' : ' ' ));
                if (input_file_keyboard_inserted.v) {

					menu_add_item_menu_format(array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard_play,NULL,"[%c] Spool file playing",(input_file_keyboard_playing.v ? 'X' : ' ' ));	

			//en tbblue no va bien la opcion de turbo
			if (!MACHINE_IS_TBBLUE) {
				menu_add_item_menu_format(array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard_turbo,menu_input_file_keyboard_turbo_cond,"[%c] Turbo mode",(input_file_keyboard_turbo.v ? 'X' : ' ') );
				menu_add_item_menu_tooltip(array_menu_input_file_keyboard,"Allow turbo mode on Spectrum models");
				menu_add_item_menu_ayuda(array_menu_input_file_keyboard,"Allow turbo mode on Spectrum models. It traps calls to ROM when keyboard is read.\n"
									"Works well with Spectrum Basic but also with Text Adventures made with Daad, Paws and GAC");
			}


			if (input_file_keyboard_turbo.v==0) {

	                        menu_add_item_menu_format(array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard_delay,NULL,"[%d ms] Key length",input_file_keyboard_delay*1000/50);
        	                menu_add_item_menu_tooltip(array_menu_input_file_keyboard,"Length of every key pressed");
                	        menu_add_item_menu_ayuda(array_menu_input_file_keyboard,"I recommend 100 ms for entering lines on Spectrum BASIC. I also suggest to send some manual delays, using unhandled character, like \\, to assure entering lines is correct ");

	                        menu_add_item_menu_format(array_menu_input_file_keyboard,MENU_OPCION_NORMAL,menu_input_file_keyboard_send_pause,NULL,"[%c] Delay after every key",(input_file_keyboard_send_pause.v==1 ? 'X' : ' ') );
        	                menu_add_item_menu_tooltip(array_menu_input_file_keyboard,"Send or not a delay of the same duration after every key");
                	        menu_add_item_menu_ayuda(array_menu_input_file_keyboard,"I recommend enabling this for entering lines on Spectrum BASIC");


			}
                }


                menu_add_item_menu(array_menu_input_file_keyboard,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_input_file_keyboard,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_input_file_keyboard);

                retorno_menu=menu_dibuja_menu(&input_file_keyboard_opcion_seleccionada,&item_seleccionado,array_menu_input_file_keyboard,"Input File Spooling" );
		

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}




void menu_debug_view_basic(MENU_ITEM_PARAMETERS)
{

	char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];
	debug_view_basic(results_buffer);

  menu_generic_message_format("View Basic","%s",results_buffer);
}

//Devuelve 1 si caracter es imprimible en pantalla (entre 32 y 126 o caracteres 10, 13, y algun otro)
int menu_file_viewer_read_text_file_char_print(z80_byte caracter)
{
	if (caracter>=32 && caracter<=126) return 1;

	if (caracter>=9 && caracter<=13) return 1; //9=horiz tab, 10=LF, 11=vert tab, 12=new page, 13=CR

	return 0;
	
}

void menu_file_viewer_sped_show(char *file_read_memory,int longitud)
{


        int index_buffer;

        char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

        index_buffer=0;

        int salir=0;

		int x=0;

		int lineas=0;

		while (!salir && longitud) {
			z80_byte caracter=*file_read_memory;
			file_read_memory++;

			if (caracter==13) {
				caracter=10;
				lineas++;
			}

			else if (caracter==127) {
				//(C)
				caracter='c';
			}

			else if (caracter>=128) {
				caracter -=128;	
				int tabcolumn;
				if (x<7) tabcolumn=7;
				else tabcolumn=12;

				while (x<tabcolumn) {
					results_buffer[index_buffer++]=' ';
					x++;
				}

			}

			results_buffer[index_buffer++]=caracter;
			//controlar maximo
            //100 bytes de margen
            if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-100 || lineas>=MAX_LINEAS_TOTAL_GENERIC_MESSAGE) {
            	debug_printf (VERBOSE_ERR,"Too many lines to show. Showing only the first %d",lineas);
                //forzar salir
                salir=1;
            }

            x++;
            if (caracter==10) x=0;

			longitud--;
		}

        results_buffer[index_buffer]=0;

        menu_generic_message("SPED file",results_buffer);

}

void menu_file_viewer_read_text_file(char *title,char *file_name)
{

	char file_read_memory[MAX_TEXTO_GENERIC_MESSAGE];



	debug_printf (VERBOSE_INFO,"Loading %s File",file_name);
	FILE *ptr_file_name;
	ptr_file_name=fopen(file_name,"rb");


	if (!ptr_file_name)
	{
		debug_printf (VERBOSE_ERR,"Unable to open %s file",file_name);
		return;
	}
	

	int leidos=fread(file_read_memory,1,MAX_TEXTO_GENERIC_MESSAGE,ptr_file_name);
	debug_printf (VERBOSE_INFO,"Read %d bytes of file: %s",leidos,file_name);
	int avisolimite=0;

	if (leidos==MAX_TEXTO_GENERIC_MESSAGE) {
		avisolimite=1;
		leidos--;
	}

	file_read_memory[leidos]=0;

	fclose(ptr_file_name);


	//Si longitud de bloque es 17, y byte inicial es 0,1,2 o 3, visor de cabecera de bloque de spectrum
	z80_byte byte_inicial=file_read_memory[0];
	if (leidos==17 && byte_inicial<4) {
		debug_printf(VERBOSE_DEBUG,"File 17 bytes length and first byte is <4: assume Spectrum tape header");

		//int longitud_bloque;
		char buffer_texto[40];

        	z80_byte flag=0;
		z80_int longitud=19;

                util_tape_get_info_tapeblock((z80_byte *)file_read_memory,flag,longitud,buffer_texto);


		//util_tape_tap_get_info((z80_byte *)file_read_memory,buffer_texto);
		//menu_generic_message_tooltip("Tape browser", 0, 0, 1, NULL, "%s", buffer_texto);
		zxvision_generic_message_tooltip("Tape browser" , 0 , 0, 0, 1, NULL, 1, "%s", buffer_texto);

		
		return;
	}


	//Ahora deducir si el archivo cargado es texto o binario.
	//Codigos mayores de 127 o menores de 32 (sin ser el 10, 13 y algun otro) hacen disparar el aviso. Cuantos tiene que haber? Por porcentaje del archivo o por numero?
	//Mejor por porcentaje. Cualquier archivo con un 10% minimo de codigos no imprimibles, se considerara binario
	int i;
	int codigos_no_imprimibles=0;
	z80_byte caracter;

	/* Detector archivos sped:
	-que hayan códigos 13 de salto de línea
	-que no hayan otros códigos menores que 32 excepto los saltos de línea
	-que haya ascii
	-que haya códigos mayores que 128
	*/

	int sped_cr=0;
	int sped_below_32=0;
	int sped_ascii=0;
	int sped_beyond_128=0;

	for (i=0;i<leidos;i++) {
		caracter=file_read_memory[i];
		//if (caracter>127) codigos_no_imprimibles++;
		if (!menu_file_viewer_read_text_file_char_print(caracter)) codigos_no_imprimibles++;

		//Deteccion sped
		if (caracter<32) {
			if (caracter==13) sped_cr=1;
			else sped_below_32=1;
		}

		if (caracter>=32 && caracter<=127) sped_ascii=1;
		if (caracter>127) sped_beyond_128=1;
		
	}

	//Deteccion sped
	if (sped_cr && !sped_below_32 && sped_ascii && sped_beyond_128) {
		debug_printf(VERBOSE_INFO,"File possibly is a sped file");
		menu_file_viewer_sped_show(file_read_memory,leidos);
		return;

	}

	//Sacar porcentaje 10%
	int umbral=leidos/10;

	if (codigos_no_imprimibles>umbral) {
		debug_printf(VERBOSE_INFO,"Considering file as hexadecimal because the invalid characters are higher than 10%% of the total size (%d/%d)",
			codigos_no_imprimibles,leidos);
		menu_file_hexdump_browser_show(file_name);
	}

	else {
		debug_printf(VERBOSE_INFO,"Considering file as text because the invalid characters are lower than 10%% of the total size (%d/%d)",
			codigos_no_imprimibles,leidos);

		if (avisolimite) debug_printf (VERBOSE_ERR,"Read max text buffer: %d bytes. Showing only these",leidos);

		menu_generic_message(title,file_read_memory);
	}

	

}

void menu_file_viewer_read_file(char *title,char *file_name)
{

	//No mostrar caracteres especiales
	menu_disable_special_chars.v=1;

	//printf ("extension vacia: %d\n",util_compare_file_extension(file_name,"") );
	//printf ("es z88 basic: %d\n",file_is_z88_basic(file_name));

	//Algunos tipos conocidos
	if (!util_compare_file_extension(file_name,"tap")) menu_tape_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"zx")) menu_file_zx_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"sp")) menu_file_sp_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"z80")) menu_file_z80_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"sna")) menu_file_sna_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"spg")) menu_file_spg_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"bas")) menu_file_basic_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"b")) menu_file_basic_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"baszx80")) menu_file_basic_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"baszx81")) menu_file_basic_browser_show(file_name);

	//Aunque esa extension no la usa nadie pero es una manera de forzar que se pueda mostrar un archivo de tokens
	//z88 en caso que la deteccion automatica (que se hace aqui mas abajo) falle
	else if (!util_compare_file_extension(file_name,"basz88")) menu_file_basic_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"p")) menu_file_p_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"o")) menu_file_o_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"mmc")) menu_file_mmc_browser_show(file_name,"MMC");

	//Suponemos que un mmcide (que sale de un hdf) es un mmc
	else if (!util_compare_file_extension(file_name,"mmcide")) menu_file_mmc_browser_show(file_name,"MMC");

	else if (!util_compare_file_extension(file_name,"ide")) menu_file_mmc_browser_show(file_name,"IDE");

	else if (!util_compare_file_extension(file_name,"trd")) menu_file_trd_browser_show(file_name,"TRD");

	else if (!util_compare_file_extension(file_name,"dsk")) menu_file_dsk_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"tzx")) menu_file_tzx_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"pzx")) menu_file_pzx_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"cdt")) menu_file_tzx_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"flash")) menu_file_flash_browser_show(file_name);

	else if (!util_compare_file_extension(file_name,"epr")) menu_z88_new_ptr_card_browser(file_name);

	else if (!util_compare_file_extension(file_name,"eprom")) menu_z88_new_ptr_card_browser(file_name);

	else if (!util_compare_file_extension(file_name,"zsf")) menu_file_zsf_browser_show(file_name);

	//Si archivo no tiene extension pero su contenido parece indicar que es z88 basic
	else if (!util_compare_file_extension(file_name,"") && file_is_z88_basic(file_name)) menu_file_basic_browser_show(file_name);


	//Por defecto, texto
	else menu_file_viewer_read_text_file(title,file_name);


	//IMPORTANTE: siempre se debe salir de la funcion desde aqui abajo, para que se vuelva a mostrar los caracteres especiales
	//Volver a mostrar caracteres especiales
	menu_disable_special_chars.v=0;
}


void menu_debug_file_utils(MENU_ITEM_PARAMETERS)
{


	int ret=1;

	while (ret!=0) {
			char *filtros[2];

			filtros[0]="";
			filtros[1]=0;


        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);


        //Obtenemos ultimo directorio visitado
        if (file_utils_file_name[0]!=0) {
                char directorio[PATH_MAX];
                util_get_dir(file_utils_file_name,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

        menu_filesel_show_utils.v=1;
        ret=menu_filesel("File utilities",filtros,file_utils_file_name);
        menu_filesel_show_utils.v=0;

        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);

        if (ret==1) {

						cls_menu_overlay();
						//menu_file_utils_read_file("File view",file_utils_file_name);
				}

    }



}


/*
Busqueda de contador de vidas.
Estados: 0. Inicial. Se indica vidas actuales. Se realiza busqueda en toda memoria con ese valor
Estados: 1. Ya se ha indicado vidas iniciales. Se puede indicar vidas actuales. Se realiza busqueda en toda memoria. Si se encuentra 1 solo valor, se pasa a estado 2.
Se puede pasar a estado 0 con "Restart"

Estados: 2. Ya se ha encontrado contador vidas. Se muestra direccion. Se puede pasar a estado 0 con "Restart"

*/

int menu_find_lives_state=0;

//Puntero a memoria de spectrum que dice donde esta el contador de vidas
z80_int menu_find_lives_pointer=0;

//Ultimo valor buscado
int lives_to_find=3;


void menu_find_lives_restart(MENU_ITEM_PARAMETERS)
{
	menu_find_lives_state=0;
}

void menu_find_lives_initial(MENU_ITEM_PARAMETERS)
{
	//Limpiar resultados
	menu_find_bytes_alloc_if_needed();
	if (menu_find_lives_state==0) menu_find_bytes_clear_results_process();

	//Suponemos que la segunda vez habra perdido al menos 1 vida
	if (menu_find_lives_state==1 && lives_to_find>0) lives_to_find--;

	//Pedir vidas actuales
	//Buscar en la memoria direccionable (0...65535) si se encuentra el byte


	char string_find[4];

	sprintf (string_find,"%d",lives_to_find);

	menu_ventana_scanf("Current lives",string_find,4);

	int valor_find=parse_string_to_number(string_find);

	if (valor_find<0 || valor_find>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_find);
					return;
	}


	lives_to_find=valor_find;


	int total_items_found;

	//Empezar a buscar desde la 16384. No tiene sentido buscar desde la rom
	total_items_found=menu_find_bytes_process_from(lives_to_find,16384);

	//menu_generic_message_format("Find","Total addresses found: %d",total_items_found);

	//Si estamos en estado 0
	if (menu_find_lives_state==0) {
		if (total_items_found==0) {
			 menu_generic_message("Find lives","Sorry, no lives counter found");
		}
		else {
			menu_generic_message("Find lives","Ok. Continue playing game and come back when you lose a life");
			menu_find_lives_state=1;
		}
	}

	//Si estamos en estado 1
	else if (menu_find_lives_state==1) {
		if (total_items_found==0) {
			menu_generic_message("Find lives","Sorry, I haven't found any addresses. The process has been restarted");
			menu_find_lives_state=0;
		}
		else if (total_items_found!=1) {
			 menu_generic_message("Find lives","Sorry, no unique address found. You may want to try again losing another live or maybe manually find it on the Find bytes menu");
		}
		else {

			//Buscar la direccion
			int index_find;

			int salir=0;

			for (index_find=0;index_find<get_efectivo_tamanyo_find_buffer() && salir==0;index_find++) {
							if (menu_find_bytes_mem_pointer[index_find]) {
											menu_find_lives_pointer=index_find;
											salir=0;
							}
			}


			menu_find_lives_state=2;

			menu_generic_message("Find lives","Great. Memory pointer found");
		}
	}



	//Buscar bytes

}


//Ultimo valor de vidas seleccionadas
z80_byte ultimo_menu_find_lives_set=9;

void menu_find_lives_set(MENU_ITEM_PARAMETERS)
{


	char string_lives[4];

	sprintf (string_lives,"%d",ultimo_menu_find_lives_set);

	menu_ventana_scanf("Lives?",string_lives,4);

	int valor_lives=parse_string_to_number(string_lives);

	if (valor_lives<0 || valor_lives>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_lives);
					return;
	}

	ultimo_menu_find_lives_set=valor_lives;

	poke_byte_z80_moto(menu_find_lives_pointer,valor_lives);

}

void menu_find_lives(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_find_lives;
        menu_item item_seleccionado;
        int retorno_menu;




        do {

								if (menu_find_lives_state==0) {
                	menu_add_item_menu_inicial_format(&array_menu_find_lives,MENU_OPCION_NORMAL,menu_find_lives_initial,NULL,"Tell current lives (initial)");
								}

								if (menu_find_lives_state==1) {
									menu_add_item_menu_inicial_format(&array_menu_find_lives,MENU_OPCION_NORMAL,menu_find_lives_initial,NULL,"Tell current lives (decr.)");
								}

								if (menu_find_lives_state==2) {
									menu_add_item_menu_inicial_format(&array_menu_find_lives,MENU_OPCION_NORMAL,NULL,NULL,"Lives pointer: %d",menu_find_lives_pointer);
									menu_add_item_menu_format(array_menu_find_lives,MENU_OPCION_NORMAL,NULL,NULL,"Lives: %d",peek_byte_z80_moto(menu_find_lives_pointer) );
									menu_add_item_menu_format(array_menu_find_lives,MENU_OPCION_NORMAL,menu_find_lives_set,NULL,"Set lives");
								}

								if (menu_find_lives_state==1 || menu_find_lives_state==2) {
									menu_add_item_menu_format(array_menu_find_lives,MENU_OPCION_NORMAL,menu_find_lives_restart,NULL,"Restart process");
								}

                menu_add_item_menu(array_menu_find_lives,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                menu_add_ESC_item(array_menu_find_lives);

                retorno_menu=menu_dibuja_menu(&find_lives_opcion_seleccionada,&item_seleccionado,array_menu_find_lives,"Find lives" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);
}


void menu_find(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_find;
        menu_item item_seleccionado;
        int retorno_menu;




        do {

                menu_add_item_menu_inicial_format(&array_menu_find,MENU_OPCION_NORMAL,menu_find_bytes,NULL,"Find byte");
                menu_add_item_menu_tooltip(array_menu_find,"Find one byte on memory");
                menu_add_item_menu_ayuda(array_menu_find,"Find one byte on the 64 KB of mapped memory, considering the last address found (if any)");

								menu_add_item_menu_format(array_menu_find,MENU_OPCION_NORMAL,menu_find_lives,NULL,"Find lives address");
                menu_add_item_menu_tooltip(array_menu_find,"Find memory pointer where lives are located");
                menu_add_item_menu_ayuda(array_menu_find,"Find memory pointer where lives are located)");


                menu_add_item_menu(array_menu_find,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                menu_add_ESC_item(array_menu_find);

                retorno_menu=menu_dibuja_menu(&find_opcion_seleccionada,&item_seleccionado,array_menu_find,"Find" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);
}

int menu_debug_view_basic_cond(void)
{
	if (MACHINE_IS_Z88) return 0;
	if (MACHINE_IS_ACE) return 0;
	if (MACHINE_IS_CPC) return 0;
	if (MACHINE_IS_SAM) return 0;
	if (MACHINE_IS_QL) return 0;
	if (MACHINE_IS_MK14) return 0;
	return 1;
}

void menu_debug_spritefinder(MENU_ITEM_PARAMETERS)
{
	if (spritefinder_enabled.v) spritefinder_disable();
	else spritefinder_enable();
}



void menu_debug_tsconf_tbblue(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_debug_tsconf_tbblue;
        menu_item item_seleccionado;
	int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_debug_tsconf_tbblue,MENU_OPCION_NORMAL,menu_debug_tsconf_tbblue_videoregisters,NULL,"Video ~~Info");
		menu_add_item_menu_shortcut(array_menu_debug_tsconf_tbblue,'i');

		menu_add_item_menu_format(array_menu_debug_tsconf_tbblue,MENU_OPCION_NORMAL,menu_tsconf_layer_settings,NULL,"Video ~~Layers");
		menu_add_item_menu_shortcut(array_menu_debug_tsconf_tbblue,'l');

		menu_add_item_menu_format(array_menu_debug_tsconf_tbblue,MENU_OPCION_NORMAL,menu_debug_tsconf_tbblue_spritenav,NULL,"~~Sprite navigator");
		menu_add_item_menu_shortcut(array_menu_debug_tsconf_tbblue,'s');

		if (MACHINE_IS_TSCONF || MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_debug_tsconf_tbblue,MENU_OPCION_NORMAL,menu_debug_tsconf_tbblue_tilenav,NULL,"~~Tile navigator");
			menu_add_item_menu_shortcut(array_menu_debug_tsconf_tbblue,'t');
		}

                menu_add_item_menu(array_menu_debug_tsconf_tbblue,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_debug_tsconf_tbblue,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_debug_tsconf_tbblue);

		char titulo_ventana[33];

		//por defecto
		strcpy(titulo_ventana,"Debug TSConf");

		if (MACHINE_IS_TBBLUE) strcpy(titulo_ventana,"Debug TBBlue");

                retorno_menu=menu_dibuja_menu(&debug_tsconf_opcion_seleccionada,&item_seleccionado,array_menu_debug_tsconf_tbblue,titulo_ventana);

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_unpaws_ungac(MENU_ITEM_PARAMETERS)
{

	char mensaje[1024];

	int retorno=util_unpawsetc_dump_words(mensaje);

	if (retorno>=0) {
		menu_generic_message_format("Extract Words",mensaje);
	}

	else {
		debug_printf (VERBOSE_ERR,mensaje);
	}


}

void menu_write_message(MENU_ITEM_PARAMETERS)
{
	char buffer_texto[256];

	buffer_texto[0]=0;
    menu_ventana_scanf("Just write...",buffer_texto,256);

	menu_generic_message("Your message",buffer_texto);


}



//menu debug settings
void menu_debug_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_debug_settings;
        menu_item item_seleccionado;
	int retorno_menu;
        do {
                menu_add_item_menu_inicial(&array_menu_debug_settings,"~~Reset",MENU_OPCION_NORMAL,menu_debug_reset,NULL);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'r');

		if (MACHINE_IS_PRISM) {
			//Reset to failsafe
			menu_add_item_menu(array_menu_debug_settings,"Reset to Failsafe mode",MENU_OPCION_NORMAL,menu_debug_prism_failsafe,NULL);

			//Para Testeo. usr0
			//menu_add_item_menu(array_menu_debug_settings,"Set PC=0",MENU_OPCION_NORMAL,menu_debug_prism_usr0,NULL);

		}

		if (MACHINE_IS_Z88 || MACHINE_IS_ZXUNO || MACHINE_IS_PRISM || MACHINE_IS_TBBLUE || superupgrade_enabled.v || MACHINE_IS_TSCONF || MACHINE_IS_BASECONF) {
	                menu_add_item_menu(array_menu_debug_settings,"~~Hard Reset",MENU_OPCION_NORMAL,menu_debug_hard_reset,NULL);
			menu_add_item_menu_shortcut(array_menu_debug_settings,'h');
	                menu_add_item_menu_tooltip(array_menu_debug_settings,"Hard resets the machine");
	                menu_add_item_menu_ayuda(array_menu_debug_settings,"Hard resets the machine.\n"
				"On Z88, it's the same as opening flap and pressing reset button.\n"
				"On ZX-Uno, it's the same as pressing Ctrl-Alt-Backspace or powering off and on the machine"
				);
		}

		if (!CPU_IS_MOTOROLA) {
    		menu_add_item_menu(array_menu_debug_settings,"Generate ~~NMI",MENU_OPCION_NORMAL,menu_debug_nmi,NULL);
			menu_add_item_menu_shortcut(array_menu_debug_settings,'n');

			if (MACHINE_IS_TBBLUE && multiface_enabled.v && (tbblue_registers[6]&8) ) menu_add_item_menu(array_menu_debug_settings,"Generate Multiface NMI",MENU_OPCION_NORMAL,menu_debug_nmi_multiface_tbblue,NULL);
		}

		if (MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
	                menu_add_item_menu(array_menu_debug_settings,"Generate Special NMI",MENU_OPCION_NORMAL,menu_debug_special_nmi,NULL);
		}

		menu_add_item_menu(array_menu_debug_settings,"~~Debug CPU",MENU_OPCION_NORMAL,menu_debug_registers,NULL);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'d');
		menu_add_item_menu_tooltip(array_menu_debug_settings,"Open debug window");
		menu_add_item_menu_ayuda(array_menu_debug_settings,"This window opens the debugger. You can see there some Z80 registers "
					"easily recognizable. Some other variables and entries need further explanation:\n"
					"TSTATES: T-states total in a frame\n"
					"TSTATL: T-states total in a scanline\n"
					"TSTATP: T-states partial. This is a counter that you can reset with key P"



					);

		if (MACHINE_IS_TSCONF || MACHINE_IS_ZXUNO || datagear_dma_emulation.v) {
			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_dma_tsconf_zxuno,NULL,"Debug D~~MA");
			menu_add_item_menu_shortcut(array_menu_debug_settings,'m');
		}					

		if (CPU_IS_Z80) {
			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_ioports,NULL,"Debug ~~I/O Ports");
			menu_add_item_menu_shortcut(array_menu_debug_settings,'i');

			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_cpu_transaction_log,NULL,"~~CPU Transaction Log");
			menu_add_item_menu_shortcut(array_menu_debug_settings,'c');
		}

		if (MACHINE_IS_TSCONF || MACHINE_IS_TBBLUE) {
			if (MACHINE_IS_TSCONF) menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_tsconf_tbblue,NULL,"~~TSConf");
			if (MACHINE_IS_TBBLUE) menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_tsconf_tbblue,NULL,"~~TBBlue");
			menu_add_item_menu_shortcut(array_menu_debug_settings,'t');
		}



		menu_add_item_menu(array_menu_debug_settings,"He~~xadecimal Editor",MENU_OPCION_NORMAL,menu_debug_hexdump,NULL);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'x');

		menu_add_item_menu(array_menu_debug_settings,"View ~~Basic",MENU_OPCION_NORMAL,menu_debug_view_basic,menu_debug_view_basic_cond);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'b');

		if (si_complete_video_driver() ) {
			menu_add_item_menu(array_menu_debug_settings,"View ~~Sprites",MENU_OPCION_NORMAL,menu_debug_view_sprites,NULL);
			menu_add_item_menu_shortcut(array_menu_debug_settings,'s');
		}

#ifdef EMULATE_CPU_STATS
		if (CPU_IS_Z80) {
			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_cpu_stats,NULL,"View CPU Statistics");
		}

#endif

#ifdef EMULATE_VISUALMEM
			//if (!CPU_IS_MOTOROLA) {
			//menu_add_item_menu(array_menu_debug_settings,"Old visual memory",MENU_OPCION_NORMAL,menu_debug_visualmem,NULL);
	                //menu_add_item_menu_tooltip(array_menu_debug_settings,"Show which memory zones are changed or which memory address with opcodes have been executed");
	                //menu_add_item_menu_ayuda(array_menu_debug_settings,"Show which memory zones are changed or which memory address with opcodes have been executed");

			menu_add_item_menu(array_menu_debug_settings,"~~Visual memory",MENU_OPCION_NORMAL,menu_debug_new_visualmem,NULL);
			menu_add_item_menu_shortcut(array_menu_debug_settings,'v');
	                menu_add_item_menu_tooltip(array_menu_debug_settings,"Show which memory zones are changed or which memory address with opcodes have been executed");
	                menu_add_item_menu_ayuda(array_menu_debug_settings,"Show which memory zones are changed or which memory address with opcodes have been executed");			
			//}
#endif

    menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_find,NULL,"~~Find");
		menu_add_item_menu_shortcut(array_menu_debug_settings,'f');
    menu_add_item_menu_tooltip(array_menu_debug_settings,"Find bytes on memory");
    menu_add_item_menu_ayuda(array_menu_debug_settings,"Find bytes on the 64 KB of mapped memory");


		menu_add_item_menu(array_menu_debug_settings,"~~Poke",MENU_OPCION_NORMAL,menu_poke,NULL);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'p');
		menu_add_item_menu_tooltip(array_menu_debug_settings,"Poke address manually or from .POK file");
		menu_add_item_menu_ayuda(array_menu_debug_settings,"Poke address for infinite lives, etc...");



		if (menu_cond_zx8081() ) {

			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_lost_vsync,NULL,
				"Simulate lost VSYNC: %s",(simulate_lost_vsync.v==1 ? "On" : "Off"));
		}




		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_load_binary,NULL,"L~~oad binary block");
		menu_add_item_menu_shortcut(array_menu_debug_settings,'o');

		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_save_binary,NULL,"S~~ave binary block");
		menu_add_item_menu_shortcut(array_menu_debug_settings,'a');


		if (menu_desactivado_file_utilities.v==0) {

			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_file_utils,NULL,"File ~~utilities");
			menu_add_item_menu_shortcut(array_menu_debug_settings,'u');
			menu_add_item_menu_tooltip(array_menu_debug_settings,"Some file utilities. NOTE: Shortcuts must be chosen pressing Shift+Key");
			menu_add_item_menu_ayuda(array_menu_debug_settings,"Some file utilities.\nNOTE: Shortcuts in file utilities must be chosen by pressing Shift+Key, "
								"I mean, shortcuts are in capital letters to differentiate from quick selecting a file, so for example, "
								"to view a file you must press Shift+v");

		}
		


		if (!CPU_IS_MOTOROLA) {
		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_input_file_keyboard,NULL,"Input File Spoolin~~g");
		menu_add_item_menu_shortcut(array_menu_debug_settings,'g');
                menu_add_item_menu_tooltip(array_menu_debug_settings,"Sends every character from a text file as keyboard presses");
                menu_add_item_menu_ayuda(array_menu_debug_settings,"Every character from a text file is sent as keyboard presses. Only Ascii characters, not UFT, Unicode or others. "
                                                                   "Symbols that require extended mode on Spectrum are not sent: [ ] (c) ~ \\ { }. These can be used "
                                                                   "as a delay.\n"
								"Note: symbol | means Shift+1 (Edit)");
		}


		//menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_zxvision_test,NULL,"ZXVision test");
		if (menu_allow_background_windows) {
			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_draw_background_windows,NULL,"ZXVision background windows");
		}


		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_write_message,NULL,"Write message");
		menu_add_item_menu_tooltip(array_menu_debug_settings,"Just lets you write text in a window, useful if you want to record the display and you want to say something");
		menu_add_item_menu_ayuda(array_menu_debug_settings,"Just lets you write text in a window, useful if you want to record the display and you want to say something");

	/*	menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_registers_console,NULL,"Show r~~egisters in console: %s",(debug_registers==1 ? "On" : "Off"));
		menu_add_item_menu_shortcut(array_menu_debug_settings,'e');

		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_verbose,NULL,"Verbose ~~level: %d",verbose_level);
		menu_add_item_menu_shortcut(array_menu_debug_settings,'l');*/


	if (MACHINE_IS_SPECTRUM_16_48) {
		menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_run_mantransfer,NULL,"Run mantransfer");
		menu_add_item_menu_tooltip(array_menu_debug_settings,"Run mantransfer, which dumps ram memory contents (snapshot) to Spectrum Tape\n"
					"Only Spectrum 48k/16k models supported");
		menu_add_item_menu_ayuda(array_menu_debug_settings,"The difference between this option and the Save snapshot option is that "
					"this option runs a Spectrum machine program (mantransfev3.bin) which dumps the ram contents to tape, "
					"so you can use a .tap file to save it or even a real tape connected to line out of your soundcard.\n"
					"It uses a small amount of RAM on memory display and some bytes on the stack, so it is not a perfect "
					"routine and sometimes may fail.\n"
					"The source code can be found on mantransfev3.tap\n"
					"Note: Although mantransfe is a Spectrum program and it could run on a real spectrum or another emulator, "
					"the saving routine needs that ZEsarUX emulator tells which im mode the cpu is (IM1 or IM2), "
					"so, a saved program can be run on a real spectrum or another emulator, "
					"but the saving routine sees im1 by default, so, saving from a real spectrum or another emulator "
					"instead ZEsarUX will only work if the cpu is in IM1 mode (and not IM2)");
	}


		/* De momento desactivado
		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_debug_settings,MENU_OPCION_NORMAL,menu_debug_spritefinder,NULL,"Spritefinder: %s",
					(spritefinder_enabled.v ? "Yes" : "No") );
		}
		*/

	


                menu_add_item_menu(array_menu_debug_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_debug_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_debug_settings);

                retorno_menu=menu_dibuja_menu(&debug_settings_opcion_seleccionada,&item_seleccionado,array_menu_debug_settings,"Debug" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_textspeech_filter_program(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

        filtros[0]="";
        filtros[1]=0;

/*
	char string_program[PATH_MAX];
	if (textspeech_filter_program!=NULL) {
		sprintf (string_program,"%s",textspeech_filter_program);
	}

	else {
		string_program[0]=0;
	}



	int ret=menu_filesel("Select Speech Program",filtros,string_program);


	if (ret==1) {
		sprintf (menu_buffer_textspeech_filter_program,"%s",string_program);
		textspeech_filter_program=menu_buffer_textspeech_filter_program;
		textspeech_filter_program_check_spaces();
	}

	else {
		textspeech_filter_program=NULL;
	}
*/

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de speech program
        //si no hay directorio, vamos a rutas predefinidas
        if (textspeech_filter_program==NULL) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(textspeech_filter_program,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

        int ret;

        ret=menu_filesel("Select Speech Program",filtros,menu_buffer_textspeech_filter_program);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


        if (ret==1) {

                textspeech_filter_program=menu_buffer_textspeech_filter_program;
					textspeech_filter_program_check_spaces();
			}

		else {
			textspeech_filter_program=NULL;
        }



}

void menu_textspeech_stop_filter_program(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="";
        filtros[1]=0;


/*
        char string_program[PATH_MAX];
        if (textspeech_stop_filter_program!=NULL) {
                sprintf (string_program,"%s",textspeech_stop_filter_program);
        }

        else {
                string_program[0]=0;
        }



        int ret=menu_filesel("Select Stop Speech Prg",filtros,string_program);


        if (ret==1) {
                sprintf (menu_buffer_textspeech_stop_filter_program,"%s",string_program);
                textspeech_stop_filter_program=menu_buffer_textspeech_stop_filter_program;
                textspeech_stop_filter_program_check_spaces();
        }

        else {
                textspeech_stop_filter_program=NULL;
        }
*/

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de speech program
        //si no hay directorio, vamos a rutas predefinidas
        if (textspeech_stop_filter_program==NULL) menu_chdir_sharedfiles();
        else {
                char directorio[PATH_MAX];
                util_get_dir(textspeech_stop_filter_program,directorio);
                //printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

                //cambiamos a ese directorio, siempre que no sea nulo
                if (directorio[0]!=0) {
                        debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
                        menu_filesel_chdir(directorio);
                }
        }

        int ret;

        ret=menu_filesel("Select Stop Speech Prg",filtros,menu_buffer_textspeech_stop_filter_program);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


        if (ret==1) {

                textspeech_stop_filter_program=menu_buffer_textspeech_stop_filter_program;
                                        textspeech_stop_filter_program_check_spaces();
                        }

                else {
                        textspeech_stop_filter_program=NULL;
        }




}

void menu_textspeech_filter_timeout(MENU_ITEM_PARAMETERS)
{

       int valor;

        char string_value[3];

        sprintf (string_value,"%d",textspeech_timeout_no_enter);


        menu_ventana_scanf("Timeout (0=never)",string_value,3);

        valor=parse_string_to_number(string_value);

	if (valor<0) debug_printf (VERBOSE_ERR,"Timeout must be 0 minimum");

	else textspeech_timeout_no_enter=valor;


}

void menu_textspeech_program_wait(MENU_ITEM_PARAMETERS)
{
	textspeech_filter_program_wait.v ^=1;
}

void menu_textspeech_send_menu(MENU_ITEM_PARAMETERS)
{
        textspeech_also_send_menu.v ^=1;
}


#ifdef COMPILE_STDOUT
void menu_display_stdout_send_speech_debug(MENU_ITEM_PARAMETERS)
{
	scrstdout_also_send_speech_debug_messages.v ^=1;
}
#endif

void menu_textspeech(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_textspeech;
        menu_item item_seleccionado;
        int retorno_menu;

        do {



                char string_filterprogram_shown[14];
		char string_stop_filterprogram_shown[14];

		if (textspeech_filter_program!=NULL) {
	                menu_tape_settings_trunc_name(textspeech_filter_program,string_filterprogram_shown,14);
		}

		else {
		sprintf (string_filterprogram_shown,"None");
		}



                if (textspeech_stop_filter_program!=NULL) {
                        menu_tape_settings_trunc_name(textspeech_stop_filter_program,string_stop_filterprogram_shown,14);
                }

                else {
                sprintf (string_stop_filterprogram_shown,"None");
                }


                        menu_add_item_menu_inicial_format(&array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_filter_program,NULL,"~~Speech program [%s]",string_filterprogram_shown);
			menu_add_item_menu_shortcut(array_menu_textspeech,'s');
        	        menu_add_item_menu_tooltip(array_menu_textspeech,"Specify which program to send generated text");
        	        menu_add_item_menu_ayuda(array_menu_textspeech,"Specify which program to send generated text. Text is send to the program "
						"to its standard input on Unix versions (Linux, Mac, etc) or sent as the first parameter on "
						"Windows (MINGW) version\n"
						"Pressing a key on the menu (or ESC with menu closed) forces the following queded speech entries to flush, and running the "
						"Stop Program to stop the current speech script.\n");


			if (textspeech_filter_program!=NULL) {

				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_stop_filter_program,NULL,"Stop program [%s]",string_stop_filterprogram_shown);

        	                menu_add_item_menu_tooltip(array_menu_textspeech,"Specify a path to a program or script in charge of stopping the running speech program");
                	        menu_add_item_menu_ayuda(array_menu_textspeech,"Specify a path to a program or script in charge of stopping the running speech program. If not specified, the current speech script can't be stopped");


				menu_add_item_menu(array_menu_textspeech,"",MENU_OPCION_SEPARADOR,NULL,NULL);


				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_filter_timeout,NULL,"[%d] ~~Timeout no enter",textspeech_timeout_no_enter);
				menu_add_item_menu_shortcut(array_menu_textspeech,'t');
				menu_add_item_menu_tooltip(array_menu_textspeech,"After some seconds the text will be sent to the Speech program when no "
						"new line is sent");
				menu_add_item_menu_ayuda(array_menu_textspeech,"After some seconds the text will be sent to the Speech program when no "
						"new line is sent. 0=never");



				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_program_wait,NULL,"[%c] ~~Wait program to exit",(textspeech_filter_program_wait.v ? 'X' : ' ' ) );
				menu_add_item_menu_shortcut(array_menu_textspeech,'w');
                	        menu_add_item_menu_tooltip(array_menu_textspeech,"Wait and pause the emulator until the Speech program returns");
                        	menu_add_item_menu_ayuda(array_menu_textspeech,"Wait and pause the emulator until the Speech program returns");


				menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_textspeech_send_menu,NULL,"[%c] Also send ~~menu",(textspeech_also_send_menu.v ? 'X' : ' ' ));
				menu_add_item_menu_shortcut(array_menu_textspeech,'m');
				menu_add_item_menu_tooltip(array_menu_textspeech,"Also send text menu entries to Speech program");
				menu_add_item_menu_ayuda(array_menu_textspeech,"Also send text menu entries to Speech program");

#ifdef COMPILE_STDOUT
				if (menu_cond_stdout() ) {
							menu_add_item_menu_format(array_menu_textspeech,MENU_OPCION_NORMAL,menu_display_stdout_send_speech_debug,NULL,"[%c] Also send debug messages", (scrstdout_also_send_speech_debug_messages.v==1 ? 'X' : ' '));
							menu_add_item_menu_tooltip(array_menu_textspeech,"Also send debug messages to speech");
							menu_add_item_menu_ayuda(array_menu_textspeech,"Also send debug messages to speech");

				}

#endif

			}


          menu_add_item_menu(array_menu_textspeech,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_textspeech,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_textspeech);

                retorno_menu=menu_dibuja_menu(&textspeech_opcion_seleccionada,&item_seleccionado,array_menu_textspeech,"Text to Speech" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_interface_fullscreen(MENU_ITEM_PARAMETERS)
{

	if (ventana_fullscreen==0) {
		scr_set_fullscreen();
	}

	else {
		scr_reset_fullscreen();
	}

	clear_putpixel_cache();
	menu_init_footer();

}

void menu_interface_rgb_inverse_common(void)
{
	modificado_border.v=1;
	screen_init_colour_table();

        //Dado que se han cambiado la paleta de colores, hay que vaciar la putpixel cache
        clear_putpixel_cache();

	menu_init_footer();
}


void menu_interface_red(MENU_ITEM_PARAMETERS)
{
	screen_gray_mode ^= 4;
	menu_interface_rgb_inverse_common();
}

void menu_interface_green(MENU_ITEM_PARAMETERS)
{
        screen_gray_mode ^= 2;
	menu_interface_rgb_inverse_common();
}

void menu_interface_blue(MENU_ITEM_PARAMETERS)
{
        screen_gray_mode ^= 1;
	menu_interface_rgb_inverse_common();
}

void menu_interface_inverse_video(MENU_ITEM_PARAMETERS)
{
        inverse_video.v ^= 1;
	menu_interface_rgb_inverse_common();
}

void menu_interface_border(MENU_ITEM_PARAMETERS)
{

	//Esperar a que no estemos redibujando pantalla
	//while (sem_screen_refresh_reallocate_layers) {
	//	printf ("-----Waiting until redraw and realloc functions finish\n");
	//}

        debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	if (border_enabled.v) disable_border();
	else enable_border();

        //scr_init_pantalla();

	//printf ("--antes de init pantalla\n");

	screen_init_pantalla_and_others();

	//printf ("--despues de init pantalla\n");

    debug_printf(VERBOSE_INFO,"Creating Screen");

	//printf ("--antes de init footer\n");
	menu_init_footer();
	//printf ("--despues de init footer\n");

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	

	//printf ("--despues de restore overlay\n");
	
}

void menu_interface_hidemouse(MENU_ITEM_PARAMETERS)
{
    debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;
	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	mouse_pointer_shown.v ^=1;

	screen_init_pantalla_and_others();

    debug_printf(VERBOSE_INFO,"Creating Screen");

	menu_init_footer();


	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);	
	
}

int menu_interface_border_cond(void)
{
	if (ventana_fullscreen) return 0;
	return 1;
}

int menu_interface_zoom_cond(void)
{
        if (ventana_fullscreen) return 0;
        return 1;
}


void menu_tool_path(char *tool_path,char *name)
{

        char *filtros[2];

        filtros[0]="";
        filtros[1]=0;

	char buffer_tool_path[PATH_MAX];

        //guardamos directorio actual
        char directorio_actual[PATH_MAX];
        getcwd(directorio_actual,PATH_MAX);

        //Obtenemos directorio de la tool

        char directorio[PATH_MAX];
        util_get_dir(tool_path,directorio);
        debug_printf (VERBOSE_INFO,"Last directory: %s",directorio);

        //cambiamos a ese directorio, siempre que no sea nulo
        if (directorio[0]!=0) {
		debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
		menu_filesel_chdir(directorio);
        }


        int ret;

        char ventana_titulo[40];
	sprintf (ventana_titulo,"Select %s tool",name);

        ret=menu_filesel(ventana_titulo,filtros,buffer_tool_path);
        //volvemos a directorio inicial
        menu_filesel_chdir(directorio_actual);


        if (ret==1) {
		sprintf (tool_path,"%s",buffer_tool_path);
        }

}


void menu_external_tool_sox(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_sox,"sox");
}

/*void menu_external_tool_unzip(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_unzip,"unzip");
}*/

void menu_external_tool_gunzip(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_gunzip,"gunzip");
}

void menu_external_tool_tar(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_tar,"tar");
}

void menu_external_tool_unrar(MENU_ITEM_PARAMETERS)
{
	menu_tool_path(external_tool_unrar,"unrar");
}



void menu_external_tools_config(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_external_tools_config;
        menu_item item_seleccionado;
        int retorno_menu;


	char string_sox[20];
	//char string_unzip[20];
	char string_gunzip[20];
	char string_tar[20];
	char string_unrar[20];


        do {

		menu_tape_settings_trunc_name(external_tool_sox,string_sox,20);
		//menu_tape_settings_trunc_name(external_tool_unzip,string_unzip,20);
		menu_tape_settings_trunc_name(external_tool_gunzip,string_gunzip,20);
		menu_tape_settings_trunc_name(external_tool_tar,string_tar,20);
		menu_tape_settings_trunc_name(external_tool_unrar,string_unrar,20);

                menu_add_item_menu_inicial_format(&array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_sox,NULL,"~~Sox [%s]",string_sox);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'s');
                menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Sox Path");
                menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Sox Path. Path can not include spaces");


                /*menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_unzip,NULL,"Un~~zip [%s]",string_unzip);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'z');
                menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Unzip Path");
                menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Unzip Path. Path can not include spaces");*/



                menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_gunzip,NULL,"~~Gunzip [%s]",string_gunzip);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'g');
                menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Gunzip Path");
                menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Gunzip Path. Path can not include spaces");



                menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_tar,NULL,"~~Tar [%s]",string_tar);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'t');
                menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Tar Path");
                menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Tar Path. Path can not include spaces");


                menu_add_item_menu_format(array_menu_external_tools_config,MENU_OPCION_NORMAL,menu_external_tool_unrar,NULL,"Un~~rar [%s]",string_unrar);
		menu_add_item_menu_shortcut(array_menu_external_tools_config,'r');
                menu_add_item_menu_tooltip(array_menu_external_tools_config,"Change Unrar Path");
                menu_add_item_menu_ayuda(array_menu_external_tools_config,"Change Unrar Path. Path can not include spaces");



                menu_add_item_menu(array_menu_external_tools_config,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_external_tools_config,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_external_tools_config);

                retorno_menu=menu_dibuja_menu(&external_tools_config_opcion_seleccionada,&item_seleccionado,array_menu_external_tools_config,"External tools paths" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}




int num_menu_scr_driver;
int num_previo_menu_scr_driver;


//Determina cual es el video driver actual
void menu_change_video_driver_get(void)
{
	int i;
        for (i=0;i<num_scr_driver_array;i++) {
		if (!strcmp(scr_driver_name,scr_driver_array[i].driver_name)) {
			num_menu_scr_driver=i;
			num_previo_menu_scr_driver=i;
			return;
		}

        }

}

void menu_change_video_driver_change(MENU_ITEM_PARAMETERS)
{
	num_menu_scr_driver++;
	if (num_menu_scr_driver==num_scr_driver_array) num_menu_scr_driver=0;
}

void menu_change_video_driver_apply(MENU_ITEM_PARAMETERS)
{

	//Si driver null, avisar
	if (!strcmp(scr_driver_array[num_menu_scr_driver].driver_name,"null")) {
		if (menu_confirm_yesno_texto("Driver is null","Sure?")==0) return;
	}

	//Si driver es cocoa, no dejar cambiar a cocoa
	if (!strcmp(scr_driver_array[num_menu_scr_driver].driver_name,"cocoa")) {
		debug_printf(VERBOSE_ERR,"You can not set cocoa driver from menu. "
				"You must start emulator with cocoa driver (with --vo cocoa or without any --vo setting)");
		return;
	}


	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;

	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);



	screen_reset_scr_driver_params();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=scr_driver_array[num_menu_scr_driver].funcion_init;
        funcion_set=scr_driver_array[num_menu_scr_driver].funcion_set;

	int resultado=funcion_init();
	set_menu_gui_zoom();
	clear_putpixel_cache();

screen_restart_pantalla_restore_overlay(previous_function,menu_antes);


                if ( resultado == 0 ) {
                        funcion_set();
			menu_generic_message_splash("Apply Driver","OK. Driver applied");
	                //Y salimos de todos los menus
	                salir_todos_menus=1;

                }

		else {
			debug_printf(VERBOSE_ERR,"Can not set video driver. Restoring to previous driver %s",scr_driver_name);
			menu_change_video_driver_get();


			screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


			//Restaurar video driver
			screen_reset_scr_driver_params();
		        funcion_init=scr_driver_array[num_previo_menu_scr_driver].funcion_init;
			set_menu_gui_zoom();

		        funcion_set=scr_driver_array[num_previo_menu_scr_driver].funcion_set;

			funcion_init();
			clear_putpixel_cache();
			funcion_set();


			screen_restart_pantalla_restore_overlay(previous_function,menu_antes);
		}

        //scr_init_pantalla();

	modificado_border.v=1;

	menu_init_footer();


	if (!strcmp(scr_driver_name,"aa")) {
		menu_generic_message_format("Warning","Remember that on aa video driver, menu is opened with %s",openmenu_key_message);
	}

	//TODO
	//Para aalib, tanto aqui como en cambio de border, no se ve el cursor del menu .... en cuanto se redimensiona la ventana, se arregla


}

void menu_change_video_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_video_driver;
        menu_item item_seleccionado;
        int retorno_menu;

	menu_change_video_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_video_driver,MENU_OPCION_NORMAL,menu_change_video_driver_change,NULL,"Video Driver: %s",scr_driver_array[num_menu_scr_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_video_driver,MENU_OPCION_NORMAL,menu_change_video_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_video_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_video_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_video_driver);

                retorno_menu=menu_dibuja_menu(&change_video_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_video_driver,"Change Video Driver" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int menu_change_video_driver_cond(void)
{
	if (ventana_fullscreen) return 0;
	else return 1;
}




void menu_interface_footer(MENU_ITEM_PARAMETERS)
{



        debug_printf(VERBOSE_INFO,"End Screen");

	//Guardar funcion de texto overlay activo, para desactivarlo temporalmente. No queremos que se salte a realloc_layers simultaneamente,
	//mientras se hace putpixel desde otro sitio -> provocaria escribir pixel en layer que se esta reasignando
  void (*previous_function)(void);
  int menu_antes;
	screen_end_pantalla_save_overlay(&previous_function,&menu_antes);


        if (menu_footer==0) {
		enable_footer();
	}

        else {
                disable_footer();
                
        }


        modificado_border.v=1;
        debug_printf(VERBOSE_INFO,"Creating Screen");
        //scr_init_pantalla();
	screen_init_pantalla_and_others();


	if (menu_footer) menu_init_footer();

	screen_restart_pantalla_restore_overlay(previous_function,menu_antes);

}


void menu_interface_frameskip(MENU_ITEM_PARAMETERS)
{

        menu_hardware_advanced_input_value(0,49,"Frameskip",&frameskip);
}

void menu_interface_show_splash_texts(MENU_ITEM_PARAMETERS)
{
	screen_show_splash_texts.v ^=1;
}

void menu_interface_tooltip(MENU_ITEM_PARAMETERS)
{
	tooltip_enabled.v ^=1;
	menu_tooltip_counter=0;
}

void menu_interface_first_aid(MENU_ITEM_PARAMETERS)
{
	menu_disable_first_aid.v ^=1;
}

void menu_interface_restore_first_aid(MENU_ITEM_PARAMETERS)
{
	menu_first_aid_restore_all();

	menu_generic_message("Restore messages","OK. Restored all first aid messages");
}

void menu_interface_force_atajo(MENU_ITEM_PARAMETERS)
{
        menu_force_writing_inverse_color.v ^=1;
}


void menu_interface_zoom(MENU_ITEM_PARAMETERS)
{
        char string_zoom[2];
	int temp_zoom;


	//comprobaciones previas para no petar el sprintf
	if (zoom_x>9 || zoom_x<1) zoom_x=1;

        sprintf (string_zoom,"%d",zoom_x);


        menu_ventana_scanf("Window Zoom",string_zoom,2);

        temp_zoom=parse_string_to_number(string_zoom);


	screen_set_window_zoom(temp_zoom);

}


void menu_interface_change_gui_style(MENU_ITEM_PARAMETERS)
{
	estilo_gui_activo++;
	if (estilo_gui_activo==ESTILOS_GUI) estilo_gui_activo=0;
	set_charset();

	menu_init_footer();
}

void menu_interface_multitask(MENU_ITEM_PARAMETERS)
{

	menu_multitarea=menu_multitarea^1;
	if (menu_multitarea==0) {
		//audio_thread_finish();
		audio_playing.v=0;
	}
	timer_reset();

}

void menu_interface_autoframeskip(MENU_ITEM_PARAMETERS)
{
	autoframeskip.v ^=1;
}

void menu_interface_show_cpu_usage(MENU_ITEM_PARAMETERS)
{
	screen_show_cpu_usage.v ^=1;
	clear_putpixel_cache();
	if (!screen_show_cpu_usage.v) menu_init_footer();
}

void menu_interface_real_1648_palette(MENU_ITEM_PARAMETERS)
{
	spectrum_1648_use_real_palette.v ^=1;
	//screen_set_spectrum_palette_offset();
	menu_interface_rgb_inverse_common();
}

void menu_colour_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_colour_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_red,NULL,"[%c] ~~Red display",(screen_gray_mode & 4 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'r');

		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_green,NULL,"[%c] ~~Green display",(screen_gray_mode & 2 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'g');
		
		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_blue,NULL,"[%c] ~~Blue display",(screen_gray_mode & 1 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'b');

		menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_inverse_video,NULL,"[%c] ~~Inverse video",(inverse_video.v==1 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_colour_settings,'i');
		menu_add_item_menu_tooltip(array_menu_colour_settings,"Inverse Color Palette");
		menu_add_item_menu_ayuda(array_menu_colour_settings,"Inverses all the colours used on the emulator, including menu");


		if (MACHINE_IS_SPECTRUM_16 || MACHINE_IS_SPECTRUM_48) {
			menu_add_item_menu_format(array_menu_colour_settings,MENU_OPCION_NORMAL,menu_interface_real_1648_palette,NULL,"[%c] R~~eal palette",(spectrum_1648_use_real_palette.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_colour_settings,'e');
			menu_add_item_menu_tooltip(array_menu_colour_settings,"Use real Spectrum 16/48/+ colour palette");
			menu_add_item_menu_ayuda(array_menu_colour_settings,"Use real Spectrum 16/48/+ colour palette. "
				"In fact, this palette is the same as a Spectrum issue 3, and almost the same as issue 1 and 2");
		}

        menu_add_item_menu(array_menu_colour_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_ESC_item(array_menu_colour_settings);

                retorno_menu=menu_dibuja_menu(&colour_settings_opcion_seleccionada,&item_seleccionado,array_menu_colour_settings,"Colour Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_interface_charwidth(MENU_ITEM_PARAMETERS)
{
	menu_char_width--;

	if (menu_char_width==4) menu_char_width=8;
}

void menu_window_settings_reduce_075(MENU_ITEM_PARAMETERS)
{
	screen_reduce_075.v ^=1;
	enable_rainbow();
}

void menu_window_settings_reduce_075_antialias(MENU_ITEM_PARAMETERS)
{
	screen_reduce_075_antialias.v ^=1;
}

void menu_window_settings_reduce_075_ofx(MENU_ITEM_PARAMETERS)
{
        char string_offset[3];
        sprintf (string_offset,"%d",screen_reduce_offset_x);
        menu_ventana_scanf("Offset x",string_offset,3);
        screen_reduce_offset_x=parse_string_to_number(string_offset);
}

void menu_window_settings_reduce_075_ofy(MENU_ITEM_PARAMETERS)
{
        char string_offset[3];
        sprintf (string_offset,"%d",screen_reduce_offset_y);
        menu_ventana_scanf("Offset y",string_offset,3);
        screen_reduce_offset_y=parse_string_to_number(string_offset);
}

void menu_interface_hide_vertical_perc_bar(MENU_ITEM_PARAMETERS)
{
		menu_hide_vertical_percentaje_bar.v ^=1;
}



void menu_interface_hide_minimize_button(MENU_ITEM_PARAMETERS)
{
	menu_hide_minimize_button.v ^=1;
}

void menu_interface_hide_close_button(MENU_ITEM_PARAMETERS)
{
	menu_hide_close_button.v ^=1;
}

void menu_interface_invert_mouse_scroll(MENU_ITEM_PARAMETERS)
{
	menu_invert_mouse_scroll.v ^=1;
}

void menu_interface_mix_menu(MENU_ITEM_PARAMETERS)
{
	screen_menu_mix_method++;
	if (screen_menu_mix_method==MAX_MENU_MIX_METHODS) screen_menu_mix_method=0;
}

void menu_interface_mix_tranparency(MENU_ITEM_PARAMETERS)
{


	char string_trans[3];

        sprintf (string_trans,"%d",screen_menu_mix_transparency);

        menu_ventana_scanf("Transparency? (0-95)",string_trans,3);

        int valor=parse_string_to_number(string_trans);
	if (valor<0 || valor>95) {
		debug_printf (VERBOSE_ERR,"Invalid value");
	}

	else {
		screen_menu_mix_transparency=valor;
	}


}

void menu_interface_reduce_bright_menu(MENU_ITEM_PARAMETERS)
{
	screen_menu_reduce_bright_machine.v ^=1;
}


void menu_interface_bw_no_multitask(MENU_ITEM_PARAMETERS)
{
	screen_machine_bw_no_multitask.v ^=1;
}

void menu_interface_restore_windows_geometry(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Restore windows geometry")) {
		util_clear_all_windows_geometry();
		menu_generic_message("Restore windows geometry","OK. All windows restored to their default positions and sizes");
	}
}

void menu_window_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_window_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


			//hotkeys usadas: fbmzropciln

        	menu_add_item_menu_inicial_format(&array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_fullscreen,NULL,"[%c] ~~Full Screen",(ventana_fullscreen ? 'X' : ' ' ) );
		menu_add_item_menu_shortcut(array_menu_window_settings,'f');

		if (!MACHINE_IS_Z88 && !MACHINE_IS_TSCONF && !MACHINE_IS_TBBLUE) {
	                menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_border,menu_interface_border_cond,"[%c] ~~Border enabled", (border_enabled.v==1 ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_window_settings,'b');
		}

		if (!strcmp(scr_driver_name,"xwindows")  || !strcmp(scr_driver_name,"sdl") || !strcmp(scr_driver_name,"cocoa") ) {
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_hidemouse,NULL,"[%c] ~~Mouse pointer", (mouse_pointer_shown.v==1 ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_window_settings,'m');
		}


                if (si_complete_video_driver() ) {
                        menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_zoom,menu_interface_zoom_cond,"[%d] Window Size ~~Zoom",zoom_x);
			menu_add_item_menu_shortcut(array_menu_window_settings,'z');
                        menu_add_item_menu_tooltip(array_menu_window_settings,"Change Window Zoom");
                        menu_add_item_menu_ayuda(array_menu_window_settings,"Changes Window Size Zoom (width and height)");
                }


		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075,NULL,"[%c] ~~Reduce to 0.75",(screen_reduce_075.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'r');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Reduce machine display output by 0.75. Enables realvideo and forces watermark");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Reduce machine display output by 0.75. Enables realvideo and forces watermark. This feature has been used on a large bulb display for the RunZX 2018 event");

		if (screen_reduce_075.v) {
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_antialias,NULL,"[%c] Reduce Antialias",(screen_reduce_075_antialias.v ? 'X' : ' ') );
			menu_add_item_menu_tooltip(array_menu_window_settings,"Antialias is only applied to the standard 16 Spectrum colors");
			menu_add_item_menu_ayuda(array_menu_window_settings,"Antialias is only applied to the standard 16 Spectrum colors");

			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_ofx,NULL,"[%d] Reduce offset x",screen_reduce_offset_x);
			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_window_settings_reduce_075_ofy,NULL,"[%d] Reduce offset y",screen_reduce_offset_y);
		}
		

		/*"--reduce-075               Reduce display size 4/3 (divide by 4, multiply by 3)\n"
		"--reduce-075-offset-x n    Destination offset x on reduced display\n"
		"--reduce-075-offset-y n    Destination offset y on reduced display\n"*/



		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_footer,NULL,"[%c] Window F~~ooter",(menu_footer ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'o');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Show on footer some machine information");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Show on footer some machine information, like tape loading");


		//Uso cpu no se ve en windows
#ifndef MINGW
		if (menu_footer) {
		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_show_cpu_usage,NULL,"[%c] Show ~~CPU usage",(screen_show_cpu_usage.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'c');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Show CPU usage on footer");
		menu_add_item_menu_ayuda(array_menu_window_settings,"It tells you how much host cpu machine is using ZEsarUX. So it's better to have it low. "
														"Higher values mean you need a faster host machine to use ZEsarUX");
		}
#endif

		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_hide_vertical_perc_bar,NULL,"[%c] ~~Percentage bar",(menu_hide_vertical_percentaje_bar.v==0 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'p');
		menu_add_item_menu_tooltip(array_menu_window_settings,"Shows vertical percentaje bar on the right of text windows and file selector");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Shows vertical percentaje bar on the right of text windows and file selector");


		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_hide_minimize_button,NULL,"[%c] M~~inimize button",(menu_hide_minimize_button.v ? ' ' : 'X') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'i');
		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_hide_close_button,NULL,"[%c] C~~lose button",(menu_hide_close_button.v ? ' ' : 'X') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'l');
		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_invert_mouse_scroll,NULL,"[%c] I~~nvert mouse scroll",(menu_invert_mouse_scroll.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_window_settings,'n');

		menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_restore_windows_geometry,NULL,"    Restore windows geometry");
		menu_add_item_menu_tooltip(array_menu_window_settings,"Restore all windows positions and sizes to their default values");
		menu_add_item_menu_ayuda(array_menu_window_settings,"Restore all windows positions and sizes to their default values");

/*

0=Menu por encima de maquina, si no es transparente
1=Menu por encima de maquina, si no es transparente. Y Color Blanco con brillo es transparente
2=Mix de los dos colores, con control de transparecnai



*/


		if (si_complete_video_driver() ) {

			menu_add_item_menu(array_menu_window_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
			menu_add_item_menu(array_menu_window_settings,"--Special FX--",MENU_OPCION_SEPARADOR,NULL,NULL);

			menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_mix_menu,NULL,"[%s] Menu Mix Method",screen_menu_mix_methods_strings[screen_menu_mix_method] );
			menu_add_item_menu_tooltip(array_menu_window_settings,"How to mix menu and the layer below");
			menu_add_item_menu_ayuda(array_menu_window_settings,"How to mix menu and the layer below");

			if (screen_menu_mix_method==2) {
				menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_mix_tranparency,NULL,"[%d%%] Transparency",screen_menu_mix_transparency );
				menu_add_item_menu_tooltip(array_menu_window_settings,"Transparency percentage to apply to menu");
				menu_add_item_menu_ayuda(array_menu_window_settings,"Transparency percentage to apply to menu");
			}

			if (screen_menu_mix_method==0 || screen_menu_mix_method==1) {
				menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_reduce_bright_menu,NULL,"[%c] Darken when menu",(screen_menu_reduce_bright_machine.v ? 'X' : ' ' ) );
				menu_add_item_menu_tooltip(array_menu_window_settings,"Darken layer below menu when menu open");
				menu_add_item_menu_ayuda(array_menu_window_settings,"Darken layer below menu when menu open");
			}
		}


				menu_add_item_menu_format(array_menu_window_settings,MENU_OPCION_NORMAL,menu_interface_bw_no_multitask,NULL,"[%c] B&W on menu+no multitask",(screen_machine_bw_no_multitask.v ? 'X' : ' ' ) );
				menu_add_item_menu_tooltip(array_menu_window_settings,"Grayscale layer below menu when menu opened and multitask is disabled");
				menu_add_item_menu_ayuda(array_menu_window_settings,"Grayscale layer below menu when menu opened and multitask is disabled");

                menu_add_item_menu(array_menu_window_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_window_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_window_settings);

                retorno_menu=menu_dibuja_menu(&window_settings_opcion_seleccionada,&item_seleccionado,array_menu_window_settings,"Window Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_osd_settings_watermark(MENU_ITEM_PARAMETERS)
{
	if (screen_watermark_enabled.v==0) {
		//Ya se permite watermark con o sin realvideo
		//enable_rainbow();
		screen_watermark_enabled.v=1;
	}

	else screen_watermark_enabled.v=0;
}

void menu_osd_settings_watermark_position(MENU_ITEM_PARAMETERS)
{
	screen_watermark_position++;
	if (screen_watermark_position>3) screen_watermark_position=0;
}


void menu_osd_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_osd_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_osd_settings,MENU_OPCION_NORMAL,menu_interface_show_splash_texts,NULL,"[%c] ~~Show splash texts",(screen_show_splash_texts.v ? 'X' : ' ' ) );
		menu_add_item_menu_tooltip(array_menu_osd_settings,"Show on display some splash texts, like display mode change or watches");
		menu_add_item_menu_ayuda(array_menu_osd_settings,"Show on display some splash texts, like display mode change or watches");
		menu_add_item_menu_shortcut(array_menu_osd_settings,'s');







		menu_add_item_menu_format(array_menu_osd_settings,MENU_OPCION_NORMAL,menu_osd_settings_watermark,NULL,"[%c] ~~Watermark",(screen_watermark_enabled.v ? 'X' : ' ' ) );
		menu_add_item_menu_tooltip(array_menu_osd_settings,"Adds a watermark to the display");
		menu_add_item_menu_ayuda(array_menu_osd_settings,"Adds a watermark to the display. May produce flickering if not enabled realvideo. If using reduce window setting, it will be forced enabled");
		menu_add_item_menu_shortcut(array_menu_osd_settings,'w');

		//Esta posicion afecta tanto al watermark normal como al forzado de 0.75
		menu_add_item_menu_format(array_menu_osd_settings,MENU_OPCION_NORMAL,menu_osd_settings_watermark_position,NULL,"[%d] Watermark ~~position",screen_watermark_position);
		menu_add_item_menu_shortcut(array_menu_osd_settings,'p');
		

                menu_add_item_menu(array_menu_osd_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_osd_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_osd_settings);

                retorno_menu=menu_dibuja_menu(&osd_settings_opcion_seleccionada,&item_seleccionado,array_menu_osd_settings,"OSD Settings");

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


void menu_setting_limit_menu_open(MENU_ITEM_PARAMETERS)
{
	menu_limit_menu_open.v ^=1;
}


void menu_setting_filesel_no_show_dirs(MENU_ITEM_PARAMETERS)
{
	menu_filesel_hide_dirs.v ^=1;
}

void menu_setting_quickexit(MENU_ITEM_PARAMETERS)
{
	quickexit.v ^=1;
}


void menu_accessibility_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_accessibility_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {



                menu_add_item_menu_inicial_format(&array_menu_accessibility_settings,MENU_OPCION_NORMAL,menu_chardetection_settings,NULL,"~~Print char traps");
                        menu_add_item_menu_shortcut(array_menu_accessibility_settings,'p');
                        menu_add_item_menu_tooltip(array_menu_accessibility_settings,"Settings on capture print character routines");
                        menu_add_item_menu_ayuda(array_menu_accessibility_settings,"Settings on capture print character routines");


                        menu_add_item_menu_format(array_menu_accessibility_settings,MENU_OPCION_NORMAL,menu_textspeech,NULL,"~~Text to speech");
                        menu_add_item_menu_shortcut(array_menu_accessibility_settings,'t');
                        menu_add_item_menu_tooltip(array_menu_accessibility_settings,"Specify a script or program to send all text generated, "
                                                "from Spectrum display or emulator menu, "
                                                "usually used on text to speech");
                        menu_add_item_menu_ayuda(array_menu_accessibility_settings,"Specify a script or program to send all text generated, "
                                                "from Spectrum display or emulator menu, "
                                                "usually used on text to speech. "
                                                "When running the script: \n"
                                                "ESC means abort next executions on queue.\n"
                                                "Enter means run pending execution.\n");





   menu_add_item_menu(array_menu_accessibility_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_accessibility_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_accessibility_settings);

                retorno_menu=menu_dibuja_menu(&accessibility_settings_opcion_seleccionada,&item_seleccionado,array_menu_accessibility_settings,"Accessibility Settings");

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

/*void menu_bw_no_multitask(MENU_ITEM_PARAMETERS)
{
	screen_bw_no_multitask_menu.v ^=1;
}*/

void menu_interface_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_interface_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		//hotkeys usados:
		//ocewsruitfalqh

		menu_add_item_menu_inicial_format(&array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_multitask,NULL,"[%c] M~~ultitask menu", (menu_multitarea==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_interface_settings,'u');
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Enable menu with multitask");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Setting multitask on makes the emulation does not stop when the menu is active");



		/*
		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_bw_no_multitask,NULL,"B&W when no multitask: %s",
			(screen_bw_no_multitask_menu.v ? "Yes" : "No") );

		menu_add_item_menu_tooltip(array_menu_interface_settings,"Emulated machine display will change to black & white colours when menu open and multitask is off");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Emulated machine display will change to black & white colours when menu open and multitask is off");

		*/





		//menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard,NULL,"On Screen Adventure KB");


		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_charwidth,NULL,"[%d] Menu char w~~idth",menu_char_width);
		menu_add_item_menu_shortcut(array_menu_interface_settings,'i');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Menu character width. EXPERIMENTAL feature");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Menu character width. EXPERIMENTAL feature");



		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_tooltip,NULL,"[%c] ~~Tooltips",(tooltip_enabled.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_interface_settings,'t');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Enable or disable tooltips");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Enable or disable tooltips");

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_first_aid,NULL,"[%c] ~~First aid help",(menu_disable_first_aid.v==0 ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_interface_settings,'f');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Enable or disable First Aid help");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Enable or disable First Aid help");		

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_restore_first_aid,NULL,"    Restore all 1st aid mess.");
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Restore all First Aid help messages");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Restore all First Aid help messages");



		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_force_atajo,NULL,"[%c] Force visible hotkeys",(menu_force_writing_inverse_color.v ? 'X' : ' ') );
                menu_add_item_menu_tooltip(array_menu_interface_settings,"Force always show hotkeys");
                menu_add_item_menu_ayuda(array_menu_interface_settings,"Force always show hotkeys. By default it will only be shown after a timeout or wrong key pressed");

		int fps;
		int divisor=frameskip+1;
		if (divisor==0) {
			fps=50; //Esto no deberia suceder nunca. Pero lo hacemos por una posible division por 0 (si frameskip fuera -1)
		}
		else {
			fps=50/divisor;
		}

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_frameskip,NULL,"[%d] F~~rameskip (%d FPS)",frameskip,fps);
		menu_add_item_menu_shortcut(array_menu_interface_settings,'r');
			menu_add_item_menu_tooltip(array_menu_interface_settings,"Sets the number of frames to skip every time the screen needs to be refreshed");
			menu_add_item_menu_ayuda(array_menu_interface_settings,"Sets the number of frames to skip every time the screen needs to be refreshed");


		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_autoframeskip,NULL,"[%c] ~~Auto Frameskip",
				(autoframeskip.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_interface_settings,'a');	
			menu_add_item_menu_tooltip(array_menu_interface_settings,"Let ZEsarUX decide when to skip frames");
			menu_add_item_menu_ayuda(array_menu_interface_settings,"ZEsarUX skips frames when the host cpu use is too high. Then skiping frames the cpu use decreases");




		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_setting_limit_menu_open,NULL,"[%c] ~~Limit menu opening",
			(menu_limit_menu_open.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_interface_settings,'l');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Limit the action to open menu (F5 by default, joystick button)");			
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Limit the action to open menu (F5 by default, joystick button). To open it, you must press the key 3 times in one second");

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_setting_quickexit,NULL,"[%c] ~~Quick exit",
			(quickexit.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_interface_settings,'q');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Exit emulator quickly: no yes/no confirmation and no fadeout");			
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Exit emulator quickly: no yes/no confirmation and no fadeout");

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_setting_filesel_no_show_dirs,NULL,"[%c] ~~Hide dirs in filesel",
			(menu_filesel_hide_dirs.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_interface_settings,'h');	
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Hide directories from file selector menus");
		menu_add_item_menu_ayuda(array_menu_interface_settings,"Hide directories from file selector menus. "
								"Useful on demo environments and you don't want the user to be able to navigate the filesystem");


		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_interface_change_gui_style,NULL,"    GUI ~~style [%s]",
						definiciones_estilos_gui[estilo_gui_activo].nombre_estilo);
		menu_add_item_menu_shortcut(array_menu_interface_settings,'s');
		menu_add_item_menu_tooltip(array_menu_interface_settings,"Change GUI Style");
                menu_add_item_menu_ayuda(array_menu_interface_settings,"You can switch between:\n"
					"- ZEsarUX: default style\n"
					"- ZXSpectr: my first old emulator, that worked on MS-DOS and Windows. Celebrate its 20th anniversay with this style! :)\n"
					"- ZX80/81: ZX80&81 style\n"
					"- Z88: Z88 style\n"
					"- CPC: Amstrad CPC style\n"
					"- Sam: Sam Coupe style\n"
					"- ManSoftware: style using my own font I created when I was a child ;)\n"
					"- QL: Sinclair QL style\n"
					"- RetroMac: MacOS classic style\n"
					"- Clean: Simple style with black & white menus\n"
					"- CleanInverse: Same style as previous but using inverted colours\n"
					
					);
        menu_add_item_menu(array_menu_interface_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_window_settings,NULL,"~~Window settings");
		menu_add_item_menu_shortcut(array_menu_interface_settings,'w');

		//Con driver cocoa, no permitimos cambiar a otro driver
		if (strcmp(scr_driver_name,"cocoa")) {
			menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_change_video_driver,menu_change_video_driver_cond,"Change Video Driver");
		}


		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_osd_settings,NULL,"~~OSD settings");
		menu_add_item_menu_shortcut(array_menu_interface_settings,'o');		

		if (scr_driver_can_ext_desktop() ) {
			menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_ext_desktop_settings,NULL,"Z~~X Desktop settings");
			menu_add_item_menu_shortcut(array_menu_interface_settings,'x');	
			menu_add_item_menu_tooltip(array_menu_interface_settings,"Expand the program window having a ZX Desktop space to the right");
			menu_add_item_menu_ayuda(array_menu_interface_settings,"ZX Desktop enables you to have a space on the right to place "
				"zxvision windows or other widgets. This is a work in progress, so expect improvements in the next versions");
		}





		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_colour_settings,NULL,"~~Colour settings");
		menu_add_item_menu_shortcut(array_menu_interface_settings,'c');			

		menu_add_item_menu_format(array_menu_interface_settings,MENU_OPCION_NORMAL,menu_external_tools_config,NULL,"~~External tools paths");	
		menu_add_item_menu_shortcut(array_menu_interface_settings,'e');								

                menu_add_item_menu(array_menu_interface_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_interface_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_interface_settings);

                retorno_menu=menu_dibuja_menu(&interface_settings_opcion_seleccionada,&item_seleccionado,array_menu_interface_settings,"GUI Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



void menu_chardetection_settings_trap_rst16(MENU_ITEM_PARAMETERS)
{
        chardetect_printchar_enabled.v ^=1;
}



void menu_chardetection_settings_second_trap(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_char_dir);

        menu_ventana_scanf("Address (0=none)",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_char_dir=dir;

}

void menu_chardetection_settings_third_trap(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_third_trap_char_dir);

        menu_ventana_scanf("Address (0=none)",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_third_trap_char_dir=dir;

}

void menu_chardetection_settings_stdout_trap_detection(MENU_ITEM_PARAMETERS)
{


        trap_char_detection_routine_number++;
        if (trap_char_detection_routine_number==TRAP_CHAR_DETECTION_ROUTINES_TOTAL) trap_char_detection_routine_number=0;

        chardetect_init_trap_detection_routine();

}

void menu_chardetection_settings_chardetect_char_filter(MENU_ITEM_PARAMETERS)
{
        chardetect_char_filter++;
        if (chardetect_char_filter==CHAR_FILTER_TOTAL) chardetect_char_filter=0;
}

void menu_chardetection_settings_stdout_line_width(MENU_ITEM_PARAMETERS)
{

        char string_width[3];

        int width;


        sprintf (string_width,"%d",chardetect_line_width);

        menu_ventana_scanf("Line width 0=no limit",string_width,3);

        width=parse_string_to_number(string_width);

        //if (width>999) {
        //        debug_printf (VERBOSE_ERR,"Invalid width %d",width);
        //        return;
        //}
        chardetect_line_width=width;

}

void menu_chardetection_settings_second_trap_sum32(MENU_ITEM_PARAMETERS)
{

        chardetect_second_trap_sum32.v ^=1;

        //y ponemos el contador al maximo para que no se cambie por si solo
        chardetect_second_trap_sum32_counter=MAX_STDOUT_SUM32_COUNTER;


}


void menu_chardetection_settings_second_trap_range_min(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_detect_pc_min);

        menu_ventana_scanf("Address",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_detect_pc_min=dir;

}

void menu_chardetection_settings_second_trap_range_max(MENU_ITEM_PARAMETERS)
{

        char string_dir[6];

        int dir;


        sprintf (string_dir,"%d",chardetect_second_trap_detect_pc_max);

        menu_ventana_scanf("Address",string_dir,6);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }

        chardetect_second_trap_detect_pc_max=dir;

}


void menu_chardetection_settings_stdout_line_witdh_space(MENU_ITEM_PARAMETERS)
{
        chardetect_line_width_wait_space.v ^=1;
}


void menu_chardetection_settings_enable(MENU_ITEM_PARAMETERS)
{
	chardetect_detect_char_enabled.v ^=1;
}



//menu chardetection settings
void menu_chardetection_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_chardetection_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                        menu_add_item_menu_inicial_format(&array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_trap_rst16,NULL,"[%c] ~~Trap print", (chardetect_printchar_enabled.v==1 ? 'X' : ' ' ));
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'t');
                        menu_add_item_menu_tooltip(array_menu_chardetection_settings,"It enables the emulator to show the text sent to standard rom print call routines and non standard, generated from some games, specially text adventures");
                        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"It enables the emulator to show the text sent to standard rom print call routines and generated from some games, specially text adventures. "
                                                "On Spectrum, ZX80, ZX81 machines, standard rom calls are RST 10H. On Z88, it traps OS_OUT and some other calls. Non standard calls are the ones indicated on Second and Third trap");


			if (chardetect_printchar_enabled.v) {


	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap,NULL,"~~Second trap address [%d]",chardetect_second_trap_char_dir);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'s');
        	                menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Address of the second print routine");
                	        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Address of the second print routine");

	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_sum32,NULL,"[%c] Second trap s~~um 32",(chardetect_second_trap_sum32.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'u');
				menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Sums 32 to the ASCII value read");
				menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Sums 32 to the ASCII value read");


        	                menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_third_trap,NULL,"T~~hird trap address [%d]",chardetect_third_trap_char_dir);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'h');
                	        menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Address of the third print routine");
                        	menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Address of the third print routine");

       menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);							

                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_line_width,NULL,"[%d] Line ~~width",chardetect_line_width);
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'w');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Line width");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Line width. Setting 0 means no limit, so "
						"even when a carriage return is received, the text will not be sent unless a Enter "
						"key is pressed or when timeout no enter no text to speech is reached\n");


                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_line_witdh_space,NULL,"[%c] Line width w~~ait space",(chardetect_line_width_wait_space.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'a');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Wait for a space before jumping to a new line");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Wait for a space before jumping to a new line");


                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_chardetect_char_filter,NULL,"Char ~~filter [%s]",chardetect_char_filter_names[chardetect_char_filter]);
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'f');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Send characters to an internal filter");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Send characters to an internal filter");


			}

                menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);



			menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_enable,NULL,"[%c] Enable 2nd trap ~~detection",(chardetect_detect_char_enabled.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_chardetection_settings,'d');
			menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Enable char detection method to guess Second Trap address");
			menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Enable char detection method to guess Second Trap address");





			if (chardetect_detect_char_enabled.v) {


	                        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_stdout_trap_detection,NULL,"Detect ~~routine [%s]",trap_char_detection_routines_texto[trap_char_detection_routine_number]);
				menu_add_item_menu_shortcut(array_menu_chardetection_settings,'r');
        			 menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Selects method for second trap character routine detection");
	                        menu_add_item_menu_ayuda(array_menu_chardetection_settings,"This function enables second trap character routine detection for programs "
                                                "that does not use RST16 calls to ROM for printing characters, on Spectrum models. "
                                                "It tries to guess where the printing "
                                                "routine is located and set Second Trap address when it finds it. This function has some pre-defined known "
                                                "detection call printing routines (for example AD Adventures) and one other totally automatic method, "
                                        	"which first tries to find automatically an aproximate range where the routine is, and then, "
						"it finds which routine is, trying all on this list. "
						"This automatic method "
                                                "makes writing operations a bit slower (only while running the detection routine)");


        	                if (trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINE_AUTOMATIC && trap_char_detection_routine_number!=TRAP_CHAR_DETECTION_ROUTINE_NONE)  {
                        	        menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_range_min,NULL,"Detection routine mi~~n [%d]",chardetect_second_trap_detect_pc_min);
					menu_add_item_menu_shortcut(array_menu_chardetection_settings,'n');
					menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Lower address limit to find character routine");
					menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Lower address limit to find character routine");


                	                menu_add_item_menu_format(array_menu_chardetection_settings,MENU_OPCION_NORMAL,menu_chardetection_settings_second_trap_range_max,NULL,"Detection routine ma~~x [%d]",chardetect_second_trap_detect_pc_max);
					menu_add_item_menu_shortcut(array_menu_chardetection_settings,'x');
					menu_add_item_menu_tooltip(array_menu_chardetection_settings,"Higher address limit to find character routine");
					menu_add_item_menu_ayuda(array_menu_chardetection_settings,"Higher address limit to find character routine");
	                        }


			}






                menu_add_item_menu(array_menu_chardetection_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_chardetection_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_chardetection_settings);

                retorno_menu=menu_dibuja_menu(&chardetection_settings_opcion_seleccionada,&item_seleccionado,array_menu_chardetection_settings,"Print char traps" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}










void menu_display_load_screen(MENU_ITEM_PARAMETERS)
{

char screen_load_file[PATH_MAX];

  char *filtros[2];

        filtros[0]="scr";
        filtros[1]=0;


        if (menu_filesel("Select Screen File",filtros,screen_load_file)==1) {
		load_screen(screen_load_file);
                //Y salimos de todos los menus
                salir_todos_menus=1;

        }

}

void menu_display_save_screen(MENU_ITEM_PARAMETERS)
{

char screen_save_file[PATH_MAX];

  char *filtros[3];

        filtros[0]="scr";
        filtros[1]="pbm";
        filtros[2]=0;


        if (menu_filesel("Select Screen File",filtros,screen_save_file)==1) {

                //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(screen_save_file, &buf_stat)==0) {

			if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

                }


		if (!util_compare_file_extension(screen_save_file,"scr")) {
	                save_screen(screen_save_file);
		}

		else if (!util_compare_file_extension(screen_save_file,"pbm")) {

			//void util_convert_scr_sprite(z80_byte *origen,z80_byte *destino)



                                //Asignar buffer temporal
                                int longitud=6144;
                                z80_byte *buf_temp=malloc(longitud);
                                if (buf_temp==NULL) {
                                        debug_printf(VERBOSE_ERR,"Error allocating temporary buffer");
                                }

                                //Convertir pantalla a sprite ahi
				z80_byte *origen;
				origen=get_base_mem_pantalla();
				util_convert_scr_sprite(origen,buf_temp);

                                util_write_pbm_file(screen_save_file,256,192,8,buf_temp);

                                free(buf_temp);


		}

		else {
	                debug_printf(VERBOSE_ERR,"Unsuported file type");
        	        return;
		} 


                //Y salimos de todos los menus
                salir_todos_menus=1;

        }

}



/*
void menu_display_shows_vsync_on_display(void)
{
	video_zx8081_shows_vsync_on_display.v ^=1;
}
*/




/*void menu_display_zx8081_wrx_first_column(void)
{
	wrx_mueve_primera_columna.v ^=1;
}
*/

int menu_cond_wrx(void)
{
	return wrx_present.v;
}













int menu_display_rainbow_cond(void)
{
	//if (MACHINE_IS_Z88) return 0;
	return 1;
}





void menu_dibuja_rectangulo_relleno(zxvision_window *w,int x, int y, int ancho, int alto, int color)
{
	int x1,y1;

	for (y1=y;y1<y+alto;y1++) {
		for (x1=x;x1<=x+ancho;x1++) {
			zxvision_putpixel(w,x1,y1,color);
		}
	}
}





//menu display settings
void menu_display_settings(MENU_ITEM_PARAMETERS)
{

	menu_item *array_menu_display_settings;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		//char string_aux[50],string_aux2[50],emulate_zx8081_disp[50],string_arttext[50],string_aaslow[50],emulate_zx8081_thres[50],string_arttext_threshold[50];
		//char buffer_string[50];


                //Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
                menu_add_item_menu_inicial(&array_menu_display_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu(array_menu_display_settings,"~~Load Screen",MENU_OPCION_NORMAL,menu_display_load_screen,NULL);
			menu_add_item_menu_shortcut(array_menu_display_settings,'l');

			menu_add_item_menu(array_menu_display_settings,"~~Save Screen",MENU_OPCION_NORMAL,menu_display_save_screen,NULL);
			menu_add_item_menu_shortcut(array_menu_display_settings,'s');

		}

		menu_add_item_menu(array_menu_display_settings,"~~View Screen",MENU_OPCION_NORMAL,menu_view_screen,NULL);
		menu_add_item_menu_shortcut(array_menu_display_settings,'v');


			menu_add_item_menu(array_menu_display_settings,"View ~~Colour Palettes",MENU_OPCION_NORMAL,menu_display_total_palette,NULL);
			menu_add_item_menu_shortcut(array_menu_display_settings,'c');
			menu_add_item_menu_tooltip(array_menu_display_settings,"View full palettes or mapped palettes");
			menu_add_item_menu_ayuda(array_menu_display_settings,"You can see in this menu full colour palettes or mapped colour palettes. \n"
			 									"Full colour palettes means all the colours available for a mode, for example 256 colours on ULAPlus.\n"
												"Mapped colour palettes means the active palette for a mode, for example 64 colours on ULAPlus."

				);


       //Teclados en pantalla
                if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081) {
	                menu_add_item_menu(array_menu_display_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                        menu_add_item_menu_format(array_menu_display_settings,MENU_OPCION_NORMAL,menu_onscreen_keyboard,NULL,"On Screen ~~Keyboard");
                        menu_add_item_menu_shortcut(array_menu_display_settings,'k');
                        menu_add_item_menu_tooltip(array_menu_display_settings,"Open on screen keyboard");
                        menu_add_item_menu_ayuda(array_menu_display_settings,"You can also get this pressing F8, only for Spectrum and ZX80/81 machines");
				}

				if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_CPC) {

			menu_add_item_menu_format(array_menu_display_settings,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard,NULL,"On Screen ~~Adventure KB");
                        menu_add_item_menu_shortcut(array_menu_display_settings,'a');
                        menu_add_item_menu_tooltip(array_menu_display_settings,"Open On Screen Adventure Keyboard");
                        menu_add_item_menu_ayuda(array_menu_display_settings,"Here you have an on screen keyboard but uses words instead of just letters. "
				"It's useful to play Text Adventures, you can redefine your own words");

				}


			if (MACHINE_IS_SPECTRUM || MACHINE_IS_CPC) {
				menu_add_item_menu_format(array_menu_display_settings,MENU_OPCION_NORMAL,menu_unpaws_ungac,NULL," ~~Extract words to Adv. Keyb.");
				menu_add_item_menu_shortcut(array_menu_display_settings,'e');
				menu_add_item_menu_tooltip(array_menu_display_settings,"Runs the word extractor tool for adventure text games");
				menu_add_item_menu_ayuda(array_menu_display_settings,"It runs the word extractor tool and insert these words on the On Screen Adventure Keyboard. "
					"It can detect words on games written with Quill, Paws, DAAD, and GAC");
			}

                



 

                menu_add_item_menu(array_menu_display_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_display_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_display_settings);

                retorno_menu=menu_dibuja_menu(&display_settings_opcion_seleccionada,&item_seleccionado,array_menu_display_settings,"Display" );

                

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

void hotswap_zxuno_to_p2as_set_pages(void)
{

                int i;
                //Punteros a paginas de la ROM
                for (i=0;i<4;i++) {
                        rom_mem_table[i]=zxuno_sram_mem_table_new[i+8];
                }

                //Punteros a paginas de la RAM
                for (i=0;i<8;i++) {
                        ram_mem_table[i]=zxuno_sram_mem_table_new[i];
                }


                //Paginas mapeadas actuales
                for (i=0;i<4;i++) {
                        memory_paged[i]=zxuno_memory_paged_brandnew[i*2];
                }
}


//Hace hotswap de cualquier maquina a spectrum 48. simplemente hace copia de los 64kb
/*NOTA: no uso esta funcion pues al pasar de zxuno a spectrum
la pantalla refresca "de otro sitio", y solo se puede ver si se habilita interfaz spectra...
es raro, es un error pero no tiene que ver con spectra, tiene que ver con divmmc:
-si hago hotswap de zxuno a 48k con divmmc, luego la pantalla no se refresca
-si hago hotswap de zxuno a 48k sin divmmc, si se ve bien
No se exactamente porque ocurre, seguramente algo que ver con peek_byte_no_time y paginas de divmmc
Uso la otra funcion de hotswap_any_machine_to_spec48 que hay mas abajo
*/
void to_delete_hotswap_any_machine_to_spec48(void)
{

	//Asignamos 64kb RAM
	z80_byte *memoria_spectrum_final;
	memoria_spectrum_final=malloc(65536);

	if (memoria_spectrum_final==NULL) {
		cpu_panic ("Error. Cannot allocate Machine memory");
	}

        //Copiamos ROM y RAM a destino
        int i;
        //ROM y RAM
        for (i=0;i<65536;i++) memoria_spectrum_final[i]=peek_byte_no_time(i);

	free(memoria_spectrum);
        memoria_spectrum=memoria_spectrum_final;

	current_machine_type=1;

        set_machine_params();
        post_set_machine(NULL);

}

void hotswap_any_machine_to_spec48(void)
{

        //Asignamos 64kb RAM
        z80_byte *memoria_buffer;
        memoria_buffer=malloc(65536);

        if (memoria_buffer==NULL) {
                cpu_panic ("Error. Cannot allocate Machine memory");
        }

        //Copiamos ROM y RAM en buffer
        int i;
        for (i=0;i<65536;i++) memoria_buffer[i]=peek_byte_no_time(i);

        //Spectrum 48k
        current_machine_type=1;

        set_machine(NULL);

        //Copiar contenido de buffer en ROM y RAM
	//Por tanto la rom de destino sera la que habia antes del hotswap
	memcpy(memoria_spectrum,memoria_buffer,65536);

        free(memoria_buffer);
}

void hotswap_p2a_to_128(void)
{
	//Copiamos ROM0 y ROM3 en ROM 0 y 1 de spectrum 128k
	//La ram la copiamos tal cual
	z80_byte old_puerto_32765=puerto_32765;

	//Asignamos 32+128k de memoria
	z80_byte *memoria_buffer;
	memoria_buffer=malloc((32+128)*1024);

	if (memoria_buffer==NULL) {
					cpu_panic ("Error. Cannot allocate Machine memory");
	}

	//Copiamos rom 0 en buffer
	int i;
	z80_byte *puntero;
	puntero=rom_mem_table[0];
	for (i=0;i<16384;i++) {
		memoria_buffer[i]=*puntero;
		puntero++;
	}

	//Copiamos rom 3 en buffer
	puntero=rom_mem_table[3];
	for (i=0;i<16384;i++) {
		memoria_buffer[16384+i]=*puntero;
		puntero++;
	}

	//Copiamos paginas de ram
	int pagina;
	for (pagina=0;pagina<8;pagina++) {
		puntero=ram_mem_table[pagina];
		for (i=0;i<16384;i++) {
			memoria_buffer[32768+pagina*16384+i]=*puntero;
			puntero++;
		}
	}

	//Spectrum 128k
        current_machine_type=6;

        set_machine(NULL);

 //Mapear como estaba
 puerto_32765=old_puerto_32765;

	//asignar ram
	mem_page_ram_128k();

	//asignar rom
	mem_page_rom_128k();

	//Copiar contenido de buffer en ROM y RAM
	//No nos complicamos la vida, como sabemos que viene lineal las dos roms y la ram, volcamos

	for (i=0;i<(32+128)*1024;i++) memoria_spectrum[i]=memoria_buffer[i];

  free(memoria_buffer);



}



void hotswap_128_to_p2a(void)
{
	//Copiamos ROM0 en ROM0, ROM1 en ROM1, ROM0 en ROM2 y ROM1 en ROM3 de +2a
	//La ram la copiamos tal cual
	z80_byte old_puerto_32765=puerto_32765;

	//Asignamos 64+128k de memoria
	z80_byte *memoria_buffer;
	memoria_buffer=malloc((64+128)*1024);

	if (memoria_buffer==NULL) {
					cpu_panic ("Error. Cannot allocate Machine memory");
	}

	//Copiamos rom 0 y en buffer
	int i;
	z80_byte *puntero;
	puntero=rom_mem_table[0];
	for (i=0;i<16384;i++) {
		memoria_buffer[i]=*puntero;
		memoria_buffer[32768+i]=*puntero;
		puntero++;
	}

	//Copiamos rom 3 en buffer
	puntero=rom_mem_table[1];
	for (i=0;i<16384;i++) {
		memoria_buffer[16384+i]=*puntero;
		memoria_buffer[49152+i]=*puntero;
		puntero++;
	}

	//Copiamos paginas de ram
	int pagina;
	for (pagina=0;pagina<8;pagina++) {
		puntero=ram_mem_table[pagina];
		for (i=0;i<16384;i++) {
			memoria_buffer[65536+pagina*16384+i]=*puntero;
			puntero++;
		}
	}

	//Spectrum +2A
        current_machine_type=11;

        set_machine(NULL);

 //Mapear como estaba
 puerto_32765=old_puerto_32765;

 puerto_8189=0;

	//asignar ram
	mem_page_ram_p2a();

	//asignar rom
	mem_page_rom_p2a();

	//Copiar contenido de buffer en ROM y RAM
	//No nos complicamos la vida, como sabemos que viene lineal las  roms y la ram, volcamos

	for (i=0;i<(64+128)*1024;i++) memoria_spectrum[i]=memoria_buffer[i];

  free(memoria_buffer);



}


//Hace hotswap de cualquier maquina 48 a spectrum 128. Se guarda los 48kb de ram en un buffer, cambia maquina, y vuelca contenido ram
void hotswap_any_machine_to_spec128(void)
{

        //Asignamos 48kb RAM
        z80_byte *memoria_buffer;
        memoria_buffer=malloc(49152);

        if (memoria_buffer==NULL) {
                cpu_panic ("Error. Cannot allocate Machine memory");
        }

        //Copiamos RAM en buffer
        int i;
        for (i=0;i<49152;i++) memoria_buffer[i]=peek_byte_no_time(16384+i);

	//Spectrum 128k
        current_machine_type=6;

        set_machine(NULL);

	//Paginar ROM 1 y RAM 0
	puerto_32765=16;

	//asignar ram
	mem_page_ram_128k();

	//asignar rom
	mem_page_rom_128k();

	//Copiar contenido de buffer en RAM
	for (i=0;i<49152;i++) poke_byte_no_time(16384+i,memoria_buffer[i]);

        free(memoria_buffer);
}



void menu_hotswap_machine(MENU_ITEM_PARAMETERS)
{

                menu_item *array_menu_machine_selection;
                menu_item item_seleccionado;
                int retorno_menu;

                do {

			//casos maquinas 16k, 48k
			if (MACHINE_IS_SPECTRUM_16_48) {
				hotswap_machine_opcion_seleccionada=current_machine_type;
        	                menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX Spectrum 16k",MENU_OPCION_NORMAL,NULL,NULL);
                	        menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);
                        	menu_add_item_menu(array_menu_machine_selection,"Inves Spectrum +",MENU_OPCION_NORMAL,NULL,NULL);
	                        menu_add_item_menu(array_menu_machine_selection,"Microdigital TK90X",MENU_OPCION_NORMAL,NULL,NULL);
        	                menu_add_item_menu(array_menu_machine_selection,"Microdigital TK90X (Spanish)",MENU_OPCION_NORMAL,NULL,NULL);
                	        menu_add_item_menu(array_menu_machine_selection,"Microdigital TK95",MENU_OPCION_NORMAL,NULL,NULL);
				menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum+ 128",MENU_OPCION_NORMAL,NULL,NULL);
			}

			//casos maquinas 128k,+2 (y no +2a)
			if (MACHINE_IS_SPECTRUM_128_P2) {
				hotswap_machine_opcion_seleccionada=current_machine_type-6;
	                        menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX Spectrum+ 128k",MENU_OPCION_NORMAL,NULL,NULL);
        	                menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum+ 128k (Spanish)",MENU_OPCION_NORMAL,NULL,NULL);
                	        menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2",MENU_OPCION_NORMAL,NULL,NULL);
                        	menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2 (French)",MENU_OPCION_NORMAL,NULL,NULL);
	                        menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2 (Spanish)",MENU_OPCION_NORMAL,NULL,NULL);
													menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2A (ROM v4.0)",MENU_OPCION_NORMAL,NULL,NULL);
				menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);
			}

			//maquinas p2a
			if (MACHINE_IS_SPECTRUM_P2A_P3) {
				hotswap_machine_opcion_seleccionada=current_machine_type-11;
	                        menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX Spectrum +2A (ROM v4.0)",MENU_OPCION_NORMAL,NULL,NULL);
        	                menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2A (ROM v4.1)",MENU_OPCION_NORMAL,NULL,NULL);
                	        menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum +2A (Spanish)",MENU_OPCION_NORMAL,NULL,NULL);
													menu_add_item_menu(array_menu_machine_selection,"Spectrum 128k",MENU_OPCION_NORMAL,NULL,NULL);
				menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);
			}

			//maquinas cpc
			if (MACHINE_IS_CPC) {
				hotswap_machine_opcion_seleccionada=current_machine_type-MACHINE_ID_CPC_464;
	                        menu_add_item_menu_inicial(&array_menu_machine_selection,"Amstrad CPC 464",MENU_OPCION_NORMAL,NULL,NULL);
        	                menu_add_item_menu(array_menu_machine_selection,"Amstrad CPC 4128",MENU_OPCION_NORMAL,NULL,NULL);
			}			

                        //maquinas zxuno
                        if (MACHINE_IS_ZXUNO) {
                                hotswap_machine_opcion_seleccionada=current_machine_type-14;
                                menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX Spectrum +2A (ROM v4.0)",MENU_OPCION_NORMAL,NULL,NULL);

	                        menu_add_item_menu_tooltip(array_menu_machine_selection,"The final machine type is "
							"Spectrum +2A (ROM v4.0) but the data ROM really comes from ZX-Uno");
	                        menu_add_item_menu_ayuda(array_menu_machine_selection,"The final machine type is "
							"Spectrum +2A (ROM v4.0) but the data ROM really comes from ZX-Uno");


				menu_add_item_menu(array_menu_machine_selection,"Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);


                        }


			//maquinas zx80, zx81
			if (MACHINE_IS_ZX8081) {
				hotswap_machine_opcion_seleccionada=current_machine_type-120;
				menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX80",MENU_OPCION_NORMAL,NULL,NULL);
	            menu_add_item_menu(array_menu_machine_selection,"ZX81",MENU_OPCION_NORMAL,NULL,NULL);
			}

                 

			//maquinas chloe
			if (MACHINE_IS_CHLOE) {
				if (MACHINE_IS_CHLOE_140SE) hotswap_machine_opcion_seleccionada=0;
				if (MACHINE_IS_CHLOE_280SE) hotswap_machine_opcion_seleccionada=1;

				menu_add_item_menu_inicial(&array_menu_machine_selection,"Chloe 140SE",MENU_OPCION_NORMAL,NULL,NULL);
				menu_add_item_menu(array_menu_machine_selection,"Chloe 280SE",MENU_OPCION_NORMAL,NULL,NULL);
				menu_add_item_menu(array_menu_machine_selection,"ZX Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);


			}

			//Diferentes maquinas que solo pueden saltar a spectrum 48k
			if (MACHINE_IS_PRISM || MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_TBBLUE || MACHINE_IS_CHROME || MACHINE_IS_ZXEVO) {
								hotswap_machine_opcion_seleccionada=0;
								menu_add_item_menu_inicial(&array_menu_machine_selection,"ZX Spectrum 48k",MENU_OPCION_NORMAL,NULL,NULL);
			}

			


                        menu_add_item_menu(array_menu_machine_selection,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                        //menu_add_item_menu(array_menu_machine_selection,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
			menu_add_ESC_item(array_menu_machine_selection);

			retorno_menu=menu_dibuja_menu(&hotswap_machine_opcion_seleccionada,&item_seleccionado,array_menu_machine_selection,"Hotswap Machine" );

	                


                        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
				//casos maquinas 16k, 48k
	                        if (MACHINE_IS_SPECTRUM_16_48) {
					//Caso especial cuando se cambia entre maquina Inves, porque la asignacion de memoria es diferente
					if (MACHINE_IS_INVES || hotswap_machine_opcion_seleccionada==2) {
						//si misma maquina inves origen o destino, no hacer nada

						//Cambiamos de Inves a otra
						if (MACHINE_IS_INVES && hotswap_machine_opcion_seleccionada!=2) {
							//Asignamos 64kb RAM
							z80_byte *memoria_spectrum_final;
						        memoria_spectrum_final=malloc(65536);

						        if (memoria_spectrum_final==NULL) {
						                cpu_panic ("Error. Cannot allocate Machine memory");
						        }

							//Copiamos ROM y RAM a destino
							int i;
							//ROM
							for (i=0;i<16384;i++) memoria_spectrum_final[i]=memoria_spectrum[65536+i];

							//RAM
							for (i=16384;i<65536;i++) memoria_spectrum_final[i]=memoria_spectrum[i];

							free(memoria_spectrum);
							memoria_spectrum=memoria_spectrum_final;

						}
						if (!(MACHINE_IS_INVES) && hotswap_machine_opcion_seleccionada==2) {
							//Asignamos 80 kb RAM
                                                        z80_byte *memoria_spectrum_final;
                                                        memoria_spectrum_final=malloc(65536+16384);

                                                        if (memoria_spectrum_final==NULL) {
                                                                cpu_panic ("Error. Cannot allocate Machine memory");
                                                        }

							//Copiamos ROM y RAM a destino
							int i;
							//ROM
                                                        for (i=0;i<16384;i++) memoria_spectrum_final[65536+i]=memoria_spectrum[i];

                                                        //RAM
                                                        for (i=16384;i<65536;i++) memoria_spectrum_final[i]=memoria_spectrum[i];

                                                        free(memoria_spectrum);
                                                        memoria_spectrum=memoria_spectrum_final;

							//Establecemos valores de low ram inves
							random_ram_inves(memoria_spectrum,16384);


						}
					}

					//Hotswap a 128
          if (hotswap_machine_opcion_seleccionada==6) {
						debug_printf (VERBOSE_INFO,"Hotswapping to Spectrum 128");
                                                hotswap_any_machine_to_spec128();
                                        }

					else {
						current_machine_type=hotswap_machine_opcion_seleccionada;
				        	set_machine_params();
				        	post_set_machine(NULL);
					}
                                        //Y salimos de todos los menus
                                        salir_todos_menus=1;
					return; //Para evitar saltar a otro if
				}

				if (MACHINE_IS_SPECTRUM_128_P2) {
					if (hotswap_machine_opcion_seleccionada==6) {
						hotswap_any_machine_to_spec48();
					}

					else if (hotswap_machine_opcion_seleccionada==5) {
						hotswap_128_to_p2a();
						menu_warn_message("Note that ROM data are the previous data coming from 128K");
					}

					else {
						current_machine_type=hotswap_machine_opcion_seleccionada+6;
        	                                set_machine_params();
                	                        post_set_machine(NULL);
					}
                        	        //Y salimos de todos los menus
                                	salir_todos_menus=1;
					return; //Para evitar saltar a otro if
                                }

				if (MACHINE_IS_SPECTRUM_P2A_P3) {
                                        if (hotswap_machine_opcion_seleccionada==4) {
                                                hotswap_any_machine_to_spec48();
                                        }
					else if (hotswap_machine_opcion_seleccionada==3) {
						//De +2A a 128k
						hotswap_p2a_to_128();
						menu_warn_message("Note that ROM data are the previous data coming from +2A");
					}
					else {
	                                        current_machine_type=hotswap_machine_opcion_seleccionada+11;
        	                                set_machine_params();
                	                        post_set_machine(NULL);
					}
                                        //Y salimos de todos los menus
                                        salir_todos_menus=1;
					return; //Para evitar saltar a otro if
                                }

				if (MACHINE_IS_CPC) {
					current_machine_type=MACHINE_ID_CPC_464+hotswap_machine_opcion_seleccionada;
					set_machine_params();
                	post_set_machine(NULL);
                    //Y salimos de todos los menus
                    salir_todos_menus=1;
					return; //Para evitar saltar a otro if

				}

				if (MACHINE_IS_ZX8081) {
					current_machine_type=hotswap_machine_opcion_seleccionada+120;
					set_machine_params();
                                        post_set_machine(NULL);

					//ajustar algunos registros
					if (MACHINE_IS_ZX80) reg_i=0x0E;

					if (MACHINE_IS_ZX81) {
						reg_i=0x1E;
						nmi_generator_active.v=0;
					}

                                        //Y salimos de todos los menus
                                        salir_todos_menus=1;
					return; //Para evitar saltar a otro if
                                }

				if (MACHINE_IS_ZXUNO) {
                                        if (hotswap_machine_opcion_seleccionada==1) {
                                                hotswap_any_machine_to_spec48();
                                        }

					else {
						current_machine_type=hotswap_machine_opcion_seleccionada+11;
						set_machine_params();

						//no cargar rom, la rom sera la que haya activa en las paginas del zxuno
						post_set_machine_no_rom_load();

						//dejamos toda la memoria que hay asignada del zx-uno, solo que
						//reasignamos los punteros de paginacion del +2a

						hotswap_zxuno_to_p2as_set_pages();


						//Si teniamos el divmmc activo. Llamar a esto manualmente, no a todo divmmc_enable(),
						//pues cargaria por ejemplo el firmware esxdos de disco, y mejor conservamos el mismo firmware
						//que haya cargado el ZX-Uno
						if (diviface_enabled.v) {
							diviface_set_peek_poke_functions();
							//diviface_paginacion_manual_activa.v=0;
							diviface_control_register&=(255-128);
							diviface_paginacion_automatica_activa.v=0;
						}

						menu_warn_message("Note that ROM data are the previous data coming from ZX-Uno");

					}

					salir_todos_menus=1;
					return; //Para evitar saltar a otro if
				}



				if (MACHINE_IS_CHLOE) {
                                        if (hotswap_machine_opcion_seleccionada==2) {
                                                hotswap_any_machine_to_spec48();
                                        }

					else {
						current_machine_type=hotswap_machine_opcion_seleccionada+15;
        	                                set_machine_params();
                	                        post_set_machine(NULL);

					}
                    //Y salimos de todos los menus
                    salir_todos_menus=1;
					return; //Para evitar saltar a otro if
				}

				if (MACHINE_IS_PRISM || MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_TBBLUE || MACHINE_IS_CHROME || MACHINE_IS_ZXEVO) {
					hotswap_any_machine_to_spec48();
                    salir_todos_menus=1;
					return; //Para evitar saltar a otro if
                }

				


            }

		} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int custom_machine_type=0;
char custom_romfile[PATH_MAX]="";

static char *custom_machine_names[]={
                "Spectrum 16k",
                "Spectrum 48k",

		"TK90X/95",

                "Spectrum 128k/+2",
                "Spectrum +2A",
		"ZX-Uno",

		"Chloe 140 SE",
		"Chloe 280 SE",
		"Timex TS 2068",
		"Prism 512",

                "ZX80",
                "ZX81",
		"Jupiter Ace",
		"Z88",
		"Amstrad CPC 464",
		"Sam Coupe",
		"QL"
};


void menu_custom_machine_romfile(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="rom";
        filtros[1]=0;


        if (menu_filesel("Select ROM File",filtros,custom_romfile)==1) {
                //

        }

        else {
                custom_romfile[0]=0;
        }
}




void menu_custom_machine_change(MENU_ITEM_PARAMETERS)
{
	custom_machine_type++;
	if (custom_machine_type==17) custom_machine_type=0;
}

void menu_custom_machine_run(MENU_ITEM_PARAMETERS)
{

	int minimum_size=0;
	int next_machine_type;

	switch (custom_machine_type) {

		case 0:
			next_machine_type=0;
		break;

		case 1:
			next_machine_type=1;
		break;

		case 2:
			next_machine_type=3;
		break;


		case 3:
			next_machine_type=6;
		break;

		case 4:
			next_machine_type=11;
		break;

		case 5:
			//ZX-Uno
			next_machine_type=14;
		break;

/*

                "Chloe 140 SE",
                "Chloe 280 SE",
                "Timex TS 2068",
                "Prism",
                "Amstrad CPC 464",
*/

/*
15=Chloe 140SE
16=Chloe 280SE
17=Timex TS2068
18=Prism
40=amstrad cpc464
*/

		case 6:
			//Chloe 140SE
			next_machine_type=15;
		break;

		case 7:
			//Chloe 280SE
			next_machine_type=16;
		break;

		case 8:
			//Timex TS2068
			next_machine_type=17;
		break;

		case 9:
			//Prism
			next_machine_type=18;
		break;



		case 10:
			//ZX-80
			next_machine_type=120;
		break;

		case 11:
			//ZX-81
			next_machine_type=121;
		break;

		case 12:
			//ACE
			next_machine_type=122;
		break;

		case 13:
			//Z88
			next_machine_type=130;
		break;

		case 14:
			//Amstrad CPC 464
			next_machine_type=140;
		break;

		case 15:
			//Sam Coupe
			next_machine_type=150;
		break;

		case 16:
			//QL
			next_machine_type=MACHINE_ID_QL_STANDARD;
		break;

		default:
			cpu_panic("Custom machine type unknown");
			//este return no hace falta, solo es para silenciar los warning de variable next_machine_type no inicializada
			return;
		break;

	}

	//Ver tamanyo archivo rom
	minimum_size=get_rom_size(next_machine_type);

	struct stat buf_stat;


                if (stat(custom_romfile, &buf_stat)!=0) {
                        debug_printf(VERBOSE_ERR,"Unable to find rom file %s",custom_romfile);
			return;
                }

                else {
                        //Tamaño del archivo es >=minimum_size
                        if (buf_stat.st_size<minimum_size) {
				debug_printf(VERBOSE_ERR,"ROM file must be at least %d bytes length",minimum_size);
                                return;
                        }
                }

	current_machine_type=next_machine_type;
	set_machine(custom_romfile);
	cold_start_cpu_registers();
	reset_cpu();

	//Y salimos de todos los menus
	salir_todos_menus=1;

}

void menu_custom_machine(MENU_ITEM_PARAMETERS)
{
   menu_item *array_menu_custom_machine;
        menu_item item_seleccionado;
        int retorno_menu;

	//Tipo de maquina: 16k,48k,128k/+2,+2a,zx80,zx81,ace,z88
	//Archivo ROM

	//sprintf(custom_romfile,"%s","alternaterom_plus2b.rom");

        do {
                menu_add_item_menu_inicial_format(&array_menu_custom_machine,MENU_OPCION_NORMAL,menu_custom_machine_change,NULL,"Machine Type: %s",custom_machine_names[custom_machine_type] );

		char string_romfile_shown[16];
                menu_tape_settings_trunc_name(custom_romfile,string_romfile_shown,16);

                menu_add_item_menu_format(array_menu_custom_machine,MENU_OPCION_NORMAL,menu_custom_machine_romfile,NULL,"Rom File: %s",string_romfile_shown);

		menu_add_item_menu_format(array_menu_custom_machine,MENU_OPCION_NORMAL,menu_custom_machine_run,NULL,"Run machine");



                menu_add_item_menu(array_menu_custom_machine,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_custom_machine);

                retorno_menu=menu_dibuja_menu(&custom_machine_opcion_seleccionada,&item_seleccionado,array_menu_custom_machine,"Custom Machine" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

int menu_hotswap_machine_cond(void) {

	//Retornar ok solo para determinadas maquinas
	if (MACHINE_IS_SPECTRUM_16_48)  return 1;
	if (MACHINE_IS_SPECTRUM_128_P2)  return 1;
	if (MACHINE_IS_SPECTRUM_P2A_P3)  return 1;
    if (MACHINE_IS_ZXUNO)  return 1;
	if (MACHINE_IS_ZX8081)  return 1;
	if (MACHINE_IS_CHLOE)  return 1;
	if (MACHINE_IS_PRISM)  return 1;
	if (MACHINE_IS_TIMEX_TS2068)  return 1;
	if (MACHINE_IS_TBBLUE)  return 1;
	if (MACHINE_IS_CHROME)  return 1;
	if (MACHINE_IS_ZXEVO)  return 1;
	if (MACHINE_IS_CPC)  return 1;


	return 0;
}

void menu_machine_selection_for_manufacturer(int fabricante)
{
	int i;
	int *maquinas;

	maquinas=return_maquinas_fabricante(fabricante);

	//cambiar linea seleccionada a maquina en cuestion
	int indice_maquina=return_machine_position(maquinas,current_machine_type);
	if (indice_maquina!=255) machine_selection_por_fabricante_opcion_seleccionada=indice_maquina;
	else {
		//Maquina no es de este menu. Resetear linea a 0
		machine_selection_por_fabricante_opcion_seleccionada=0;
	}

	char *nombre_maquina;

	int total_maquinas;

	int m;

        //Seleccion por fabricante
                menu_item *array_menu_machine_selection_por_fabricante;
                menu_item item_seleccionado;
                int retorno_menu;
	do {

		for (i=0;maquinas[i]!=255;i++) {
			m=maquinas[i];
			//printf ("%d\n",m);
			nombre_maquina=get_machine_name(m);
			//printf ("%d %s\n",m,nombre_maquina);


			if (i==0) {
				//Primer fabricante
	                        menu_add_item_menu_inicial_format(&array_menu_machine_selection_por_fabricante,MENU_OPCION_NORMAL,NULL,NULL,"%s",nombre_maquina);

			}

			else {
				menu_add_item_menu_format(array_menu_machine_selection_por_fabricante,MENU_OPCION_NORMAL,NULL,NULL,"%s",nombre_maquina);
			}


		}

		total_maquinas=i;




      menu_add_item_menu(array_menu_machine_selection_por_fabricante,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                        //menu_add_item_menu(array_menu_machine_selection_por_fabricante,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                        menu_add_ESC_item(array_menu_machine_selection_por_fabricante);



                        retorno_menu=menu_dibuja_menu(&machine_selection_por_fabricante_opcion_seleccionada,&item_seleccionado,array_menu_machine_selection_por_fabricante,"Select machine" );

                        //printf ("Opcion seleccionada: %d\n",machine_selection_por_fabricante_opcion_seleccionada);

                        

                        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

                                if (machine_selection_por_fabricante_opcion_seleccionada>=0 && machine_selection_por_fabricante_opcion_seleccionada<=total_maquinas) {



					//printf ("Seleccion opcion=%d\n",machine_selection_por_fabricante_opcion_seleccionada);
					int id_maquina=maquinas[machine_selection_por_fabricante_opcion_seleccionada];
					//printf ("Maquina= %d %s\n",id_maquina, get_machine_name(id_maquina) );

					current_machine_type=id_maquina;

				     set_machine(NULL);
                                        cold_start_cpu_registers();
                                        reset_cpu();

                                        //desactivar autoload
                                        //noautoload.v=1;
                                        //initial_tap_load.v=0;


                                        //expulsamos cintas
                                        eject_tape_load();
                                        eject_tape_save();

                                        //Y salimos de todos los menus
                                        salir_todos_menus=1;


                              }
                                //llamamos por valor de funcion
                                if (item_seleccionado.menu_funcion!=NULL) {
                                        //printf ("actuamos por funcion\n");
                                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                }


         
                        }

                //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);
                } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);






}

void menu_machine_selection(MENU_ITEM_PARAMETERS)
{

	//Seleccion por fabricante
                menu_item *array_menu_machine_selection;
                menu_item item_seleccionado;
                int retorno_menu;

//return_fabricante_maquina
		//Establecemos linea menu segun fabricante activo
		machine_selection_opcion_seleccionada=return_fabricante_maquina(current_machine_type);

                do {

			//Primer fabricante
                        menu_add_item_menu_inicial_format(&array_menu_machine_selection,MENU_OPCION_NORMAL,NULL,NULL,"%s",array_fabricantes_hotkey[0]);
			menu_add_item_menu_shortcut(array_menu_machine_selection,array_fabricantes_hotkey_letra[0]);

		//Siguientes fabricantes
			int i;
			for (i=1;i<TOTAL_FABRICANTES;i++) {
				menu_add_item_menu_format(array_menu_machine_selection,MENU_OPCION_NORMAL,NULL,NULL,"%s",array_fabricantes_hotkey[i]);
				z80_byte letra=array_fabricantes_hotkey_letra[i];
				if (letra!=' ') menu_add_item_menu_shortcut(array_menu_machine_selection,letra);
			}


                       menu_add_item_menu(array_menu_machine_selection,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                        //Hotswap de Z88 o Jupiter Ace o CHLOE no existe
                        menu_add_item_menu(array_menu_machine_selection,"~~Hotswap machine",MENU_OPCION_NORMAL,menu_hotswap_machine,menu_hotswap_machine_cond);
                        menu_add_item_menu_shortcut(array_menu_machine_selection,'h');
                        menu_add_item_menu_tooltip(array_menu_machine_selection,"Change machine type without resetting");
                        menu_add_item_menu_ayuda(array_menu_machine_selection,"Change machine type without resetting.");

                        menu_add_item_menu(array_menu_machine_selection,"Cust~~om machine",MENU_OPCION_NORMAL,menu_custom_machine,NULL);
                        menu_add_item_menu_shortcut(array_menu_machine_selection,'o');
                        menu_add_item_menu_tooltip(array_menu_machine_selection,"Specify custom machine type & ROM");
                        menu_add_item_menu_ayuda(array_menu_machine_selection,"Specify custom machine type & ROM");

                        menu_add_item_menu(array_menu_machine_selection,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                        //menu_add_item_menu(array_menu_machine_selection,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                        menu_add_ESC_item(array_menu_machine_selection);



                        retorno_menu=menu_dibuja_menu(&machine_selection_opcion_seleccionada,&item_seleccionado,array_menu_machine_selection,"Select manufacturer" );

                        //printf ("Opcion seleccionada: %d\n",machine_selection_opcion_seleccionada);

                        

                        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

                                if (machine_selection_opcion_seleccionada>=0 && machine_selection_opcion_seleccionada<=TOTAL_FABRICANTES) {

					//printf ("Seleccionado fabricante %s\n",array_fabricantes[machine_selection_opcion_seleccionada]);

                                        //int last_machine_type=machine_type;


					menu_machine_selection_for_manufacturer(machine_selection_opcion_seleccionada);



			      }
                                //llamamos por valor de funcion
                                if (item_seleccionado.menu_funcion!=NULL) {
                                        //printf ("actuamos por funcion\n");
                                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                        
                                }


                             
                        }

                //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);
                } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


}





void menu_warn_message(char *texto)
{
	menu_generic_message_warn("Warning",texto);

}

void menu_error_message(char *texto)
{
	menu_generic_message_warn("ERROR",texto);

}

//Similar a snprintf
void menu_generic_message_aux_copia(char *origen,char *destino, int longitud)
{
	while (longitud) {
		*destino=*origen;
		origen++;
		destino++;
		longitud--;
	}
}

//Aplicar filtros para caracteres extranyos y cortar linea en saltos de linea
int menu_generic_message_aux_filter(char *texto,int inicio, int final)
{
	//int copia_inicio=inicio;

	unsigned char caracter;

	int prefijo_utf=0;

        while (inicio!=final) {
		caracter=texto[inicio];

                if (caracter=='\n' || caracter=='\r') {
			//printf ("detectado salto de linea en posicion %d\n",inicio);
			texto[inicio]=' ';
			return inicio+1;
		}

		//TAB. Lo cambiamos por espacio
		else if (caracter==9) {
			texto[inicio]=' ';
		}

		else if (menu_es_prefijo_utf(caracter)) {
			//Si era prefijo utf, saltar
			prefijo_utf=1;
		}

		//Y si venia de prefijo utf, saltar ese caracter
		else if (prefijo_utf) {
			prefijo_utf=0;
		}

		else if ( !(si_valid_char(caracter)) ) {
			//printf ("detectado caracter extranyo %d en posicion %d\n",caracter,inicio);

			texto[inicio]='?';
		}

                inicio++;
        }

        return final;

}


//Cortar las lineas, si se puede, por espacio entre palabras
int menu_generic_message_aux_wordwrap(char *texto,int inicio, int final)
{

	int copia_final=final;

	//ya acaba en espacio, volver
	//if (texto[final]==' ') return final;

	while (final!=inicio) {
		if (texto[final]==' ' || texto[final]=='\n' || texto[final]=='\r') return final+1;
		final--;
	}

	return copia_final;
}

int menu_generic_message_cursor_arriba(int primera_linea)
{
	if (primera_linea>0) primera_linea--;
	return primera_linea;
}

int menu_generic_message_cursor_arriba_mostrar_cursor(int primera_linea,int mostrar_cursor,int *linea_cursor)
{
                                     if (mostrar_cursor) {
                                                        int off=0;
                                                        //no limitar primera linea if (primera_linea) off++;
                                                        if (*linea_cursor>off) (*linea_cursor)--;
                                                        else primera_linea=menu_generic_message_cursor_arriba(primera_linea);
                                                }
                                                else {
                                                        primera_linea=menu_generic_message_cursor_arriba(primera_linea);
                                                }

	return primera_linea;
}



int menu_generic_message_cursor_abajo (int primera_linea,int alto_ventana,int indice_linea)
{


	//if (primera_linea<indice_linea-2) primera_linea++;
	if (primera_linea+alto_ventana-2<indice_linea) primera_linea++;
	return primera_linea;


}


int menu_generic_message_cursor_abajo_mostrar_cursor(int primera_linea,int alto_ventana,int indice_linea,int mostrar_cursor,int *linea_cursor)
{
                                                if (mostrar_cursor) {
                                                        if (*linea_cursor<alto_ventana-3) (*linea_cursor)++;
                                                        else primera_linea=menu_generic_message_cursor_abajo(primera_linea,alto_ventana,indice_linea);
                                                }
                                                else {
                                                        primera_linea=menu_generic_message_cursor_abajo(primera_linea,alto_ventana,indice_linea);
                                                }

	return primera_linea;
}


//int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea,int mostrar_cursor,int linea_cursor)
int menu_generic_message_final_abajo(int primera_linea,int alto_ventana,int indice_linea)
{
	/*if (mostrar_cursor) {
		if (linea_cursor<alto_ventana-3) return 1;
	}

	else*/ if (primera_linea+alto_ventana-2<indice_linea) return 1;

	return 0;
}


//dibuja ventana simple, una sola linea de texto interior, sin esperar tecla
void menu_simple_ventana(char *titulo,char *texto)
{


	unsigned int ancho_ventana=strlen(titulo);
	if (strlen(texto)>ancho_ventana) ancho_ventana=strlen(texto);

	int alto_ventana=3;

	ancho_ventana +=2;

	if (ancho_ventana>ZXVISION_MAX_ANCHO_VENTANA) {
		cpu_panic("window width too big");
	}

        int xventana=menu_center_x()-ancho_ventana/2;
        int yventana=menu_center_y()-alto_ventana/2;


        menu_dibuja_ventana(xventana,yventana,ancho_ventana,alto_ventana,titulo);

	menu_escribe_linea_opcion(0,-1,1,texto);

}

void menu_copy_clipboard(char *texto)
{

	//Si puntero no NULL, liberamos clipboard anterior
	if (menu_clipboard_pointer!=NULL) {
		debug_printf(VERBOSE_INFO,"Freeing previous clipboard memory");
		free(menu_clipboard_pointer);
		menu_clipboard_pointer=NULL;
	}

	//Si puntero NULL, asignamos memoria
	if (menu_clipboard_pointer==NULL) {
		menu_clipboard_size=strlen(texto);
		debug_printf(VERBOSE_INFO,"Allocating %d bytes to clipboard",menu_clipboard_size+1);
		menu_clipboard_pointer=malloc(menu_clipboard_size+1); //+1 del 0 final
		if (menu_clipboard_pointer==NULL) {
			debug_printf(VERBOSE_ERR,"Error allocating clipboard memory");
			return;
		}
		strcpy((char *)menu_clipboard_pointer,texto);
	}

	
}

void menu_paste_clipboard_to_file(char *destination_file)
{
	//util_file_save(destination_file,menu_clipboard_pointer,menu_clipboard_size);
	//extern void util_file_save(char *filename,z80_byte *puntero, long int tamanyo);
	//extern void util_save_file(z80_byte *origin, long int tamanyo_origen, char *destination_file);

	util_save_file(menu_clipboard_pointer,menu_clipboard_size,destination_file);
}


//Funcion generica para guardar un archivo de texto a disco
//Supondra que es de texto y por tanto pone filtro de "*.txt"
//Ademas el tamaño del archivo a guardar se determina por el caracter 0 final
//de momento funcion no usada
/*void menu_save_text_to_file(char *puntero_memoria,char *titulo_ventana)
{
	char file_save[PATH_MAX];

	char *filtros[2];

	filtros[0]="txt";
    filtros[1]=0;

    int ret;

	ret=menu_filesel(titulo_ventana,filtros,file_save);

	if (ret==1) {

		//Ver si archivo existe y preguntar
		if (si_existe_archivo(file_save)) {

			if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

        }

		int file_size=strlen(puntero_memoria);

		util_save_file((z80_byte *)puntero_memoria,file_size,file_save);

		menu_generic_message_splash(titulo_ventana,"OK File saved");

		menu_espera_no_tecla();


	}
}*/


//Cortar linea en dos, pero teniendo en cuenta que solo puede cortar por los espacios
void menu_util_cut_line_at_spaces(int posicion_corte, char *texto,char *linea1, char *linea2)
{

	int indice_texto=0;
	int ultimo_indice_texto=0;
	int longitud=strlen(texto);

	indice_texto+=posicion_corte;


		//Si longitud es menor
		if (indice_texto>=longitud) {
			strcpy(linea1,texto);
			linea2[0]=0;
			return;
		}


		//Si no, miramos si hay que separar por espacios
		else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

		//Separamos por salto de linea, filtramos caracteres extranyos
		//indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);


		//snprintf(buffer_lineas[indice_linea],longitud_texto,&texto[ultimo_indice_texto]);
		//printf ("indice texto: %d\n",indice_texto);

		menu_generic_message_aux_copia(texto,linea1,indice_texto);
		linea1[indice_texto]=0;

		//copiar texto
		int longitud_texto=longitud-indice_texto;

		//printf ("indice texto: %d longitud: %d\n",indice_texto,longitud_texto);

		menu_generic_message_aux_copia(&texto[indice_texto],linea2,longitud_texto);
		linea2[longitud_texto]=0;

}

//estilo_invertido:
//0: no invertir colores
//1: invertir color boton arriba
//2: invertir color boton abajo
//3: invertir color barra
void menu_ventana_draw_horizontal_perc_bar(int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
		if (porcentaje<0) porcentaje=0;
		if (porcentaje>100) porcentaje=100;

		// mostrar * abajo para indicar donde estamos en porcentaje
		int xbase=x+2;

		int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
		int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
		int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

		int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
		int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
		int papel_barra=ESTILO_GUI_PAPEL_NORMAL;	

		int tinta_aux;

		switch (estilo_invertido) {
			case 1:
				tinta_aux=tinta_boton_arriba;
				tinta_boton_arriba=papel_boton_arriba;
				papel_boton_arriba=tinta_aux;
			break;

			case 2:
				tinta_aux=tinta_boton_abajo;
				tinta_boton_abajo=papel_boton_abajo;
				papel_boton_abajo=tinta_aux;
			break;	

			case 3:
				tinta_aux=tinta_barra;
				tinta_barra=papel_barra;
				papel_barra=tinta_aux;
			break;

		}			


			//mostrar cursores izquierda y derecha
		putchar_menu_overlay(xbase-1,y+alto-1,'<',tinta_boton_arriba,papel_boton_arriba);
		putchar_menu_overlay(xbase+ancho-3,y+alto-1,'>',tinta_boton_abajo,papel_boton_abajo);

		//mostrar linea horizontal para indicar que es zona de porcentaje
		z80_byte caracter_barra='-';
		if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';

		int i;
		for (i=0;i<ancho-3;i++) putchar_menu_overlay(xbase+i,y+alto-1,caracter_barra,tinta_barra,papel_barra);	

		
		int sumarancho=((ancho-4)*porcentaje)/100;

		putchar_menu_overlay(xbase+sumarancho,y+alto-1,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}

//estilo_invertido:
//0: no invertir colores
//1: invertir color boton arriba
//2: invertir color boton abajo
//3: invertir color barra
void menu_ventana_draw_vertical_perc_bar(int x,int y,int ancho,int alto,int porcentaje,int estilo_invertido)
{
		if (porcentaje<0) porcentaje=0;
		if (porcentaje>100) porcentaje=100;

		// mostrar * a la derecha para indicar donde estamos en porcentaje
		int ybase=y+2;

		int tinta_boton_arriba=ESTILO_GUI_TINTA_NORMAL;
		int tinta_boton_abajo=ESTILO_GUI_TINTA_NORMAL;
		int tinta_barra=ESTILO_GUI_TINTA_NORMAL;

		int papel_boton_arriba=ESTILO_GUI_PAPEL_NORMAL;
		int papel_boton_abajo=ESTILO_GUI_PAPEL_NORMAL;
		int papel_barra=ESTILO_GUI_PAPEL_NORMAL;	

		int tinta_aux;

		switch (estilo_invertido) {
			case 1:
				tinta_aux=tinta_boton_arriba;
				tinta_boton_arriba=papel_boton_arriba;
				papel_boton_arriba=tinta_aux;
			break;

			case 2:
				tinta_aux=tinta_boton_abajo;
				tinta_boton_abajo=papel_boton_abajo;
				papel_boton_abajo=tinta_aux;
			break;	

			case 3:
				tinta_aux=tinta_barra;
				tinta_barra=papel_barra;
				papel_barra=tinta_aux;
			break;

		}	


		//mostrar cursores arriba y abajo
		putchar_menu_overlay(x+ancho-1,ybase-1,'^',tinta_boton_arriba,papel_boton_arriba);
		putchar_menu_overlay(x+ancho-1,ybase+alto-3,'v',tinta_boton_abajo,papel_boton_abajo);

		//mostrar linea vertical para indicar que es zona de porcentaje
		z80_byte caracter_barra='|';
		if (menu_hide_vertical_percentaje_bar.v) caracter_barra=' ';		

		//mostrar linea vertical para indicar que es zona de porcentaje
		int i;
		for (i=0;i<alto-3;i++) 	putchar_menu_overlay(x+ancho-1,ybase+i,caracter_barra,tinta_barra,papel_barra);	
		
		
		int sumaralto=((alto-4)*porcentaje)/100;
		putchar_menu_overlay(x+ancho-1,ybase+sumaralto,'*',ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL);
}



int splash_zesarux_logo_paso=0;
int splash_zesarux_logo_active=0;

void reset_splash_zesarux_logo(void)
{
	splash_zesarux_logo_active=0;
}



//Esta rutina estaba originalmente en screen.c pero dado que se ha modificado para usar rutinas auxiliares de aqui, mejor que este aqui
void screen_print_splash_text(int y,z80_byte tinta,z80_byte papel,char *texto)
{

        //Si no hay driver video
        if (scr_putpixel==NULL || scr_putpixel_zoom==NULL) return;


  if (menu_abierto==0 && screen_show_splash_texts.v==1) {
                cls_menu_overlay();

                int x;

#define MAX_LINEAS_SPLASH 24
        const int max_ancho_texto=31;
	//al trocear, si hay un espacio despues, se agrega, y por tanto puede haber linea de 31+1=32 caracteres

        //texto que contiene cada linea con ajuste de palabra. Al trocear las lineas aumentan
	//33 es ancho total linea(32)+1
        char buffer_lineas[MAX_LINEAS_SPLASH][33];



        int indice_linea=0;
        int indice_texto=0;
        int ultimo_indice_texto=0;
        int longitud=strlen(texto);

        //int indice_segunda_linea;


        do {
                indice_texto+=max_ancho_texto;

                //Controlar final de texto
                if (indice_texto>=longitud) indice_texto=longitud;

                //Si no, miramos si hay que separar por espacios
                else indice_texto=menu_generic_message_aux_wordwrap(texto,ultimo_indice_texto,indice_texto);

                //Separamos por salto de linea, filtramos caracteres extranyos
                indice_texto=menu_generic_message_aux_filter(texto,ultimo_indice_texto,indice_texto);

                //copiar texto
                int longitud_texto=indice_texto-ultimo_indice_texto;



                menu_generic_message_aux_copia(&texto[ultimo_indice_texto],buffer_lineas[indice_linea],longitud_texto);
                buffer_lineas[indice_linea++][longitud_texto]=0;
                //printf ("copiado %d caracteres desde %d hasta %d: %s\n",longitud_texto,ultimo_indice_texto,indice_texto,buffer_lineas[indice_linea-1]);


        //printf ("texto indice: %d : longitud: %d: -%s-\n",indice_linea-1,longitud_texto,buffer_lineas[indice_linea-1]);
		//printf ("indice_linea: %d indice_linea+y: %d MAX: %d\n",indice_linea,indice_linea+y,MAX_LINEAS_SPLASH);

                if (indice_linea==MAX_LINEAS_SPLASH) {
                        //cpu_panic("Max lines on menu_generic_message reached");
                        debug_printf(VERBOSE_INFO,"Max lines on screen_print_splash_text reached (%d)",MAX_LINEAS_SPLASH);
                        //finalizamos bucle
                        indice_texto=longitud;
                }

                ultimo_indice_texto=indice_texto;
                //printf ("ultimo indice: %d %c\n",ultimo_indice_texto,texto[ultimo_indice_texto]);

        } while (indice_texto<longitud);

	int i;
	for (i=0;i<indice_linea && y<24;i++) {
		debug_printf (VERBOSE_DEBUG,"line %d y: %d length: %d contents: -%s-",i,y,strlen(buffer_lineas[i]),buffer_lineas[i]);
		x=menu_center_x()-strlen(buffer_lineas[i])/2;
		if (x<0) x=0;
		menu_escribe_texto(x,y,tinta,papel,buffer_lineas[i]);
		y++;
	}


        set_menu_overlay_function(normal_overlay_texto_menu);
        menu_splash_text_active.v=1;
        menu_splash_segundos=5;

				//no queremos que reaparezca el logo, por si no había llegado al final de splash. Improbable? Si. Pero mejor ser precavidos
				reset_splash_zesarux_logo();
   }

}



//Esta rutina estaba originalmente en screen.c pero dado que se ha modificado para usar rutinas auxiliares de aqui, mejor que este aqui
void screen_print_splash_text_center(z80_byte tinta,z80_byte papel,char *texto)
{
	screen_print_splash_text(menu_center_y(),tinta,papel,texto);
}

//retorna 1 si y
//otra cosa, 0
int menu_confirm_yesno_texto(char *texto_ventana,char *texto_interior)
{

	//Si se fuerza siempre yes
	if (force_confirm_yes.v) return 1;


	//printf ("confirm\n");

        //En caso de stdout, es mas simple, mostrar texto y esperar tecla
        if (!strcmp(scr_driver_name,"stdout")) {
		char buffer_texto[256];
                printf ("%s\n%s\n",texto_ventana,texto_interior);

		scrstdout_menu_print_speech_macro(texto_ventana);
		scrstdout_menu_print_speech_macro(texto_interior);

		fflush(stdout);
                scanf("%s",buffer_texto);
		if (buffer_texto[0]=='y' || buffer_texto[0]=='Y') return 1;
		return 0;
        }


        cls_menu_overlay();

        menu_espera_no_tecla();


	menu_item *array_menu_confirm_yes_no;
        menu_item item_seleccionado;
        int retorno_menu;

	//Siempre indicamos el NO
	int confirm_yes_no_opcion_seleccionada=2;
        do {

		menu_add_item_menu_inicial_format(&array_menu_confirm_yes_no,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

                menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,"~~Yes");
		menu_add_item_menu_shortcut(array_menu_confirm_yes_no,'y');

                menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_NORMAL,NULL,NULL,"~~No");
		menu_add_item_menu_shortcut(array_menu_confirm_yes_no,'n');

                //separador adicional para que quede mas grande la ventana y mas mono
                menu_add_item_menu_format(array_menu_confirm_yes_no,MENU_OPCION_SEPARADOR,NULL,NULL," ");



                retorno_menu=menu_dibuja_menu(&confirm_yes_no_opcion_seleccionada,&item_seleccionado,array_menu_confirm_yes_no,texto_ventana);

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
			if (confirm_yes_no_opcion_seleccionada==1) return 1;
			else return 0;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

	return 0;


}


//retorna 1 si opcion 1
//retorna 2 si opcion 2
//retorna 0 si ESC
int menu_simple_two_choices(char *texto_ventana,char *texto_interior,char *opcion1,char *opcion2)
{

        cls_menu_overlay();

        menu_espera_no_tecla();


	menu_item *array_menu_simple_two_choices;
        menu_item item_seleccionado;
        int retorno_menu;

	//Siempre indicamos la primera opcion
	int simple_two_choices_opcion_seleccionada=1;
        do {

		menu_add_item_menu_inicial_format(&array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

                menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion1);

                menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_NORMAL,NULL,NULL,opcion2);

                //separador adicional para que quede mas grande la ventana y mas mono
                menu_add_item_menu_format(array_menu_simple_two_choices,MENU_OPCION_SEPARADOR,NULL,NULL," ");



                retorno_menu=menu_dibuja_menu(&simple_two_choices_opcion_seleccionada,&item_seleccionado,array_menu_simple_two_choices,texto_ventana);

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        return simple_two_choices_opcion_seleccionada;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

	return 0;


}

//Retorna 0=Cancel, 1=Append, 2=Truncate
int menu_ask_no_append_truncate_texto(char *texto_ventana,char *texto_interior)
{

	

        cls_menu_overlay();

        menu_espera_no_tecla();


	menu_item *array_menu_ask_no_append_truncate;
        menu_item item_seleccionado;
        int retorno_menu;

	//Siempre indicamos el Cancel
	int ask_no_append_truncate_opcion_seleccionada=1;
        do {

		menu_add_item_menu_inicial_format(&array_menu_ask_no_append_truncate,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

                menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Cancel");
		menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'c');

                menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Append");
		menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'a');

                menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_NORMAL,NULL,NULL,"~~Truncate");
                menu_add_item_menu_shortcut(array_menu_ask_no_append_truncate,'t');

                //separador adicional para que quede mas grande la ventana y mas mono
                menu_add_item_menu_format(array_menu_ask_no_append_truncate,MENU_OPCION_SEPARADOR,NULL,NULL," ");



                retorno_menu=menu_dibuja_menu(&ask_no_append_truncate_opcion_seleccionada,&item_seleccionado,array_menu_ask_no_append_truncate,texto_ventana);

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
			//if (ask_no_append_truncate_opcion_seleccionada==1) return 1;
			//else return 0;
                        return ask_no_append_truncate_opcion_seleccionada-1; //0=Cancel, 1=Append, 2=Truncate
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

	return 0;


}


//Funcion para preguntar opcion de una lista, usando interfaz de menus
//Entradas de lista finalizadas con NULL
//Retorna 0=Primera opcion, 1=Segunda opcion, etc
//Retorna <0 si salir con ESC
int menu_ask_list_texto(char *texto_ventana,char *texto_interior,char *entradas_lista[])
{



        cls_menu_overlay();

        menu_espera_no_tecla();


        menu_item *array_menu_ask_list;
        menu_item item_seleccionado;
        int retorno_menu;

        //Siempre indicamos primera opcion
        int ask_list_texto_opcion_seleccionada=1;
        do {

                menu_add_item_menu_inicial_format(&array_menu_ask_list,MENU_OPCION_SEPARADOR,NULL,NULL,texto_interior);

                int i=0;

                while(entradas_lista[i]!=NULL) {
                        menu_add_item_menu_format(array_menu_ask_list,MENU_OPCION_NORMAL,NULL,NULL,entradas_lista[i]);
			i++;
                }

                //separador adicional para que quede mas grande la ventana y mas mono
                menu_add_item_menu_format(array_menu_ask_list,MENU_OPCION_SEPARADOR,NULL,NULL," ");



                retorno_menu=menu_dibuja_menu(&ask_list_texto_opcion_seleccionada,&item_seleccionado,array_menu_ask_list,texto_ventana);

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        return ask_list_texto_opcion_seleccionada-1;
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

        return -1;

}


void menu_generic_message_format(char *titulo, const char * texto_format , ...)
{

        //Buffer de entrada
        char texto[MAX_TEXTO_GENERIC_MESSAGE];
        va_list args;
        va_start (args, texto_format);
        vsprintf (texto,texto_format, args);
        va_end (args);


	//menu_generic_message_tooltip(titulo, 0, 0, 0, NULL, "%s", texto);
	zxvision_generic_message_tooltip(titulo , 0 , 0, 0, 0, NULL, 1, "%s", texto);


	//En Linux esto funciona bien sin tener que hacer las funciones va_ previas:
	//menu_generic_message_tooltip(titulo, 0, texto_format)
	//Pero en Mac OS X no obtiene los valores de los parametros adicionales
}

void menu_generic_message(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 0, 0, 0, NULL, "%s", texto);
		zxvision_generic_message_tooltip(titulo , 0 , 0, 0, 0, NULL, 1, "%s", texto);
}

//Mensaje con setting para marcar
void zxvision_menu_generic_message_setting(char *titulo, const char *texto, char *texto_opcion, int *valor_opcion)
{

	int lineas_agregar=4;

	//Asumimos opcion ya marcada
	*valor_opcion=1;
	
	zxvision_generic_message_tooltip(titulo , lineas_agregar , 0, 0, 0, NULL, 1, "%s", texto);
	
	if (!strcmp(scr_driver_name,"stdout")) {
		printf ("%s\n",texto_opcion);
		scrstdout_menu_print_speech_macro (texto_opcion);
		printf("Enable or disable setting? 0 or 1?\n");
		scrstdout_menu_print_speech_macro("Enable or disable setting? 0 or 1?");
		scanf("%d",valor_opcion);
		return;
	}

	zxvision_window *ventana;

	//Nuestra ventana sera la actual
	ventana=zxvision_current_window;

	int posicion_y_opcion=ventana->visible_height-lineas_agregar-1;
	//printf ("%d %d\n",posicion_y_opcion,ventana->visible_height);

	int ancho_ventana=ventana->visible_width;
	int posicion_centro_x=ancho_ventana/2-1; //un poco mas a la izquierda

	if (posicion_centro_x<0) posicion_centro_x=0;


		menu_item *array_menu_generic_message_setting;
        menu_item item_seleccionado;
		int array_menu_generic_message_setting_opcion_seleccionada=1;
        int retorno_menu;

	int salir=0;
    do {


		char buffer_texto_opcion[64];
		char buffer_texto_ok[64];

		sprintf (buffer_texto_opcion,"[%c] %s",(*valor_opcion ? 'X' : ' ' ),texto_opcion);
		strcpy(buffer_texto_ok,"<OK>");

		//Tengo antes los textos para sacar longitud y centrarlos

		int posicion_x_opcion=posicion_centro_x-strlen(buffer_texto_opcion)/2;
		int posicion_x_ok=posicion_centro_x-strlen(buffer_texto_ok)/2;


		menu_add_item_menu_inicial_format(&array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto_opcion);
		menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_opcion,posicion_y_opcion);


		menu_add_item_menu_format(array_menu_generic_message_setting,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto_ok);
		menu_add_item_menu_tabulado(array_menu_generic_message_setting,posicion_x_ok,posicion_y_opcion+2);


		//Nombre de ventana solo aparece en el caso de stdout
    	retorno_menu=menu_dibuja_menu(&array_menu_generic_message_setting_opcion_seleccionada,&item_seleccionado,array_menu_generic_message_setting,titulo);


        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	
				//Si opcion 1, conmutar valor		
				//Conmutar valor
				if (array_menu_generic_message_setting_opcion_seleccionada==0) *valor_opcion ^=1;

				//Si opcion 2, volver
				if (array_menu_generic_message_setting_opcion_seleccionada==1) {
					salir=1;
				}
                
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus && !salir);


	cls_menu_overlay();
	zxvision_destroy_window(ventana);

	//Y liberar esa memoria, dado que la ventana esta asignada en memoria global
	free(ventana);
}


void menu_generic_message_splash(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 1, 0, 0, NULL, "%s", texto);
		zxvision_generic_message_tooltip(titulo , 0 , 1, 0, 0, NULL, 0, "%s", texto);
}

void menu_generic_message_warn(char *titulo, const char * texto)
{

        //menu_generic_message_tooltip(titulo, 1, 0, 0, NULL, "%s", texto);
		zxvision_generic_message_tooltip(titulo , 0 , 2, 0, 0, NULL, 0, "%s", texto);
}

int menu_confirm_yesno(char *texto_ventana)
{
	return menu_confirm_yesno_texto(texto_ventana,"Sure?");
}


//max_length contando caracter 0 del final, es decir, para un texto de 4 caracteres, debemos especificar max_length=5
void menu_ventana_scanf(char *titulo,char *texto,int max_length)
{

        //En caso de stdout, es mas simple, mostrar texto y esperar texto
        if (!strcmp(scr_driver_name,"stdout")) {
		printf ("%s\n",titulo);
		scrstdout_menu_print_speech_macro(titulo);
		scanf("%s",texto);

		return;
	}

	//int scanf_x=1;
	//int scanf_y=10;
	int scanf_ancho=30;
	int scanf_alto=3;	
	int scanf_x=menu_center_x()-scanf_ancho/2;
	int scanf_y=menu_center_y()-scanf_alto/2;


        menu_espera_no_tecla();

	zxvision_window ventana;

	zxvision_new_window(&ventana,scanf_x,scanf_y,scanf_ancho,scanf_alto,
							scanf_ancho-1,scanf_alto-2,titulo);

	//No queremos que se pueda redimensionar
	ventana.can_be_resized=0;

	zxvision_draw_window(&ventana);


	zxvision_scanf(&ventana,texto,max_length,scanf_ancho-2,1,0);

	//menu_scanf(texto,max_length,scanf_ancho-2,scanf_x+1,scanf_y+1);
	//int menu_scanf(char *string,unsigned int max_length,int max_length_shown,int x,int y)

	//Al salir
	//menu_refresca_pantalla();
	menu_cls_refresh_emulated_screen();

	zxvision_destroy_window(&ventana);

}


void menu_about_read_file(char *title,char *aboutfile)
{

	char about_file[MAX_TEXTO_GENERIC_MESSAGE];

	debug_printf (VERBOSE_INFO,"Loading %s File",aboutfile);
	FILE *ptr_aboutfile;
	//ptr_aboutfile=fopen(aboutfile,"rb");
	open_sharedfile(aboutfile,&ptr_aboutfile);

	if (!ptr_aboutfile)
	{
		debug_printf (VERBOSE_ERR,"Unable to open %s file",aboutfile);
	}
	else {

		int leidos=fread(about_file,1,MAX_TEXTO_GENERIC_MESSAGE,ptr_aboutfile);
		debug_printf (VERBOSE_INFO,"Read %d bytes of file: %s",leidos,aboutfile);

		if (leidos==MAX_TEXTO_GENERIC_MESSAGE) {
			debug_printf (VERBOSE_ERR,"Reached maximum text buffer: %d bytes. Showing only these",leidos);
			leidos--;
		}

		about_file[leidos]=0;


		fclose(ptr_aboutfile);

		menu_generic_message(title,about_file);

	}

}

void menu_about_changelog(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Changelog","Changelog");
}


void menu_about_history(MENU_ITEM_PARAMETERS)
{
	menu_about_read_file("History","HISTORY");
}

void menu_about_features(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Features","FEATURES");
}

void menu_about_readme(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Readme","README");
}


void menu_about_install(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Install","INSTALL");
}

void menu_about_installwindows(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Install on Windows","INSTALLWINDOWS");
}

void menu_about_alternate_roms(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Alternate ROMS","ALTERNATEROMS");
}

void menu_about_included_tapes(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Included Tapes","INCLUDEDTAPES");
}



void menu_about_acknowledgements(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Acknowledgements","ACKNOWLEDGEMENTS");
}

void menu_about_donate(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Donate","DONATE");
}

void menu_about_faq(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("FAQ","FAQ");
}

void menu_about_license(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("ZEsarUX License","LICENSE");
}

void menu_about_licenses_info(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Licenses information","LICENSES_info");
}

void menu_about_license_motorola_core(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Motorola Core License","licenses/LICENSE_MOTOROLA_CORE");
}

void menu_about_license_scmp_core(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("SCMP Core License","licenses/LICENSE_SCMP_CORE");
}

void menu_about_license_scl2trd(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("scl2trd License","licenses/LICENSE_scl2trd");
}

void menu_about_license_fuse(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("Fuse License","licenses/LICENSE_fuse");
}

void menu_about_license_atomlite(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("SimCoupe License","licenses/LICENSE_simcoupe");
}

void menu_about_license_unrealspeccy(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("UnrealSpeccy License","licenses/LICENSE_unrealspeccy");
}

void menu_about_license_mdvtool(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("mdvtool License","licenses/LICENSE_mdvtool");
}

void menu_about_license_zip(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("zip License","licenses/LICENSE_zip");
}

void menu_about_license_unpaws(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("unpaws/unquill License","licenses/LICENSE_unpaws");
}

void menu_about_license_undaad(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("undaad License","licenses/LICENSE_undaad");
}

void menu_about_license_grackle(MENU_ITEM_PARAMETERS)
{
        menu_about_read_file("grackle License","licenses/LICENSE_grackle");
}


void menu_about_statistics(MENU_ITEM_PARAMETERS)
{

 int tiempo_trabajado_en_zesarux=timer_get_worked_time();

	menu_generic_message_format("Statistics",
		"Source code lines: %d\n"
		"Total time invested on programming ZEsarUX: ^^%d^^ hours (and growing)\n\n"
		"Edited with VSCode, Working Copy and vim\n"
		"Developed on macOS Catalina, Debian 9, Raspbian, and MinGW environment on Windows\n"
		,LINES_SOURCE,tiempo_trabajado_en_zesarux);

}



void menu_about_running_info(MENU_ITEM_PARAMETERS)
{

	char string_video_drivers[1024];
	char string_audio_drivers[1024];
	int driver;

	int i;

	i=0;
        for (driver=0;driver<num_scr_driver_array;driver++) {
		sprintf(&string_video_drivers[i],"%s ",scr_driver_array[driver].driver_name);
		i=i+strlen(scr_driver_array[driver].driver_name)+1;
	}
	string_video_drivers[i]=0;


	i=0;
        for (driver=0;driver<num_audio_driver_array;driver++) {
                sprintf(&string_audio_drivers[i],"%s ",audio_driver_array[driver].driver_name);
                i=i+strlen(audio_driver_array[driver].driver_name)+1;
        }
        string_audio_drivers[i]=0;

				char configfile[PATH_MAX];

				if (util_get_configfile_name(configfile)==0)  {
					sprintf(configfile,"Unknown");
				}

				int uptime_seconds=timer_get_uptime_seconds();

				char hora_inicio[100];
				timer_get_texto_time(&zesarux_start_time,hora_inicio);


	//int cpu_use_total_acumulado=0;
	//int cpu_use_total_acumulado_medidas=0;

	int media_cpu=0;

	if (cpu_use_total_acumulado_medidas>0) {
		media_cpu=cpu_use_total_acumulado/cpu_use_total_acumulado_medidas;
	}

	char mensaje_cpu_usage[100];

	if (screen_show_cpu_usage.v && menu_footer) {
		sprintf(mensaje_cpu_usage,"Average CPU Use: %d%%\n",media_cpu);
	}
	else {
		mensaje_cpu_usage[0]=0;
	} 
	
	char mensaje_total_uptime[100];

	//tiempo total de uso del emulador solo si esta guardado de config
	if (save_configuration_file_on_exit.v) {
		sprintf (mensaje_total_uptime,"Total minutes use %d mins\n",
  		stats_get_current_total_minutes_use() );
	}
	else {
		mensaje_total_uptime[0]=0;
	}

	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);	

	menu_generic_message_format("Running info",
		"Video Driver: %s\nAvailable video drivers: %s\n\nAudio Driver: %s\nAvailable audio drivers: %s\n\n"
		"Current directory: %s\n\n"
		"Configuration file: %s\n\n"
		"Start time: %s\n"
		"Uptime %d secs (%d mins)\n"
		"%s"
		"%s"
		,
		scr_driver_name,string_video_drivers,audio_driver_name,string_audio_drivers,
		directorio_actual,
		configfile,hora_inicio,
		uptime_seconds,uptime_seconds/60,mensaje_total_uptime,mensaje_cpu_usage);

	//Average CPU use solo sale si screen_show_cpu_usage.v




}



void menu_about_about(MENU_ITEM_PARAMETERS)
{

	char mensaje_about[1024];
	unsigned char letra_enye;

	if (si_complete_video_driver() ) {
		//mensaje completo con enye en segundo apellido
		letra_enye=129;
	}

	else {
		//mensaje con n en vez de enye
		letra_enye='n';
	}

	sprintf (mensaje_about,"ZEsarUX V." EMULATOR_VERSION " (" EMULATOR_SHORT_DATE ")\n"
                        " - " EMULATOR_EDITION_NAME " - \n"

#ifdef SNAPSHOT_VERSION
                "Build number: " BUILDNUMBER "\n"
#endif

                        "(C) 2013 Cesar Hernandez Ba%co\n",letra_enye);

	//menu_generic_message("About",mensaje_about);

/*menu_generic_message_tooltip("Select file", 0, 0, 0, &retorno_archivo, "%s", texto_buffer);
menu_generic_message_tooltip(char *titulo, int volver_timeout, int tooltip_enabled, int mostrar_cursor, generic_message_tooltip_return *retorno, const char * texto_format , ...);
*/
	generic_message_tooltip_return retorno_ventana;
	//menu_generic_message_tooltip("About",0,0,0,&retorno_ventana,mensaje_about);
	zxvision_generic_message_tooltip("About" , 0 ,0,0,0,&retorno_ventana,0,mensaje_about);

	//printf ("retorno ventana: %d\n",retorno_ventana.estado_retorno);

	//Si se sale con ESC
    if (retorno_ventana.estado_retorno==0) return;

	//Linea seleccionada es 1? quiere decir que se selecciona texto "--- edition"
/*
	Por defecto, linea seleccionada es 0, incluso aunque no se haya habilitado linea de cursor, por ejemplo
	al buscar texto con f y n
	Como la que buscamos es la 1, no hay problema de falso positivo
*/
	int linea=retorno_ventana.linea_seleccionada;

	//printf ("retorno ventana linea: %d\n",retorno_ventana.linea_seleccionada);

	debug_printf(VERBOSE_INFO,"Closing window with Enter and selected line=%d",linea);
	if (linea==1) {
		if (si_existe_editionnamegame(NULL)) {
			util_load_editionnamegame();
			salir_todos_menus=1;
		}
		else {
			//Se ha seleccionado texto edition name pero el juego no esta disponible
			debug_printf(VERBOSE_INFO,"Edition name game %s is not available",EMULATOR_GAME_EDITION);
		}
	}

}


void menu_about_compile_info(MENU_ITEM_PARAMETERS)
{
        char buffer[MAX_COMPILE_INFO_LENGTH];
        get_compile_info(buffer);

	menu_generic_message("Compile info",
		buffer
	);
}


void menu_about_help(MENU_ITEM_PARAMETERS)
{

	if (!menu_cond_stdout() ) {
		menu_generic_message("Help",
			"Use cursor keys to move\n"
			"Press F1 (or h on some video drivers) on an option to see its help\n"
			"Press Enter to select a menu item\n"
			"Press Space on some menu items to enable/disable\n"
			"Press a key between a and z to select an entry with shortcuts; shortcut clues appear when you wait some seconds or press a key "
			"not associated with any shortcut.\n"
			"Unavailable options may be hidden, or disabled, which are shown with red colour or with x cursor on some video drivers\n"
			"ESC Key gives you to the previous menu, except in the case with aalib driver and pure text console, which is changed to another key (shown on the menu). On curses driver, ESC key is a bit slow, you have to wait one second after pressing it; you can also use key @ to simulate ESC on menu on curses driver. "
			"\n\n"
			"On fileselector:\n"
			"- Use cursors and PgDn/Up\n"
			"- Use Enter or left mouse click to select item. Compressed files will be opened like folders\n"
			"- Use Space to expand files, currently supported: tap, tzx, trd, scl, dsk, mdv, hdf, P, O, Z88 Cards (.epr, .eprom, .flash) and also all the compressed supported files\n"
			"- Use TAB to change section\n"
			"- Use Space/cursor on filter to change filter\n"
			"- Press the initial letter\n"
			"  for faster searching\n"
			"When using fileselector from file utilities menu:\n"
			"- Press shift+key to select functions, like shift+v to view files or shift+d to select Windows drive\n"
			"\n"
			"On message windows:\n"
			"- Use cursors and PgDn/Up\n"
			"- Use f and n to find text\n"
			"- Use c to copy to ZEsarUX clipboard\n"
			"\n"
			"On numeric input fields, numbers can be written on decimal, hexadecimal (with suffix H), binary (with suffix %) or as a character (with quotes '' or \"\")\n\n"
			"Symbols on menu must be written according to the Spectrum keyboard mapping, so for example, to write the symbol minus (<), you have to press "
			"ctrl(symbol shift)+r. You should use ctrl/alt (no need to Spectrum extended mode) to write any of the following: ~|\\{}[], located on letters asdfgyu\n\n"
			"\n"
			"On ZX-Vision windows:\n"
			"- Use mouse to move windows dragging from the title bar\n"
			"- Drag mouse from the bottom-right part of the window to resize it\n"
			"- Doble click on the title bar to mazimize/restore\n"
			"- Click out of the window to put the focus on the emulated machine and send there keyboard presses\n"
			"- Can also be moved with the keyboard: Shift+QAOP\n"
			"- Can also be resized with the keyboard: Shift+WSKL\n"
			"Note: non ZX-Vision windows are marked with a small pixel in the right of the title bar\n"

			"\n"

			"Inside a machine, the keys are mapped this way:\n"
			"ESC: If text to speech is not enabled, sends Shift+Space (break) on Spectrum. If enabled, stops playing text to speech\n"
			"CTRL/ALT: Symbol shift\n"
			"TAB: Extended mode (symbol shift + caps shift)\n"
			"\n"
			"F4: Send display content to speech program, including only known characters (unknown characters are shown as a space)\n"
			"F5: Open menu\n"
			"F8: Open On Screen Keyboard (only on Spectrum & ZX80/81)\n"
			"F9: Open Smart Load window\n"
			"\n"
			"On Z88, the following keys have special meaning:\n"
			"F1: Help\n"
			"F2: Index\n"
			"F3: Menu\n"
			"CTRL: Diamond\n"
			"ALT: Square\n"
			"\n"
			"Note: Drivers curses, aa, caca do not read CTRL or ALT. Instead, you can use F6 for CTRL and F7 for ALT on these drivers.\n"
	);
	}

	else {
		menu_generic_message("Help","Press h<num> to see help for <num> option, for example: h3\n"
				"Press t<num> to see tooltip for <num> option\n"
				"Unavailable options may be hidden, or disabled, which are shown with x cursor\n"
				"ESC text gives you to the previous menu\n"
				"On fileselector, write the file without spaces\n"
				"On input fields, numbers can be written on decimal or hexadecimal (with suffix H)\n"
	);
	}

}


void menu_licenses(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_common;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
        
            menu_add_item_menu_inicial(&array_menu_common,"~~Information",MENU_OPCION_NORMAL,menu_about_licenses_info,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'i');

            menu_add_item_menu(array_menu_common,"~~ZEsarUX",MENU_OPCION_NORMAL,menu_about_license,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'z');

			menu_add_item_menu(array_menu_common,"~~Motorola Core",MENU_OPCION_NORMAL,menu_about_license_motorola_core,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'m');

			menu_add_item_menu(array_menu_common,"~~SCMP Core",MENU_OPCION_NORMAL,menu_about_license_scmp_core,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'s');

			menu_add_item_menu(array_menu_common,"s~~cl2trd",MENU_OPCION_NORMAL,menu_about_license_scl2trd,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'c');

			menu_add_item_menu(array_menu_common,"~~Fuse disassembler",MENU_OPCION_NORMAL,menu_about_license_fuse,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'f');			


			menu_add_item_menu(array_menu_common,"~~Atomlite (from simcoupe)",MENU_OPCION_NORMAL,menu_about_license_atomlite,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'a');	

			menu_add_item_menu(array_menu_common,"S~~PG loader (from unrealspeccy)",MENU_OPCION_NORMAL,menu_about_license_unrealspeccy,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'p');				

			menu_add_item_menu(array_menu_common,"m~~dvtool",MENU_OPCION_NORMAL,menu_about_license_mdvtool,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'d');						

			menu_add_item_menu(array_menu_common,"z~~ip",MENU_OPCION_NORMAL,menu_about_license_zip,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'i');	
			
				
	menu_add_item_menu(array_menu_common,"unpa~~ws/unquill",MENU_OPCION_NORMAL,menu_about_license_unpaws,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'w');		
			
			
			menu_add_item_menu(array_menu_common,"~~undaad",MENU_OPCION_NORMAL,menu_about_license_undaad,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'u');		
			
			menu_add_item_menu(array_menu_common,"un~~gac",MENU_OPCION_NORMAL,menu_about_license_grackle,NULL);
			menu_add_item_menu_shortcut(array_menu_common,'g');	
					

            menu_add_item_menu(array_menu_common,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_common);

            retorno_menu=menu_dibuja_menu(&licenses_opcion_seleccionada,&item_seleccionado,array_menu_common,"Licenses" );

			

            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            	//llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                	//printf ("actuamos por funcion\n");
                    item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                    
                }
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



//menu about settings
void menu_about(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_about;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
            menu_add_item_menu_inicial(&array_menu_about,"~~About",MENU_OPCION_NORMAL,menu_about_about,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'a');

			menu_add_item_menu(array_menu_about,"~~Help",MENU_OPCION_NORMAL,menu_about_help,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'h');

			menu_add_item_menu(array_menu_about,"~~Readme",MENU_OPCION_NORMAL,menu_about_readme,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'r');

			menu_add_item_menu(array_menu_about,"~~Features",MENU_OPCION_NORMAL,menu_about_features,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'f');

            menu_add_item_menu(array_menu_about,"H~~istory",MENU_OPCION_NORMAL,menu_about_history,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'i');

            menu_add_item_menu(array_menu_about,"A~~cknowledgements",MENU_OPCION_NORMAL,menu_about_acknowledgements,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'c');

			menu_add_item_menu(array_menu_about,"~~Donate",MENU_OPCION_NORMAL,menu_about_donate,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'d');

			menu_add_item_menu(array_menu_about,"FA~~Q",MENU_OPCION_NORMAL,menu_about_faq,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'q');

			menu_add_item_menu(array_menu_about,"Cha~~ngelog",MENU_OPCION_NORMAL,menu_about_changelog,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'n');

			menu_add_item_menu(array_menu_about,"Alternate RO~~MS",MENU_OPCION_NORMAL,menu_about_alternate_roms,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'m');

			menu_add_item_menu(array_menu_about,"Included ~~tapes",MENU_OPCION_NORMAL,menu_about_included_tapes,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'t');

			menu_add_item_menu(array_menu_about,"Insta~~ll",MENU_OPCION_NORMAL,menu_about_install,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'l');

			menu_add_item_menu(array_menu_about,"Install on ~~Windows",MENU_OPCION_NORMAL,menu_about_installwindows,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'w');

            menu_add_item_menu(array_menu_about,"C~~ompile info",MENU_OPCION_NORMAL,menu_about_compile_info,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'o');			

            menu_add_item_menu(array_menu_about,"~~Statistics",MENU_OPCION_NORMAL,menu_about_statistics,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'s');

            menu_add_item_menu(array_menu_about,"Core Statistics",MENU_OPCION_NORMAL,menu_about_core_statistics,NULL);
			//menu_add_item_menu_shortcut(array_menu_about,'r');

			menu_add_item_menu(array_menu_about,"R~~unning info",MENU_OPCION_NORMAL,menu_about_running_info,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'u');

			menu_add_item_menu(array_menu_about,"Lic~~enses",MENU_OPCION_NORMAL,menu_licenses,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'e');

			/*
            menu_add_item_menu(array_menu_about,"Lic~~ense",MENU_OPCION_NORMAL,menu_about_license,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'e');

			menu_add_item_menu(array_menu_about,"Motorola Core License",MENU_OPCION_NORMAL,menu_about_license_motorola_core,NULL);

			menu_add_item_menu(array_menu_about,"SCM~~P Core License",MENU_OPCION_NORMAL,menu_about_license_scmp_core,NULL);
			menu_add_item_menu_shortcut(array_menu_about,'p');
			*/


            menu_add_item_menu(array_menu_about,"",MENU_OPCION_SEPARADOR,NULL,NULL);

            menu_add_ESC_item(array_menu_about);

            retorno_menu=menu_dibuja_menu(&about_opcion_seleccionada,&item_seleccionado,array_menu_about,"Help" );

			

            if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
            	//llamamos por valor de funcion
                if (item_seleccionado.menu_funcion!=NULL) {
                	//printf ("actuamos por funcion\n");
                    item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                    
                }
            }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


//menu tape settings
void menu_settings_tape(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_tape;
	menu_item item_seleccionado;
	int retorno_menu;

        do {
                //char string_tape_load_shown[20],string_tape_load_inserted[50],string_tape_save_shown[20],string_tape_save_inserted[50];
		//char string_realtape_shown[23];

		menu_add_item_menu_inicial_format(&array_menu_settings_tape,MENU_OPCION_NORMAL,NULL,NULL,"--Standard Tape--");


		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_standard_to_real_tape_fallback,NULL,"[%c] Fa~~llback to real tape",(standard_to_real_tape_fallback.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_settings_tape,'l');
		menu_add_item_menu_tooltip(array_menu_settings_tape,"If this standard tape is detected as real tape, reinsert tape as real tape");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"While loading the standard tape, if a custom loading routine is detected, "
					"the tape will be ejected from standard tape and inserted it as real tape. If autoload tape is enabled, "
					"the machine will be resetted and loaded the tape from the beginning");


		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_any_flag,NULL,"[%c] A~~ny flag loading", (tape_any_flag_loading.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_tape,'n');
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Enables tape load routine to load without knowing block flag");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Enables tape load routine to load without knowing block flag. You must enable it on Tape Copy programs and also on Rocman game");



                //menu_add_item_menu(array_menu_settings_tape,"",MENU_OPCION_SEPARADOR,NULL,NULL);


			menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_simulate_real_load,NULL,"[%c] ~~Simulate real load", (tape_loading_simulate.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_tape,'s');
			menu_add_item_menu_tooltip(array_menu_settings_tape,"Simulate sound and loading stripes");
			menu_add_item_menu_ayuda(array_menu_settings_tape,"Simulate sound and loading stripes. You can skip simulation pressing any key (and the data is loaded)");

			menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_tape_simulate_real_load_fast,menu_tape_simulate_real_load_cond,"[%c] Fast Simulate real load", (tape_loading_simulate_fast.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_tape,"Simulate sound and loading stripes at faster speed");
                        menu_add_item_menu_ayuda(array_menu_settings_tape,"Simulate sound and loading stripes at faster speed");


        	        menu_add_item_menu(array_menu_settings_tape,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,NULL,NULL,"--Input Real Tape--");



		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_loading_sound,NULL,"[%c] Loading sound", (realtape_loading_sound.v==1 ? 'X' : ' '));
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Enable loading sound");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Enable loading sound. With sound disabled, the tape is also loaded");



		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_volumen,NULL,"[%s%d] Volume bit 1 range",(realtape_volumen>0 ? "+" : ""),realtape_volumen);
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Volume bit 1 starting range value");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"The input audio value read (considering range from -128 to +127) is treated "
					"normally as 1 if the value is in range 0...+127, and 0 if it is in range -127...-1. This setting "
					"increases this 0 (of range 0...+127) to consider it is a bit 1. I have found this value is better to be 0 "
					"on Spectrum, and 2 on ZX80/81");



		menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_wave_offset,NULL,"[%d] Level Offset",realtape_wave_offset);
		menu_add_item_menu_tooltip(array_menu_settings_tape,"Apply offset to sound value read");
		menu_add_item_menu_ayuda(array_menu_settings_tape,"Indicates some value (positive or negative) to sum to the raw value read "
					"(considering range from -128 to +127) to the input audio value read");

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings_tape,MENU_OPCION_NORMAL,menu_realtape_accelerate_loaders,NULL,"[%c] A~~ccelerate loaders",
				(accelerate_loaders.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_tape,'c');
			menu_add_item_menu_tooltip(array_menu_settings_tape,"Set top speed setting when loading a real tape");
			menu_add_item_menu_ayuda(array_menu_settings_tape,"Set top speed setting when loading a real tape");
		}





                menu_add_item_menu(array_menu_settings_tape,"",MENU_OPCION_SEPARADOR,NULL,NULL);




                //menu_add_item_menu(array_menu_settings_tape,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_tape);

                retorno_menu=menu_dibuja_menu(&settings_tape_opcion_seleccionada,&item_seleccionado,array_menu_settings_tape,"Tape Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC);


}

void menu_zxuno_spi_write_protect(MENU_ITEM_PARAMETERS)
{
	zxuno_flash_write_protection.v ^=1;
}




void menu_zxuno_spi_flash(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_zxuno_spi_flash;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                     char string_spi_flash_file_shown[12]; //,string_mmc_file_shown[13];            
			if (zxuno_flash_spi_name[0]==0) sprintf (string_spi_flash_file_shown,"Default");
			else menu_tape_settings_trunc_name(zxuno_flash_spi_name,string_spi_flash_file_shown,12);

			menu_add_item_menu_inicial_format(&array_menu_zxuno_spi_flash,MENU_OPCION_NORMAL,menu_zxuno_spi_flash_file,NULL,"~~Flash File: %s",string_spi_flash_file_shown);
			menu_add_item_menu_shortcut(array_menu_zxuno_spi_flash,'f');
			menu_add_item_menu_tooltip(array_menu_zxuno_spi_flash,"File used for the ZX-Uno SPI Flash");
			menu_add_item_menu_ayuda(array_menu_zxuno_spi_flash,"File used for the ZX-Uno SPI Flash");

			menu_add_item_menu_format(array_menu_zxuno_spi_flash,MENU_OPCION_NORMAL,menu_zxuno_spi_write_protect,NULL,"~~Write protect: %s", (zxuno_flash_write_protection.v ? "Yes" : "No"));
			menu_add_item_menu_shortcut(array_menu_zxuno_spi_flash,'w');
                        menu_add_item_menu_tooltip(array_menu_zxuno_spi_flash,"If ZX-Uno SPI Flash is write protected");
                        menu_add_item_menu_ayuda(array_menu_zxuno_spi_flash,"If ZX-Uno SPI Flash is write protected");



			menu_add_item_menu_format(array_menu_zxuno_spi_flash,MENU_OPCION_NORMAL,menu_zxuno_spi_persistent_writes,NULL,"Persistent Writes: %s",
					(zxuno_flash_persistent_writes.v ? "Yes" : "No") );
			menu_add_item_menu_tooltip(array_menu_zxuno_spi_flash,"Tells if ZX-Uno SPI Flash writes are saved to disk");
			menu_add_item_menu_ayuda(array_menu_zxuno_spi_flash,"Tells if ZX-Uno SPI Flash writes are saved to disk. "
			"When you enable it, all previous changes (before enable it and since machine boot) and "
			"future changes made to spi flash will be saved to disk.\n"
			"Note: all writing operations to SPI Flash are always saved to internal memory (unless you disable write permission), but this setting "
			"tells if these changes are written to disk or not.");
							

                   


				menu_add_item_menu(array_menu_zxuno_spi_flash,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_zxuno_spi_flash,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_zxuno_spi_flash);

                retorno_menu=menu_dibuja_menu(&zxuno_spi_flash_opcion_seleccionada,&item_seleccionado,array_menu_zxuno_spi_flash,"ZX-Uno Flash" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}


void menu_tape_settings_fast_autoload(MENU_ITEM_PARAMETERS)
{
	fast_autoload.v ^=1;
}

//menu storage settings
void menu_settings_storage(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_storage;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

                



                menu_add_item_menu_inicial_format(&array_menu_settings_storage,MENU_OPCION_NORMAL,menu_tape_autoloadtape,NULL,"[%c] ~~Autoload medium", (noautoload.v==0 ? 'X' : ' '));
                            menu_add_item_menu_shortcut(array_menu_settings_storage,'a');

                            menu_add_item_menu_tooltip(array_menu_settings_storage,"Autoload medium and set machine");
                            menu_add_item_menu_ayuda(array_menu_settings_storage,"This option first change to the machine that handles the medium file type selected (tape, cartridge, etc), resets it, set some default machine values, and then, it sends "
                                                            "a LOAD sentence to load the medium\n"
                                                            "Note: The machine is changed only using smartload. Inserting a medium only resets the machine but does not change it");


			if (noautoload.v==0) {
				menu_add_item_menu_format(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_tape_settings_fast_autoload,NULL,"[%c] ~~Fast autoload",
					(fast_autoload.v ? 'X' : ' ' ) );
				menu_add_item_menu_shortcut(array_menu_settings_storage,'f');
				menu_add_item_menu_tooltip(array_menu_settings_storage,"Do the autoload process at top speed");
				menu_add_item_menu_ayuda(array_menu_settings_storage,"Do the autoload process at top speed");
			}


                            menu_add_item_menu_format(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_tape_autoselectfileopt,NULL,"[%c] A~~utoselect medium opts", (autoselect_snaptape_options.v==1 ? 'X' : ' ' ));
                            menu_add_item_menu_shortcut(array_menu_settings_storage,'u');
                            menu_add_item_menu_tooltip(array_menu_settings_storage,"Detect options for the selected medium file and the needed machine");
                            menu_add_item_menu_ayuda(array_menu_settings_storage,"The emulator uses a database for different included programs "
                                            "(and some other not included) and reads .config files to select emulator settings and the needed machine "
                                            "to run them. If you disable this, the database nor the .config files are read");


							if (!MACHINE_IS_Z88 && !MACHINE_IS_CHLOE && !MACHINE_IS_QL) {
                						menu_add_item_menu(array_menu_settings_storage,"",MENU_OPCION_SEPARADOR,NULL,NULL);
								menu_add_item_menu_format(array_menu_settings_storage,MENU_OPCION_NORMAL,menu_settings_tape,NULL,"~~Tape");
								menu_add_item_menu_shortcut(array_menu_settings_storage,'t');
						}


																			
						

                menu_add_item_menu(array_menu_settings_storage,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_storage,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_settings_storage);

                retorno_menu=menu_dibuja_menu(&settings_storage_opcion_seleccionada,&item_seleccionado,array_menu_settings_storage,"Storage Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}








void menu_snapshot_close_menu_after_smartload(MENU_ITEM_PARAMETERS)
{
	no_close_menu_after_smartload.v ^=1;
}



void menu_snapshot_sna_set_machine(MENU_ITEM_PARAMETERS)
{
	sna_setting_no_change_machine.v ^=1;
}

void menu_snapshot_settings_compressed_zsf(MENU_ITEM_PARAMETERS)
{
	zsf_force_uncompressed ^=1;
}

void menu_snapshot_autosave_exit(MENU_ITEM_PARAMETERS)
{
	autosave_snapshot_on_exit.v ^=1;
}

void menu_snapshot_autoload_start(MENU_ITEM_PARAMETERS)
{
        autoload_snapshot_on_start.v ^=1;
}


void menu_snapshot_autosnap_path(MENU_ITEM_PARAMETERS)
{
	menu_storage_string_root_dir(autosave_snapshot_path_buffer);
}


void menu_settings_snapshot(MENU_ITEM_PARAMETERS)
{

        menu_item *array_menu_settings_snapshot;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


			//hotkeys usados: uvctslpinrh
					char string_autosave_interval_prefix[16];
					menu_tape_settings_trunc_name(snapshot_autosave_interval_quicksave_name,string_autosave_interval_prefix,16);

					char string_autosave_interval_path[16];
					menu_tape_settings_trunc_name(snapshot_autosave_interval_quicksave_directory,string_autosave_interval_path,16);


		menu_add_item_menu_inicial_format(&array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_permitir_versiones_desconocidas,NULL,"[%c] Allow ~~Unknown .ZX versions",(snap_zx_permitir_versiones_desconocidas.v ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'u');
		menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Allow loading ZX Snapshots of unknown versions");
		menu_add_item_menu_ayuda(array_menu_settings_snapshot,"This setting permits loading of ZX Snapshots files of unknown versions. "
					"It can be used to load snapshots saved on higher emulator versions than this one");


		menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_save_version,NULL,"[%d] Save ZX Snapshot ~~version",snap_zx_version_save);
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'v');
                menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Decide which kind of .ZX version file is saved");
                menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Version 1,2,3 works on ZEsarUX and ZXSpectr\n"
					"Version 4 works on ZEsarUX V1.3 and higher\n"
					"Version 5 works on ZEsarUX V2 and higher\n"
				);

                menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_settings_compressed_zsf,NULL,"[%c] ~~Compressed ZSF",(zsf_force_uncompressed ? ' ' : 'X') );
				menu_add_item_menu_shortcut(array_menu_settings_snapshot,'c');
                menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Setting to save compressed ZSF files or not"); 
                menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Setting to save compressed ZSF files or not"); 

                menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_sna_set_machine,NULL,"[%c] Se~~t machine snap load",(sna_setting_no_change_machine.v ? ' ' : 'X'));
				menu_add_item_menu_shortcut(array_menu_settings_snapshot,'t');
                menu_add_item_menu_tooltip(array_menu_settings_snapshot,"If machine is reset to 48k/128k when loading a .sna or .z80 snapshot file");
                menu_add_item_menu_ayuda(array_menu_settings_snapshot,"If machine is reset to 48k/128k when loading a .sna or .z80 snapshot file.\n"
					"Disabling it, the .sna snapshot is loaded but the machine is not changed, so it allows to load, for example, a 48k snapshot on a Prism machine, or TBBlue, or any Spectrum machine different than 48/128.\n"
					"If current machine is not a Spectrum, loading a .sna snapshot will always switch to 48k/128k.\n"
					"This setting only applies to .sna snapshots, but not to .z80, .zx, or any other snapshot type."
				);

                menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_close_menu_after_smartload,NULL,"[%c] Close menu on smartload",(no_close_menu_after_smartload.v ? ' ' : 'X'));
                menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Closes the menu after Smartload");
				menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Closes the menu after Smartload");
               

                menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_exit,NULL,"[%c] Auto~~save on exit",
			(autosave_snapshot_on_exit.v ? 'X' : ' ' ) );
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'s');
		 menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Saves a snapshot with the machine state when exiting ZEsarUX. Saved file is " AUTOSAVE_NAME);
		 menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Saves a snapshot with the machine state when exiting ZEsarUX. Saved file is " AUTOSAVE_NAME);



		menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autoload_start,NULL,"[%c] Auto~~load on start",
			(autoload_snapshot_on_start.v ? 'X' : ' ') );
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'l');
		menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Loads the snapshot saved when starting ZEsarUX (previous menu item)");
		menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Loads the snapshot saved when starting ZEsarUX (previous menu item)");



		if (autosave_snapshot_on_exit.v || autoload_snapshot_on_start.v) {
                	char string_autosnap_path[14];
	                menu_tape_settings_trunc_name(autosave_snapshot_path_buffer,string_autosnap_path,14);
			menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosnap_path,NULL,"Autosnap ~~path [%s]",string_autosnap_path);
			menu_add_item_menu_shortcut(array_menu_settings_snapshot,'p');
			menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Where to save/load automatic snapshot. If not set, uses current directory");
			menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Where to save/load automatic snapshot. If not set, uses current directory");
		}

		

                menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);



					menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval,NULL,"[%c] Contsave at ~~interval",
									(snapshot_contautosave_interval_enabled.v ? 'X' : ' ' ) );
					menu_add_item_menu_shortcut(array_menu_settings_snapshot,'i');
					menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Enable continuous autosave snapshot every fixed interval");
					menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Enable continuous autosave snapshot every fixed interval");


					if (snapshot_contautosave_interval_enabled.v) {
						menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_seconds,NULL,"[%d] Contsave Seco~~nds",snapshot_autosave_interval_seconds);
						menu_add_item_menu_shortcut(array_menu_settings_snapshot,'n');
						menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Save snapshot every desired interval");
						menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Save snapshot every desired interval");
					}


		menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_prefix,NULL,"QS&CA P~~refix [%s]",string_autosave_interval_prefix);
		menu_add_item_menu_shortcut(array_menu_settings_snapshot,'r');
					menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Name prefix for quicksave and continous autosave snapshots");
					menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Name prefix for quicksave and continous autosave snapshots. The final name will be: prefix-date-hour.zsf");

						menu_add_item_menu_format(array_menu_settings_snapshot,MENU_OPCION_NORMAL,menu_snapshot_autosave_at_interval_directory,NULL,"QS&CA Pat~~h [%s]",string_autosave_interval_path);
						menu_add_item_menu_shortcut(array_menu_settings_snapshot,'h');
						menu_add_item_menu_tooltip(array_menu_settings_snapshot,"Path to save quicksave & continous autosave");
						menu_add_item_menu_ayuda(array_menu_settings_snapshot,"Path to save quicksave & continous autosave. If not set, will use current directory");



					menu_add_item_menu(array_menu_settings_snapshot,"",MENU_OPCION_SEPARADOR,NULL,NULL);



		menu_add_ESC_item(array_menu_settings_snapshot);

                retorno_menu=menu_dibuja_menu(&settings_snapshot_opcion_seleccionada,&item_seleccionado,array_menu_settings_snapshot,"Snapshot Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}













void menu_settings_config_file_save_config(MENU_ITEM_PARAMETERS)
{
	if (util_write_configfile()) {
		menu_generic_message_splash("Save configuration","OK. Configuration saved");
	};
}

void menu_settings_config_file_save_on_exit(MENU_ITEM_PARAMETERS)
{
	if (save_configuration_file_on_exit.v) {
		if (menu_confirm_yesno_texto("Write configuration","To disable setting saveconf")==0) return;
		save_configuration_file_on_exit.v=0;
		util_write_configfile();
		menu_generic_message_splash("Save configuration","OK. Configuration saved");
	}
	else save_configuration_file_on_exit.v=1;
}

void menu_settings_config_file_show(MENU_ITEM_PARAMETERS)
{
                          char configfile[PATH_MAX];

                                if (util_get_configfile_name(configfile)==0)  {
					menu_warn_message("Unknown configuration file");
                                }
	else {
		menu_file_viewer_read_text_file("Config file",configfile);
	}
}


void menu_settings_config_file_reset(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno_texto("Reset defaults","Need to exit. Sure?")==0) return;

	util_create_sample_configfile(1);
	menu_warn_message("Configuration settings reset to defaults. Press enter to close ZEsarUX. You should start ZEsarUX again to read default configuration");

	//Y nos aseguramos que al salir no se guarde configuración con lo que tenemos en memoria
	save_configuration_file_on_exit.v=0;
	end_emulator();

}


//menu config_file settings
void menu_settings_config_file(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_config_file;
        menu_item item_seleccionado;
	int retorno_menu;
        do {



		menu_add_item_menu_inicial_format(&array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_save_on_exit,NULL,"[%c] ~~Autosave on exit",(save_configuration_file_on_exit.v ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'a');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Auto save configuration on exit emulator");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Auto save configuration on exit emulator and overwrite it. Note: not all settings are saved");

		menu_add_item_menu_format(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_save_config,NULL,"    ~~Save configuration");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'s');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Overwrite your configuration file with current settings");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Overwrite your configuration file with current settings");


		menu_add_item_menu_format(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_show,NULL,"    ~~View config file");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'v');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"View configuration file");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"View configuration file");


		menu_add_item_menu_format(array_menu_settings_config_file,MENU_OPCION_NORMAL,menu_settings_config_file_reset,NULL,"    ~~Reset config file");
		menu_add_item_menu_shortcut(array_menu_settings_config_file,'r');
		menu_add_item_menu_tooltip(array_menu_settings_config_file,"Reset configuration file to default values");
		menu_add_item_menu_ayuda(array_menu_settings_config_file,"Reset configuration file to default values");		



                menu_add_item_menu(array_menu_settings_config_file,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                
		menu_add_ESC_item(array_menu_settings_config_file);

                retorno_menu=menu_dibuja_menu(&settings_config_file_opcion_seleccionada,&item_seleccionado,array_menu_settings_config_file,"Configuration file" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}


//menu settings
void menu_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings;
	menu_item item_seleccionado;
	int retorno_menu;

        do {


		menu_add_item_menu_inicial(&array_menu_settings,"~~Accessibility",MENU_OPCION_NORMAL,menu_accessibility_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'a');
		menu_add_item_menu_tooltip(array_menu_settings,"Accessibility settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Accessibility settings, to use text-to-speech facilities on ZEsarUX menu and games");

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_audio,NULL,"A~~udio");
		menu_add_item_menu_shortcut(array_menu_settings,'u');
		menu_add_item_menu_tooltip(array_menu_settings,"Audio settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Audio settings");

		menu_add_item_menu(array_menu_settings,"C~~onfiguration file",MENU_OPCION_NORMAL,menu_settings_config_file,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'o');
		menu_add_item_menu_tooltip(array_menu_settings,"Configuration file");
		menu_add_item_menu_ayuda(array_menu_settings,"Configuration file");

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_cpu_settings,NULL,"~~CPU");
		menu_add_item_menu_shortcut(array_menu_settings,'c');
	    	menu_add_item_menu_tooltip(array_menu_settings,"Change some CPU settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Change some CPU settings");		

		menu_add_item_menu(array_menu_settings,"D~~ebug",MENU_OPCION_NORMAL,menu_settings_debug,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'e');
		menu_add_item_menu_tooltip(array_menu_settings,"Debug settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Debug settings");

		menu_add_item_menu(array_menu_settings,"~~Display",MENU_OPCION_NORMAL,menu_settings_display,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'d');
		menu_add_item_menu_tooltip(array_menu_settings,"Display settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Display settings");

		menu_add_item_menu(array_menu_settings,"~~GUI",MENU_OPCION_NORMAL,menu_interface_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'g');
		menu_add_item_menu_tooltip(array_menu_settings,"Settings for the GUI");
		menu_add_item_menu_ayuda(array_menu_settings,"These settings are related to the GUI interface");

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_hardware_settings,NULL,"~~Hardware");
		menu_add_item_menu_shortcut(array_menu_settings,'h');
		menu_add_item_menu_tooltip(array_menu_settings,"Other hardware settings for the running machine (not CPU or ULA)");
		menu_add_item_menu_ayuda(array_menu_settings,"Select different settings for the machine and change its behaviour (not CPU or ULA)");

		menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_settings_snapshot,NULL,"~~Snapshot");
		menu_add_item_menu_shortcut(array_menu_settings,'s');
		menu_add_item_menu_tooltip(array_menu_settings,"Snapshot settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Snapshot settings");

		menu_add_item_menu(array_menu_settings,"Stat~~istics",MENU_OPCION_NORMAL,menu_settings_statistics,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'i');
		menu_add_item_menu_tooltip(array_menu_settings,"Statistics settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Statistics settings");		

		menu_add_item_menu(array_menu_settings,"S~~torage",MENU_OPCION_NORMAL,menu_settings_storage,NULL);
		menu_add_item_menu_shortcut(array_menu_settings,'t');
		menu_add_item_menu_tooltip(array_menu_settings,"Storage settings");
		menu_add_item_menu_ayuda(array_menu_settings,"Storage settings");

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings,MENU_OPCION_NORMAL,menu_ula_settings,NULL,"U~~LA");
			menu_add_item_menu_shortcut(array_menu_settings,'l');
	                menu_add_item_menu_tooltip(array_menu_settings,"Change some ULA settings");
	                menu_add_item_menu_ayuda(array_menu_settings,"Change some ULA settings");


		}	





  menu_add_item_menu(array_menu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings);

                retorno_menu=menu_dibuja_menu(&settings_opcion_seleccionada,&item_seleccionado,array_menu_settings,"Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}





void menu_exit_emulator(MENU_ITEM_PARAMETERS)
{

	menu_reset_counters_tecla_repeticion();

	int salir=0;

	//Si quickexit, no preguntar
	if (quickexit.v) salir=1;

	else salir=menu_confirm_yesno("Exit ZEsarUX");

                        if (salir) {

				//menu_footer=0;

                                cls_menu_overlay();

                                reset_menu_overlay_function();
                                menu_abierto=0;

                                if (autosave_snapshot_on_exit.v) autosave_snapshot();

                                end_emulator();

                        }
                        cls_menu_overlay();
}

int menu_tape_settings_cond(void)
{
	return !(MACHINE_IS_Z88);
}


void menu_principal_salir_emulador(MENU_ITEM_PARAMETERS)
{
	menu_exit_emulator(0);	
}


void menu_inicio_bucle(void)
{

		menu_first_aid("initial_menu");

		//Si descargar stats
		//Si se pregunta si se quiere enviar estadisticas, solo si esta el grabado de configuracion, e interfaz permite menu (no stdout ni simpletext ni null)
		if (save_configuration_file_on_exit.v && stats_asked.v==0 && si_normal_menu_video_driver()) {
			stats_ask_if_enable();
		}
		
		int retorno_menu;

		menu_item *array_menu_principal;
		menu_item item_seleccionado;

		int salir_menu=0;


		do {

		if (strcmp(scr_driver_name,"xwindows")==0 || strcmp(scr_driver_name,"sdl")==0 || strcmp(scr_driver_name,"caca")==0 || strcmp(scr_driver_name,"fbdev")==0 || strcmp(scr_driver_name,"cocoa")==0 || strcmp(scr_driver_name,"curses")==0) f_functions=1;
		else f_functions=0;


		menu_add_item_menu_inicial(&array_menu_principal,"~~Smart load",MENU_OPCION_NORMAL,menu_quickload,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'s');
                menu_add_item_menu_tooltip(array_menu_principal,"Smart load tape, snapshot, Z88 memory card or Timex Cartridge");
                menu_add_item_menu_ayuda(array_menu_principal,"This option loads the file depending on its type: \n"
					"-Binary tapes are inserted as standard tapes and loaded quickly\n"
					"-Audio tapes are loaded as real tapes\n"
					"-Snapshots are loaded at once\n"
					"-Timex Cartridges are inserted on the machine and you should do a reset to run the cartridge\n"
					"-Memory cards on Z88 are inserted on the machine\n\n"
					"Note: Tapes will be autoloaded if the autoload setting is on (by default)"

					);

		menu_add_item_menu(array_menu_principal,"~~Machine",MENU_OPCION_NORMAL,menu_machine_selection,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'m');
		menu_add_item_menu_tooltip(array_menu_principal,"Change active machine");
		menu_add_item_menu_ayuda(array_menu_principal,"You can switch to another machine. It also resets the machine");

		menu_add_item_menu_format(array_menu_principal,MENU_OPCION_NORMAL,menu_storage_settings,NULL,"S~~torage");
		menu_add_item_menu_shortcut(array_menu_principal,'t');
		menu_add_item_menu_tooltip(array_menu_principal,"Select storage mediums, like tape, MMC, IDE, etc");
		menu_add_item_menu_ayuda(array_menu_principal,"Select storage mediums, like tape, MMC, IDE, etc");		

		menu_add_item_menu(array_menu_principal,"S~~napshot",MENU_OPCION_NORMAL,menu_snapshot,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'n');
		menu_add_item_menu_tooltip(array_menu_principal,"Load or save snapshots");
		menu_add_item_menu_ayuda(array_menu_principal,"Load or save different snapshot images. Snapshot images are loaded or saved at once");

		menu_add_item_menu(array_menu_principal,"~~Audio",MENU_OPCION_NORMAL,menu_audio_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'a');
		menu_add_item_menu_tooltip(array_menu_principal,"Audio related actions");
		menu_add_item_menu_ayuda(array_menu_principal,"Audio related actions");


		menu_add_item_menu(array_menu_principal,"Net~~work",MENU_OPCION_NORMAL,menu_network,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'w');
		menu_add_item_menu_tooltip(array_menu_principal,"Network related actions");
		menu_add_item_menu_ayuda(array_menu_principal,"Network related actions");

		menu_add_item_menu(array_menu_principal,"D~~ebug",MENU_OPCION_NORMAL,menu_debug_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'e');
		menu_add_item_menu_tooltip(array_menu_principal,"Debug tools");
		menu_add_item_menu_ayuda(array_menu_principal,"Tools to debug the machine");

		menu_add_item_menu(array_menu_principal,"~~Display",MENU_OPCION_NORMAL,menu_display_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'d');
		menu_add_item_menu_tooltip(array_menu_principal,"Display related actions");
		menu_add_item_menu_ayuda(array_menu_principal,"Display related actions");


		menu_add_item_menu(array_menu_principal,"Sett~~ings",MENU_OPCION_NORMAL,menu_settings,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'i');
		menu_add_item_menu_tooltip(array_menu_principal,"General Settings");
		menu_add_item_menu_ayuda(array_menu_principal,"General Settings");

		

		menu_add_item_menu(array_menu_principal,"",MENU_OPCION_SEPARADOR,NULL,NULL);
		menu_add_item_menu(array_menu_principal,"He~~lp...",MENU_OPCION_NORMAL,menu_about,NULL);
		menu_add_item_menu_shortcut(array_menu_principal,'l');
		menu_add_item_menu_tooltip(array_menu_principal,"Help menu");
		menu_add_item_menu_ayuda(array_menu_principal,"Some help and related files");


		menu_add_ESC_item(array_menu_principal);

		menu_add_item_menu_format(array_menu_principal,MENU_OPCION_NORMAL,menu_principal_salir_emulador,NULL,"%sExit emulator",(f_functions==1 ? "F10 ": "") );
		menu_add_item_menu_tooltip(array_menu_principal,"Exit emulator");
		menu_add_item_menu_ayuda(array_menu_principal,"Exit emulator");


		retorno_menu=menu_dibuja_menu(&menu_inicio_opcion_seleccionada,&item_seleccionado,array_menu_principal,"ZEsarUX v." EMULATOR_VERSION );

		//printf ("Opcion seleccionada: %d\n",menu_inicio_opcion_seleccionada);
		//printf ("Tipo opcion: %d\n",item_seleccionado.tipo_opcion);
		//printf ("Retorno menu: %d\n",retorno_menu);

		

		//opcion 12 es F10 salir del emulador
		if ( (retorno_menu!=MENU_RETORNO_ESC) &&  (retorno_menu==MENU_RETORNO_F10)  ) {

			//menu_exit_emulator(0);
			menu_principal_salir_emulador(0);

	        }


		else if (retorno_menu>=0) {
			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
				
                        	item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				

				//si ha generado error, no salir
				if (if_pending_error_message) salir_todos_menus=0;
	                }

			
		}

                //printf ("Opcion seleccionada: %d\n",menu_inicio_opcion_seleccionada);
                //printf ("Tipo opcion: %d\n",item_seleccionado.tipo_opcion);
                //printf ("Retorno menu: %d\n",retorno_menu);

		//if (retorno_menu==MENU_RETORNO_F2) salir_menu=1;

		//opcion numero 11: ESC back

		if (retorno_menu!=MENU_RETORNO_ESC && menu_inicio_opcion_seleccionada==11) salir_menu=1;
		if (retorno_menu==MENU_RETORNO_ESC) salir_menu=1;

	} while (!salir_menu && !salir_todos_menus);

	textspeech_print_speech("Closing emulator menu and going back to emulated machine");


	        //} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}

void menu_inicio_pre_retorno_reset_flags(void)
{
    //desactivar botones de acceso directo
    menu_button_quickload.v=0;
    menu_button_osdkeyboard.v=0;
    menu_button_osd_adv_keyboard_return.v=0;
    menu_button_osd_adv_keyboard_openmenu.v=0;
    menu_button_exit_emulator.v=0;
    menu_event_drag_drop.v=0;
    menu_breakpoint_exception.v=0;
    menu_event_remote_protocol_enterstep.v=0;
    menu_button_f_function.v=0;
	menu_event_open_menu.v=0;
}

void menu_inicio_pre_retorno(void)
{
	menu_inicio_pre_retorno_reset_flags();

    reset_menu_overlay_function();
    menu_set_menu_abierto(0);


    //Para refrescar border en caso de tsconf por ejemplo, en que el menu sobreescribe el border
    //modificado_border.v=1;

    timer_reset();

	//Y refrescar footer. Hacer esto para que redibuje en pantalla y no en layer de mezcla de menu
	menu_init_footer();  
/*
menu_init_footer hace falta pues el layer de menu se borra y se queda negro en las zonas izquierda y derecha del footer
*/

	redraw_footer();

}

void menu_process_f_function_pause(void)
{

	int antes_multitarea;

	//Guardar valor anterior multitarea
	antes_multitarea=menu_multitarea;
	menu_multitarea=0;
	audio_playing.v=0;
	z80_byte tecla=0;

	while (tecla==0) {
		menu_espera_tecla();
		tecla=menu_get_pressed_key();
	}

	menu_espera_no_tecla();

	//restaurar
	menu_multitarea=antes_multitarea;
}

void menu_process_f_functions_by_action(int accion)
{


	char final_name[PATH_MAX];

	switch (accion)
	{
		case F_FUNCION_DEFAULT:
		break;

		case F_FUNCION_NOTHING:
		break;

		case F_FUNCION_RESET:
			reset_cpu();
		break;

		case F_FUNCION_HARDRESET:
			hard_reset_cpu();
		break;

		case F_FUNCION_NMI:
			generate_nmi();
		break;

		case F_FUNCION_OPENMENU:
			if (util_if_open_just_menu() ) menu_inicio_bucle();
		break;

		case F_FUNCION_OCR:
			textspeech_enviar_speech_pantalla();
		break;

		case F_FUNCION_SMARTLOAD:
			menu_quickload(0);
		break;

		case F_FUNCION_QUICKSAVE:

			snapshot_quick_save(final_name);
			menu_generic_message_format("Quicksave","OK. Snapshot name: %s",final_name);

		break;

		case F_FUNCION_LOADBINARY:
			menu_debug_load_binary(0);
		break;

		case F_FUNCION_SAVEBINARY:
			menu_debug_save_binary(0);
		break;

		case F_FUNCION_ZENG_SENDMESSAGE:
			if (menu_zeng_send_message_cond()) menu_zeng_send_message(0);
		break;

		case F_FUNCION_OSDKEYBOARD:
			menu_onscreen_keyboard(0);
		break;

		case F_FUNCION_OSDTEXTKEYBOARD:
			menu_osd_adventure_keyboard(0);
		break;

		case F_FUNCION_SWITCHBORDER:
			if (menu_interface_border_cond() ) menu_interface_border(0);
		break;

		case F_FUNCION_SWITCHFULLSCREEN:
			menu_interface_fullscreen(0);
		break;		

		case F_FUNCION_RELOADMMC:
			mmc_read_file_to_memory();
		break;

		case F_FUNCION_REINSERTTAPE:
			menu_reinsert_tape();
		break;

		case F_FUNCION_DEBUGCPU:
			menu_debug_registers(0);
		break;

		case F_FUNCION_PAUSE:
			menu_process_f_function_pause();
		break;

		case F_FUNCION_EXITEMULATOR:
			end_emulator();
		break;



	}

}

void menu_process_f_functions(void)
{



	int indice=menu_button_f_function_index;

	enum defined_f_function_ids accion=defined_f_functions_keys_array[indice];

	//printf ("Menu process Tecla: F%d Accion: %s\n",indice+1,defined_f_functions_array[accion].texto_funcion);

	menu_process_f_functions_by_action(accion);

}


void menu_inicio_reset_emulated_keys(void)
{
	//Resetear todas teclas excepto bits de puertos especiales y esperar a no pulsar tecla
	z80_byte p_puerto_especial1,p_puerto_especial2,p_puerto_especial3,p_puerto_especial4;

	p_puerto_especial1=puerto_especial1;
	p_puerto_especial2=puerto_especial2;
	p_puerto_especial3=puerto_especial3;
	p_puerto_especial4=puerto_especial4;

	reset_keyboard_ports();

	//Restaurar estado teclas especiales, para poder esperar a liberar dichas teclas, por ejemplo
	puerto_especial1=p_puerto_especial1;
	puerto_especial2=p_puerto_especial2;
	puerto_especial3=p_puerto_especial3;
	puerto_especial4=p_puerto_especial4;


	//Desactivar fire, por si esta disparador automatico
	joystick_release_fire(1);	

	menu_espera_no_tecla();
}

//menu principal
void menu_inicio(void)
{

	//Pulsado boton salir del emulador, en drivers xwindows, sdl, etc, en casos con menu desactivado, sale del todo
	if (menu_button_exit_emulator.v && (menu_desactivado.v || menu_desactivado_andexit.v)
		) {
		end_emulator();
	}

	//Menu desactivado y volver
	if (menu_desactivado.v) {
		menu_inicio_pre_retorno_reset_flags();

    	menu_set_menu_abierto(0);		
		return;
	}

	//Menu desactivado y salida del emulador
	if (menu_desactivado_andexit.v) end_emulator();

	//No permitir aparecer osd keyboard desde aqui. Luego se reactiva en cada gestion de tecla
	osd_kb_no_mostrar_desde_menu=1;

	menu_contador_teclas_repeticion=CONTADOR_HASTA_REPETICION;


	//Resetear todas teclas. Por si esta el spool file activo. Conservando estado de tecla ESC pulsada o no
	/*z80_byte estado_ESC=puerto_especial1&1;
	reset_keyboard_ports();

	//estaba pulsado ESC
	if (!estado_ESC) puerto_especial1 &=(255-1);

	*/

	/*No liberar teclas ni esperar a no pulsar teclas si solo hay evento printe, etc
	  Pero tener en cuenta que en los eventos se pueden abrir menus tambien
	/*/

	int liberar_teclas_y_esperar=1; //Si se liberan teclas y se espera a liberar teclado


	


	if (menu_breakpoint_exception.v) {
		if (!debug_if_breakpoint_action_menu(catch_breakpoint_index)) {
			//Accion no es de abrir menu
			/*
			Tecnicamente, haciendo esto, no estamos controlando que se dispare un evento de breakpoin accion, por ejemplo , printe,
			y a la vez, se genere otro evento, por ejemplo quickload. En ese caso sucederia que al llamar a quickload, no se liberarian
			las teclas en la maquina emulada ni se esperaria a no pulsar tecla
			Para evitar este remoto caso, habria que hacer que no se liberen las teclas aqui al principio, sino que cada evento
			libere teclas por su cuenta
			*/
			liberar_teclas_y_esperar=0;
		}
	}

	if (liberar_teclas_y_esperar) {
		menu_inicio_reset_emulated_keys();
	}


	//printf ("before menu_espera_no_tecla\n");

	//Si se ha pulsado tecla de OSD keyboard, al llamar a espera_no_tecla, se abrira osd y no conviene.
	


			//printf ("Event open menu: %d\n",menu_event_open_menu.v);

	//printf ("after menu_espera_no_tecla\n");


        if (!strcmp(scr_driver_name,"stdout")) {
		//desactivar menu multitarea con stdout
		menu_multitarea=0;
        }

	//simpletext no soporta menu
        if (!strcmp(scr_driver_name,"simpletext")) {
		printf ("Can not open menu: simpletext video driver does not support menu.\n");
		menu_inicio_pre_retorno();
		return;
        }



	if (menu_multitarea==0) {
		audio_playing.v=0;
		//audio_thread_finish();
	}


	//quitar splash text por si acaso
	menu_splash_segundos=1;
	reset_splash_text();


	cls_menu_overlay();
    set_menu_overlay_function(normal_overlay_texto_menu);


	//Y refrescar footer. Hacer esto para que redibuje en pantalla y no en layer de mezcla de menu
	//menu_init_footer();
	menu_clear_footer();
	redraw_footer();

	//Establecemos variable de salida de todos menus a 0
	salir_todos_menus=0;


	//Si first aid al inicio
	if (menu_first_aid_must_show_startup) {
		menu_first_aid_must_show_startup=0;
		menu_first_aid_title(string_config_key_aid_startup,"First aid of the day");
		//No mostrara nada mas que esto y luego volvera del menu
	}	


	if (menu_button_osdkeyboard.v) {
		//menu_espera_no_tecla();
		menu_onscreen_keyboard(0);
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer
		cls_menu_overlay();
  	}


	else {


	//Evento para generar siguiente tecla
	if (menu_button_osd_adv_keyboard_return.v) {
		//printf ("Debe abrir menu adventure keyboard\n");
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

		menu_osd_adventure_keyboard_next();
		//menu_osd_adventure_keyboard(0);
		cls_menu_overlay();

	}

	//Evento de abrir menu adventure text
	if (menu_button_osd_adv_keyboard_openmenu.v) {
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

                menu_osd_adventure_keyboard(0);
                cls_menu_overlay();
						//printf ("Returning from osd keyboard\n");
        }


	//Gestionar pulsaciones directas de teclado o joystick
	if (menu_button_quickload.v) {
		//Pulsado smartload
		//menu_button_quickload.v=0;

		//para evitar que entre con la pulsacion de teclas activa
		//menu_espera_no_tecla_con_repeticion();
		//menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

		menu_quickload(0);
		cls_menu_overlay();
	}

	if (menu_button_exit_emulator.v) {
		//Pulsado salir del emulador
        //para evitar que entre con la pulsacion de teclas activa
        //menu_espera_no_tecla_con_repeticion();
        //menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

        menu_exit_emulator(0);
        cls_menu_overlay();
	}

        if (menu_event_drag_drop.v) {
							debug_printf(VERBOSE_INFO,"Received drag and drop event with file %s",quickload_file);
		//Entrado drag-drop de archivo
                //para evitar que entre con la pulsacion de teclas activa
                //menu_espera_no_tecla_con_repeticion();
                //menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
		quickfile=quickload_file;


		last_filesused_insert(quickload_file); //Agregar a lista de archivos recientes

                if (quickload(quickload_file)) {
                        debug_printf (VERBOSE_ERR,"Unknown file format");

			//menu_generic_message("ERROR","Unknown file format");
                }
		menu_muestra_pending_error_message(); //Si se genera un error derivado del quickload
                cls_menu_overlay();
        }


	//ha saltado un breakpoint
	if (menu_breakpoint_exception.v) {
		//Ver tipo de accion para ese breakpoint
		//printf ("indice breakpoint & accion : %d\n",catch_breakpoint_index);
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd


		//Si accion nula o menu o break
		if (debug_if_breakpoint_action_menu(catch_breakpoint_index)) {

			//menu_espera_no_tecla();
      //desactivamos multitarea, guardando antes estado multitarea
			int antes_menu_multitarea=menu_multitarea;
      			menu_multitarea=0;
      			audio_playing.v=0;
			//printf ("pc: %d\n",reg_pc);

			menu_breakpoint_fired(catch_breakpoint_message);


			menu_debug_registers(0);

			//restaurar estado multitarea

			menu_multitarea=antes_menu_multitarea;

      cls_menu_overlay();


			//Y despues de un breakpoint hacer que aparezca el menu normal y no vuelva a la ejecucion
			if (!salir_todos_menus) menu_inicio_bucle();
		}

		else {
			//Gestion acciones
			debug_run_action_breakpoint(debug_breakpoints_actions_array[catch_breakpoint_index]);
		}


	}

	if (menu_event_remote_protocol_enterstep.v) {
		//Entrada
		//menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

		remote_ack_enter_cpu_step.v=1; //Avisar que nos hemos enterado
		//Mientras no se salga del modo step to step del remote protocol
		while (menu_event_remote_protocol_enterstep.v) {
			timer_sleep(100);

			//Truco para que desde windows se pueda ejecutar el core loop desde aqui cuando zrcp lo llama
			/*if (towindows_remote_cpu_run_loop) {
				remote_cpu_run_loop(towindows_remote_cpu_run_misocket,towindows_remote_cpu_run_verbose,towindows_remote_cpu_run_limite);
				towindows_remote_cpu_run_loop=0;
			}*/
#ifdef MINGW
			int antes_menu_abierto=menu_abierto;
			menu_abierto=0; //Para que no aparezca en gris al refrescar
				scr_refresca_pantalla();
			menu_abierto=antes_menu_abierto;
				scr_actualiza_tablas_teclado();
#endif

		}

		debug_printf (VERBOSE_DEBUG,"Exiting remote enter step from menu");

		//Salida
		cls_menu_overlay();
	}

	if (menu_button_f_function.v) {
		//printf ("pulsada tecl de funcion\n");
		//Entrada
		//menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

		//Procesar comandos F

		//Procesamos cuando se pulsa tecla F concreta
		if (menu_button_f_function_action==0) menu_process_f_functions();
		else {
			//O procesar cuando se envia una accion concreta
			menu_process_f_functions_by_action(menu_button_f_function_action);
			menu_button_f_function_action=0;
		}

		menu_muestra_pending_error_message(); //Si se genera un error derivado de funcion F
		cls_menu_overlay();
	}

	if (menu_event_new_version_show_changes.v) {
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
		menu_event_new_version_show_changes.v=0;
		menu_generic_message_format("Updated version","You have updated ZEsarUX :)\nPlease take a look at the changes:");
		menu_about_changelog(0);

		cls_menu_overlay();
	}

	if (menu_event_new_update.v) {
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
		menu_event_new_update.v=0;
		menu_generic_message_format("New version available","ZEsarUX version %s is available on github",stats_last_remote_version);

		cls_menu_overlay();
	}


	if (menu_event_open_menu.v) {

		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd
		
		//Abrir menu normal
		//printf ("Abrir menu normal\n");
		menu_inicio_bucle();

	}

	}


	//printf ("salir menu\n");


	//Volver
	menu_inicio_pre_retorno();



}



//Escribe bloque de cuadrado de color negro  
void set_splash_zesarux_logo_put_space(int x,int y)
{
	if (!strcmp(scr_driver_name,"aa")) {
		putchar_menu_overlay(x,y,'X',7,0);
	}
	else putchar_menu_overlay(x,y,' ',7,0);
}


//Hace cuadrado de 2x2
void set_splash_zesarux_logo_cuadrado(int x,int y)
{
	set_splash_zesarux_logo_put_space(x,y);
	set_splash_zesarux_logo_put_space(x+1,y);
	set_splash_zesarux_logo_put_space(x,y+1);
	set_splash_zesarux_logo_put_space(x+1,y+1);
}




//Escribe caracter  128 (franja de color-triangulo)
void set_splash_zesarux_franja_color(int x,int y,int tinta, int papel)
{
	if (si_complete_video_driver() ) {
		putchar_menu_overlay(x,y,128,tinta,papel);
	}
	else {
		putchar_menu_overlay(x,y,'/',tinta,7);
	}
}

//Escribe caracter ' ' con color
void set_splash_zesarux_cuadrado_color(int x,int y,int color)
{
	if (si_complete_video_driver() ) {
		putchar_menu_overlay(x,y,' ',0,color);
	}
}

void set_splash_zesarux_franja_color_repetido(int x,int y,int longitud, int color1, int color2)
{

	int j;
	for (j=0;j<longitud;j++) {
		set_splash_zesarux_franja_color(x+j,y-j,color1,color2);
	}

}


int get_zsplash_y_coord(void)
{
	return menu_center_y()-4;
}


//Dibuja el logo pero en diferentes pasos:
//0: solo la z
//1: franja roja
//2: franja roja y amarilla
//3: franja roja y amarilla y verde
//4 o mayor: franja roja y amarilla y verde y cyan
void set_splash_zesarux_logo_paso(int paso)
{
	int x,y;

	int ancho_z=6;
	int alto_z=6;

	int x_inicial=menu_center_x()-ancho_z;  //Centrado
	int y_inicial=get_zsplash_y_coord();

	debug_printf(VERBOSE_DEBUG,"Drawing ZEsarUX splash logo, step %d",paso);


	//Primero todo texto en gris. Envolvemos un poco mas
	for (y=y_inicial-1;y<y_inicial+ancho_z*2+1;y++) {
		for (x=x_inicial-1;x<x_inicial+ancho_z*2+1;x++) {
			putchar_menu_overlay_parpadeo(x,y,' ',0,7,0);


		}
	}


	y=y_inicial;

	//Linea Arriba Z, Abajo
	for (x=x_inicial;x<x_inicial+ancho_z*2;x++) {
		set_splash_zesarux_logo_put_space(x,y);
		set_splash_zesarux_logo_put_space(x,y+1);

		set_splash_zesarux_logo_put_space(x,y+alto_z*2-2);
		set_splash_zesarux_logo_put_space(x,y+alto_z*2-1);
	}

	//Cuadrados diagonales
	y+=2;

	for (x=x_inicial+(ancho_z-2)*2;x>x_inicial;x-=2,y+=2) {
		set_splash_zesarux_logo_cuadrado(x,y);
	}

	//Y ahora las lineas de colores
	//Rojo amarillo verde cyan
	//2      6       4     5

	if (paso==0) return;

	/*

    012345678901
0	XXXXXXXXXXXX
1	XXXXXXXXXXXX
2	        XX
3	        XX /
4	      XX  /
5	      XX / /
6	    XX  / /
7		XX / / /
8	  XX  / / /
9	  XX / / / /
0	XXXXXXXXXXXX
1	XXXXXXXXXXXX

    012345678901
	*/

/*
  RRRY
 RRRYY
RRRYYY

*/

/*
	        XX .
	      XX  .x
	      XX .x.
	    XX  .x.x
		XX .x.x.
	  XX  .x.x.x
	  XX .x.x.x.
	XXXXXXXXXXXX
	XXXXXXXXXXXX

    012345678901
	*/



	int j;


	set_splash_zesarux_franja_color_repetido(x_inicial+5,y_inicial+9,7, 2, 7);

	for (j=0;j<6;j++) {
		set_splash_zesarux_cuadrado_color(x_inicial+6+j,y_inicial+9-j,2);
	}

	//Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
	if (paso==1) {
		if (si_complete_video_driver() ) {
			set_splash_zesarux_franja_color_repetido(x_inicial+7,y_inicial+9,5, 7, 2);
		}
		return;
	}


	set_splash_zesarux_franja_color_repetido(x_inicial+7,y_inicial+9,5, 6, 2);

	for (j=0;j<4;j++) {
		set_splash_zesarux_cuadrado_color(x_inicial+8+j,y_inicial+9-j,6);
	}

	//Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
	if (paso==2) {
		if (si_complete_video_driver() ) {
			set_splash_zesarux_franja_color_repetido(x_inicial+9,y_inicial+9,3, 7, 6);
		}
		return;
	}



	set_splash_zesarux_franja_color_repetido(x_inicial+9,y_inicial+9,3, 4, 6);

	for (j=0;j<2;j++) {
		set_splash_zesarux_cuadrado_color(x_inicial+10+j,y_inicial+9-j,4);
	}

	//Lo que queda a la derecha de esa franja - el udg diagonal con el color de papel igual que tinta anterior, y papel blanco
	if (paso==3) {
		if (si_complete_video_driver() ) {
			set_splash_zesarux_franja_color(x_inicial+ancho_z*2-1,y_inicial+ancho_z*2-3,7,4);
		}
		return;
	}

	set_splash_zesarux_franja_color(x_inicial+ancho_z*2-1,y_inicial+ancho_z*2-3,5,4);

}



char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO]={
    //01234567890123456789012345
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
  	"WXXXXXXXXXXXXXXXXXXXXXXXXW",      
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",		
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",		
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",	
	"WWWWWWWWWWWWWWWWWXXXXWWWWW",			
	"                WXXXXW   W",			
	"                WXXXXW  RW", 		
	"             WWWWXXXXW RRW",		
	"            WXXXXWWWW RRRW",		
	"            WXXXXW   RRRRW",	//10	
	"            WXXXXW  RRRRYW",		
	"         WWWWXXXXW RRRRYYW",		
	"        WXXXXWWWW RRRRYYYW",		
	"        WXXXXW   RRRRYYYYW",		
	"        WXXXXW  RRRRYYYYGW",		
	"     WWWWXXXXW RRRRYYYYGGW",		
	"    WXXXXWWWW RRRRYYYYGGGW",		
	"    WXXXXW   RRRRYYYYGGGGW",		
	"    WXXXXW  RRRRYYYYGGGGCW",		
	"WWWWWXXXXW RRRRYYYYGGGGCCW",    //20
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",		
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",		
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",		
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};


//Retorna color de paleta spectrum segun letra color logo ascii W: white, X: Black, etc
int return_color_zesarux_ascii(char c)
{
	switch (c) {
		case 'W':
			return 7;
		break;

		case 'X':
			return 0;
		break;

		case 'R':
			return 2;
		break;

		case 'G':
			return 4;
		break;

		case 'C':
			return 5;
		break;

		case 'Y':
			return 6;
		break;

		default:
			return 0;
		break;
	}
}

void set_splash_zesarux_logo(void)
{
	splash_zesarux_logo_active=1;
	splash_zesarux_logo_paso=0;
	set_splash_zesarux_logo_paso(splash_zesarux_logo_paso);
}

void set_splash_text(void)
{
	cls_menu_overlay();
	char texto_welcome[40];
	sprintf(texto_welcome," Welcome to ZEsarUX v." EMULATOR_VERSION " ");

	int yinicial=get_zsplash_y_coord()-6;

	//centramos texto
	int x=menu_center_x()-strlen(texto_welcome)/2;
	if (x<0) x=0;

	menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_welcome);

	set_splash_zesarux_logo();


        char texto_edition[40];
        sprintf(texto_edition," " EMULATOR_EDITION_NAME " ");

		int longitud_texto=strlen(texto_edition);
		//temporal, como estamos usando parpadeo mediante caracteres ^^, no deben contar en la longitud
		//cuando no se use parpadeo, quitar esta resta
		//longitud_texto -=4;

        //centramos texto
        x=menu_center_x()-longitud_texto/2;
        if (x<0) x=0;

        menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_edition);




	char texto_esc_menu[32];
	sprintf(texto_esc_menu," Press %s for menu ",openmenu_key_message);
	longitud_texto=strlen(texto_esc_menu);
        x=menu_center_x()-longitud_texto/2;
        if (x<0) x=0;	
	menu_escribe_texto(x,yinicial++,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,texto_esc_menu);

	set_menu_overlay_function(normal_overlay_texto_menu);
	menu_splash_text_active.v=1;
	menu_splash_segundos=5;


	//Enviar texto de bienvenida tambien a speech
	//stdout y simpletext no
	if (!strcmp(scr_driver_name,"stdout")) return;
	if (!strcmp(scr_driver_name,"simpletext")) return;

	textspeech_print_speech(texto_welcome);
	textspeech_print_speech(texto_esc_menu);

}

int first_time_menu_footer_f5_menu=1;

void reset_splash_text(void)
{
	if (menu_splash_text_active.v==1) {

		menu_splash_segundos--;
		if (menu_splash_segundos==0) {
			reset_splash_zesarux_logo();
			menu_splash_text_active.v=0;
			cls_menu_overlay();
			reset_menu_overlay_function();
			debug_printf (VERBOSE_DEBUG,"End splash text");

			//Quitamos el splash text pero dejamos el F5 menu abajo en el footer, hasta que algo borre ese mensaje
			//(por ejemplo que cargamos una cinta/snap con configuracion y genera mensaje en texto inverso en el footer)
			if (first_time_menu_footer_f5_menu) {
				menu_footer_f5_menu();
				first_time_menu_footer_f5_menu=0; //Solo mostrarlo una sola vez
			}



			//abrir el menu si hay first aid en startup disponible
			//Para que aparezca el mensaje del dia, tiene que estar habilitado el setting de welcome message
			//Si no, no llegara aqui nunca
			if (menu_first_aid_startup) menu_first_aid_random_startup();
		}

		else {
			if (splash_zesarux_logo_active) {
				splash_zesarux_logo_paso++;
				set_splash_zesarux_logo_paso(splash_zesarux_logo_paso);
			}
		}


	}
}

#define FILESEL_ANCHO 30
#define FILESEL_INICIAL_ANCHO 30
#define FILESEL_MAX_ANCHO OVERLAY_SCREEN_MAX_WIDTH

#define FILESEL_INICIAL_ALTO 23

#define FILESEL_INICIAL_X (menu_center_x()-FILESEL_INICIAL_ANCHO/2)
#define FILESEL_INICIAL_Y (menu_center_y()-FILESEL_INICIAL_ALTO/2)

#define FILESEL_INICIO_DIR 4

#define ZXVISION_POS_FILTER 6
#define ZXVISION_POS_LEYENDA 7




void zxvision_menu_filesel_print_filters(zxvision_window *ventana,char *filtros[])
{


        if (menu_filesel_show_utils.v) return; //Si hay utilidades activas, no mostrar filtros

        //texto para mostrar filtros. darle bastante margen aunque no quepa en pantalla
        char buffer_filtros[FILESEL_MAX_ANCHO+1]; //+1 para el 0 final


        char *f;

        int i,p;
        p=0;
        sprintf(buffer_filtros,"Filter: ");

        p=p+8;  //8 es lo que ocupa el texto "Filter: "


        for (i=0;filtros[i];i++) {
                //si filtro es "", significa todo (*)

                f=filtros[i];
                if (f[0]==0) f="*";

                //copiamos
                //sprintf(&buffer_filtros[p],"*.%s ",f);
                sprintf(&buffer_filtros[p],"%s ",f);
                p=p+strlen(f)+1;

        }

//Si texto filtros pasa del tope, rellenar con "..."
		int max_visible=(ventana->visible_width)-2;
	
        if (p>max_visible && max_visible>=3) {
                p=max_visible;
                buffer_filtros[p-1]='.';
                buffer_filtros[p-2]='.';
                buffer_filtros[p-3]='.';
        }


        buffer_filtros[p]=0;


        //borramos primero con espacios


	int posicion_filtros=ZXVISION_POS_FILTER;


	zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros,"");


        //y luego escribimos


        //si esta filesel_zona_pantalla=2, lo ponemos en otro color. TODO
        int inverso=0;
        if (filesel_zona_pantalla==2) inverso=1;



	int tinta=ESTILO_GUI_TINTA_NORMAL;
	int papel=ESTILO_GUI_PAPEL_NORMAL;

	if (inverso) {
		tinta=ESTILO_GUI_TINTA_SELECCIONADO;
		papel=ESTILO_GUI_PAPEL_SELECCIONADO;
	}


	zxvision_print_string(ventana,1,posicion_filtros,tinta,papel,0,buffer_filtros);
}



void zxvision_menu_filesel_print_legend(zxvision_window *ventana)
{

                //Forzar a mostrar atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
                menu_writing_inverse_color.v=1;

	int posicion_leyenda=ZXVISION_POS_LEYENDA;
	int posicion_filtros=ZXVISION_POS_FILTER;

	char leyenda_inferior[64];

	/*
#ifdef MINGW

			//01234567890123456789012345678901
			// TAB: Section R: Recent D: Drive
	sprintf (leyenda_inferior,"~~T~~A~~B:Section ~~Recent ~~Drive");
#else
	sprintf (leyenda_inferior,"~~T~~A~~B: Section ~~R: Recent");
#endif

*/

	//Drive también mostrado en Linux y Mac
			//01234567890123456789012345678901
			// TAB: Section R: Recent D: Drive
	sprintf (leyenda_inferior,"~^T~^A~^B:Section ~^Recent ~^Drives");	

	zxvision_print_string_defaults_fillspc(ventana,1,posicion_leyenda,leyenda_inferior);


        if (menu_filesel_show_utils.v) {


                                                                //    01234  567890  12345  678901  2345678901
                zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros-1,"~^View ~^Trunc D~^El M~^Kdr C~^Onv ~^Inf");
                zxvision_print_string_defaults_fillspc(ventana,1,posicion_filtros,"~^Copy ~^Move Re~^N ~^Paste ~^Filemem");

        }

                //Restaurar comportamiento mostrar atajos
                menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;
}



filesel_item *menu_get_filesel_item(int index)
{
	filesel_item *p;

	p=primer_filesel_item;

	int i;

	for(i=0;i<index;i++) {
		p=p->next;
	}

	return p;

}

//Dice si archivo es de tipo comprimido/empaquetado. filename tiene que ser sin directorio
int menu_util_file_is_compressed(char *filename)
{
		//Si seleccion es archivo comprimido
							if (
							    //strstr(item_seleccionado->d_name,".zip")!=NULL ||
							    !util_compare_file_extension(filename,"zip") ||
                                                            !util_compare_file_extension(filename,"gz")  ||
                                                            !util_compare_file_extension(filename,"tar") ||
                                                            !util_compare_file_extension(filename,"rar") 


							) {
								return 1;
							}
	else return 0;
}

//obtiene linea a escribir con nombre de archivo + carpeta
void menu_filesel_print_file_get(char *buffer, char *s,unsigned char  d_type,unsigned int max_length_shown)
{
	unsigned int i;

        for (i=0;i<max_length_shown && (s[i])!=0;i++) {
                buffer[i]=s[i];
        }


        //si sobra espacio, rellenar con espacios
        for (;i<max_length_shown;i++) {
                buffer[i]=' ';
        }

        buffer[i]=0;


        //si no cabe, poner puntos suspensivos
        if (strlen(s)>max_length_shown && i>=3) {
                buffer[i-1]='.';
                buffer[i-2]='.';
                buffer[i-3]='.';
        }

    //y si es un directorio (sin nombre nulo ni espacio), escribir "<dir>
	//nota: se envia nombre " " (un espacio) cuando se lista el directorio y sobran lineas en blanco al final

	int test_dir=1;

	if (s[0]==0) test_dir=0;
	if (s[0]==' ' && s[1]==0) test_dir=0;

	if (test_dir) {
	        if (get_file_type(d_type,s) == 2 && i>=5) {
        	        buffer[i-1]='>';
                	buffer[i-2]='r';
	                buffer[i-3]='i';
        	        buffer[i-4]='d';
                	buffer[i-5]='<';
	        }

			//O si es empaquetado
			/*else if (menu_util_file_is_compressed(s) && i>=5) {
				    buffer[i-1]='>';
                	buffer[i-2]='p';
	                buffer[i-3]='x';
        	        buffer[i-4]='e';
                	buffer[i-5]='<';
			}*/
	}


}

//escribe el nombre de archivo o carpeta

//Margen de 8 lineas (4+4) de leyendas
#define ZXVISION_FILESEL_INITIAL_MARGIN 8

void zxvision_menu_filesel_print_file(zxvision_window *ventana,char *s,unsigned char  d_type,unsigned int max_length_shown,int y)
{

        char buffer[PATH_MAX];



        menu_filesel_print_file_get(buffer, s, d_type, max_length_shown);


	zxvision_print_string_defaults_fillspc(ventana,1,y+ZXVISION_FILESEL_INITIAL_MARGIN,buffer);	
}




void menu_filesel_switch_filters(void)
{

	//si filtro inicial, ponemos el *.*
	if (filesel_filtros==filesel_filtros_iniciales)
		filesel_filtros=filtros_todos_archivos;

	//si filtro *.* , ponemos el filtro inicial
	else filesel_filtros=filesel_filtros_iniciales;

}

void menu_filesel_chdir(char *dir)
{
	chdir(dir);
}

/*char menu_minus_letra(char letra)
{
	if (letra>='A' && letra<='Z') letra=letra+('a'-'A');
	return letra;
}*/



void zxvision_menu_filesel_localiza_letra(zxvision_window *ventana,char letra)
{

        int i;
        filesel_item *p;
        p=primer_filesel_item;

        for (i=0;i<filesel_total_items;i++) {
                if (letra_minuscula(p->d_name[0])>=letra_minuscula(letra)) {
                        filesel_linea_seleccionada=0;
                        filesel_archivo_seleccionado=i;
			ventana->cursor_line=i;
			zxvision_set_offset_y_or_maximum(ventana,i);
			//printf ("linea seleccionada en localizacion: %d\n",i);
                        return;
                }


                p=p->next;
        }

}



void zxvision_menu_filesel_localiza_archivo(zxvision_window *ventana,char *nombrebuscar)
{
        debug_printf (VERBOSE_DEBUG,"Searching last file %s",nombrebuscar);
        int i;
        filesel_item *p;
        p=primer_filesel_item;

        for (i=0;i<filesel_total_items;i++) {
                debug_printf (VERBOSE_DEBUG,"File number: %d Name: %s",i,p->d_name);
                //if (menu_minus_letra(p->d_name[0])>=menu_minus_letra(letra)) {
                if (strcasecmp(nombrebuscar,p->d_name)<=0) {
                        filesel_linea_seleccionada=0;
                        filesel_archivo_seleccionado=i;
						ventana->cursor_line=i;
						zxvision_set_offset_y_or_maximum(ventana,i);
                        debug_printf (VERBOSE_DEBUG,"Found at position %d",i);
                        return;
                }


                p=p->next;
        }

}


int si_menu_filesel_no_mas_alla_ultimo_item(int linea)
{
	if (filesel_archivo_seleccionado+linea<filesel_total_items-1) return 1;
	return 0;
}

void file_utils_file_convert(char *fullpath)
{

	//Obtener directorio y archivo
	char archivo[PATH_MAX];
	char directorio[PATH_MAX];

	util_get_file_no_directory(fullpath,archivo);
	util_get_dir(fullpath,directorio);

	//Archivo de destino
	char archivo_destino[PATH_MAX];



	//printf ("convert\n");

	if (!util_compare_file_extension(archivo,"tap")) {
		char *opciones[]={
			"TAP to TZX",
			"TAP to RWA",
			"TAP to WAV",
			NULL};

		int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}

		switch (opcion) {
			case 0:
				sprintf(archivo_destino,"%s/%s.tzx",directorio,archivo);
				util_extract_tap(fullpath,NULL,archivo_destino);
			break;	

			case 1:
				sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
				convert_tap_to_rwa(fullpath,archivo_destino);
			break;

			case 2:
				sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
				convert_any_to_wav(fullpath,archivo_destino);
			break;

		}
	}

        else if (!util_compare_file_extension(archivo,"tzx")) {
                char *opciones[]={
						"TZX to TAP",
                        "TZX to RWA",
			"TZX to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
								util_extract_tzx(fullpath,NULL,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_tzx_to_rwa(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }
/*
extern int convert_smp_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_wav_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_o_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_p_to_rwa_tmpdir(char *origen, char *destino);
*/

        else if (!util_compare_file_extension(archivo,"smp")) {
                char *opciones[]={
                        "SMP to RWA",
			"SMP to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_smp_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }

        else if (!util_compare_file_extension(archivo,"wav")) {
                char *opciones[]={
                        "WAV to RWA",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_wav_to_rwa(fullpath,archivo_destino);
                        break;

                }
        }

        else if (!util_compare_file_extension(archivo,"o")) {
                char *opciones[]={
                        "O to RWA",
			"O to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_o_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }

        else if (!util_compare_file_extension(archivo,"p")) {
                char *opciones[]={
                        "P to RWA",
			"P to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_p_to_rwa(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;

                }
        }

        else if (!util_compare_file_extension(archivo,"pzx")) {
                char *opciones[]={
					"PZX to TAP",
                        "PZX to RWA",
			"PZX to WAV",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.tap",directorio,archivo);
								util_extract_pzx(fullpath,NULL,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.rwa",directorio,archivo);
                                convert_pzx_to_rwa(fullpath,archivo_destino);
                        break;

                        case 2:
                                sprintf(archivo_destino,"%s/%s.wav",directorio,archivo);
                                convert_any_to_wav(fullpath,archivo_destino);
                        break;
 
                }
        }		


        else if (!util_compare_file_extension(archivo,"hdf")) {
                char *opciones[]={
                        "HDF to IDE",
			"HDF to MMC",
                        NULL};

                int opcion=menu_ask_list_texto("File converter","Select conversion",opciones);
		if (opcion<0) {
			//Salido con ESC
			return;
		}				
                switch (opcion) {
                        case 0:
                                sprintf(archivo_destino,"%s/%s.ide",directorio,archivo);
                                convert_hdf_to_raw(fullpath,archivo_destino);
                        break;

                        case 1:
                                sprintf(archivo_destino,"%s/%s.mmc",directorio,archivo);
                                convert_hdf_to_raw(fullpath,archivo_destino);
                        break;

                }
        }		

	else {
		menu_error_message("No conversion valid for this file type");
		return;
	}

	//Si no hay error
	if (!if_pending_error_message) {
		//char buffer_mensaje_ok[PATH_MAX+1024];
		//sprintf (buffer_mensaje_ok,"File converted to %s",archivo_destino);

		//menu_generic_message_splash("File converter",buffer_mensaje_ok);
		//menu_warn_message(buffer_mensaje_ok);

		menu_generic_message_format("File converter","OK. File converted to %s",archivo_destino);
	}

}

//Cargar archivo en memory zone
void file_utils_file_mem_load(char *archivo)
{
		int tamanyo=get_file_size(archivo);
		//Asignar memoria si no estaba asignada

		int error_limite=0;

		//Max 16 mb  (0x1000000), para no usar mas de 6 digitos al mostrar la direccion
		if (tamanyo>0x1000000) {
			tamanyo=0x1000000;
			error_limite=1;
		}

		//liberar si habia algo
		if (memory_zone_by_file_size>0) {
			debug_printf(VERBOSE_DEBUG,"Freeing previous file memory zone");
			free(memory_zone_by_file_pointer);
		}

		debug_printf(VERBOSE_DEBUG,"Allocating %d bytes for file memory zone",tamanyo);
		memory_zone_by_file_pointer=malloc(tamanyo);
		if (memory_zone_by_file_pointer==NULL) {
			cpu_panic("Can not allocate memory for file read");
		}

		memory_zone_by_file_size=tamanyo;

                FILE *ptr_load;
                ptr_load=fopen(archivo,"rb");

                if (!ptr_load) {
                        debug_printf (VERBOSE_ERR,"Unable to open file %s",archivo);
                        return;
                }

/*
extern char memory_zone_by_file_name[];
extern z80_byte *memory_zone_by_file_pointer;
extern int memory_zone_by_file_size;
*/

		//Copiamos el nombre del archivo aunque de momento no lo uso
		strcpy(memory_zone_by_file_name,archivo);


                int leidos=fread(memory_zone_by_file_pointer,1,tamanyo,ptr_load);
                if (leidos!=tamanyo) {
                        debug_printf (VERBOSE_ERR,"Error reading file. Bytes read: %d bytes",leidos);
                }

		fclose(ptr_load);

		if (error_limite) menu_warn_message("File too big. Reading first 16 Mb");
		else menu_generic_message_splash("File memory zone","File loaded to File memory zone");
}


//parametro rename: 
//si 0, move
//si 1, es rename
//si 2, copy
void file_utils_move_rename_copy_file(char *archivo,int rename_move)
{
	char nombre_sin_dir[PATH_MAX];
	char directorio[PATH_MAX];
	char nombre_final[PATH_MAX];

	util_get_dir(archivo,directorio);
	util_get_file_no_directory(archivo,nombre_sin_dir);



	int ejecutar_accion=1;

	//Rename
	if (rename_move==1) {
		menu_ventana_scanf("New name",nombre_sin_dir,PATH_MAX);
		sprintf(nombre_final,"%s/%s",directorio,nombre_sin_dir);
	}

	//Copy or move
	else if (rename_move==2 || rename_move==0) {
		//Move or copy
		char *filtros[2];

       	 	filtros[0]="nofiles";
        	filtros[1]=0;


        	//guardamos directorio actual
        	char directorio_actual[PATH_MAX];
        	getcwd(directorio_actual,PATH_MAX);

        	int ret;


        	char nada[PATH_MAX];


        	//Ocultar utilidades
        	menu_filesel_show_utils.v=0;
        	ret=menu_filesel("Enter dir & press ESC",filtros,nada);
        	//Volver a mostrar utilidades
        	menu_filesel_show_utils.v=1;


        	//Si sale con ESC
        	if (ret==0) {

        		//Move
                      	if (rename_move==0) debug_printf (VERBOSE_DEBUG,"Move file %s to directory %s",archivo,menu_filesel_last_directory_seen);

                      	//Copy
                      	if (rename_move==2) debug_printf (VERBOSE_DEBUG,"Copy file %s to directory %s",archivo,menu_filesel_last_directory_seen);
                	sprintf(nombre_final,"%s/%s",menu_filesel_last_directory_seen,nombre_sin_dir);

        	}
        	else {
        		//TODO: hacer de manera facil que menu_filesel no deje seleccionar archivos con enter y solo deje salir con ESC
        		menu_warn_message("You must select the directory exiting with ESC key. Aborting!");
        		ejecutar_accion=0;
        	}


        	//volvemos a directorio inicial
        	menu_filesel_chdir(directorio_actual);
	}

	if (ejecutar_accion) {
		debug_printf (VERBOSE_INFO,"Original name: %s dir: %s new name %s final name %s"
				,archivo,directorio,nombre_sin_dir,nombre_final);


		if (menu_confirm_yesno_texto("Confirm operation","Sure?")==0) return;

		//Si existe archivo destino
		if (si_existe_archivo(nombre_final)) {
			if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;
		}

		if (rename_move==2) util_copy_file(archivo,nombre_final);
		else rename(archivo,nombre_final);


		//Copy
		if (rename_move==2) menu_generic_message("Copy file","OK. File copied");
		//Rename
		else if (rename_move==1) menu_generic_message("Rename file","OK. File renamed");
		//Move
		else menu_generic_message("Move file","OK. File moved");
	}
}



void file_utils_paste_clipboard(void)
{

	if (menu_clipboard_pointer==NULL) {
		debug_printf(VERBOSE_ERR,"Clipboard is empty, you can fill it from a text window and press key c");
		return;
	}

	char directorio_actual[PATH_MAX];
    getcwd(directorio_actual,PATH_MAX);

	char nombre_sin_dir[PATH_MAX];
	char nombre_final[PATH_MAX];


	nombre_sin_dir[0]=0;
	menu_ventana_scanf("Filename?",nombre_sin_dir,PATH_MAX);
	sprintf(nombre_final,"%s/%s",directorio_actual,nombre_sin_dir);


	//Ver si archivo existe y preguntar
	if (si_existe_archivo(nombre_final)) {

		if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) return;

    }


	menu_paste_clipboard_to_file(nombre_final);

	menu_generic_message_splash("Clipboard","File saved with ZEsarUX clipboard contents");


}



void zxvision_menu_filesel_cursor_arriba(zxvision_window *ventana)
{
	//ver que no sea primer archivo
    if (filesel_archivo_seleccionado+filesel_linea_seleccionada!=0) {
	 ventana->cursor_line--;
                                                //ver si es principio de pantalla
                                                if (filesel_linea_seleccionada==0) {
							zxvision_send_scroll_up(ventana);
                                                        filesel_archivo_seleccionado--;
                                                }
                                                else {
                                                        filesel_linea_seleccionada--;
                                                }
                                        }

	//Por si el cursor no esta visible en pantalla (al haberse hecho scroll con raton)	
	if (zxvision_adjust_cursor_top(ventana)) {
		zxvision_send_scroll_up(ventana);
		filesel_linea_seleccionada=0;
		filesel_archivo_seleccionado=ventana->cursor_line;
	}
}

int zxvision_get_filesel_alto_dir(zxvision_window *ventana)
{
	return ventana->visible_height - ventana->upper_margin - ventana->lower_margin - 2;
}

int zxvision_get_filesel_pos_filters(zxvision_window *ventana)
{
	return ventana->visible_height - 3;
}


void zxvision_menu_filesel_cursor_abajo(zxvision_window *ventana)
{
	//ver que no sea ultimo archivo
	if (si_menu_filesel_no_mas_alla_ultimo_item(filesel_linea_seleccionada)) {
		ventana->cursor_line++;
                                                //ver si es final de pantalla
                                                if (filesel_linea_seleccionada==zxvision_get_filesel_alto_dir(ventana)-1) {
                                                        filesel_archivo_seleccionado++;
							zxvision_send_scroll_down(ventana);
                                                }
                                                else {
                                                        filesel_linea_seleccionada++;
                                                }
                                        }
	//Por si el cursor no esta visible en pantalla (al haberse hecho scroll con raton)									
	if (zxvision_adjust_cursor_bottom(ventana)) {
		zxvision_send_scroll_down(ventana);
		filesel_linea_seleccionada=zxvision_get_filesel_alto_dir(ventana)-1;
		filesel_archivo_seleccionado=ventana->cursor_line-filesel_linea_seleccionada;
	}

}


//liberar memoria usada por la lista de archivos
void menu_filesel_free_mem(void)
{

	filesel_item *aux;
        filesel_item *nextfree;


        aux=primer_filesel_item;

        //do {

	//puede que no haya ningun archivo, por tanto esto es NULL
	//sucede con las carpetas /home en macos por ejemplo
	while (aux!=NULL) {

                //printf ("Liberando %p : %s\n",aux,aux->d_name);
                nextfree=aux->next;
                free(aux);

                aux=nextfree;
        };
        //} while (aux!=NULL);

	//printf ("fin liberar filesel\n");


}


int menu_filesel_mkdir(char *directory)
{
#ifndef MINGW
     int tmpdirret=mkdir(directory,S_IRWXU);
#else
	int tmpdirret=mkdir(directory);
#endif

     if (tmpdirret!=0 && errno!=EEXIST) {
                  debug_printf (VERBOSE_ERR,"Error creating %s directory : %s",directory,strerror(errno) );
     }

	return tmpdirret;

}

void menu_filesel_exist_ESC(void)
{
                                                cls_menu_overlay();
                                                menu_espera_no_tecla();
                                                menu_filesel_chdir(filesel_directorio_inicial);
                                                menu_filesel_free_mem();
}

//Elimina la extension de un nombre
void menu_filesel_file_no_ext(char *origen, char *destino)
{



	int j;

        j=strlen(origen);

	//buscamos desde el punto final

        for (;j>=0 && origen[j]!='.';j--);

	if (j==-1) {
		//no hay punto
		j=strlen(origen);
	}

	//printf ("posicion final: %d\n",j);


	//y copiamos
	for (;j>0;j--) {
		*destino=*origen;
		origen++;
		destino++;
	}

	//Y final de cadena
	*destino = 0;

	//printf ("archivo sin extension: %s\n",copiadestino);

}


#define COMPRESSED_ZIP 1
#define COMPRESSED_GZ  2
#define COMPRESSED_TAR 3
#define COMPRESSED_RAR 4

int menu_filesel_is_compressed(char *archivo)
{
  int compressed_type=0;

	//if ( strstr(archivo,".zip")!=NULL || strstr(archivo,".ZIP")!=NULL) {
	if ( !util_compare_file_extension(archivo,"zip") ) {
		debug_printf (VERBOSE_DEBUG,"Is a zip file");
		compressed_type=COMPRESSED_ZIP;
	}

        else if ( !util_compare_file_extension(archivo,"gz") ) {
                debug_printf (VERBOSE_DEBUG,"Is a gz file");
                compressed_type=COMPRESSED_GZ;
        }

        else if ( !util_compare_file_extension(archivo,"tar") ) {
                debug_printf (VERBOSE_DEBUG,"Is a tar file");
                compressed_type=COMPRESSED_TAR;
        }

        else if ( !util_compare_file_extension(archivo,"rar") ) {
                debug_printf (VERBOSE_DEBUG,"Is a rar file");
                compressed_type=COMPRESSED_RAR;
        }

	return compressed_type;	
}

//Devuelve 0 si ok

int menu_filesel_uncompress (char *archivo,char *tmpdir)
{


//descomprimir creando carpeta TMPDIR_BASE/zipname
 sprintf (tmpdir,"%s/%s",get_tmpdir_base(),archivo);
 menu_filesel_mkdir(tmpdir);


  int compressed_type=menu_filesel_is_compressed(archivo);


//descomprimir
 char uncompress_command[PATH_MAX];

 char uncompress_program[PATH_MAX];

		char archivo_no_ext[PATH_MAX];

switch (compressed_type) {

	case COMPRESSED_ZIP:
		//sprintf (uncompress_program,"/usr/bin/unzip");
 		//-n no sobreescribir
		//sprintf (uncompress_command,"unzip -n \"%s\" -d %s",archivo,tmpdir);

		/*sprintf (uncompress_program,"%s",external_tool_unzip);
 		//-n no sobreescribir
		sprintf (uncompress_command,"%s -n \"%s\" -d %s",external_tool_unzip,archivo,tmpdir);*/


		//printf ("Using internal zip decompressor\n");
		util_extract_zip(archivo,tmpdir);
		return 0;
	break;

        case COMPRESSED_GZ:
		menu_filesel_file_no_ext(archivo,archivo_no_ext);
                //sprintf (uncompress_program,"/bin/gunzip");
                //sprintf (uncompress_command,"gunzip -c \"%s\" > \"%s/%s\" ",archivo,tmpdir,archivo_no_ext);
                sprintf (uncompress_program,"%s",external_tool_gunzip);
                sprintf (uncompress_command,"%s -c \"%s\" > \"%s/%s\" ",external_tool_gunzip,archivo,tmpdir,archivo_no_ext);
        break;

        case COMPRESSED_TAR:
                //sprintf (uncompress_program,"/bin/tar");
                //sprintf (uncompress_command,"tar -xvf \"%s\" -C %s",archivo,tmpdir);
                sprintf (uncompress_program,"%s",external_tool_tar);
                sprintf (uncompress_command,"%s -xvf \"%s\" -C %s",external_tool_tar,archivo,tmpdir);
        break;

        case COMPRESSED_RAR:
                //sprintf (uncompress_program,"/usr/bin/unrar");
                //sprintf (uncompress_command,"unrar x -o+ \"%s\" %s",archivo,tmpdir);
                sprintf (uncompress_program,"%s",external_tool_unrar);
                sprintf (uncompress_command,"%s x -o+ \"%s\" %s",external_tool_unrar,archivo,tmpdir);
        break;




	default:
		debug_printf(VERBOSE_ERR,"Unknown compressed file");
		return 1;
	break;

}

 //unzip
 struct stat buf_stat;


 	if (stat(uncompress_program, &buf_stat)!=0) {
		debug_printf(VERBOSE_ERR,"Unable to find uncompress program: %s",uncompress_program);
		return 1;

	}

	debug_printf (VERBOSE_DEBUG,"Running %s",uncompress_command);

	if (system (uncompress_command)==-1) {
		debug_printf (VERBOSE_DEBUG,"Error running command %s",uncompress_command);
		return 1;
 	}


	return 0;

}


//Expandir archivos (no descomprimir, sino expandir por ejemplo un tap o un hdf)
//Devuelve 0 si ok
int menu_filesel_expand(char *archivo,char *tmpdir)
{

	sprintf (tmpdir,"%s/%s",get_tmpdir_base(),archivo);
	menu_filesel_mkdir(tmpdir);


        if (!util_compare_file_extension(archivo,"hdf") ) {
                debug_printf (VERBOSE_DEBUG,"Is a hdf file");
                return util_extract_hdf(archivo,tmpdir);
        }

        else if (!util_compare_file_extension(archivo,"tap") ) {
                debug_printf (VERBOSE_DEBUG,"Is a tap file");
        	return util_extract_tap(archivo,tmpdir,NULL);
        }

        else if (!util_compare_file_extension(archivo,"tzx") ) {
                debug_printf (VERBOSE_DEBUG,"Is a tzx file");
                return util_extract_tzx(archivo,tmpdir,NULL);
        }


        else if (!util_compare_file_extension(archivo,"pzx") ) {
                debug_printf (VERBOSE_DEBUG,"Is a pzx file");
                return util_extract_pzx(archivo,tmpdir,NULL);
        }		

        else if (!util_compare_file_extension(archivo,"trd") ) {
                debug_printf (VERBOSE_DEBUG,"Is a trd file");
                return util_extract_trd(archivo,tmpdir);
        }		

        else if (!util_compare_file_extension(archivo,"dsk") ) {
                debug_printf (VERBOSE_DEBUG,"Is a dsk file");
                return util_extract_dsk(archivo,tmpdir);
        }		

        else if (
			!util_compare_file_extension(archivo,"epr")  ||
			!util_compare_file_extension(archivo,"eprom")  ||
			!util_compare_file_extension(archivo,"flash")  
		) {
                debug_printf (VERBOSE_DEBUG,"Is a Z88 card file");
                return util_extract_z88_card(archivo,tmpdir);
        }				

        else if (!util_compare_file_extension(archivo,"p") ) {
                debug_printf (VERBOSE_DEBUG,"Is a P file");
        	return util_extract_p(archivo,tmpdir);
        }	

        else if (!util_compare_file_extension(archivo,"o") ) {
                debug_printf (VERBOSE_DEBUG,"Is a O file");
        	return util_extract_o(archivo,tmpdir);
        }				

        else if ( !util_compare_file_extension(archivo,"mdv") ) {
                debug_printf (VERBOSE_DEBUG,"Is a mdv file");
                return util_extract_mdv(archivo,tmpdir);
        }

        else if ( !util_compare_file_extension(archivo,"scl") ) {
                debug_printf (VERBOSE_DEBUG,"Is a scl file");
                return util_extract_scl(archivo,tmpdir);
        }		

		else if (menu_filesel_is_compressed(archivo)) {
			debug_printf (VERBOSE_DEBUG,"Expanding Compressed file");
			return menu_filesel_uncompress(archivo,tmpdir);
		}

		//else debug_printf(VERBOSE_DEBUG,"Don't know how to expand that file");


        return 1;


}


//escribir archivo que indique directorio anterior
void menu_filesel_write_file_last_dir(char *directorio_anterior)
{
	debug_printf (VERBOSE_DEBUG,"Writing temp file " MENU_LAST_DIR_FILE_NAME " to tell last directory before uncompress (%s)",directorio_anterior);


    FILE *ptr_lastdir;
    ptr_lastdir=fopen(MENU_LAST_DIR_FILE_NAME,"wb");

	if (ptr_lastdir!=NULL) {
	        fwrite(directorio_anterior,1,strlen(directorio_anterior),ptr_lastdir);
        	fclose(ptr_lastdir);
	}
}

//leer contenido de archivo que indique directorio anterior
//Devuelve 0 si OK. 1 si error
int menu_filesel_read_file_last_dir(char *directorio_anterior)
{

        FILE *ptr_lastdir;
        ptr_lastdir=fopen(MENU_LAST_DIR_FILE_NAME,"rb");

	if (ptr_lastdir==NULL) {
		debug_printf (VERBOSE_DEBUG,"Error opening " MENU_LAST_DIR_FILE_NAME);
                return 1;
        }


        int leidos=fread(directorio_anterior,1,PATH_MAX,ptr_lastdir);
        fclose(ptr_lastdir);

	if (leidos) {
		directorio_anterior[leidos]=0;
	}
	else {
		if (leidos==0) debug_printf (VERBOSE_DEBUG,"Error. Read 0 bytes from " MENU_LAST_DIR_FILE_NAME);
		if (leidos<0) debug_printf (VERBOSE_DEBUG,"Error reading from " MENU_LAST_DIR_FILE_NAME);
		return 1;
	}

	return 0;
}


void menu_textspeech_say_current_directory(void)
{

	//printf ("\nmenu_textspeech_say_current_directory\n");

        char current_dir[PATH_MAX];
        char buffer_texto[PATH_MAX+200];
        getcwd(current_dir,PATH_MAX);

        sprintf (buffer_texto,"Current directory: %s",current_dir);

	//Quiero que siempre se escuche
	menu_speech_tecla_pulsada=0;
	menu_textspeech_send_text(buffer_texto);
}




int zxvision_si_mouse_zona_archivos(zxvision_window *ventana)
{
    int inicio_y_dir=1+FILESEL_INICIO_DIR;

    if (menu_mouse_y>=inicio_y_dir && menu_mouse_y<inicio_y_dir+zxvision_get_filesel_alto_dir(ventana) && menu_mouse_x<ventana->visible_width-1) {
		//printf ("Mouse en zona de archivos\n");
		return 1;
	}
    
	return 0;
}


void zxvision_menu_filesel_print_text_contents(zxvision_window *ventana)
{
	zxvision_print_string_defaults_fillspc(ventana,1,2,"Directory Contents:");
}

void file_utils_info_file(char *archivo)
{

	long int tamanyo=get_file_size(archivo);
	//fecha
       int hora;
        int minutos;
        int segundos;

        int anyo;
        int mes;
        int dia;


        get_file_date_from_name(archivo,&hora,&minutos,&segundos,&dia,&mes,&anyo);



	menu_generic_message_format("Info file","Full path: %s\n\nSize: %ld bytes\nModified: %02d:%02d:%02d %02d/%02d/%02d",
		archivo,tamanyo,hora,minutos,segundos,dia,mes,anyo);

}


//Si hay que iniciar el filesel pero mover el cursor a un archivo concreto
z80_bit menu_filesel_posicionar_archivo={0};
char menu_filesel_posicionar_archivo_nombre[PATH_MAX]="";

void menu_filesel_change_to_tmp(char *tmpdir)
{
                                                                        //cambiar a tmp dir.

                                                                        //Dejar antes un archivo temporal en ese directorio que indique directorio anterior
                                                                        char directorio_actual[PATH_MAX];
                                                                        getcwd(directorio_actual,PATH_MAX);

                                                                        menu_filesel_chdir(tmpdir);

                                                                        //escribir archivo que indique directorio anterior
                                                                        menu_filesel_write_file_last_dir(directorio_actual);

                                                                        menu_filesel_free_mem();
}


void estilo_gui_retorna_nombres(void)
{
	int i;

	for (i=0;i<ESTILOS_GUI;i++) {
		printf ("%s ",definiciones_estilos_gui[i].nombre_estilo);
	}
}

void set_charset(void)
{

	if (estilo_gui_activo==ESTILO_GUI_CPC) char_set=char_set_cpc;
	else if (estilo_gui_activo==ESTILO_GUI_Z88) char_set=char_set_z88;
	else if (estilo_gui_activo==ESTILO_GUI_SAM) char_set=char_set_sam;
	else if (estilo_gui_activo==ESTILO_GUI_MANSOFTWARE) char_set=char_set_mansoftware;
	else if (estilo_gui_activo==ESTILO_GUI_QL) char_set=char_set_ql;
	else if (estilo_gui_activo==ESTILO_GUI_RETROMAC) char_set=char_set_retromac;
	else char_set=char_set_spectrum;
}

void zxvision_menu_print_dir(int inicial,zxvision_window *ventana)
{

	//TODO: no tiene sentido usar variable "inicial"
	inicial=0;

	//printf ("\nmenu_print_dir\n");

	//escribir en ventana directorio de archivos

	//Para speech
	char texto_opcion_activa[PATH_MAX+100]; //Dado que hay que meter aqui el nombre del archivo y un poquito mas de texto
	//Asumimos por si acaso que no hay ninguna activa
	texto_opcion_activa[0]=0;



	filesel_item *p;
	int i;

	int mostrados_en_pantalla=(ventana->visible_height)-10;
	//trucar el maximo en pantalla. dado que somos zxvision, se pueden mostar ya todos en resolucion virtual de ventana
	mostrados_en_pantalla=999999;

	p=menu_get_filesel_item(inicial);

	//Para calcular total de archivos de ese directorio, siguiendo el filtro. Util para mostrar indicador de porcentaje '*'
	//int total_archivos=inicial;

	filesel_total_archivos=inicial;

	for (i=0;p!=NULL;i++,filesel_total_archivos++) {
		//printf ("file: %s\n",p->d_name);

		//Solo hacer esto si es visible en pantalla
		if (i<mostrados_en_pantalla) {
		
		zxvision_menu_filesel_print_file(ventana,p->d_name,p->d_type,(ventana->total_width)-2,i);
		

		//if (filesel_linea_seleccionada==i) {
		if (ventana->cursor_line==i) {
			char buffer[OVERLAY_SCREEN_MAX_WIDTH+1],buffer2[OVERLAY_SCREEN_MAX_WIDTH+1+32];
			

			strcpy(filesel_nombre_archivo_seleccionado,p->d_name);

			//menu_tape_settings_trunc_name(filesel_nombre_archivo_seleccionado,buffer,22);
			//printf ("antes de trunc\n");

			int tamanyo_mostrar=ventana->visible_width-6-1; //6 ocupa el texto "File: "

				menu_tape_settings_trunc_name(filesel_nombre_archivo_seleccionado,buffer,tamanyo_mostrar); 

			sprintf (buffer2,"File: %s",buffer);
			
			zxvision_print_string_defaults_fillspc(ventana,1,1,buffer2);


				debug_printf (VERBOSE_DEBUG,"Selected: %s. filesel_zona_pantalla: %d",p->d_name,filesel_zona_pantalla);
				//Para speech
				//Si estamos en zona central del selector de archivos, decirlo
				if (filesel_zona_pantalla==1) {

	                                if (menu_active_item_primera_vez) {
						

        	                                sprintf (texto_opcion_activa,"Selected item: %s %s",p->d_name,(get_file_type(p->d_type,p->d_name) == 2 ? "directory" : ""));
                	                        menu_active_item_primera_vez=0;
                        	        }

                                	else {
	                                        sprintf (texto_opcion_activa,"%s %s",p->d_name,(get_file_type(p->d_type,p->d_name) == 2 ? "directory" : ""));
        	                        }

				}


		}
		}

		p=p->next;

    }



	//int texto_no_cabe=0;
	filesel_no_cabe_todo=0;

	debug_printf (VERBOSE_DEBUG,"Total files read (applying filters): %d",filesel_total_archivos);
	if (filesel_total_archivos>mostrados_en_pantalla) {
		filesel_no_cabe_todo=1;
	}




	//Imprimir directorio actual
	//primero borrar con espacios

    //menu_escribe_texto_ventana(14,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"               ");


	char current_dir[PATH_MAX];
	char buffer_dir[OVERLAY_SCREEN_MAX_WIDTH+1];
	char buffer3[OVERLAY_SCREEN_MAX_WIDTH+1+32];
	getcwd(current_dir,PATH_MAX);

	menu_tape_settings_trunc_name(current_dir,buffer_dir,ventana->visible_width-14); //14 es lo que ocupa el texto "Current dir: "
	sprintf (buffer3,"Current dir: %s",buffer_dir);
	//menu_escribe_texto_ventana(1,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer3);
	zxvision_print_string_defaults_fillspc(ventana,1,0,buffer3);


                if (texto_opcion_activa[0]!=0) {

			debug_printf (VERBOSE_DEBUG,"Send active line to speech: %s",texto_opcion_activa);
                        //Selected item siempre quiero que se escuche

                        //Guardamos estado actual
                        int antes_menu_speech_tecla_pulsada=menu_speech_tecla_pulsada;
                        menu_speech_tecla_pulsada=0;

                        menu_textspeech_send_text(texto_opcion_activa);

                        //Restauro estado
                        //Pero si se ha pulsado tecla, no restaurar estado
                        //Esto sino provocaria que , por ejemplo, en la ventana de confirmar yes/no,
                        //se entra con menu_speech_tecla_pulsada=0, se pulsa tecla mientras se esta leyendo el item activo,
                        //y luego al salir de aqui, se pierde el valor que se habia metido (1) y se vuelve a poner el 0 del principio
                        //provocando que cada vez que se mueve el cursor, se relea la ventana entera
                        if (menu_speech_tecla_pulsada==0) menu_speech_tecla_pulsada=antes_menu_speech_tecla_pulsada;
                }


}


              //Si en linea de "File"
int menu_filesel_change_zone_if_clicked(zxvision_window *ventana,int *filesel_zona_pantalla,int *tecla)
{
     if (!si_menu_mouse_en_ventana() ) return 0;
	if (!mouse_left) return 0;

	int futura_zona=-1;
    if (menu_mouse_y==2 && menu_mouse_x<ventana->visible_width-1) {
                //printf ("Pulsado zona File\n");
						futura_zona=0;
    }

                //Si en linea de filtros
    if (menu_mouse_y==zxvision_get_filesel_pos_filters(ventana)  && menu_mouse_x<ventana->visible_width-1) {
								//printf ("Pulsado zona Filtros\n");
                                                                futura_zona=2;
    }


		//En zona seleccion archivos
    if (zxvision_si_mouse_zona_archivos(ventana)) {	
							//printf ("En zona seleccion archivos\n");
							futura_zona=1;
		}


	if (futura_zona!=-1) {
		if (futura_zona!=*filesel_zona_pantalla) {
			//Cambio de zona
			menu_reset_counters_tecla_repeticion();
			*filesel_zona_pantalla=futura_zona;
			*tecla=0;
			return 1;
		}
	}




	return 0;
}


//Cambiar unidad Windows
//Retorna 0 si cancelado
char menu_filesel_cambiar_unidad_windows(void)
{

	char buffer_unidades[100]; //Aunque son 26 maximo, pero por si acaso
	int unidades=util_get_available_drives(buffer_unidades);
	if (unidades==0) {
		menu_error_message("No available drives");
		return 0;
	}

	//printf ("total unidades: %d string Unidades: %s 0 %d 1 %d 2 %d 3 %d\n",unidades,buffer_unidades,buffer_unidades[0],buffer_unidades[1],buffer_unidades[2],buffer_unidades[3]);


        menu_item *array_menu_filesel_unidad;
        menu_item item_seleccionado;
        int retorno_menu;

	int menu_filesel_unidad_opcion_seleccionada=0;


	menu_add_item_menu_inicial(&array_menu_filesel_unidad,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	int i;

	for (i=0;i<unidades;i++) {
		char letra=buffer_unidades[i];
		menu_add_item_menu_format(array_menu_filesel_unidad,MENU_OPCION_NORMAL,NULL,NULL,"~~%c:",letra);
		menu_add_item_menu_shortcut(array_menu_filesel_unidad,letra_minuscula(letra));
		menu_add_item_menu_valor_opcion(array_menu_filesel_unidad,letra);
	}

                menu_add_item_menu(array_menu_filesel_unidad,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_filesel_unidad);
                retorno_menu=menu_dibuja_menu(&menu_filesel_unidad_opcion_seleccionada,&item_seleccionado,array_menu_filesel_unidad,"Select Drive" );

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
				//Sacamos la letra del texto mismo
				char unidad=item_seleccionado.valor_opcion;
				//printf ("Leida unidad de menu: %c\n",unidad);
				return unidad;
                }



	return 0;
}

//Retorna 1 si ha realizado cambio cursor. 0 si no
int menu_filesel_set_cursor_at_mouse(zxvision_window *ventana)
{
								int inicio_y_dir=1+FILESEL_INICIO_DIR;
                            //if (menu_mouse_y>=inicio_y_dir && menu_mouse_y<inicio_y_dir+(FILESEL_ALTO-10)) {
                            //printf ("Dentro lista archivos\n");

                            //Ver si linea dentro de rango
                            int linea_final=menu_mouse_y-inicio_y_dir;

                            //Si esta en la zona derecha de selector de porcentaje no hacer nada
                            
                            //if (menu_mouse_x==FILESEL_ANCHO-1) return 0;
							if (menu_mouse_x==(ventana->visible_width)-1) return 0;
							

                            //filesel_linea_seleccionada=menu_mouse_y-inicio_y_dir;

                            if (si_menu_filesel_no_mas_alla_ultimo_item(linea_final-1)) {

								//Ajustar cursor ventana

								//if (zxvision_cursor_out_view(ventana)) {
								filesel_archivo_seleccionado=ventana->offset_y;
								ventana->cursor_line=filesel_archivo_seleccionado;
								//}

								//ventana->cursor_line -=filesel_linea_seleccionada;
	
								//printf ("Seleccionamos item %d\n",linea_final);
                                filesel_linea_seleccionada=linea_final;

								ventana->cursor_line +=filesel_linea_seleccionada;
                                menu_speech_tecla_pulsada=1;
								return 1;
                            }
                            else {
                                //printf ("Cursor mas alla del ultimo item\n");
                            }

	return 0;

}

void menu_filesel_recent_files_clear(MENU_ITEM_PARAMETERS)
{
	last_filesused_clear();
	menu_generic_message_splash("Clear List","OK. List cleared");
}

char *menu_filesel_recent_files(void)
{
	menu_item *array_menu_recent_files;
    menu_item item_seleccionado;
    int retorno_menu;


	menu_add_item_menu_inicial(&array_menu_recent_files,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

    char string_last_file_shown[29];


    int i;
	int hay_alguno=0;
    for (i=0;i<MAX_LAST_FILESUSED;i++) {
		if (last_files_used_array[i][0]!=0) {

			//Mostrar solo nombre de archivo sin path
			char archivo_sin_dir[PATH_MAX];
			util_get_file_no_directory(last_files_used_array[i],archivo_sin_dir);

            menu_tape_settings_trunc_name(archivo_sin_dir,string_last_file_shown,29);
			menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,NULL,NULL,string_last_file_shown);

			//Agregar tooltip con ruta entera
			menu_add_item_menu_tooltip(array_menu_recent_files,last_files_used_array[i]);

			hay_alguno=1;
		}
	}

	if (!hay_alguno) menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,NULL,NULL,"<Empty List>");

	menu_add_item_menu(array_menu_recent_files,"",MENU_OPCION_SEPARADOR,NULL,NULL);
	menu_add_item_menu_format(array_menu_recent_files,MENU_OPCION_NORMAL,menu_filesel_recent_files_clear,NULL,"Clear List");

    menu_add_item_menu(array_menu_recent_files,"",MENU_OPCION_SEPARADOR,NULL,NULL);
    menu_add_ESC_item(array_menu_recent_files);

    retorno_menu=menu_dibuja_menu(&menu_recent_files_opcion_seleccionada,&item_seleccionado,array_menu_recent_files,"Recent files" );

    if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

		//Seleccion de borrar lista
		if (item_seleccionado.menu_funcion!=NULL) {
            item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
        }

		//Seleccion de un archivo
		else if (hay_alguno) {
        	int indice=menu_recent_files_opcion_seleccionada;

//lastfilesuser_scrolldown
//Quitar el que hay ahi, desplazando hacia abajo y ponerlo arriba del todo
//Copiarlo temporamente a otro sitio
		char buffer_recent[PATH_MAX];
		strcpy(buffer_recent,last_files_used_array[indice]);

		//Movemos el trozo desde ahi hasta arriba
		lastfilesuser_scrolldown(0,indice);

		//Y lo metemos arriba del todo
		strcpy(last_files_used_array[0],buffer_recent);

		//Por tanto el indice final sera 0
		indice=0;

		//Y cursor ponerloa arriba entonces tambien
		menu_recent_files_opcion_seleccionada=0;

			debug_printf (VERBOSE_INFO,"Returning recent file %s",last_files_used_array[indice]);
			return last_files_used_array[indice];
		}
	}

	return NULL;

}



struct s_first_aid_list first_aid_list[MAX_MENU_FIRST_AID];


int total_first_aid=0;
 
void menu_first_aid_add(char *key_string,int *puntero_setting,char *texto_opcion,int si_startup)
{

	if (total_first_aid==MAX_MENU_FIRST_AID) return; //error

	//first_aid_list[total_first_aid].indice_setting=indice_aid;
	strcpy(first_aid_list[total_first_aid].config_name,key_string);
	first_aid_list[total_first_aid].puntero_setting=puntero_setting;
	first_aid_list[total_first_aid].texto_opcion=texto_opcion;
	first_aid_list[total_first_aid].si_startup=si_startup;

	total_first_aid++;
}


//No mostrar la opcion. por defecto a 0 (mostrarla)
int first_aid_no_filesel_uppercase_keys=0;
char *first_aid_string_filesel_uppercase_keys="If you want to select a file by its initial letter, please press the letter as it is. "
							"If you want to execute actions shown in the bottom of the window, in inverted colour, please press shift+letter";

int first_aid_no_filesel_enter_key=0;
char *first_aid_string_filesel_enter_key="Press ENTER to select a file or change directory.\n"
							"Press Space to expand files, like tap, tzx, trd, scl... etc and also all the compressed supported files";							


int first_aid_no_ssl_wos=0;
char *first_aid_string_no_ssl_wos="Warning: as SSL support is not compiled, results newer than 2013 may fail";	


int first_aid_no_smartload=0;
char *first_aid_string_smartload="This smartload window allows you to load any file known by the emulator. Just select it and go!\n"
							"Press TAB to change between areas in the file selector";


int first_aid_no_initial_menu=0;
char *first_aid_string_initial_menu="This is the Main Menu. You can select an item by using cursor keys and mouse. Most of them have help, "
	"try pressing F1. Also, many have tooltip help, that means if you don't press a key, it will appear a tooltip "
	"about what the item does. ESC or right mouse button closes a menu, you can also close it by pressing the top-left button in the window. "
	"You can also use your mouse to resize or move windows";

int first_aid_no_startup_aid=0;
char *first_aid_string_startup_aid="This is a first aid help message. You will be shown some of these at the emulator startup, but also "
	"when opening some menus. All of them are suggestions, advices and pieces of help of ZEsarUX. You can disable entirely them by going to Settings-> "
	"GUI Settings-> First aid help";

int first_aid_no_multiplattform=0;
char *first_aid_string_multiplattform="Do you know that ZEsarUX is multiplattform? There are main versions for Linux, Mac, Windows and Raspberry pi. "
 "But also for Retropie/EmulationStation, Open Pandora, PocketCHIP and MorhpOS. "
 "You can even compile it by yourself, you only need a compatible Unix environment to do that!";

int first_aid_no_accessibility=0;
char *first_aid_string_accessibility="Do you know ZEsarUX has accessibility settings? You can hear the menu or even hear text adventures games. "
 "You will only need an external text-to-speech program to do that. Please read the FAQ to know more";

int first_aid_no_documentation=0;
char *first_aid_string_documentation="You can find a lot of info, videos, documentation, etc on:\n"
  "-Menu entries with help (pressing F1 in most of them) and item tooltips when no key is pressed\n"
  "-FAQ file\n"
  "-docs folder (on the extra package)\n"
  "-Youtube channel: https://www.youtube.com/user/chernandezba\n"
  "-Twitter: @zesarux\n"
  "-Facebook: ZEsarUX group\n";

int first_aid_no_zrcp=0;
char *first_aid_string_zrcp="You can connect to ZEsarUX by using a telnet client using the ZEsarUX Remote Control Protocol (ZRCP). "
	"This protocol allows you to interact, debug and do a lot of internal actions to ZEsarUX. "
	"Just enable it on Settings-> Debug and use a telnet client to port 10000. "
	"Note: Windows users must use the pthreads version of ZEsarUX";

int first_aid_no_votext=0;
char *first_aid_string_votext="Do you know you can run ZEsarUX using a Text mode video driver? There are ncurses, aalib, cacalib, "
 "stdout and simpletext drivers. They are not all compiled by default, only stdout, you maybe need to compile ZEsarUX by yourself to test all of them";	

int first_aid_no_easteregg=0;
char *first_aid_string_eastereg="ZEsarUX includes three easter eggs. Can you find them? :)";


int first_aid_no_multitask=0;
char *first_aid_string_multitask="The multitask setting (enabled by default) tells ZEsarUX to continue running the emulation when you open the menu. "
	"Sometimes, if you cpu is not fast enough, the emulation can be stopped or drop FPS when you open the menu. You can disable it on Settings->GUI";

int first_aid_no_zxvision_clickout=0;
char *first_aid_string_zxvision_clickout="If you have multitask enabled, with menu open, and you click out of a window menu (inside ZEsarUX windwow), "
	"that menu window loses focus and the emulated machine gets the focus, so you can use the keyboard on the emulated machine and the menu window "
	"is still alive";


int first_aid_no_conversion=0;
char *first_aid_string_conversion="You can convert some known file formats from the File utilities menu. For example, you can "
	"convert a TAP to a TZX file";


int first_aid_no_fileextensions=0;
char *first_aid_string_fileextensions="If you save a file, for example, a snapshot, you must write the file with the desired extension, "
	"for example test.z80 or test.zsf, so ZEsarUX will know what kind of file you want to save depending on the extension you write";	

int first_aid_no_zsfextension=0;
char *first_aid_string_zsfextension="ZEsarUX uses two native snapshot file formats: .zsf and .zx.\n.zsf, which means 'ZEsarUX Snapshot File', "
	"is the preferred snapshot type, as it is supported on almost all emulated computers and can save things like: ZX-Uno memory, Divmmc status, etc.\n"
	".zx is the old snapshot native file format, which was the default format for ZEsarUX previous versions and also used in my other "
	"emulator, the ZXSpectr";	

int first_aid_no_spaceexpand=0;
char *first_aid_string_spaceexpand="Do you know you can navigate inside files, like tap, tzx, trd?\n"
	"Use the fileselector and press space over that kind of file.\n"
	"Remember to change fileselector filter to show all contents";

void menu_first_aid_init(void)
{
	total_first_aid=0;

	//Items que se disparan en ciertos eventos, con parametro si_startup=0
	menu_first_aid_add("filesel_uppercase_keys",&first_aid_no_filesel_uppercase_keys,first_aid_string_filesel_uppercase_keys,0);
	menu_first_aid_add("filesel_enter_key",&first_aid_no_filesel_enter_key,first_aid_string_filesel_enter_key,0);
	menu_first_aid_add("smartload",&first_aid_no_smartload,first_aid_string_smartload,0);
	menu_first_aid_add("initial_menu",&first_aid_no_initial_menu,first_aid_string_initial_menu,0);
	menu_first_aid_add("no_ssl_wos",&first_aid_no_ssl_wos,first_aid_string_no_ssl_wos,0);

	//Items que se disparan en startup
	menu_first_aid_add("startup_aid",&first_aid_no_startup_aid,first_aid_string_startup_aid,1);
	menu_first_aid_add("multiplattform",&first_aid_no_multiplattform,first_aid_string_multiplattform,1);
	menu_first_aid_add("accessibility",&first_aid_no_accessibility,first_aid_string_accessibility,1);
	menu_first_aid_add("documentation",&first_aid_no_documentation,first_aid_string_documentation,1);
	menu_first_aid_add("zrcp",&first_aid_no_zrcp,first_aid_string_zrcp,1);
	menu_first_aid_add("votext",&first_aid_no_votext,first_aid_string_votext,1);
	menu_first_aid_add("easteregg",&first_aid_no_easteregg,first_aid_string_eastereg,1);
	menu_first_aid_add("multitask",&first_aid_no_multitask,first_aid_string_multitask,1);
	menu_first_aid_add("zxvisionclickout",&first_aid_no_zxvision_clickout,first_aid_string_zxvision_clickout,1);
	menu_first_aid_add("conversion",&first_aid_no_conversion,first_aid_string_conversion,1);
	menu_first_aid_add("fileextensions",&first_aid_no_fileextensions,first_aid_string_fileextensions,1);
	menu_first_aid_add("zsfextension",&first_aid_no_zsfextension,first_aid_string_zsfextension,1);
	menu_first_aid_add("spaceexpand",&first_aid_no_spaceexpand,first_aid_string_spaceexpand,1);

}

//Mostrar random ayuda al iniciar. No se activa si no hay multitask
//Nota: realmente no son random, salen en orden de aparicion
void menu_first_aid_random_startup(void)
{

	//printf ("menu_first_aid_random_startup\n");
	menu_first_aid_startup=0;

	//Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
	if (save_configuration_file_on_exit.v==0) return;

	//Si desactivadas ayudas first aid
	if (menu_disable_first_aid.v) return;	

	//Si desactivado multitask
	if (!menu_multitarea) return;
	
	//si video driver no permite menu normal (no stdout ni simpletext ni null)
	if (!si_normal_menu_video_driver() ) return;

	//Lanzar la primera que este activa y sea de tipo si_startup=1
	int i;
	int encontrado=0;
	for (i=0;i<total_first_aid && !encontrado;i++) {
		int *valor_opcion;
		if (first_aid_list[i].si_startup) {
			valor_opcion=first_aid_list[i].puntero_setting;
			if ((*valor_opcion)==0) {
				string_config_key_aid_startup=first_aid_list[i].config_name;
				encontrado=1;
				menu_abierto=1;
				menu_first_aid_must_show_startup=1;
			}
		}
	}	

	debug_printf (VERBOSE_DEBUG,"Set first aid of the day to: %s",string_config_key_aid_startup);

}

//Retornar indice a opcion implicada. -1 si no
int menu_first_aid_get_setting(char *texto)
{
	//if (!strcasecmp(texto,"filesel_uppercase_keys")) first_aid_no_filesel_uppercase_keys=1;
	//buscar texto en array
	int i;
	int encontrado=-1;
	for (i=0;i<total_first_aid && encontrado==-1;i++) {
		if (!strcasecmp(texto,first_aid_list[i].config_name)) encontrado=i;
	}

	if (encontrado==-1) {
		debug_printf (VERBOSE_DEBUG,"Can not find first aid setting %s",texto);
		return -1;
	}



	//printf ("setting indice %d nombre [%s]\n",encontrado,first_aid_list[encontrado].config_name);

	//return first_aid_list[i].puntero_setting;

	return encontrado;

}

//Restaurar todos mensages first aid
void menu_first_aid_restore_all(void)
{
	int i;
	for (i=0;i<total_first_aid;i++) {
		int *opcion;
		opcion=first_aid_list[i].puntero_setting;
		*opcion=0;
        }
}

//Deshabilitar first aid de lectura de config. Si no existe, volver sin decir nada
void menu_first_aid_disable(char *texto)
{
	int indice;
	
	indice=menu_first_aid_get_setting(texto);
	if (indice<0) return;

	int *opcion;
	opcion=first_aid_list[indice].puntero_setting;

	*opcion=1; //desactivarla

}



z80_bit menu_disable_first_aid={0};



 
 //Mostrar first aid si conviene. Retorna 1 si se ha mostrado. 0 si no
int menu_first_aid_title(char *key_setting,char *title) //(enum first_aid_number_list indice)
{

	//Si no hay autoguardado de config, no mostrarlo (pues no se podria desactivar)
	if (save_configuration_file_on_exit.v==0) return 0;

	//Si desactivadas ayudas first aid
	if (menu_disable_first_aid.v) return 0;

	//Si driver no permite menu normal
	if (!si_normal_menu_video_driver()) return 0;

	int indice=menu_first_aid_get_setting(key_setting);
	if (indice<0) return 0;

	int *valor_opcion;
	char *texto_opcion;	


	valor_opcion=first_aid_list[indice].puntero_setting;
	texto_opcion=first_aid_list[indice].texto_opcion;

	//Esta desmarcada. no mostrar nada
	if (*valor_opcion) return 0;


	cls_menu_overlay();
	zxvision_menu_generic_message_setting(title,texto_opcion,"Do not show it again",valor_opcion);
		
	return 1;

}


//Mostrar first aid si conviene. Retorna 1 si se ha mostrado. 0 si no
int menu_first_aid(char *key_setting) //(enum first_aid_number_list indice)
{
	return menu_first_aid_title(key_setting,"First aid");

}


int menu_filesel_cambiar_unidad_o_volumen(void)
{

	int releer_directorio=0;

#ifdef MINGW
	//Mostrar selector de unidades
						char letra=menu_filesel_cambiar_unidad_windows();
					//printf ("letra: %d\n",letra);
					if (letra!=0) {
						char directorio[3];
						sprintf (directorio,"%c:",letra);

						//printf ("Changing to unit %s\n",directorio);

						menu_filesel_chdir(directorio);
						releer_directorio=1;
						
					}
#else

//Cambiar a ruta /media (en linux) o a /Volumes en Mac

	#if defined(__APPLE__)

//En Mac
		menu_filesel_chdir("/Volumes");
		releer_directorio=1;

	#else

//En Linux

		menu_filesel_chdir("/media");
		releer_directorio=1;	

	#endif


#endif

	return releer_directorio;
}

//Ultimas coordenadas de filesel
int last_filesel_ventana_x=0;
int last_filesel_ventana_y=0;
int last_filesel_ventana_visible_ancho=FILESEL_INICIAL_ANCHO;
int last_filesel_ventana_visible_alto=FILESEL_INICIAL_ALTO;
int filesel_primera_vez=1;

void menu_filesel_save_params_window(zxvision_window *ventana)
{
				//Guardar anteriores tamaños ventana
			/*last_filesel_ventana_x=ventana->x;
			last_filesel_ventana_y=ventana->y;

			last_filesel_ventana_visible_ancho=ventana->visible_width;
			last_filesel_ventana_visible_alto=ventana->visible_height;*/

	util_add_window_geometry("filesel",ventana->x,ventana->y,ventana->visible_width,ventana->visible_height);
}


//Retorna 1 si seleccionado archivo. Retorna 0 si sale con ESC
//Si seleccionado archivo, lo guarda en variable *archivo
//Si sale con ESC, devuelve en menu_filesel_last_directory_seen ultimo directorio
int menu_filesel(char *titulo,char *filtros[],char *archivo)
{

	//En el caso de stdout es mucho mas simple
    if (!strcmp(scr_driver_name,"stdout")) {
		printf ("%s :\n",titulo);
		scrstdout_menu_print_speech_macro(titulo);
		scanf("%s",archivo);
		return 1;
    }



	//if (filesel_primera_vez) {
		//La primera de todas metemos ventana centrada. Las siguientes, conservamos posicion
		/*if (!util_find_window_geometry("filesel",&last_filesel_ventana_x,&last_filesel_ventana_y,
			&last_filesel_ventana_visible_ancho,&last_filesel_ventana_visible_alto)) {

			last_filesel_ventana_x=FILESEL_INICIAL_X;
			last_filesel_ventana_y=FILESEL_INICIAL_Y;
			last_filesel_ventana_visible_ancho=FILESEL_INICIAL_ANCHO;
			last_filesel_ventana_visible_alto=FILESEL_INICIAL_ALTO;	
		}*/

		//filesel_primera_vez=0;
	//}

	//int primera_ventana=1;

	menu_reset_counters_tecla_repeticion();

	int tecla;


	filesel_zona_pantalla=1;

	getcwd(filesel_directorio_inicial,PATH_MAX);


    //printf ("confirm\n");

	menu_espera_no_tecla();
    	
	zxvision_window ventana_filesel;
	zxvision_window *ventana;

	//Inicialmente a NULL
	ventana=NULL;


	//guardamos filtros originales
	filesel_filtros_iniciales=filtros;



    filtros_todos_archivos[0]="";
    filtros_todos_archivos[1]=0;

	filesel_filtros=filtros;

	filesel_item *item_seleccionado;

	int aux_pgdnup;
	//menu_active_item_primera_vez=1;

	//Decir directorio activo
	menu_textspeech_say_current_directory();

	//Inicializar mouse wheel a 0, por si acaso
	mouse_wheel_vertical=mouse_wheel_horizontal=0;


//Esto lo hago para poder debugar facilmente la opcion de cambio de unidad
/*#ifdef MINGW
	int we_are_windows=1;
#else
	int we_are_windows=0;
	
#endif*/


	do {
		menu_speech_tecla_pulsada=0;
		menu_active_item_primera_vez=1;
		filesel_linea_seleccionada=0;
		filesel_archivo_seleccionado=0;
		//leer todos archivos
		int ret=menu_filesel_readdir();
		if (ret) {
			//Error leyendo directorio
   
			cls_menu_overlay();
			menu_espera_no_tecla();
			menu_filesel_chdir(filesel_directorio_inicial);
			menu_filesel_free_mem();
			zxvision_destroy_window(ventana);
			return 0;
                                		
		}


		//printf ("Total archivos en directorio: %d\n",filesel_total_items);
		//printf ("despues leer directorio\n");
		//Crear ventana. Si ya existia, borrarla
		if (ventana!=NULL) {
			//printf ("Destroy previous filesel window\n");
			cls_menu_overlay();

			//Guardar anteriores tamaños ventana
			menu_filesel_save_params_window(ventana);


			zxvision_destroy_window(ventana);
		}
		ventana=&ventana_filesel;

		int alto_total=filesel_total_items+ZXVISION_FILESEL_INITIAL_MARGIN; //Sumarle las leyendas, etc
		

		//Usar ultimas coordenadas y tamaño, sin comprobar rango de maximo ancho y alto 32x24
		//Si no hay ultimas, poner las de por defecto
		if (!util_find_window_geometry("filesel",&last_filesel_ventana_x,&last_filesel_ventana_y,&last_filesel_ventana_visible_ancho,&last_filesel_ventana_visible_alto)) {
			last_filesel_ventana_x=FILESEL_INICIAL_X;
			last_filesel_ventana_y=FILESEL_INICIAL_Y;
			last_filesel_ventana_visible_ancho=FILESEL_INICIAL_ANCHO;
			last_filesel_ventana_visible_alto=FILESEL_INICIAL_ALTO;	
		}


		//zxvision_new_window_check_range(&last_filesel_ventana_x,&last_filesel_ventana_y,&last_filesel_ventana_visible_ancho,&last_filesel_ventana_visible_alto);
		//zxvision_new_window_no_check_range(ventana,last_filesel_ventana_x,last_filesel_ventana_y,last_filesel_ventana_visible_ancho,last_filesel_ventana_visible_alto,last_filesel_ventana_visible_ancho-1,alto_total,titulo);
		zxvision_new_window_nocheck_staticsize(ventana,last_filesel_ventana_x,last_filesel_ventana_y,last_filesel_ventana_visible_ancho,last_filesel_ventana_visible_alto,last_filesel_ventana_visible_ancho-1,alto_total,titulo);

	    ventana->upper_margin=4;
	    ventana->lower_margin=4;
		ventana->visible_cursor=1;

		zxvision_draw_window(ventana);

		zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
		zxvision_menu_filesel_print_text_contents(ventana);
		zxvision_menu_filesel_print_legend(ventana);
		int releer_directorio=0;



		//El menu_print_dir aqui no hace falta porque ya entrara en el switch (filesel_zona_pantalla) inicialmente cuando filesel_zona_pantalla vale 1
		//menu_print_dir(filesel_archivo_seleccionado);

		zxvision_draw_window_contents(ventana);

		menu_refresca_pantalla();


		if (menu_filesel_posicionar_archivo.v) {
			zxvision_menu_filesel_localiza_archivo(ventana,menu_filesel_posicionar_archivo_nombre);

			menu_filesel_posicionar_archivo.v=0;
		}


		do {
			//printf ("\nReleer directorio\n");
			//printf ("cursor_line: %d filesel_linea_seleccionada: %d filesel_archivo_seleccionado %d\n",
			//	ventana->cursor_line,filesel_linea_seleccionada,filesel_archivo_seleccionado);


			//printf ("(FILESEL_ALTO-10): %d zxvision_get_filesel_alto_dir: %d\n",(FILESEL_ALTO-10),zxvision_get_filesel_alto_dir(ventana) );

			switch (filesel_zona_pantalla) {
				case 0:
				//zona superior de nombre de archivo
				ventana->visible_cursor=0;
		                zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);
				zxvision_draw_window_contents(ventana);
                		//para que haga lectura del edit box
		                menu_speech_tecla_pulsada=0;

				int ancho_mostrado=ventana->visible_width-6-2;
				if (ancho_mostrado<2) {
					//La ventana es muy pequeña como para editar
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=1;
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;					

				}

				else {


				tecla=zxvision_scanf(ventana,filesel_nombre_archivo_seleccionado,PATH_MAX,ancho_mostrado,7,1);
				//); //6 ocupa el texto "File: "

				if (tecla==15) {
					//printf ("TAB. siguiente seccion\n");
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=1;
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;
				}

				//ESC
                if (tecla==2) {
                	menu_filesel_exist_ESC();
					zxvision_destroy_window(ventana);
                    return 0;
				}

				if (tecla==13) {

					//Si es Windows y se escribe unidad: (ejemplo: "D:") hacer chdir
					int unidadwindows=0;
#ifdef MINGW
					if (filesel_nombre_archivo_seleccionado[0] &&
						filesel_nombre_archivo_seleccionado[1]==':' &&
						filesel_nombre_archivo_seleccionado[2]==0 )
						{
						debug_printf (VERBOSE_INFO,"%s is a Windows drive",filesel_nombre_archivo_seleccionado);
						unidadwindows=1;
					}
#endif


					//si es directorio, cambiamos
					struct stat buf_stat;
					int stat_valor;
					stat_valor=stat(filesel_nombre_archivo_seleccionado, &buf_stat);
					if (
						(stat_valor==0 && S_ISDIR(buf_stat.st_mode) ) ||
						(unidadwindows)
						) {
						debug_printf (VERBOSE_DEBUG,"%s Is a directory or windows drive. Change",filesel_nombre_archivo_seleccionado);
                                                menu_filesel_chdir(filesel_nombre_archivo_seleccionado);
						menu_filesel_free_mem();
                                                releer_directorio=1;
						filesel_zona_pantalla=1;

					        //Decir directorio activo
						//Esperar a liberar tecla si no la tecla invalida el speech
						menu_espera_no_tecla();
					        menu_textspeech_say_current_directory();


					}


					//sino, devolvemos nombre con path, siempre que extension sea conocida
					else {
                    	cls_menu_overlay();
                        menu_espera_no_tecla();

						if (menu_avisa_si_extension_no_habitual(filtros,filesel_nombre_archivo_seleccionado)) {

                        //unimos directorio y nombre archivo. siempre que inicio != '/'
						if (filesel_nombre_archivo_seleccionado[0]!='/') {
                        	getcwd(archivo,PATH_MAX);
                            sprintf(&archivo[strlen(archivo)],"/%s",filesel_nombre_archivo_seleccionado);
						}

						else sprintf(archivo,"%s",filesel_nombre_archivo_seleccionado);


                        menu_filesel_chdir(filesel_directorio_inicial);
						menu_filesel_free_mem();

						//return menu_avisa_si_extension_no_habitual(filtros,archivo);
						cls_menu_overlay();
						zxvision_destroy_window(ventana);
						last_filesused_insert(archivo);
						return 1;

						}

						else {
							//Extension no conocida. No modificar variable archivo
							//printf ("Unknown extension. Do not modify archivo. Contents: %s\n",archivo);
							cls_menu_overlay();
							zxvision_destroy_window(ventana);
							return 0;
						}
						



						//Volver con OK
                        //return 1;

					}
				}

				}

				break;
			
			case 1:
				//zona selector de archivos

				debug_printf (VERBOSE_DEBUG,"Read directory. menu_speech_tecla_pulsada=%d",menu_speech_tecla_pulsada);
				ventana->visible_cursor=1;
				zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);
				zxvision_draw_window_contents(ventana);
				//Para no releer todas las entradas
				menu_speech_tecla_pulsada=1;


				tecla=zxvision_common_getkey_refresh();
				//printf ("Despues lee tecla\n");


				//Si se ha pulsado boton de raton
                                if (mouse_left) {
					//printf ("Pulsado boton raton izquierdo\n");

					 //Si en linea de "File"
					menu_filesel_change_zone_if_clicked(ventana,&filesel_zona_pantalla,&tecla);
                                        /*if (menu_mouse_y==2 && menu_mouse_x<ventana->visible_width-1) {
						printf ("Pulsado zona File\n");
                                                                menu_reset_counters_tecla_repeticion();
                                                                filesel_zona_pantalla=0;
                                                                tecla=0;
                                        }*/

					if (si_menu_mouse_en_ventana() && zxvision_si_mouse_zona_archivos(ventana) ) {
						//Ubicamos cursor donde indica raton
						if (menu_filesel_set_cursor_at_mouse(ventana)) {
							//Como pulsar enter
							tecla=13;
						}
					}
				}


				//Si se ha movido raton. Asumimos que ha vuelto de leer tecla, tecla=0 y no se ha pulsado mouse
				if (!tecla && !mouse_left) {
				 //if (mouse_movido) {
                    //printf ("mouse x: %d y: %d menu mouse x: %d y: %d\n",mouse_x,mouse_y,menu_mouse_x,menu_mouse_y);
                    //printf ("ventana x %d y %d ancho %d alto %d\n",ventana_x,ventana_y,ventana_ancho,ventana_alto);
                    if (si_menu_mouse_en_ventana() ) {
                        //printf ("dentro ventana\n");
                        //Ver en que zona esta
                        
                        if (zxvision_si_mouse_zona_archivos(ventana)) {
							menu_filesel_set_cursor_at_mouse(ventana);						

                        }

                        else if (menu_mouse_y==(ventana->visible_height-4)+1) {
                            //En la linea de filtros
                            //nada en especial
                            //printf ("En linea de filtros\n");
                        }
                    }
                else {
                    //printf ("fuera ventana\n");
                }

        }



				switch (tecla) {
					//abajo
					case 10:
						zxvision_menu_filesel_cursor_abajo(ventana);
						//Para no releer todas las entradas
						menu_speech_tecla_pulsada=1;
					break;

					//arriba
					case 11:
						zxvision_menu_filesel_cursor_arriba(ventana);
						//Para no releer todas las entradas
						menu_speech_tecla_pulsada=1;
					break;

					//PgDn
					case 25:
						for (aux_pgdnup=0;aux_pgdnup<zxvision_get_filesel_alto_dir(ventana);aux_pgdnup++)
							zxvision_menu_filesel_cursor_abajo(ventana);
						//releer todas entradas
						menu_speech_tecla_pulsada=0;
						//y decir selected item
						menu_active_item_primera_vez=1;
                    break;

					//PgUp
					case 24:
						for (aux_pgdnup=0;aux_pgdnup<zxvision_get_filesel_alto_dir(ventana);aux_pgdnup++)
							zxvision_menu_filesel_cursor_arriba(ventana);
						//releer todas entradas
						menu_speech_tecla_pulsada=0;
						//y decir selected item
						menu_active_item_primera_vez=1;
                    break;


					case 15:
					//tabulador
						menu_reset_counters_tecla_repeticion();
						if (menu_filesel_show_utils.v==0) filesel_zona_pantalla=2;
						else filesel_zona_pantalla=0; //Si hay utils, cursor se va arriba
					break;

					//ESC
					case 2:
						//meter en menu_filesel_last_directory_seen nombre directorio
						//getcwd(archivo,PATH_MAX);
						getcwd(menu_filesel_last_directory_seen,PATH_MAX);
						//printf ("salimos con ESC. nombre directorio: %s\n",archivo);
                        menu_filesel_exist_ESC();

						zxvision_destroy_window(ventana);
                        return 0;

					break;

					//Expandir archivos
					case 32:
						menu_first_aid("filesel_enter_key");

                                                item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);
                                                menu_reset_counters_tecla_repeticion();

                                                //printf ("despues de get filesel item. item_seleccionado=%p\n",item_seleccionado);

                                                if (item_seleccionado==NULL) {
                                                        //Esto pasa en las carpetas vacias, como /home en Mac OS
                                                                        menu_filesel_exist_ESC();
																		zxvision_destroy_window(ventana);
                                                                        return 0;


                                                }

						if (get_file_type(item_seleccionado->d_type,item_seleccionado->d_name)==2) {
							debug_printf(VERBOSE_INFO,"Can't expand directories");
						}

						else {
								debug_printf(VERBOSE_DEBUG,"Expanding file %s",item_seleccionado->d_name);
                                                                char tmpdir[PATH_MAX];

                                                                if (menu_filesel_expand(item_seleccionado->d_name,tmpdir) ) {
									//TODO: Si lanzo este warning se descuadra el dibujado de ventana
									//menu_warn_message("Don't know how to expand that file");
									debug_printf(VERBOSE_INFO,"Don't know how to expand that file");
                                                                }

                                                                else {
                                                                        menu_filesel_change_to_tmp(tmpdir);
																		releer_directorio=1;
                                                                }
						}


					break;

					case 13:

						menu_first_aid("filesel_enter_key");

						//si seleccion es directorio
						item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);
						menu_reset_counters_tecla_repeticion();

						//printf ("despues de get filesel item. item_seleccionado=%p\n",item_seleccionado);

						if (item_seleccionado==NULL) {
							//Esto pasa en las carpetas vacias, como /home en Mac OS
                                                                        menu_filesel_exist_ESC();
																		zxvision_destroy_window(ventana);
                                                                        return 0;


						}

						if (get_file_type(item_seleccionado->d_type,item_seleccionado->d_name)==2) {
							debug_printf (VERBOSE_DEBUG,"Is a directory. Change");
							char *directorio_a_cambiar;

							//suponemos esto:
							directorio_a_cambiar=item_seleccionado->d_name;
							char last_directory[PATH_MAX];

							//si es "..", ver si directorio actual contiene archivo que indica ultimo directorio
							//en caso de descompresiones
							if (!strcmp(item_seleccionado->d_name,"..")) {
								debug_printf (VERBOSE_DEBUG,"Is directory ..");
								if (si_existe_archivo(MENU_LAST_DIR_FILE_NAME)) {
									debug_printf (VERBOSE_DEBUG,"Directory has file " MENU_LAST_DIR_FILE_NAME " Changing "
											"to previous directory");

									if (menu_filesel_read_file_last_dir(last_directory)==0) {
										debug_printf (VERBOSE_DEBUG,"Previous directory was: %s",last_directory);

										directorio_a_cambiar=last_directory;
									}

								}
							}

							debug_printf (VERBOSE_DEBUG,"Changing to directory %s",directorio_a_cambiar);

							menu_filesel_chdir(directorio_a_cambiar);


							menu_filesel_free_mem();
							releer_directorio=1;

						        //Decir directorio activo
							//Esperar a liberar tecla si no la tecla invalida el speech
							menu_espera_no_tecla();
						        menu_textspeech_say_current_directory();

						}

						else {

							//Si seleccion es archivo comprimido
							if (menu_util_file_is_compressed(item_seleccionado->d_name) ) {
								debug_printf (VERBOSE_DEBUG,"Is a compressed file");

								char tmpdir[PATH_MAX];

								if (menu_filesel_uncompress(item_seleccionado->d_name,tmpdir) ) {
									menu_filesel_exist_ESC();
									zxvision_destroy_window(ventana);
									return 0;
								}

								else {
									menu_filesel_change_to_tmp(tmpdir);
                        	                                        releer_directorio=1;
								}

							}

							else {
								//Enter. No es directorio ni archivo comprimido
								//Si estan las file utils, enter no hace nada
								if (menu_filesel_show_utils.v==0) { 

					                cls_menu_overlay();
        	                        menu_espera_no_tecla();

									if (menu_avisa_si_extension_no_habitual(filtros,filesel_nombre_archivo_seleccionado)) {

									//unimos directorio y nombre archivo
									getcwd(archivo,PATH_MAX);
									sprintf(&archivo[strlen(archivo)],"/%s",item_seleccionado->d_name);

									menu_filesel_chdir(filesel_directorio_inicial);
									menu_filesel_free_mem();

									//return menu_avisa_si_extension_no_habitual(filtros,archivo);
									//Guardar anteriores tamaños ventana
									menu_filesel_save_params_window(ventana);

									cls_menu_overlay();
									zxvision_destroy_window(ventana);
									last_filesused_insert(archivo);
									return 1;

									}

                                    else {
                                                        //Extension no conocida. No modificar variable archivo
                                                        //printf ("Unknown extension. Do not modify archivo. Contents: %s\n",archivo);
														cls_menu_overlay();
														zxvision_destroy_window(ventana);
                                                        return 0;
                                    }


									//Volver con OK
									//return 1;
								}
							}

						}
					break;
				}

				//entre a y z y numeros
				if ( (tecla>='a' && tecla<='z') || (tecla>='0' && tecla<='9') ) {
					menu_first_aid("filesel_uppercase_keys");
					zxvision_menu_filesel_localiza_letra(ventana,tecla);
				}


				if (tecla=='D') {
					releer_directorio=menu_filesel_cambiar_unidad_o_volumen();
					
				}

				if (tecla=='R') {	

					//Archivos recientes
					char *archivo_reciente=menu_filesel_recent_files();
					if (archivo_reciente!=NULL) {
						//printf ("Loading file %s\n",archivo_reciente);
						strcpy(archivo,archivo_reciente);

                                                                      menu_filesel_chdir(filesel_directorio_inicial);
                                                                        menu_filesel_free_mem();

                                                                        //return menu_avisa_si_extension_no_habitual(filtros,archivo);
                                                                        cls_menu_overlay();
                                                                        zxvision_destroy_window(ventana);
                                                                        return 1;

					}
				}

				//Si esta filesel, opciones en mayusculas
				if (menu_filesel_show_utils.v) {
					
					if ( (tecla>='A' && tecla<='Z') ) {
						menu_espera_no_tecla();
						//TODO: Si no se pone espera_no_tecla,
						//al aparecer menu de, por ejemplo truncate, el texto se fusiona con el fondo de manera casi transparente,
						//como si no borrase el putpixel cache
						//esto también sucede en otras partes del código del menú pero no se por que es

						menu_reset_counters_tecla_repeticion();
						
						//Comun para acciones que usan archivo seleccionado
						if (tecla=='V' || tecla=='T' || tecla=='E' || tecla=='M' || tecla=='N' || tecla=='C' || tecla=='P' || tecla=='F' || tecla=='O' || tecla=='I') {
							
							//Obtener nombre del archivo al que se apunta
							char file_utils_file_selected[PATH_MAX]="";
							item_seleccionado=menu_get_filesel_item(filesel_archivo_seleccionado+filesel_linea_seleccionada);
							if (item_seleccionado!=NULL) {
								//Esto pasa en las carpetas vacias, como /home en Mac OS
									//unimos directorio y nombre archivo
									getcwd(file_utils_file_selected,PATH_MAX);
									sprintf(&file_utils_file_selected[strlen(file_utils_file_selected)],"/%s",item_seleccionado->d_name);								
								//Info para cualquier tipo de archivo
								if (tecla=='I') file_utils_info_file(file_utils_file_selected);

								//Si no es directorio
								if (get_file_type(item_seleccionado->d_type,item_seleccionado->d_name)!=2) {
									//unimos directorio y nombre archivo
									//getcwd(file_utils_file_selected,PATH_MAX);
									//sprintf(&file_utils_file_selected[strlen(file_utils_file_selected)],"/%s",item_seleccionado->d_name);
									
									//Visor de archivos
									if (tecla=='V') menu_file_viewer_read_file("Text file view",file_utils_file_selected);

									//Truncate
									if (tecla=='T') {
										if (menu_confirm_yesno_texto("Truncate","Sure?")) util_truncate_file(file_utils_file_selected);
									}

									//Delete
									if (tecla=='E') {
										if (menu_confirm_yesno_texto("Delete","Sure?")) {
											util_delete(file_utils_file_selected);
											//unlink(file_utils_file_selected);
											releer_directorio=1;
										}

									}
									//Move
									if (tecla=='M') {
										file_utils_move_rename_copy_file(file_utils_file_selected,0);
										//Restaurar variables globales que se alteran al llamar al otro filesel
										//TODO: hacer que estas variables no sean globales sino locales de esta funcion menu_filesel
										filesel_filtros_iniciales=filtros;
										filesel_filtros=filtros;
		
										releer_directorio=1;

									}

									//Rename
									if (tecla=='N') {
										file_utils_move_rename_copy_file(file_utils_file_selected,1);
										releer_directorio=1;
									}

									//Filemem
									if (tecla=='F') {
										file_utils_file_mem_load(file_utils_file_selected);
									}

									//Convert
									if (tecla=='O') {
										file_utils_file_convert(file_utils_file_selected);
										releer_directorio=1;
									}

						

									//Copy
									if (tecla=='C') {
										file_utils_move_rename_copy_file(file_utils_file_selected,2);
										//Restaurar variables globales que se alteran al llamar al otro filesel
										//TODO: hacer que estas variables no sean globales sino locales de esta funcion menu_filesel
										filesel_filtros_iniciales=filtros;
										filesel_filtros=filtros;
										
										releer_directorio=1;
									}

									

								}
							}

							
						}

						//Mkdir
						if (tecla=='K') {
							char string_carpeta[200];
							string_carpeta[0]=0;
							menu_ventana_scanf("Folder name",string_carpeta,200);
							if (string_carpeta[0]) {
								menu_filesel_mkdir(string_carpeta);
								releer_directorio=1;
							}
						}


						//Paste text
						if (tecla=='P') {
							file_utils_paste_clipboard();
										
										
							releer_directorio=1;
						}
			
						

						//Redibujar ventana
						//releer_directorio=1;
						
						zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
						zxvision_menu_filesel_print_legend(ventana);

						zxvision_menu_filesel_print_text_contents(ventana);
					}
					
				}

				//menu_espera_no_tecla();
				menu_espera_no_tecla_con_repeticion();




			break;

			case 2:
				//zona filtros
				ventana->visible_cursor=0;
                                zxvision_menu_print_dir(filesel_archivo_seleccionado,ventana);

                                //para que haga lectura de los filtros
                                menu_speech_tecla_pulsada=0;

				zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
				zxvision_draw_window_contents(ventana);
	

				tecla=zxvision_common_getkey_refresh();


				if (menu_filesel_change_zone_if_clicked(ventana,&filesel_zona_pantalla,&tecla)) {
					zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
                                         releer_directorio=1;

                                                menu_espera_no_tecla();
				}

		                //ESC
                                else if (tecla==2) {
                                                cls_menu_overlay();
                                                menu_espera_no_tecla();
                                                menu_filesel_chdir(filesel_directorio_inicial);
						menu_filesel_free_mem();
						zxvision_destroy_window(ventana);
                                                return 0;
                                }

				//cambiar de zona con tab
				else if (tecla==15) {
					menu_reset_counters_tecla_repeticion();
					filesel_zona_pantalla=0;
					zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
					//no releer todos archivos
					menu_speech_tecla_pulsada=1;
				}


				else {

					//printf ("conmutar filtros\n");
					if (tecla || (tecla==0 && mouse_left)) { 

						//conmutar filtros
						menu_filesel_switch_filters();

					        zxvision_menu_filesel_print_filters(ventana,filesel_filtros);
						releer_directorio=1;

						menu_espera_no_tecla();
					}
				}

			break;
			}

		} while (releer_directorio==0);
	} while (1);


	//Aqui no se va a llegar nunca


}



//Inicializar vacio
void last_filesused_clear(void)
{

	int i;
	for (i=0;i<MAX_LAST_FILESUSED;i++) {
		last_files_used_array[i][0]=0;
	}
}

//Desplazar hacia abajo desde posicion superior indicada. La posicion indicada sera un duplicado de la siguiente posicion por tanto
void lastfilesuser_scrolldown(int posicion_up,int posicion_down)
{
	int i;
	for (i=posicion_down;i>=posicion_up+1;i--) {
		strcpy(last_files_used_array[i],last_files_used_array[i-1]);
	}	
}

//Insertar entrada en last smartload
void last_filesused_insert(char *s)
{
	//Desplazar todos hacia abajo e insertar en posicion 0
	//Desde abajo a arriba

	//int i;
	/*for (i=MAX_LAST_FILESUSED-1;i>=1;i--) {
		strcpy(last_files_used_array[i],last_files_used_array[i-1]);
	}*/

	lastfilesuser_scrolldown(0,MAX_LAST_FILESUSED-1);


	//Meter en posicion 0
	strcpy(last_files_used_array[0],s);

	debug_printf (VERBOSE_INFO,"Inserting recent file %s at position 0",s);

	//printf ("Dump smartload:\n");

	//for (i=0;i<MAX_LAST_FILESUSED;i++) {
	//	printf ("Entry %d: [%s]\n",i,last_files_used_array[i]);
	//}
}


void menu_network_error(int error)
{
	menu_error_message(z_sock_get_error(error));
}