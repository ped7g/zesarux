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

#include "cpu.h"
#include "audio.h"
#include "debug.h"
#include "screen.h"
#include "utils.h"
#include "zx8081.h"
#include "jupiterace.h"
#include "autoselectoptions.h"
#include "operaciones.h"
#include "cpc.h"
#include "settings.h"
#include "audio_sine_table.h"
#include "ay38912.h"

#include "audionull.h"

#ifdef USE_SNDFILE
	#include <sndfile.h>
#endif

#ifdef COMPILE_ALSA
#include "audioalsa.h"
#endif

#ifdef COMPILE_COREAUDIO
#include "audiocoreaudio.h"
#endif 

//Para rutinas midi windows que hay aqui
#ifdef MINGW
#include <windows.h>   
#include <mmsystem.h>  
#endif

//Si se usa audio sdl2. para workaround con el detector de silencio, que genera zumbido con sdl2
//Esta a 0 inicialmente. Si se usa sdl2, se pondra a 1, y ya no se pondra a 0 hasta no salir del emulador
//esto solo afecta si cambiamos driver audio en caliente de sdl2 a otro, no saltara el detector de silencio
int audio_using_sdl2=0;

char *audio_buffer_one;
char *audio_buffer_two;
//char audio_buffer_oneandtwo[AUDIO_BUFFER_SIZE*2];

//con stereo, cada posicion par, contiene canal izquierdo. Posicion impar, canal derecho
char audio_buffer_one_assigned[AUDIO_BUFFER_SIZE*2];  //Doble porque es estereo
char audio_buffer_two_assigned[AUDIO_BUFFER_SIZE*2];  //Doble porque es estereo

char *audio_driver_name;

//Si el driver de audio soporta stereo. En teoria lo soportan todos, se pone de momento como transicion 
//del sistema mono a stereo, hasta que no esten todos los drivers
//z80_bit audio_driver_accepts_stereo={0};

z80_bit silence_detector_setting={0};

//Rutinas de audio
int (*audio_init) (void);
int (*audio_thread_finish) (void);
void (*audio_end) (void);
void (*audio_send_frame) (char *buffer);
void (*audio_get_buffer_info) (int *buffer_size,int *current_size);

//en porcentaje
int audiovolume=100;

//Indica si esta activo que cuando se llena una fifo de driver de sonido, no se debe reiniciar
z80_bit audio_noreset_audiobuffer_full={0};

//Frecuencia teniendo en cuenta la velocidad de la cpu. inicializado a frecuencia normal
int frecuencia_sonido_variable=FRECUENCIA_CONSTANTE_NORMAL_SONIDO;

//Detectar cuando se hace un SAVE desde rom (spectrum) y enviar el sonido solo del bit MIC del puerto FEH (y no el EAR)
//Por defecto desactivado para no interferir con otras roms que puedan generar sonido en esas direcciones
z80_bit output_beep_filter_on_rom_save={0};

z80_bit output_beep_filter_alter_volume={0};
char output_beep_filter_volume=122;


//Valor del sample de sonido que se envia cada vez. Usado en cada core_
char audio_valor_enviar_sonido;
//int audio_valor_enviar_sonido;

char audio_valor_enviar_sonido_izquierdo;
char audio_valor_enviar_sonido_derecho;


z80_byte audiodac_last_value_data;

z80_bit audiodac_enabled={0};

//z80_int audiodac_port=0xDF; //Por defecto puerto specdrum

int audiodac_selected_type=0;

/*
Para el generador de tonos, usado en testeo. Posibles valores:
0: desactivado
1: generamos valor maximo (127)
2: generamos valor minimo (-128)
3: conmutamos entre valor maximo y minimo cada 50 Hz
*/
int audio_tone_generator=0;
char audio_tone_generator_last=-127;

char audio_tone_generator_get(void)
{
	switch (audio_tone_generator) {
		case 1:
			return 127;
		break;

		case 2:
			return -128;
		break;

		case 3:
			return audio_tone_generator_last;
		break;
	}

	//no se deberia llegar aqui
	return 0;
}


audiodac_type audiodac_types[MAX_AUDIODAC_TYPES]={
	{"Specdrum",0xDF},
	{"Covox Pentagon",0xFB},
	{"Covox Scorpion",0xDD},
	{"GS Covox",0xB3},
	{"Custom",0}
};

void audiodac_print_types(void)
{
	int i;
	for (i=0;i<MAX_AUDIODAC_TYPES;i++) printf ("\"%s\" ",audiodac_types[i].name);
}

//Retorna 0 si error
int audiodac_set_type(char *texto)
{
	int i;
	for (i=0;i<MAX_AUDIODAC_TYPES;i++) {
		if (!strcasecmp(texto,audiodac_types[i].name)) {
			audiodac_selected_type=i;
			return 1;
		}
	}

	return 0;

}

void audiodac_set_custom_port(z80_byte valor)
{
	audiodac_types[MAX_AUDIODAC_TYPES-1].port=valor;
	audiodac_selected_type=MAX_AUDIODAC_TYPES-1;
}

/*
http://velesoft.speccy.cz/da_for_zx-cz.htm

STEREO COVOX PORTS
 #0F = channel 1
 #4F = channel 2
 STEREO COVOX PORTS  - SPRINTER 2000
 #FB = channel 1
 #4F = channel 2
 STEREO COVOX PORTS  - PROFI
 #3F = left channel
 #5F = right channel
 #7F = 8255 control register ( out value #80)





 COVOX PORTS

 computer

port

 Pentagon/ATM
 ZS Scorpion/ZX Profi
 ZX Profi
 GS COVOX	#FB
#DD
#BB55
#B3


Česká varianta s 8255(UR-4)

V ČR je nejrozšířenější varianta tříkanálového převodníku připojeného k interface UR-4(PIO). Všechny tři převodníky jsou připojeny přímo na výstupy bran A,B,C obvodu 8255 v UR-4. Stereo výstup je zapojený jako ACB stereo, podobně jako u Melodiku.

 PORTY
  #1F = channel A
 #3F = channel B
 #5F = channel C
 #7F = 8255 control register (out value #80)


 */




//apunta al buffer de audio activo para llenar
char *audio_buffer=NULL;
//apunta al buffer de audio activo para reproducir. se usa en audiodsp y audioalsa con phtreads
char *audio_buffer_playback;

//apunta a la posicion dentro del buffer de audio activo
int audio_buffer_indice=0;

//indica 0 o 1 segun el buffer de audio activo
z80_bit audio_buffer_switch;

//Bit a enviar al altavoz (0 o 1) para zx8081
z80_bit bit_salida_sonido_zx8081;

//Valor del ultimo altavoz para spectrum
char value_beeper=0;

//Ultimo valor (bits 3 y 4 solo significativos) del puerto 254 enviado
z80_byte ultimo_altavoz;

//Contador del modo silencio
//Se incrementa cada interrupcion 1/50
//Se pone a 0 cada vez que se envia out por el puerto 211
//Si el valor es SILENCE_DETECTION_MAX, no se envia audio
int silence_detection_counter=0;

z80_bit beeper_enabled={1};


//Similar a silence_detection_counter pero solo para operaciones del beeper
int beeper_silence_detection_counter=0;

//generada interrupcion debido a fin de sonido
z80_bit interrupt_finish_sound;

//decir que el thread de sonido debe enviar audio o no
z80_bit audio_playing;



//aofile
FILE *ptr_aofile;
char *aofilename;

z80_bit aofile_inserted;

//este valor lo alteramos al simular sonido de carga del zx8081
int amplitud_speaker_actual_zx8081=AMPLITUD_BEEPER;

char *aofile_buffer;

int aofile_type;

//Para sonido beeper mas real
//contiene valor normal (con rango de char -127...+128) o 65535 si no esta inicializado
int buffer_beeper[MAX_BEEPER_ARRAY_LENGTH];
int beeper_real_enabled=1;




#ifdef USE_SNDFILE

SF_INFO info;
SNDFILE *output_wavfile;


void aofile_send_frame_wav(char *buffer)
{

        sf_write_raw(output_wavfile,(const short *)buffer,AUDIO_BUFFER_SIZE*2); //*2 porque es stereo
}

void init_aofile_wav(void)
{


	info.format=SF_FORMAT_WAV | SF_FORMAT_PCM_U8;
	info.samplerate=FRECUENCIA_SONIDO;
	info.channels=2;
	info.frames= 123456789 ; /* Wrong length. Library should correct this on sf_close. */


	// open output file
	output_wavfile = sf_open(aofilename, SFM_WRITE, &info);

	//printf ("open wav\n");

	if (output_wavfile==NULL) {
		debug_printf (VERBOSE_ERR,"Failed with error: %s",sf_strerror(NULL));
		aofile_inserted.v=0;
		return;
	}


}
#endif






void init_aofile(void)
{

		//por defecto
		aofile_type=AOFILE_TYPE_RAW;

		//debug_printf (VERBOSE_INFO,"Initializing Audio Output File");
		if (!util_compare_file_extension(aofilename,"wav")) {
#ifdef USE_SNDFILE
			debug_printf (VERBOSE_INFO,"Output file is wav file");
			aofile_type=AOFILE_TYPE_WAV;
			init_aofile_wav();

			sprintf(last_message_helper_aofile_vofile_file_format,"Writing audio output file, format wav, %dHz, 8 bit, unsigned, 2 channels",FRECUENCIA_SONIDO);
			debug_printf(VERBOSE_INFO,"%s",last_message_helper_aofile_vofile_file_format);
#else
			debug_printf (VERBOSE_ERR,"Output file is wav file but sndfile support is not compiled");
			aofile_inserted.v=0;
			return;

#endif


		}

		if (aofile_type==AOFILE_TYPE_RAW) {

	                ptr_aofile=fopen(aofilename,"wb");
			//printf ("ptr_aofile: %p\n",ptr_aofile);


	                if (!ptr_aofile)
        	        {
                	        debug_printf(VERBOSE_ERR,"Unable to create aofile %s",aofilename);
				aofilename=NULL;
				aofile_inserted.v=0;
				return;
                	}

			sprintf(last_message_helper_aofile_vofile_file_format,"Writing audio output file, format raw, %dHz, 8 bit, unsigned, 1 channel",FRECUENCIA_SONIDO);
			debug_printf(VERBOSE_INFO,"%s",last_message_helper_aofile_vofile_file_format);

		}

        aofile_buffer=malloc(AUDIO_BUFFER_SIZE*2); //*2 porque es stereo en wav. en rwa, es mono (usara la mitad del buffer)
        if (aofile_buffer==NULL) {
                cpu_panic("Error allocating audio output buffer");
        }


	aofile_inserted.v=1;




	print_helper_aofile_vofile();


	//se puede convertir con sox mediante: sox  -t .raw -r 15600 -b 8 -e unsigned -c 2 archivo.raw archivo.wav

}

void close_aofile(void)
{

	if (aofile_inserted.v==0) {
		debug_printf (VERBOSE_INFO,"Closing aofile. But already closed");
		return;
	}

	aofile_inserted.v=0;

	if (aofile_type==AOFILE_TYPE_RAW) {
		debug_printf (VERBOSE_INFO,"Closing aofile type RAW");
		fclose(ptr_aofile);
	}

#ifdef USE_SNDFILE

	if (aofile_type==AOFILE_TYPE_WAV) {
		debug_printf (VERBOSE_INFO,"Closing aofile type WAV");
		sf_close(output_wavfile);
	}

#endif


}


void aofile_send_frame(char *buffer)
{
	int escritos;


	//printf ("buffer: %p ptr_aofile: %p\n",buffer,ptr_aofile);

	//copiamos a buffer temporal pasandolo a unsigned
	//Esto conviene por dos cosas:
	//1) aofile a wav, solo deja generar archivos de 8 bit unsigned
	//2) mplayer, por ejemplo, al reproducir un .raw espera un unsigned (y no se como decirle que espere un signed)
	//memcpy(aofile_buffer,buffer,AUDIO_BUFFER_SIZE);


        //Convertimos buffer signed en unsigned
        char *aofile_buffer_orig;
        aofile_buffer_orig=aofile_buffer;
        unsigned char valor_unsigned;

        int i;
        for (i=0;i<AUDIO_BUFFER_SIZE*2;i++) { //*2 porque es stereo

		//Pero si genera rwa, el archivo de salida es mono
		//sumar los dos canales en 1
		if (aofile_type==AOFILE_TYPE_RAW) {
			int canal_izq;
			int canal_der;
			canal_izq=*buffer;
			buffer++;
			canal_der=*buffer;
			
			int suma=(canal_izq+canal_der)/2;
			char suma_byte=suma; //necesario???
	
			valor_unsigned=suma_byte;
		}

		else {
	                valor_unsigned=*buffer;
		}
        	valor_unsigned=128+valor_unsigned;

               	*aofile_buffer=valor_unsigned;

                buffer++;
		aofile_buffer++;

        }


	aofile_buffer=aofile_buffer_orig;

	//Envio de audio a raw file
	if (aofile_type==AOFILE_TYPE_RAW) {

		//escritos=fwrite(aofile_buffer, 1, AUDIO_BUFFER_SIZE*2, ptr_aofile);  //*2 porque es stereo
		escritos=fwrite(aofile_buffer, 1, AUDIO_BUFFER_SIZE, ptr_aofile);  //es mono el rwa

		//if (escritos!=AUDIO_BUFFER_SIZE*2) {
		if (escritos!=AUDIO_BUFFER_SIZE) {
                	        debug_printf(VERBOSE_ERR,"Unable to write to aofile %s",aofilename);
	                        aofilename=NULL;
        	                aofile_inserted.v=0;


			//debug_printf(VERBOSE_ERR,"Bytes escritos: %d\n",escritos);
		}
	}

#ifdef USE_SNDFILE
	//Envio de audio a wav file
	if (aofile_type==AOFILE_TYPE_WAV) {
		aofile_send_frame_wav(aofile_buffer);
	}
#endif

}

void set_active_audio_buffer(void)
{
        if (audio_buffer_switch.v==0) {
                audio_buffer=audio_buffer_one;
		audio_buffer_playback=audio_buffer_two;
        }
        else {
                audio_buffer=audio_buffer_two;
		audio_buffer_playback=audio_buffer_one;
        }


}

void audio_empty_buffer(void)
{

	if (audio_buffer==NULL) return;

	debug_printf (VERBOSE_DEBUG,"Emptying audio buffer");

	int i=0;
	while (i<AUDIO_BUFFER_SIZE*2) { //*2 porque es stereo
		audio_buffer[i++]=0;
	}

}

//esto se puede borrar
//int temp_borrarrrr=0;

void envio_audio(void)
{

	//beeper_silence_detection_counter=0;

	//temporal. ver contenido buffer. deberia ser todo igual
	//if (silence_detection_counter==SILENCE_DETECTION_MAX) {
	//	int i;
	//	for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
        //                printf ("%02X ",audio_buffer[i]);
	//	}
	//}

	//temporal
	//if (beeper_silence_detection_counter==SILENCE_DETECTION_MAX) printf ("silencio beeper\n");


	//Si aofile, silencio=0
	if (aofile_inserted.v) silence_detection_counter=0;


	//Incrementar si conviene y avisar con mensaje por debug cuando se llega al maximo
	if (beeper_silence_detection_counter!=SILENCE_DETECTION_MAX) {
		beeper_silence_detection_counter++;
		if (beeper_silence_detection_counter==SILENCE_DETECTION_MAX) debug_printf(VERBOSE_DEBUG,"Beeper Silence detected");
	}


	//Incrementar si conviene y avisar con mensaje por debug cuando se llega al maximo
	if (silence_detection_counter!=SILENCE_DETECTION_MAX) {
		silence_detection_counter++;
		if (audio_using_sdl2) silence_detection_counter=0; //workaround para que no salte detector con sdl2

		if (silence_detector_setting.v==0) silence_detection_counter=0;
        	
		if (silence_detection_counter==SILENCE_DETECTION_MAX) debug_printf(VERBOSE_DEBUG,"Silence detected");
	}

	//para aumentar buffer sonido
	contador_frames_veces_buffer_audio++;

	if (contador_frames_veces_buffer_audio==FRAMES_VECES_BUFFER_AUDIO) {
		contador_frames_veces_buffer_audio=0;

	        //Parche para maquinas que no generan 312 lineas, porque si enviamos menos sonido se escuchara un click al final
                //Es necesario que el buffer de sonido este completo
		//En ZX80,81 y spectrum ya se comprueba en el core. Pero en Z88 no, y es necesario hacerlo aqui
		//para llenar el buffer cuando conviene
		//si no se ha llenado el buffer de audio, repetir ultimo valor

		//Esto se hacia antes incorrectamente desde core_xxx.c, y solo daba buen resultado con sonido sin pthreads
		//con pthreads, el comportamiento era un tanto extranyo, sobretodo en z88 y mac, en que el driver es lento,
		//y no llega a llenar el buffer de audio

		//si se ha llenado el buffer, audio_buffer_indice=AUDIO_BUFFER_SIZE-1
		//pero si falta un byte para llenar el buffer, audio_buffer_indice=AUDIO_BUFFER_SIZE-1 tambien!
		//entonces nos faltara 1 byte para llenar,
		//pero... como aqui se llama cada 5 frames de pantalla, esto o bien esta lleno el buffer,
		//o al buffer le faltan 5 bytes para llenar minimo.

		//printf ("audio_buffer_indice: %d AUDIO_BUFFER_SIZE: %d\n",audio_buffer_indice,AUDIO_BUFFER_SIZE);
		if (audio_buffer_indice<AUDIO_BUFFER_SIZE-1 && audio_buffer_indice>0) {
			//int debug_diferencia=AUDIO_BUFFER_SIZE-1-audio_buffer_indice;
			//printf ("entrando %d dif: %d\n",temp_borrarrrr++,debug_diferencia);
			//Aqui audio_buffer_indice siempre deberia entrar >0. pero comprobamos por si acaso
			char valor_enviar;
			valor_enviar=audio_buffer[audio_buffer_indice-1];

			//printf ("valor a enviar: %d\n",valor_enviar);

	                while (audio_buffer_indice<AUDIO_BUFFER_SIZE) {
        	        	audio_buffer[audio_buffer_indice++]=valor_enviar;
				//printf ("en envio_audio. audio_buffer_indice: %d\n",audio_buffer_indice);
	                }

			//printf ("saliendo %d dif: %d\n",temp_borrarrrr++,debug_diferencia);
		}

		//conmutar audio buffer
		audio_buffer_switch.v ^=1;
		audio_buffer_indice=0;
		set_active_audio_buffer();


		//aofile
		if (aofile_inserted.v) {
			aofile_send_frame(audio_buffer_playback);
		}

		//Enviar frame de sonido despues de aofile.
		//Esto ahora quiza no es muy importante, antes el driver pulse por ejemplo, alteraba el contenido
		//del buffer pasandolo a unsigned ; ese cambio en pulseaudio ahora se hace en otro buffer diferente

		//Enviar sonido si no hay silencio
		if (silence_detection_counter!=SILENCE_DETECTION_MAX) {
			audio_send_frame(audio_buffer_playback);
		}


        }

	mid_frame_event();

	midi_output_frame_event();

}


void set_value_beeper_on_array(char value)
{
        if (beeper_silence_detection_counter==SILENCE_DETECTION_MAX) return;

                if (beeper_real_enabled) {
                        int i;
                        i=t_estados % screen_testados_linea;
                        if (i>=0 && i<CURRENT_BEEPER_ARRAY_LENGTH) {
                                buffer_beeper[i]=value;
                        }
                }
}

char beeper_get_last_value_send(void)
{
                if (MACHINE_IS_SPECTRUM) {
                        return value_beeper;
                }

                else if (MACHINE_IS_ZX8081) {
                        return da_amplitud_speaker_zx8081();
                }

		else if (MACHINE_IS_ACE) {
			return da_amplitud_speaker_ace();
		}

		//Sera Z88
                //else if (MACHINE_IS_Z88) {
                return z88_get_beeper_sound();
                //}
}

void beeper_new_line(void) {

        if (beeper_silence_detection_counter==SILENCE_DETECTION_MAX) return;


                int i;

                //en principio inicializamos el primer valor con el ultimo out del border
		buffer_beeper[0]=beeper_get_last_value_send();

                //Siguientes valores inicializamos a 65535
                for (i=1;i<CURRENT_BEEPER_ARRAY_LENGTH;i++) buffer_beeper[i]=65535;

		//printf ("beeper array length: %d\n",i);
}

char get_value_beeper_sum_array(void)
{
        if (beeper_silence_detection_counter==SILENCE_DETECTION_MAX) {
		//Si estamos en beeper silencio, retornamos siempre ultimo valor enviado
		return beeper_get_last_value_send();
	}

        char result_8bit;
        char leido8bit;

        int result;

        //Primer valor
        leido8bit=buffer_beeper[0];
        result=leido8bit;

        int i;
        for (i=0;i<screen_testados_linea;i++) {
                if (buffer_beeper[i]!=65535) leido8bit=buffer_beeper[i];

                //vamos sumando valores
                result +=leido8bit;
        }

        //Finalmente dividimos entre total
        result =  result / screen_testados_linea;

        //este valor debe entrar en el rango de char
        if (result>127 || result<-128) debug_printf (VERBOSE_DEBUG,"Beeper audio value out of range: %d",result);
        //printf ("result: %d\n",result);

        result_8bit=result;

        return result_8bit;
}



//Esto deberia ser char, pero pongo int para ver si asi el sonido en raspberry, bajando el volumen, va bien, pero ni asi
//TODO
int audio_adjust_volume(int valor_enviar)
{
                                int v;

				v=valor_enviar;

                                v=v*audiovolume;

				v=v/100;

                                return v;
}

/* Frecuencias notas musicales, incluido longitud de onda
extraido de http://www.phy.mtu.edu/~suits/notefreqs.html
y
http://liutaiomottola.com/formulae/freqtab.htm

Note	Frequency (Hz)	Wavelength (cm)
C0	16.35 	2109.89
C#0  	17.32 	1991.47
D0	18.35 	1879.69
D#0  	19.45 	1774.20
E0	20.60 	1674.62
F0	21.83 	1580.63
F#0  	23.12 	1491.91
G0	24.50 	1408.18
G#0  	25.96 	1329.14
A0	27.50 	1254.55
A#0  	29.14 	1184.13
B0	30.87 	1117.67
C1	32.70 	1054.94
C#1  	34.65 	995.73
D1	36.71 	939.85
D#1  	38.89 	887.10
E1	41.20 	837.31
F1	43.65 	790.31
F#1  	46.25 	745.96
G1	49.00 	704.09
G#1  	51.91 	664.57
A1	55.00 	627.27
A#1  	58.27 	592.07
B1	61.74 	558.84
C2	65.41 	527.47
C#2  	69.30 	497.87
D2	73.42 	469.92
D#2  	77.78 	443.55
E2	82.41 	418.65
F2	87.31 	395.16
F#2  	92.50 	372.98
G2	98.00 	352.04
G#2  	103.83 	332.29
A2	110.00 	313.64
A#2  	116.54 	296.03
B2	123.47 	279.42
C3	130.81 	263.74
C#3  	138.59 	248.93
D3	146.83 	234.96
D#3  	155.56 	221.77
E3	164.81 	209.33
F3	174.61 	197.58
F#3  	185.00 	186.49
G3	196.00 	176.02
G#3  	207.65 	166.14
A3	220.00 	156.82
A#3  	233.08 	148.02
B3	246.94 	139.71
C4	261.63 	131.87
C#4  	277.18 	124.47
D4	293.66 	117.48
D#4  	311.13 	110.89
E4	329.63 	104.66
F4	349.23 	98.79
F#4  	369.99 	93.24
G4	392.00 	88.01
G#4  	415.30 	83.07
A4	440.00 	78.41
A#4  	466.16 	74.01
B4	493.88 	69.85
C5	523.25 	65.93
C#5  	554.37 	62.23
D5	587.33 	58.74
D#5  	622.25 	55.44
E5	659.25 	52.33
F5	698.46 	49.39
F#5  	739.99 	46.62
G5	783.99 	44.01
G#5  	830.61 	41.54
A5	880.00 	39.20
A#5  	932.33 	37.00
B5	987.77 	34.93
C6	1046.50 	32.97
C#6  	1108.73 	31.12
D6	1174.66 	29.37
D#6  	1244.51 	27.72
E6	1318.51 	26.17
F6	1396.91 	24.70
F#6  	1479.98 	23.31
G6	1567.98 	22.00
G#6  	1661.22 	20.77
A6	1760.00 	19.60
A#6  	1864.66 	18.50
B6	1975.53 	17.46
C7	2093.00 	16.48
C#7  	2217.46 	15.56
D7	2349.32 	14.69
D#7  	2489.02 	13.86
E7	2637.02 	13.08
F7	2793.83 	12.35
F#7  	2959.96 	11.66
G7	3135.96 	11.00
G#7  	3322.44 	10.38
A7	3520.00 	9.80
A#7  	3729.31 	9.25
B7	3951.07 	8.73
C8	4186.01 	8.24
C#8  	4434.92 	7.78
D8	4698.63 	7.34
D#8  	4978.03 	6.93
E8	5274.04 	6.54
F8	5587.65 	6.17
F#8  	5919.91 	5.83
G8	6271.93 	5.50
G#8  	6644.88 	5.19
A8	7040.00 	4.90
A#8  	7458.62 	4.63
B8	7902.13 	4.37
C9 	8372.018 	4.1
C#9 	8869.844 	3.8
D9	9397.272 	3.6
D#9 	9956.064 	3.4
E9 	10548.084 	3.2
F9 	11175.304 	3
F#9 	11839.82 	2.9
G9 	12543.856 	2.7
G#9 	13289.752 	2.6
A9	14080 	2.4
A#9 	14917.24 	2.3
B9 	15804.264 	2.2


*/

//10 octavas. 12 notas por cada octava
//#define NOTAS_MUSICALES_OCTAVAS 10
//#define NOTAS_MUSICALES_NOTAS_POR_OCTAVA 12
//sacamos la lista con: cat audio.c|sed 's/\..*//'|awk '{printf "\{\"%s\",%s\},\n",$1,$2}'

nota_musical tabla_notas_musicales[MAX_NOTAS_MUSICALES]={
{"C0",16},
{"C#0",17},
{"D0",18},
{"D#0",19},
{"E0",20},
{"F0",21},
{"F#0",23},
{"G0",24},
{"G#0",25},
{"A0",27},
{"A#0",29},
{"B0",30},
{"C1",32},
{"C#1",34},
{"D1",36},
{"D#1",38},
{"E1",41},
{"F1",43},
{"F#1",46},
{"G1",49},
{"G#1",51},
{"A1",55},
{"A#1",58},
{"B1",61},
{"C2",65},
{"C#2",69},
{"D2",73},
{"D#2",77},
{"E2",82},
{"F2",87},
{"F#2",92},
{"G2",98},
{"G#2",103},
{"A2",110},
{"A#2",116},
{"B2",123},
{"C3",130},
{"C#3",138},
{"D3",146},
{"D#3",155},
{"E3",164},
{"F3",174},
{"F#3",185},
{"G3",196},
{"G#3",207},
{"A3",220},
{"A#3",233},
{"B3",246},
{"C4",261},
{"C#4",277},
{"D4",293},
{"D#4",311},
{"E4",329},
{"F4",349},
{"F#4",369},
{"G4",392},
{"G#4",415},
{"A4",440},
{"A#4",466},
{"B4",493},
{"C5",523},
{"C#5",554},
{"D5",587},
{"D#5",622},
{"E5",659},
{"F5",698},
{"F#5",739},
{"G5",783},
{"G#5",830},
{"A5",880},
{"A#5",932},
{"B5",987},
{"C6",1046},
{"C#6",1108},
{"D6",1174},
{"D#6",1244},
{"E6",1318},
{"F6",1396},
{"F#6",1479},
{"G6",1567},
{"G#6",1661},
{"A6",1760},
{"A#6",1864},
{"B6",1975},
{"C7",2093},
{"C#7",2217},
{"D7",2349},
{"D#7",2489},
{"E7",2637},
{"F7",2793},
{"F#7",2959},
{"G7",3135},
{"G#7",3322},
{"A7",3520},
{"A#7",3729},
{"B7",3951},
{"C8",4186},
{"C#8",4434},
{"D8",4698},
{"D#8",4978},
{"E8",5274},
{"F8",5587},
{"F#8",5919},
{"G8",6271},
{"G#8",6644},
{"A8",7040},
{"A#8",7458},
{"B8",7902},
{"C9",8372},
{"C#9",8869},
{"D9",9397},
{"D#9",9956},
{"E9",10548},
{"F9",11175},
{"F#9",11839},
{"G9",12543},
{"G#9",13289},
{"A9",14080},
{"A#9",14917},
{"B9",15804}
};

//Para si hay que retornar una nota desconocida;
char *unknown_nota_musical="XX";

//convertir nombre nota en formato string a formato mid
//Si no coincide, retornar -1
int get_mid_number_note(char *str)
{
	//nota_musical tabla_notas_musicales[MAX_NOTAS_MUSICALES]={
	const int offset_inicial=12;  //C0=12

	//cadena vacia, -1
	if (str[0]==0) return -1;

	int i;
	for (i=0;i<MAX_NOTAS_MUSICALES;i++) {
		if (!strcasecmp(str,tabla_notas_musicales[i].nombre)) return i+offset_inicial;
	}

	return -1;
}

//devuelve nombre nota, segun su frecuencia se aproxime lo maximo
char *get_note_name(int frecuencia)
{
//nota_musical tabla_notas_musicales[MAX_NOTAS_MUSICALES]

	int i;

	int diferencia_anterior=99999999;

	int diferencia;

	for (i=0;i<MAX_NOTAS_MUSICALES;i++) {
		diferencia=frecuencia-tabla_notas_musicales[i].frecuencia;
		//nos interesa el valor absoluto
		if (diferencia<0) diferencia=-diferencia;

		//si es menor que anterior, guardamos y seguimos
		if (diferencia<diferencia_anterior) diferencia_anterior=diferencia;
		else {
			//si es mayor, fin bucle
			break;
		}
	}

	//retornamos indice anterior. tambien tener en cuenta que si se llega al final del array, i=MAX_NOTAS_MUSICALES, pero al restar 1,
	//nos situaremos en el ultimo item
	int indice_result=i-1;
	if (i<0) i=0;

	//debug_printf (VERBOSE_DEBUG,"Nota frecuencia: %d Indice result: %d nota: %s",frecuencia,indice_result,tabla_notas_musicales[indice_result].nombre);
	return tabla_notas_musicales[indice_result].nombre;
}

//devuelve nombre nota, segun su indice (igual que el pitch)
char *get_note_name_by_index(int index)
{
	if (index<0 || index>=MAX_NOTAS_MUSICALES) return unknown_nota_musical;

	else return tabla_notas_musicales[index].nombre;
}

//devuelve numero nota (0...6 do,re... si), si es sostenido, y numero de octava, segun string obtenido de funcion get_note_name
//Devuelve nota -1 si string no coincide con lo esperado
void get_note_values(char *texto,int *nota_final,int *si_sostenido,int *octava)
{
	char *nota_string="cdefgab";

	int i;
	
	*nota_final=-1;
	*octava=0;
	for (i=0;i<7;i++) {
		if (letra_minuscula(texto[0])==nota_string[i]) {
			*nota_final=i;
			break;
		}
	}

	*si_sostenido=0;
	if (*nota_final>=0) {
		if (texto[1]=='#') {
			*si_sostenido=1;
			*octava=texto[2]-'0';
		}
		else {
			*octava=texto[1]-'0';
		}
	}
}

int set_audiodriver_null(void) {
                        audio_init=audionull_init;
                        audio_send_frame=audionull_send_frame;
                        audio_thread_finish=audionull_thread_finish;
                        audio_end=audionull_end;
			audio_get_buffer_info=audionull_get_buffer_info;
                        return 0;

                }


//Ha fallado el init del driver de audio y hacemos fallback a null
void fallback_audio_null(void)
{
	debug_printf (VERBOSE_ERR,"Error using audio output driver %s. Fallback to null",audio_driver_name);
	set_audiodriver_null();
	audio_init();
}


/*

temporal escribir frecuencia en archivo
Se puede llamar aqui desde envio_audio, para generar un archivo de log de frecuencias de sonido cada 1/10 segundos
la manera de llamar seria:
                //temp
                int freq=temp_audio_return_frecuencia_buffer();
                printf ("freq: %d\n",freq);
                temp_write_frequency_disk(freq);

esto justo antes de

             //conmutar audio buffer
                audio_buffer_switch.v ^=1;

Se utiliza luego el programa freqlog_to_rwa.c para pasar ese archivo de log de frecuencias a un archivo de audio

La funcion siguiente temp_audio_return_frecuencia_buffer tambien es en pruebas

*/
void temp_write_frequency_disk(int f)
{
	unsigned char buffer[2];

	buffer[0]=f&0xFF;
	buffer[1]=(f>>8)&0xFF;

	FILE *ptr_file;


	ptr_file=fopen("pruebafreq.log","a");
                                  if (!ptr_file)
                                {
                                      debug_printf (VERBOSE_ERR,"Unable to open Freq file");
                                  }

				else {
                                        fwrite(buffer, 1, 2, ptr_file);

                                  fclose(ptr_file);

				}

}



//casi misma funcion que saca la frecuencia del buffer de sonido usado en view waveform
int temp_audio_return_frecuencia_buffer(void)
{


	//Obtenemos tambien cuantas veces cambia de signo (y por tanto, obtendremos frecuencia aproximada)
        int cambiossigno=0;
        int signoanterior=0;
        int signoactual=0;

        char valor_sonido;


        z80_byte valor_sonido_sin_signo;
        z80_byte valor_anterior_sin_signo=0;

        int i;
        for (i=0;i<AUDIO_BUFFER_SIZE;i++) {
                valor_sonido=audio_buffer[i];

                valor_sonido_sin_signo=valor_sonido;

                if (valor_sonido_sin_signo>valor_anterior_sin_signo) signoactual=+1;
                if (valor_sonido_sin_signo<valor_anterior_sin_signo) signoactual=-1;

                valor_anterior_sin_signo=valor_sonido_sin_signo;

                if (signoactual!=signoanterior) {
                        cambiossigno++;
                        signoanterior=signoactual;
                }

        }

        //Calculo frecuencia aproximada
        return ((FRECUENCIA_SONIDO/AUDIO_BUFFER_SIZE)*cambiossigno)/2;


}

z80_byte *audio_ay_player_mem;

//Longitud de la cancion en 1/50s
z80_int ay_song_length=0;
//Contador actual de transcurrido
z80_int ay_song_length_counter=0;

z80_byte ay_player_pista_actual=1;

z80_bit ay_player_playing={0};

//Cerrar emulador al reproducir todas canciones de archivo AY
z80_bit ay_player_exit_emulator_when_finish={0};

//Limite en 1/50 segundos para canciones que tienen duracion infinita (0)
z80_int ay_player_limit_infinite_tracks=0;

//Limite en 1/50 segundos para cualquier cancion. Si es 0, no limitar
z80_int ay_player_limit_any_track=0;

//Ir a primera pista cuando se llega al final
z80_bit ay_player_repeat_file={1};

//Reproducir en modo cpc
z80_bit ay_player_cpc_mode={0};

//Retorna valor de 16 bit apuntado como big endian
z80_int audio_ay_player_get_be_word(int index)
{
	return ((audio_ay_player_mem[index])*256 ) + audio_ay_player_mem[index+1];
}


//Retorna valor a indice teniendo como entrada indice y un puntero de 16 bits absoluto en ese indice inicial
int audio_ay_player_get_abs_index(int initial_index)
{
	int final_index;

	final_index=initial_index+audio_ay_player_get_be_word(initial_index);

	return final_index;
}

z80_byte ay_player_total_songs(void)
{
	return audio_ay_player_mem[16]+1;
}

z80_byte ay_player_first_song(void)
{
	return audio_ay_player_mem[17]+1;
}

z80_byte ay_player_version(void)
{
	return audio_ay_player_mem[8];
}

void ay_player_set_footer(char *texto1,char *texto2)
{
			sprintf (mostrar_footer_first_message,"%s",texto1);

	//Texto mostrado
	sprintf (mostrar_footer_first_message_mostrado,"%s",mostrar_footer_first_message);

	mostrar_footer_second_message=texto2;
	//mostrar_footer_second_message=NULL;


	indice_first_message_mostrado=indice_second_message_mostrado=0;

	//Cortar a 32
	tape_options_corta_a_32(mostrar_footer_first_message_mostrado);

	tape_options_set_first_message_counter=4;

	autoselect_options_put_footer();

}

char ay_player_file_author[1024]="";
char ay_player_file_misc[1024]="";
char ay_player_file_song_name[1024]="";
char ay_player_second_footer[1024]="";

//Retorna 0 si ok
int audio_ay_player_load(char *filename)
{
	//Leemos archivo en memoria
	int total_mem=get_file_size(filename);


	      FILE *ptr_ayfile;
	      ptr_ayfile=fopen(filename,"rb");

	      if (!ptr_ayfile) {
	  debug_printf(VERBOSE_ERR,"Unable to open ay file");
	  return 1;
	}

	//Si estaba asignado, desasignar
	if (audio_ay_player_mem!=NULL) free(audio_ay_player_mem);

	audio_ay_player_mem=malloc(total_mem);
	if (audio_ay_player_mem==NULL) cpu_panic("Error allocating memory for ay file");

	//z80_byte *puntero_lectura;
	//puntero_lectura=audio_ay_player_mem;


	      int leidos=fread(audio_ay_player_mem,1,total_mem,ptr_ayfile);

	if (leidos==0) {
	              debug_printf(VERBOSE_ERR,"Error reading ay file");
	  free(audio_ay_player_mem);
					audio_ay_player_mem=NULL;
	              return 1;
	      }


	      fclose(ptr_ayfile);

  //Mostrar información de la canción
	int indice_autor=audio_ay_player_get_abs_index(12);
	int indice_misc=audio_ay_player_get_abs_index(14);
	z80_byte file_version=ay_player_version();

	debug_printf (VERBOSE_INFO,"Version: %d",file_version);
	debug_printf (VERBOSE_INFO,"Author: %s",&audio_ay_player_mem[indice_autor]);
	debug_printf (VERBOSE_INFO,"Misc: %s",&audio_ay_player_mem[indice_misc]);
	debug_printf (VERBOSE_INFO,"Total songs: %d",ay_player_total_songs() );
	debug_printf (VERBOSE_INFO,"First song: %d",ay_player_first_song() );

//sprintf (ay_player_file_author_misc,"%s - %s",&audio_ay_player_mem[indice_autor],&audio_ay_player_mem[indice_misc]);
sprintf (ay_player_file_author,"%s",&audio_ay_player_mem[indice_autor]);
sprintf (ay_player_file_misc,"%s",&audio_ay_player_mem[indice_misc]);

//temp
//sprintf (ay_player_file_author,"%s","This is my second spectrum emulator after ZXSpectr");
//sprintf (ay_player_file_misc,"%s","I recommend you to read FEATURES, INSTALL and HISTORY files");


//ay_player_set_footer(ay_player_file_author_misc);

	/*mostrar_footer_game_name=argv[puntero_parametro];
  mostrar_footer_second_message=argv[puntero_parametro];*/


	if (file_version>3) {
		debug_printf(VERBOSE_ERR,"File version>3 not supported yet (file version: %d)",file_version);
		return 1;
	}


				return 0;
}

//Retorna indice a estructura canciones
int audio_ay_player_song_str(void)
{
	return audio_ay_player_get_abs_index(18);
	//return 20;

	/*
	Que es psongstructure???
	18   PSongsStructure:smallint;	//Relative pointer to song structure
	  end;

	20  Next is song structure (repeated NumOfSongs + 1 times):

	*/
}

void ay_player_poke(z80_int destino,z80_byte valor)
{
	if (MACHINE_IS_SPECTRUM) {
		if (destino<16384) memoria_spectrum[destino]=valor;
		else poke_byte_no_time(destino,valor);

		//printf ("poke %04X valor %02X\n",destino,valor);
	}

	if (MACHINE_IS_CPC) {
		poke_byte_no_time_cpc(destino,valor);
	}
}

void ay_player_copy_mem(int origen_archivo,z80_int destino,z80_int longitud)
{
	for (;longitud;longitud--) {
		//memoria_spectrum[destino++]=audio_ay_player_mem[origen_archivo++];
		ay_player_poke(destino++,audio_ay_player_mem[origen_archivo++]);
	}
}
//memcpy(&memoria_spectrum[bloque_direccion],&audio_ay_player_mem[origen_archivo],bloque_longitud);

void ay_player_copy_to_rom(z80_byte *origen,z80_int destino,z80_int longitud)
{
	for (;longitud;longitud--) {
		ay_player_poke(destino++,*origen);
		//printf ("poke %d value %02X\n",destino,*origen);

		origen++;
	}
}

void ay_player_mem_set(z80_int destino,z80_byte valor,z80_int longitud)
{
	for (;longitud;longitud--) {
		ay_player_poke(destino++,valor);
	}
}

//Retorna 0 si ok
int audio_ay_player_play_song(z80_byte song)
{

	if (audio_ay_player_mem==NULL) {
		debug_printf(VERBOSE_ERR,"No song loaded");
		return 1;
	}

	if (song<1) {
			debug_printf (VERBOSE_ERR,"Song number must be >0");
			return 1;
	}

	z80_byte version=ay_player_version();

	//Puntero a estructura inicial
	int song_struct=audio_ay_player_song_str();

	//Puntero a cancion concreta
	int offset=(song-1)*4;

	song_struct +=offset;

	//Mostrar nombre cancion
	int indice_nombre=audio_ay_player_get_abs_index(song_struct);
	debug_printf (VERBOSE_INFO,"Song %d name: %s",song,&audio_ay_player_mem[indice_nombre]);





	int song_data=audio_ay_player_get_abs_index(song_struct+2);


if (ay_player_cpc_mode.v==0) {
	//Si maquina actual ya es spectrum, no cargar de nuevo rom
	if (current_machine_type!=MACHINE_ID_SPECTRUM_48) {
		//Resetear a maquina spectrum 48
		current_machine_type=MACHINE_ID_SPECTRUM_48;
		set_machine(NULL);
	}

}


else {
	if (current_machine_type!=MACHINE_ID_CPC_464) {
		current_machine_type=MACHINE_ID_CPC_464;
		set_machine(NULL);
	}

}


	cold_start_cpu_registers();
	reset_cpu();

if (ay_player_cpc_mode.v==1) {
	cpc_gate_registers[2] |=(4|8); //Mapear todo RAM
	cpc_set_memory_pages();
}

	sprintf (ay_player_file_song_name,"%s",&audio_ay_player_mem[indice_nombre]);

	sprintf (ay_player_second_footer,"%s - %s",ay_player_file_author,ay_player_file_misc);

	//temp
	//sprintf (ay_player_file_song_name,"%s","You can open them from the help menu or from an ");



	ay_player_set_footer(ay_player_file_song_name,ay_player_second_footer);

	ay_song_length=audio_ay_player_get_be_word(song_data+4);

	if (ay_song_length==0) ay_song_length=ay_player_limit_infinite_tracks;

	if (ay_player_limit_any_track>0) {
		if (ay_song_length>ay_player_limit_any_track || ay_song_length==0) ay_song_length=ay_player_limit_any_track;
	}



	//printf ("Song length: %d 1/50s (%d s)\n",ay_song_length,ay_song_length/50);

	ay_song_length_counter=0;
	ay_player_playing.v=1;

	//Cargar registros
	/*
	12   HiReg,LoReg:byte;		//HiReg and LoReg for all common registers:
					//AF,AF',HL,HL',DE,DE',BC,BC',IX and IY
					*/

	//Mostrar hexadecimal registros
	int offset_to_registers=song_data+8;

	/*
	int i;
	printf ("Dump registers:\n");
	for (i=0;i<20;i++){
		printf("%02d %02XH - ",i,audio_ay_player_mem[offset_to_registers+i]);
	}

	printf ("\n");
*/

	/*
	reg_a=0xff;
	Z80_FLAGS=0xff;
	BC=HL=DE=0xffff;
	reg_ix=reg_iy=reg_sp=0xffff;

	reg_h_shadow=reg_l_shadow=reg_b_shadow=reg_c_shadow=reg_d_shadow=reg_e_shadow=reg_a_shadow=Z80_FLAGS_SHADOW=0xff;
	reg_i=0;
	reg_r=reg_r_bit7=0;
	*/
//AF,AF',HL,HL',DE,DE',BC,BC',IX and IY

//No se en que version de ay se usan los registros... Al  menos en 0 no se usan!
//En 1,2,3 parece que si...

//Quiza solo lee registro A??
	reg_a=audio_ay_player_mem[offset_to_registers+0];
	Z80_FLAGS=audio_ay_player_mem[offset_to_registers+1];
	/*reg_a_shadow=audio_ay_player_mem[offset_to_registers+2];
	Z80_FLAGS_SHADOW=audio_ay_player_mem[offset_to_registers+3];

	reg_h=audio_ay_player_mem[offset_to_registers+4];
	reg_l=audio_ay_player_mem[offset_to_registers+5];
	reg_h_shadow=audio_ay_player_mem[offset_to_registers+6];
	reg_l_shadow=audio_ay_player_mem[offset_to_registers+7];

	reg_d=audio_ay_player_mem[offset_to_registers+8];
	reg_e=audio_ay_player_mem[offset_to_registers+9];
	reg_d_shadow=audio_ay_player_mem[offset_to_registers+10];
	reg_e_shadow=audio_ay_player_mem[offset_to_registers+11];

	reg_b=audio_ay_player_mem[offset_to_registers+12];
	reg_c=audio_ay_player_mem[offset_to_registers+13];
	reg_b_shadow=audio_ay_player_mem[offset_to_registers+14];
	reg_c_shadow=audio_ay_player_mem[offset_to_registers+15];

	reg_ix=audio_ay_player_mem[offset_to_registers+16]*256+audio_ay_player_mem[offset_to_registers+17];
	reg_iy=audio_ay_player_mem[offset_to_registers+18]*256+audio_ay_player_mem[offset_to_registers+19];*/

	//Volcar desde song_data en adelante

	/*
	int jj;
	printf ("--volcado desde song_data--\n");
	for (jj=0;jj<64;jj++)
	{
		if ((jj%16)==0) printf ("\n");
		printf ("%02X ",audio_ay_player_mem[song_data+jj]);

	}

	printf ("--fin volcado--\n");
	*/

	int ppoints,pdata;

	//if (version==0) {
	if (version==0 || version==1 || version==2 || version==3)  {
			ppoints=audio_ay_player_get_abs_index(song_data+10);
			pdata=audio_ay_player_get_abs_index(song_data+12);
	}

	else {
		ppoints=audio_ay_player_get_abs_index(offset_to_registers+20);
	//int ppoints=audio_ay_player_get_abs_index(offset_to_registers+10);

		pdata=audio_ay_player_get_abs_index(offset_to_registers+22);
	//int pdata=audio_ay_player_get_abs_index(offset_to_registers+12);

}

	reg_sp=audio_ay_player_get_be_word(ppoints);
	z80_int reg_init=audio_ay_player_get_be_word(ppoints+2);
	z80_int reg_inter=audio_ay_player_get_be_word(ppoints+4);

	debug_printf (VERBOSE_DEBUG,"SP: %04XH init=%04XH inter:%04XH",reg_sp,reg_init,reg_inter);

	/*if (reg_init==0) {
			debug_printf(VERBOSE_ERR,"AY files with init=0 not supported yet");
			return 1;
	}*/

	/*if (reg_inter==0) {
		debug_printf(VERBOSE_ERR,"AY files with inter=0 not supported yet");
		return 1;
	}*/


	//a) Fill #0000-#00FF range with #C9 value
	//b) Fill #0100-#3FFF range with #FF value
	//c) Fill #4000-#FFFF range with #00 value
	//d) Place to #0038 address #FB value
	//memset(memoria_spectrum+0x0000,0xc9,0x0100);
	//memset(memoria_spectrum+0x0100,0xff,0x3f00);
	//memset(memoria_spectrum+0x4000,0x00,0xc000);

ay_player_mem_set(0x0000,0xc9,0x0100);
ay_player_mem_set(0x0100,0xff,0x3f00);
ay_player_mem_set(0x4000,0x00,0xc000);



	//memoria_spectrum[0x38]=0xfb;         /* ei */
	ay_player_poke(0x38,0xfb);/* ei */
/*
	f) if INTERRUPT equal to ZERO then place at ZERO address next player:

	DI
	CALL    INIT
LOOP:	IM      2
	EI
	HALT
	JR      LOOP
	*/

/*	g) if INTERRUPT not equal to ZERO then place at ZERO address next player:

	DI
	CALL    INIT
	LOOP:	IM      1
	EI
	HALT
	CALL INTERRUPT
	JR      LOOP
*/

z80_byte player[256];

if (reg_inter==0) {
	//ay_player_copy_to_rom(z80_byte *origen,z80_int destino,z80_int longitud)

	player[0]=243;

	player[1]=205;
	player[2]=value_16_to_8l(reg_init);
	player[3]=value_16_to_8h(reg_init);

	player[4]=237;
	player[5]=94;

	player[6]=251;

	player[7]=118;  //HALT


	player[8]=24;
	player[9]=256-6;

	ay_player_copy_to_rom(player,0,10);
}
 else {

	player[0]=243;

	player[1]=205;
	player[2]=value_16_to_8l(reg_init);
	player[3]=value_16_to_8h(reg_init);

	player[4]=237;
	player[5]=86;

	player[6]=251;

	player[7]=118;  //HALT

	player[8]=205; //CALL INTER
	player[9]=value_16_to_8l(reg_inter);
	player[10]=value_16_to_8h(reg_inter);

	player[11]=24;
	player[12]=256-9;

	ay_player_copy_to_rom(player,0,13);

}
	//	h) Load all blocks for this song
	//		m) Load to PC ZERO value
	reg_pc=0;

//Ir leyendo bloques

int bloque=0;
z80_int bloque_direccion,bloque_longitud,bloque_offset;
do {
	bloque_direccion=audio_ay_player_get_be_word(pdata);
	bloque_longitud=audio_ay_player_get_be_word(pdata+2);
	bloque_offset=audio_ay_player_get_be_word(pdata+4);



		if (bloque_direccion!=0) {
			debug_printf (VERBOSE_DEBUG,"Block: %d address: %04XH length: %d offset in ay file: %d",
				bloque,bloque_direccion,bloque_longitud,bloque_offset);

			z80_int origen_archivo=pdata+4+bloque_offset;
			ay_player_copy_mem(origen_archivo,bloque_direccion,bloque_longitud);
			//memcpy(&memoria_spectrum[bloque_direccion],&audio_ay_player_mem[origen_archivo],bloque_longitud);

/*
e) if INIT equal to ZERO then place to first CALL instruction address of
	 first AY file block instead of INIT (see next f) and g) steps)
	 */
			if (reg_init==0 && bloque==0) reg_pc=bloque_direccion;
		}

bloque++;
pdata +=6;

} while (bloque_direccion!=0);


	//	i) Load all common lower registers with LoReg value (including AF register)
//		j) Load all common higher registers with HiReg value
//		k) Load into I register 3 (this player version)
reg_i=3;
//		l) load to SP stack value from points data of this song

//		n) Disable Z80 interrupts and set IM0 mode
iff1.v=iff2.v=0;
im_mode=0;
//		o) Emulate resetting of AY chip
//		p) Start Z80 emulation*/

return 0;

}



void ay_player_load_and_play(char *filename)
{
	if (audio_ay_player_load(filename)) {
		audio_ay_player_mem=NULL;
		return;
	}



 ay_player_pista_actual=1;
	 audio_ay_player_play_song(ay_player_pista_actual);
}



void 	ay_player_next_track (void)
{

	if (audio_ay_player_mem==NULL) return;

  if (ay_player_pista_actual<ay_player_total_songs() ) {
    ay_player_pista_actual++;

  }

	else {
		if (ay_player_exit_emulator_when_finish.v) end_emulator();

		else {
			if (ay_player_repeat_file.v) ay_player_pista_actual=1;
			else {
				ay_player_stop_player();
				return;
			}
		}
	}

	audio_ay_player_play_song(ay_player_pista_actual);
}


void 	ay_player_previous_track (void)
{

	if (audio_ay_player_mem==NULL) return;

	if (ay_player_pista_actual==1) ay_player_pista_actual=ay_player_total_songs();
	else ay_player_pista_actual--;

	audio_ay_player_play_song(ay_player_pista_actual);
}

//A donde se llama desde timer
void ay_player_playing_timer(void)
{
	if (audio_ay_player_mem==NULL) return;
	if (ay_player_playing.v==0) return;

	//Si es infinito, no saltar nunca a siguiente cancion
	if (ay_song_length==0) return;
	ay_song_length_counter++;

	//printf ("Contador cancion: %d limite: %d  (%d s)\n",ay_song_length_counter,ay_song_length,ay_song_length/50);

	if (ay_song_length_counter>ay_song_length) {
		ay_player_next_track();
	}


}

void ay_player_stop_player(void)
{
	ay_player_playing.v=0;
	audio_ay_player_mem=NULL;

	set_machine(NULL);
	cold_start_cpu_registers();
	reset_cpu();
}


//Mezclar la salida actual de sonido con el audiodac.
//El audiodac es muy simple, lo que hace es generar un valor de onda de 8 bits signed
void audiodac_mix(void)
{
	//Pasar valor a signed
	char valor_signed_audiodac=(audiodac_last_value_data-128);

	//Mezclar con el valor de salida
	int v;
	v=audio_valor_enviar_sonido_izquierdo+valor_signed_audiodac;
	v /=2;
	audio_valor_enviar_sonido_izquierdo=v;

	v=audio_valor_enviar_sonido_derecho+valor_signed_audiodac;
	v /=2;
	audio_valor_enviar_sonido_derecho=v;

}



//Obtener valores medio, maximo, minimo etc del buffer de audio

void audio_get_audiobuffer_stats(audiobuffer_stats *audiostats)
{
        //Obtenemos antes valor medio total y tambien maximo y minimo
        //Esto solo es necesario para dibujar onda llena

        //Obtenemos tambien cuantas veces cambia de signo (y por tanto, obtendremos frecuencia aproximada)

/*
Obtener:
valor maximo
valor minimo
valor medio
frecuencia aproximada
volumen maximo 


struct s_audiobuffer_stats
{
        int maximo;
        int minimo;
        int medio;
        int frecuencia;
        int volumen;
};

typedef struct s_audiobuffer_stats audiobuffer_stats;

*/
        int cambiossigno=0;
        int signoanterior=0;
        int signoactual=0;

        int audiomedio=0,audiomin=127,audiomax=-128;

        char valor_sonido;

		int valor_sonido_mezcla_stereo;


        z80_byte valor_sonido_sin_signo;
        z80_byte valor_anterior_sin_signo=0;

        //En AY Player tambien se usa una funcion similar. Se deberia estandarizar
        int i;
        for (i=0;i<AUDIO_BUFFER_SIZE;i++) {

				//if (audio_driver_accepts_stereo.v) {
					valor_sonido_mezcla_stereo=(audio_buffer[i*2]+audio_buffer[(i*2)+1])/2;
					valor_sonido=valor_sonido_mezcla_stereo;
				//}

				//else valor_sonido=audio_buffer[i];

                audiomedio +=valor_sonido;

                if (valor_sonido>audiomax) audiomax=valor_sonido;
                if (valor_sonido<audiomin) audiomin=valor_sonido;

                valor_sonido_sin_signo=128+valor_sonido;
				//if (valor_sonido<0) printf ("%d %d\n",valor_sonido,valor_sonido_sin_signo);

                if (valor_sonido_sin_signo>valor_anterior_sin_signo) signoactual=+1;
                if (valor_sonido_sin_signo<valor_anterior_sin_signo) signoactual=-1;

                valor_anterior_sin_signo=valor_sonido_sin_signo;

                if (signoactual!=signoanterior) {
                        cambiossigno++;
                        signoanterior=signoactual;
                }

        }

       //Calculo frecuencia aproximada
        //menu_audio_draw_sound_wave_frecuencia_aproximada=((FRECUENCIA_SONIDO/AUDIO_BUFFER_SIZE)*cambiossigno)/2;
        int frecuencia=((FRECUENCIA_SONIDO/AUDIO_BUFFER_SIZE)*cambiossigno)/2;

        //printf ("%d %d %d %d\n",FRECUENCIA_SONIDO,AUDIO_BUFFER_SIZE,cambiossigno,menu_audio_draw_sound_wave_frecuencia_aproximada);


        audiomedio /=AUDIO_BUFFER_SIZE;
      



	audiostats->maximo=audiomax;
	audiostats->minimo=audiomin;
	audiostats->medio=audiomedio;
	audiostats->frecuencia=frecuencia;

	//Falta obtener volumen




	//Sacamos volumen como el mayor valor de los dos, maximo o minimo
        //Obtenemos valores absolutos
        audiomax=util_get_absolute(audiomax);
        audiomin=util_get_absolute(audiomin);

        //Nos quedamos con el valor mayor
        int volumen=audiomax; //Suponemos este pero no tiene por que ser asi

        if (audiomin>audiomax) volumen=audiomin;

        //Ahora tenemos valor entre 0 y 128.

	//Le restamos el valor medio (en valor absoluto), para "centrar" el valor en la linea del 0
	volumen -=util_get_absolute(audiomedio);

	//Por si acaso, aunque no deberia pasar
	if (volumen<0) volumen=0;
	if (volumen>127) volumen=127;

	audiostats->volumen=volumen;





        int volumen_escalado=volumen;

        //Ahora tenemos valor entre 0 y 128. Pasar a entre 0 y 15
        //int valor_escalado=(mayor*16)/128;
        volumen_escalado=(volumen_escalado*16)/128;

        //Vigilar que no pase de 15
        if (volumen_escalado>15) volumen_escalado=15;

	audiostats->volumen_escalado=volumen_escalado;



}

char audio_change_top_speed_sound(char sonido)
{
	int valor_sonido=sonido;

        //
        //Valor random
        ay_randomize(0);

        //randomize_noise es valor de 16 bits
        int aleatorio=randomize_noise[0] % 100;
        valor_sonido=valor_sonido*aleatorio;
        valor_sonido=valor_sonido/100;


        //printf ("sonido: %d valor_sonido: %d dividir: %d\n",sonido,valor_sonido,dividir);

        sonido=valor_sonido;

	return sonido;
}


int audio_change_top_speed_index=0;
//Esta no la utilizo, era un intento de distorsionar el sonido de carga mediante onda senoidal

char old_audio_change_top_speed_sound(char sonido)
{
	int valor_sonido=sonido;
	char dividir=audio_sine_table[audio_change_top_speed_index];



	/*	
	//
	//Dividir

	//Evitar division por 0
	
	if (dividir==0) {
		int signo=util_get_sign(valor_sonido);
		valor_sonido=127;
		valor_sonido=valor_sonido * signo;
	}
	else {
		valor_sonido=valor_sonido*100;
		valor_sonido=valor_sonido/dividir;

		if (valor_sonido>127) valor_sonido=127;
		if (valor_sonido<-127) valor_sonido=-127;

		printf ("indice: %d sonido: %d valor_sonido: %d dividir: %d\n",audio_change_top_speed_index,sonido,valor_sonido,dividir);
	}

	*/
	
	

	
	 
	//
	// Multiplicar
	valor_sonido=valor_sonido*dividir;
	valor_sonido=valor_sonido/100;

	//Y bajamos un poco el volumen para que no sea tan molesto
	valor_sonido /=2;
	
	

	/*
	//
	// Mezclar
	valor_sonido=valor_sonido+dividir;
	valor_sonido /=2;
	*/



	//printf ("sonido: %d valor_sonido: %d dividir: %d\n",sonido,valor_sonido,dividir);

	sonido=valor_sonido;



	//Mover el indice el incremento correspondiente
	audio_change_top_speed_index+=(FRECUENCIA_CONSTANTE_NORMAL_SONIDO-FREQ_TOP_SPEED_CHANGE);

	if (audio_change_top_speed_index>=FREQ_TOP_SPEED_CHANGE) audio_change_top_speed_index -=FREQ_TOP_SPEED_CHANGE;

	return sonido;
}


void audio_send_mono_sample(char valor_sonido)
{

	int limite_buffer_audio;

	//if (audio_driver_accepts_stereo.v==0) {
	//	limite_buffer_audio=AUDIO_BUFFER_SIZE;
	//	audio_buffer[audio_buffer_indice]=valor_sonido;
	//	if (audio_buffer_indice<limite_buffer_audio-1) audio_buffer_indice++;
	//}


	//else {
		limite_buffer_audio=AUDIO_BUFFER_SIZE*2;

		

		audio_buffer[audio_buffer_indice]=valor_sonido;
		audio_buffer[audio_buffer_indice+1]=valor_sonido;

		if (audio_buffer_indice<limite_buffer_audio-2) audio_buffer_indice+=2;
	//}

}


void audio_send_stereo_sample(char valor_sonido_izquierdo,char valor_sonido_derecho)
{

	int limite_buffer_audio;

	//if (audio_driver_accepts_stereo.v==0) {
	//	limite_buffer_audio=AUDIO_BUFFER_SIZE;

	//	//Mezclar los dos canales en uno
	//	int suma=valor_sonido_izquierdo+valor_sonido_derecho;
	//	suma /=2;

	//	audio_buffer[audio_buffer_indice]=suma;
	//	if (audio_buffer_indice<limite_buffer_audio-1) audio_buffer_indice++;
	//}


	//else {
		limite_buffer_audio=AUDIO_BUFFER_SIZE*2;

		audio_buffer[audio_buffer_indice]=valor_sonido_izquierdo;
		audio_buffer[audio_buffer_indice+1]=valor_sonido_derecho;

		if (audio_buffer_indice<limite_buffer_audio-2) audio_buffer_indice+=2;
	//}

}


void audiodac_send_sample_value(z80_byte value)
{
	audiodac_last_value_data=value;
	silence_detection_counter=0;
}


//Funciones midi

//Convierte valor entero en variable length. Se devuelve en el orden tal cual tiene que ir en destino
//Devuelve longitud en bytes
int util_int_variable_length(unsigned int valor,z80_byte *destino)
{

    //Controlar rango maximo
    if (valor>0x0FFFFFFF) valor=0x0FFFFFFF;

    //Inicializarlo a cero
    //destino[0]=destino[1]=destino[2]=destino[3]=0;
    int longitud=1;

    //Lo metemos temporalmente ahi
    unsigned int final=0;

    final=valor & 0x7F;

    do {
        valor=valor>>7;
        if (valor) {
            final=final<<8;
            final |=(valor & 0x7f);
            final |=128;
            longitud++;
        }
    } while (valor);

    //Y lo metemos en buffer destino
    int i;
    for (i=0;i<longitud;i++) { //maximo 4 bytes, por si acaso
        z80_byte valor_leer=final & 0xFF;
        final=final>>8;

        destino[i]=valor_leer;
    }

    return longitud;


}




//Meter cabecera archivo mid. Retorna longitud en bytes
int mid_mete_cabecera(z80_byte *midi_file,int pistas,int division)
{

    //cabecera
    memcpy(midi_file,"MThd",4);

    

    //Valor 6
    midi_file[4]=0;
    midi_file[5]=0;
    midi_file[6]=0;
    midi_file[7]=6;

    //Formato
    midi_file[8]=0;
    midi_file[9]=1;

    //Pistas. This is a 16-bit binary number, MSB first.
    midi_file[10]=(pistas>>8) & 0xFF;
    midi_file[11]=pistas & 0xFF;   

 
    //Division. Ticks per quarter note (negra?)
    //int division=50; //96; //lo que dura la negra. hacemos 50 para 1/50s

    midi_file[12]=0x00;
    midi_file[13]=division;   

	return 14;
}

//Pone inicio pista. Retorna longitud bloque
//Posteriormente habra que poner longitud del bloque en (mem+4)
int mid_mete_inicio_pista(z80_byte *mem,int division)
{

      //Pista
    memcpy(mem,"MTrk",4);
    int indice=4;

    //int notas=7;

    //longitud eventos. meter cuando se finalice pista
    indice +=4;


    //Time signature
    //4 bytes; 4/4 time; 24 MIDI clocks/click, 8 32nd notes/ 24 MIDI clocks (24 MIDI clocks = 1 crotchet = 1 beat)
    //El 0 del principio es el deltatime
    unsigned char midi_clocks=0x18; //24=96/4

    midi_clocks=division/4;

	//un poco mas lento
	//midi_clocks /=4;

    unsigned char midi_time_signature[]={0x00,0xFF,0x58,0x04,0x04,0x02,midi_clocks,0x08};
    memcpy(&mem[indice],midi_time_signature,8);
    indice +=8;

    //Tempo
    //3 bytes: 500,000 usec/ quarter note = 120 beats/minute
    //El 0 del principio es el deltatime
	int tempo=500000;
	tempo*=2;



    unsigned char midi_tempo[]={0x00,0xFF,0x51,0x03,0x07,0xA1,0x20};
	midi_tempo[4]=(tempo>>16)&0xFF;
	midi_tempo[5]=(tempo>>8)&0xFF;
	midi_tempo[6]=(tempo)&0xFF;
    memcpy(&mem[indice],midi_tempo,7);
    indice +=7;

	//Y texto de ZEsarUX en cada pista
	//FF 03 : Sequence/Track Name
	char *midi_zesarux_message="\x00\xff\x03\x1b" "Created on ZEsarUX emulator"; //27=0x1B
	int longitud_bloque=27+4;

    memcpy(&mem[indice],midi_zesarux_message,longitud_bloque);
    indice +=longitud_bloque;	

	return indice;

}

//Devuelve longitud en bytes
int mid_mete_evento_final_pista(unsigned char *mem)
{

    int indice=0;

    //Evento al momento
    mem[indice++]=0;    


    mem[indice++]=0xFF;
    mem[indice++]=0x2F;
    mem[indice++]=0x00;

    return indice;

}

//Mete los 4 bytes que indican longitud de pista
//mem apunta a inicio de cabecera pista "MTrk"
//longitud es todo el bloque de pista, desde MTrk hasta despues del evento de final de pista
void mid_mete_longitud_pista(z80_byte *mem,int longitud)
{
	    //Meter longitud eventos
    int longitud_eventos=longitud-4-4; //evitar los 4 bytes que indican precisamente longitud y los 4 de "Mtrk"

	int puntero_longitud_pista=4;

    //longitud eventos. meter al final
    mem[puntero_longitud_pista++]=(longitud_eventos>>24) & 0xFF;
    mem[puntero_longitud_pista++]=(longitud_eventos>>16) & 0xFF;
    mem[puntero_longitud_pista++]=(longitud_eventos>>8) & 0xFF;
    mem[puntero_longitud_pista++]=(longitud_eventos  ) & 0xFF;    
}


//Mete nota mid. Devuelve longitud en bytes
int mid_mete_nota(z80_byte *mem,int silencio_anterior,int duracion,int canal_midi,int keynote,int velocity)
{

    int indice=0;

    unsigned int deltatime=duracion;



    //Evento note on. meter silencio anterior
    //mem[indice++]=0;
    indice +=util_int_variable_length(silencio_anterior,&mem[indice]);
   

    //int canal_midi=0;
    unsigned char noteonevent=(128+16) | (canal_midi & 0xf);


    mem[indice++]=noteonevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;



    //Evento note off
    int longitud_delta=util_int_variable_length(deltatime,&mem[indice]);
    indice +=longitud_delta;


    unsigned char noteoffevent=(128) | (canal_midi & 0xf);

    mem[indice++]=noteoffevent;
    mem[indice++]=keynote & 127;
    mem[indice++]=velocity & 127;

    return indice;
}



//Notas anteriores sonando, 3 canales
char mid_nota_sonando[MAX_AY_CHIPS*3][4];


int mid_nota_sonando_duracion[MAX_AY_CHIPS*3];


//Puntero a inicio pista de cada canal
int mid_inicio_pista[MAX_AY_CHIPS*3];

//Indice actual en cada buffer destino
int mid_indices_actuales[MAX_AY_CHIPS*3];


//Silencios acumulados en cada canal
int mid_silencios_acumulados[MAX_AY_CHIPS*3];



int mid_parm_division=50;


z80_byte *mid_memoria_export[MAX_AY_CHIPS*3];//3 canales


z80_bit mid_is_recording={0};

z80_bit mid_is_paused={0};

char mid_export_file[PATH_MAX];

int mid_chips_al_start=1;

//Para saber si se ha grabado algo, para menu
//int mid_record_at_least_one=0;

//Para estadistica
int mid_notes_recorded=0;

//Dice que ya se han finalizado las pistas, metiendo cabecera de final y longitud de pistas
z80_bit mid_flush_finished_tracks={0};


//Permite grabar canales que sean tono+ruido, en exportar a .mid
z80_bit mid_record_noisetone={0};


//Permite enviar canales que sean tono+ruido, en midi output
z80_bit midi_output_record_noisetone={0};

void mid_reset_export_buffers(void)
{
	//Poner todos bufferes a null para decir que no estan asignados
	int i;

	for (i=0;i<MAX_AY_CHIPS*3;i++) {
		mid_memoria_export[i]=NULL;
	}
}

//Dice de todos los bufferes de canales cuanto esta ocupado el que mas
int mid_max_buffer(void)
{

	//Poner todos bufferes a null para decir que no estan asignados
	int maximo=0;
	int i;

	int total_canales;

	total_canales=3*mid_chips_al_start; 

	for (i=0;i<total_canales;i++) {
		if (mid_indices_actuales[i]>maximo) maximo=mid_indices_actuales[i];
	}

	return maximo;

}


void mid_initialize_export(void)
{

	mid_chips_al_start=ay_retorna_numero_chips();
	mid_flush_finished_tracks.v=0;
	mid_notes_recorded=0;	

	int total_pistas=3*mid_chips_al_start;

			int canal;
			for (canal=0;canal<total_pistas;canal++) {


				//Metemos cabecera bloque
				int indice=0;


				//int pistas=3;

				//Cabecera archivo
				//indice +=mid_mete_cabecera(&mid_memoria_export[canal][indice],pistas,division);

				//Si esta NULL, asignar
				if (mid_memoria_export[canal]==NULL) {

					mid_memoria_export[canal]=malloc(MAX_MID_EXPORT_BUFFER);

					if (mid_memoria_export[canal]==NULL) cpu_panic("Can not allocate mid export buffer");

				}


				//Inicio pista 
				mid_inicio_pista[canal]=indice; //TODO: esto es 0 siempre

				indice +=mid_mete_inicio_pista(&mid_memoria_export[canal][indice],mid_parm_division);
				mid_indices_actuales[canal]=indice;

				mid_silencios_acumulados[canal]=0;

				//Al principio decimos que hay un silencio sonando. De 1/50 s
				mid_nota_sonando[canal][0]=0;

				mid_nota_sonando_duracion[canal]=1;

				//Decir que no ha sonado aun ninguna noda
				//mid_record_at_least_one=0;

				
			}


}

int mid_has_been_initialized(void)
{
	//Solo con que el primer buffer apunte a algun sitio
	if (mid_memoria_export[0]!=NULL) return 1;
	else return 0;
}


void mid_export_put_note(int canal,char *nota,int duracion)
{
	//Si era silencio
	if (nota[0]==0) {
		//Acumular silencio para siguiente nota y volver
		mid_silencios_acumulados[canal]=duracion;
		return;
	}


	//Si no habia sonado nada aun, no meter silencio acumulado, resetearlos todos a cero
	//Para no grabar silencios del principio
	//Esto NO va bien
	/*if (!mid_notes_recorded) {
			int total_pistas=3*mid_chips_al_start;

			int canal;
			for (canal=0;canal<total_pistas;canal++) {	
				//Acumulados tienen que ser todos iguales
				//printf ("acumulado %d\n",mid_silencios_acumulados[canal]);
				mid_silencios_acumulados[canal]=0;
			}	
	}*/


	mid_notes_recorded++;

	//Leer indice actual
	int indice=mid_indices_actuales[canal];

	//Comprobar si se acerca al final del buffer, en ese caso error y dejar de grabar
	if (indice>=MAX_MID_EXPORT_BUFFER-128) {
		mid_is_recording.v=0;
		debug_printf (VERBOSE_ERR,"Error exporting to MID. Memory buffer full. Stopping recording");
		return;
	}


	//Obtener nota
	int nota_numero=get_mid_number_note(nota);

	if (nota_numero<0) {
		//Nota invalida. no se deberia llegar aqui nunca
		debug_printf (VERBOSE_DEBUG,"Invalid note %s",nota);
		return;
	}

	debug_printf (VERBOSE_DEBUG,"Adding note %d in channel %d silence before %d length %d",nota_numero,canal,mid_silencios_acumulados[canal],duracion);

	indice +=mid_mete_nota(&mid_memoria_export[canal][indice],mid_silencios_acumulados[canal],duracion,canal,nota_numero,0x40);


	//Ya no hay silencio acumulado
	mid_silencios_acumulados[canal]=0;	


	//Guardar indice 
	mid_indices_actuales[canal]=indice;	
	
}

//Cierra pistas y graba a disco
void mid_flush_file(void)
{

	//if (temp_desactivado_mid) return ;

	//Cerrar pistas
	int canal;

	if (mid_flush_finished_tracks.v==0) {

		//printf ("Cerrando pistas\n");

		mid_flush_finished_tracks.v=1;

		for (canal=0;canal<3*mid_chips_al_start;canal++) {
			int indice=mid_indices_actuales[canal];			
			//Final de pista
			indice +=mid_mete_evento_final_pista(&mid_memoria_export[canal][indice]);

			int inicio_pista=mid_inicio_pista[canal];

			//Indicar longitud de pista
			int longitud_pista=indice-inicio_pista;

			mid_mete_longitud_pista(&mid_memoria_export[canal][inicio_pista],longitud_pista);	

			//Guardar indice 
			mid_indices_actuales[canal]=indice;			

		}


	}


	//Generar cabecera
	z80_byte cabecera_midi[256];

	//Escribir todas las pistas
	int pistas=3*mid_chips_al_start;

	//Cabecera archivo
	int division=mid_parm_division;
	int longitud_cabecera=mid_mete_cabecera(cabecera_midi,pistas,division);

	//Abrir archivo. Grabar cabecera y grabar las 3 pistas


	//Grabar a disco
FILE *ptr_midfile;

     ptr_midfile=fopen(mid_export_file,"wb");
     if (!ptr_midfile) {
                        debug_printf(VERBOSE_ERR,"Can not write midi file");
                        return;
      }

	//la cabecera
    fwrite(cabecera_midi, 1, longitud_cabecera, ptr_midfile);




	//cada pista
	for (canal=0;canal<pistas;canal++) {
		int longitud_pista=mid_indices_actuales[canal];
		debug_printf (VERBOSE_DEBUG,"Writing Channel %d length %d",canal,longitud_pista);
		fwrite(mid_memoria_export[canal], 1, longitud_pista, ptr_midfile);
	}


      fclose(ptr_midfile);		

}

//Evento de frame
void mid_frame_event(void)
{

	if (mid_is_recording.v==1 && mid_is_paused.v==0) {





		int chip;


			char nota[4];


		for (chip=0;chip<mid_chips_al_start;chip++) {
			int canal;
			for (canal=0;canal<3;canal++) {


				int freq=ay_retorna_frecuencia(canal,chip);


				sprintf(nota,"%s",get_note_name(freq) );

			
				
				//int reg_tono;
				int reg_vol;

				reg_vol=8+canal;

				int mascara_mezclador=1|8; 
				int valor_esperado_mezclador=8; //Esperamos por defecto no ruido (bit3 a 1) y tono (bit0 a 0)

				int valor_esperado_mezclador_tonoruido=0; //Canal con tono y ruido (bit3 a 0) y tono (bit0 a 0)


				/*
				1xx1 -> no tono ni ruido
				0xx1 -> ruido

				0xx0 -> ruido+tono
				1xx0 -> tono
				*/


				if (canal>0) {
					mascara_mezclador=mascara_mezclador<<canal;
					valor_esperado_mezclador=valor_esperado_mezclador<<canal;
				}


				//Si canales no suenan como tono, o volumen 0 meter cadena vacia en nota
				int suena_nota=0;


				if ( (ay_retorna_mixer_register(chip) &mascara_mezclador)==valor_esperado_mezclador) suena_nota=1; //Solo tono

				//Se permite tono y ruido?
				if (mid_record_noisetone.v) {
					if ( (ay_retorna_mixer_register(chip) &mascara_mezclador)==valor_esperado_mezclador_tonoruido) {
						suena_nota=1;
						//printf ("tonoruido\n");
					}
				}


				//Pero si no hay volumen, no hay nota
				if (ay_3_8912_registros[chip][reg_vol]==0) suena_nota=0;

				//if (!suena_nota) printf ("no suena\n");
				//else printf ("suena\n");

				if (!suena_nota) nota[0]=0;


				
				

				int canal_final=3*chip+canal;

				//Comparar si igual o anterior
				if (!strcasecmp(nota,mid_nota_sonando[canal_final])) {
					mid_nota_sonando_duracion[canal_final]++;
					//printf ("nota igual [%s] duracion [%d]\n",
					//nota,mid_nota_sonando_duracion[canal]);
				}
				else {
					
					//printf ("nota diferente canal %d. anterior [%s] duracion %d\n",canal_final,mid_nota_sonando[canal_final],mid_nota_sonando_duracion[canal_final]);


					//printf ("nota diferente canal %d. nueva [%s]\n",canal_final,nota);

					//Metemos nota
					mid_export_put_note(canal_final,mid_nota_sonando[canal_final],mid_nota_sonando_duracion[canal_final]);


					mid_nota_sonando_duracion[canal_final]=1;
					strcpy(mid_nota_sonando[canal_final],nota);
				}
			}
			
		}

	}


	
}


void midi_output_frame_event(void)
{

	audio_midi_output_frame_event();
}


enum AUDIO_MIDI_RAW_PARSER_STATUS {
	MIDI_STATUS_UNKNOWN,
	MIDI_STATUS_RECEIVING_DATA
	/*,
	MIDI_STATUS_RECEIVED_DATA*/
};

//Estado inicial
enum AUDIO_MIDI_RAW_PARSER_STATUS audio_midi_raw_parse_estado=MIDI_STATUS_UNKNOWN;

//Estado que lee desde el post
enum AUDIO_MIDI_RAW_PARSER_STATUS audio_midi_raw_parse_estado_next=MIDI_STATUS_UNKNOWN;


#define MAX_AUDIO_MIDI_RAW_PARSER_ARRAY 256

z80_byte audio_midi_raw_parse_array[MAX_AUDIO_MIDI_RAW_PARSER_ARRAY];

//Indice al array de bytes recibidos
int audio_midi_raw_parse_indice=0;


#define MAX_MIDI_STATUS_COMMAND_TEXT 128

void audio_midi_raw_get_status_name(z80_byte value,char *destination)
{
	//Retorna nombre para el status. Maximo MAX_MIDI_STATUS_COMMAND_TEXT
	if      ( (value & 0xF0) == 0x80 ) sprintf(destination,"Note off channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0x90 ) sprintf(destination,"Note on channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0xA0 ) sprintf(destination,"Key pressure channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0xB0 ) sprintf(destination,"Controller change channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0xC0 ) sprintf(destination,"Program change channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0xD0 ) sprintf(destination,"Channel pressure channel %d",value & 0xF);
	else if ( (value & 0xF0) == 0xE0 ) sprintf(destination,"Pitch blend channel %d",value & 0xF);

	else if (  value         == 0xF0 ) strcpy(destination,"System Exclusive");

	else if (  value         == 0xF2 ) strcpy(destination,"Song Position");
	else if (  value         == 0xF3 ) strcpy(destination,"Song Select");

	else if (  value         == 0xF5 ) strcpy(destination,"Unofficial Bus Select");
	else if (  value         == 0xF6 ) strcpy(destination,"Tune Request");
	else if (  value         == 0xF7 ) strcpy(destination,"End of System Exclusive");
	else if (  value         == 0xF8 ) strcpy(destination,"Timing Trick");

	else if (  value         == 0xFA ) strcpy(destination,"Start Song");
	else if (  value         == 0xFB ) strcpy(destination,"Continue Song");
	else if (  value         == 0xFC ) strcpy(destination,"Stop Song");

	else if (  value         == 0xFE ) strcpy(destination,"Active Sensing");
	else if (  value         == 0xFF ) strcpy(destination,"System Reset");


	else sprintf(destination,"Unknown status byte: %02XH",value);
}

//Parsear byte midi y saber si es de status o data, llevar conteo de mensajes finalizados etc
//Retorna el siguiente estado que tiene que procesar el _post
void audio_midi_raw_parse_value(z80_byte value)
{
	if (value & 128) {
		//printf ("audio_midi_raw_parse_value: is a status byte\n");

		
		switch (audio_midi_raw_parse_estado) {
			//Si estamos en estado desconocido, pasar a MIDI_STATUS_RECEIVING_DATA
			case MIDI_STATUS_UNKNOWN:
				//printf ("Pasando a MIDI_STATUS_RECEIVING_DATA\n");
				audio_midi_raw_parse_indice=0;
				audio_midi_raw_parse_array[audio_midi_raw_parse_indice++]=value;

				audio_midi_raw_parse_estado=MIDI_STATUS_RECEIVING_DATA;
			break;

			case MIDI_STATUS_RECEIVING_DATA:
				//Recibimos byte de status mientras recibiamos datos. Finalizar
				//printf ("Received status byte while receiving data. Starting a new command. Previous command lenght: %d\n",audio_midi_raw_parse_indice);


				/* 
				//Todo esto solo es debug
				char buf_status_mensaje[MAX_MIDI_STATUS_COMMAND_TEXT];

				audio_midi_raw_get_status_name(audio_midi_raw_parse_array[0],buf_status_mensaje);


				//Meter data bytes en una string para escribirlo luego
				//3 caracteres por cada byte. Y 1 de final de string
				char buf_data_mensaje[MAX_AUDIO_MIDI_RAW_PARSER_ARRAY*3+1];

				int i;
				int destino_string=0;
				for (i=1;i<audio_midi_raw_parse_indice;i++) {
					sprintf(&buf_data_mensaje[destino_string],"%02X ",audio_midi_raw_parse_array[i]);

					destino_string +=3;
				}
				printf ("Previous message was: [%s] Data: [%s]\n",buf_status_mensaje,buf_data_mensaje);

				//Debug en el caso de noteon/off

				if ( (audio_midi_raw_parse_array[0] & 0xF0) == 0x80 || 
				     (audio_midi_raw_parse_array[0] & 0xF0) == 0x90
					 ) {
					int i;
					for (i=1;i<audio_midi_raw_parse_indice;i+=2) {
						printf ("Notes: %s\n",get_note_name_by_index(audio_midi_raw_parse_array[i]));
					}
				}

				//Fin Debug
				*/

				audio_midi_raw_parse_indice=0;
				audio_midi_raw_parse_array[audio_midi_raw_parse_indice++]=value;

				audio_midi_raw_parse_estado=MIDI_STATUS_RECEIVING_DATA;
			break;

			/*case MIDI_STATUS_RECEIVED_DATA:
				//esto se actuara en el post
			break;*/
		}
	}

	else {
		//printf ("audio_midi_raw_parse_value: is a data byte\n");

		switch (audio_midi_raw_parse_estado) {
			case MIDI_STATUS_UNKNOWN:
				//printf ("Receiving a data byte while in unknown state. Discarding\n");
			break;

			// Pues seguimos recibiendo datos
			case MIDI_STATUS_RECEIVING_DATA:
				//printf ("Receiving a data byte while in receiving state. Adding it\n");

				if (audio_midi_raw_parse_indice!=MAX_AUDIO_MIDI_RAW_PARSER_ARRAY) {
					audio_midi_raw_parse_array[audio_midi_raw_parse_indice++]=value;
				}

				else {
					//printf ("Reached the end of audio_midi_raw_parse_array. Not adding it!\n");
				}
			break;

			/*case MIDI_STATUS_RECEIVED_DATA:

			break;*/
		}		

	}
}



void audio_midi_output_raw(z80_byte value)
{


	if (audio_midi_output_initialized==0) return;

	

#ifdef COMPILE_COREAUDIO
		coreaudio_mid_raw_send(value);
		
#endif

#ifdef COMPILE_ALSA
alsa_midi_raw(value);
#endif


#ifdef MINGW
//De momento no va


windows_midi_raw(value);


audio_midi_raw_parse_value(value);
#endif

	


}

int audio_midi_output_note_on(unsigned char channel, unsigned char note)
{
	#ifdef COMPILE_ALSA
	return alsa_note_on(channel,note,ALSA_MID_VELOCITY);
	#endif


	#ifdef COMPILE_COREAUDIO
	return coreaudio_note_on(channel,note,127);
	#endif

	#ifdef MINGW
	return windows_note_on(channel,note,127);
	#endif		
}

int audio_midi_output_note_off(unsigned char channel, unsigned char note)
{
	#ifdef COMPILE_ALSA
	return alsa_note_off(channel,note,ALSA_MID_VELOCITY);
	#endif


	#ifdef COMPILE_COREAUDIO
	return coreaudio_note_off(channel,note,127);
	#endif	

	#ifdef MINGW
	return windows_note_off(channel,note,127);
	#endif		
}



void audio_midi_output_flush_output(void)
{
	#ifdef COMPILE_ALSA
	alsa_midi_output_flush_output();
	#endif

	#ifdef COMPILE_COREAUDIO
	coreaudio_midi_output_flush_output();
	#endif	

	#ifdef MINGW
	windows_midi_output_flush_output();
	#endif
}


void audio_midi_output_reset(void)
{
	#ifdef COMPILE_ALSA
	alsa_midi_output_reset();
	#endif

	#ifdef COMPILE_COREAUDIO
	coreaudio_midi_output_reset();
	#endif	

	#ifdef MINGW
	windows_midi_output_reset();
	#endif	
}

//Notas anteriores sonando, 3 canales
char midi_output_nota_sonando[MAX_AY_CHIPS*3][4];

//Puerto y cliente, para diferentes drivers
int audio_midi_client=0;
int audio_midi_port=0;
//Client solo es usado por alsa
//Port lo utilizan alsa y windows 

char audio_raw_midi_device_out[MAX_AUDIO_RAW_MIDI_DEVICE_OUT]="hw:0,0";

//de momento usado en Linux
int audio_midi_raw_mode=1;


void audio_midi_output_finish(void)
{

	//Aqui no se puede entrar desde menu, pero si al finalizar ZEsarUX, y podria intentar activarse cuando no hay dichos drivers disponibles
	if (!audio_midi_available()) return;


	//No hay nada que finalizar?
	if (!audio_midi_output_initialized) return;

	debug_printf (VERBOSE_DEBUG,"Closing midi output");

#ifdef COMPILE_ALSA
     
	alsa_mid_finish_all();

#endif

#ifdef COMPILE_COREAUDIO

	coreaudio_mid_finish_all();

#endif


#ifdef MINGW

	windows_mid_finish_all();

#endif	
}

//Dice si esta disponible midi, cuando es en windows, o con coreaudio compilado, o con alsa compilado
int audio_midi_available(void)
{
#ifdef COMPILE_ALSA
		return 1;
#endif

#ifdef COMPILE_COREAUDIO
		return 1;
#endif


#ifdef MINGW
		return 1;
#endif	

	//Cualquier otro caso, no disponible
	return 0;

}


//Devuelve 1 si error
int audio_midi_output_init(void)
{

	//Aqui no se puede entrar desde menu, pero si desde command line, y podria intentar activarse cuando no hay dichos drivers disponibles
	if (!audio_midi_available()) return 0;


	debug_printf (VERBOSE_DEBUG,"Initializing midi output");

#ifdef COMPILE_ALSA
     

	if (alsa_mid_initialize_all()) {
		return 1;
	}

#endif

#ifdef COMPILE_COREAUDIO
	if (coreaudio_mid_initialize_all()) {
		return 1;
	}
#endif


#ifdef MINGW
	if (windows_mid_initialize_all()) {
		return 1;
	}
#endif


	//printf ("Inicializado midi\n");
	audio_midi_output_initialized=1;



    int total_pistas=3*MAX_AY_CHIPS;

	//mid_chips_al_start=ay_retorna_numero_chips();

	int canal;
	for (canal=0;canal<total_pistas;canal++) {

                                //Al principio decimos que hay un silencio sonando
                                midi_output_nota_sonando[canal][0]=0;


	}


	return 0;



}

int audio_midi_output_initialized=0;

void audio_midi_output_beeper(char *nota_a)
{
	   audiobuffer_stats audiostats;
        audio_get_audiobuffer_stats(&audiostats);


        int frecuencia=audiostats.frecuencia;

		//printf ("frecuencia %d\n",frecuencia);



			int freq_a=frecuencia;

		//alteramos frecuencia para que no considere los 4 bits inferiores, para "redondear" un poco
		//freq_a &=(65535-7);


			//char nota_a[4];
			sprintf(nota_a,"%s",get_note_name(freq_a) );

			//Si no hay sonido, suele dar frecuencia 5 o menos
			if (freq_a<=5) nota_a[0]=0;
}

int contador_nota_igual_beeper=0;
char nota_beeper_anterior[4]="";

void audio_midi_output_frame_event(void)
{

	if (audio_midi_output_initialized==0) return;


		int chip;


		char nota[4];


		for (chip=0;chip<ay_retorna_numero_chips();chip++) {
			int canal;
			for (canal=0;canal<3;canal++) {


				int freq=ay_retorna_frecuencia(canal,chip);


				sprintf(nota,"%s",get_note_name(freq) );



				//int reg_tono;
				int reg_vol;

				reg_vol=8+canal;

				int mascara_mezclador=1|8;
				int valor_esperado_mezclador=8; //Esperamos por defecto no ruido (bit3 a 1) y tono (bit0 a 0)

				int valor_esperado_mezclador_tonoruido=0; //Canal con tono y ruido (bit3 a 0) y tono (bit0 a 0)

	

				/*
				1xx1 -> no tono ni ruido
				0xx1 -> ruido

				0xx0 -> ruido+tono
				1xx0 -> tono
				*/


				if (canal>0) {
					mascara_mezclador=mascara_mezclador<<canal;
					valor_esperado_mezclador=valor_esperado_mezclador<<canal;
				}


				//Si canales no suenan como tono, o volumen 0 meter cadena vacia en nota
				int suena_nota=0;


				if ( (ay_retorna_mixer_register(chip)&mascara_mezclador)==valor_esperado_mezclador) suena_nota=1; //Solo tono

				//Se permite tono y ruido?
				if (midi_output_record_noisetone.v) {
					if ( (ay_retorna_mixer_register(chip)&mascara_mezclador)==valor_esperado_mezclador_tonoruido) {
						suena_nota=1;
						//printf ("tonoruido\n");
					}
				}


				//Pero si no hay volumen, no hay nota
				if (ay_3_8912_registros[chip][reg_vol]==0) suena_nota=0;

				//if (!suena_nota) printf ("no suena\n");
				//else printf ("suena\n");

				if (!suena_nota) nota[0]=0;

				int canal_final=3*chip+canal;

				int nota_igual=0;

				if (!strcasecmp(nota,midi_output_nota_sonando[canal_final])) nota_igual=1;


				//temp probar beeper
				/*
				if (canal==0 && chip==0) {
					audio_midi_output_beeper(nota);
					printf ("nota %s\n",nota);
					strcpy(nota_beeper_anterior,nota);

					int canal_final=3*chip+canal;
					if (!strcasecmp(nota,nota_beeper_anterior)) {
						contador_nota_igual_beeper++;
					}
					else {
						contador_nota_igual_beeper=0;
					}

					nota_igual=1;

					if (contador_nota_igual_beeper>=5) {
						//decir cambio de nota si es que es diferente de la que sonaba
						if (strcasecmp(nota,midi_output_nota_sonando[canal_final])) {
							nota_igual=0;
							printf ("cambio nota %s %s\n",nota,midi_output_nota_sonando[canal_final]);
							contador_nota_igual_beeper=0;
						}
						else {
							printf ("misma nota que la anterior %s %s\n",nota,midi_output_nota_sonando[canal_final]);
						}
					}

					//Guardar nota actual como la anterior
					strcpy(midi_output_nota_sonando[canal_final],nota);

						
				}
				//fin temp probar beeper
				*/
				




				//Comparar si igual o anterior
				if (nota_igual) {
					//midi_output_nota_sonando_duracion[canal_final]++;
					//printf ("nota igual [%s] duracion [%d]\n",
					//nota,midi_output_nota_sonando_duracion[canal]);
				}
				else {

					//printf ("nota diferente canal %d. anterior [%s] duracion %d\n",canal_final,midi_output_nota_sonando[canal_final],midi_output_nota_sonando_duracion[canal_final]);


					//printf ("nota diferente canal %d. nueva [%s]\n",canal_final,nota);

					//Metemos nota
					//Note off de la anterior y note on de la actual

					//note off si no era un silencio
					if (midi_output_nota_sonando[canal_final][0]!=0) {
					    int nota_numero=get_mid_number_note(midi_output_nota_sonando[canal_final]);

					    if (nota_numero<0) {
					        //Nota invalida. no se deberia llegar aqui nunca
					        debug_printf (VERBOSE_DEBUG,"Invalid note %s",midi_output_nota_sonando[canal_final]);
        				}	
						else audio_midi_output_note_off(canal_final,nota_numero);
					}


					//note on si no es un silencio
					if (nota[0]!=0) {
                    	int nota_numero=get_mid_number_note(nota);

                        if (nota_numero<0) {
                            //Nota invalida. no se deberia llegar aqui nunca
                            debug_printf (VERBOSE_DEBUG,"Invalid note %s",nota);
                        }
                        else audio_midi_output_note_on(canal_final,nota_numero);
                    }
	
					


					strcpy(midi_output_nota_sonando[canal_final],nota);
				}
			}

		}


	//Y enviar todos los eventos
	audio_midi_output_flush_output();
		

}


//Inicio rutinas Midi Windows. Esto quiza deberia estar en un archivo aparte, tipo "windows.c"
#ifdef MINGW


// Midi code derived from work of Craig Stuart Sapp:
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jan  1 20:29:45 PST 2005
// Last Modified: Sat Jan  1 20:37:28 PST 2005
// Filename:      ...midiio/doc/windowsmidi/keymidi/keymidi.c
// URL:           http://midiio.sapp.org/doc/windowsmidi/keymidi/keymidi.c
// Syntax:        C; Visual C/C++ 5/6
//
// Description:   The example program shows how to open MIDI output,
//                send a MIDI message, and close MIDI output.
//                When you press a key on the computer keyboard, a MIDI
//     

HMIDIOUT windows_midi_device;
//int windows_midi_midiport=0;

typedef union 
{ 
    DWORD word; 
	BYTE data[4]; 

    //unsigned int word; 
	//unsigned char data[4]; 

} windows_midi_message;

void windows_mid_add_note(windows_midi_message mensaje)
{

	//printf ("%d\n",mensaje.word);
   int flag = midiOutShortMsg(windows_midi_device, mensaje.word);
            //if (flag != MMSYSERR_NOERROR) {
            //printf("Warning: MIDI Output is not open.\n");
        // }

}



void windows_midi_output_flush_output(void)
{

   //nada

}


HMIDISTRM lphStream;

void other_windows_mid_initialize_raw(void)
{
	
	//LPHMIDISTRM lphStream;


//LPUINT puDeviceID;
unsigned int puDeviceID;
DWORD cMidi=1;
DWORD_PTR dwCallback=NULL;
DWORD_PTR dwInstance=NULL;
DWORD fdwOpen=CALLBACK_NULL;

puDeviceID=audio_midi_port; //que dispositivo???

//open in raw mode
MMRESULT resultado;
  resultado=midiStreamOpen(&lphStream,&puDeviceID,cMidi,dwCallback,dwInstance,fdwOpen);
  if (resultado!=MMSYSERR_NOERROR) {
char men[100];
    if (resultado==MMSYSERR_BADDEVICEID) strcpy(men,"BADDEVICEID");
    else if (resultado==MMSYSERR_INVALPARAM) strcpy(men,"INVALPARAM");
    else if (resultado==MMSYSERR_NOMEM) strcpy(men,"NOMEM");
    else sprintf(men,"error number: %d",resultado);

    debug_printf(VERBOSE_ERR,"Error opening MIDI in raw mode. %s",men);
    //return 1;
   }

midiStreamRestart(lphStream);
}

HMIDISTRM out;

void windows_mid_initialize_raw(void)
{
unsigned int device = 0;
midiStreamOpen(&out, &device, 1, NULL, 0, CALLBACK_NULL);

midiStreamRestart(out);
}

int windows_mid_initialize_all(void)
{
// Open the MIDI output port

   int flag = midiOutOpen(&windows_midi_device, audio_midi_port, 0, 0, CALLBACK_NULL);
   if (flag != MMSYSERR_NOERROR) {
      debug_printf(VERBOSE_ERR,"Error opening MIDI Output");
      return 1;
   }

//De momento nada de RAW en windows
//windows_mid_initialize_raw();

  return 0;
}

void windows_midi_output_reset(void)
{
  		windows_midi_message mensaje;

  		mensaje.data[0]=0xFF;
  		mensaje.data[1]=0;
  		mensaje.data[2]=0;
  		mensaje.data[3]=0;


				//Y envio a midi
				windows_mid_add_note(mensaje);	


	midiOutReset(windows_midi_device);				
}

void windows_mid_finish_all(void)
{
   // turn any MIDI notes currently playing:
   midiOutReset(windows_midi_device);

   // Remove any data in MIDI device and close the MIDI Output port
   midiOutClose(windows_midi_device);	
}

void test_windows_midi_raw(z80_byte value)
{


	MIDIHDR mhdr;
mhdr.lpData = &value;
mhdr.dwBufferLength = mhdr.dwBytesRecorded = 1;
mhdr.dwFlags = 0;
midiOutPrepareHeader((HMIDIOUT)out, &mhdr, sizeof(MIDIHDR));
}

int windows_midi_raw_indice=0;

int windows_midi_raw_multibyte=0;

void windows_midi_raw(z80_byte value)
{

//No me gusta para nada este código. Lo ideal es que Windows funcionase el envio raw de eventos midi,
//pero como no va, tengo que hacer este tipo de inventos....

	/*if ( (value & 128)==0) {
		printf ("windows midi raw: byte is not status. Not sending anything\n");
		return;
	} */

	//printf ("windows_midi_raw_indice %d audio_midi_raw_parse_indice %d\n",windows_midi_raw_indice,audio_midi_raw_parse_indice);

	//Si es menor indice actual, pasar a 0
	if (audio_midi_raw_parse_indice<windows_midi_raw_indice) windows_midi_raw_indice=0;

	//Estado anterior cual es?
	if (audio_midi_raw_parse_estado==MIDI_STATUS_UNKNOWN) {
		//printf ("windows midi raw: previous state is unknown. Not sending anything\n");
		return;
	}

	int enviar=0;



	//Si es multibyte, y hay dos mas para enviar
	if (windows_midi_raw_multibyte) {
		if (audio_midi_raw_parse_indice-windows_midi_raw_indice==2) {
			enviar=1;
			//printf ("Es multibyte y ha dos mas para enviar\n");
		}
	}

	//Si pasamos de 3 bytes y el actual no es status, es multibyte
	if (!windows_midi_raw_multibyte && (value & 128)==0) {
		if (audio_midi_raw_parse_indice-windows_midi_raw_indice==3) {
			windows_midi_raw_multibyte=1;
			enviar=1;
			//printf ("Son 3 bytes. Activamos multibyte\n");
		}
	}

	//Si hay bit 7 alzado, enviar lo que tengamos pendiente
	if ( (value & 128)) {
		enviar=1;
		windows_midi_raw_multibyte=0;
		//printf ("Es byte de status\n");
	}	

	///Enviar si hay al menos 3 bytes por enviar

//z80_byte audio_midi_raw_parse_array[MAX_AUDIO_MIDI_RAW_PARSER_ARRAY];

//Indice al array de bytes recibidos
//int audio_midi_raw_parse_indice=0;
	//printf ("windows midi raw: sending %d bytes of command\n",audio_midi_raw_parse_indice-windows_midi_raw_indice);

	/*
	  debug_printf (VERBOSE_PARANOID,"noteoff event channel %d note %d velocity %d",channel,note,velocity);



  windows_mid_add_note(mensaje);
	*/

	// Si longitud es 4 o menos, enviarlo tal cual
	//if (audio_midi_raw_parse_indice<=4) {
	if (enviar) {
			//primero inicializo todo a 0
  		windows_midi_message mensaje;

  		mensaje.data[0]=0;
  		mensaje.data[1]=0;
  		mensaje.data[2]=0;
  		mensaje.data[3]=0;

		  //Luego meto valores
		  int i;
		  int total=audio_midi_raw_parse_indice-windows_midi_raw_indice;

		  //printf ("Posible enviar total %d bytes a midi\n",total);

		  if (total>0) {
			//printf ("---Enviando total %d bytes a midi\n",total);

			int destino=0;
			for (i=windows_midi_raw_indice;i<windows_midi_raw_indice+total;i++) {
				mensaje.data[destino++]=audio_midi_raw_parse_array[i];
			}

				//Y envio a midi
				windows_mid_add_note(mensaje);

				windows_midi_raw_indice=audio_midi_raw_parse_indice;
		  }
	}

	else {
		//printf ("enviar=0\n");
	}

	/*else {
		//Si son mas , trocear
		//Ejemplo: Received status byte while receiving data. Starting a new command. Previous command lenght: 5
		//Previous message was: [Note on channel 0] Data: [3B 00 40 17 ]
		// noteon con 4 parámetros. Total 5 bytes
		// El señor microsoft dice que se envien en packs de 4 bytes, el segundo pack y siguientes tienen solo 2 bytes, el resto vacios

		//Enviamos primero 3
		windows_midi_message mensaje;

  		mensaje.data[0]=audio_midi_raw_parse_array[0];
  		mensaje.data[1]=audio_midi_raw_parse_array[1];
  		mensaje.data[2]=audio_midi_raw_parse_array[2];
  		mensaje.data[3]=0;

		//Y envio a midi
		windows_mid_add_note(mensaje);

		//Y ahora por cada dos

		int i;
		for (i=3;i<audio_midi_raw_parse_indice;i+=2) {
		  		mensaje.data[0]=audio_midi_raw_parse_array[i];
  				mensaje.data[1]=audio_midi_raw_parse_array[i+1];
  				mensaje.data[2]=0;
  				mensaje.data[3]=0;	

				//Y envio a midi
				windows_mid_add_note(mensaje);
		}

	}*/

}

void other_windows_midi_raw(z80_byte value)
{

MIDIHDR buffer;
//LPMIDIHDR buffer;

buffer.lpData=&value;
buffer.dwBufferLength=1;
buffer.dwFlags=0;
midiOutPrepareHeader((HMIDIOUT)lphStream,&buffer,sizeof(MIDIHDR));

midiStreamOut(lphStream,&buffer,sizeof(MIDIHDR));
printf("Enviando windows_midi raw value %02XH\n",value);

  /*windows_midi_message mensaje;

  mensaje.data[0]=value;
  mensaje.data[1]=0;
  mensaje.data[2]=0;
  mensaje.data[3]=0;


  windows_mid_add_note(mensaje);

  return ;*/

  //midiStreamOut(hms,pmh,1)


}


//Hacer note on de una nota inmediatamente
int windows_note_on(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteon event channel %d note %d velocity %d",channel,note,velocity);

  windows_midi_message mensaje;

  mensaje.data[0]=0x90;
  mensaje.data[1]=note;
  mensaje.data[2]=velocity;
  mensaje.data[3]=0;


  windows_mid_add_note(mensaje);

  return 0;
}

int windows_note_off(unsigned char channel, unsigned char note,unsigned char velocity)
{

  debug_printf (VERBOSE_PARANOID,"noteoff event channel %d note %d velocity %d",channel,note,velocity);

  windows_midi_message mensaje;

  mensaje.data[0]=0x80;
  mensaje.data[1]=note;
  mensaje.data[2]=velocity;
  mensaje.data[3]=0;


  windows_mid_add_note(mensaje);


  return 0;  
}


#endif
//Fin rutinas Midi Windows
