/*
 * Author: Haley Weinstein, Zachary Stern
 * Date: 02/09/2020
 * Description: Just the echo effect
 */
#define CHIP_6713
#define BUF_SIZE 5000
#define CHUNK_SIZE 200

//Add board support libraries
#include <c6x.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include <csl_i2c.h>
#include <csl_i2chal.h>

//Add dsk specific libraries
#include "dsk6713.h"
#include "dsk6713_led.h"
#include "dsk6713_aic23.h"

//add c libraries
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//add library haley snagged for the wah
#include <wahwah.h>

float *buffer;

float right, right_out, ret;
int iright;




int writeIndex = 0;
float damping = .05;
int minf = 500;
int maxf = 3000;
int wah_freq = 2000;
int sampling_freq = 8000;
float damp = .05;

int i;

Uint32 sample_pair, output;



void main()
{
    DSK6713_init();
    DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;
    DSK6713_AIC23_CodecHandle hCodec;
    float* buffer = calloc(BUF_SIZE, sizeof(float));
    hCodec = DSK6713_AIC23_openCodec(0, &config);               //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);
    bool echo = true;
    bool wahwah = false;
    for(i=0; i<BUF_SIZE; i++){
        buffer[i] = 0;
    }
    AutoWah_init(2000,  /*Effect rate 2000*/
                 sampling_freq, /*Sampling Frequency*/
                 1000,  /*Maximum frequency*/
                 500,   /*Minimum frequency*/
                 4,     /*Q*/
                 0.707, /*Gain factor*/
                 10     /*Frequency increment*/
                 );
    while(TRUE){
        if (echo)
        {
            while(!DSK6713_AIC23_read(hCodec, &sample_pair));
            right =( (int) sample_pair) << 16 >> 16;
            right_out = right+.9*buffer[1] + .9*buffer[1000] +.9*buffer[2000]+.9*buffer[3000];
            buffer[0] = right_out; //try changing this to right if it sounds funky

            for(i=0 ; i<BUF_SIZE; i++){
                buffer[i+1] = buffer[i]; //move the buffer down
            }

            iright = (int) right_out;
            output = (iright <<16)|(iright & 0x0000FFFF);
            while(!DSK6713_AIC23_write(hCodec, output));
        }
        if (wahwah){
            while(!DSK6713_AIC23_read(hCodec, &sample_pair));
            right =( (int) sample_pair) << 16 >> 16;
            ret = AutoWah_process(right);
            iright = (int) ret;
            output = (iright <<16)|(iright & 0x0000FFFF);
            while(!DSK6713_AIC23_write(hCodec, output));
            AutoWah_sweep();

        }


    }
}

