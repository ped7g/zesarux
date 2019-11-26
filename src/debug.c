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
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>


#if defined(linux) || defined(__APPLE__)
#include <execinfo.h>
#endif


#include "cpu.h"
#include "debug.h"
#include "mem128.h"
#include "screen.h"
#include "menu.h"
#include "zx8081.h"
#include "operaciones.h"
#include "core_spectrum.h"
#include "core_zx8081.h"
#include "core_z88.h"
#include "core_ace.h"
#include "core_cpc.h"
#include "core_sam.h"
#include "core_ql.h"
#include "disassemble.h"
#include "utils.h"
#include "prism.h"

#include "spectra.h"
#include "tbblue.h"
#include "zxuno.h"
#include "ulaplus.h"
#include "timex.h"
#include "ay38912.h"
#include "ula.h"
#include "ql.h"
#include "m68k.h"
#include "superupgrade.h"
#include "core_mk14.h"
#include "scmp.h"

#include "dandanator.h"
#include "multiface.h"
#include "chloe.h"
#include "cpc.h"
#include "sam.h"

#include "snap.h"
#include "kartusho.h"
#include "ifrom.h"
#include "diviface.h"
#include "betadisk.h"

#include "tsconf.h"

#include "core_reduced_spectrum.h"

#include "remote.h"
#include "charset.h"
#include "settings.h"
#include "expression_parser.h"
#include "atomic.h"


struct timeval debug_timer_antes, debug_timer_ahora;




z80_bit menu_breakpoint_exception={0};

z80_bit debug_breakpoints_enabled={0};

//breakpoints de condiciones. nuevo formato para nuevo parser de tokens
token_parser debug_breakpoints_conditions_array_tokens[MAX_BREAKPOINTS_CONDITIONS][MAX_PARSER_TOKENS_NUM];

//watches. nuevo formato con parser de tokens
token_parser debug_watches_array[DEBUG_MAX_WATCHES][MAX_PARSER_TOKENS_NUM];


//Ultimo breakpoint activo+1 (o sea, despues del ultimo activo) para optimizar la comprobacion de breakpoints,
//asi solo se comprueba hasta este en vez de comprobarlos todos
int last_active_breakpoint=0;

//acciones a ejecutar cuando salta un breakpoint
char debug_breakpoints_actions_array[MAX_BREAKPOINTS_CONDITIONS][MAX_BREAKPOINT_CONDITION_LENGTH];

//A 0 si ese breakpoint no ha saltado. A 1 si ya ha saltado
int debug_breakpoints_conditions_saltado[MAX_BREAKPOINTS_CONDITIONS];

//A 1 si ese breakpoint esta activado. A 0 si no
int debug_breakpoints_conditions_enabled[MAX_BREAKPOINTS_CONDITIONS];



optimized_breakpoint optimized_breakpoint_array[MAX_BREAKPOINTS_CONDITIONS];



//Para hacer breakpoints de lectura de direcciones (cuando se hace peek de alguna direccion). Si vale -1, no hay breakpoint
//int debug_breakpoints_peek_array[MAX_BREAKPOINTS_PEEK];

//Punteros a las funciones originales
//z80_byte (*peek_byte_no_time_no_debug)(z80_int dir);
//z80_byte (*peek_byte_no_debug)(z80_int dir);
//void (*poke_byte_no_time_no_debug)(z80_int dir,z80_byte valor);
//void (*poke_byte_no_debug)(z80_int dir,z80_byte valor);

z80_byte (*lee_puerto_no_debug)(z80_byte puerto_h,z80_byte puerto_l);
void (*out_port_no_debug)(z80_int puerto,z80_byte value);


/*Variables de lectura/escritura en direcciones/puertos
Memory/port
Read/write
Address/value
ejemplo : MRA: memory read address
*/

//Todos estos valores podrian ser z80_byte o z80_int, pero dado que al arrancar el ordenador
//estarian a 0, una condicion tipo MRA=0 o MWV=0 etc haria saltar esos breakpoints
//por tanto los inicializo a un valor fuera de rango de z80_int incluso

unsigned int debug_mmu_mrv=65536; //Memory Read Value (valor leido en peek)
unsigned int debug_mmu_mwv=65536; //Memory Write Value (valor escrito en poke)
unsigned int debug_mmu_prv=65536; //Port Read Value (valor leido en lee_puerto)
unsigned int debug_mmu_pwv=65536; //Port Write Value (valor escrito en out_port)

unsigned int debug_mmu_pra=65536; //Port Read Address (direccion usada en lee_puerto)
unsigned int debug_mmu_pwa=65536; //Port Write Address (direccion usada en out_port)

//Inicializar a algo invalido porque si no podria saltar el primer MRA=0 al leer de la rom,
//por tanto a -1 (y si no fuera por ese -1, podria ser un tipo z80_int en vez de int)
unsigned int debug_mmu_mra=65536; //Memory Read Addres (direccion usada peek)
unsigned int debug_mmu_mwa=65536; //Memory Write Address (direccion usada en poke)

//Anteriores valores para mra y mwa. Solo usado en los nuevos memory-breakpoints
//Si es -1, no hay valor anterior
unsigned int anterior_debug_mmu_mra=65536;
unsigned int anterior_debug_mmu_mwa=65536;

//Array usado en memory-breakpoints
/*
Es equivalente a MRA o MWA pero mucho mas rapido
Valores:
0: no hay mem breakpoint para esa direccion
1: hay mem breakpoint de lectura para esa direccion
2: hay mem breakpoint de escritura para esa direccion
3: hay mem breakpoint de lectura o escritura para esa direccion
*/
z80_byte mem_breakpoint_array[65536];

char *mem_breakpoint_types_strings[]={
	"Disabled",
	"Read",
	"Write",
	"Read & Write"
};

/*
Pruebas de uso de cpu entre el parser clasico (versiones anteriores a la 7.1), el parser optimizado para PC=, MRA=, MWA, y
los mem_breakpoints:

mra=32768

Con zesarux de siempre: 66% de cpu
Con MRA optimizado: 44% de cpu
Con memory breakpoint: 44% de cpu
Mismo uso de cpu en los dos casos anteriores sin breakpoints, solo habilitando breakpoints: 44% cpu


Con 10 mra en optimizado: 48% 
Con 10 memory breakpoints: 44%

*/

//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;


//Avisa que el ultimo opcode ha sido un out a puerto, para poder hacer breakpoints con esto
int debug_fired_out=0;
//Avisa que el ultimo opcode ha sido un in a puerto, para poder hacer breakpoints con esto
int debug_fired_in=0;
//Avisa que se ha generado interrupcion, para poder hacer breakpoints con esto
int debug_fired_interrupt=0;

//Mensaje que ha hecho saltar el breakpoint
char catch_breakpoint_message[MAX_MESSAGE_CATCH_BREAKPOINT];

//Id indice breakpoint que ha saltado
//Si -1, no ha saltado por indice, quiza por un membreakpoint
int catch_breakpoint_index=0;


//Core loop actual
int cpu_core_loop_active;

//puntero a cpu core actual
void (*cpu_core_loop) (void);

//nombre identificativo del core. De momento solo usado para mostrar nombre en comando de remote command
char *cpu_core_loop_name=NULL;

//puntero a cpu core actual, sin degug, usado en la funcion de debug
//void (*cpu_core_loop_no_debug) (void);

//si se hace debug en el core (para breakpoints, y otras condiciones)
//z80_bit debug_cpu_core_loop={0};

void cpu_core_loop_debug_check_breakpoints(void);

void debug_dump_nested_print(char *string_inicial, char *string_to_print);


//mostrar ademas mensajes de debug en consola con printf, adicionalmente de donde lo muestre ya (en curses, aa, caca salen en dentro ventana)
z80_bit debug_always_show_messages_in_console={0};

//Si volcar snapshot zsf cuando hay cpu_panic
z80_bit debug_dump_zsf_on_cpu_panic={0};

//Si ya se ha volcado snapshot zsf cuando hay cpu_panic, para evitar un segundo volcado (y siguientes) si se genera otro panic al hacer el snapshot
z80_bit dumped_debug_dump_zsf_on_cpu_panic={0};

//nombre del archivo volcado
char dump_snapshot_panic_name[PATH_MAX]="";


//empieza en el 163
char *spectrum_rom_tokens[]={
"SPECTRUM","PLAY",
"RND","INKEY$","PI","FN","POINT","SCREEN$","ATTR","AT","TAB",
"VAL$","CODE","VAL","LEN","SIN","COS","TAN","ASN","ACS",
"ATN","LN","EXP","INT","SQR","SGN","ABS","PEEK","IN",
"USR","STR$","CHR$","NOT","BIN","OR","AND","<=",">=",
"<>","LINE","THEN","TO","STEP","DEF FN","CAT","FORMAT","MOVE",
"ERASE","OPEN #","CLOSE #","MERGE","VERIFY","BEEP","CIRCLE","INK","PAPER",
"FLASH","BRIGHT","INVERSE","OVER","OUT","LPRINT","LLIST","STOP","READ",
"DATA","RESTORE","NEW","BORDER","CONTINUE","DIM","REM","FOR","GO TO",
"GO SUB","INPUT","LOAD","LIST","LET","PAUSE","NEXT","POKE","PRINT",
"PLOT","RUN","SAVE","RANDOMIZE","IF","CLS","DRAW","CLEAR","RETURN","COPY"
};



char *zx81_rom_tokens[]={
//estos a partir de 192
"\"","AT","TAB","?","CODE","VAL","LEN","SIN","COS","TAN","ASN","ACS",
"ATN","LN","EXP","INT","SQR","SGN","ABS","PEEK","USR",
"STR$","CHR$","NOT","**"," OR"," AND","<=",">=","<>",
" THEN"," TO"," STEP"," LPRINT"," LLIST"," STOP",
" SLOW"," FAST"," NEW"," SCROLL"," CONT"," DIM",
" REM"," FOR"," GOTO"," GOSUB"," INPUT"," LOAD",
" LIST"," LET"," PAUSE"," NEXT"," POKE"," PRINT",
" PLOT"," RUN"," SAVE"," RAND"," IF"," CLS"," UNPLOT"," CLEAR"," RETURN"," COPY",

//estos en el 64,65,66
"RND","INKEY$","PI"

};

char *zx80_rom_tokens[]={
"THEN","TO",";", "," ,")" ,"(","NOT","-","+",
"*","/","AND","OR","**","=","<",">","LIST",
"RETURN","CLS","DIM","SAVE","FOR","GO TO",
"POKE","INPUT","RANDOMISE","LET","'?'",
"'?'","NEXT","PRINT","'?'","NEW",
"RUN","STOP","CONTINUE","IF","GO SUB","LOAD",
"CLEAR","REM","?"
};
 

struct s_z88_basic_rom_tokens {
	z80_byte index;
	char token[20];
};


struct s_z88_basic_rom_tokens z88_basic_rom_tokens[]={
{0x80,"AND"},
{0x94,"ABS"},
{0x95,"ACS"},
{0x96,"ADVAL"},
{0x97,"ASC"},
{0x98,"ASN"},
{0x99,"ATN"},
{0xC6,"AUTO"},
{0x9A,"BGET"},
{0xD5,"BPUT"},
{0xFB,"COLOUR"},
{0xFB,"COLOR"},
{0xD6,"CALL"},
{0xD7,"CHAIN"},
{0xBD,"CHR$"},
{0xD8,"CLEAR"},
{0xD9,"CLOSE"},
{0xDA,"CLG"},
{0xDB,"CLS"},
{0x9B,"COS"},
{0x9C,"COUNT"},
{0xDC,"DATA"},
{0x9D,"DEG"},
{0xDD,"DEF"},
{0xC7,"DELETE"},
{0x81,"DIV"},
{0xDE,"DIM"},
{0xDF,"DRAW"},
{0xE1,"ENDPROC"},
{0xE0,"END"},
{0xE2,"ENVELOPE"},
{0x8B,"ELSE"},
{0xA0,"EVAL"},
{0x9E,"ERL"},
{0x85,"ERROR"},
{0xC5,"EOF"},
{0x82,"EOR"},
{0x9F,"ERR"},
{0xA1,"EXP"},
{0xA2,"EXT"},
{0xE3,"FOR"},
{0xA3,"FALSE"},
{0xA4,"FN"},
{0xE5,"GOTO"},
{0xBE,"GET$"},
{0xA5,"GET"},
{0xE4,"GOSUB"},
{0xE6,"GCOL"},
{0x93,"HIMEM"},
{0xE8,"INPUT"},
{0xE7,"IF"},
{0xBF,"INKEY$"},
{0xA6,"INKEY"},
{0xA8,"INT"},
{0xA7,"INSTR("},
{0xC9,"LIST"},
{0x86,"LINE"},
{0xC8,"LOAD"},
{0x92,"LOMEM"},
{0xEA,"LOCAL"},
{0xC0,"LEFT$("},
{0xA9,"LEN"},
{0xE9,"LET"},
{0xAB,"LOG"},
{0xAA,"LN"},
{0xC1,"MID$("},
{0xEB,"MODE"},
{0x83,"MOD"},
{0xEC,"MOVE"},
{0xED,"NEXT"},
{0xCA,"NEW"},
{0xAC,"NOT"},
{0xCB,"OLD"},
{0xEE,"ON"},
{0x87,"OFF"},
{0x84,"OR"},
{0x8E,"OPENIN"},
{0xAE,"OPENOUT"},
{0xAD,"OPENUP"},
{0xFF,"OSCLI"},
{0xF1,"PRINT"},
{0x90,"PAGE"},
{0x8F,"PTR"},
{0xAF,"PI"},
{0xF0,"PLOT"},
{0xB0,"POINT("},
{0xF2,"PROC"},
{0xB1,"POS"},
{0xCE,"PUT"},
{0xF8,"RETURN"},
{0xF5,"REPEAT"},
{0xF6,"REPORT"},
{0xF3,"READ"},
{0xF4,"REM"},
{0xF9,"RUN"},
{0xB2,"RAD"},
{0xF7,"RESTORE"},
{0xC2,"RIGHT$("},
{0xB3,"RND"},
{0xCC,"RENUMBER"},
{0x88,"STEP"},
{0xCD,"SAVE"},
{0xB4,"SGN"},
{0xB5,"SIN"},
{0xB6,"SQR"},
{0x89,"SPC"},
{0xC3,"STR$"},
{0xC4,"STRING$("},
{0xD4,"SOUND"},
{0xFA,"STOP"},
{0xB7,"TAN"},
{0x8C,"THEN"},
{0xB8,"TO"},
{0x8A,"TAB("},
{0xFC,"TRACE"},
{0x91,"TIME"},
{0xB9,"TRUE"},
{0xFD,"UNTIL"},
{0xBA,"USR"},
{0xEF,"VDU"},
{0xBB,"VAL"},
{0xBC,"VPOS"},
{0xFE,"WIDTH"},
{0xD3,"HIMEM"},
{0xD2,"LOMEM"},
{0xD0,"PAGE"},
{0xCF,"PTR"},
{0xD1,"TIME"},
{0x01,""}  //Importante este 01 final
};

//Rutina auxiliar que pueden usar los drivers de video para mostrar los registros. Mete en una string los registros
void print_registers(char *buffer)
{

  if (CPU_IS_SCMP) {
    char buffer_flags[9];
    scmp_get_flags_letters(scmp_m_SR,buffer_flags);
    sprintf (buffer,"PC=%04x P1=%04x P2=%04x P3=%04x AC=%02x ER=%02x SR=%s",

      get_pc_register(),scmp_m_P1.w.l,scmp_m_P2.w.l,scmp_m_P3.w.l,
      scmp_m_AC, scmp_m_ER,buffer_flags);

  }

  else if (CPU_IS_MOTOROLA) {

unsigned int registro_sr=m68k_get_reg(NULL, M68K_REG_SR);

      sprintf (buffer,"PC: %05X SP: %05X USP: %05X SR: %04X : %c%c%c%c%c%c%c%c%c%c "
            "A0: %08X A1: %08X A2: %08X A3: %08X A4: %08X A5: %08X A6: %08X A7: %08X "
            "D0: %08X D1: %08X D2: %08X D3: %08X D4: %08X D5: %08X D6: %08X D7: %08X "



      ,get_pc_register(),m68k_get_reg(NULL, M68K_REG_SP),m68k_get_reg(NULL, M68K_REG_USP),registro_sr,
      (registro_sr&32768 ? 'T' : ' '),
      (registro_sr&8192  ? 'S' : ' '),
      (registro_sr&1024  ? '2' : ' '),
      (registro_sr&512   ? '1' : ' '),
      (registro_sr&256   ? '0' : ' '),
          (registro_sr&16 ? 'X' : ' '),
          (registro_sr&8  ? 'N' : ' '),
          (registro_sr&4  ? 'Z' : ' '),
          (registro_sr&2  ? 'V' : ' '),
          (registro_sr&1  ? 'C' : ' '),
          m68k_get_reg(NULL, M68K_REG_A0),m68k_get_reg(NULL, M68K_REG_A1),m68k_get_reg(NULL, M68K_REG_A2),m68k_get_reg(NULL, M68K_REG_A3),
          m68k_get_reg(NULL, M68K_REG_A4),m68k_get_reg(NULL, M68K_REG_A5),m68k_get_reg(NULL, M68K_REG_A6),m68k_get_reg(NULL, M68K_REG_A7),
          m68k_get_reg(NULL, M68K_REG_D0),m68k_get_reg(NULL, M68K_REG_D1),m68k_get_reg(NULL, M68K_REG_D2),m68k_get_reg(NULL, M68K_REG_D3),
          m68k_get_reg(NULL, M68K_REG_D4),m68k_get_reg(NULL, M68K_REG_D5),m68k_get_reg(NULL, M68K_REG_D6),m68k_get_reg(NULL, M68K_REG_D7)

    );
  }

  else {
  sprintf (buffer,"PC=%04x SP=%04x AF=%04x BC=%04x HL=%04x DE=%04x IX=%04x IY=%04x AF'=%04x BC'=%04x HL'=%04x DE'=%04x I=%02x R=%02x  "
                  "F=%c%c%c%c%c%c%c%c F'=%c%c%c%c%c%c%c%c MEMPTR=%04x IM%d IFF%c%c VPS: %d ",
  reg_pc,reg_sp,(reg_a<<8)|Z80_FLAGS,(reg_b<<8)|reg_c,(reg_h<<8)|reg_l,(reg_d<<8)|reg_e,reg_ix,reg_iy,(reg_a_shadow<<8)|Z80_FLAGS_SHADOW,(reg_b_shadow<<8)|reg_c_shadow,
  (reg_h_shadow<<8)|reg_l_shadow,(reg_d_shadow<<8)|reg_e_shadow,reg_i,(reg_r&127)|(reg_r_bit7&128),DEBUG_STRING_FLAGS,
  DEBUG_STRING_FLAGS_SHADOW,memptr,im_mode, DEBUG_STRING_IFF12 ,last_vsync_per_second
                        );
  }

			//printf ("128k. p32765=%d p8189=%d\n\r",puerto_32765,puerto_8189);


}


//Para poder saltar los step-to-step
//Evitar en step to step las rutinas de interrupciones maskable/nmi
//z80_bit debug_core_evitamos_inter={0};


//Se ha entrado en una rutina de maskable/nmi
z80_bit debug_core_lanzado_inter={0};

//Valores registro PC de retorno
z80_int debug_core_lanzado_inter_retorno_pc_nmi=0;
z80_int debug_core_lanzado_inter_retorno_pc_maskable=0;

void clear_mem_breakpoints(void)
{

	int i;

	for (i=0;i<65536;i++) {
		mem_breakpoint_array[i]=0;	
	}
}

void init_breakpoints_table(void)
{
	int i;

	//for (i=0;i<MAX_BREAKPOINTS;i++) debug_breakpoints_array[i]=-1;

	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		//debug_breakpoints_conditions_array[i][0]=0;

		
		debug_breakpoints_conditions_array_tokens[i][0].tipo=TPT_FIN;
		

	    	debug_breakpoints_actions_array[i][0]=0;
		debug_breakpoints_conditions_saltado[i]=0;
		debug_breakpoints_conditions_enabled[i]=0;

		optimized_breakpoint_array[i].optimized=0;
	}

        //for (i=0;i<MAX_BREAKPOINTS_PEEK;i++) debug_breakpoints_peek_array[i]=-1;


	clear_mem_breakpoints();
	last_active_breakpoint=0;


}


void init_watches_table(void)
{
	int i;

	for (i=0;i<DEBUG_MAX_WATCHES;i++) {
		debug_watches_array[i][0].tipo=TPT_FIN;
	}


}



//Dibuja la pantalla de panico
void screen_show_panic_screen(int xmax, int ymax)
{
    
    //int colores_rainbow[]={2+8,6+8,4+8,5+8,0};

	int x,y;


	int total_colores=5;
	int grueso_colores=8; //grueso de 8 pixeles cada franja

	//printf ("Filling colour bars up to %dX%d\n",xmax,ymax);


	for (x=0;x<xmax;x++) {
        int color=0;
		for (y=0;y<ymax;y++) {
			//scr_putpixel(x,y,(color&15) );
            scr_putpixel(x,y,screen_colores_rainbow[(color%total_colores)] );

			if ((y%grueso_colores)==grueso_colores-1) color++;

		}
	}
}

//Compile with -g -rdynamic to show function names
//In Mac, with -g
//These functions on Mac OS X are available starting from Mac OS 10.5
void debug_exec_show_backtrace(void) 
{

#if defined(linux) || defined(__APPLE__) 
  int max_items=50;
  void *array[max_items];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, max_items);

  // print out all the frames to stderr
  //fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
}


void cpu_panic_printf_mensaje(char *mensaje)
{

	char buffer[1024];

        printf ("\n\n ZEsarUX kernel panic: %s \n",mensaje);
        print_registers(buffer);
        printf ("%s\n",buffer);

}





int cpu_panic_last_x;
int cpu_panic_last_y;

int cpu_panic_xmax;
int cpu_panic_ymax;

int cpu_panic_current_tinta;
int cpu_panic_current_papel;

int cpu_panic_pixel_zoom=1;

//Escribir caracter en pantalla, teniendo coordenadas en pixeles. Colores sobre tabla de colores de spectrum
//Pixeles de 2x2 en caso de que la ventana sea al menos de 512x384
void cpu_panic_printchar_lowlevel(int x,int y,int tinta,int papel,unsigned char c)
{
    //Detectar caracteres fuera de rango
    if (c<32 || c>127) c='?';

    int indice_charset=(c-32)*8;
    //char_set_spectrum[indice_charset]

    int scanline;
    int nbit;


    for (scanline=0;scanline<8;scanline++) {
        z80_byte byte_leido=char_set_spectrum[indice_charset++];
        for (nbit=0;nbit<8;nbit++) {
            int color;
            color=(byte_leido & 128 ? tinta : papel);

            if (cpu_panic_pixel_zoom==2){
                scr_putpixel(x+nbit*2,y+scanline*2,color);
                scr_putpixel(x+nbit*2,y+scanline*2+1,color);
                scr_putpixel(x+nbit*2+1,y+scanline*2,color);
                scr_putpixel(x+nbit*2+1,y+scanline*2+1,color);
            }

            else scr_putpixel(x+nbit,y+scanline,color);

            byte_leido=byte_leido<<1;
        }
    }
}


void cpu_panic_printchar_newline(void)
{
    cpu_panic_last_x=0;
    cpu_panic_last_y+=8*cpu_panic_pixel_zoom;

    //Si llega al final
    if (cpu_panic_last_y>cpu_panic_ymax-8) cpu_panic_last_y=cpu_panic_ymax-8;
}

void cpu_panic_printchar_nextcolumn(void)
{
    cpu_panic_last_x+=8*cpu_panic_pixel_zoom;

    //Final de linea
    if (cpu_panic_last_x>cpu_panic_xmax-8) cpu_panic_printchar_newline();

}

void cpu_panic_printchar(unsigned char c)
{
    if (c==10 || c==13) cpu_panic_printchar_newline();
    else {
        cpu_panic_printchar_lowlevel(cpu_panic_last_x,cpu_panic_last_y,cpu_panic_current_tinta,cpu_panic_current_papel,c);
        cpu_panic_printchar_nextcolumn();
    }
}

void cpu_panic_printstring(char *message)
{
	while (*message) {
		cpu_panic_printchar(*message);
		message++;
	}
}

//Abortar ejecucion del emulador con kernel panic
void cpu_panic(char *mensaje)
{
	char buffer[1024];

	//Liberar bloqueo de semaforo de print, por si acaso
	debug_printf_sem_init();

	//por si acaso, antes de hacer nada mas, vamos con el printf, para que muestre el error (si es que el driver de video lo permite)
	//hacemos pantalla de panic en xwindows y fbdev, y despues de finalizar el driver, volvemos a mostrar error
	cpu_panic_printf_mensaje(mensaje);

	debug_exec_show_backtrace();

	snap_dump_zsf_on_cpu_panic();

    cpu_panic_last_x=cpu_panic_last_y=0;

    cpu_panic_current_tinta=6;
    cpu_panic_current_papel=1;


	if (scr_end_pantalla!=NULL) {

		//si es xwindows o fbdev, mostramos panic mas mono
		if (si_complete_video_driver() ) {
			//quitar splash text por si acaso
			menu_splash_segundos=1;
			reset_splash_text();


			//cls_menu_overlay();
			//set_menu_overlay_function(normal_overlay_texto_menu);
			//no tiene sentido tener el menu overlay abierto... o si?

			menu_overlay_activo=0;

            cpu_panic_xmax=screen_get_emulated_display_width_zoom_border_en();
            cpu_panic_ymax=screen_get_emulated_display_height_zoom_border_en();

            //Determinar si hacemos zoom 1 o 2, segun tamanyo total ventana
            int desired_width=32*8*2;
            int desired_height=24*8*2;

            if (cpu_panic_xmax>=desired_width && cpu_panic_ymax>=desired_height) cpu_panic_pixel_zoom=2;

			screen_show_panic_screen(cpu_panic_xmax,cpu_panic_ymax);

			print_registers(buffer);

            //Maximo 32 caracteres, aunque aprovechamos todo (border incluso) pero hay que considerar
            //por ejemplo pantalla sin border con zoom 1, en ese caso habra un minimo de 256 de ancho (32 caracteres de ancho)
                                 //01234567890123456789012345678901
            cpu_panic_printstring("******************************\n");
			cpu_panic_printstring("*  ZEsarUX kernel panic  :-( *\n");
            cpu_panic_printstring("******************************\n");
            cpu_panic_printstring("\n\n");
            cpu_panic_printstring("Panic message:\n");
			cpu_panic_printstring(mensaje);

            cpu_panic_printstring("\n\nCPU registers:\n");


			//los registros los mostramos dos lineas por debajo de la ultima usada
			cpu_panic_printstring(buffer);

			if (dumped_debug_dump_zsf_on_cpu_panic.v) {
				cpu_panic_printstring("\n\nDumped cpu panic snapshot:\n");
				cpu_panic_printstring(dump_snapshot_panic_name);
				cpu_panic_printstring("\non current directory");
				printf ("Dumped cpu panic snapshot: %s on current directory\n",dump_snapshot_panic_name);
			}
		
			scr_refresca_pantalla_solo_driver();

			//Para xwindows hace falta esto, sino no refresca
			scr_actualiza_tablas_teclado();

			sleep(20);
			scr_end_pantalla();
		}

		else {
			scr_end_pantalla();
		}
	}


	exit(1);
}


//Para calcular tiempos funciones. Iniciar contador antes
void debug_tiempo_inicial(void)
{

	gettimeofday(&debug_timer_antes, NULL);

}

//Para calcular tiempos funciones. Contar contador despues e imprimir tiempo por pantalla
void debug_tiempo_final(void)
{

	long debug_timer_mtime, debug_timer_seconds, debug_timer_useconds;

	gettimeofday(&debug_timer_ahora, NULL);

        debug_timer_seconds  = debug_timer_ahora.tv_sec  - debug_timer_antes.tv_sec;
        debug_timer_useconds = debug_timer_ahora.tv_usec - debug_timer_antes.tv_usec;

        debug_timer_mtime = ((debug_timer_seconds) * 1000 + debug_timer_useconds/1000.0) + 0.5;

        printf("Elapsed time: %ld milliseconds\n\r", debug_timer_mtime);
}

z_atomic_semaphore debug_printf_semaforo;

void debug_printf_sem_init(void)
{
	z_atomic_reset(&debug_printf_semaforo);
}

void debug_printf (int debuglevel, const char * format , ...)
{
	//Adquirir lock
	while(z_atomic_test_and_set(&debug_printf_semaforo)) {
		//printf("Esperando a liberar lock en debug_printf\n");
	} 
  	int copia_verbose_level;

  	copia_verbose_level=verbose_level;

  	if (debuglevel<=copia_verbose_level) {
		//tamaño del buffer bastante mas grande que el valor constante definido
	    char buffer_final[DEBUG_MAX_MESSAGE_LENGTH*2];
	    char buffer_inicial[DEBUG_MAX_MESSAGE_LENGTH*2+64];
	    char *verbose_message;
	    va_list args;
	    va_start (args, format);
    	vsprintf (buffer_inicial,format, args);
    	va_end (args);

		//TODO: controlar maximo mensaje


    	switch (debuglevel) {
			case VERBOSE_ERR:
				verbose_message=VERBOSE_MESSAGE_ERR;
			break;

			case VERBOSE_WARN:
				verbose_message=VERBOSE_MESSAGE_WARN;
			break;

			case VERBOSE_INFO:
				verbose_message=VERBOSE_MESSAGE_INFO;
			break;

			case VERBOSE_DEBUG:
				verbose_message=VERBOSE_MESSAGE_DEBUG;
			break;

        	case VERBOSE_PARANOID:
            	verbose_message=VERBOSE_MESSAGE_PARANOID;
        	break;


			default:
				verbose_message="UNKNOWNVERBOSELEVEL";
			break;

    	}

    	sprintf (buffer_final,"%s%s",verbose_message,buffer_inicial);

    	if (scr_messages_debug!=NULL) scr_messages_debug (buffer_final);
    	else printf ("%s\n",buffer_final);

		//Si tambien queremos mostrar log en consola,
		//esto es un caso un tanto especial pues la mayoria de drivers ya muestra mensajes en consola,
		//excepto curses, caca y aa lib, pues muestran 1 solo mensaje dentro de la interfaz del emulador
		//En esos casos puede ser necesario que el mensaje salga tal cual en consola, con scroll, aunque se desplace toda la interfaz
		//pero ayudara a que se vean los mensajes
		if (debug_always_show_messages_in_console.v) printf ("%s\n",buffer_final);

    	//Hacer aparecer menu, siempre que el driver no sea null ni.. porque no inicializado tambien? no inicializado
    	if (debuglevel==VERBOSE_ERR) {

			//en el caso de stdout, no aparecera ventana igualmente, pero el error ya se vera por consola
        	if (
				!strcmp(scr_driver_name,"stdout") ||
        		!strcmp(scr_driver_name,"simpletext") ||
        		!strcmp(scr_driver_name,"null") 
			)
			{
				//nada
			}

        	else {
	        	sprintf (pending_error_message,"%s",buffer_inicial);
    	    	if_pending_error_message=1;
        		menu_fire_event_open_menu();
			}
    	}

	}


	//Liberar lock
	z_atomic_reset(&debug_printf_semaforo);

}


//igual que debug_printf pero mostrando nombre archivo fuente y linea
//util para debug con modo debug o paranoid. mensajes de info o warn no tienen sentido mostrar archivo fuente
//Usar con, ejemplo:
//debug_printf_source (VERBOSE_DEBUG, __FILE__, __LINE__, __FUNCTION__, "Probando mensaje"); 
//o usando un macro que he definido:
//debug_printf_source (VERBOSE_DEBUG_SOURCE, "Probando mensaje");
void debug_printf_source (int debuglevel, char *archivo, int linea, const char *funcion, const char * format , ...)
{
  int copia_verbose_level;

  copia_verbose_level=verbose_level;

  if (debuglevel<=copia_verbose_level) {
        //tamaño del buffer bastante mas grande que el valor constante definido
    char buffer_inicial[DEBUG_MAX_MESSAGE_LENGTH*2+64];
    va_list args;
    va_start (args, format);
    vsprintf (buffer_inicial,format, args);
    va_end (args);
    debug_printf (debuglevel,"%s:%d (%s) %s",archivo,linea,funcion,buffer_inicial);
  }


}


int debug_nested_id_poke_byte;
int debug_nested_id_poke_byte_no_time;
int debug_nested_id_peek_byte;
int debug_nested_id_peek_byte_no_time;


void do_breakpoint_exception(char *message)
{
	if (strlen(message)>MAX_MESSAGE_CATCH_BREAKPOINT-1) {
		cpu_panic("do_breakpoint_exception: strlen>MAX_MESSAGE_CATCH_BREAKPOINT");
	}

	menu_breakpoint_exception.v=1;
	sprintf(catch_breakpoint_message,"%s",message);
	debug_printf (VERBOSE_INFO,"Catch breakpoint: %s",message);
}

void set_peek_byte_function_debug(void)
{

	debug_printf(VERBOSE_INFO,"Enabling debug on MMU");

	//peek_byte_no_time
	//peek_byte_no_time_no_debug=peek_byte_no_time;
	//peek_byte_no_time=peek_byte_no_time_debug;

	//peek_byte_time
	//peek_byte_no_debug=peek_byte;
	//peek_byte=peek_byte_debug;

	//poke_byte_no_time
	//poke_byte_no_time_no_debug=poke_byte_no_time;
	//poke_byte_no_time=poke_byte_no_time_debug;

	//poke_byte
	//poke_byte_no_debug=poke_byte;
	//poke_byte=poke_byte_debug;

	//out port
	//TODO. funciones out y lee_puerto de aqui habra que meterlas en una lista nested si se empiezan a usar en mas sitios...
	out_port_no_debug=out_port;
	out_port=out_port_debug;

	//lee puerto
	lee_puerto_no_debug=lee_puerto;
	lee_puerto=lee_puerto_debug;

        debug_nested_id_poke_byte=debug_nested_poke_byte_add(poke_byte_debug,"Debug poke_byte");
        debug_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(poke_byte_no_time_debug,"Debug poke_byte_no_time");
        debug_nested_id_peek_byte=debug_nested_peek_byte_add(peek_byte_debug,"Debug peek_byte");
        debug_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(peek_byte_no_time_debug,"Debug peek_byte_no_time");


}

void reset_peek_byte_function_debug(void)
{
	debug_printf(VERBOSE_INFO,"Clearing debug on MMU");

	//peek_byte_no_time=peek_byte_no_time_no_debug;
	//peek_byte=peek_byte_no_debug;

	//poke_byte_no_time=poke_byte_no_time_no_debug;
	//poke_byte=poke_byte_no_debug;

	out_port=out_port_no_debug;
	lee_puerto=lee_puerto_no_debug;


        debug_nested_poke_byte_del(debug_nested_id_poke_byte);
        debug_nested_poke_byte_no_time_del(debug_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(debug_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(debug_nested_id_peek_byte_no_time);
}



z80_byte peek_byte_no_time_debug (z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor;

	anterior_debug_mmu_mra=debug_mmu_mra;
	debug_mmu_mra=dir;

	//valor=peek_byte_no_time_no_debug(dir);
	valor=debug_nested_peek_byte_no_time_call_previous(debug_nested_id_peek_byte_no_time,dir);

	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)



	return valor;
}


z80_byte peek_byte_debug (z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor;

	anterior_debug_mmu_mra=debug_mmu_mra;
	debug_mmu_mra=dir;

        //valor=peek_byte_no_debug(dir);
	valor=debug_nested_peek_byte_call_previous(debug_nested_id_peek_byte,dir);

	debug_mmu_mrv=valor; //Memory Read Value (valor leido en peek)


	//cpu_core_loop_debug_check_breakpoints();


	return valor;

}


z80_byte poke_byte_no_time_debug(z80_int dir,z80_byte value)
{
	debug_mmu_mwv=value;
	anterior_debug_mmu_mwa=debug_mmu_mwa;
	debug_mmu_mwa=dir;

	//poke_byte_no_time_no_debug(dir,value);
	debug_nested_poke_byte_no_time_call_previous(debug_nested_id_poke_byte_no_time,dir,value);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;

}

z80_byte poke_byte_debug(z80_int dir,z80_byte value)
{
	debug_mmu_mwv=value;
	anterior_debug_mmu_mwa=debug_mmu_mwa;
	debug_mmu_mwa=dir;

	//poke_byte_no_debug(dir,value);
	debug_nested_poke_byte_call_previous(debug_nested_id_poke_byte,dir,value);

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;
}

void out_port_debug(z80_int puerto,z80_byte value)
{
        debug_mmu_pwv=value;
        debug_mmu_pwa=puerto;

	out_port_no_debug(puerto,value);
}

z80_byte lee_puerto_debug(z80_byte puerto_h,z80_byte puerto_l)
{
	z80_byte valor;

        debug_mmu_pra=value_8_to_16(puerto_h,puerto_l);

        valor=lee_puerto_no_debug(puerto_h,puerto_l);

        debug_mmu_prv=valor;


	return valor;
}



//Mostrar mensaje que ha hecho saltar el breakpoint y ejecutar accion (por defecto abrir menu)
void cpu_core_loop_debug_breakpoint(char *message)
{
	menu_abierto=1;
	do_breakpoint_exception(message);
}










#define BREAKPOINT_CONDITION_OP_AND 0
#define BREAKPOINT_CONDITION_OP_OR 1
#define BREAKPOINT_CONDITION_OP_XOR 2

#define BREAKPOINT_MAX_OPERADORES 3

char *breakpoint_cond_operadores[BREAKPOINT_MAX_OPERADORES]={
	" and ", " or ", " xor "
};








/*
Sobre el parser optimizado y otras optimizaciones. Usos de cpu antes y ahora:

-hasta ayer. 
--noconfigfile --set-breakpoint 1 bc=4444 ; 52 %  cpu

-con nuevo parser que permite meter registros y valores en ambos lados del operador de comparación
--noconfigfile --set-breakpoint 1 bc=4444 ; 70 % cpu

-con optimizaron si valor empieza por dígito:
54%

-con optimizacion metiendo comparación de “pc” arriba del todo y condición:
51% cpu. Con codigo de ayer 51%


-si valor no empieza por dígito, por ejemplo PC=CCCCH. Uso cpu 70%



——————

Optimizando expresiones PC=XXXX, MRA=XXXX, MWA=XXXX, optimizado basado en lo comentado con Thomas Busse

**1 breakpoint

--noconfigfile --set-breakpoint 1 pc=4444
-con optimizado 
 43% cpu

-con parser anterior 
55%cpu


** Con 10 breakpoints. 
./zesarux --noconfigfile --set-breakpoint 1 pc=40000 --set-breakpoint 2 pc=40001 --set-breakpoint 3 pc=40002 --set-breakpoint 4 pc=40003 --set-breakpoint 5 pc=40005 --set-breakpoint 6 pc=40006 --set-breakpoint 7 pc=40007 --set-breakpoint 8 pc=40008 --set-breakpoint 9 pc=40009 --set-breakpoint 10 pc=40010

-con optimizado  45 %

-con parser anterior   85% cpu. (Desactivando auto frameskip. con autoframeskip, hace 75% cpu, 2 FPS


*/

//Parsea un breakpoint optimizado, basado en codigo de Thomas Busse
int debug_breakpoint_condition_optimized(int indice)
{

	int tipo_optimizacion;

    tipo_optimizacion=optimized_breakpoint_array[indice].operator;

	unsigned int valor;
	unsigned int valor_variable;

	valor=optimized_breakpoint_array[indice].valor;

	//Segun el tipo
	switch (tipo_optimizacion) {
		case OPTIMIZED_BRK_TYPE_PC:
			valor_variable=reg_pc;
		break;

		case OPTIMIZED_BRK_TYPE_MRA:
			valor_variable=debug_mmu_mra;
		break;

		case OPTIMIZED_BRK_TYPE_MWA:
			valor_variable=debug_mmu_mwa;
		break;		

		default:
			return 0;
		break;
	}

	if (valor_variable==valor) {
		debug_printf (VERBOSE_DEBUG,"Fired optimized breakpoint. Optimizer type: %d value: %04XH",tipo_optimizacion,valor);
		return 1;
	}

	//printf ("NOT return variable is ok from optimizer tipo: %d valor: %d\n",tipo_optimizacion,valor);

	return 0;
}

void debug_set_mem_breakpoint(z80_int dir,z80_byte brkp_type)
{
	mem_breakpoint_array[dir]=brkp_type;
}
	


//Ver si salta breakpoint y teniendo en cuenta setting de saltar siempre o con cambio
int cpu_core_loop_debug_check_mem_brkp_aux(unsigned int dir, z80_byte tipo_mascara, unsigned int anterior_dir)
{

	//dir no deberia estar fuera de rango 0...65535. Pero por si acaso...
	if (dir<0 || dir>65535) return 0;

	if (mem_breakpoint_array[dir] & tipo_mascara) {
		//Coincide condicion

		int saltar_breakpoint=0;

                //Setting de saltar siempre
                if (debug_breakpoints_cond_behaviour.v==0) saltar_breakpoint=1;

                else {
                        //Solo saltar con cambio
                        if (dir != anterior_dir) saltar_breakpoint=1;
                }

		return saltar_breakpoint;
	}
	else return 0;
}

void cpu_core_loop_debug_check_mem_breakpoints(void)
{

	
//	mem_breakpoint_array
//Ver si coincide mra o mwa
/*
z80_int debug_mmu_mra; //Memory Read Addres (direccion usada peek)
z80_int debug_mmu_mwa; //Memory Write Address (direccion usada en poke)

//Anteriores valores para mra y mwa. De momento usado en los nuevos memory-breakpoints
//Si es -1, no hay valor anterior
int anterior_debug_mmu_mra=-1;
int anterior_debug_mmu_mwa=-1;
*/

	//Probar mra
	if (cpu_core_loop_debug_check_mem_brkp_aux(debug_mmu_mra,1,anterior_debug_mmu_mra)) {
		//Hacer saltar breakpoint MRA
		//printf ("Saltado breakpoint MRA en %04XH PC=%04XH\n",debug_mmu_mra,reg_pc);
		catch_breakpoint_index=-1;


		char buffer_mensaje[100];
		sprintf(buffer_mensaje,"Memory Breakpoint Read Address: %04XH",debug_mmu_mra);
                cpu_core_loop_debug_breakpoint(buffer_mensaje);

		//Cambiar esto tambien aqui porque si no escribiera en los siguientes opcodes, no llamaria a peek_debug y por tanto no 
		//cambiaria esto. Aunque es absurdo porque al leer opcodes siempre esta cambiando MRA. por tanto lo comento, solo
		//tiene sentido para mwa
		//anterior_debug_mmu_mra=debug_mmu_mra;

	}

	//Probar mwa
    if (cpu_core_loop_debug_check_mem_brkp_aux(debug_mmu_mwa,2,anterior_debug_mmu_mwa)) {
                //Hacer saltar breakpoint MWA
                //printf ("Saltado breakpoint MWA en %04XH PC=%04XH\n",debug_mmu_mwa,reg_pc);
		catch_breakpoint_index=-1;

		char buffer_mensaje[100];
		sprintf(buffer_mensaje,"Memory Breakpoint Write Address: %04XH",debug_mmu_mwa);
                cpu_core_loop_debug_breakpoint(buffer_mensaje);

		//Cambiar esto tambien aqui porque si no escribiera en los siguientes opcodes, no llamaria a poke_debug y por tanto no 
		//cambiaria esto
		anterior_debug_mmu_mwa=debug_mmu_mwa;

        }

	

}

//Establece variable al ultimo activo+1
void debug_set_last_active_breakpoint(void)
{
	int i;
	for (i=MAX_BREAKPOINTS_CONDITIONS-1;i>=0;i--) {
		if (debug_breakpoints_conditions_enabled[i]) {
			//Esta activado, pero tiene contenido?


			
			if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {
				last_active_breakpoint=i+1;
				debug_printf (VERBOSE_DEBUG,"Last active breakpoint +1: %d",last_active_breakpoint);
				return;				
			}
					
	


		}
		
	}

	last_active_breakpoint=0; //no hay breakpoints activos
	debug_printf (VERBOSE_DEBUG,"Last active breakpoint +1: %d",last_active_breakpoint);
}


//conmutar enabled/disabled
void debug_breakpoints_conditions_toggle(int indice)
{
	//printf ("Ejecutada funcion para espacio, condicion: %d\n",valor_opcion);

	debug_breakpoints_conditions_enabled[indice] ^=1;

	//si queda activo, decir que no ha saltado aun ese breakpoint
	if (debug_breakpoints_conditions_enabled[indice]) {
		debug_breakpoints_conditions_saltado[indice]=0;
	}

	debug_set_last_active_breakpoint();
}


void debug_breakpoints_conditions_enable(int indice)
{
  debug_breakpoints_conditions_enabled[indice]=1;
  debug_set_last_active_breakpoint();
}

void debug_breakpoints_conditions_disable(int indice)
{
  debug_breakpoints_conditions_enabled[indice]=0;
  debug_set_last_active_breakpoint();
}




//Comprobar condiciones. Usando nuevo breakpoint parser.  Solo lo hacemos en core_loop
void cpu_core_loop_debug_check_breakpoints(void)
{
	//Condicion de debug
	if (debug_breakpoints_enabled.v) {

		//Comprobar los mem-breakpoints
		cpu_core_loop_debug_check_mem_breakpoints();

		int i;

		//Breakpoint de condicion
		//for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		for (i=0;i<last_active_breakpoint;i++) {
			//Si ese breakpoint esta activo
			if (debug_breakpoints_conditions_enabled[i]) {
				if (debug_breakpoints_conditions_array_tokens[i][0].tipo!=TPT_FIN) {

					int se_cumple_breakpoint;
					//printf ("Checking breakpoint %d\n",i);
					//Si esta optimizado

					if (optimized_breakpoint_array[i].optimized) {
						//printf ("Parsing optimized breakpoint\n");
						se_cumple_breakpoint=debug_breakpoint_condition_optimized(i);
					}
					else {
						//se_cumple_breakpoint=debug_breakpoint_condition_loop(&debug_breakpoints_conditions_array[i][0],0);
						int error_code;
						se_cumple_breakpoint=exp_par_evaluate_token(debug_breakpoints_conditions_array_tokens[i],MAX_PARSER_TOKENS_NUM,&error_code);
						//Nota: aqui no comprobamos error_code, gestionamos el valor de retorno tal cual vuelve, haya habido o no error
					}

					if ( se_cumple_breakpoint ) {
						//Si condicion pasa de false a true o bien el comportamiento por defecto es saltar siempre
						if (debug_breakpoints_cond_behaviour.v==0 || debug_breakpoints_conditions_saltado[i]==0) {
							debug_breakpoints_conditions_saltado[i]=1;

							char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
							exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);

	        	        	char buffer_mensaje[MAX_BREAKPOINT_CONDITION_LENGTH+64];
        	    	    	sprintf(buffer_mensaje,"%s",buffer_temp);

	                    	//Ejecutar accion, por defecto es abrir menu
							catch_breakpoint_index=i;
        	        		cpu_core_loop_debug_breakpoint(buffer_mensaje);
						}
            		}
					else {
						//No se cumple condicion. Indicarlo que esa condicion esta false
						debug_breakpoints_conditions_saltado[i]=0;
					}
    	    	}
			}
    	}

    }

}





//int debug_watches_mostrado_frame=0;
//char debug_watches_texto_destino[1024];

//Misma limitacion de longitud que un breakpoint.
//Si cadena vacia, no hay breakpoint
//char debug_watches_text_to_watch[MAX_BREAKPOINT_CONDITION_LENGTH]="";

//z80_byte debug_watches_y_position=0;

//void cpu_core_loop_debug(void)

int debug_nested_id_core;
z80_byte cpu_core_loop_debug(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	debug_fired_out=0;
	//Si se ejecuta un out en el core (justo despues que esto) se activara dicha variable

	debug_fired_in=0;
	//Si se ejecuta un in en el core (justo despues que esto) se activara dicha variable

	debug_fired_interrupt=0;
	//Si se lanza una interrupcion en el core (justo despues que esto) se activara dicha variable


  	//Llamamos al core normal
	debug_nested_core_call_previous(debug_nested_id_core);


  //Evaluamos condiciones debug

	//Condiciones enterrom y exitrom
/*
//Avisa cuando se ha entrado o salido de rom. Solo salta una vez el breakpoint
//0: no esta en rom
//1: esta en rom y aun no ha saltado breakpoint
//2: esta en rom y ya ha saltado breakpoint
int debug_enterrom=0;

//0: no ha salido de rom
//1: ha salido de rom y aun no ha saltado breakpoint
//2: ha salido de rom y ya ha saltado breakpoint
int debug_exitrom=0;
*/

	if (reg_pc<16384) {
		//no ha salido de rom
		debug_exitrom=0;

		//ver si hay que avisar de un enterrom
		if (debug_enterrom==0) debug_enterrom=1;
	}

	if (reg_pc>16383) {
		//no esta en rom
		debug_enterrom=0;

		//ver si hay que avisar de un exitrom
		if (debug_exitrom==0) debug_exitrom=1;
	}

	cpu_core_loop_debug_check_breakpoints();





	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;

}



void set_cpu_core_loop(void)
{
        switch (cpu_core_loop_active) {

                case CPU_CORE_SPECTRUM:
                        debug_printf(VERBOSE_INFO,"Setting Spectrum CPU core");
			if (core_spectrum_uses_reduced.v==0) {
	                        cpu_core_loop=cpu_core_loop_spectrum;
			}
			else {
				debug_printf(VERBOSE_WARN,"Setting REDUCED Spectrum CPU core, the following features will NOT be available or will NOT be properly emulated: Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, RZX, Raster interrupts, TBBlue Copper, Audio DAC, Video out to file");
				cpu_core_loop=cpu_core_loop_reduced_spectrum;
			}
                        cpu_core_loop_name="Spectrum";
                break;

                case CPU_CORE_ZX8081:
                        debug_printf(VERBOSE_INFO,"Setting ZX80/81 CPU core");
                        cpu_core_loop=cpu_core_loop_zx8081;
                        cpu_core_loop_name="ZX80/81";
                break;

                case CPU_CORE_ACE:
                        debug_printf(VERBOSE_INFO,"Setting Jupiter ACE core");
                        cpu_core_loop=cpu_core_loop_ace;
                        cpu_core_loop_name="Jupiter ACE";
                break;

                case CPU_CORE_CPC:
                        debug_printf(VERBOSE_INFO,"Setting CPC core");
                        cpu_core_loop=cpu_core_loop_cpc;
                        cpu_core_loop_name="CPC";
                break;

                case CPU_CORE_Z88:
                        debug_printf(VERBOSE_INFO,"Setting Z88 CPU core");
                        cpu_core_loop=cpu_core_loop_z88;
                        cpu_core_loop_name="Z88";
                break;

		case CPU_CORE_SAM:
			debug_printf(VERBOSE_INFO,"Setting Sam Coupe CPU core");
			cpu_core_loop=cpu_core_loop_sam;
      cpu_core_loop_name="Sam Coupe";
		break;

		case CPU_CORE_QL:
			debug_printf(VERBOSE_INFO,"Setting QL CPU core");
			cpu_core_loop=cpu_core_loop_ql;
      cpu_core_loop_name="QL";
		break;

    case CPU_CORE_MK14:
      debug_printf(VERBOSE_INFO,"Setting MK14 CPU core");
      cpu_core_loop=cpu_core_loop_mk14;
      cpu_core_loop_name="MK14";
    break;


                default:
                        cpu_panic("Unknown cpu core");
                break;

        }

	/*
        //Activar core de debug si es necesario
        if (debug_cpu_core_loop.v) {
                debug_printf(VERBOSE_INFO,"Enabling debug on cpu core");
                cpu_core_loop_no_debug=cpu_core_loop;
                cpu_core_loop=cpu_core_loop_debug;
        }
	*/

}


void set_cpu_core_loop_debug(void)
{
	debug_printf(VERBOSE_INFO,"Enabling debug on cpu core");
	debug_nested_id_core=debug_nested_core_add(cpu_core_loop_debug,"Debug core");
}

void reset_cpu_core_loop_debug(void)
{
	debug_printf(VERBOSE_INFO,"Disabling debug on cpu core");
	debug_nested_core_del(debug_nested_id_core);
}



void breakpoints_enable(void)
{
	debug_breakpoints_enabled.v=1;
	//debug_cpu_core_loop.v=1;
	set_peek_byte_function_debug();

        set_cpu_core_loop_debug();


}

void breakpoints_disable(void)
{
	debug_breakpoints_enabled.v=0;
	//debug_cpu_core_loop.v=0;
	reset_peek_byte_function_debug();

        reset_cpu_core_loop_debug();

}


void machine_emulate_memory_refresh_debug_counter(void)
{
        debug_printf (VERBOSE_DEBUG,"Emulate_memory_refresh_counter: %d Limit: %d Max: %d %s",machine_emulate_memory_refresh_counter,MAX_EMULATE_MEMORY_REFRESH_LIMIT,MAX_EMULATE_MEMORY_REFRESH_COUNTER,(machine_emulate_memory_refresh_counter>MAX_EMULATE_MEMORY_REFRESH_LIMIT ? "Beyond Limit" : "") );

}



//Puntero a la funcion original
//void (*cpu_core_loop_no_transaction_log) (void);

z80_bit cpu_transaction_log_enabled={0};
char transaction_log_dumpassembler[32];
size_t transaction_log_longitud_opcode;

z80_bit cpu_history_enabled={0};
int cpu_history_nested_id_core;

z80_bit cpu_code_coverage_enabled={0};
int cpu_code_coverage_nested_id_core;


z80_bit extended_stack_enabled={0};
int extended_stack_nested_id_core;

//Array para el code coverage. De momento solo tiene el contenido:
//0: no ha ejecutado la cpu esa dirección
//diferente de 0: ha ejecutado la cpu esa dirección
//en el futuro se pueden usar mas bits de cada elemento
z80_byte cpu_code_coverage_array[65536];

FILE *ptr_transaction_log=NULL;

char transaction_log_filename[PATH_MAX];


//Tamanyo del archivo de transaction log. Para leer desde aqui en vez de usar ftell para saber que tamanyo tiene, que es mas rapido
long transaction_log_tamanyo_escrito=0;

//Lineas del archivo de transaction log
long transaction_log_tamanyo_lineas=0;

char transaction_log_line_to_store[2048];


//campos a guardar en el transaction log
z80_bit cpu_transaction_log_store_datetime={0};
z80_bit cpu_transaction_log_store_address={1};
z80_bit cpu_transaction_log_store_tstates={0};
z80_bit cpu_transaction_log_store_opcode={1};
z80_bit cpu_transaction_log_store_registers={0};

//Si se habilita rotacion del transaction log
z80_bit cpu_transaction_log_rotate_enabled={0};
//Numero de archivos rotados
int cpu_transaction_log_rotated_files=10;
//Tamanyo maximo antes de rotar archivo, en MB. Si es 0, no rotar
int cpu_transaction_log_rotate_size=100;

//Lineas maximas antes de rotar archivo. Si es 0, no rotar
int cpu_transaction_log_rotate_lines=1000000;


int transaction_log_nested_id_core;




//Para tener una memory zone que apunte a un archivo
char memory_zone_by_file_name[PATH_MAX];
z80_byte *memory_zone_by_file_pointer;
int memory_zone_by_file_size=0;

//Si ultima instruccion era HALT. Para ignorar hasta lo que no sea HALT. Contar al menos 1
int cpu_trans_log_last_was_halt=0;

z80_bit cpu_trans_log_ignore_repeated_halt={0};




void transaction_log_rotate_files(int archivos)
{
	//Primero cerrar archivo
	transaction_log_close_file();

	//Borrar el ultimo
	char buffer_last_file[PATH_MAX];

	sprintf(buffer_last_file,"%s.%d",transaction_log_filename,archivos);

	debug_printf (VERBOSE_DEBUG,"Erasing oldest transaction log file %s",buffer_last_file);

	util_delete(buffer_last_file);

	//Y renombrar, el penultimo al ultimo, el antepenultimo al penultimo, etc
	//con 10 archivos:
	//ren file.9 file.10
	//ren file.8 file.9
	//ren file.7 file.8
	//...
	//ren file.1 file.2 
	//ren file file.1 esto a mano
	int i;

	for (i=archivos-1;i>=0;i--) {
		char buffer_file_orig[PATH_MAX];
		char buffer_file_dest[PATH_MAX];

		//Caso especial ultimo (seria el .0)
		if (i==0) {
			strcpy(buffer_file_orig,transaction_log_filename);
		}
		else {
			sprintf(buffer_file_orig,"%s.%d",transaction_log_filename,i);
		}

		sprintf(buffer_file_dest,"%s.%d",transaction_log_filename,i+1);

		debug_printf (VERBOSE_DEBUG,"Renaming transaction log file %s -> %s",buffer_file_orig,buffer_file_dest);
		rename(buffer_file_orig,buffer_file_dest);
	}


	//Y finalmente truncar
	transaction_log_truncate();

	//Y tenemos que abrirlo a mano
	transaction_log_open_file();
}

void transaction_log_rotate_by_size(void)
{


	if (cpu_transaction_log_rotate_enabled.v==0) return;

	if (cpu_transaction_log_rotate_size==0) return; //no rotar si vale 0

	//Obtener tamanyo archivo a ver si hay que rotar o no
	//nota: dado que el flush en mac por ejemplo se hace muy de vez en cuando, ver el tamanyo del archivo
	//tal cual con la estructura en memoria, no mirando el archivo en disco

	//ftell es muy lento
	//long tamanyo=ftell(ptr_transaction_log);


	long tamanyo=transaction_log_tamanyo_escrito;

	//printf ("posicion: (tamanyo) %ld\n",tamanyo);

	//Si hay que rotar
	

	long tamanyo_a_rotar=cpu_transaction_log_rotate_size;

	//Pasar tamanyo a bytes
	tamanyo_a_rotar *=1024;
	tamanyo_a_rotar *=1024;

	if (tamanyo>=tamanyo_a_rotar) {
		debug_printf (VERBOSE_DEBUG,"Rotating transaction log. File size %ld exceeds maximum %ld",tamanyo,tamanyo_a_rotar);
		transaction_log_rotate_files(cpu_transaction_log_rotated_files);
	}
}

void transaction_log_rotate_by_lines(void)
{


	if (cpu_transaction_log_rotate_enabled.v==0) return;

	if (cpu_transaction_log_rotate_lines==0) return; //no rotar si vale 0


	long tamanyo=transaction_log_tamanyo_lineas;

	//printf ("posicion: (tamanyo) %ld\n",tamanyo);

	//Si hay que rotar
	

	long tamanyo_a_rotar=cpu_transaction_log_rotate_lines;

	if (tamanyo>=tamanyo_a_rotar) {
		debug_printf (VERBOSE_DEBUG,"Rotating transaction log. File lines %ld exceeds maximum %ld",tamanyo,tamanyo_a_rotar);
		transaction_log_rotate_files(cpu_transaction_log_rotated_files);
	}
}



int transaction_log_set_rotate_number(int numero)
{
	if (numero<1 || numero>999) {
        return 1;
	}


	cpu_transaction_log_rotated_files=numero;
	return 0;
}


int transaction_log_set_rotate_size(int numero)
{
	if (numero<0 || numero>9999) {
        return 1;
	}


	cpu_transaction_log_rotate_size=numero;
	return 0;
}

int transaction_log_set_rotate_lines(int numero)
{
	if (numero<0 || numero>2147483647) { //maximo 2^31-1
        return 1;
	}


	cpu_transaction_log_rotate_lines=numero;
	return 0;
}



z80_byte cpu_core_loop_transaction_log(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{


	//Si la cpu ha acabado un ciclo y esta esperando final de frame, no hacer nada
	if (esperando_tiempo_final_t_estados.v==0) {

		int index=0;

        if (cpu_transaction_log_store_datetime.v) {

	                //fecha grabacion
			/*
        	        time_t tiempo = time(NULL);
                	struct tm tm = *localtime(&tiempo);

			//funciones localtime no son tan precisas como gettimeofday
			//parece que localtime tarda unos milisegundos en actualizar los segundos
			//y aparecen tiempos como
			//10:00:01.9999
			//10:00:01.0003   <-aqui deberia haber saltado el segundo
			//10:00:01.0005
			//10:00:02.0008

			/
			*/

			int longitud=debug_get_timestamp(&transaction_log_line_to_store[index]);

			index +=longitud;

			//Agregar espacio
			transaction_log_line_to_store[index++]=' ';
			transaction_log_line_to_store[index++]=0;

        }

		if (cpu_transaction_log_store_tstates.v) {
			sprintf (&transaction_log_line_to_store[index],"%05d ",t_estados);
			index +=6;
		}

    menu_z80_moto_int registro_pc=get_pc_register();
    registro_pc=adjust_address_space_cpu(registro_pc);

		if (cpu_transaction_log_store_address.v) {
      if (CPU_IS_MOTOROLA) {
        sprintf (&transaction_log_line_to_store[index],"%05X ",registro_pc);
        index +=6;
      }
			else {
        sprintf (&transaction_log_line_to_store[index],"%04X ",registro_pc);
			  index +=5;
      }
		}





	        if (cpu_transaction_log_store_opcode.v) {
			debugger_disassemble(&transaction_log_line_to_store[index],32,&transaction_log_longitud_opcode,registro_pc);
			int len=strlen(&transaction_log_line_to_store[index]);
			index +=len;
			transaction_log_line_to_store[index++]=' ';
        	}


		//Si es halt lo ultimo
		if (cpu_trans_log_ignore_repeated_halt.v) {
			if (CPU_IS_Z80) {
				z80_byte opcode=peek_byte_no_time(registro_pc);
				if (opcode==118) {
					if (cpu_trans_log_last_was_halt<2) cpu_trans_log_last_was_halt++;
					//printf ("halts %d\n",cpu_trans_log_last_was_halt);

				}
				else {
					cpu_trans_log_last_was_halt=0;
				}

			}	
			else {
				cpu_trans_log_last_was_halt=0;
			}	
		}

		if (cpu_transaction_log_store_registers.v) {
			print_registers(&transaction_log_line_to_store[index]);
                	int len=strlen(&transaction_log_line_to_store[index]);
	                index +=len;
        	        transaction_log_line_to_store[index++]=' ';
	        }

		//salto de linea
		transaction_log_line_to_store[index++]=10;

		//Si esta NULL es que no se ha abierto correctamente, y aqui no deberiamos llegar nunca
		if (ptr_transaction_log!=NULL) {

			//Si era halt los dos ultimos y hay que ignorarlo
			if (cpu_trans_log_ignore_repeated_halt.v && cpu_trans_log_last_was_halt>1) {
				//no hacer log
			}
			else {

				fwrite(transaction_log_line_to_store,1,index,ptr_transaction_log);

				transaction_log_tamanyo_escrito +=index;

				transaction_log_tamanyo_lineas++;
			}
		}




		//Rotar log si conviene por tamanyo
		transaction_log_rotate_by_size();

		//Rotar log si conviene por lineas
		transaction_log_rotate_by_lines();		

	}

	//Llamar a core anterior
	debug_nested_core_call_previous(transaction_log_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}

void transaction_log_close_file(void)
{
	if (ptr_transaction_log!=NULL) {
		fclose(ptr_transaction_log);
		ptr_transaction_log=NULL;
	}	
}

//Retorna 1 si error
int transaction_log_open_file(void)
{

  transaction_log_tamanyo_escrito=0; 
  transaction_log_tamanyo_lineas=0;

  //Si el archivo existia, inicializar tamanyo, no poner a 0

  if (si_existe_archivo(transaction_log_filename)) {
	 transaction_log_tamanyo_escrito=get_file_size(transaction_log_filename);
	 

	 //tiempo antes
	//char tiempo[200];
	//debug_get_timestamp(tiempo);
	//printf ("tiempo antes leer archivo: %s\n",tiempo);

	transaction_log_tamanyo_lineas=get_file_lines(transaction_log_filename);

	//debug_get_timestamp(tiempo);
	//printf ("tiempo despues leer archivo: %s\n",tiempo);

	 //tiempo despues

  }

  debug_printf (VERBOSE_DEBUG,"Transaction log file size: %ld lines: %ld",transaction_log_tamanyo_escrito,transaction_log_tamanyo_lineas);

  ptr_transaction_log=fopen(transaction_log_filename,"ab");
  if (!ptr_transaction_log) {
 		debug_printf (VERBOSE_ERR,"Unable to open Transaction log");
		debug_nested_core_del(transaction_log_nested_id_core);
		return 1;
	}	



	return 0;
}

void transaction_log_truncate(void)
{

 	if (ptr_transaction_log) {
        transaction_log_close_file();
        util_truncate_file(transaction_log_filename);
        transaction_log_open_file();    
    }
    else {
        util_truncate_file(transaction_log_filename);
    }

}

//Truncar los logs rotados
void transaction_log_truncate_rotated(void)
{

 	

	int archivos=cpu_transaction_log_rotated_files;
	int i;

	for (i=1;i<=archivos;i++) {
		
		char buffer_file_dest[PATH_MAX];

		sprintf(buffer_file_dest,"%s.%d",transaction_log_filename,i);

		debug_printf (VERBOSE_DEBUG,"Truncating rotated transaction log file %s",buffer_file_dest);
		util_truncate_file(buffer_file_dest);
	}


}

void set_cpu_core_transaction_log(void)
{
        debug_printf(VERBOSE_INFO,"Enabling Transaction Log");
	if (cpu_transaction_log_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	transaction_log_nested_id_core=debug_nested_core_add(cpu_core_loop_transaction_log,"Transaction Log Core");


	if (transaction_log_open_file()) return;



	cpu_transaction_log_enabled.v=1;																

}

void reset_cpu_core_transaction_log(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Transaction Log");
	if (cpu_transaction_log_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(transaction_log_nested_id_core);
	cpu_transaction_log_enabled.v=0;

	transaction_log_close_file();

}

void cpu_code_coverage_clear(void)
{
  int i;
  for (i=0;i<65536;i++) cpu_code_coverage_array[i]=0;
}

z80_byte cpu_core_loop_code_coverage(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{


	//hacer cosas antes...
	//printf ("running cpu code coverage addr: %04XH\n",reg_pc);
	
	int indice=reg_pc & 0xffff;
	
	cpu_code_coverage_array[indice]=1;

	//Llamar a core anterior
	debug_nested_core_call_previous(cpu_code_coverage_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}



void set_cpu_core_code_coverage(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Cpu code coverage");

	if (cpu_code_coverage_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	cpu_code_coverage_nested_id_core=debug_nested_core_add(cpu_core_loop_code_coverage,"CPU code coverage Core");



	cpu_code_coverage_enabled.v=1;
	cpu_code_coverage_clear();
																

}

void reset_cpu_core_code_coverage(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Cpu code coverage");
	if (cpu_code_coverage_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(cpu_code_coverage_nested_id_core);
	cpu_code_coverage_enabled.v=0;


}


// Codigo para extended stack




//Array con todo el extended stack
struct s_extended_stack_item extended_stack_array_items[65536];

//Retornar el tipo de valor de extended stack 
char *extended_stack_get_string_type(z80_byte tipo)
{

	//Algunas comprobaciones por si acaso
	if (tipo>=TOTAL_PUSH_VALUE_TYPES) {
		//Si se sale de rango, devolver default
		return push_value_types_strings[0];
	}

	else return push_value_types_strings[tipo];

}


z80_byte push_valor_extended_stack(z80_int valor,z80_byte tipo)
{

	//printf ("Putting in stack value: %04XH type: %d (%s) SP=%04XH\n",valor,tipo,extended_stack_get_string_type(tipo),reg_sp);

	extended_stack_array_items[reg_sp-1].valor=value_16_to_8h(valor);
	extended_stack_array_items[reg_sp-1].tipo=tipo;

	extended_stack_array_items[reg_sp-2].valor=value_16_to_8l(valor);
	extended_stack_array_items[reg_sp-2].tipo=tipo;


	debug_nested_push_valor_call_previous(extended_stack_nested_id_core,valor,tipo);

	//Para que no se queje el compilador
	return 0;

}

void set_extended_stack(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Extended stack");

	if (extended_stack_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	extended_stack_nested_id_core=debug_nested_push_valor_add(push_valor_extended_stack,"Extended stack");



	extended_stack_enabled.v=1;


}

void reset_extended_stack(void)
{
  debug_printf(VERBOSE_INFO,"Disabling Extended stack");
	if (extended_stack_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_push_valor_del(extended_stack_nested_id_core);
	extended_stack_enabled.v=0;


}

// Fin codigo para extended stack




//IMPORTANTE: Aqui se define el tamaño del los registros en binario en la estructura
//Si se modifica dicho tamaño, actualizar este valor

#define CPU_HISTORY_REGISTERS_SIZE 32

//Dado un puntero z80_byte, con contenido de registros en binario, retorna valores registros
//Registros 16 bits guardados en little endian
void cpu_history_regs_bin_to_string(z80_byte *p,char *destino)
{

	//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  sprintf (destino,"PC=%02x%02x SP=%02x%02x AF=%02x%02x BC=%02x%02x HL=%02x%02x DE=%02x%02x IX=%02x%02x IY=%02x%02x "
  				   "AF'=%02x%02x BC'=%02x%02x HL'=%02x%02x DE'=%02x%02x "
				   "I=%02x R=%02x IM%d IFF%c%c (PC)=%02x%02x%02x%02x",
  p[1],p[0], 	//pc
  p[3],p[2], 	//sp
  p[5],p[4], 	//af
  p[7],p[6], 	//bc
  p[9],p[8], 	//hl
  p[11],p[10], 	//de
  p[13],p[12], 	//ix
  p[15],p[14], 	//iy
  p[17],p[16], 	//af'
  p[19],p[18], 	//bc'
  p[21],p[20], 	//hl'
  p[23],p[22], 	//de'
  p[24], 		//I
  p[25], 		//R
  p[26], 		//IM
  DEBUG_STRING_IFF12_PARAM(p[27]),  //IFF1,2
  //contenido (pc) 4 bytes
  p[28],p[29],p[30],p[31]
  );
}

//Dado un puntero z80_byte, con contenido de registros en binario, retorna valor registro PC
//Registros 16 bits guardados en little endian
void cpu_history_reg_pc_bin_to_string(z80_byte *p,char *destino)
{

//Nota: funcion print_registers escribe antes BC que AF. Aqui ponemos AF antes, que es mas lógico
  sprintf (destino,"%02x%02x",
  p[1],p[0] 	//pc
  );
}


//Guarda en puntero z80_byte en con contenido de registros en binario
//Registros 16 bits guardados en little endian
void cpu_history_regs_to_bin(z80_byte *p)
{

	p[0]=value_16_to_8l(reg_pc);
	p[1]=value_16_to_8h(reg_pc);

	p[2]=value_16_to_8l(reg_sp);
	p[3]=value_16_to_8h(reg_sp);

	p[4]=Z80_FLAGS;
	p[5]=reg_a;	

	p[6]=reg_c;
	p[7]=reg_b;

	p[8]=reg_l;
	p[9]=reg_h;

	p[10]=reg_e;
	p[11]=reg_d;

	p[12]=value_16_to_8l(reg_ix);
	p[13]=value_16_to_8h(reg_ix);

	p[14]=value_16_to_8l(reg_iy);
	p[15]=value_16_to_8h(reg_iy);

	p[16]=Z80_FLAGS_SHADOW;
	p[17]=reg_a_shadow;	

	p[18]=reg_c_shadow;
	p[19]=reg_b_shadow;

	p[20]=reg_l_shadow;
	p[21]=reg_h_shadow;

	p[22]=reg_e_shadow;
	p[23]=reg_d_shadow;

	p[24]=reg_i;
  	p[25]=(reg_r&127)|(reg_r_bit7&128);
  	p[26]=im_mode;
	p[27]=iff1.v | (iff2.v<<1);

    p[28]=peek_byte_no_time(reg_pc);
    p[29]=peek_byte_no_time(reg_pc+1);
    p[30]=peek_byte_no_time(reg_pc+2);
    p[31]=peek_byte_no_time(reg_pc+3);

 
}



int cpu_history_max_elements=1000000; //1 millon
//multiplicado por 28 bytes, esto da que ocupa aproximadamente 28 MB por defecto

/*
Historial se guarda como un ring buffer
Tenemos indice que apunta a primer elemento en el ring. Esta inicializado a 0
Tenemos contador de total elementos en el ring. Inicializado a 0
Tenemos indice de siguiente posicion a insertar. Inicializado a 0
*/ 

int cpu_history_primer_elemento=0;
int cpu_history_total_elementos=0;
int cpu_history_siguiente_posicion=0;

z80_byte *cpu_history_memory_buffer=NULL;

z80_bit cpu_history_started={0};

int cpu_history_increment_pointer(int indice)
{
	//Si va mas alla del final, retornar 0
	indice++;

	if (indice>=cpu_history_max_elements) indice=0;
	return indice;
}

void cpu_history_init_buffer(void)
{
	cpu_history_primer_elemento=0;
	cpu_history_total_elementos=0;
	cpu_history_siguiente_posicion=0;


	if (cpu_history_memory_buffer!=NULL) {
		free(cpu_history_memory_buffer);
		//TODO: liberar buffer al inicializar cpu_core en set_machine
	}


	cpu_history_memory_buffer=malloc(cpu_history_max_elements*CPU_HISTORY_REGISTERS_SIZE);
	if (cpu_history_memory_buffer==NULL) cpu_panic("Can not allocate memory for cpu history");

}

long int cpu_history_get_offset_index(int indice)
{
	return indice*CPU_HISTORY_REGISTERS_SIZE;
}

//int temp_conta=0;

void cpu_history_add_element(void)
{

	//-Insertar elemento: Meter contenido en posicion indicada por indice de siguiente posicion. Incrementar indice y si va mas alla del final, poner a 0
	//printf ("Insertando elemento en posicion %d. Primer elemento: %d Total_elementos: %d\n",
	//		cpu_history_siguiente_posicion,cpu_history_primer_elemento,cpu_history_total_elementos);


	//Obtener posicion en memoria
	long int offset_memoria;
	offset_memoria=cpu_history_get_offset_index(cpu_history_siguiente_posicion);
	
	//printf ("Offset en memoria: %ld\n",offset_memoria);

	//Meter registros en memoria
	cpu_history_regs_to_bin(&cpu_history_memory_buffer[offset_memoria]);

	
	cpu_history_siguiente_posicion=cpu_history_increment_pointer(cpu_history_siguiente_posicion);

	//Si total elementos es menor que maximo, incrementar
	if (cpu_history_total_elementos<cpu_history_max_elements) cpu_history_total_elementos++;

	//Si total elementos es igual que maximo, no incrementar y aumentar posicion de indice del primer elemento. Si va mas alla del final, poner a 0
	else {
		cpu_history_primer_elemento=cpu_history_increment_pointer(cpu_history_primer_elemento);
	} 

	//temp_conta++;
	//if (temp_conta==100) cpu_history_started.v=0;


}

int cpu_history_get_array_pos_element(int indice)
{
	//Retorna indice a posicion de un elemento teniendo en cuenta que el primero (indice=0) sera donde apunte cpu_history_primer_elemento
	//Aplicar retorno a 0 si se "da la vuelta"

	if (cpu_history_primer_elemento+indice<cpu_history_max_elements) {
		//No da la vuelta. Retornar tal cual
		//TODO: ver si no pide por un elemento mas alla del total escritos
		return cpu_history_primer_elemento+indice;
	}
	else {
		//Ha dado la vuelta.
		int indice_final=cpu_history_primer_elemento+indice-cpu_history_max_elements;
		return indice_final;
		//Ejemplo: 
		//array de 3. primero es el 1 y pedimos el 2
		//tiene que retornar el 0:
		//1+2-3=0
		//array de 3. primero es el 2 y pedimos el 2
		//tiene que retornar el 1:
		//2+2-3=1
		//primero+indice-maximo
	}
}

void cpu_history_get_registers_element(int indice,char *string_destino)
{

	if (indice<0) {
		strcpy(string_destino,"ERROR: index out of range");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}

	int posicion=cpu_history_get_array_pos_element(indice);

	long int offset_memoria=cpu_history_get_offset_index(posicion);

	cpu_history_regs_bin_to_string(&cpu_history_memory_buffer[offset_memoria],string_destino);
}

void cpu_history_get_pc_register_element(int indice,char *string_destino)
{

	if (indice<0) {
		strcpy(string_destino,"ERROR: index can't be negative");
		return;
	}

	if (indice>=cpu_history_total_elementos) {
		sprintf(string_destino,"ERROR: index beyond total elements (%d)",cpu_history_total_elementos);
		return;
	}

	int posicion=cpu_history_get_array_pos_element(indice);

	long int offset_memoria=cpu_history_get_offset_index(posicion);

	cpu_history_reg_pc_bin_to_string(&cpu_history_memory_buffer[offset_memoria],string_destino);
}



int cpu_history_get_total_elements(void)
{

	return cpu_history_total_elementos;
}

int cpu_history_get_max_size(void)
{
	return cpu_history_max_elements;
}

int cpu_history_set_max_size(int total)
{
	if (total<1 || total>CPU_HISTORY_MAX_ALLOWED_ELEMENTS) return -1;
	else {
		cpu_history_max_elements=total;
		cpu_history_init_buffer();
		return 0;
	}
}

z80_byte cpu_core_loop_history(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{


	//hacer cosas antes...
	//printf ("running cpu history addr: %04XH\n",reg_pc);



	if (cpu_history_started.v) {

		//Prueba comparar legacy registers con nuevo
		/*
		printf ("array elemento en posicion %d. Primer elemento: %d Total_elementos: %d\n",cpu_history_siguiente_posicion,cpu_history_primer_elemento,cpu_history_total_elementos);


		char registros_string_legacgy[1024];
		print_registers(registros_string_legacgy);
		printf ("Legacy registers: %s\n",registros_string_legacgy);


		//Guardar en binario y obtener de nuevo 
		char registros_history_string[1024];
		z80_byte registers_history_binary[CPU_HISTORY_REGISTERS_SIZE];

		//Guardar en binario
		cpu_history_regs_to_bin(registers_history_binary);
		//Obtener en string
		cpu_history_regs_bin_to_string(registers_history_binary,registros_history_string);
		printf ("Newbin registers: %s\n",registros_history_string);
		*/


		
		cpu_history_add_element();

		//printf ("\n");
	}

	//Llamar a core anterior
	debug_nested_core_call_previous(cpu_history_nested_id_core);

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos

	return 0;
}



void set_cpu_core_history(void)
{
    debug_printf(VERBOSE_INFO,"Enabling Cpu history");

	if (cpu_history_enabled.v) {
		debug_printf(VERBOSE_INFO,"Already enabled");
		return;
	}

	cpu_history_init_buffer();

	cpu_history_nested_id_core=debug_nested_core_add(cpu_core_loop_history,"CPU history Core");

	cpu_history_enabled.v=1;
																

}

void reset_cpu_core_history(void)
{
	debug_printf(VERBOSE_INFO,"Disabling Cpu history");
	if (cpu_history_enabled.v==0) {
		debug_printf(VERBOSE_INFO,"Already disabled");
		return;
	}

	debug_nested_core_del(cpu_history_nested_id_core);
	cpu_history_enabled.v=0;

	/*if (cpu_history_memory_buffer!=NULL) {
		free(cpu_history_memory_buffer);
		cpu_history_memory_buffer=NULL;

		//TODO: liberar buffer al inicializar cpu_core en set_machine
	}*/

}




int debug_antes_t_estados_parcial=0;

void debug_get_t_stados_parcial_pre(void)
{
debug_antes_t_estados_parcial=t_estados;
}

void debug_get_t_stados_parcial_post(void)
{
//Incrementar variable debug_t_estados_parcial segun lo que haya incrementado
                       if (t_estados<debug_antes_t_estados_parcial) {
                               //Contador ha dado la vuelta
                               int dif_hasta_final_frame=screen_testados_total-debug_antes_t_estados_parcial;
                               debug_t_estados_parcial+=dif_hasta_final_frame+t_estados;
                       }
                       else {
                               debug_t_estados_parcial+=(t_estados-debug_antes_t_estados_parcial);
                       }
}


//Para saltar los step by step
void debug_anota_retorno_step_nmi(void)
{
	debug_core_lanzado_inter.v=1;
	debug_core_lanzado_inter_retorno_pc_nmi=reg_pc;
}

void debug_anota_retorno_step_maskable(void)
{
        debug_core_lanzado_inter.v=1;
        debug_core_lanzado_inter_retorno_pc_maskable=reg_pc;
}



/*

Pruebas stack trace

Linkar con librerias unwind

#include <unwind.h>
#include <libunwind.h>

int getFileAndLine (unw_word_t addr, char *file, size_t flen, int *line)
{
	static char buf[256];
	char *p;

	// prepare command to be executed
	// our program need to be passed after the -e parameter
	sprintf (buf, "/usr/bin/addr2line -C -e zesarux -f -i %lx", addr);

	printf("%s\n",buf);

	FILE* f = popen (buf, "r");

	if (f == NULL)
	{
		perror (buf);
		return 0;
	}

	// get function name
	fgets (buf, 256, f);

	// get file and line
	fgets (buf, 256, f);

	if (buf[0] != '?')
	{
		int l;
		char *p = buf;

		// file name is until ':'
		while (*p != ':')
		{
			p++;
		}

		*p++ = 0;
		// after file name follows line number
		strcpy (file , buf);
		sscanf (p,"%d", line);
	}
	else
	{
		strcpy (file,"unkown");
		*line = 0;
	}

	pclose(f);
}

void show_backtrace (void)
{
	char name[256];
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip, sp, offp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	while (unw_step(&cursor) > 0)
	{
		char file[256];
		int line = 0;

		name[0] = '\0';
		unw_get_proc_name(&cursor, name, 256, &offp);
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		//printf ("%s ip = %lx, sp = %lx\n", name, (long) ip, (long) sp);
		getFileAndLine((long)ip, file, 256, &line);
		printf("%s in file %s line %d\n", name, file, line);
	}
}

*/




//Funciones para gestion de listas de funciones
/*
Gestión de funciones anidadas para el core, peek, etc
Rutina común: devuelve z80, admite dir y z80
Es responsabilidad de cada rutina trap el llamar a la anterior
Funciones para agregar, quitar y llamar a anterior
Al agregar se le asigna un id, o bien la rutina que llama lo hace con un id fijo o un string identificativo
Si se agrega y no hay ninguna, se crea una de cero
Lista de anidados mediante estructura con un puntero a la siguiente

Funciones de peek y poke deben llamar a las anteriores normalmente, para hacer que se gestione la contienda o los breakpoints de debug
En caso de funciones con su propia contienda, estas no llamaran a la anterior pero entonces tampoco llamaran a debug y no funcionarán bien los breakpoints y habrá un problema... hay de esas?

Funciones de core también llamaran a las anteriores

Resumiendo: todas deberían llamar a las anteriores
*/

//Asignar memoria para elemento.
//Retorna: puntero a elemento asignado
debug_nested_function_element *debug_nested_alloc_element(void)
{
	debug_nested_function_element *puntero;
	//Asignar memoria
	puntero=malloc(sizeof(debug_nested_function_element));
        if (puntero==NULL) {
                cpu_panic ("No enough memory to create nested element");
        }

	return puntero;

}


//Llenar valores de la estructura
void debug_nested_fill(debug_nested_function_element *estructura,char *function_name, int id, debug_nested_function funcion, debug_nested_function_element *next,debug_nested_function_element *previous)
{

	if (strlen(function_name)>MAX_DEBUG_FUNCTION_NAME) {
		cpu_panic("Nested function name too large");
	}

	strcpy(estructura->function_name,function_name);
	estructura->id=id;
	estructura->funcion=funcion;
	estructura->next=next;
	estructura->previous=previous;

	debug_printf (VERBOSE_DEBUG,"Filling nested function. ID: %d Name: %s",id,function_name);
	//printf ("Filling nested function. ID: %d Name: %s\n",id,function_name);

}

//Buscar un identificador dentro de una lista
debug_nested_function_element *debug_nested_find_id(debug_nested_function_element *e,int id)
{

	if (e==NULL) {
		debug_printf (VERBOSE_DEBUG,"Pointer is NULL when calling to debug_nested_find_id");
		return NULL;
	}

	int salir=0;
	do {
		if (e->id==id) return e;

		//Hay siguiente?
		if (e->next!=NULL) e=e->next;
		else salir=1;
	} while (!salir);


	//No encontrado
	return NULL;
}

//Buscar un nombre de funcion dentro de una lista
debug_nested_function_element *debug_nested_find_function_name(debug_nested_function_element *e,char *nombre)
{
        int salir=0;
        do {
                if (!strcmp(e->function_name,nombre)) return e;

                //Hay siguiente?
                if (e->next!=NULL) e=e->next;
                else salir=1;
        } while (!salir);


        //No encontrado
        return NULL;
}


//Buscar primer identificador libre. Empezando desde 0
int debug_nested_find_free_id(debug_nested_function_element *e)
{
	int id;

	for (id=0;id<MAX_DEBUG_NESTED_ELEMENTS;id++) {
		if (debug_nested_find_id(e,id)==NULL) {
			//ID no encontrado. retornamos ese
			return id;
		}

	}

	//Si no hay ids libres, cpu_panic
	cpu_panic("Maximum nested elements reached");

	//Para que no se queje el compilador. Aqui no llega nunca
	return 0;
}

debug_nested_function_element *debug_nested_find_last(debug_nested_function_element *e)
{
	//debug_nested_function_element *last;

	//last=e;

	while (e->next!=NULL) {
		if (e->next!=NULL) e=e->next;
	}

	return e;
}


//Agregar un elemento a la lista. Retorna id
int debug_nested_add(debug_nested_function_element *e,char *function_name, debug_nested_function funcion)
{
	int id;
	debug_nested_function_element *last_element;
	debug_nested_function_element *new_element;

	//Obtener id libre
	id=debug_nested_find_free_id(e);
	//printf ("New id on add: %d\n",id);

	//Buscar ultimo elemento
	last_element=debug_nested_find_last(e);

	//Asignar uno nuevo
	new_element=debug_nested_alloc_element();

	//Indicar puntero del anterior hacia el siguiente
	last_element->next=new_element;

	//Y llenar valores del actual
	debug_nested_fill(new_element,function_name, id, funcion, NULL, last_element);

	debug_printf (VERBOSE_DEBUG,"Adding nested function id: %d name: %d",id,function_name);

	return id;
}


//Pide puntero al puntero inicial de la lista
void debug_nested_del(debug_nested_function_element **puntero,int id)
{
	debug_nested_function_element *e;

	e=*puntero;

	//Si puntero es nulo, no hacer nada
	if (e==NULL) {
		debug_printf (VERBOSE_DEBUG,"Nested pointer NULL calling to debug_nested_del. Not deleting anything");
		return;
	}

	//Primero buscar elemento
	debug_nested_function_element *borrar;

	//Elemento anterior al que buscamos
	debug_nested_function_element *anterior;
	//Elemento siguiente al que buscamos
	debug_nested_function_element *siguiente;

	borrar=debug_nested_find_id(e,id);

	//Si NULL, no encontrado
	if (borrar==NULL) {
		debug_printf (VERBOSE_DEBUG,"Nested element to delete with id %d not found",id);
		//printf ("Nested element to delete with id %d not found\n",id);
		return;
	}

	//Anterior
	anterior=borrar->previous;

	//Siguiente
	siguiente=borrar->next;

	//Si hay anterior, asignarle el siguiente
	if (anterior) {
		anterior->next=siguiente;
	}

	//Si no hay anterior, quiere decir que es el inicial. Reasignar puntero inicial
	else {
		*puntero=siguiente;
	}

	//Si hay siguiente, asignarle el anterior
	if (siguiente) {
		siguiente->previous=anterior;
	}

	//Liberar memoria
	debug_printf (VERBOSE_DEBUG,"Freeing element id %d name %s",borrar->id,borrar->function_name);
	free(borrar);
}

//Funcion generica que gestiona las llamadas a los elementos anidados
z80_byte debug_nested_generic_handler(debug_nested_function_element *e,z80_int dir,z80_byte value)
{
       //Buscar el ultimo
        debug_nested_function_element *last;

        last=debug_nested_find_last(e);

        //Y llamar a su funcion. Dado que son funciones genericas, enviar parametros sin sentido
        //if (t_estados<20) printf ("Calling last element function name: %s\n",last->function_name);
	z80_byte resultado;
        resultado=last->funcion(dir,value);

        return resultado;

}



//
//Para testeo
//
z80_byte debug_test_funcion(z80_int dir, z80_byte value)
{
	//Prueba simple
	return dir+value*2;

}

//Testeo recorrer adelante
void debug_test_needed_adelante(debug_nested_function_element *e,char *result)
{
char buffer[1024];

	//printf ("Recorriendo list adelante\n");
        int contador=0;
        do {
                sprintf (buffer,"Element: %p (%d) id: %d name: %s pointer function: %p previous: %p next: %p\n",e,contador,e->id,e->function_name,e->funcion, e->previous,e->next);
                debug_dump_nested_print(result,buffer);

                contador++;
                e=e->next;
        } while (e!=NULL);

}

void debug_test_needed_atras(debug_nested_function_element *e,int contador)
{

	printf ("Recorriendo list atras\n");
        //Buscar ultimo
        e=debug_nested_find_last(e);

        do {
                printf ("elemento: %p (%d) id: %d nombre: %s puntero_funcion: %p previous: %p next: %p\n",e,contador,e->id,e->function_name,e->funcion,
                        e->previous,e->next);
                contador--;
                e=e->previous;
        } while (e!=NULL);
}

//Prueba crear unos cuantos elementos
//Se puede llamar aqui desde donde se quiera, para testear
const int debug_test_needed_max=100;
void debug_test_nested(void)
{

	printf ("Allocating list\n");

	//Creamos el inicial
	debug_nested_function_element *lista_inicial;
	//debug_nested_function_element *e;
	lista_inicial=debug_nested_alloc_element();
	//e=lista_inicial;

	//Le metemos datos

	//Primer identificador cero
	debug_nested_fill(lista_inicial,"Pruebas",0, debug_test_funcion, NULL, NULL);

	//Asignar otros mas
	int i;
	char nombre_funcion[30];
	for (i=0;i<debug_test_needed_max-1;i++) {
		sprintf (nombre_funcion,"Pruebas%d",i);
		int nuevo_id=debug_nested_add(lista_inicial,nombre_funcion,debug_test_funcion);
		printf ("contador: %d nuevo_id: %d\n",i,nuevo_id);
	}

	//Recorrer array hacia adelante e ir mostrando
	debug_test_needed_adelante(lista_inicial,NULL);

	//Recorrer array hacia atras e ir mostrando
	debug_test_needed_atras(lista_inicial,debug_test_needed_max-1);


	//Borrar a peticion de usuario
	int elemento_a_borrar;
	int salir=0;

	do {
		printf ("Id a borrar: ");
		scanf ("%d",&elemento_a_borrar);
		if (elemento_a_borrar<0 || elemento_a_borrar>debug_test_needed_max-1) salir=1;
		else {
			debug_nested_del(&lista_inicial,elemento_a_borrar);
			debug_test_needed_adelante(lista_inicial,NULL);
		}
	} while (!salir);


	printf ("\n\nEliminando elemento con id 4\n\n");
	debug_nested_del(&lista_inicial,4);

	//Y volver a recorrer array
        //Recorrer array hacia adelante e ir mostrando
        debug_test_needed_adelante(lista_inicial,NULL);

        //Recorrer array hacia atras e ir mostrando
        debug_test_needed_atras(lista_inicial,debug_test_needed_max-1);


	//Prueba llamar a funcion
	z80_byte resultado=lista_inicial->funcion(10,2);
	printf ("Resultado funcion asignada: %d\n",resultado);

	//Ir eliminando todos los ids
	for (i=0;i<debug_test_needed_max;i++) {
		printf ("\nEliminando id %d\n",i);
		debug_nested_del(&lista_inicial,i);
		//Mostrar
		if (lista_inicial!=NULL) debug_test_needed_adelante(lista_inicial,NULL);
		else printf ("(lista vacia)\n");
	}
}


//
//Fin testeo
//


//
//Punteros de funciones nested
//
//
//Para Core
//
//puntero a cpu core normal sin lista
void (*cpu_core_loop_no_nested) (void);
//puntero a primer item en lista de funciones de core
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_core;
//
//Para peek_byte
//
//....




//
//INICIO Funciones de anidacion de core mediante listas nested
//

//Funcion que gestiona las llamadas a los cores anidados
void cpu_core_loop_nested_handler(void)
{
	debug_nested_generic_handler(nested_list_core,0,0);
}


//Agregar un core sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_core_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de core
	//if (nested_list_core==NULL) {
	if (cpu_core_loop!=cpu_core_loop_nested_handler) {

		debug_printf (VERBOSE_DEBUG,"Adding first core to nested list");

        	//Creamos el inicial
	        nested_list_core=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_core,nombre,0,funcion, NULL, NULL);

		cpu_core_loop_no_nested=cpu_core_loop;
		cpu_core_loop=cpu_core_loop_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_core,nombre,funcion);
	}

}

void debug_nested_core_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

	//Si esta a NULL, no hacer nada
	if (cpu_core_loop!=cpu_core_loop_nested_handler) {
		debug_printf (VERBOSE_DEBUG,"Core nested is not enabled. Not deleting anything");
		return;
	}

	debug_nested_del(&nested_list_core,id);

	if (nested_list_core==NULL) {
		//lista vacia. asignar core normal
		debug_printf (VERBOSE_DEBUG,"Core nested empty. Assign normal core");
		cpu_core_loop=cpu_core_loop_no_nested;
	}
}


//Llama a core anterior, llamando por numero de id
void debug_nested_core_call_previous(int id)
{

	//if (t_estados<20) printf ("Calling previous core to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al core original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_core->next==NULL) {
		//Solo un elemento. Llamar al core original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		cpu_core_loop_no_nested();
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_core,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("Core id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al core original
	                cpu_core_loop_no_nested();
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			actual->funcion(0,0); //los parametros 0,0 no se usan, se hace solo porque es una funcion generica de dos variables
		}
	}
}


//
//FIN Funciones de anidacion de core mediante listas nested
//

/*
Los siguientes 5 secciones generados mediante:
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte/g' > debug_nested_functions.c
cat template_nested_peek.tpl | sed 's/NOMBRE_FUNCION/peek_byte_no_time/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte/g' >> debug_nested_functions.c
cat template_nested_poke.tpl | sed 's/NOMBRE_FUNCION/poke_byte_no_time/g' >> debug_nested_functions.c
cat template_nested_push.tpl | sed 's/NOMBRE_FUNCION/push_valor/g' >> debug_nested_functions.c

*/

#include "debug_nested_functions.c"




//Inicializar punteros de cambio de funciones a NULL
void debug_nested_cores_pokepeek_init(void)
{
        nested_list_core=NULL;
        nested_list_poke_byte=NULL;
        nested_list_poke_byte_no_time=NULL;
        nested_list_peek_byte=NULL;
        nested_list_peek_byte_no_time=NULL;
		nested_list_push_valor=NULL;
}


void debug_dump_nested_add_string(char *string_inicial, char *string_to_add)
{
  //Agregar string_to_add a string. Suponemos que si cadena vacia, habra un 0 al inicio
  //No comprobamos overflow

  int longitud=strlen(string_inicial);

  strcpy(&string_inicial[longitud],string_to_add);
}

void debug_dump_nested_print(char *string_inicial, char *string_to_print)
{
  if (string_inicial==NULL) {
    printf ("%s",string_to_print);
  }
  else {
    debug_dump_nested_add_string(string_inicial,string_to_print);
  }
}

//Si result es NULL, lo muestra por salida standard. Sino, lo muestra por pantalla
void debug_dump_nested_functions(char *result)
{

  if (result!=NULL) {
    result[0]=0;
  }
  /*Ver en cada caso que haya algo en la lista y que ademas,
  el handler (por ejemplo, cpu_core_loop) apunte a handler nested
  Sucede que si por ejemplo activamos kartusho, y luego hacemos un smartload,
  el kartusho se desactiva, pero la lista contiene funciones nested, aunque los handler de peek y poke
  apuntan a los normales y no a kartusho (como debe ser)
  */

	if (nested_list_core!=NULL && cpu_core_loop==cpu_core_loop_nested_handler) {
		debug_dump_nested_print (result,"\nNested Core functions\n");
		debug_test_needed_adelante(nested_list_core,result);
	}

	if (nested_list_poke_byte!=NULL && poke_byte==poke_byte_nested_handler) {
		debug_dump_nested_print (result,"\nNested poke_byte functions\n");
		debug_test_needed_adelante(nested_list_poke_byte,result);
	}

	if (nested_list_poke_byte_no_time!=NULL && poke_byte_no_time==poke_byte_no_time_nested_handler) {
		debug_dump_nested_print (result,"\nNested poke_byte_no_time functions\n");
		debug_test_needed_adelante(nested_list_poke_byte_no_time,result);
	}

	if (nested_list_peek_byte!=NULL && peek_byte==peek_byte_nested_handler) {
		debug_dump_nested_print (result,"\nNested peek_byte functions\n");
		debug_test_needed_adelante(nested_list_peek_byte,result);
	}

	if (nested_list_peek_byte_no_time!=NULL && peek_byte_no_time==peek_byte_no_time_nested_handler) {
		debug_dump_nested_print (result,"\nNested peek_byte_no_time functions\n");
		debug_test_needed_adelante(nested_list_peek_byte_no_time,result);
	}

	if (nested_list_push_valor!=NULL && push_valor==push_valor_nested_handler) {
		debug_dump_nested_print (result,"\nNested push_valor functions\n");
		debug_test_needed_adelante(nested_list_push_valor,result);
	}

}


//Funcion de debug para cambiar valor registro
//Entrada: cadena de texto. Tipo DE=0234H
//Salida: 0 si ok. Diferente de 0 si error

int debug_change_register(char *texto)
{
	//Primero buscar hasta caracter =
	char texto_registro[100];
	unsigned int valor_registro;

	texto_registro[0]=0;

	int i;

	for (i=0;texto[i] && texto[i]!='=';i++) {
		texto_registro[i]=texto[i];
	}

	if (texto[i]==0) return 1; //Llegado hasta final de cadena y no hay igual
	texto_registro[i]=0;

	//Saltamos el =
	i++;

	if (texto[i]==0) return 2; //No hay nada despues del igual

	//Parsear valor
	valor_registro=parse_string_to_number(&texto[i]);

  if (CPU_IS_SCMP) {

  }

	//Cambiar registros
  //Motorola
	else if (CPU_IS_MOTOROLA) {


    if (!strcasecmp(texto_registro,"PC")) {
			m68k_set_reg(M68K_REG_PC,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D0")) {
			m68k_set_reg(M68K_REG_D0,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D1")) {
			m68k_set_reg(M68K_REG_D1,valor_registro);
			return 0;
		}

    else if (!strcasecmp(texto_registro,"D2")) {
      m68k_set_reg(M68K_REG_D2,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D3")) {
      m68k_set_reg(M68K_REG_D3,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D4")) {
      m68k_set_reg(M68K_REG_D4,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D5")) {
      m68k_set_reg(M68K_REG_D5,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D6")) {
      m68k_set_reg(M68K_REG_D6,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"D7")) {
      m68k_set_reg(M68K_REG_D7,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A0")) {
      m68k_set_reg(M68K_REG_A0,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A1")) {
      m68k_set_reg(M68K_REG_A1,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A2")) {
      m68k_set_reg(M68K_REG_A2,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A3")) {
      m68k_set_reg(M68K_REG_A3,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A4")) {
      m68k_set_reg(M68K_REG_A4,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A5")) {
      m68k_set_reg(M68K_REG_A5,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A6")) {
      m68k_set_reg(M68K_REG_A6,valor_registro);
      return 0;
    }

    else if (!strcasecmp(texto_registro,"A7")) {
      m68k_set_reg(M68K_REG_A7,valor_registro);
      return 0;
    }

//TODO el resto de registros...

	}

  //Z80
	else {
		if (!strcasecmp(texto_registro,"PC")) {
			reg_pc=valor_registro;
			return 0;
		}

		else if (!strcasecmp(texto_registro,"SP")) {
                        reg_sp=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IX")) {
                        reg_ix=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IY")) {
                        reg_iy=valor_registro;
                        return 0;
                }

    else if (!strcasecmp(texto_registro,"AF")) {
      reg_a=value_16_to_8h(valor_registro);
      Z80_FLAGS=value_16_to_8l(valor_registro);
      return 0;
    }


              


		else if (!strcasecmp(texto_registro,"BC")) {
                        reg_bc=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"DE")) {
                        reg_de=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"HL")) {
                        reg_hl=valor_registro;
                        return 0;
                }

                else if (!strcasecmp(texto_registro,"AF'")) {
                  reg_a_shadow=value_16_to_8h(valor_registro);
                  Z80_FLAGS_SHADOW=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"BC'")) {
                  reg_b_shadow=value_16_to_8h(valor_registro);
                  reg_c_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"DE'")) {
                  reg_d_shadow=value_16_to_8h(valor_registro);
                  reg_e_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }

                else if (!strcasecmp(texto_registro,"HL'")) {
                  reg_h_shadow=value_16_to_8h(valor_registro);
                  reg_l_shadow=value_16_to_8l(valor_registro);
                  return 0;
                }


      


		else if (!strcasecmp(texto_registro,"A")) {
                        reg_a=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"B")) {
                        reg_b=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"C")) {
                        reg_c=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"D")) {
                        reg_d=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"E")) {
                        reg_e=valor_registro;
                        return 0;
                }

                else if (!strcasecmp(texto_registro,"F")) {
                                    Z80_FLAGS=valor_registro;
                                    return 0;
                            }

		else if (!strcasecmp(texto_registro,"H")) {
                        reg_h=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"L")) {
                        reg_l=valor_registro;
                        return 0;
                }



else if (!strcasecmp(texto_registro,"A'")) {
                    reg_a_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"B'")) {
                    reg_b_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"C'")) {
                    reg_c_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"D'")) {
                    reg_d_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"E'")) {
                    reg_e_shadow=valor_registro;
                    return 0;
            }

            else if (!strcasecmp(texto_registro,"F'")) {
                                Z80_FLAGS_SHADOW=valor_registro;
                                return 0;
                        }

else if (!strcasecmp(texto_registro,"H'")) {
                    reg_h_shadow=valor_registro;
                    return 0;
            }

else if (!strcasecmp(texto_registro,"L'")) {
                    reg_l_shadow=valor_registro;
                    return 0;
            }





		else if (!strcasecmp(texto_registro,"I")) {
                        reg_i=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"R")) {
                        reg_r=(valor_registro&127);
			reg_r_bit7=(valor_registro&128);
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IFF1")) {
                    iff1.v=valor_registro;
                        return 0;
                }

		else if (!strcasecmp(texto_registro,"IFF2")) {
                    iff2.v=valor_registro;
                        return 0;
                }				


	}

	return 3;

}

void debug_set_breakpoint_optimized(int breakpoint_index,char *condicion)
{
	//de momento suponemos que no esta optimizado
	optimized_breakpoint_array[breakpoint_index].optimized=0;

	//Si no es Z80, no optimizar
	if (!CPU_IS_Z80) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: CPU is not Z80. Not optimized");
		return;
	}

	//Aqui asumimos los siguientes:
	//PC=VALOR
	//MWA=VALOR
	//MRA=VALOR

	//Minimo 4 caracteres
	int longitud=strlen(condicion);
	if (longitud<4) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: length<4. Not optimized");
		return;
	}

	//Copiamos los 3 primeros caracteres
	char variable[4];
	int i;
	for (i=0;i<3;i++) variable[i]=condicion[i];

	/*
	+#define OPTIMIZED_BRK_TYPE_PC 0
+#define OPTIMIZED_BRK_TYPE_MWA 1
+#define OPTIMIZED_BRK_TYPE_MRA 2
*/
	int tipo_optimizacion=OPTIMIZED_BRK_TYPE_NINGUNA;
	int posicion_igual;

	//Ver si variable de 2 o 3 caracteres
	if (variable[2]=='=') {
		posicion_igual=2;
		variable[2]=0;

		//Comparar con admitidos
		if (!strcasecmp(variable,"PC")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_PC;

	}

	else if (condicion[3]=='=') {
		//3 caracteres
		posicion_igual=3;
		variable[3]=0;

		//Comparar con admitidos
		if (!strcasecmp(variable,"MWA")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_MWA;
		if (!strcasecmp(variable,"MRA")) tipo_optimizacion=OPTIMIZED_BRK_TYPE_MRA;

	}

	else {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: not detected = on 3th or 4th position. Not optimized");
		return; //Volver sin mas, no se puede optimizar
	}

	if (tipo_optimizacion==OPTIMIZED_BRK_TYPE_NINGUNA) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: not detected known optimizable register/variable. Not optimized");
		return;
	}

	//Sabemos el tipo de optimizacion
	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Detected possible optimized type=%d",tipo_optimizacion);

	//Ver si lo que hay al otro lado es un valor y nada mas
	//Buscar si hay un espacio copiando en destino
	char valor_comparar[MAX_BREAKPOINT_CONDITION_LENGTH];

	int index_destino=0;

	for (i=posicion_igual+1;condicion[i]!=' ' && condicion[i];i++,index_destino++) {
		valor_comparar[index_destino]=condicion[i];
	}

	valor_comparar[index_destino]=0;

	//Si ha acabado con un espacio, no optimizar
	if (condicion[i]==' ') {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Space after number. Not optimized");
		return;
    }		

	//Ver si eso que hay a la derecha del igual es una variable
	//int si_cond_opcode=0;
	unsigned int valor; 

    //old parser valor=cpu_core_loop_debug_registro(valor_comparar,&si_cond_opcode);
	int final_numero;
	//printf ("Comprobar si [%s] es numero\n",valor_comparar);
	int result_is_number;
	result_is_number=exp_par_is_number(valor_comparar,&final_numero);
	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Testing expression [%s] to see if it's a single number",valor_comparar);

	if (result_is_number<=0) {
			//Resulta que es una variable, no un numero . no optimizar
			debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Value is a variable. Not optimized");
			return;
    }

	//Ver si el final del numero ya es el final de texto
	if (valor_comparar[final_numero]!=0) {
		debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: More characters left after the number. Not optimized");
		return;
	}

	//Pues tenemos que suponer que es un valor. Parsearlo y meterlo en array de optimizacion
	valor=parse_string_to_number(valor_comparar);

	optimized_breakpoint_array[breakpoint_index].optimized=1;
	optimized_breakpoint_array[breakpoint_index].operator=tipo_optimizacion;
	optimized_breakpoint_array[breakpoint_index].valor=valor;

	debug_printf(VERBOSE_DEBUG,"set_breakpoint_optimized: Set optimized breakpoint operator index %d type %d value %04XH",
				breakpoint_index,tipo_optimizacion,valor);


}



//Indice entre 0 y MAX_BREAKPOINTS_CONDITIONS-1
//Retorna 0 si ok
int debug_set_breakpoint(int breakpoint_index,char *condicion)
{

    if (breakpoint_index<0 || breakpoint_index>MAX_BREAKPOINTS_CONDITIONS-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting breakpoint");
      return 1;
    }

	
	int result=exp_par_exp_to_tokens(condicion,debug_breakpoints_conditions_array_tokens[breakpoint_index]);
	if (result<0) {
		debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo=TPT_FIN; //Inicializarlo vacio
		debug_printf (VERBOSE_ERR,"Error adding breakpoint [%s]",condicion);
		return 1;
	}

	//Ver si se puede evaluar la expresion resultante. Aqui basicamente generara error
	//cuando haya un parentesis sin cerrar
	int error_evaluate; 

	//Si no es token vacio
	if (debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo!=TPT_FIN) {
		exp_par_evaluate_token(debug_breakpoints_conditions_array_tokens[breakpoint_index],MAX_PARSER_TOKENS_NUM,&error_evaluate);
		if (error_evaluate) {
			debug_breakpoints_conditions_array_tokens[breakpoint_index][0].tipo=TPT_FIN; //Inicializarlo vacio
			debug_printf (VERBOSE_ERR,"Error adding breakpoint, can not be evaluated [%s]",condicion);
			return 1;
		}	
	}
	


  	debug_breakpoints_conditions_saltado[breakpoint_index]=0;
  	debug_breakpoints_conditions_enabled[breakpoint_index]=1;

	//Llamamos al optimizador
	debug_set_breakpoint_optimized(breakpoint_index,condicion);

	//Miramos cual es el ultimo breakpoint activo
	debug_set_last_active_breakpoint();

	return 0;

}


void debug_set_watch(int watch_index,char *condicion)
{

    if (watch_index<0 || watch_index>DEBUG_MAX_WATCHES-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting watch");
      return;
    }

	
	int result=exp_par_exp_to_tokens(condicion,debug_watches_array[watch_index]);
	if (result<0) {
		debug_watches_array[watch_index][0].tipo=TPT_FIN; //Inicializarlo vacio
		debug_printf (VERBOSE_ERR,"Error adding watch [%s]",condicion);
	}


}

//Indice entre 0 y MAX_BREAKPOINTS_CONDITIONS-1
void debug_set_breakpoint_action(int breakpoint_index,char *accion)
{

    if (breakpoint_index<0 || breakpoint_index>MAX_BREAKPOINTS_CONDITIONS-1) {
      debug_printf(VERBOSE_ERR,"Index out of range setting breakpoint action");
      return;
    }


    strcpy(debug_breakpoints_actions_array[breakpoint_index],accion);

}


//Borra todas las apariciones de un breakpoint concreto
void debug_delete_all_repeated_breakpoint(char *texto)
{

	int posicion=0;

	//char breakpoint_add[64];

	//debug_get_daad_breakpoint_string(breakpoint_add);

	do {
		//Si hay breakpoint ahi, quitarlo
		posicion=debug_find_breakpoint_activeornot(texto);
		if (posicion>=0) {
			debug_printf (VERBOSE_DEBUG,"Clearing breakpoint at index %d",posicion);
			debug_clear_breakpoint(posicion);
		}
	} while (posicion>=0);

	//Y salir
}

//Poner un breakpoint si no estaba como existente y activo y ademas activar breakpoints
//Nota: quiza tendria que haber otra funcion que detecte que existe pero si no esta activo, que solo lo active sin agregar otro repetido
void debug_add_breakpoint_ifnot_exists(char *breakpoint_add)
{
	//Si no hay breakpoint ahi, ponerlo y 
	int posicion=debug_find_breakpoint(breakpoint_add);
	if (posicion<0) {

        if (debug_breakpoints_enabled.v==0) {
                debug_breakpoints_enabled.v=1;

                breakpoints_enable();
    	}
		debug_printf (VERBOSE_DEBUG,"Putting breakpoint [%s] at next free slot",breakpoint_add);

		debug_add_breakpoint_free(breakpoint_add,""); 
	}
}


//tipo: tipo maquina: 0: spectrum. 1: zx80. 2: zx81
void debug_view_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,char **dir_tokens,
int inicio_tokens,z80_byte (*lee_byte_function)(z80_int dir), int tipo )
{

	  	z80_int dir;

  	debug_printf (VERBOSE_INFO,"Start Basic: %d. End Basic: %d",dir_inicio_linea,final_basic);

          int index_buffer;



          index_buffer=0;

          int salir=0;

  	z80_int numero_linea;

  	z80_int longitud_linea;

  	//deberia ser un byte, pero para hacer tokens de pi,rnd, inkeys en zx81, que en el array estan en posicion al final
  	z80_int byte_leido;

  	int lo_ultimo_es_un_token;


  	while (dir_inicio_linea<final_basic && salir==0) {
  		lo_ultimo_es_un_token=0;
  		dir=dir_inicio_linea;
  		//obtener numero linea. orden inverso
  		//numero_linea=(peek_byte_no_time(dir++))*256 + peek_byte_no_time(dir++);
  		numero_linea=(lee_byte_function(dir++))*256;
  		numero_linea +=lee_byte_function(dir++);

  		//escribir numero linea
  		sprintf (&results_buffer[index_buffer],"%4d",numero_linea);
  		index_buffer +=4;

  		//obtener longitud linea. orden normal. zx80 no tiene esto
  		if (tipo!=1) {

  			//longitud_linea=(peek_byte_no_time(dir++))+256*peek_byte_no_time(dir++);
  			longitud_linea=(lee_byte_function(dir++));
  			longitud_linea += 256*lee_byte_function(dir++);

  			debug_printf (VERBOSE_DEBUG,"Line length: %d",longitud_linea);

  		}

  		else longitud_linea=65535;

  		//asignamos ya siguiente direccion.
  		dir_inicio_linea=dir+longitud_linea;

  		while (longitud_linea>0) {
  			byte_leido=lee_byte_function(dir++);
  			longitud_linea--;

  			if (tipo==1 || tipo==2) {
  				//numero
  				if (byte_leido==126) byte_leido=14;

  				else if (byte_leido==118) byte_leido=13;


  				//Convertimos a ASCII
  				else {

  					if (tipo==2) {
  						if (byte_leido>=64 && byte_leido<=66) {
  							//tokens EN ZX81, 64=RND, 65=PI, 66=INKEY$

  							//para que no haga conversion de byte leido, sino token
  							byte_leido=byte_leido-64+256;
  						}
  					}



  					if (byte_leido>=128 && byte_leido<=191) {
  						//inverso
  						byte_leido-=128;
  					}



  					if (byte_leido<=63) {
  						if (tipo==2) byte_leido=da_codigo_zx81_no_artistic(byte_leido);
  						else byte_leido=da_codigo_zx80_no_artistic(byte_leido);
  					}

  					//Entre 64 y 127, es codigo desconocido
  					else if (byte_leido>=64 && byte_leido<=127) {
  						byte_leido='?';
  					}
  				}



  			}


  			if (byte_leido>=32 && byte_leido<=127) {
  				results_buffer[index_buffer++]=byte_leido;
  				lo_ultimo_es_un_token=0;
  			}

  			else if (byte_leido>=inicio_tokens) {

  				if (tipo==0 || tipo==1) {
  					//si lo de antes no es un token, meter espacio
  					if (lo_ultimo_es_un_token==0) {
  						results_buffer[index_buffer++]=' ';
  					}
  				}


  				int indice_token=byte_leido-inicio_tokens;
  				//printf ("byte_leido: %d inicio_tokens: %d indice token: %d\n",byte_leido,inicio_tokens,indice_token);
  				sprintf (&results_buffer[index_buffer],"%s ",dir_tokens[indice_token]);
  				index_buffer +=strlen(dir_tokens[indice_token])+1;
  				lo_ultimo_es_un_token=1;
  			}



  			else if (byte_leido==14) {
  				//representacion numero. saltar
  				dir +=5;
  				longitud_linea -=5;
  				lo_ultimo_es_un_token=0;
  			}


  			else if (byte_leido==13) {
  				//ignorar salto de linea excepto en zx80
  				if (tipo==1) {
  					longitud_linea=0;
  					dir_inicio_linea=dir;
  				}
  			}

  			else {
  				results_buffer[index_buffer++]='?';
  			}



  			//controlar maximo
  			//1024 bytes de margen
  			if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          	debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                  	        //forzar salir
  				longitud_linea=0;
          	                salir=1;
  	                }


  		}


                  //controlar maximo
                  //1024 bytes de margen
                  if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                          //forzar salir
                          salir=1;
                  }


  		//meter dos saltos de linea
  		results_buffer[index_buffer++]='\n';
  		results_buffer[index_buffer++]='\n';

  	}


          results_buffer[index_buffer]=0;

}


void debug_view_z88_print_token(z80_byte index,char *texto_destino)
{

	int salir=0;
	int i;

	for (i=0;!salir;i++) {
		if (z88_basic_rom_tokens[i].index==1) {
			sprintf (texto_destino,"?TOKEN%02XH?",index);
			salir=1;
		}

		if (z88_basic_rom_tokens[i].index==index) {
			strcpy (texto_destino,z88_basic_rom_tokens[i].token);
			salir=1;
		}
	}

}

void debug_view_z88_basic_from_memory(char *results_buffer,int dir_inicio_linea,int final_basic,
	z80_byte (*lee_byte_function)(z80_int dir) )
{

	  	z80_int dir;

  	debug_printf (VERBOSE_INFO,"Start Basic: %d. End Basic: %d",dir_inicio_linea,final_basic);

          int index_buffer;



          index_buffer=0;

          int salir=0;

  	z80_int numero_linea;

  	z80_byte longitud_linea;

  	//deberia ser un byte, pero para hacer tokens de pi,rnd, inkeys en zx81, que en el array estan en posicion al final
  	z80_int byte_leido;

  	//int lo_ultimo_es_un_token;


  	while (dir_inicio_linea<final_basic && salir==0) {
  		//lo_ultimo_es_un_token=0;
  		dir=dir_inicio_linea;
  		//obtener numero linea. orden inverso
  		//numero_linea=(peek_byte_no_time(dir++))*256 + peek_byte_no_time(dir++);
		longitud_linea=(lee_byte_function(dir++));
		//debug_printf (VERBOSE_DEBUG,"Line length: %d",longitud_linea);

  		numero_linea=lee_byte_function(dir++);
  		numero_linea +=(lee_byte_function(dir++))*256;

		if (numero_linea==65535) {
			salir=1;
		}

		else {

  			//escribir numero linea
  			sprintf (&results_buffer[index_buffer],"%5d ",numero_linea);
  			index_buffer +=6;

  			
	  		//asignamos ya siguiente direccion.
  			dir_inicio_linea=dir+longitud_linea;

			//descontar los 3 bytes
			longitud_linea -=3;
			dir_inicio_linea -=3;

  			while (longitud_linea>0) {
  				byte_leido=lee_byte_function(dir++);
  				longitud_linea--;

				if (byte_leido>=32 && byte_leido<=126) {
					results_buffer[index_buffer++]=byte_leido;
				}

				else if (byte_leido>=128) {
					//token
					char buffer_token[20];
					debug_view_z88_print_token(byte_leido,buffer_token);
	  				sprintf (&results_buffer[index_buffer],"%s",buffer_token);
  					index_buffer +=strlen(buffer_token);				  
  					//lo_ultimo_es_un_token=1;
  				}


	  			else if (byte_leido==13) {
  				}

  				else {
  					results_buffer[index_buffer++]='?';
  				}


	  			//controlar maximo
  				//1024 bytes de margen
  				if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          	debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                  	        //forzar salir
  					longitud_linea=0;
          	                salir=1;
  	            }


  			}


	  	}

        //controlar maximo
        //1024 bytes de margen
        if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-1024) {
                          debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first ones");
                          //forzar salir
                          salir=1;
        }


  		//meter dos saltos de linea
  		results_buffer[index_buffer++]='\n';
  		results_buffer[index_buffer++]='\n';

  	}


          results_buffer[index_buffer]=0;

}



void debug_view_basic(char *results_buffer)
{

  	char **dir_tokens;
  	int inicio_tokens;




  	int dir_inicio_linea;
  	int final_basic;

	int tipo=0; //Asumimos spectrum



  	if (MACHINE_IS_SPECTRUM) {
  		//Spectrum

  		//PROG
  		dir_inicio_linea=peek_byte_no_time(23635)+256*peek_byte_no_time(23636);

  		//VARS
  		final_basic=peek_byte_no_time(23627)+256*peek_byte_no_time(23628);

  		dir_tokens=spectrum_rom_tokens;

  		inicio_tokens=163;

  	}

  	else if (MACHINE_IS_ZX81) {
  		//ZX81
  		dir_inicio_linea=16509;

  		//D_FILE
  		final_basic=peek_byte_no_time(0x400C)+256*peek_byte_no_time(0x400D);

  		dir_tokens=zx81_rom_tokens;

  		inicio_tokens=192;

		tipo=2;
  	}

          //else if (MACHINE_IS_ZX80) {
    else  {
  		//ZX80
                  dir_inicio_linea=16424;

                  //VARS
                  final_basic=peek_byte_no_time(0x4008)+256*peek_byte_no_time(0x4009);

                  dir_tokens=zx80_rom_tokens;

                  inicio_tokens=213;

		tipo=1;
    }


	debug_view_basic_from_memory(results_buffer,dir_inicio_linea,final_basic,dir_tokens,inicio_tokens,peek_byte_no_time,tipo);

}


void debug_get_ioports(char *stats_buffer)
{

          //int index_op,
  	int index_buffer;



          //margen suficiente para que quepa una linea y un contador int de 32 bits...
          //aunque si pasa el ancho de linea, la rutina de generic_message lo troceara
          char buf_linea[64];

          index_buffer=0;

  	if (MACHINE_IS_SPECTRUM) {
  		sprintf (buf_linea,"Spectrum FE port: %02X\n",out_254_original_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		//Spectra
  		if (spectra_enabled.v) {
  			sprintf (buf_linea,"Spectra video mode register: %02X\n",spectra_display_mode_register);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		//ULAplus
  		if (ulaplus_enabled.v) {
  			sprintf (buf_linea,"ULAplus video mode register: %02X\n",ulaplus_mode);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  			sprintf (buf_linea,"ULAplus extended video mode register: %02X\n",ulaplus_extended_mode);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		//Timex Video
  		if (timex_video_emulation.v) {
  			sprintf (buf_linea,"Timex FF port: %02X\n",timex_port_ff);
                          sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                  }


  	}

  	if (MACHINE_IS_TIMEX_TS2068 || MACHINE_IS_CHLOE_280SE || MACHINE_IS_PRISM) {
  		sprintf (buf_linea,"Timex F4 port: %02X\n",timex_port_f4);
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
          }

  	if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || MACHINE_IS_CHLOE || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_128_P2 || TBBLUE_MACHINE_P2A) {

		//En el caso de zxuno, no mostrar si paginacion desactivada por DI7FFD
		int mostrar=1;

		if (MACHINE_IS_ZXUNO && zxuno_get_devcontrol_di7ffd()) mostrar=0;

		if (mostrar) {
            sprintf (buf_linea,"Spectrum 7FFD port: %02X\n",puerto_32765);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
		}
    }

  	if (MACHINE_IS_SPECTRUM_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED || MACHINE_IS_PRISM || superupgrade_enabled.v || MACHINE_IS_CHROME || TBBLUE_MACHINE_P2A) {
		//En el caso de zxuno, no mostrar si paginacion desactivada por DI1FFD
		int mostrar=1;

		if (MACHINE_IS_ZXUNO && zxuno_get_devcontrol_di1ffd()) mostrar=0;

		if (mostrar) {
  			sprintf (buf_linea,"Spectrum 1FFD port: %02X\n",puerto_8189);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
		}
  	}

	if (diviface_enabled.v) {
  		sprintf (buf_linea,"Diviface control port: %02X\n",diviface_control_register);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);		
	}

    if (superupgrade_enabled.v) {
      sprintf (buf_linea,"Superupgrade 43B port: %02X\n",superupgrade_puerto_43b);
      sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
    }

  	if (MACHINE_IS_TBBLUE) {
		sprintf (buf_linea,"\nTBBlue port 123b: %02X\n",tbblue_port_123b);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);  

  								sprintf (buf_linea,"\nTBBlue last register: %02X\n",tbblue_last_register);
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


  								sprintf (buf_linea,"TBBlue Registers:\n");
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								int index_ioport;
  								for (index_ioport=0;index_ioport<99;index_ioport++) {
  									//sprintf (buf_linea,"%02X : %02X \n",index_ioport,tbblue_registers[index_ioport]);
  									sprintf (buf_linea,"%02X : %02X \n",index_ioport,tbblue_get_value_port_register(index_ioport) );
  									sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  									index_buffer +=strlen(buf_linea);
  								}
  	}

	if (MACHINE_IS_TSCONF) {

  								sprintf (buf_linea,"TSConf Registers:\n");
  								sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  								int index_ioport;
  								for (index_ioport=0;index_ioport<256;index_ioport++) {
  									sprintf (buf_linea,"%02X : %02X \n",index_ioport,tsconf_af_ports[index_ioport] );
  									sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  									index_buffer +=strlen(buf_linea);
  								}
  	}

  	//Registros ULA2 de Prism, paginacion, etc
  	if (MACHINE_IS_PRISM) {

  		sprintf (buf_linea,"\nPrism EE3B port: %02X\n",prism_rom_page);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"ULA2:\n");
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

                  int i;
                  for (i=0;i<16;i++) {
                          sprintf (buf_linea,"%02X:  %02X\n",i,prism_ula2_registers[i]);
                          sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
                  }


          }



  	//Esto no en Z88
  	if (ay_chip_present.v && (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081)) {
  		int chips=ay_retorna_numero_chips();
  		int j;
  		for (j=0;j<chips;j++) {
  			sprintf (buf_linea,"\nAY-3-8912 chip %d:\n",j);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);


                  	int i;
  	                for (i=0;i<16;i++) {
          	                sprintf (buf_linea,"%02X:  %02X\n",i,ay_3_8912_registros[j][i]);
                  	        sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	                }

  		}


  	}

  	if (MACHINE_IS_Z88) {
  		sprintf (buf_linea,"Z88 Blink:\n\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"SBR:  %04X\n",blink_sbr);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		int i;
  		for (i=0;i<4;i++) {
  			sprintf (buf_linea,"PB%d:  %04X\n",i,blink_pixel_base[i]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}

  		for (i=0;i<5;i++) {
  			sprintf (buf_linea,"TIM%d: %04X\n",i,blink_tim[i]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  		}



  		sprintf (buf_linea,"COM:  %02X\n",blink_com);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"INT:  %02X\n",blink_int);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"STA:  %02X\n",blink_sta);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"EPR:  %02X\n",blink_epr);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"TMK:  %02X\n",blink_tmk);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"TSTA: %02X\n",blink_tsta);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

  	if (MACHINE_IS_ZXUNO) {
                  sprintf (buf_linea,"\nZX-Uno FC3B port: %02X\n",last_port_FC3B);
                  sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"ZX-Uno Registers:\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		int index_ioport;
  		for (index_ioport=0;index_ioport<256;index_ioport++) {
  			sprintf (buf_linea,"%02X : %02X \n",index_ioport,zxuno_ports[index_ioport]);
  			sprintf (&stats_buffer[index_buffer],"%s",buf_linea);
  			index_buffer +=strlen(buf_linea);
  		}

		//Registros DMA
  		sprintf (buf_linea,"\nZX-Uno DMA Registers:\n");
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);	

  		sprintf (buf_linea,"DMASRC:  %02X%02X\n",zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMADST:  %02X%02X\n",zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMAPRE:  %02X%02X\n",zxuno_dmareg[2][1],zxuno_dmareg[2][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMALEN:  %02X%02X\n",zxuno_dmareg[3][1],zxuno_dmareg[3][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);

  		sprintf (buf_linea,"DMAPROB: %02X%02X\n",zxuno_dmareg[4][1],zxuno_dmareg[4][0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);	

  		sprintf (buf_linea,"DMACTRL: %02X\n",zxuno_ports[0xa0]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);		  	  		  		  

  		sprintf (buf_linea,"DMASTAT: %02X\n",zxuno_ports[0xa6]);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);			  

  	}

  	if (MACHINE_IS_ZX8081) {
  		sprintf (buf_linea,"ZX80/81 last out port value: %02X\n",zx8081_last_port_write_value);
  		sprintf (&stats_buffer[index_buffer],"%s",buf_linea); index_buffer +=strlen(buf_linea);
  	}

          stats_buffer[index_buffer]=0;

}

int debug_if_breakpoint_action_menu(int index)
{

 //Si indice -1 quiza ha saltado por un membreakpoint
 if (index==-1) return 1;

  //Si accion nula o menu o break
  if (debug_breakpoints_actions_array[index][0]==0 ||
    !strcmp(debug_breakpoints_actions_array[index],"menu") ||
    !strcmp(debug_breakpoints_actions_array[index],"break")
  )  return 1;

  return 0;
}


//Parseo de parametros de comando.
#define ACTION_MAX_PARAMETERS_COMMAND 10
//array de punteros a comando y sus argumentos
char *breakpoint_action_command_argv[ACTION_MAX_PARAMETERS_COMMAND];
int breakpoint_action_command_argc;

//Separar comando con codigos 0 y rellenar array de parametros
void breakpoint_action_parse_commands_argvc(char *texto)
{
  breakpoint_action_command_argc=util_parse_commands_argvc(texto, breakpoint_action_command_argv, ACTION_MAX_PARAMETERS_COMMAND);

}



void debug_run_action_breakpoint(char *comando)
{
                                //Gestion acciones
                        debug_printf (VERBOSE_DEBUG,"Full command: %s",comando);

int i;

                                                                //Interpretar comando hasta espacio o final de linea
                                                                char comando_sin_parametros[1024];

                                                                for (i=0;comando[i] && comando[i]!=' ' && comando[i]!='\n' && comando[i]!='\r';i++) {
                                                                        comando_sin_parametros[i]=comando[i];
                                                                }

                                                                comando_sin_parametros[i]=0;

        debug_printf (VERBOSE_DEBUG,"Command without parameters: [%s]",comando_sin_parametros);


        char parametros[1024];
        parametros[0]=0;
        int pindex=0;
        if (comando[i]==' ') {
                i++;
                for (;comando[i] && comando[i]!='\n' && comando[i]!='\r';i++,pindex++) {
                        parametros[pindex]=comando[i];
                }
        }

        parametros[pindex]=0;


        debug_printf (VERBOSE_DEBUG,"Action parameters: [%s]",parametros);

        //A partir de aqui se tiene:
        //variable comando_sin_parametros: comando tal cual inicial, sin parametros
        //variable parametros: todos los comandos tal cual se han escrito, son sus espacios y todos

        //Luego los comandos que necesiten parsear parametros pueden hacer:
        //llamar a breakpoint_action_parse_commands_argvc para los comandos de 1 o mas parametros
        //comandos de 1 solo parametro pueden usar tal cual la variable parametros. Util tambien para 1 solo parametro con espacios


        //Separar parametros
	       //breakpoint_action_parse_commands_argvc(parametros);

	//debug_printf (VERBOSE_DEBUG,"Total parameters: %d",breakpoint_action_command_argc);

	//for (i=0;i<breakpoint_action_command_argc;i++) {
	//	debug_printf (VERBOSE_DEBUG,"Parameter %d : [%s]",i,breakpoint_action_command_argv[i]);
	//}

    //Gestion parametros
    if (!strcmp(comando_sin_parametros,"write")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<2) debug_printf (VERBOSE_DEBUG,"Command needs two parameters");
      else {
        unsigned int direccion;
        z80_byte valor;

        direccion=parse_string_to_number(breakpoint_action_command_argv[0]);
        valor=parse_string_to_number(breakpoint_action_command_argv[1]);

        debug_printf (VERBOSE_DEBUG,"Running write command address %d value %d",direccion,valor);

        poke_byte_z80_moto(direccion,valor);
      }
    }

    else if (!strcmp(comando_sin_parametros,"call")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int direccion;

        direccion=parse_string_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running call command address : %d",direccion);
        if (CPU_IS_MOTOROLA) debug_printf (VERBOSE_DEBUG,"Unimplemented call command for motorola");
        else {
          push_valor(reg_pc,PUSH_VALUE_TYPE_CALL);
          reg_pc=direccion;
        }
      }
    }

    else if (!strcmp(comando_sin_parametros,"printc")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        unsigned int caracter;

        caracter=parse_string_to_number(breakpoint_action_command_argv[0]);

        debug_printf (VERBOSE_DEBUG,"Running printc command character: %d",caracter);

        printf ("%c",caracter);
      }
    }

    else if (!strcmp(comando_sin_parametros,"printe")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running printe command : %s",parametros);
        //char resultado_expresion[256];
        //debug_watches_loop(parametros,resultado_expresion);
  		char salida[MAX_BREAKPOINT_CONDITION_LENGTH];
		char string_detoken[MAX_BREAKPOINT_CONDITION_LENGTH];

		exp_par_evaluate_expression(parametros,salida,string_detoken);

        printf ("%s\n",salida);
      }
    }

    else if (!strcmp(comando_sin_parametros,"prints")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running prints command : %s",parametros);
        printf ("%s\n",parametros);
      }
    }


    else if (!strcmp(comando_sin_parametros,"quicksave")) {
      debug_printf (VERBOSE_DEBUG,"Running quicksave command");
      snapshot_quick_save(NULL);
    }

    else if (!strcmp(comando_sin_parametros,"set-register")) {
      breakpoint_action_parse_commands_argvc(parametros);
      if (breakpoint_action_command_argc<1) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running set-register command : %s",breakpoint_action_command_argv[0]);
        debug_change_register(breakpoint_action_command_argv[0]);
      }
    }

    else if (!strcmp(comando_sin_parametros,"putv")) {
      if (parametros[0]==0) debug_printf (VERBOSE_DEBUG,"Command needs one parameter");
      else {
        debug_printf (VERBOSE_DEBUG,"Running putv command : %s",parametros);
		z80_byte resultado;
		resultado=exp_par_evaluate_expression_to_number(parametros);
        debug_memory_zone_debug_write_value(resultado);
      }
    }	

    else {
      debug_printf (VERBOSE_DEBUG,"Unknown command %s",comando_sin_parametros);
    }

}


//Estas funciones de debug_registers_get_mem_page_XXXX sin extended, son a borrar tambien



void debug_run_until_return_interrupt(void)
{
        //Ejecutar hasta que registro PC vuelva a su valor anterior o lleguemos a un limite
        //873600 instrucciones es 50 frames de instrucciones de 4 t-estados (69888/4*50)
        int limite_instrucciones=0;
        int salir=0;
        while (limite_instrucciones<873600 && salir==0) {
                if (reg_pc==debug_core_lanzado_inter_retorno_pc_nmi ||
                reg_pc==debug_core_lanzado_inter_retorno_pc_maskable) {
                        debug_printf (VERBOSE_DEBUG,"PC=0x%04X is now on the interrupt return address. Returning",reg_pc);
                        salir=1;
                }
                else {
                        debug_printf (VERBOSE_DEBUG,"Running and step over interrupt handler. PC=0x%04X TSTATES=%d",reg_pc,t_estados);
                        cpu_core_loop();
                        limite_instrucciones++;
                }
        }
}


//Retorna la pagina mapeada para el segmento
void debug_registers_get_mem_page_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{

        //Si es dandanator
        if (segmento==0 && dandanator_enabled.v && dandanator_switched_on.v==1) {
                sprintf (texto_pagina_short,"DB%d",dandanator_active_bank);
                sprintf (texto_pagina,"Dandanator Block %d",dandanator_active_bank);
                return;
        }

        //Si es kartusho
        if (segmento==0 && kartusho_enabled.v==1) {
                sprintf (texto_pagina_short,"KB%d",kartusho_active_bank);
                sprintf (texto_pagina,"Kartusho Block %d",kartusho_active_bank);
                return;
        }

        //Si es ifrom
        if (segmento==0 && ifrom_enabled.v==1) {
                sprintf (texto_pagina_short,"IB%d",ifrom_active_bank);
                sprintf (texto_pagina,"iFrom Block %d",ifrom_active_bank);
                return;
        }		

        //Si es betadisk
        if (segmento==0 && betadisk_enabled.v && betadisk_active.v) {
                sprintf (texto_pagina_short,"BDSK");
                sprintf (texto_pagina,"Betadisk ROM");
                return;
        }

        //Si es superupgrade
        if (superupgrade_enabled.v) {
                if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                        //ROM
                        sprintf (texto_pagina_short,"RO%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                        sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                }

                else {
                        //RAM
                        sprintf (texto_pagina_short,"RA%d",debug_paginas_memoria_mapeadas[segmento]);
                        sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
                }
                return;

        }

        //Con multiface
        if (segmento==0 && multiface_enabled.v && multiface_switched_on.v) {
                strcpy(texto_pagina_short,"MLTF");
                strcpy(texto_pagina,"Multiface ROM");
                return;
        }

        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM
                sprintf (texto_pagina_short,"RO%X",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %X",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"RA%X",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %X",debug_paginas_memoria_mapeadas[segmento]);
        }
}



//Retorna la pagina mapeada para el segmento en zxuno
void debug_registers_get_mem_page_zxuno_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"RO%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"RA%02d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %02d",debug_paginas_memoria_mapeadas[segmento]);
        }

}


//Retorna la pagina mapeada para el segmento en tbblue
void debug_registers_get_mem_page_tbblue_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}


//Retorna la pagina mapeada para el segmento en tsconf
void debug_registers_get_mem_page_tsconf_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}

//Retorna la pagina mapeada para el segmento en baseconf
void debug_registers_get_mem_page_baseconf_extended(z80_byte segmento,char *texto_pagina,char *texto_pagina_short)
{
        if (debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_ES_ROM) {
                //ROM.
                sprintf (texto_pagina_short,"O%d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
                sprintf (texto_pagina,"ROM %d",debug_paginas_memoria_mapeadas[segmento] & DEBUG_PAGINA_MAP_MASK);
        }

        else {
                //RAM
                sprintf (texto_pagina_short,"A%d",debug_paginas_memoria_mapeadas[segmento]);
                sprintf (texto_pagina,"RAM %d",debug_paginas_memoria_mapeadas[segmento]);
        }

}

//Retorna numero de segmentos en uso
int debug_get_memory_pages_extended(debug_memory_segment *segmentos)
{

	//Por si caso, inicializamos todos los strings a ""
	//debug_memory_segment segmentos[MAX_DEBUG_MEMORY_SEGMENTS];
	int i;
	for (i=0;i<MAX_DEBUG_MEMORY_SEGMENTS;i++) {
		segmentos[i].longname[0]=0;
		segmentos[i].shortname[0]=0;
	}

	//Por defecto
	int segmentos_totales=2;

	strcpy(segmentos[0].longname,"System ROM");
	strcpy(segmentos[0].shortname,"ROM");
	segmentos[0].start=0;
	segmentos[0].length=16384;

        strcpy(segmentos[1].longname,"System RAM");
        strcpy(segmentos[1].shortname,"RAM");
        segmentos[1].start=16384;
        segmentos[1].length=49152;



/*

#define MAX_DEBUG_MEMORY_SEGMENTS 8

struct s_debug_memory_segment {
        //texto largo del nombre del segmento
        char longname[100];

	//texto corto
	char shortname[32];

	//Primera direccion del segmento
	int start;

	//Longitud del segmento
	int length;


};

typedef struct s_debug_memory_segment debug_memory_segment;
*/



   //Paginas memoria
      if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 ||  superupgrade_enabled.v || MACHINE_IS_CHROME) {
		segmentos_totales=4;
                                  int pagina;

        for (pagina=0;pagina<4;pagina++) {

                           debug_registers_get_mem_page_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
				segmentos[pagina].length=16384;
				segmentos[pagina].start=16384*pagina;

          }

    }



	if (MACHINE_IS_TBBLUE) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=8;

                            for (pagina=0;pagina<8;pagina++) {

                                                            //Caso tbblue y modo config en pagina 0
                                                            if (pagina==0 || pagina==1) {
                                                                    //z80_byte maquina=(tbblue_config1>>6)&3;
                                                                    z80_byte maquina=(tbblue_registers[3])&3;
                                                                    if (maquina==0){
                                                                            if (tbblue_bootrom.v) {
											strcpy (segmentos[pagina].shortname,"RO");
											strcpy (segmentos[pagina].longname,"ROM");
										}
                                                                            else {
                                                                                    z80_byte romram_page=(tbblue_registers[4]&31);
                                                                                    sprintf (segmentos[pagina].shortname,"SR%d",romram_page);
                                                                                    sprintf (segmentos[pagina].longname,"SRAM %d",romram_page);
                                                                            }
                                                                    }
                                                                    else debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                                                            }
                                                            else {
                                                                    debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                                                            }

				segmentos[pagina].length=8192;
                                segmentos[pagina].start=8192*pagina;
                            }


                      //D5


      }


                 //Si dandanator y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && dandanator_enabled.v && dandanator_switched_on.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si kartusho y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && kartusho_enabled.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si ifrom y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && ifrom_enabled.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }						

                        //Si betadisk y maquina 48kb
                        if (MACHINE_IS_SPECTRUM_16_48 && betadisk_enabled.v && betadisk_active.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }

                        //Si multiface y maquina 48kb. TODO. Si esta dandanator y tambien multiface, muestra siempre dandanator
                        if (MACHINE_IS_SPECTRUM_16_48 && multiface_enabled.v && multiface_switched_on.v) {
                                debug_registers_get_mem_page_extended(0,segmentos[0].longname,segmentos[0].shortname);
                        }



/*
   if (tbblue_bootrom.v) {
											strcpy (segmentos[pagina].shortname,"RO");
											strcpy (segmentos[pagina].longname,"ROM");
										}
                                                                            else {
                                                                                    z80_byte romram_page=(tbblue_registers[4]&31);
                                                                                    sprintf (segmentos[pagina].shortname,"SR%d",romram_page);
                                                                                    sprintf (segmentos[pagina].longname,"SRAM %d",romram_page);
                                                                            }
                                                                    }
                                                                    else debug_registers_get_mem_page_tbblue_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
*/


//Paginas memoria
                          if (MACHINE_IS_ZXUNO && !zxuno_is_chloe_mmu() ) {
                                  int pagina;
                                  //4 paginas, texto 6 caracteres max
                                  //char texto_paginas[4][7];
				segmentos_totales=4;

                                  for (pagina=0;pagina<4;pagina++) {
                                          debug_registers_get_mem_page_zxuno_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);

				segmentos[pagina].length=16384;
                                segmentos[pagina].start=16384*pagina;
                                  }
				if (ZXUNO_BOOTM_ENABLED) {
					sprintf (segmentos[0].shortname,"%s","RO");
					sprintf (segmentos[0].longname,"%s","Boot ROM");
				}


                          }


  			//BANK PAGES
  			if (MACHINE_IS_Z88) {
				int pagina;
				segmentos_totales=4;
				for (pagina=0;pagina<4;pagina++) {
	  				sprintf (segmentos[pagina].shortname,"BANK%02X",blink_mapped_memory_banks[pagina]);
	  				sprintf (segmentos[pagina].longname,"BANK %02X",blink_mapped_memory_banks[pagina]);
	                                segmentos[pagina].length=16384;
	                                segmentos[pagina].start=16384*pagina;
				}
  			}



  			//Paginas RAM en CHLOE
  			if (MACHINE_IS_CHLOE || is_zxuno_chloe_mmu() ) {
  				//char texto_paginas[8][3];
  				//char tipo_memoria[3];
  				int pagina;
				segmentos_totales=8;
  				for (pagina=0;pagina<8;pagina++) {
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_ROM)  {
						sprintf (segmentos[pagina].shortname,"R%d",debug_chloe_paginas_memoria_mapeadas[pagina]);
						sprintf (segmentos[pagina].longname,"ROM %d",debug_chloe_paginas_memoria_mapeadas[pagina]);
					}

  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_HOME) {
						sprintf (segmentos[pagina].shortname,"H%d",debug_chloe_paginas_memoria_mapeadas[pagina]);
						sprintf (segmentos[pagina].longname,"HOME %d",debug_chloe_paginas_memoria_mapeadas[pagina]);
					}
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_DOCK) {
						sprintf (segmentos[pagina].shortname,"%s","DO");
						sprintf (segmentos[pagina].longname,"%s","DOCK");
					}
  					if (chloe_type_memory_paged[pagina]==CHLOE_MEMORY_TYPE_EX)   {
						sprintf (segmentos[pagina].shortname,"%s","EX");
						sprintf (segmentos[pagina].longname,"%s","EX");
					}

        	                        segmentos[pagina].length=8192;
	                                segmentos[pagina].start=8192*pagina;
  				}

  			}




  			if (MACHINE_IS_PRISM) {
  				segmentos_totales=8;        
  				//Si modo ram en rom
          			if (puerto_8189 & 1) {



  		                  //Paginas RAM en PRISM
                                  //char texto_paginas[8][4];
                                  		//char tipo_memoria[3];
  		                         int pagina;
  		                        

  				   for (pagina=0;pagina<8;pagina++) {
                                                  sprintf (segmentos[pagina].shortname,"A%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"RAM %02d",debug_prism_paginas_memoria_mapeadas[pagina]);

                                                  segmentos[pagina].length=8192;
	                               		  segmentos[pagina].start=8192*pagina;
  				  }

                                  


  				}



  			//Informacion VRAM en PRISM
  			  else {
  				//char texto_vram[32];




  				//Paginas RAM en PRISM
                                  //char texto_paginas[8][4];
                                  //char tipo_memoria[3];
                                  int pagina;
  				//TODO. como mostrar texto reducido aqui para paginas 2 y 3 segun vram aperture/no aperture??
                                  for (pagina=0;pagina<8;pagina++) {
                                          if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_ROM)  {
                                          		sprintf (segmentos[pagina].shortname,"O%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                          		sprintf (segmentos[pagina].longname,"ROM %02d",debug_prism_paginas_memoria_mapeadas[pagina]);
                                          }

                                          if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_HOME) {
  						sprintf (segmentos[pagina].shortname,"H%02d",debug_prism_paginas_memoria_mapeadas[pagina]);
  						sprintf (segmentos[pagina].longname,"HOME %02d",debug_prism_paginas_memoria_mapeadas[pagina]);
  						if (pagina==2 || pagina==3) {
  							//La info de segmentos 2 y 3 (vram aperture si/no) se muestra de info anterior
  							sprintf (segmentos[pagina].shortname,"VRA");
  							sprintf (segmentos[pagina].longname,"VRAM");
  						}
  					}
            				if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_DOCK) {
            					sprintf (segmentos[pagina].shortname,"%s","DO");
            					sprintf (segmentos[pagina].longname,"%s","DOCK");
            				}

            				if (prism_type_memory_paged[pagina]==PRISM_MEMORY_TYPE_EX)   {
            					sprintf (segmentos[pagina].shortname,"%s","EX");
            					sprintf (segmentos[pagina].longname,"%s","EX");
            				}

  					//Si pagina rom failsafe
  					if (prism_failsafe_mode.v) {
  						if (pagina==0 || pagina==1) {
  							sprintf (segmentos[pagina].shortname,"%s","FS");
  							sprintf (segmentos[pagina].longname,"%s","Failsafe ROM");
  						}
  					}


  					//Si paginando zona alta c000h con paginas 10,11 (que realmente son vram0 y 1) o paginas 14,15 (que realmente son vram 2 y 3)
  					if (pagina==6 || pagina==7) {
  						int pagina_mapeada=debug_prism_paginas_memoria_mapeadas[pagina];
  						int vram_pagina=-1;
  						switch (pagina_mapeada) {
  							case 10:
  								vram_pagina=0;
  							break;

  							case 11:
  								vram_pagina=1;
  							break;

  							case 14:
  								vram_pagina=2;
  							break;

  							case 15:
  								vram_pagina=3;
  							break;
  						}

  						if (vram_pagina!=-1) {
  							sprintf (segmentos[pagina].shortname,"V%d",vram_pagina);
  							sprintf (segmentos[pagina].longname,"VRAM %d",vram_pagina);
  						}
  					}

  					 segmentos[pagina].length=8192;
	                               	 segmentos[pagina].start=8192*pagina;
                                          //sprintf (texto_paginas[pagina],"%c%d",tipo_memoria,debug_prism_paginas_memoria_mapeadas[pagina]);
                                  }

                              


          }
  			}

  			  //Paginas RAM en TIMEX
                          if (MACHINE_IS_TIMEX_TS2068) {
                          		segmentos_totales=8;        
                                  //char texto_paginas[8][3];
                                  //char tipo_memoria;
                                  int pagina;
                                  for (pagina=0;pagina<8;pagina++) {
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_ROM)  {
                                          	sprintf (segmentos[pagina].shortname,"%s","RO");
                                          	sprintf (segmentos[pagina].longname,"%s","ROM");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_HOME) {
                                          	sprintf (segmentos[pagina].shortname,"%s","HO");
                                          	sprintf (segmentos[pagina].longname,"%s","HOME");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_DOCK) {
                                          	sprintf (segmentos[pagina].shortname,"%s","DO");
                                          	sprintf (segmentos[pagina].longname,"%s","DOCK");
                                          }
                                          if (timex_type_memory_paged[pagina]==TIMEX_MEMORY_TYPE_EX)   {
                                          	sprintf (segmentos[pagina].shortname,"%s","EX");
                                          	sprintf (segmentos[pagina].longname,"%s","EX");
                                          }
                                          //sprintf (texto_paginas[pagina],"%c%d",tipo_memoria,debug_timex_paginas_memoria_mapeadas[pagina]);


  					 segmentos[pagina].length=8192;
	                               	 segmentos[pagina].start=8192*pagina;
                                  }

                           
                          }

  			//Paginas RAM en CPC
  //#define CPC_MEMORY_TYPE_ROM 0
  //#define CPC_MEMORY_TYPE_RAM 1

  //extern z80_byte debug_cpc_type_memory_paged_read[];
  //extern z80_byte debug_cpc_paginas_memoria_mapeadas_read[];
  			if (MACHINE_IS_CPC) {
                        //char texto_paginas[4][5];
                        segmentos_totales=4;
                        int pagina;
                        for (pagina=0;pagina<4;pagina++) {
                            if (debug_cpc_type_memory_paged_read[pagina]==CPC_MEMORY_TYPE_ROM) {
  								sprintf (segmentos[pagina].shortname,"ROM%d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  								sprintf (segmentos[pagina].longname,"ROM %d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  					
  					   		}

                            if (debug_cpc_type_memory_paged_read[pagina]==CPC_MEMORY_TYPE_RAM) {
  								sprintf (segmentos[pagina].shortname,"RAM%d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  								sprintf (segmentos[pagina].longname,"RAM %d",debug_cpc_paginas_memoria_mapeadas_read[pagina]);
  					  		}

							//Si es kartusho
        					if (pagina==0 && kartusho_enabled.v==1) {
                				sprintf (segmentos[pagina].shortname,"KB%d",kartusho_active_bank);
                				sprintf (segmentos[pagina].longname,"Kartusho Block %d",kartusho_active_bank);
							}


							//Si es ifrom
        					if (pagina==0 && ifrom_enabled.v==1) {
                				sprintf (segmentos[pagina].shortname,"IB%d",ifrom_active_bank);
                				sprintf (segmentos[pagina].longname,"iFrom Block %d",ifrom_active_bank);
							}							


  							segmentos[pagina].length=16384;
	                        segmentos[pagina].start=16384*pagina;

                        }

            }

  			//Paginas RAM en SAM
  			if (MACHINE_IS_SAM) {
  				//char texto_paginas[4][6];
  				segmentos_totales=4;
                                  int pagina;
                                  for (pagina=0;pagina<4;pagina++) {
                                          if (sam_memory_paged_type[pagina]==0) {
                                                  sprintf (segmentos[pagina].shortname,"RAM%02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"RAM %02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                          }

                                          if (sam_memory_paged_type[pagina]) {
                                                  sprintf (segmentos[pagina].shortname,"ROM%02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                                  sprintf (segmentos[pagina].longname,"ROM %02d",debug_sam_paginas_memoria_mapeadas[pagina]);
                                          }

                                          segmentos[pagina].length=16384;
	                               	 segmentos[pagina].start=16384*pagina;

                                  }

                                 
                          }

                          if (MACHINE_IS_QL) {
                          	segmentos_totales=3;

                          		strcpy(segmentos[0].longname,"System ROM");
					strcpy(segmentos[0].shortname,"ROM");
					segmentos[0].start=0;
					segmentos[0].length=49152;


        				strcpy(segmentos[1].longname,"I/O Space");
        				strcpy(segmentos[1].shortname,"I/O");
        				segmentos[1].start=0x18000;
        				segmentos[1].length=16384;


        				strcpy(segmentos[2].longname,"System RAM");
        				strcpy(segmentos[2].shortname,"RAM");
        				segmentos[2].start=0x20000;
        				segmentos[2].length=QL_MEM_LIMIT+1-0x20000;

                          }


                            if (MACHINE_IS_MK14) {
                          	segmentos_totales=5;

                          		strcpy(segmentos[0].longname,"System ROM");
					strcpy(segmentos[0].shortname,"ROM");
					segmentos[0].start=0;
					segmentos[0].length=512;

					strcpy(segmentos[1].longname,"Shadow ROM");
					strcpy(segmentos[1].shortname,"SROM");
					segmentos[1].start=0x200;
					segmentos[1].length=512*3;


        				strcpy(segmentos[2].longname,"I/O Space");
        				strcpy(segmentos[2].shortname,"I/O");
        				segmentos[2].start=0x800;
        				segmentos[2].length=512;


        				strcpy(segmentos[3].longname,"Extended RAM");
        				strcpy(segmentos[3].shortname,"ERAM");
        				segmentos[3].start=0xb00;
        				segmentos[3].length=256;

        				strcpy(segmentos[4].longname,"Standard RAM");
        				strcpy(segmentos[4].shortname,"RAM");
        				segmentos[4].start=0xf00;
        				segmentos[4].length=256;

                          }



                          if (MACHINE_IS_TSCONF) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=4;

                            	for (pagina=0;pagina<4;pagina++) {

                                   debug_registers_get_mem_page_tsconf_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                              

					segmentos[pagina].length=16384;
                                	segmentos[pagina].start=16384*pagina;
                           	 }


      			}

          if (MACHINE_IS_BASECONF) {
                                                      int pagina;
                                                      //4 paginas, texto 5 caracteres max
				segmentos_totales=4;

                            	for (pagina=0;pagina<4;pagina++) {

                                   debug_registers_get_mem_page_baseconf_extended(pagina,segmentos[pagina].longname,segmentos[pagina].shortname);
                              

					segmentos[pagina].length=16384;
                                	segmentos[pagina].start=16384*pagina;
                           	 }


      			}


  			//Fin paginas ram


      	//Caso divmmc
      			

      	if (diviface_enabled.v) {
      		if ( !   ( (diviface_control_register&128)==0 && diviface_paginacion_automatica_activa.v==0) )  {


			//Caso tbblue
			int div_segment_zero=1;
			int div_segment_one=1;

			if (MACHINE_IS_TBBLUE) {
				if (!(debug_paginas_memoria_mapeadas[0] & DEBUG_PAGINA_MAP_ES_ROM)) div_segment_zero=0;
				if (!(debug_paginas_memoria_mapeadas[1] & DEBUG_PAGINA_MAP_ES_ROM)) div_segment_one=0;
			}

			if (div_segment_zero) {

	      			strcpy(segmentos[0].longname,"Diviface");
				strcpy(segmentos[0].shortname,"DIV");	

			}

			if (div_segment_one) {

				//En maquinas de 8 segmentos, bloque 1 es 8192-16383
				if (segmentos_totales==8) {
				      	strcpy(segmentos[1].longname,"Diviface");
					strcpy(segmentos[1].shortname,"DIV");		
				}

			}
      		}
      	}


	return segmentos_totales;

}


//Devuelve texto estado pagina video(5/7) y si paginacion esta activa o no. Solo para maquinas spectrum y que no sean 16/48
void debug_get_paging_screen_state(char *s)
{

	//por defecto
	*s=0;

	if (!MACHINE_IS_SPECTRUM) return;

	if (MACHINE_IS_SPECTRUM_16_48) return;


	sprintf (s,"SCR%d %s", ( (puerto_32765&8) ? 7 : 5) ,  ( (puerto_32765&32) ? "PDI" : "PEN"  ) );


}



int si_cpu_step_over_jpret(void)
{
        if (CPU_IS_MOTOROLA || CPU_IS_SCMP) return 0;
        z80_byte opcode=peek_byte_no_time(reg_pc);

	debug_printf(VERBOSE_DEBUG,"cpu step over, first opcode at %04XH is %02XH",reg_pc,opcode);

        switch (opcode)
        {

                case 0xC3: // JP
                case 0xCA: // JP Z
                case 0xD2: // JP NC
                case 0xDA: // JP C
                case 0xE2: // JP PO
                case 0xE9: // JP (HL)
                case 0xEA: // JP PE
                case 0xF2: // JP P
                case 0xFA: // JP M

                case 0xC0: // RET NZ
                case 0xC8: // RET Z
                case 0xC9: // RET
                case 0xD0: // RET NC
                case 0xD8: // RET C
                case 0xE0: // RET PO
                case 0xE8: // RET PE
                case 0xF0: // RET P
                case 0xF8: // RET M

                        return 1;
                break;
        }

        return 0;

}


void debug_cpu_step_over(void)
{
  unsigned int direccion=get_pc_register();
  int longitud_opcode=debug_get_opcode_length(direccion);

  unsigned int direccion_final=direccion+longitud_opcode;
  direccion_final=adjust_address_space_cpu(direccion_final);


  //Parar hasta volver de la instruccion actual o cuando se produzca algun evento de apertura de menu, como un breakpoint
  menu_abierto=0;
  int salir=0;
  while (get_pc_register()!=direccion_final && !salir) {
    debug_core_lanzado_inter.v=0;
    cpu_core_loop();

    if (debug_core_lanzado_inter.v && (remote_debug_settings&32)) {
        debug_run_until_return_interrupt();
    }

    if (menu_abierto) salir=1;
  }
}



int debug_get_opcode_length(unsigned int direccion)
{
  char buffer_retorno[101];
  size_t longitud_opcode;

  debugger_disassemble(buffer_retorno,100,&longitud_opcode,direccion);

  return longitud_opcode;

}

//Retorna si el texto indicado es de tipo PC=XXXX
//Retorna 0 si no
//Retorna 1 si es
int debug_text_is_pc_condition(char *cond)
{
	if (cond[0]=='P' || cond[0]=='p') {
				if (cond[1]=='C' || cond[1]=='c') {
					if (cond[2]=='=') {
						//Ahora a partir de aqui ver que no haya ningun espacio
						int j;

						for (j=3;cond[j];j++) {
							if (cond[j]==' ') return 0;
						}
						return 1;
					}
				}
	}
	return 0;
}

//Retorna si el breakpoint indicado es de tipo PC=XXXX y action=""
//Retorna 0 si no
//Retorna 1 si es
int debug_return_brk_pc_condition(int indice)
{
	if (debug_breakpoints_enabled.v==0) return -1;

	char *cond;

	int i=indice;
	
		if (debug_breakpoints_conditions_enabled[i]) {
			if (debug_breakpoints_actions_array[i][0]!=0) return 0;

			
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			cond=buffer_temp;


			return debug_text_is_pc_condition(cond);
		}
	

	return 0;
}

//Retorna si hay breakpoint tipo PC=XXXX donde XXXX coincide con direccion y action=""
//Teniendo en cuenta que breakpoints tiene que estar enable, y ese breakpoint tambien tiene que estar activado
//Retorna -1 si no
//Retorna indice a breakpoint si coincide
int debug_return_brk_pc_dir_condition(menu_z80_moto_int direccion)
{

	if (debug_breakpoints_enabled.v==0) return -1;

	char *cond;

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
			if (debug_return_brk_pc_condition(i)) {


			//TODO: esto se podria mejorar analizando los tokens
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			cond=buffer_temp;
			

				menu_z80_moto_int valor=parse_string_to_number(&cond[3]);
				if (valor==direccion) return i;
			}
	}
						
	return -1;
}

//Retorna primera posicion en array de breakpoint libres. -1 si no hay
int debug_find_free_breakpoint(void)
{
	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
			
			if (debug_breakpoints_conditions_array_tokens[i][0].tipo==TPT_FIN) return i;
			
	}

	return -1;
}

//Retorna primera posicion en array que coindice con breakpoint y que este activado
int debug_find_breakpoint(char *to_find)
{

	if (debug_breakpoints_enabled.v==0) return -1;

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
		if (debug_breakpoints_conditions_enabled[i]) {
			
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);
			if  (!strcasecmp(buffer_temp,to_find)) return i;
			
		}
	}

	return -1;
}

//Retorna primera posicion en array que coindice con breakpoint,este activo o no
int debug_find_breakpoint_activeornot(char *to_find)
{

	int i;
	for (i=0;i<MAX_BREAKPOINTS_CONDITIONS;i++) {
			
			char buffer_temp[MAX_BREAKPOINT_CONDITION_LENGTH];
			exp_par_tokens_to_exp(debug_breakpoints_conditions_array_tokens[i],buffer_temp,MAX_PARSER_TOKENS_NUM);

			//printf ("%d temp: [%s] comp: [%s]\n",i,buffer_temp,to_find);

			if (!strcasecmp(buffer_temp,to_find)) return i;
			
	}

	return -1;
}



//Agrega un breakpoint, con action en la siguiente posicion libre. -1 si no hay
//Retorna indice posicion si hay libre

int debug_add_breakpoint_free(char *breakpoint, char *action)
{
	int posicion=debug_find_free_breakpoint();
	if (posicion<0) {
		debug_printf (VERBOSE_ERR,"No free breakpoint entry");
		return -1;
	}

	debug_set_breakpoint(posicion,breakpoint);
	debug_set_breakpoint_action(posicion,action);

	return posicion;

}

void debug_clear_breakpoint(int indice)
{
	//Elimina una linea de breakpoint. Pone condicion vacia y enabled a 0
	debug_set_breakpoint(indice,"");
	debug_set_breakpoint_action(indice,"");
	//debug_breakpoints_conditions_enabled[indice]=0;
	debug_breakpoints_conditions_disable(indice);
}

void debug_get_stack_moto(menu_z80_moto_int p,int items, char *texto)
{
	int i;
  	for (i=0;i<items;i++) {
		//menu_z80_moto_int valor=16777216*peek_byte_z80_moto(p)+65536*peek_byte_z80_moto(p+1)+256*peek_byte_z80_moto(p+2)+256*peek_byte_z80_moto(p+3);
		sprintf(&texto[i*9],"%02X%02X%02X%02X ",peek_byte_z80_moto(p),peek_byte_z80_moto(p+1),peek_byte_z80_moto(p+2),peek_byte_z80_moto(p+3) );
		p +=4;
	}
}

//Retorna valores en el stack separados por espacios
//Para Z80: retorna 16 bits
//Para motorola, scmp: no implementado aun
void debug_get_stack_values(int items, char *texto)
{

	//Por si acaso, por defecto
	texto[0]=0;

	if (CPU_IS_Z80) {
		int i;
  		for (i=0;i<items;i++) {
			z80_int valor=peek_byte_z80_moto(reg_sp+i*2)+256*peek_byte_z80_moto(reg_sp+1+i*2);
			sprintf(&texto[i*5],"%04X ",valor);
		  }
		  
	}

	if (CPU_IS_MOTOROLA) {
		//int i;
		menu_z80_moto_int p=m68k_get_reg(NULL, M68K_REG_SP);
		debug_get_stack_moto(p,items,texto);
  		/*for (i=0;i<items;i++) {
			menu_z80_moto_int valor=16777216*peek_byte_z80_moto(p)+65536*peek_byte_z80_moto(p+1)+256*peek_byte_z80_moto(p+2)+256*peek_byte_z80_moto(p+3);
			sprintf(&texto[i*9],"%08X ",valor);
			p +=4;
		}*/
	}


}

//Retornar el user stack de motorola
void debug_get_user_stack_values(int items, char *texto)
{
	
	//Por si acaso, por defecto
	texto[0]=0;	

	if (CPU_IS_MOTOROLA) {
		//int i;
		menu_z80_moto_int p=m68k_get_reg(NULL, M68K_REG_USP);
		debug_get_stack_moto(p,items,texto);
  		/*for (i=0;i<items;i++) {
			menu_z80_moto_int valor=16777216*peek_byte_z80_moto(p)+65536*peek_byte_z80_moto(p+1)+256*peek_byte_z80_moto(p+2)+256*peek_byte_z80_moto(p+3);
			sprintf(&texto[i*9],"%08X ",valor);
			p +=4;
		}*/
	}


}


void debug_get_t_estados_parcial(char *buffer_estadosparcial)
{

			int estadosparcial=debug_t_estados_parcial;

			if (estadosparcial>999999999) sprintf (buffer_estadosparcial,"%s","OVERFLOW");
			else sprintf (buffer_estadosparcial,"%09u",estadosparcial);
}

void debug_get_daad_breakpoint_string(char *texto)
{
	/*
	Retorna cadena breakpoint tipo 	PC=617D si A=188
	Debe detener justo despues del tipico LD A,(BC)

	#define DAAD_PARSER_BREAKPOINT_PC 0x617c
#define DAAD_PARSER_CONDACT_BREAKPOINT 0xbc
	*/


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND A=%d",util_daad_get_pc_parser()+1,DAAD_PARSER_CONDACT_BREAKPOINT);

}


//Retorna cadena de breakpoint de step to step para pararse en el parser de condacts, y siempre que condact no sea FFH
void debug_get_daad_step_breakpoint_string(char *texto)
{
	z80_int breakpoint_dir;

	if (util_daad_detect() ) breakpoint_dir=util_daad_get_pc_parser();
	if (util_paws_detect() ) breakpoint_dir=util_paws_get_pc_parser();	


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND PEEK(BC)<>255",breakpoint_dir);

}


//Retorna cadena de breakpoint cuando va a leer condact PARSE en daad
void debug_get_daad_runto_parse_string(char *texto)
{
	z80_int breakpoint_dir;

	if (util_daad_detect() ) breakpoint_dir=util_daad_get_pc_parser();
	if (util_paws_detect() ) breakpoint_dir=util_paws_get_pc_parser();


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND PEEK(BC)=73",breakpoint_dir);

}



z80_byte *memory_zone_debug_ptr=NULL;

int memory_zone_current_size=0;

void debug_memory_zone_debug_reset(void)
{
	memory_zone_current_size=0;
}

void debug_memory_zone_debug_write_value(z80_byte valor)
{
	if (memory_zone_debug_ptr==NULL) {
		debug_printf (VERBOSE_DEBUG,"Allocating memory for debug memory zone");
		memory_zone_debug_ptr=malloc(MEMORY_ZONE_DEBUG_MAX_SIZE);
		if (memory_zone_debug_ptr==NULL) {
			cpu_panic ("Can not allocate memory for debug memory zone");
		}
	}

	//Si aun hay espacio disponible
	if (memory_zone_current_size<MEMORY_ZONE_DEBUG_MAX_SIZE) {
		memory_zone_debug_ptr[memory_zone_current_size]=valor;
		memory_zone_current_size++;
	}
	//else {
	//	printf ("Memory zone full\n");
	//}
}


//Obtener fecha, hora , minutos y microsegundos
//Retorna longitud del texto
int debug_get_timestamp(char *destino)
{
	

	struct timeval tv;
	struct tm* ptm;
	long microseconds;


	// 2015/01/01 11:11:11.999999"
	// 12345678901234567890123456
	const int longitud_timestamp=26;

	/* Obtain the time of day, and convert it to a tm struct. */
	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
	/* Format the date and time, down to a single second. */
	char time_string[40];

	strftime (time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", ptm);

	microseconds = tv.tv_usec;
		/* Print the formatted time, in seconds, followed by a decimal point and the microseconds. */
	sprintf (destino,"%s.%06ld ", time_string, microseconds);


	return longitud_timestamp;
			 
        
}
