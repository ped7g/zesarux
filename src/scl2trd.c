/*
 The MIT License (MIT)

Copyright (c) 2019 Alexander Sharikhin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
IN THE SOFTWARE.
*/

//https://github.com/nihirash/esxdos-scl2trd

//More info:
//http://www.zx-modules.de/fileformats/sclformat.html

//#include <input.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"

//Para usar definiciones de PATH_MAX
#include "utils.h"

//#include "lib/textUtils.h"
//#include "lib/esxdos.h"

//#define PATH_SIZE ( 200 )
//uint8_t filePath[ PATH_SIZE ];

char scl_inputfile[PATH_MAX];
char scl_outputfile[PATH_MAX];


//uint16_t drive;
FILE *iStream;
FILE *oStream;
z80_byte buff[256];
unsigned freeTrack = 1;
unsigned freeSec = 0;
unsigned char count;
unsigned char isFull = 0;
int totalFreeSect = 2544;

void cleanBuffer()
{
  int i;
    for (i=0;i<256;buff[i++] = 0); 
}

void showMessage(char *e) 
{
    debug_printf(VERBOSE_ERR,e);
} 

void writeDiskData()
{
    //int leidos=fread(taperead,1,total_mem,ptr_tapebrowser);
    //uint16_t r = ESXDOS_fread(&buff, 256, iStream);
    int r = fread(&buff, 1,256, iStream);
    while (r == 256) {
        //fwrite(puntero,1,tamanyo,ptr_filesave);
        //ESXDOS_fwrite(&buff, r, oStream);
        fwrite(&buff,1,r,oStream);
        //r = ESXDOS_fread(&buff, 256, iStream);
        r = fread(&buff, 1,256, iStream);
    }
    
    if (isFull) {
        cleanBuffer();
        for (r=0;r<totalFreeSect;r++)
        //fwrite(puntero,1,tamanyo,ptr_filesave);
            //ESXDOS_fwrite(&buff, 256, oStream);
            fwrite(&buff,1,256,oStream);
    }
    
    /*iferror {
        showMessage("Issue with writing disk data to TRD-file");
        return;
    }*/

    fclose(iStream);
    fclose(oStream);
    /*textUtils_println(" * All data written!");
    textUtils_println("");
    textUtils_println(" TRD file is stored and ready to use!");
    textUtils_println("");
    textUtils_println(" Press any key to convert other SCL-file");
    waitKeyPress();
    */
   debug_printf (VERBOSE_INFO,"All scl to trd data written");
}

void writeDiskInfo()
{
    cleanBuffer();
    buff[0xe3] = 0x16; // IMPORTANT! 80 track double sided
    buff[0xe4] = count;
    buff[0xe1] = freeSec;
    buff[0xe2] = freeTrack;
    
    if (isFull) {
        buff[0xe6] = totalFreeSect / 256;
        buff[0xe5] = totalFreeSect & 255;
    }

    buff[0xe7] = 0x10;
    buff[0xf5] = 's';
    buff[0xf6] = 'c';
    buff[0xf7] = 'l';
    buff[0xf8] = '2';
    buff[0xf9] = 't';
    buff[0xfa] = 'r';
    buff[0xfb] = 'd';
     //fwrite(puntero,1,tamanyo,ptr_filesave);
    //ESXDOS_fwrite(&buff, 256, oStream);
    //ESXDOS_fwrite(0, 1792, oStream); // Any dirt is ok
    fwrite(&buff, 1,256, oStream);

    char dirt_data[1792];
    fwrite(&dirt_data, 1,1792, oStream); // Any dirt is ok

    /*iferror {
        showMessage("Issue with writing disk info to TRD-file");
        return;
    }*/
    //textUtils_println(" * Disk information written");
    writeDiskData();
}

void writeCatalog()
{
    int i;
    totalFreeSect = 2544;
    freeTrack = 1;
    freeSec = 0;
    count = 0;

    //oStream = ESXDOS_fopen(filePath, ESXDOS_FILEMODE_WRITE_CREATE, drive);
    oStream = fopen(scl_outputfile,"wb");
    if  (oStream==NULL) {
        showMessage("Can't open output file");
        return ;
    }

    //ESXDOS_fread(&count, 1, iStream);
    fread(&count,1,1,iStream);
    for (i=0;i<count; i++) {
        //ESXDOS_fread(&buff, 14, iStream);
        fread(&buff, 1,14, iStream);
        buff[14] = freeSec;
        buff[15] = freeTrack;
        freeSec += buff[0xd];
        freeTrack += freeSec / 16;
        totalFreeSect -= (int) buff[0xd];
        freeSec = freeSec % 16;
        //ESXDOS_fwrite(&buff, 16, oStream);
        fwrite(&buff, 1,16, oStream);
    }
    cleanBuffer();

    for (i = count;i<128;i++) {
        //ESXDOS_fwrite(&buff, 16, oStream);
        fwrite(&buff, 1,16, oStream);
    }
    /*iferror {
        showMessage("Issue with writing catalog to TRD-file");
        return;
    }*/
    //textUtils_println(" * Disk catalog written");
    writeDiskInfo();
}


void validateScl()
{
    char *expected = "SINCLAIR";
    //drive = ESXDOS_getDefaultDrive();
    //iStream = ESXDOS_fopen(filePath, ESXDOS_FILEMODE_READ, drive );
    iStream = fopen(scl_inputfile,"rb");
    if (iStream==NULL) {
        showMessage("Can't open input file");
        return ;
    }

    cleanBuffer();
    //ESXDOS_fread(&buff, 8, iStream);
    fread(&buff, 1,8, iStream);
    if (strcmp(expected, (char *)&buff)) {
        showMessage("Wrong file! Select only SCL files");
        return;
    }
    //sprintf(strstr(filePath, ".SCL"), ".TRD");
    //textUtils_println(" * File is valid SCL");
    writeCatalog();
}

void selectFile() 
{
    /*char c;
    textUtils_cls();
    textUtils_setAttributes( INK_BLUE | PAPER_BLACK );
    fileDialogpathUpOneDir( filePath );
    while ( 
        openFileDialog( 
            "SCL2TRD by Nihirash v. 1.1.0", 
            "<Cursor/Sincl> - movement  <Ent/0> - select file  <Space> - exit",
            filePath, 
            PATH_SIZE, 
            INK_BLUE | PAPER_WHITE, 
            INK_WHITE | PAPER_BLACK 
            ) == false ) {
        __asm
            rst 0
        __endasm;
    }

    textUtils_cls();
    textUtils_println("");
    textUtils_print("Do you want full TRD-file? (y/n) ");
    
    while (!(c == 'y' || c == 'n')) {
        c = waitKeyPress();
    }

    isFull = (c == 'y');

    textUtils_println(isFull ? "yes" : "no");

    textUtils_println(" Converting file!");

    validateScl();*/
}

void scl2trd_main(char *input,char *output)
{
    strcpy(scl_inputfile,input);
    strcpy(scl_outputfile,output);
    validateScl();
}