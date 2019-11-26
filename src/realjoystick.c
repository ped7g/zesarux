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

/*

https://www.kernel.org/doc/Documentation/input/joystick-api.txt

		      Joystick API Documentation                -*-Text-*-

		        Ragnar Hojland Espinosa
			  <ragnar@macula.net>

			      7 Aug 1998

1. Initialization
~~~~~~~~~~~~~~~~~

Open the joystick device following the usual semantics (that is, with open).
Since the driver now reports events instead of polling for changes,
immediately after the open it will issue a series of synthetic events
(JS_EVENT_INIT) that you can read to check the initial state of the
joystick.

By default, the device is opened in blocking mode.

	int fd = open ("/dev/input/js0", O_RDONLY);


2. Event Reading
~~~~~~~~~~~~~~~~

	struct js_event e;
	read (fd, &e, sizeof(e));

where js_event is defined as

	struct js_event {
		__u32 time;     // event timestamp in milliseconds
		__s16 value;    // value
		__u8 type;      // event type
		__u8 number;    // axis/button number
	};

If the read is successful, it will return sizeof(e), unless you wanted to read
more than one event per read as described in section 3.1.


2.1 js_event.type
~~~~~~~~~~~~~~~~~

The possible values of ``type'' are

	#define JS_EVENT_BUTTON         0x01    // button pressed/released
	#define JS_EVENT_AXIS           0x02    // joystick moved
	#define JS_EVENT_INIT           0x80    // initial state of device

As mentioned above, the driver will issue synthetic JS_EVENT_INIT ORed
events on open. That is, if it's issuing a INIT BUTTON event, the
current type value will be

	int type = JS_EVENT_BUTTON | JS_EVENT_INIT;	// 0x81

If you choose not to differentiate between synthetic or real events
you can turn off the JS_EVENT_INIT bits

	type &= ~JS_EVENT_INIT;				// 0x01


2.2 js_event.number
~~~~~~~~~~~~~~~~~~~

The values of ``number'' correspond to the axis or button that
generated the event. Note that they carry separate numeration (that
is, you have both an axis 0 and a button 0). Generally,

			number
	1st Axis X	0
	1st Axis Y	1
	2nd Axis X	2
	2nd Axis Y	3
	...and so on

Hats vary from one joystick type to another. Some can be moved in 8
directions, some only in 4, The driver, however, always reports a hat as two
independent axis, even if the hardware doesn't allow independent movement.


2.3 js_event.value
~~~~~~~~~~~~~~~~~~

For an axis, ``value'' is a signed integer between -32767 and +32767
representing the position of the joystick along that axis. If you
don't read a 0 when the joystick is `dead', or if it doesn't span the
full range, you should recalibrate it (with, for example, jscal).

For a button, ``value'' for a press button event is 1 and for a release
button event is 0.

Though this

	if (js_event.type == JS_EVENT_BUTTON) {
		buttons_state ^= (1 << js_event.number);
	}

may work well if you handle JS_EVENT_INIT events separately,

	if ((js_event.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
		if (js_event.value)
			buttons_state |= (1 << js_event.number);
		else
			buttons_state &= ~(1 << js_event.number);
	}

is much safer since it can't lose sync with the driver. As you would
have to write a separate handler for JS_EVENT_INIT events in the first
snippet, this ends up being shorter.


2.4 js_event.time
~~~~~~~~~~~~~~~~~

The time an event was generated is stored in ``js_event.time''. It's a time
in milliseconds since ... well, since sometime in the past.  This eases the
task of detecting double clicks, figuring out if movement of axis and button
presses happened at the same time, and similar.


3. Reading
~~~~~~~~~~

If you open the device in blocking mode, a read will block (that is,
wait) forever until an event is generated and effectively read. There
are two alternatives if you can't afford to wait forever (which is,
admittedly, a long time;)

	a) use select to wait until there's data to be read on fd, or
	   until it timeouts. There's a good example on the select(2)
	   man page.

	b) open the device in non-blocking mode (O_NONBLOCK)


3.1 O_NONBLOCK
~~~~~~~~~~~~~~

If read returns -1 when reading in O_NONBLOCK mode, this isn't
necessarily a "real" error (check errno(3)); it can just mean there
are no events pending to be read on the driver queue. You should read
all events on the queue (that is, until you get a -1).

For example,

	while (1) {
		while (read (fd, &e, sizeof(e)) > 0) {
			process_event (e);
		}
		// EAGAIN is returned when the queue is empty
		if (errno != EAGAIN) {
			// error
		}
		// do something interesting with processed events
	}

One reason for emptying the queue is that if it gets full you'll start
missing events since the queue is finite, and older events will get
overwritten.

The other reason is that you want to know all what happened, and not
delay the processing till later.

Why can get the queue full? Because you don't empty the queue as
mentioned, or because too much time elapses from one read to another
and too many events to store in the queue get generated. Note that
high system load may contribute to space those reads even more.

If time between reads is enough to fill the queue and lose an event,
the driver will switch to startup mode and next time you read it,
synthetic events (JS_EVENT_INIT) will be generated to inform you of
the actual state of the joystick.

[As for version 1.2.8, the queue is circular and able to hold 64
 events. You can increment this size bumping up JS_BUFF_SIZE in
 joystick.h and recompiling the driver.]


In the above code, you might as well want to read more than one event
at a time using the typical read(2) functionality. For that, you would
replace the read above with something like

	struct js_event mybuffer[0xff];
	int i = read (fd, mybuffer, sizeof(mybuffer));

In this case, read would return -1 if the queue was empty, or some
other value in which the number of events read would be i /
sizeof(js_event)  Again, if the buffer was full, it's a good idea to
process the events and keep reading it until you empty the driver queue.


4. IOCTLs
~~~~~~~~~

The joystick driver defines the following ioctl(2) operations.

				// function			3rd arg
	#define JSIOCGAXES	// get number of axes		char
	#define JSIOCGBUTTONS	// get number of buttons	char
	#define JSIOCGVERSION	// get driver version		int
	#define JSIOCGNAME(len) // get identifier string	char
	#define JSIOCSCORR	// set correction values	&js_corr
	#define JSIOCGCORR	// get correction values	&js_corr

For example, to read the number of axes

	char number_of_axes;
	ioctl (fd, JSIOCGAXES, &number_of_axes);


4.1 JSIOGCVERSION
~~~~~~~~~~~~~~~~~

JSIOGCVERSION is a good way to check in run-time whether the running
driver is 1.0+ and supports the event interface. If it is not, the
IOCTL will fail. For a compile-time decision, you can test the
JS_VERSION symbol

	#ifdef JS_VERSION
	#if JS_VERSION > 0xsomething


4.2 JSIOCGNAME
~~~~~~~~~~~~~~

JSIOCGNAME(len) allows you to get the name string of the joystick - the same
as is being printed at boot time. The 'len' argument is the length of the
buffer provided by the application asking for the name. It is used to avoid
possible overrun should the name be too long.

	char name[128];
	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
		strncpy(name, "Unknown", sizeof(name));
	printf("Name: %s\n", name);


4.3 JSIOC[SG]CORR
~~~~~~~~~~~~~~~~~

For usage on JSIOC[SG]CORR I suggest you to look into jscal.c  They are
not needed in a normal program, only in joystick calibration software
such as jscal or kcmjoy. These IOCTLs and data types aren't considered
to be in the stable part of the API, and therefore may change without
warning in following releases of the driver.

Both JSIOCSCORR and JSIOCGCORR expect &js_corr to be able to hold
information for all axis. That is, struct js_corr corr[MAX_AXIS];

struct js_corr is defined as

	struct js_corr {
		__s32 coef[8];
		__u16 prec;
		__u16 type;
	};

and ``type''

	#define JS_CORR_NONE            0x00    // returns raw values
	#define JS_CORR_BROKEN          0x01    // broken line


5. Backward compatibility
~~~~~~~~~~~~~~~~~~~~~~~~~

The 0.x joystick driver API is quite limited and its usage is deprecated.
The driver offers backward compatibility, though. Here's a quick summary:

	struct JS_DATA_TYPE js;
	while (1) {
		if (read (fd, &js, JS_RETURN) != JS_RETURN) {
			// error
		}
		usleep (1000);
	}

As you can figure out from the example, the read returns immediately,
with the actual state of the joystick.

	struct JS_DATA_TYPE {
		int buttons;    // immediate button state
		int x;          // immediate x axis value
		int y;          // immediate y axis value
	};

and JS_RETURN is defined as

	#define JS_RETURN       sizeof(struct JS_DATA_TYPE)

To test the state of the buttons,

	first_button_state  = js.buttons & 1;
	second_button_state = js.buttons & 2;

The axis values do not have a defined range in the original 0.x driver,
except for that the values are non-negative. The 1.2.8+ drivers use a
fixed range for reporting the values, 1 being the minimum, 128 the
center, and 255 maximum value.

The v0.8.0.2 driver also had an interface for 'digital joysticks', (now
called Multisystem joysticks in this driver), under /dev/djsX. This driver
doesn't try to be compatible with that interface.


6. Final Notes
~~~~~~~~~~~~~~

____/|	Comments, additions, and specially corrections are welcome.
\ o.O|	Documentation valid for at least version 1.2.8 of the joystick
 =(_)=	driver and as usual, the ultimate source for documentation is
   U	to "Use The Source Luke" or, at your convenience, Vojtech ;)

					- Ragnar
EOF


*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef MINGW
#include <sys/ioctl.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "realjoystick.h"
#include "cpu.h"
#include "debug.h"
#include "joystick.h"
#include "menu.h"
#include "utils.h"
#include "screen.h"
#include "compileoptions.h"





//#define STRING_DEV_JOYSTICK "/dev/input/js0"

char string_dev_joystick[1024]="/dev/input/js0";

int ptr_realjoystick;

z80_bit realjoystick_present={1};


//Si se desactiva por opcion de linea de comandos
z80_bit realjoystick_disabled={0};

//Si cada vez que se llama a smartload, se resetea tabla de botones a teclas
z80_bit realjoystick_clear_keys_on_smartload={0};


//si no usamos un joystick real sino un simulador. Solo para testing en desarrollo
int simulador_joystick=0;


//se activa evento del simulador con F8 en scrxwindows
//se puede forzar, por ejemplo, al redefinir tecla
//pero como al redefinir tecla, esta en un bucle esperando,
//se puede pulsar ahi F8 y se activara el forzado... y no hay que tenerlo forzado antes
int simulador_joystick_forzado=0;

//tecla que envia con la accion de numselect
//desde '0','1'.. hasta '9'.. y otras adicionales, como espacio
z80_byte realjoystick_numselect='1';

int realjoystick_hit()
{


    if (simulador_joystick==1) {
                if (simulador_joystick_forzado==1) {
			//simulador_joystick_forzado=0;
			return 1;
		}
		else return 0;
    }

#ifndef MINGW
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(ptr_realjoystick, &fds);
	return select(ptr_realjoystick+1, &fds, NULL, NULL, &tv);
#else
	//Para windows retornar siempre 0
	//aunque aqui no llegara, solo para que no se queje el compilador
	return 0;
#endif
}






//asignacion de botones de joystick a funciones
//posicion 0 del array es para REALJOYSTICK_EVENT_UP
//posicion 1 del array es para REALJOYSTICK_EVENT_DOWN
//etc...
realjoystick_events_keys_function realjoystick_events_array[MAX_EVENTS_JOYSTICK];

//asignacion de botones de joystick a letras de teclado
//la posicion dentro del array no indica nada
realjoystick_events_keys_function realjoystick_keys_array[MAX_KEYS_JOYSTICK];

char *realjoystick_event_names[]={
    "Up",
    "Down",
    "Left",
    "Right",
    "Fire",
    "EscMenu",
    "Enter",
    "MenuTab",
    "Smartload",
	"Quicksave",
	"Osdkeyboard",
	"Osdtextkb",
	"NumSelect",
	"NumAction",
	"JoySelect",
	"Aux1",
	"Aux2",
	"Aux3",
	"Aux4"
};

void realjoystick_print_event_keys(void)
{
	int i;

	for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
		printf ("%s ",realjoystick_event_names[i]);
	}
}

//Retorna numero de evento para texto evento indicado, sin tener en cuenta mayusculas
//-1 si no encontrado
int realjoystick_get_event_string(char *texto)
{

        int i;

        for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
		if (!strcasecmp(texto,realjoystick_event_names[i])) {
			debug_printf (VERBOSE_DEBUG,"Event %s has event number: %d",texto,i);
			return i;
		}
        }


	//error no encontrado
	debug_printf (VERBOSE_DEBUG,"Event %s unknown",texto);
	return -1;
}

//Retorna numero de boton y su tipo para texto dado
//tipo de boton: 0-boton normal, +1 axis positivo, -1 axis negativo
void realjoystick_get_button_string(char *texto, int *button,int *button_type)
{
	//Ver si hay signo
	if (texto[0]!='+' && texto[0]!='-') {
		*button=parse_string_to_number(texto);
		*button_type=0;
		debug_printf (VERBOSE_DEBUG,"Button/Axis %s is button number %d",texto,*button);
		return;
	}

	if (texto[0]=='+') *button_type=+1;
	else *button_type=-1;

	*button=parse_string_to_number(&texto[1]);

	debug_printf (VERBOSE_DEBUG,"Button/Axis %s is axis number %d and sign %d",texto,*button,*button_type);
}


void realjoystick_clear_keys_array(void)
{

	debug_printf (VERBOSE_INFO,"Clearing joystick to keys table");

	int i;
        for (i=0;i<MAX_KEYS_JOYSTICK;i++) {
                realjoystick_keys_array[i].asignado.v=0;
        }

}

void realjoystick_clear_events_array(void)
{

	debug_printf (VERBOSE_INFO,"Clearing joystick to events table");

        int i;
        for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
                realjoystick_events_array[i].asignado.v=0;
        }

}


void realjoystick_set_default_functions(void)
{
	//primero desasignamos todos
	//eventos
	realjoystick_clear_events_array();

	//y teclas
	realjoystick_clear_keys_array();


	//y luego asignamos algunos por defecto
	//se pueden redefinir por linea de comandos previo a esto, o luego por el menu
/*

Botones asignados por defecto:
Arriba, Abajo, Izquierda, Derecha: direcciones
Fire: Boton normal
EscMenu: Boton normal
Enter. Quickload: botones pequenyos
Numberselect, Numberaction: botones pequenyos
Aux1: Boton grande (left)
Osdkeyboard: Boton grande (right)
Aux2: desasignado

*/


	realjoystick_events_array[REALJOYSTICK_EVENT_UP].asignado.v=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_UP].button=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_UP].button_type=-1;

	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].asignado.v=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].button=1;
	realjoystick_events_array[REALJOYSTICK_EVENT_DOWN].button_type=+1;

        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_LEFT].button_type=-1;

        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_RIGHT].button_type=+1;

        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].button=3;
        realjoystick_events_array[REALJOYSTICK_EVENT_FIRE].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].button=0;
        realjoystick_events_array[REALJOYSTICK_EVENT_ESC_MENU].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].button=8;
        realjoystick_events_array[REALJOYSTICK_EVENT_ENTER].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button=9;
        realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button_type=0;
	//normalmente las acciones no de axis van asociadas a botones... pero tambien pueden ser a axis
	//realjoystick_events_array[REALJOYSTICK_EVENT_QUICKLOAD].button_type=-1;

        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].button=5;
        realjoystick_events_array[REALJOYSTICK_EVENT_OSDKEYBOARD].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].button=2;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERSELECT].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].button=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_NUMBERACTION].button_type=0;

        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].asignado.v=1;
        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].button=4;
        realjoystick_events_array[REALJOYSTICK_EVENT_AUX1].button_type=0;

	//En mi joystick de test me faltan botones para poder asignar tambien este
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].asignado.v=1;
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].button=5;
        //realjoystick_events_array[REALJOYSTICK_EVENT_AUX2].button_type=0;


	//prueba
        //realjoystick_keys_array[0].asignado.v=1;
        //realjoystick_keys_array[0].button=20;
        //realjoystick_keys_array[0].button_type=0;
	//realjoystick_keys_array[0].caracter='a';



}

//retorna 0 si ok
//retorna 1 is no existe o error
int realjoystick_init(void)
{

	debug_printf(VERBOSE_INFO,"Initializing real joystick");

	if (simulador_joystick==1) {
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
		        "WARNING: using joystick simulator. Don't enable it on production version. Use F7 key to simulate joystick event\n"
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		sleep(4);
		return 0;
	}

#ifndef USE_LINUXREALJOYSTICK
	debug_printf(VERBOSE_INFO,"Linux real joystick support disabled on compilation");
	return 1;
#endif


#ifndef MINGW
	ptr_realjoystick=open(string_dev_joystick,O_RDONLY|O_NONBLOCK);
	if (ptr_realjoystick==-1) {
		debug_printf(VERBOSE_INFO,"Unable to open joystick %s : %s",string_dev_joystick,strerror(errno));
                return 1;
        }



	int flags;
	if((flags=fcntl(ptr_realjoystick,F_GETFL))==-1)
	  {
		  debug_printf(VERBOSE_ERR,"couldn't get flags from joystick device: %s",strerror(errno));
		  return 1;
	  }
	flags &= ~O_NONBLOCK;
	if(fcntl(ptr_realjoystick,F_SETFL,flags)==-1)
	  {
		  debug_printf(VERBOSE_ERR,"couldn't set joystick device non-blocking: %s",strerror(errno));
		  return 1;
	  }


	/*if (fcntl(ptr_realjoystick, F_SETFL, O_NONBLOCK)==-1)
          {
                  debug_printf(VERBOSE_ERR,"couldn't set joystick device non-blocking: %s",strerror(errno));
                  return 1;
          }
	*/


	realjoystick_present.v=1;

	return 0;
#endif

}

void read_simulador_joystick(int fd,struct js_event *e,int bytes)
{

	int value,type,number;

	printf ("button number: ");
	scanf ("%d",&number);

        printf ("button type: (%d=button, %d=axis)",JS_EVENT_BUTTON,JS_EVENT_AXIS);
        scanf ("%d",&type);

        printf ("button value: ");
        scanf ("%d",&value);

	e->number=number;
	e->type=type;
	e->value=value;

	simulador_joystick_forzado=0;

	//para que no salte warning de no usado
	fd++;
	bytes++;


}


//lectura de evento de joystick
//devuelve 0 si no hay nada
int realjoystick_read_event(int *button,int *type,int *value)
{
	//debug_printf(VERBOSE_INFO,"Reading joystick event");

	struct js_event e;

	if (!realjoystick_hit()) return 0;

	if (simulador_joystick==1) {
		read_simulador_joystick(ptr_realjoystick, &e, sizeof(e));
	}

	else {
		int leidos=read (ptr_realjoystick, &e, sizeof(e));
		if (leidos<0) {
			debug_printf (VERBOSE_ERR,"Error reading real joystick. Disabling it");
			realjoystick_present.v=0;
		}
	}

	debug_printf (VERBOSE_DEBUG,"event: time: %d value: %d type: %d number: %d",e.time,e.value,e.type,e.number);

	*button=e.number;
	*type=e.type;
	*value=e.value;


	int t=e.type;

	if ( (t&JS_EVENT_INIT)==JS_EVENT_INIT) {
		debug_printf (VERBOSE_DEBUG,"JS_EVENT_INIT");
		t=t&127;
	}

	if (t==JS_EVENT_BUTTON) debug_printf (VERBOSE_DEBUG,"JS_EVENT_BUTTON");

	if (t==JS_EVENT_AXIS) debug_printf (VERBOSE_DEBUG,"JS_EVENT_AXIS");

	/*
	Movimientos sobre mismo eje son asi:
	-boton 0: axis x. en negativo, hacia izquierda. en positivo, hacia derecha
	-boton 1: axis y. en negativo, hacia arriba. en positivo, hacia abajo

	Con botones normales, no ejes:
	-valor 1: indica boton pulsado
	-valor 0: indica boton liberado

	*/

	return 1;

}

//Retorna indice o -1 si no encontrado
int realjoystick_find_event_or_key(int indice_inicial,realjoystick_events_keys_function *tabla,int maximo,int button,int type,int value)
{
        int i;
        for (i=indice_inicial;i<maximo;i++) {
		if (tabla[i].asignado.v==1) {
			if (tabla[i].button==button) {

				//boton normal. no axis
				if (type==JS_EVENT_BUTTON && tabla[i].button_type==0) return i;


				if (type==JS_EVENT_AXIS) {

					//ver si coindice el axis
					if (tabla[i].button_type==+1) {
						if (value>0) return i;
					}

                                	if (tabla[i].button_type==-1) {
                                        	if (value<0) return i;
	                                }

					//si es 0, representara poner 0 en eje izquierdo y derecho por ejemplo

					if
					(
						(tabla[i].button_type==+1 || tabla[i].button_type==-1)
						&& value==0
					)
					return i;

					//if (value==0) return i;
				}
			}

		}
        }

	return -1;

}

int realjoystick_find_event(int indice_inicial,int button,int type,int value)
{
	return realjoystick_find_event_or_key(indice_inicial,realjoystick_events_array,MAX_EVENTS_JOYSTICK,button,type,value);
}

int realjoystick_find_key(int indice_inicial,int button,int type,int value)
{
        return realjoystick_find_event_or_key(indice_inicial,realjoystick_keys_array,MAX_KEYS_JOYSTICK,button,type,value);
}

//Busca evento en tabla segun numero de boton y tipo
int realjoystick_buscar_evento_en_tabla(int button, int button_type)
{
/*
	//numero de boton
	int button;

	//tipo de boton: 0-boton normal, +1 axis positivo, -1 axis negativo
	int button_type;
*/
//realjoystick_events_keys_function realjoystick_events_array[MAX_EVENTS_JOYSTICK];
	int i;

	for (i=0;i<MAX_EVENTS_JOYSTICK;i++) {
		if (realjoystick_events_array[i].asignado.v &&
			realjoystick_events_array[i].button==button && realjoystick_events_array[i].button_type==button_type) return i;
	}

	return -1;
}


void realjoystick_print_char(z80_byte caracter)
{

	//if (menu_footer==0) return;

	char buffer_mensaje[33];


	//si ya hay second layer (quiza fijo) no hacerlo por contador
	/*
	if (menu_footer==0) {
		menu_second_layer_counter=seconds;
		enable_second_layer();
		menu_overlay_activo=1;
	}
	*/

	sprintf (buffer_mensaje,"Key: %c",caracter);

	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


}

void realjoystick_print_joyselect(void)
{
	char buffer_mensaje[64];
	sprintf (buffer_mensaje,"Set joystick type: %s",joystick_texto[joystick_emulation]);
	screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,buffer_mensaje);


}

//si value=0, es reset
//si value != no, es set
void realjoystick_set_reset_key(int index,int value)
{

	z80_byte tecla=realjoystick_keys_array[index].caracter;

	//printf ("key: %c\n",tecla);

	if (value) {
		debug_printf (VERBOSE_DEBUG,"set key %c",tecla);
		ascii_to_keyboard_port(tecla);
	}


        else {
		debug_printf (VERBOSE_DEBUG,"reset key %c",tecla);
		ascii_to_keyboard_port_set_clear(tecla,0);	
	}

}

void realjoystick_send_f_function(int accion)
{
		//printf ("pulsada tecl de funcion\n");

      menu_button_f_function.v=1;
      menu_button_f_function_action=accion;
      menu_abierto=1;
	  /*

		//Entrada
		//menu_espera_no_tecla();
		osd_kb_no_mostrar_desde_menu=0; //Volver a permitir aparecer teclado osd

		//Procesar comandos F

		//menu_button_f_function.v=1;
      	//menu_button_f_function_index=F_FUNCION_QUICKSAVE;
      	menu_abierto=1;
		menu_process_f_functions_by_action(accion);

		menu_muestra_pending_error_message(); //Si se genera un error derivado de funcion F
		cls_menu_overlay();

		menu_abierto=0;
	*/
}

//si value=0, es reset
//si value != no, es set
void realjoystick_set_reset_action(int index,int value)
{

	switch (index) {
		case REALJOYSTICK_EVENT_UP:
			if (value) joystick_set_up(1);
			else joystick_release_up(1);
		break;

		case REALJOYSTICK_EVENT_DOWN:
			 if (value) joystick_set_down(1);
			 else joystick_release_down(1);
		break;

		case REALJOYSTICK_EVENT_LEFT:
			if (value) joystick_set_left(1);
			else joystick_release_left(1);
		break;

		case REALJOYSTICK_EVENT_RIGHT:
			if (value) joystick_set_right(1);
			else joystick_release_right(1);
		break;

		case REALJOYSTICK_EVENT_FIRE:
			if (value) joystick_set_fire(1);
			else joystick_release_fire(1);
		break;


		//Evento de ESC representa ESC para navegar entre menus y tambien abrir el menu (lo que ahora es con F5 y antes era ESC)
		//NO activa ESC de Z88
		case REALJOYSTICK_EVENT_ESC_MENU:
                                if (value) {
                                        puerto_especial1 &=255-1;
                                        if (util_if_open_just_menu() ) menu_fire_event_open_menu();
                                }
                                else {
                                        puerto_especial1 |=1;
                                }

		break;


                case REALJOYSTICK_EVENT_MENU_TAB:
			//printf ("Event menutab %d\n",value);
                                if (value) {
					menu_tab.v=1;
                                }
                                else {
                                        menu_tab.v=0;
                                }

                break;

		case REALJOYSTICK_EVENT_NUMBERSELECT:

			//cambiar valor solo cuando libera
			//lo hacemos asi porque en axis, aqui se envia muchas veces y saltaria demasiado rapido

			if (!value) {

				//Liberar tecla pulsada. Esto sirve para eventos asociados a botones de tipo axis,
				//si es que hemos puesto numselect y numaction a mismos botones de axis (uno + y otro -)
				//Dado que al liberar numaction, lo que interpreta es que numselect vale 0 y entra aqui
				//Aun asi con esto lo que provoca es que cuando se pulse numaction, se llame tambien a numselect de golpe
				//por lo general no se debian usar los botones de axis para mas que para direcciones
				ascii_to_keyboard_port_set_clear(realjoystick_numselect,0);


				if (realjoystick_numselect=='9') realjoystick_numselect=' ';
				else if (realjoystick_numselect==' ') realjoystick_numselect='0';

				else realjoystick_numselect++;
				debug_printf (VERBOSE_DEBUG,"numberselect: %d",realjoystick_numselect);
				realjoystick_print_char(realjoystick_numselect);

			}
		break;

		case REALJOYSTICK_EVENT_NUMBERACTION:

			if (value) {
				debug_printf (VERBOSE_DEBUG,"send key tecla %c",realjoystick_numselect);
				ascii_to_keyboard_port(realjoystick_numselect);
				realjoystick_print_char(realjoystick_numselect);
			}

			else {
				ascii_to_keyboard_port_set_clear(realjoystick_numselect,0);
			}
		break;


		case REALJOYSTICK_EVENT_JOYSELECT:
			if (value) {
				joystick_cycle_next_type();
				realjoystick_print_joyselect();
			}
		break;

		case REALJOYSTICK_EVENT_ENTER:

			if (value) puerto_49150 &=255-1;
                        else puerto_49150 |=1;
                break;


		case REALJOYSTICK_EVENT_QUICKLOAD:
			if (value) {
				menu_abierto=1;
				menu_button_quickload.v=1;
			}
		break;

		case REALJOYSTICK_EVENT_QUICKSAVE:

			if (value) {
				//printf ("boton quicksave\n");
				realjoystick_send_f_function(F_FUNCION_QUICKSAVE);
		      	//Activar funcion f en menu. Primera funcion que llamo desde joystick de la misma manera que cuando se pulsa tecla F de funcion
			}
		break;		

		case REALJOYSTICK_EVENT_OSDKEYBOARD:
				if (value) {
        	                        menu_abierto=1;
                	                menu_button_osdkeyboard.v=1;
	                        }
                break;

		case REALJOYSTICK_EVENT_OSD_TEXT_KEYBOARD:
				if (value) {
        	                        menu_abierto=1;
                	                menu_button_osd_adv_keyboard_openmenu.v=1;
	                        }
                break;


	}

}


//Variables leidas desde menu para el comprobador de joystick
int realjoystick_last_button,realjoystick_last_type,realjoystick_last_value,realjoystick_last_index;

//lectura de evento de joystick y conversion a movimiento de joystick spectrum
void realjoystick_main(void)
{


	if (realjoystick_present.v==0) return;

	int button,type,value;

	while (realjoystick_read_event(&button,&type,&value) ==1 && realjoystick_present.v) {
		//eventos de init no hacerles caso, de momento
		if ( (type&JS_EVENT_INIT)!=JS_EVENT_INIT) {

			realjoystick_last_button=button;
			realjoystick_last_type=type;
			realjoystick_last_value=value;
			

			//buscamos el evento
			int index=-1;
			do {
			index=realjoystick_find_event(index+1,button,type,value);
			//realjoystick_last_index=index;
			//printf ("last index: %d\n",realjoystick_last_index);
			if (index>=0) {
				debug_printf (VERBOSE_DEBUG,"Event found on index: %d",index);

				realjoystick_last_index=index;

				//ver tipo boton normal

				if (type==JS_EVENT_BUTTON) {
					realjoystick_set_reset_action(index,value);
				}


				//ver tipo axis
				if (type==JS_EVENT_AXIS) {
					switch (index) {
						case REALJOYSTICK_EVENT_UP:
							//reset abajo
							joystick_release_down(1);
							realjoystick_set_reset_action(index,value);
						break;

						case REALJOYSTICK_EVENT_DOWN:
								//reset arriba
								joystick_release_up(1);
								realjoystick_set_reset_action(index,value);
						break;

						case REALJOYSTICK_EVENT_LEFT:
								//reset derecha
								joystick_release_right(1);
								realjoystick_set_reset_action(index,value);
						break;

						case REALJOYSTICK_EVENT_RIGHT:
								//reset izquierda
								joystick_release_left(1);
								realjoystick_set_reset_action(index,value);
						break;



						default:
							//acciones que no son de axis
							realjoystick_set_reset_action(index,value);
						break;
					}
				}

					//gestionar si es 0, poner 0 en izquierda y derecha por ejemplo (solo para acciones de left/right/up/down)

					//si es >0, hacer que la accion de -1 se resetee (solo para acciones de left/right/up/down)
					//si es <0, hacer que la accion de +1 se resetee (solo para acciones de left/right/up/down)
			}
			} while (index>=0);

			//despues de evento, buscar boton a tecla
			//buscamos el evento
			index=-1;
			do {
                        index=realjoystick_find_key(index+1,button,type,value);
                        if (index>=0) {
                                debug_printf (VERBOSE_DEBUG,"Event found on index: %d. key=%c value:%d",index,realjoystick_keys_array[index].caracter,value);

                                //ver tipo boton normal o axis

                                if (type==JS_EVENT_BUTTON || type==JS_EVENT_AXIS) {
                                        realjoystick_set_reset_key(index,value);
                                }
			}
			} while (index>=0);



		}

	}

}


int realjoystick_find_if_already_defined_button(realjoystick_events_keys_function *tabla,int maximo,int button,int type)
{
	int i;
        for (i=0;i<maximo;i++) {
                if (tabla[i].asignado.v==1) {
			if (tabla[i].button==button && tabla[i].button_type==type) return i;
		}
	}
	return -1;
}



//redefinir evento
//Devuelve 1 si ok
//0 si salimos con tecla
int realjoystick_redefine_event_key(realjoystick_events_keys_function *tabla,int indice,int maximo)
{

	menu_espera_no_tecla();

        //leemos boton
        int button,type,value;

	debug_printf (VERBOSE_DEBUG,"redefine action: %d",indice);

	simulador_joystick_forzado=1;

	menu_espera_tecla_o_joystick();

	/*
	while (!realjoystick_hit()) {
		//sleep(1);
		menu_espera_tecla();
		//si se pulsa
	}
	*/

	simulador_joystick_forzado=1;

	//si no se ha pulsado joystick, pues se habra pulsado tecla
	if (!realjoystick_hit() ) {
		debug_printf (VERBOSE_DEBUG,"Pressed key, not joystick");
		return 0;
	}


	debug_printf (VERBOSE_DEBUG,"Pressed joystick");


        //while (realjoystick_read_event(&button,&type,&value) ==1 ) {
	if (realjoystick_read_event(&button,&type,&value) ==1 ) {
		debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);
                //eventos de init no hacerles caso, de momento
                if ( (type&JS_EVENT_INIT)!=JS_EVENT_INIT) {
			debug_printf (VERBOSE_DEBUG,"redefine for button: %d type: %d value: %d",button,type,value);

			int button_type=0;


                        if (type==JS_EVENT_BUTTON) {
                                //tabla[indice].button_type=0;
				button_type=0;
                        }

                        if (type==JS_EVENT_AXIS) {
                                //if (value<0) tabla[indice].button_type=-1;
                                //else tabla[indice].button_type=+1;
                                if (value<0) button_type=-1;
                                else button_type=+1;
                        }

			//Antes de asignarlo, ver que no exista uno antes
			//desasignamos primero el actual
			tabla[indice].asignado.v=0;

			int existe_evento=realjoystick_find_if_already_defined_button(tabla,maximo,button,button_type);

			if (existe_evento!=-1) {
				debug_printf (VERBOSE_ERR,"Button already mapped");
				return 0;
			}

                        tabla[indice].asignado.v=1;
			tabla[indice].button=button;
			tabla[indice].button_type=button_type;

                }


        }

	return 1;
}

//redefinir evento
//redefinir evento
//Devuelve 1 si ok
//0 si salimos con ESC
int realjoystick_redefine_event(int indice)
{
        return realjoystick_redefine_event_key(realjoystick_events_array,indice,MAX_EVENTS_JOYSTICK);
}



//redefinir tecla
//Devuelve 1 si ok
//0 si salimos con ESC
int realjoystick_redefine_key(int indice,z80_byte caracter)
{
	if (realjoystick_redefine_event_key(realjoystick_keys_array,indice,MAX_KEYS_JOYSTICK)) {
		realjoystick_keys_array[indice].caracter=caracter;
		return 1;
	}

	return 0;
}



//copiar boton asociado a un evento hacia boton-tecla
void realjoystick_copy_event_button_key(int indice_evento,int indice_tecla,z80_byte caracter)
{
	if (realjoystick_events_array[indice_evento].asignado.v) {
		debug_printf (VERBOSE_DEBUG,"Setting event %d to key %c on index %d (button %d button_type: %d)",
			indice_evento,caracter,indice_tecla,realjoystick_events_array[indice_evento].button,
			realjoystick_events_array[indice_evento].button_type);
		realjoystick_keys_array[indice_tecla].asignado.v=1;
		realjoystick_keys_array[indice_tecla].button=realjoystick_events_array[indice_evento].button;
		realjoystick_keys_array[indice_tecla].button_type=realjoystick_events_array[indice_evento].button_type;
		realjoystick_keys_array[indice_tecla].caracter=caracter;
	}

	else {
		debug_printf (VERBOSE_DEBUG,"On realjoystick_copy_event_button_key, event %d is not assigned",indice_evento);
	}
}

//Devuelve 0 si ok
int realjoystick_set_type(char *tipo) {

				debug_printf (VERBOSE_INFO,"Setting joystick type %s",tipo);

                                int i;
                                for (i=0;i<=JOYSTICK_TOTAL;i++) {
                                        if (!strcasecmp(tipo,joystick_texto[i])) break;
                                }
                                if (i>JOYSTICK_TOTAL) {
                                        debug_printf (VERBOSE_ERR,"Invalid joystick type %s",tipo);
					return 1;
                                }


                                joystick_emulation=i;
	return 0;
}


//Devuelve 0 si ok
int realjoystick_set_button_event(char *text_button, char *text_event)
{

				debug_printf (VERBOSE_INFO,"Setting button %s to event %s",text_button,text_event);

//--joystickevent but evt    Set a joystick button or axis to an event (changes joystick to event table)

                                //obtener boton
                                int button,button_type;
                                realjoystick_get_button_string(text_button,&button,&button_type);

                                //obtener evento
                                int evento=realjoystick_get_event_string(text_event);
                                if (evento==-1) {
					debug_printf (VERBOSE_ERR,"Unknown event %s",text_event);
					return 1;
                                }


                                //Y definir el evento
                                realjoystick_events_array[evento].asignado.v=1;
                                realjoystick_events_array[evento].button=button;
                                realjoystick_events_array[evento].button_type=button_type;

	return 0;
}



//Devuelve 0 si ok
int realjoystick_set_button_key(char *text_button,char *text_key)
{
				debug_printf (VERBOSE_INFO,"Setting button %s to key %s",text_button,text_key);

//"--joystickkeybt but key    Define a key pressed when a joystick button pressed (changes joystick to key table)\n"

                                //ver si maximo definido
                                if (joystickkey_definidas==MAX_KEYS_JOYSTICK) {
                                        debug_printf (VERBOSE_ERR,"Maximum defined joystick to keys defined (%d)",joystickkey_definidas);
                                        return 1;
                                }

                                //obtener boton
                                int button,button_type;
                                realjoystick_get_button_string(text_button,&button,&button_type);

                                z80_byte caracter=parse_string_to_number(text_key);


                                //realjoystick_copy_event_button_key(evento,joystickkey_definidas,caracter);
                                realjoystick_keys_array[joystickkey_definidas].asignado.v=1;
                                realjoystick_keys_array[joystickkey_definidas].button=button;
                                realjoystick_keys_array[joystickkey_definidas].button_type=button_type;
                                realjoystick_keys_array[joystickkey_definidas].caracter=caracter;

                                joystickkey_definidas++;


 return 0;
}


//Devuelve 0 si ok
int realjoystick_set_event_key(char *text_event,char *text_key)
{
				debug_printf (VERBOSE_INFO,"Setting event %s to key %s",text_event,text_key);

//    "--joystickkeyev evt key    Define a key pressed when a joystick event is generated (changes joystick to key table)\n"



                                //ver si maximo definido
                                if (joystickkey_definidas==MAX_KEYS_JOYSTICK) {
                                        debug_printf (VERBOSE_ERR,"Maximum defined joystick to keys defined (%d)",joystickkey_definidas);
                                        return 1;
                                }


                                //obtener evento

                                int evento=realjoystick_get_event_string(text_event);
                                if (evento==-1) {
                                        debug_printf (VERBOSE_ERR,"Unknown event %s",text_event);
                                        return 1;
                                }

                                //Y obtener tecla

                                z80_byte caracter=parse_string_to_number(text_key);

                                realjoystick_copy_event_button_key(evento,joystickkey_definidas,caracter);

                                joystickkey_definidas++;
	return 0;
}
