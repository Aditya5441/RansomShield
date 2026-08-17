/**
 * @file    cs.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "cs.h"
#include "bm.h"

static float Phi[CS_MEASUREMENTS][CS_SIZE];

/**
 * @brief   TODO: describe what CS_Init() does
 */
void CS_Init(void)
{
    for (int i = 0; i < CS_MEASUREMENTS; i++) {
        for (int j = 0; j < CS_SIZE; j++) {
            uint32_t r;

            bm_rng_read_u32(&r, 100);
            Phi[i][j] = (r & 1U) ? 1.0f : -1.0f;
        }
    }
}

/**
 * @brief   TODO: describe what CS_Compress() does
 * @param   input  TODO: describe parameter
 * @param   output  TODO: describe parameter
 */
void CS_Compress(float *input, float *output)
{
    for (int i = 0; i < CS_MEASUREMENTS; i++)
    {
        output[i] = 0;

        for (int j = 0; j < CS_SIZE; j++)
        {
            output[i] += Phi[i][j] * input[j];
        }
    }
}

/**
 * @brief   TODO: describe what CS_Reconstruct() does
 * @param   compressed  TODO: describe parameter
 * @param   reconstructed  TODO: describe parameter
 */
void CS_Reconstruct(float *compressed, float *reconstructed)
{
    for (int i = 0; i < CS_SIZE; i++)
    {
        reconstructed[i] = 0;

        for (int j = 0; j < CS_MEASUREMENTS; j++)
        {
            reconstructed[i] += Phi[j][i] * compressed[j];
        }
    }
}
