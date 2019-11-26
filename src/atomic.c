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


#include "atomic.h"
#include "compileoptions.h"



#if defined(__APPLE__)
    //En Mac OS X
	

int z_atomic_test_and_set(z_atomic_semaphore *s)
{
	return OSAtomicTestAndSet(1,s);
}
 
void z_atomic_reset(z_atomic_semaphore *s)
{
	OSAtomicAnd32(0,s);
}

    #else 

    #ifdef MINGW
        //En Windows
        //TODO: how to use semafores on mingw


int z_atomic_test_and_set(z_atomic_semaphore *s)
{
	//Estaba a 1, no establecemos bloqueo
	if (*s) return 1;

	//Adquirimos bloqueo
	*s=1;
	return 0;
}

void z_atomic_reset(z_atomic_semaphore *s)
{
	*s=0;
}



        #else

            //En Linux
            //Son builtins de Gcc
            //https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#_005f_005fatomic-Builtins


int z_atomic_test_and_set(z_atomic_semaphore *s)
{
	return __atomic_test_and_set(s,__ATOMIC_RELAXED);
}

void z_atomic_reset(z_atomic_semaphore *s)
{
	__atomic_clear(s,__ATOMIC_RELAXED);
}
		

    #endif

#endif