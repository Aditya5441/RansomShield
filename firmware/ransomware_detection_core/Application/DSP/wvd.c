/**
 * @file    wvd.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "wvd.h"
#include <math.h>

/**
 * @brief   TODO: describe what WVD_Compute() does
 * @param   signal  TODO: describe parameter
 * @param   float wvd[WVD_SIZE][WVD_SIZE]  TODO: describe parameter
 */
void WVD_Compute(float *signal, float wvd[WVD_SIZE][WVD_SIZE])
{
    for (int t = 0; t < WVD_SIZE; t++)
    {
        for (int f = 0; f < WVD_SIZE; f++)
        {
            float sum = 0;

            for (int tau = -WVD_SIZE/2; tau < WVD_SIZE/2; tau++)
            {
                int t1 = t + tau;
                int t2 = t - tau;

                if (t1 >= 0 && t1 < WVD_SIZE && t2 >= 0 && t2 < WVD_SIZE)
                {
                    float angle = 2.0f * 3.1415926f * f * tau / WVD_SIZE;
                    sum += signal[t1] * signal[t2] * cosf(angle);
                }
            }

            wvd[t][f] = sum;
        }
    }
}
