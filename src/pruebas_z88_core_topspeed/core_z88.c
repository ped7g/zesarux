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
#include "z88.h"
#include "chardetect.h"

#include "scrstdout.h"

z80_byte byte_leido_core_z88;

int tempcontafifty=0;

//contador que se incrementa al final de cada frame (5 ms)
//cuando llega a 3 (20 ms) se refresca pantalla y lee teclado.. y otras cosas que suceden cada 20ms (como envio de sonido)
int z88_5ms_contador=0;

int temp_veces_actualiza_teclas=0;

//int contador_sonido_3200hz=0;
int contador_z88_sonido_3200hz=0;

void z88_gestionar_tim(void)
{

	//Si esta en reset, no hacer nada
	if (blink_com&16) return;

	//printf ("z88_gestionar_tim\n");


	int signalTimeInterrupt=0;


                        //Gestion RTC.
                        z80_byte TIM0=blink_tim[0];
                        z80_byte TIM1=blink_tim[1];
			//Estos dos siguientes son int porque sumando aqui salen de rango 255
                        z80_int TIM2=blink_tim[2];
                        z80_int TIM3=blink_tim[3];
                        z80_byte TIM4=blink_tim[4];

                // fire a single interrupt for TSTA.TICK register,
                // but only if the flap is closed (the Blink doesn't emit
                // RTC interrupts while the flap is open, even if INT.TIME
                // is enabled)
                if ( (blink_sta & BM_STAFLAPOPEN) == 0 && ((blink_tmk & BM_TMKTICK) == BM_TMKTICK) && ((blink_tsta & BM_TSTATICK) == 0)) {
                    if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                        // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                        // TMK.TICK interrupts are enabled, signal that a tick occurred
                        blink_tsta |= BM_TSTATICK;
                        blink_sta |= BM_STATIME;

                        signalTimeInterrupt = 1;
                    }
                }

                if (TIM0 > 199) {
                    // When this counter reaches 200, a second has passed... (200 * 5 ms ticks = 1 sec)
                    TIM0 = 0;
                } else {
                    TIM0++;
                }

                if (TIM0 == 0x80) {
                    // According to blink dump monitoring on Z88, when TIM0 reaches 0x80, a second interrupt
                    // is signaled
                    ++TIM1;

                    // but only if the flap is closed (the Blink doesn't emit RTC interrupts while the flap is open,
                    // even if INT.TIME is enabled)
                    if ((blink_sta & BM_STAFLAPOPEN) == 0 && (blink_tmk & BM_TMKSEC) == BM_TMKSEC && ((blink_tsta & BM_TSTASEC) == 0)) {
                        // TMK.SEC interrupts are enabled, signal that a second occurred only if it not already signaled

                        if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                            // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                            blink_tsta |= BM_TSTASEC;
                            blink_sta |= BM_STATIME;

                            signalTimeInterrupt = 1;
                        }
                    }
                }

                if (TIM1 > 59) {
                    // 1 minute has passed
                    TIM1 = 0;

                    if ((blink_sta & BM_STAFLAPOPEN) == 0 && (blink_tmk & BM_TMKMIN) == BM_TMKMIN && ((blink_tsta & BM_TSTAMIN) == 0)) {
                        // TMK.MIN interrupts are enabled, signal that a minute occurred only if it not already signaled.
                        // but only if the flap is closed (the Blink doesn't emit RTC interrupts while the flap is open,
                        // even if INT.TIME is enabled)
                        if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                            // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                            blink_tsta |= BM_TSTAMIN;
                            blink_sta |= BM_STATIME;

                            signalTimeInterrupt = 1;
                        }
                    }

                    if (++TIM2 > 255) {
                        TIM2 = 0; // 256 minutes has passed
                        if (++TIM3 > 255) {
                            TIM3 = 0; // 65536 minutes has passed
                            if (++TIM4 > 31) {
                                TIM4 = 0; // 65536 * 32 minutes has passed
                            }
                        }
                    }
                }



                        blink_tim[0]=TIM0;
                        blink_tim[1]=TIM1;
                        blink_tim[2]=TIM2;
                        blink_tim[3]=TIM3;
                        blink_tim[4]=TIM4;

			//temp
			//debug_printf (VERBOSE_PARANOID,"registros RTC TIM: %d %d %d %d %d",blink_tim[0],blink_tim[1],blink_tim[2],blink_tim[3],blink_tim[4]);


                if (signalTimeInterrupt) {
			//printf ("reloj. despertar de snooze y coma\n");
                    z88_awake_from_snooze();
                    z88_awake_from_coma();

                    // Signal INT interrupt to Z80...
                    interrupcion_maskable_generada.v=1;
                }


}


//gestionar INT o NMI
void z88_gestionar_interrupcion(void)
{
                        //ver si esta en HALT
                        if (z80_ejecutando_halt.v) {
                                        z80_ejecutando_halt.v=0;
                                        reg_pc++;
					//z88_awake_from_coma();
                        //printf ("final de halt en refresca: %d max_cpu_cycles: %d\n",refresca,max_cpu_cycles);
                        }

			//z88_awake_from_snooze();


                         if (interrupcion_non_maskable_generada.v) {
						printf ("tratando interrupcion non maskable\n");
                                                interrupcion_non_maskable_generada.v=0;
                                                //printf ("Calling NMI with pc=0x%x\n",reg_pc);
                                                //call_address(0x66);
						iff1.v=0;
                                                push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT); 
                                                reg_pc=0x66; 


                                        }

                        else {

			//si estamos en DI, volver
	
			
			if (iff1.v==0) {
				if (interrupcion_maskable_generada.v) interrupcion_maskable_generada.v=0;
				//printf ("generada interrupcion pero DI. volvemos sin hacer nada\n");
				return;
			}
			
			


                                        //justo despues de EI no debe generar interrupcion
                                        //e interrupcion nmi tiene prioridad
                               if (interrupcion_maskable_generada.v && byte_leido_core_z88!=251) {
						printf ("tratando interrupcion maskable\n");
                                                //Tratar interrupciones maskable
                                                interrupcion_maskable_generada.v=0;


                                                z80_byte reg_pc_h,reg_pc_l;
                                                reg_pc_h=value_16_to_8h(reg_pc);
                                                reg_pc_l=value_16_to_8l(reg_pc);

                                                poke_byte(--reg_sp,reg_pc_h);
                                                poke_byte(--reg_sp,reg_pc_l);

                                                reg_r++;



                                                //desactivar interrupciones al generar una
                                                iff1.v=0;

                                                if (im_mode==0 || im_mode==1) {
                                                        reg_pc=56;
                                                        t_estados += 7;
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


//char temp_valor_z88_speaker=255;

//bucle principal de ejecucion de la cpu de spectrum
void cpu_core_loop_z88(void)
{

                debug_get_t_stados_parcial_post();
                debug_get_t_stados_parcial_pre();

  
		timer_check_interrupt();




		if (0==1) {
		}
		
		else {
			if (esperando_tiempo_final_t_estados.v==0) {


				if (z88_snooze.v==0) {
                                	contend_read( reg_pc, 4 );

					byte_leido_core_z88=fetch_opcode();

                	                reg_pc++;

					reg_r++;

	                		codsinpr[byte_leido_core_z88]  () ;

	printf ("t_estados: %d opcode %d mask: %d non mask: %d\n",t_estados,byte_leido_core_z88,interrupcion_maskable_generada.v,interrupcion_non_maskable_generada.v);
					
					
				}

				else {
					//printf ("modo snooze reg_pc=%d\n",reg_pc);
					//si esta en modo snooze o coma, no hacer mucho
					//contend_read( reg_pc, 4 );
					//lee NOP
					byte_leido_core_z88=0;

					//simulamos que es un nop pero con t_estados muy largo, para liberar la cpu bastante
					t_estados +=Z88_T_ESTADOS_COMA_SNOOZE;


				}

                                //si en modo snooze o coma, leer teclado para notificar
                                if (z88_snooze.v || z88_coma.v) {

                                        if (z88_return_keyboard_port_value(0)!=255) {
                                                //printf ("notificar tecla\n");
                                                z88_notificar_tecla();

                                                //printf ("tecla kbd A10: %d A11: %d\n", blink_kbd_a10, blink_kbd_a11);
                                        }

                                }


				//Si en modo coma, alargar los t_estados del halt, para liberar la cpu bastante
				if (z88_snooze.v==0 && z88_coma.v==1 && z80_ejecutando_halt.v) {
					t_estados +=Z88_T_ESTADOS_COMA_SNOOZE;
				}

				

                        }
                }



		//ejecutar esto al final de cada una de las scanlines (312)
		//esto implica que al final del frame de pantalla habremos enviado 312 bytes de sonido

		
		//A final de cada scanline 
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {
			//printf ("%d\n",t_estados);

			
				audio_buffer[audio_buffer_indice]=audio_valor_enviar_sonido;


				if (audio_buffer_indice<AUDIO_BUFFER_SIZE-1) audio_buffer_indice++;
				//else printf ("Overflow audio buffer: %d \n",audio_buffer_indice);
			
			//}
			


			//final de linea
			
			t_scanline_next_line();



			//se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
			//esperar para ver si se ha generado una interrupcion 1/50

                        if (t_estados>=screen_testados_total) {

				//printf ("total t_estados: %d\n",screen_testados_total);


		                 t_scanline=0;

					set_t_scanline_draw_zero();


                                t_estados -=screen_testados_total;

				if (z88_5ms_contador==3) {
					//printf ("z88_5ms_contador=%d\n",z88_5ms_contador);

					cpu_loop_refresca_pantalla();

					siguiente_frame_pantalla();


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


			}

		}

		if (esperando_tiempo_final_t_estados.v) {
			printf ("esperando final frame\n");
			timer_pause_waiting_end_frame();
		}




              //Interrupcion de 1/200s. mapa teclas activas y joystick
                if (interrupcion_fifty_generada.v) {
                        interrupcion_fifty_generada.v=0;

			if (z88_5ms_contador==3) {
	                        //y de momento actualizamos tablas de teclado segun tecla leida
				printf ("Actualizamos tablas teclado %d\n", temp_veces_actualiza_teclas++);
                	       scr_actualiza_tablas_teclado();

			}

                        z88_5ms_contador++;
                        if (z88_5ms_contador==4) z88_5ms_contador=0;

                        printf ("temp conta fifty: %d z88_5ms_contador: %d\n",tempcontafifty++,z88_5ms_contador);
				z88_gestionar_tim();
				printf ("registros RTC TIM: %d %d %d %d %d\n",blink_tim[0],blink_tim[1],blink_tim[2],blink_tim[3],blink_tim[4]);
                }


                //Interrupcion de procesador y marca final de frame
                if (interrupcion_timer_generada.v) {
			//printf ("generada interrupcion timer\n");
                        interrupcion_timer_generada.v=0;
                        esperando_tiempo_final_t_estados.v=0;
                        interlaced_numero_frame++;
			z88_contador_para_flap++;


			
			//Gestionar interrupciones al final del frame
			if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {
				printf ("Gestionar interrupcion Z80 mask: %d non mask: %d\n",interrupcion_maskable_generada.v,interrupcion_non_maskable_generada.v);
				z88_gestionar_interrupcion();
			}


                }


}



