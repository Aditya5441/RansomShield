/**
 * @file    dsp.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "dsp.h"
#include "wvd.h"
#include "cs.h"
#include <math.h>

static float filtered[DSP_SIZE];
static float weights[DSP_SIZE];
static float cs_compressed[CS_MEASUREMENTS];

/**
 * @brief   TODO: describe what DSP_AdaptiveFilter() does
 * @param   input  TODO: describe parameter
 * @param   output  TODO: describe parameter
 */
static void DSP_AdaptiveFilter(float *input, float *output)
{
    const float mu = 0.05f;

    for (int n = 0; n < DSP_SIZE; n++) {
        float y = 0.0f;
        float norm = 1e-6f;

        for (int i = 0; i < DSP_SIZE; i++) {
            y += weights[i] * input[i];
            norm += input[i] * input[i];
        }

        float e = input[n] - y;

        for (int i = 0; i < DSP_SIZE; i++) {
            weights[i] += (mu / norm) * e * input[i];
        }

        output[n] = e;
    }
}

/**
 * @brief   TODO: describe what DSP_Spectral() does
 * @param   input  TODO: describe parameter
 * @param   energy  TODO: describe parameter
 * @param   centroid  TODO: describe parameter
 */
static void DSP_Spectral(const float *input, float *energy, float *centroid)
{
    float num = 0.0f;
    float den = 0.0f;

    *energy = 0.0f;
    for (int i = 0; i < DSP_SIZE; i++) {
        float m = fabsf(input[i]);
        *energy += m * m;
        num += (float)i * m;
        den += m;
    }
    *centroid = num / (den + 1e-6f);
}

/**
 * @brief   TODO: describe what DSP_ExtractWVDFeatures() does
 * @param   input  TODO: describe parameter
 * @param   wvd_energy  TODO: describe parameter
 * @param   wvd_ridge  TODO: describe parameter
 */
static void DSP_ExtractWVDFeatures(float *input, float *wvd_energy, float *wvd_ridge)
{
    float wvd[WVD_SIZE][WVD_SIZE];
    float energy = 0.0f;
    float ridge_sum = 0.0f;

    WVD_Compute(input, wvd);

    for (int t = 0; t < WVD_SIZE; t++) {
        float max_val = wvd[t][0];
        int max_idx = 0;

        for (int f = 0; f < WVD_SIZE; f++) {
            float v = fabsf(wvd[t][f]);
            energy += v;

            if (wvd[t][f] > max_val) {
                max_val = wvd[t][f];
                max_idx = f;
            }
        }

        ridge_sum += (float)max_idx;
    }

    *wvd_energy = energy;
    *wvd_ridge = ridge_sum / (float)WVD_SIZE;
}

/**
 * @brief   TODO: describe what DSP_Energy() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float DSP_Energy(float *x)
{
    float e = 0.0f;
    for (int i = 0; i < DSP_SIZE; i++) {
        e += x[i] * x[i];
    }
    return e;
}

/**
 * @brief   TODO: describe what DSP_Entropy() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float DSP_Entropy(float *x)
{
    float sum = 0.0f;
    float entropy = 0.0f;

    for (int i = 0; i < DSP_SIZE; i++) {
        sum += fabsf(x[i]);
    }

    for (int i = 0; i < DSP_SIZE; i++) {
        float p = fabsf(x[i]) / (sum + 1e-6f);
        if (p > 0.0f) {
            entropy -= p * logf(p);
        }
    }

    return entropy;
}

/**
 * @brief   TODO: describe what DSP_Chaos() does
 * @param   x  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static float DSP_Chaos(float *x)
{
    float chaos = 0.0f;
    for (int i = 1; i < DSP_SIZE; i++) {
        chaos += fabsf(x[i] - x[i - 1]);
    }
    return chaos;
}

/**
 * @brief   TODO: describe what DSP_Stats() does
 * @param   x  TODO: describe parameter
 * @param   mean  TODO: describe parameter
 * @param   variance  TODO: describe parameter
 * @param   peak  TODO: describe parameter
 */
static void DSP_Stats(const float *x, float *mean, float *variance, float *peak)
{
    float sum = 0.0f;
    float maxv = x[0];
    float minv = x[0];

    for (int i = 0; i < DSP_SIZE; i++) {
        sum += x[i];
        if (x[i] > maxv) {
            maxv = x[i];
        }
        if (x[i] < minv) {
            minv = x[i];
        }
    }

    *mean = sum / (float)DSP_SIZE;
    *peak = maxv;

    float var = 0.0f;
    for (int i = 0; i < DSP_SIZE; i++) {
        float d = x[i] - *mean;
        var += d * d;
    }
    *variance = var / (float)DSP_SIZE;
}

/**
 * @brief   TODO: describe what DSP_Init() does
 */
void DSP_Init(void)
{
    for (int i = 0; i < DSP_SIZE; i++) {
        weights[i] = 0.01f;
    }

    CS_Init();
}

/**
 * @brief   TODO: describe what DSP_RunPipeline() does
 * @param   input  TODO: describe parameter
 * @param   f  TODO: describe parameter
 */
void DSP_RunPipeline(float *input, DSP_Features_t *f)
{
    DSP_AdaptiveFilter(input, filtered);
    DSP_Stats(filtered, &f->mean, &f->variance, &f->peak);
    DSP_Spectral(filtered, &f->spectral_energy, &f->spectral_centroid);

    f->energy = DSP_Energy(filtered);
    f->entropy = DSP_Entropy(filtered);
    f->chaos = DSP_Chaos(filtered);
    DSP_ExtractWVDFeatures(filtered, &f->wvd_energy, &f->wvd_ridge);

    CS_Compress(filtered, cs_compressed);
    f->spectral_centroid += cs_compressed[0] * 1e-4f;
}
