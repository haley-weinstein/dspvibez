/*
 * Author: Haley Weinstein, Zachary Stern
 * Date: 02/09/2020
 * Description: Just the echo effect
 */
#define CHIP_6713
#define BUF_SIZE 5000
#define CHUNK_SIZE 200
#define BP_MAX_COEFS 120
#define PI 3.1415926
#define DELAYS 10

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


float buffer[BUF_SIZE];

float right, right_out, ret, delay_line;
int iright;
int i, buf_idx, read_idx;
int delay;
Uint32 sample_pair, output;

int sampling_frequency = 8000;

void main()
{
    DSK6713_init();
    CSL_init();
    DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;
    DSK6713_AIC23_CodecHandle hCodec;

    hCodec = DSK6713_AIC23_openCodec(0, &config);               //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);
    bool echo = true;
    bool wahwah = false;
    double delay_time = .05;

    for(i=0; i<BUF_SIZE; i++){
        buffer[i] = 0;
    }

    while(true){
        if(echo){
            while(!DSK6713_AIC23_read(hCodec, &sample_pair));
            right =( (int) sample_pair) << 16 >> 16;
            delay = sampling_frequency*delay_time;

            delay_line = 0;
            float gain_factor = 1;
            for(i=0; i<DELAYS; i++)
            {
                buf_idx = (i+delay) % BUF_SIZE;
                delay_line += ((float) pow(.9,i))*buffer[buf_idx];
                gain_factor += pow(.9,i);
            }
            right_out = (delay_line + right);// / gain_factor;
            buffer[read_idx] = right;
            read_idx++;
            if(read_idx >= BUF_SIZE){
                read_idx = 0;
            }

            iright = (int) right_out;
            output = (iright <<16)|(iright & 0x0000FFFF);
            while(!DSK6713_AIC23_write(hCodec, output));

        }
    }


}

