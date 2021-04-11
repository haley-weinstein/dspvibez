/*
 * bp_iir.h
 *
 *  Created on: Apr 10, 2021
 *      Author: haley
 */

#ifndef BP_IIR_H_
#define BP_IIR_H_

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

extern void bp_iir_init(double fsfilt,double gb,double Q,short fstep, short fmin);
extern void bp_iir_setup(struct bp_filter * H,int index);
extern double bp_iir_filter(double yin,struct bp_filter * H);



#endif /* BP_IIR_H_ */
