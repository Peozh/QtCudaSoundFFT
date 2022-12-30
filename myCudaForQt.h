#pragma once
#define MYCUDAFORQT_EXPORTS // only activate this when building .dll
#include "cuda_runtime.h"
#include "cufft.h"
#include "helpers/helper_cuda.h"
#include "helpers/helper_functions.h"
#include "device_launch_parameters.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MYCUDAFORQT_EXPORTS
#define MYCUDAFORQT_API __declspec(dllexport)
#else
#define MYCUDAFORQT_API __declspec(dllimport)
#endif

static __global__ void cuSeparateLR(const int8_t* idata, int8_t* odata, const size_t dataCountTotal);
static __global__ void cuSeparateLR(const int16_t* idata, int16_t* odata, const size_t dataCountTotal);

static __global__ void cuScatterLR(const int8_t* idata, int8_t* odata, const size_t dataCountTotal);
static __global__ void cuScatterLR(const int16_t* idata, int16_t* odata, const size_t dataCountTotal);

static __global__ void cuCopyPCM2R(const int8_t* idata, cufftReal* odata, const size_t dataCount);
static __global__ void cuCopyPCM2R(const int16_t* idata, cufftReal* odata, const size_t dataCount);

static __global__ void cuCopyR2PCM(const cufftReal* idata, int8_t* odata);
static __global__ void cuCopyR2PCM(const cufftReal* idata, int16_t* odata);

static __global__ void cuMultFilterInplace(cufftReal* data, const float* filter);
static __global__ void cuMultFilterInplace(cufftComplex* data, const float* filter);

static __global__ void cuAddInplace(const int8_t* idata, int8_t* odata);
static __global__ void cuAddInplace(const int16_t* idata, int16_t* odata);

static __global__ void cuSqrtComplex_8(const cufftComplex* idata, cufftReal* odata);
static __global__ void cuSqrtComplex_16(const cufftComplex* idata, cufftReal* odata);

static __global__ void cuInitFloatFilter(float* data, const float initValue);
static __global__ void cuMultFloatWeight(float* data, const float weight);

static __global__ void cuNormPCM_8(const uint8_t* idata, float* odata); // [0, 1]
static __global__ void cuNormPCM_16(const int16_t* idata, float* odata); // [-1, 1]

// static __global__ void cuNormSqrtComplex_8(const cufftComplex* idata, float* odata); // [0, 1]
// static __global__ void cuNormSqrtComplex_16(const cufftComplex* idata, float* odata); // [0, 1]



#ifdef __cplusplus
extern "C" {
#endif

namespace myCUDA
{
    // MYCUDAFORQT_API __host__ void cuFFT(const int16_t* host_pcm, cufftComplex*& host_freq, const size_t dataCountPerChannel);
    // MYCUDAFORQT_API __host__ void cuFFT(const int8_t* host_pcm, cufftComplex*& host_freq, const size_t dataCountPerChannel);

    // MYCUDAFORQT_API __host__ void cuFreqFilter(const cufftComplex* host_freq_i, cufftComplex*& host_freq_o, const size_t dataCountPerChannel, const float* host_filter);

    // MYCUDAFORQT_API __host__ void cuIFFT(const cufftComplex* host_freq, int8_t*& host_pcm, const size_t dataCountPerChannel);
    // MYCUDAFORQT_API __host__ void cuIFFT(const cufftComplex* host_freq, int16_t*& host_pcm, const size_t dataCountPerChannel);

    MYCUDAFORQT_API void printCudaDevice(int argc, char **argv);

    MYCUDAFORQT_API __host__ void afterRecord_8(const int8_t* host_pcm1, const int8_t* host_pcm2, const float* host_hannFilter, void*& host_freq, const size_t dataCountPerChannel, const size_t channelCount);
    MYCUDAFORQT_API __host__ void beforePlay_8(const cufftComplex* host_freq, void*& host_freq_filtered, void*& host_pcm_filtered_1, void*& host_pcm_filtered_2, const float* host_freq_filter, const size_t dataCountPerChannel, const size_t channelCount);

    MYCUDAFORQT_API __host__ void afterRecord_16(const int16_t* host_pcm1, const int16_t* host_pcm2, const float* host_hannFilter, void*& host_freq, const size_t dataCountPerChannel, const size_t channelCount);
    MYCUDAFORQT_API __host__ void beforePlay_16(const cufftComplex* host_freq, void*& host_freq_filtered, void*& host_pcm_filtered_1, void*& host_pcm_filtered_2, const float* host_freq_filter, const size_t dataCountPerChannel, const size_t channelCount);
        
    MYCUDAFORQT_API __host__ void freePtr(void*& ptr);

    MYCUDAFORQT_API __host__ void logSqrtComplexBatch_8(const cufftComplex* host_freq, float* host_logSqrt_freq, const size_t dataCountPerChannel);
    MYCUDAFORQT_API __host__ void logSqrtComplexBatch_16(const cufftComplex* host_freq, float* host_logSqrt_freq, const size_t dataCountPerChannel);
    
    MYCUDAFORQT_API __host__ void initFreqFilter(float* host_freqFilter, const float initValue, const size_t filterSize);
    MYCUDAFORQT_API __host__ void setFreqFilter(float* pFreqFilter, const float weight, const size_t freqIdxSt, const size_t freqIdxEn);
    
    MYCUDAFORQT_API __host__ void normPCMBatch_8(const uint8_t* host_pcm, float* host_norm_pcm, const size_t dataCountPerChannel); // [-1, 1]
    MYCUDAFORQT_API __host__ void normPCMBatch_16(const int16_t* host_pcm, float* host_norm_pcm, const size_t dataCountPerChannel); // [-1, 1]

    MYCUDAFORQT_API __host__ void normSqrtComplexBatch_8(const cufftComplex* host_freq, float* host_normSqrt_freq, const size_t dataCountPerChannel); // [-1, 1]
    MYCUDAFORQT_API __host__ void normSqrtComplexBatch_16(const cufftComplex* host_freq, float* host_normSqrt_freq, const size_t dataCountPerChannel); // [-1, 1]
}

#ifdef __cplusplus
}
#endif