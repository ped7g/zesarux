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

#include <time.h>
#include <sys/time.h>
#include <errno.h>


#include "cpu.h"
#include "debug.h"
#include "tape.h"
#include "audio.h"
#include "screen.h"
#include "ay38912.h"
#include "operaciones.h"
#include "snap.h"
#include "timer.h"
#include "menu.h"
#include "compileoptions.h"
#include "contend.h"
#include "ula.h"
#include "utils.h"
#include "realjoystick.h"
#include "chardetect.h"

#include "scrstdout.h"
#include "cpc.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"

z80_byte byte_leido_core_cpc;




//bucle principal de ejecucion de la cpu de cpc
void cpu_core_loop_cpc(void)
{

                debug_get_t_stados_parcial_pre();

  
		timer_check_interrupt();


//#ifdef COMPILE_STDOUT 
//		if (screen_stdout_driver) scr_stdout_printchar();
//#endif
//
//#ifdef COMPILE_SIMPLETEXT
//                if (screen_simpletext_driver) scr_simpletext_printchar();
//#endif
		if (chardetect_detect_char_enabled.v) chardetect_detect_char();
		if (chardetect_printchar_enabled.v) chardetect_printchar();


		//Gestionar autoload
		gestionar_autoload_cpc();
		

		if (tap_load_detect()) {
				
        	                //si estamos en pausa, no hacer nada
                	        if (!tape_pause) {
					audio_playing.v=0;

					draw_tape_text();

					tap_load();
					all_interlace_scr_refresca_pantalla();

					//printf ("refresco pantalla\n");
					//audio_playing.v=1;
					timer_reset();
				}
			
				else {
					//core_cpc_store_rainbow_current_atributes();
					//generamos nada. como si fuera un NOP
					
					contend_read( reg_pc, 4 );
		

				}
		}

		else  if (tap_save_detect()) {
	               	        audio_playing.v=0;

				draw_tape_text();

                        	tap_save();
	                        //audio_playing.v=1;
				timer_reset();
               	}


		else {
			if (esperando_tiempo_final_t_estados.v==0) {

				//core_cpc_store_rainbow_current_atributes();

				//printf ("t-estados %d\n",t_estados);


#ifdef DEBUG_SECOND_TRAP_STDOUT

        //Para poder debugar rutina que imprima texto. Util para aventuras conversacionales 
        //hay que definir este DEBUG_SECOND_TRAP_STDOUT manualmente en compileoptions.h despues de ejecutar el configure

	scr_stdout_debug_print_char_routine();

#endif



        	                        contend_read( reg_pc, 4 );
					byte_leido_core_cpc=fetch_opcode();



#ifdef EMULATE_CPU_STATS
				util_stats_increment_counter(stats_codsinpr,byte_leido_core_cpc);
#endif

                                reg_pc++;

				reg_r++;

				//printf ("ejecutando opcode %d\n",byte_leido_core_cpc);

	                	codsinpr[byte_leido_core_cpc]  () ;

				//printf ("despues ejecucion opcode\n");


                        }
                }



		//ejecutar esto al final de cada una de las scanlines (312)
		//esto implica que al final del frame de pantalla habremos enviado 312 bytes de sonido


		
		//A final de cada scanline 
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {
			//printf ("%d\n",t_estados);
			//if (t_estados>69000) printf ("t_scanline casi final: %d\n",t_scanline);

			audio_valor_enviar_sonido=0;

			audio_valor_enviar_sonido +=da_output_ay();

			if (realtape_inserted.v && realtape_playing.v) {
				realtape_get_byte();
				if (realtape_loading_sound.v) {
                                audio_valor_enviar_sonido /=2;
                                audio_valor_enviar_sonido += realtape_last_value/2;
                                //Sonido alterado cuando top speed
                                if (timer_condicion_top_speed() ) audio_valor_enviar_sonido=audio_change_top_speed_sound(audio_valor_enviar_sonido);
				}
			}

			//Ajustar volumen
			if (audiovolume!=100) {
				audio_valor_enviar_sonido=audio_adjust_volume(audio_valor_enviar_sonido);
			}

			audio_send_mono_sample(audio_valor_enviar_sonido);


			ay_chip_siguiente_ciclo();



			//final de linea
			

			//copiamos contenido linea y border a buffer rainbow
			if (rainbow_enabled.v==1) {
				//screen_store_scanline_rainbow_solo_border();
				//screen_store_scanline_rainbow_solo_display();	

				//t_scanline_next_border();

			}

			t_scanline_next_line();

			cpc_handle_vsync_state();


			//CPC genera interrupciones a 300 hz
			//Esto supone lanzar 6  (50*6=300) interrupciones en cada frame
			//al final de un frame ya va una interrupcion
			//generar otras 5
			//tenemos unas 300 scanlines en cada pantalla
			//generamos otras 5 interrupciones en cada scanline: 50,100,150,200,250
			cpc_scanline_counter++;

			//Con ay player, interrupciones a 50 Hz
			if (cpc_scanline_counter>=52 && ay_player_playing.v==0) {
				if (iff1.v==1) {
					//printf ("Generamos interrupcion en scanline: %d cpc_scanline_counter: %d\n",t_scanline,cpc_scanline_counter);
                                       	interrupcion_maskable_generada.v=1;
				}
				cpc_scanline_counter=0;
			}



			//se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
			//esperar para ver si se ha generado una interrupcion 1/50

                        if (t_estados>=screen_testados_total) {



			//Con ay player, interrupciones a 50 Hz
                        if (ay_player_playing.v==1) {
                                if (iff1.v==1) {
                                        //printf ("Generamos interrupcion en scanline: %d cpc_scanline_counter: %d\n",t_scanline,cpc_scanline_counter);
                                        interrupcion_maskable_generada.v=1;
                                }
                        }



				//if (rainbow_enabled.v==1) t_scanline_next_fullborder();

		                t_scanline=0;

		                //printf ("final scan lines. total: %d\n",screen_scanlines);
                		        //printf ("reset no inves\n");
					set_t_scanline_draw_zero();


                                //Parche para maquinas que no generan 312 lineas, porque si enviamos menos sonido se escuchara un click al final
                                //Es necesario que cada frame de pantalla contenga 312 bytes de sonido
                                //Igualmente en la rutina de envio_audio se vuelve a comprobar que todo el sonido a enviar
                                //este completo; esto es necesario para Z88


                                int linea_estados=t_estados/screen_testados_linea;

                                while (linea_estados<312) {
										audio_send_mono_sample(audio_valor_enviar_sonido);
                                        linea_estados++;
                                }




                                t_estados -=screen_testados_total;

				//Para paperboy, thelosttapesofalbion0 y otros que hacen letras en el border, para que no se desplacen en diagonal
				//t_estados=0;
				//->paperboy queda fijo. thelosttapesofalbion0 no se desplaza, sino que tiembla si no forzamos esto



				//Final de instrucciones ejecutadas en un frame de pantalla. Esto no en cpc
				//if (iff1.v==1) {
				//	interrupcion_maskable_generada.v=1;
				//}


				cpu_loop_refresca_pantalla();

				vofile_send_frame(rainbow_buffer);	


				siguiente_frame_pantalla();


				if (debug_registers) scr_debug_registers();

	  	                contador_parpadeo--;
                        	//printf ("Parpadeo: %d estado: %d\n",contador_parpadeo,estado_parpadeo.v);
	                        if (!contador_parpadeo) {
        	                        contador_parpadeo=16;
                	                estado_parpadeo.v ^=1;
	                        }

			
				if (!interrupcion_timer_generada.v) {
					//Llegado a final de frame pero aun no ha llegado interrupcion de timer. Esperemos...
					//printf ("no demasiado\n");
					esperando_tiempo_final_t_estados.v=1;
				}

				else {
					//Llegado a final de frame y ya ha llegado interrupcion de timer. No esperamos.... Hemos tardado demasiado
					//printf ("demasiado\n");
					esperando_tiempo_final_t_estados.v=0;
				}

				core_end_frame_check_zrcp_zeng_snap.v=1;


			}

			//Bit de vsync
			//Duracion vsync
			/*z80_byte vsync_lenght=cpc_ppi_ports[3]&15;

			//Si es 0, en algunos chips significa 16
			if (vsync_lenght==0) vsync_lenght=16;
			//cpc_ppi_ports[1];
			if (t_scanline>=0 && t_scanline<=vsync_lenght-1) {
			//if (t_scanline>=0 && t_scanline<=7) {
				//printf ("Enviando vsync en linea %d\n",t_scanline);
				cpc_ppi_ports[1] |=1;
			}

			else {
				//printf ("NO enviando vsync en linea %d\n",t_scanline);
				cpc_ppi_ports[1] &=(255-1);
			}*/

		}

		if (esperando_tiempo_final_t_estados.v) {
			timer_pause_waiting_end_frame();
		}



              //Interrupcion de 1/50s. mapa teclas activas y joystick
                if (interrupcion_fifty_generada.v) {
                        interrupcion_fifty_generada.v=0;

                        //y de momento actualizamos tablas de teclado segun tecla leida
                        //printf ("Actualizamos tablas teclado %d ", temp_veces_actualiza_teclas++);
                       scr_actualiza_tablas_teclado();


                       //lectura de joystick
                       realjoystick_main();

                        //printf ("temp conta fifty: %d\n",tempcontafifty++);
                }


                //Interrupcion de procesador y marca final de frame
                if (interrupcion_timer_generada.v) {
                        interrupcion_timer_generada.v=0;
                        esperando_tiempo_final_t_estados.v=0;
                        interlaced_numero_frame++;
                        //printf ("%d\n",interlaced_numero_frame);
                }


		//Interrupcion de cpu. gestion im0/1/2. Esto se hace al final de cada frame en cpc o al cambio de bit6 de R en zx80/81
		if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {

			debug_fired_interrupt=1;

			//if (interrupcion_non_maskable_generada.v) printf ("generada nmi\n");

                        //if (interrupts.v==1) {   //esto ya no se mira. si se ha producido interrupcion es porque estaba en ei o es una NMI
                        //ver si esta en HALT
                        if (z80_ejecutando_halt.v) {
                                        z80_ejecutando_halt.v=0;
                                        reg_pc++;
                        }

			if (1==1) {

					if (interrupcion_non_maskable_generada.v) {
						debug_anota_retorno_step_nmi();
						//printf ("generada nmi\n");
                                                interrupcion_non_maskable_generada.v=0;


                                                //NMI wait 14 estados
                                                t_estados += 14;


                                                
												push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);


                                                reg_r++;
                                                iff1.v=0;
                                                //printf ("Calling NMI with pc=0x%x\n",reg_pc);

                                                //Otros 6 estados
                                                t_estados += 6;

                                                //Total NMI: NMI WAIT 14 estados + NMI CALL 12 estados
                                                reg_pc= 0x66;

                                                //temp

                                                t_estados -=15;

						//Prueba
						//Al recibir nmi tiene que poner paginacion normal. Luego ya saltara por autotrap de divmmc
						//if (divmmc_enabled.v) divmmc_paginacion_activa.v=0;


						
					}

					if (1==1) {
					//else {

						
					//justo despues de EI no debe generar interrupcion
					//e interrupcion nmi tiene prioridad
						if (interrupcion_maskable_generada.v && byte_leido_core_cpc!=251) {
						debug_anota_retorno_step_maskable();
						//Tratar interrupciones maskable
						interrupcion_maskable_generada.v=0;

						

						push_valor(reg_pc,PUSH_VALUE_TYPE_MASKABLE_INTERRUPT);

						reg_r++;

						//Caso Inves. Hacer poke (I*256+R) con 255
						if (MACHINE_IS_INVES) {
							//z80_byte reg_r_total=(reg_r&127) | (reg_r_bit7 &128);

							//Se usan solo los 7 bits bajos del registro R
							z80_byte reg_r_total=(reg_r&127);

							z80_int dir=reg_i*256+reg_r_total;

							poke_byte_no_time(dir,255);
						}
						
						
						//desactivar interrupciones al generar una
						iff1.v=iff2.v=0;
						//Modelos cpc

						if (im_mode==0 || im_mode==1) {
							cpu_common_jump_im01();
						}
						else {
						//IM 2.

							z80_int temp_i;
							z80_byte dir_l,dir_h;   
							temp_i=reg_i*256+255;
							dir_l=peek_byte(temp_i++);
							dir_h=peek_byte(temp_i);
							reg_pc=value_8_to_16(dir_h,dir_l);
							t_estados += 7;


						}
						
					}
				}


			}

    }
	//Fin gestion interrupciones

	//Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
	if (core_end_frame_check_zrcp_zeng_snap.v) {
		core_end_frame_check_zrcp_zeng_snap.v=0;
		check_pending_zrcp_put_snapshot();
		zeng_send_snapshot_if_needed();			
	}				
     
	debug_get_t_stados_parcial_post();

}




