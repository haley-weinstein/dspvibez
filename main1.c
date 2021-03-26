
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
static CODECEventId;

void comm_intr()                            //for communication/init using interrupt
{
    IRQ_globalDisable();             //disable interrupts
    DSK6713_init();                   //init DSP and codec
    CODECEventId=MCBSP_getXmtEventId(DSK6713_AIC23_codecdatahandle);//McBSP1 Xmit

    IRQ_map(CODECEventId, 11);          //map McBSP1 Xmit to INT11
    IRQ_reset(CODECEventId);            //reset codec INT 11
    IRQ_globalEnable();                  //globally enable interrupts
    IRQ_nmiEnable();                    //enable NMI interrupt
    IRQ_enable(CODECEventId);            //enable CODEC eventXmit INT11

    MCBSP_write(DSK6713_AIC23_DATAHANDLE,0 )  ;             //start McBSP interrupt outputting a sample
}



interrupt void c_int11()
{
    input = MCBSP_read(DSK6713_AIC23_DATAHANDLE);
    delayed = buffer[i];
    output = input + delayed;
    MCBSP_write(DSK6713_AIC23_DATAHANDLE, output);
    buffer[i] = input + delayed*GAIN;
    if(++i >= BUF_SIZE){
        i=0;
    }
    return;
}

void main()
{
    comm_intr();
    for(i=0; i<BUF_SIZE; i++)
    {
        buffer[i] = 0;
    }
    while(1);
}

