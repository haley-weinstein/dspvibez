/*
 * Author: Haley Weinstein
 * Date: 02/09/2020
 * Description: Capstone working product [1]
 */
#define CHIP_6713

//includes from the chip lib:
#include <stdio.h>
#include <math.h>
#include <c6x.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include "dsk6713.h"
#include "dsk6713_led.h"
#include "dsk6713_aic23.h"
#include <stdint.h>
#include <inttypes.h>

#define BUF_SIZE 200
#define A 5
#define B 2
#define GAIN 10

Uint32 buffer[BUF_SIZE];
float filterTaps[9] = {.002385, 0.011910, 0.026352, .038925, .045351, .039825, .026352, .011910, .002385}; // might need different filter values
Uint32 filter_buffer[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
OVERDRIVE_VAL = 10; // get this from the i2c interface

void echo(DSK6713_AIC23_CodecHandle hCodec);
void overdrive(DSK6713_AIC23_CodecHandle hCodec);
void fir_filter(Uint32 *sample_pair);

DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;

void echo(DSK6713_AIC23_CodecHandle hCodec){
    /*
     * Description: simple echo effect
     */
    buffer[0] = 1;
    buffer[1] = 2;
    int i;
    Uint32 sample_pair = 0;
    for (i=2; i<BUF_SIZE; i++)
    {
        while(!DSK6713_AIC23_read(hCodec, &sample_pair));
        fir_filter(&sample_pair);
        sample_pair = A*buffer[i-1]+B*buffer[i-2]+GAIN*sample_pair;
        buffer[i] = sample_pair ;// how did I forget this before I am actually dense
        while(!DSK6713_AIC23_write(hCodec, sample_pair));
    }

}

void fir_filter(Uint32 *sample_pair)
{
    /* we can repurpose this with different tap vals for the wah effect
     * I definitely fucked up the pointer shit here idk what a pointer is sorry lol
     * the filter math should be gucci tho might need to adjust the taps
     */
    int i;
    Uint32 result = *sample_pair;
    for (i = 0; i < 9; i++) { // for each tap
            result = result + filter_buffer[i] * filterTaps[i]; // multiply
            if (i==0)
                filter_buffer[0] = *sample_pair;
            else
                filter_buffer[i] = filter_buffer[i-1]; // move everything down the line
    }
    *sample_pair = result;
}

void overdrive(DSK6713_AIC23_CodecHandle hCodec)
{
    /*
     * Description: simple overdrive effect
     * Input: Int32 type sound sample
     * Output: Amplified Int32 sample
     */
    Uint32 sample_pair = 0;
    while(!DSK6713_AIC23_read(hCodec, &sample_pair));
    sample_pair = sample_pair*OVERDRIVE_VAL;
    while(!DSK6713_AIC23_write(hCodec, sample_pair));
}

void main()
{
    DSK6713_AIC23_CodecHandle hCodec;

    DSK6713_init(); //THIS MUST BE CALLED BEFORE ANYTHING ELSE

    hCodec = DSK6713_AIC23_openCodec(0, &config); //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_44KHZ);  //set sampling frequency to 44 kHz

    while(TRUE){
        overdrive(hCodec);
    }

}
