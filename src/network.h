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

#ifndef NETWORK_H
#define NETWORK_H

#include "compileoptions.h"

#ifdef USE_PTHREADS
	#include <pthread.h>
#endif


#include <sys/types.h>

#ifdef MINGW
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

extern int enviar_cr;

extern int crear_socket_TCP(void);
extern int escribir_socket(int socket, char *buffer);
extern int omplir_adr_internet(struct sockaddr_in *adr,char *host,unsigned short n_port);
extern int leer_socket(int s, char *buffer, int longitud);
extern void escribir_socket_format (int misocket, const char * format , ...);
extern int assignar_adr_internet(int sock,char *host,unsigned short n_port);
extern int connectar_socket(int s,struct sockaddr_in *adr);

#endif
