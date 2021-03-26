
/*
 * Author: Hayle
 * Date: 03/23
 * Description:test this first
 *
 */
#define CHIP_6713

//includes from the chip lib:
#include <stdio.h>
#include <math.h>
#include <c6x.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include <csl_i2c.h>
#include <csl_i2chal.h>
#include "dsk6713.h"
#include "dsk6713_led.h"
#include "dsk6713_aic23.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUF_SIZE 4000
#define A 5
#define B 2
#define GAIN 1

Uint32* buffer;
float filterTaps[9] = {.002385, 0.011910, 0.026352, .038925, .045351, .039825, .026352, .011910, .002385}; // might need different filter values
Uint32 filter_buffer[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
Uint32 OVERDRIVE_VAL = 10; // get this from the i2c interface

int i;
Uint32 input, output, delayed;

void echo(DSK6713_AIC23_CodecHandle hCodec);

void echo(DSK6713_AIC23_CodecHandle hCodec){

    for(i=0; i<(BUF_SIZE-1); i++)
    {
        input = MCBSP_read(DSK6713_AIC23_DATAHANDLE);
        delayed = buffer[i];
        output = input + delayed;
        buffer[i+1] = input + delayed;
        MCBSP_write(DSK6713_AIC23_DATAHANDLE, output);
    }
}

/**
 * main.c
 */
int main(void)
{
    DSK6713_init();
    DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;
    DSK6713_AIC23_CodecHandle hCodec;
    hCodec = DSK6713_AIC23_openCodec(0, &config);
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);
    Uint32* buffer = calloc(4000, sizeof(Uint32));


    while(1){
        echo(hCodec);
    }
	return 0;
}
