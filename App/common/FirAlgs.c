/****************************************************************************
*
* Name: FirAlgs.c
*
* Synopsis: FIR filter algorithms for use in C
*
* Description:
*
*   This module provides functions to implement Finite Impulse Response (FIR)
*   filters using a variety of algorithms.  
*
*   These functions use most or all of the following input parameters:
*
*       input    - the input sample data
*       ntaps    - the number of taps in the filter   
*       h[]      - the FIR coefficient array
*       z[]      - the FIR delay line array 
*       *p_state - the "state" of the filter; initialize this to 0 before
*                  the first call
*
*   The functions fir_basic, fir_shuffle, and fir_circular are not very
*   efficient in C, and are provided here mainly to illustrate FIR 
*   algorithmic concepts.  However, the functions fir_split, fir_double_z
*   and fir_double_h are all fairly efficient ways to implement FIR filters
*   in C.
*
* Copyright 2000-2015 by Grant R. Griffin
*
* Thanks go to contributors of comp.dsp for teaching me some of these
* techniques, and to Jim Thomas for his review and great suggestions.
*
*                          The Wide Open License (WOL)
*
* Permission to use, copy, modify, distribute and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice and this license appear in all source copies. 
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
* ANY KIND. See http://www.dspguru.com/wide-open-license for more information.
*
*****************************************************************************/

#include <stdio.h>
#ifdef WIN32
    #include <conio.h>
#endif 

#define SAMPLE  double      /* define the type used for data samples */


/****************************************************************************
* clear: This zeroizes a FIR delay line                                    
/****************************************************************************/
void clear(int ntaps, SAMPLE z[])      
{
    int ii;
    for (ii = 0; ii < ntaps; ii++) {
        z[ii] = 0;
    }
}

/****************************************************************************
* fir_basic: This does the basic FIR algorithm: store input sample, calculate  
* output sample, shift delay line                                           
*****************************************************************************/
SAMPLE fir_basic(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[])       
{
    int ii;
    SAMPLE accum;
    
    /* store input at the beginning of the delay line */
    z[0] = input;

    /* calc FIR */
    accum = 0;
    for (ii = 0; ii < ntaps; ii++) {
        accum += h[ii] * z[ii];
    }

    /* shift delay line */
    for (ii = ntaps - 2; ii >= 0; ii--) {
        z[ii + 1] = z[ii];
    }

    return accum;
}

/****************************************************************************
* fir_circular: This function illustrates the use of "circular" buffers
* in FIR implementations.  The advantage of circular buffers is that they
* alleviate the need to move data samples around in the delay line (as
* was done in all the functions above).  Most DSP microprocessors implement
* circular buffers in hardware, which allows a single FIR tap to be
* calculated in a single instruction.  That works fine when programming in
* assembly, but since C doesn't have any constructs to represent circular
* buffers, you need to fake them in C by adding an extra "if" statement
* inside the FIR calculation.
*****************************************************************************/
SAMPLE fir_circular(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[],
                    int *p_state)         
{
    int ii, state;
    SAMPLE accum;

    state = *p_state;               /* copy the filter's state to a local */

    /* store input at the beginning of the delay line */
    z[state] = input;
    if (++state >= ntaps) {         /* incr state and check for wrap */
        state = 0;
    }

    /* calc FIR and shift data */
    accum = 0;
    for (ii = ntaps - 1; ii >= 0; ii--) {
        accum += h[ii] * z[state];
        if (++state >= ntaps) {     /* incr state and check for wrap */
            state = 0;
        }
    }

    *p_state = state;               /* return new state to caller */

    return accum;
}

/****************************************************************************
* fir_shuffle: This is like fir_basic, except that data is shuffled by     
* moving it _inside_ the calculation loop.  This is similar to the MACD    
* instruction on TI's fixed-point processors.       
*****************************************************************************/
SAMPLE fir_shuffle(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[])         
{
    int ii;
    SAMPLE accum;
    
    /* store input at the beginning of the delay line */
    z[0] = input;

    /* calc FIR and shift data */
    accum = h[ntaps - 1] * z[ntaps - 1];
    for (ii = ntaps - 2; ii >= 0; ii--) {
        accum += h[ii] * z[ii];
        z[ii + 1] = z[ii];
    }

    return accum;
}

/****************************************************************************
* fir_split: This splits the calculation into two parts so the circular    
* buffer logic doesn't have to be done inside the calculation loop.   
*****************************************************************************/
SAMPLE fir_split(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[],
                 int *p_state)
{
    int ii, end_ntaps, state = *p_state;
    SAMPLE accum;
    SAMPLE const *p_h;
    SAMPLE *p_z;

    /* setup the filter */
    accum = 0;
    p_h = h;

    /* calculate the end part */
    p_z = z + state;
    *p_z = input;
    end_ntaps = ntaps - state;
    for (ii = 0; ii < end_ntaps; ii++) {
        accum += *p_h++ * *p_z++;
    }

    /* calculate the beginning part */
    p_z = z;
    for (ii = 0; ii < state; ii++) {
        accum += *p_h++ * *p_z++;
    }

    /* decrement the state, wrapping if below zero */
    if (--state < 0) {
        state += ntaps;
    }
    *p_state = state;       /* pass new state back to caller */

    return accum;
}

/****************************************************************************
* fir_double_z: This uses a doubled delay line so the FIR calculation always
* operates on a flat buffer. 
*****************************************************************************/
SAMPLE fir_double_z(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[],
            int *p_state)
{
    SAMPLE accum;
    int ii, state = *p_state;
    SAMPLE const *p_h, *p_z;

    /* store input at the beginning of the delay line as well as ntaps more */
    z[state] = z[state + ntaps] = input;

    /* calculate the filter */
    p_h = h;
    p_z = z + state;
    accum = 0;
    for (ii = 0; ii < ntaps; ii++) {
        accum += *p_h++ * *p_z++;
    }

    /* decrement state, wrapping if below zero */
    if (--state < 0) {
        state += ntaps;
    }
    *p_state = state;       /* return new state to caller */

    return accum;
}

/****************************************************************************
* fir_double_h: This uses doubled coefficients (supplied by caller) so that 
* the filter calculation always operates on a flat buffer.
*****************************************************************************/
SAMPLE fir_double_h(SAMPLE input, int ntaps, const SAMPLE h[], SAMPLE z[],
                    int *p_state)
{
    SAMPLE accum;
    int ii, state = *p_state;
    SAMPLE const *p_h, *p_z;

    /* store input at the beginning of the delay line */
    z[state] = input;

    /* calculate the filter */
    p_h = h + ntaps - state;
    p_z = z;
    accum = 0;
    for (ii = 0; ii < ntaps; ii++) {
        accum += *p_h++ * *p_z++;
    }

    /* decrement state, wrapping if below zero */
    if (--state < 0) {
        state += ntaps;
    }
    *p_state = state;       /* return new state to caller */

    return accum;
}

/****************************************************************************
* main: This provides a simple test suite for the functions above.  An
* impulse is fed into each filter implementation, so the output should be
* the "impulse response", that is, the coefficients of the filter.  You
* should see some zeroes, followed by the "coefficents" below, followed by a
* few more zeroes.
*****************************************************************************/
int main(void)
{
    #define NTAPS 6
    static const SAMPLE h[NTAPS] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    static SAMPLE h2[2 * NTAPS];
    static SAMPLE z[2 * NTAPS];
    #define IMP_SIZE (3 * NTAPS)
    static SAMPLE imp[IMP_SIZE];
    SAMPLE output;
    int ii, state;

    /* make impulse input signal */
    clear(IMP_SIZE, imp);
    imp[5] = 1.0;

    /* create h2 by doubling h */
    for (ii = 0; ii < NTAPS; ii++) {
        h2[ii] = h2[ii + NTAPS] = h[ii];
    }

    /* test FIR algorithms */

    printf("Testing fir_basic:\n   ");
    clear(NTAPS, z);
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_basic(imp[ii], NTAPS, h, z);
        printf("%3.1lf ", (double) output);
    }   
    printf("\n\n");

    printf("Testing fir_shuffle:\n   ");
    clear(NTAPS, z);
    state = 0;
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_shuffle(imp[ii], NTAPS, h, z);
        printf("%3.1lf ", (double) output);
    }   
    printf("\n\n");

    printf("Testing fir_circular:\n   ");
    clear(NTAPS, z);
    state = 0;
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_circular(imp[ii], NTAPS, h, z, &state);
        printf("%3.1lf ", (double) output);
    }   
    printf("\n\n");

    printf("Testing fir_split:\n   ");
    clear(NTAPS, z);
    state = 0;
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_split(imp[ii], NTAPS, h, z, &state);
        printf("%3.1lf ", (double) output);
    }   
    printf("\n\n");

    printf("Testing fir_double_z:\n   ");
    clear(2 * NTAPS, z);
    state = 0;
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_double_z(imp[ii], NTAPS, h, z, &state);
        printf("%3.1lf ", (double) output);
    }   
    printf("\n\n");

    printf("Testing fir_double_h:\n   ");
    clear(NTAPS, z);
    state = 0;
    for (ii = 0; ii < IMP_SIZE; ii++) {
        output = fir_double_h(imp[ii], NTAPS, h2, z, &state);
        printf("%3.1lf ", (double) output);
    }   

    #ifdef WIN32
        printf("\n\nHit any key to continue.");
        _getch();
    #endif

    return 0;
}