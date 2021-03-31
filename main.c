/*
 * Author: Haley Weinstein, Zachary Stern
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
void echo2(DSK6713_AIC23_CodecHandle hCodec);
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
        buffer[i] = sample_pair ;
        sample_pair = A*buffer[i-1]+B*buffer[i-2]+GAIN*sample_pair;
        while(!DSK6713_AIC23_write(hCodec, sample_pair));
    }

}


void echo2(DSK6713_AIC23_CodecHandle hCodec){
    Uint32 sample_pair = 0;
    Uint32 delayed = 0;

    for(i=1; i<BUF_SIZE; i++)
    {
        while(!DSK6713_AIC23_read(hCodec, &sample_pair));
        output = sample_pair + delayed;
        delayed = output*GAIN;
        while(!DSK6713_AIC23_write(hCodec, output));
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
    I2C_Handle hi2c;

    I2C_Config i2c_config;
    i2c_config.i2coar = 0x09;                           // set slave address to 9
    i2c_config.i2cier = 0;
    i2c_config.i2cclkl = 0xFF;
    i2c_config.i2cclkh = 0xFF;
    i2c_config.i2ccnt = 0;
    i2c_config.i2csar = 0;
    i2c_config.i2cmdr = 0x2020;                         // configured using doc SPRU175D
    i2c_config.i2cpsc = 13;                             // not sure about this one, found online

    DSK6713_init(); //THIS MUST BE CALLED BEFORE ANYTHING ELSE

    CSL_init();

    hi2c = I2C_open(I2C_DEV0, 0);
    I2C_reset(hi2c);                                    // Reset I2C module to configure
    I2C_config(hi2c,&i2c_config);                        // Configure I2C module using config structure
    I2C_outOfReset(hi2c);                               // Take module out of reset mode
    I2C_start(hi2c);

    hCodec = DSK6713_AIC23_openCodec(0, &config);               //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);    //set sampling frequency to 44 kHz

    int value = 0;

    Uint32* buffer = calloc(4000, sizeof(Uint32));

    while(TRUE){

        if (I2C_rrdy(hi2c)) {
            value = I2C_readByte(hi2c);
            int j = 0;
        }
        echo2(hCodec);
    }

}
