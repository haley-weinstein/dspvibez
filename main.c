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
#include <math.h>

float buffer[BUF_SIZE];

float right, right_out, ret, delay_line;
int iright;
int i, buf_idx, read_idx;
int delay;
Uint32 sample_pair, output;
int center_freq = 0;
int samp_freq = 8000;
int counter = 2000;
int control = 0;
int maxf = 3000;
int minf = 1000;
int freq_step = 1;
int counter_limit = 2000;
int ind;
/*Convert frequencies to index ranges*/
int min_freq = 0;


int i;
float damp;
double wo;

int sampling_frequency = 8000;
struct bp_coeffs{
    double e;
    double p;
    double d[3];
};

struct bp_filter{
    double e;
    double p;
    double d[3];
    double x[3];
    double y[3];
};

double yout;

struct bp_coeffs bp_coeff_arr[BP_MAX_COEFS];
struct bp_filter H;

void main()
{
    DSK6713_init();
    CSL_init();
    for(i=0;i<BP_MAX_COEFS;i++)
    {
      wo = 2*PI*(1*i + minf)/samp_freq;
      bp_coeff_arr[i].e = 1/(1 + damp*tan(wo/(4*2)));
      bp_coeff_arr[i].p = cos(wo);
      bp_coeff_arr[i].d[0] = (1-bp_coeff_arr[i].e);
      bp_coeff_arr[i].d[1] = 2*bp_coeff_arr[i].e*bp_coeff_arr[i].p;
      bp_coeff_arr[i].d[2] = (2*bp_coeff_arr[i].e-1);
    }
    DSK6713_AIC23_Config config = DSK6713_AIC23_DEFAULTCONFIG;
    DSK6713_AIC23_CodecHandle hCodec;
    int max_freq = (maxf - minf)/freq_step;
    damp = .707/pow((1 - pow(.707,2)), 1/2);
    hCodec = DSK6713_AIC23_openCodec(0, &config);               //open codec with default configuration
    DSK6713_AIC23_setFreq(hCodec, DSK6713_AIC23_FREQ_8KHZ);

    while(true){
        while(!DSK6713_AIC23_read(hCodec, &sample_pair));
        right =( (int) sample_pair) << 16 >> 16;
        H.x[0] =  H.x[1];
        H.x[1] =  H.x[2];
        H.x[2] = right;

        H.y[0] =  H.y[1];
        H.y[1] =  H.y[2];

        H.y[2] = H.d[0]* H.x[2] - H.d[0]* H.x[0] + (H.d[1]* H.y[1]) - H.d[2]* H.y[0];

        yout =  H.y[2];
        iright = (int) yout;
        output = (iright <<16)|(iright & 0x0000FFFF);
        while(!DSK6713_AIC23_write(hCodec, output));
        if (!--counter) {
            if (!control) {
                ind = center_freq+freq_step;
                H.e = bp_coeff_arr[ind].e;
                H.p = bp_coeff_arr[ind].p;
                H.d[0] = bp_coeff_arr[ind].d[0];
                H.d[1] = bp_coeff_arr[ind].d[1];
                H.d[2] = bp_coeff_arr[ind].d[2];
                if (center_freq > max_freq) {
                    control = 1;
                }
            }
            else if (control) {
                ind = center_freq-freq_step;
                H.e = bp_coeff_arr[ind].e;
                H.p = bp_coeff_arr[ind].p;
                H.d[0] = bp_coeff_arr[ind].d[0];
                H.d[1] = bp_coeff_arr[ind].d[1];
                H.d[2] = bp_coeff_arr[ind].d[2];
                if (center_freq == min_freq) {
                    control = 0;
                }
            }

            counter = counter_limit;
        }

    }


}

