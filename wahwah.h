/*
 * wahwah.h
 *
 *  Created on: Apr 10, 2021
 *      Author: haley
 */

#ifndef WAHWAH_H_
#define WAHWAH_H_


extern void AutoWah_init(short effect_rate,short sampling,short maxf,short minf,short Q,double gainfactor,short freq_step);
extern double AutoWah_process(int xin);
extern void AutoWah_sweep(void);


#endif /* WAHWAH_H_ */
