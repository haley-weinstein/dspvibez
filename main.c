/*
 * Author: Haley Weinstein, Zachary Stern
 * Date: 02/09/2020
 * Description: Just the echo effect
 */
#define CHIP_6713
#define BUF_SIZE 8000

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
#include <stdbool.h>
#include <stdlib.h>


float *buffer;
float *triangle;
float right, right_out, ret, peak;
int iright;

int readIndex = 0;


int writeIndex = BUF_SIZE - 1;
float damping = .05;
int minf = 500;
int maxf = 3000;
int wah_freq = 2000;
int sampling_freq = 8000;
//float delta_f = wah_freq/sampling_freq;

Uint32 sample_pair, output;

void add_delays(DSK6713_AIC23_CodecHandle hCodec, int del_time1, int del_time2, int del_time3, float gain_1, float gain_2, float gain_3){
    while(!DSK6713_AIC23_read(hCodec, &sample_pair));
    right =( (int) sample_pair) << 16 >> 16;
    right_out = right+.9*buffer[readIndex] + gain_1*buffer[readIndex+del_time1] +gain_2*buffer[readIndex+del_time2]+gain_3*buffer[readIndex+del_time3];
    ++readIndex;
    if ((readIndex+del_time3) >= BUF_SIZE)
        readIndex=0;
    buffer[writeIndex] = right_out;
    ++writeIndex;
    if(writeIndex >= BUF_SIZE)
        writeIndex = 0;
    iright = (int) right_out;
    output = (iright <<16)|(iright & 0x0000FFFF);
    while(!DSK6713_AIC23_write(hCodec, output));
}

/*
void wah(DSK6713_AIC23_CodecHandle hCodec){
    change_in_freq = wah_freq/sampling_freq;
    float q1 = 2 *damp;
    while(!DSK6713_AIC23_read(hCodec, &sample_pair));
    right =( (int) sample_pair) << 16 >> 16;

}*/

void main()
{
    DSK6713_init();
    DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;
    DSK6713_AIC23_CodecHandle hCodec;
    float* buffer = calloc(BUF_SIZE, sizeof(float));
    hCodec = DSK6713_AIC23_openCodec(0, &config);               //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);
    bool echo = true;
    bool reverb = false; //get these variables from i2c bus
    bool slapback = false;
    bool chorus = false;
    bool wahwah = false;
    //int tot = (maxf - minf)/delta_f;
    /*
    float* triangle = calloc(tot, sizeof(float));
    int i = 0;

    float freq;

    while(freq < max_freq){
        freq = minf + delta_f;
        triangle[i] = freq;
        i = i + 1;
    }


    while(sizeof triangle < BUF_SIZE){

    }
    */
    while(TRUE){
        if (echo)
        {
            add_delays(hCodec, 1000, 1000, 1000, .9, .7, .5);
        }
        if (reverb)
        {
            add_delays(hCodec, 50, 50, 50, .9, .7, .5);
        }
        if (slapback)
        {
            add_delays(hCodec, 10, 10, 10, 1, 1, 1);
        }


    }
}

