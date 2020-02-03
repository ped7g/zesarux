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
#include <unistd.h>                
#include <string.h>

#include "cpu.h"

//Proceso inicial
int main (int main_argc,char *main_argv[]) {

	// check for extra quotes around whole argument and remove them here
	for (int i = 0; i < main_argc; ++i) {
		const long unsigned argv_len = strlen(main_argv[i]);
		if (2 < argv_len && '"' == main_argv[i][0] && '"' == main_argv[i][argv_len-1]) {
			// seems like whole argument is enclosed in extra quotes, remove them here
			main_argv[i][argv_len-1] = 0;
			strcpy(main_argv[i]+0, main_argv[i]+1);
		}
	}

	return zesarux_main (main_argc,main_argv);
}
