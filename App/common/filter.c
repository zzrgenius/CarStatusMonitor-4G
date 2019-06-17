#include "filter.h"

/* ----------------------------------------------------------------------
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 *
* $Date:         17. January 2013
* $Revision:     V1.4.0
*
* Project:       CMSIS DSP Library
 * Title:        arm_fir_example_f32.c
 *
 * Description:  Example code demonstrating how an FIR filter can be used
 *               as a low pass filter.
 *
 * Target Processor: Cortex-M4/Cortex-M3
 *
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------- */
/* ----------------------------------------------------------------------
** Include Files
** ------------------------------------------------------------------- */
#include "main.h"
#include "filter.h"
#include "math_helper.h"
/* ----------------------------------------------------------------------
** Macro Defines
** ------------------------------------------------------------------- */
#define MPU_FILTER_LENGTH	 320
#define BLOCK_SIZE            32
#define NUM_TAPS              17
/* -------------------------------------------------------------------
 * The input signal and reference output (computed with MATLAB)
 * are defined externally in arm_fir_lpf_data.c.
 * ------------------------------------------------------------------- */
  float32_t Mpu_Input_20HZ[MPU_FILTER_LENGTH];
/* -------------------------------------------------------------------
 * Declare Test output buffer
 * ------------------------------------------------------------------- */
static float32_t FilterOutput[MPU_FILTER_LENGTH];
/* -------------------------------------------------------------------
 * Declare State buffer of size (numTaps + blockSize - 1)
 * ------------------------------------------------------------------- */
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];


static arm_fir_instance_f32 mpu_firS;
/* ----------------------------------------------------------------------
** FIR Coefficients buffer generated using fir1() MATLAB function.
** fir1(28, 6/24)
** ------------------------------------------------------------------- */
//const float32_t firCoeffs32[NUM_TAPS] = {
//  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
//  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
//  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
//  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
//};
//const float32_t firCoeffs32[NUM_TAPS] = {
//  -0.0019f, 0.0052f,-0.0035f, -0.0164f,0.0409f, -0.0119f, -0.1115f,0.2739f, 
//  0.6505f, 0.2739f, -0.1115f, -0.0119f,0.0409f, -0.0164f, -0.0035f, 0.0052f, -0.0019f
//};
 const float32_t firCoeffs32[NUM_TAPS] = { 
	 -0.0020f,  -0.0009f,    0.0038f,    0.0178f,    0.0445f,    0.0817f,    0.1214f,    0.1519f,    0.1634f,    0.1519f,    0.1214f,    0.0817f, 
	 0.0445f,    0.0178f,    0.0038f,   -0.0009f,   -0.0020f};
//const float32_t firCoeffs32[NUM_TAPS] = {
//    -0.003062003292f,-0.005031108391f,-0.006772691384f,2.870123165e-18f,  0.02554769814f,
//    0.07308331877f,   0.1324728429f,   0.1826158166f,   0.2022922337f,   0.1826158166f,
//     0.1324728429f,  0.07308331877f,  0.02554769814f,2.870123165e-18f,-0.006772691384f,
//  -0.005031108391f,-0.003062003292f
//};

/* ------------------------------------------------------------------
 * Global variables for FIR LPF Example
 * ------------------------------------------------------------------- */
uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = MPU_FILTER_LENGTH/BLOCK_SIZE;
	 float32_t acc_zthreshold = LIMITED_AM;
 /* ----------------------------------------------------------------------
 * FIR LPF Example
 * ------------------------------------------------------------------- */
int32_t mpu_acc_filter(void)
{
  uint32_t i;
   float32_t  *inputF32, *outputF32;
  /* Initialize input and output buffer pointers */
  inputF32 = &Mpu_Input_20HZ[0];
  outputF32 = &FilterOutput[0];
    /* ----------------------------------------------------------------------
  ** Call the FIR process function for every blockSize samples
  ** ------------------------------------------------------------------- */
  for(i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&mpu_firS, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
	  
  }
  for(i=0;i<MPU_FILTER_LENGTH;i++)
  {
	  printf(" %7.5f ",FilterOutput[i]);
  }
  printf("\r\n");
  return 0;
}
void mpu_filter_init(void)
{
	arm_fir_init_f32(&mpu_firS, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

}
  float32_t filter_average(float32_t sam)             //¾ùÖµÂË²¨
{
    static float32_t buf_va[4];
    static char offset_va = 0;
    float32_t z;
    char i;  
    buf_va[offset_va] = sam;
    z = buf_va[offset_va];
    for (i = 1;  i < 4;  i++)
    {
         z=z+ buf_va[((offset_va - i) & 0x3)];         
    }
    offset_va = (offset_va + 1) & 0x3;
    return (z/4.0f);
}
uint8_t filter_limit(float32_t data)
{
	static float32_t old_data;
	 
	if((data - old_data > acc_zthreshold) || (old_data -data > acc_zthreshold))
	{
		old_data = data;
		return 1;
	}
	else
		old_data = data; 
	return 0;
}

//float32_t fir_pro(float32_t data)
//{
//	static uint16_t i = 0;
//	static uint16_t j = 0;
//	uint16_t n;
//	Mpu_Input_20HZ[i++] =   ( data);
//	FilterOutput[j++] =  vs_filter_average( data);
//	//printf(" %7.5f ",data); 
//	if(i == MPU_FILTER_LENGTH)
//	{
//		for(n=0;n<MPU_FILTER_LENGTH;n++)
//		{
//			printf(" %7.5f ",Mpu_Input_20HZ[n]);
//		}
//		printf("\r\n");
//		for(n=0;n<MPU_FILTER_LENGTH;n++)
//		{
//			printf(" %7.5f ",FilterOutput[n]);
//		}
//		printf("\r\n");
//		//mpu_acc_filter();
//		
//		i = 0;
//		j=0;
//	}
//	return 0.0;
//}