#include "myCudaForQt.h"
#include <chrono>
/**/
static __global__ void cuSeparateLR(const int8_t* idata, int8_t* odata, const size_t dataCountTotal)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int targetIdx = idx;
    if (targetIdx%2 == 1) targetIdx += dataCountTotal;
    targetIdx >>= 1;
    odata[targetIdx] = idata[idx];
}
static __global__ void cuSeparateLR(const int16_t* idata, int16_t* odata, const size_t dataCountTotal)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int targetIdx = idx;
    if (targetIdx%2 == 1) targetIdx += dataCountTotal;
    targetIdx >>= 1;
    odata[targetIdx] = idata[idx];
}

static __global__ void cuScatterLR(const int8_t* idata, int8_t* odata, const size_t dataCountTotal)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int targetIdx = idx;
    targetIdx <<= 1;
    if (targetIdx >= dataCountTotal) {
        targetIdx -= dataCountTotal;
        ++targetIdx;
    }
    odata[targetIdx] = idata[idx];
}
static __global__ void cuScatterLR(const int16_t* idata, int16_t* odata, const size_t dataCountTotal)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    unsigned int targetIdx = idx;
    targetIdx <<= 1;
    if (targetIdx >= dataCountTotal) {
        targetIdx -= dataCountTotal;
        ++targetIdx;
    }
    odata[targetIdx] = idata[idx];
}

static __global__ void cuCopyPCM2R(const int8_t* idata, cufftReal* odata, const size_t dataCountPerChannel)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (cufftReal)idata[idx]/dataCountPerChannel;
}
static __global__ void cuCopyPCM2R(const int16_t* idata, cufftReal* odata, const size_t dataCountPerChannel)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (cufftReal)idata[idx]/dataCountPerChannel;
}

static __global__ void cuCopyR2PCM(const cufftReal* idata, int8_t* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (int8_t)std::round(idata[idx]);
}
static __global__ void cuCopyR2PCM(const cufftReal* idata, int16_t* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (int16_t)std::round(idata[idx]);
}

static __global__ void cuMultFilterInplace(cufftReal* data, const float* filter)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    data[idx] *= filter[idx];
}
static __global__ void cuMultFilterInplace(cuComplex* data, const float* filter)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    data[idx].x *= filter[idx];
    data[idx].y *= filter[idx];
}

static __global__ void cuAddInplace(const int8_t* idata, int8_t* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] += idata[idx];
}
static __global__ void cuAddInplace(const int16_t* idata, int16_t* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] += idata[idx];
}

static __global__ void cuSqrtComplex_8(const cufftComplex* idata, cufftReal* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = log2f(std::hypotf(idata[idx].x, idata[idx].y)*2)/8;
    if (odata[idx] < 0) odata[idx] = 0;
    if (odata[idx] > 1) odata[idx] = 1;
}
static __global__ void cuSqrtComplex_16(const cufftComplex* idata, cufftReal* odata)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = log2f(std::hypotf(idata[idx].x, idata[idx].y)*2)/15;
    if (odata[idx] < 0) odata[idx] = 0;
    if (odata[idx] > 1) odata[idx] = 1;
}

static __global__ void cuInitFloatFilter(float* data, const float initValue)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    data[idx] = initValue;
}
static __global__ void cuMultFloatWeight(float* data, const float weight)
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    data[idx] *= weight;
}

static __global__ void cuNormPCM_8(const uint8_t* idata, float* odata) // [0, 1]
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (float)idata[idx]/(1ULL<<8);
    if (odata[idx] < 0) odata[idx] = 0;
    if (odata[idx] > 1) odata[idx] = 1;
}
static __global__ void cuNormPCM_16(const int16_t* idata, float* odata) // [-1, 1]
{
    unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
    odata[idx] = (float)idata[idx]/(1ULL<<15);
    if (odata[idx] < -1) odata[idx] = -1;
    if (odata[idx] > 1) odata[idx] = 1;
}

// static __global__ void cuNormSqrtComplex_8(const cufftComplex* idata, float* odata) // [0, 1]
// {
//     unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
//     odata[idx] = std::hypotf(idata[idx].x, idata[idx].y)/(1ULL<<8);
//     if (odata[idx] < 0) odata[idx] = 0;
//     if (odata[idx] > 1) odata[idx] = 1;
// }
// static __global__ void cuNormSqrtComplex_16(const cufftComplex* idata, float* odata) // [0, 1]
// {
//     unsigned int idx = blockIdx.x*blockDim.x + threadIdx.x;
//     odata[idx] = std::hypotf(idata[idx].x, idata[idx].y)/(1ULL<<15);
//     if (odata[idx] < 0) odata[idx] = 0;
//     if (odata[idx] > 1) odata[idx] = 1;
// }
/*
__host__ void myCUDA::cuFFT(const int8_t* host_pcm, cufftComplex*& host_freq, const size_t dataCountPerChannel)
{
    // host memory allocation
    if (host_pcm == nullptr) return;
    if (host_freq == nullptr) host_freq = (cufftComplex*)malloc(sizeof(cufftComplex)*(dataCountPerChannel/2+1));
    
    // device memory allocation
    int8_t* dev_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int8_t)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_pcm, host_pcm, sizeof(int8_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
    cufftReal* dev_iR;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_iR), sizeof(cufftReal)*dataCountPerChannel));
    cufftComplex* dev_oC;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_oC), sizeof(cufftComplex)*(dataCountPerChannel/2+1)));

    // copy dev_pcm to dev_iR
    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;
    cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_iR, dataCountPerChannel); // kernel launch with dataCount blocks

    // plan & FFT
    cufftHandle plan;
    checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel, CUFFT_R2C, 1));
    checkCudaErrors(cufftExecR2C(plan, dev_iR, dev_oC));
    checkCudaErrors(cufftDestroy(plan));

    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_freq, dev_oC, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_pcm));
    checkCudaErrors(cudaFree(dev_iR));
    checkCudaErrors(cudaFree(dev_oC));
}
__host__ void myCUDA::cuFFT(const int16_t* host_pcm, cufftComplex*& host_freq, const size_t dataCountPerChannel)
{
    // host memory allocation
    if (host_pcm == nullptr) return;
    if (host_freq == nullptr) host_freq = (cufftComplex*)malloc(sizeof(cufftComplex)*(dataCountPerChannel/2+1));
    
    // device memory allocation
    int16_t* dev_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int16_t)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_pcm, host_pcm, sizeof(int16_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
    cufftReal* dev_iR;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_iR), sizeof(cufftReal)*dataCountPerChannel));
    cufftComplex* dev_oC;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_oC), sizeof(cufftComplex)*(dataCountPerChannel/2+1)));

    // copy dev_pcm to dev_iR
    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;
    cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_iR, dataCountPerChannel); // kernel launch with dataCount blocks

    // plan & FFT
    cufftHandle plan;
    checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel, CUFFT_R2C, 1));
    checkCudaErrors(cufftExecR2C(plan, dev_iR, dev_oC));
    checkCudaErrors(cufftDestroy(plan));

    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_freq, dev_oC, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_pcm));
    checkCudaErrors(cudaFree(dev_iR));
    checkCudaErrors(cudaFree(dev_oC));
}

__host__ void myCUDA::cuFreqFilter(const cufftComplex* host_freq_i, cufftComplex*& host_freq_o, const size_t dataCountPerChannel, const float* host_filter)
{
    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = (dataCountPerChannel/2+threadsPerBlock)/threadsPerBlock;

    // host memory allocation
    if (host_filter == nullptr) return;
    if (host_freq_i == nullptr) return;
    if (host_freq_o == nullptr)
        host_freq_o = (cufftComplex*)malloc(sizeof(cufftComplex)*(dataCountPerChannel/2+1));

    // device memory allocation
    cufftComplex* dev_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel/2+threadsPerBlock)));
    checkCudaErrors(cudaMemcpy(dev_freq, host_freq_i, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyHostToDevice));
    float* dev_filter;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_filter), sizeof(float)*(dataCountPerChannel/2+threadsPerBlock)));
    checkCudaErrors(cudaMemcpy(dev_filter, host_filter, sizeof(float)*(dataCountPerChannel/2+1), cudaMemcpyHostToDevice));

    // multiply filter
    cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_freq, dev_filter);

    // copy result from device to host
    checkCudaErrors(cudaMemcpy(host_freq_o, dev_freq, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freq));
    checkCudaErrors(cudaFree(dev_filter));
}

__host__ void myCUDA::cuIFFT(const cufftComplex* host_freq, int8_t*& host_pcm, const size_t dataCountPerChannel)
{
    // host memory allocation
    if (host_freq == nullptr) return;
    if (host_pcm == nullptr)
        host_pcm = (int8_t*)malloc(sizeof(int8_t)*dataCountPerChannel);

    // device memory allocation
    cufftComplex* dev_iC;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_iC), sizeof(cufftComplex)*(dataCountPerChannel/2+1)));
    checkCudaErrors(cudaMemcpy(dev_iC, host_freq, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyHostToDevice));
    cufftReal* dev_oR;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_oR), sizeof(cufftReal)*dataCountPerChannel));
    int8_t* dev_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int8_t)*dataCountPerChannel));

    // plan & FFT
    cufftHandle plan;
    checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel, CUFFT_C2R, 1));
    checkCudaErrors(cufftExecC2R(plan, dev_iC, dev_oR));
    checkCudaErrors(cufftDestroy(plan));

    // copy transformed result from device to host
    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;
    cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_oR, dev_pcm);
    checkCudaErrors(cudaMemcpy(host_pcm, dev_pcm, sizeof(int8_t)*dataCountPerChannel, cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_iC));
    checkCudaErrors(cudaFree(dev_oR));
    checkCudaErrors(cudaFree(dev_pcm));
}
__host__ void myCUDA::cuIFFT(const cufftComplex* host_freq, int16_t*& host_pcm, const size_t dataCountPerChannel)
{
    // host memory allocation
    if (host_freq == nullptr) return;
    if (host_pcm == nullptr)
        host_pcm = (int16_t*)malloc(sizeof(int16_t)*dataCountPerChannel);

    // device memory allocation
    cufftComplex* dev_iC;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_iC), sizeof(cufftComplex)*(dataCountPerChannel/2+1)));
    checkCudaErrors(cudaMemcpy(dev_iC, host_freq, sizeof(cufftComplex)*(dataCountPerChannel/2+1), cudaMemcpyHostToDevice));
    cufftReal* dev_oR;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_oR), sizeof(cufftReal)*dataCountPerChannel));
    int16_t* dev_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int16_t)*dataCountPerChannel));

    // plan & FFT
    cufftHandle plan;
    checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel, CUFFT_C2R, 1));
    checkCudaErrors(cufftExecC2R(plan, dev_iC, dev_oR));
    checkCudaErrors(cufftDestroy(plan));

    // copy transformed result from device to host
    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;
    cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_oR, dev_pcm);
    checkCudaErrors(cudaMemcpy(host_pcm, dev_pcm, sizeof(int16_t)*dataCountPerChannel, cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_iC));
    checkCudaErrors(cudaFree(dev_oR));
    checkCudaErrors(cudaFree(dev_pcm));
}
*/
void myCUDA::printCudaDevice(int argc, char **argv)
{
    printf("Printing CUDA Device... [%s]\n", __FILE__);
    findCudaDevice(argc, (const char **)argv);
}

void myCUDA::afterRecord_8(const int8_t* host_pcm1, const int8_t* host_pcm2, const float* host_hannFilter, void*& host_freq, const size_t dataCountPerChannel, const size_t channelCount)
{
    // [afterRecord]
    // (host pcm) 
    // 두개 연결 -> 채널 분리 -> 채널별 한필터 -> fft 
    // (host freq)

    // host memory allocation
    if (host_hannFilter == nullptr) return;
    if (host_pcm1 == nullptr) return;
    if (host_pcm2 == nullptr) return;
    // if (host_freq == nullptr) host_freq = malloc(sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount);
    if (host_freq == nullptr) checkCudaErrors(cudaMallocHost(&host_freq, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount));

    if (channelCount == 1)
    {
        // allocate device memory
        int8_t* dev_pcm;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int8_t)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_pcm, host_pcm1, sizeof(int8_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_pcm + dataCountPerChannel, host_pcm2, sizeof(int8_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
        
        constexpr size_t threadsPerBlock = 32;
        size_t blockCount = dataCountPerChannel*2/threadsPerBlock;

        // PCM to Real
        cufftReal* dev_real;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real), sizeof(cufftReal)*dataCountPerChannel*2));
        cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_real, dataCountPerChannel*2);

        // Hann filter
        float* dev_hannFilter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_hannFilter), sizeof(float)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_hannFilter, host_hannFilter, sizeof(float)*dataCountPerChannel*2, cudaMemcpyHostToDevice));
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real, dev_hannFilter);

        // FFT
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)));
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_R2C, 1));
        checkCudaErrors(cufftExecR2C(plan, dev_real, dev_freq));
        checkCudaErrors(cufftDestroy(plan));

        // copy transformed result from device to host
        checkCudaErrors(cudaMemcpy(host_freq, dev_freq, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_pcm));
        checkCudaErrors(cudaFree(dev_real));
        checkCudaErrors(cudaFree(dev_hannFilter));
        checkCudaErrors(cudaFree(dev_freq));
    }
    if (channelCount == 2)
    {
        // allocate device memory
        int8_t* dev_pcm_scatter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_scatter), sizeof(int8_t)*dataCountPerChannel*channelCount*2));
        checkCudaErrors(cudaMemcpy(dev_pcm_scatter, host_pcm1, sizeof(int8_t)*dataCountPerChannel*channelCount, cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_pcm_scatter + dataCountPerChannel*channelCount, host_pcm2, sizeof(int8_t)*dataCountPerChannel*channelCount, cudaMemcpyHostToDevice));
        int8_t* dev_pcm_separate;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_separate), sizeof(int8_t)*dataCountPerChannel*channelCount*2));

        // separate channel LLLLL...RRRR...
        constexpr size_t threadsPerBlock = 32;
        size_t blockCount = dataCountPerChannel*channelCount*2/threadsPerBlock;
        cuSeparateLR<<<blockCount, threadsPerBlock>>>(dev_pcm_scatter, dev_pcm_separate, dataCountPerChannel*channelCount*2);
        // int8_t* dev_pcm_L = dev_pcm_separate;
        // int8_t* dev_pcm_R = dev_pcm_separate + dataCountPerChannel*2;

        // PCM to Real
        cufftReal* dev_real;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real), sizeof(cufftReal)*dataCountPerChannel*channelCount*2));
        cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm_separate, dev_real, dataCountPerChannel*2);
        cufftReal* dev_real_L = dev_real;
        cufftReal* dev_real_R = dev_real + dataCountPerChannel*2;

        // Hann filter each channel
        blockCount = dataCountPerChannel*2/threadsPerBlock;
        float* dev_hannFilter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_hannFilter), sizeof(float)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_hannFilter, host_hannFilter, sizeof(float)*dataCountPerChannel*2, cudaMemcpyHostToDevice));
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real_L, dev_hannFilter);
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real_R, dev_hannFilter);

        // FFT each channel
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount));
        cufftComplex* dev_freq_L = dev_freq;
        cufftComplex* dev_freq_R = dev_freq + dataCountPerChannel+1;
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_R2C, 1));
        checkCudaErrors(cufftExecR2C(plan, dev_real_L, dev_freq_L));
        checkCudaErrors(cufftExecR2C(plan, dev_real_R, dev_freq_R));
        checkCudaErrors(cufftDestroy(plan));

        // copy transformed result from device to host
        checkCudaErrors(cudaMemcpy(host_freq, dev_freq, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount, cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_pcm_scatter));
        checkCudaErrors(cudaFree(dev_pcm_separate));
        checkCudaErrors(cudaFree(dev_real));
        checkCudaErrors(cudaFree(dev_hannFilter));
        checkCudaErrors(cudaFree(dev_freq));
    }
}
void myCUDA::beforePlay_8(const cufftComplex* host_freq, void*& host_freq_filtered, void*& host_pcm_filtered_1, void*& host_pcm_filtered_2, const float* host_freq_filter, const size_t dataCountPerChannel, const size_t channelCount)
{
    // [beforePlay]
    // (host freq)
    // -> freq filter each channel
    // (host freq filtered)
    // -> ifft each channel -> real to pcm -> 채널 믹스 -> batch 두개 분리 -> 호스트 db 에 각각 더해주기
    // (host pcm filtered)
    
    // host memory allocation
    if (host_freq == nullptr) return;
    if (host_freq_filter == nullptr) return;
    // if (host_freq_filtered == nullptr) host_freq_filtered = malloc(sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount);
    if (host_freq_filtered == nullptr) checkCudaErrors(cudaMallocHost(&host_freq_filtered, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount));
    if (host_pcm_filtered_1 == nullptr) host_pcm_filtered_1 = calloc(dataCountPerChannel*channelCount, sizeof(int8_t));
    if (host_pcm_filtered_2 == nullptr) host_pcm_filtered_2 = malloc(sizeof(int8_t)*dataCountPerChannel*channelCount);

    if (channelCount == 1)
    {
        constexpr size_t threadsPerBlock = 32;

        // allocate device memory
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq, host_freq, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyHostToDevice));
        float* dev_freq_filter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq_filter), sizeof(float)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq_filter, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));

        // freq filter
        size_t blockCount = (dataCountPerChannel+threadsPerBlock)*channelCount/threadsPerBlock;
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_freq, dev_freq_filter);
        cufftComplex* dev_freq_filtered = dev_freq;

        // copy filtered freq from dev to host
        checkCudaErrors(cudaMemcpy(host_freq_filtered, dev_freq_filtered, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyDeviceToHost));

        // ifft
        cufftReal* dev_real_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real_filtered), sizeof(cufftReal)*(dataCountPerChannel*2)*channelCount));
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_C2R, 1));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered, dev_real_filtered));
        checkCudaErrors(cufftDestroy(plan));

        // real to PCM
        int8_t* dev_pcm_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered), sizeof(int8_t)*(dataCountPerChannel*2)*channelCount));
        blockCount = (dataCountPerChannel*2)*channelCount/threadsPerBlock;
        cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_real_filtered, dev_pcm_filtered);
        
        // add each batch to host 
        int8_t* dev_pcm_filtered_new_1 = dev_pcm_filtered;
        int8_t* dev_pcm_filtered_new_2 = dev_pcm_filtered + dataCountPerChannel*channelCount;
        int8_t* dev_pcm_filtered_old_1;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_old_1), sizeof(int8_t)*dataCountPerChannel*channelCount));
        checkCudaErrors(cudaMemcpy(dev_pcm_filtered_old_1, host_pcm_filtered_1, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyHostToDevice));
        blockCount = (dataCountPerChannel)*channelCount/threadsPerBlock;
        cuAddInplace<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_new_1, dev_pcm_filtered_old_1);
        int8_t* dev_pcm_filtered_sum_1 = dev_pcm_filtered_old_1;
        int8_t* dev_pcm_filtered_sum_2 = dev_pcm_filtered_new_2;
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_1, dev_pcm_filtered_sum_1, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_2, dev_pcm_filtered_sum_2, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_freq));
        checkCudaErrors(cudaFree(dev_freq_filter));
        checkCudaErrors(cudaFree(dev_real_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered_old_1));
    }
    else if (channelCount == 2)
    {
        constexpr size_t threadsPerBlock = 32;

        // allocate device memory
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq, host_freq, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyHostToDevice));
        float* dev_freq_filter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq_filter), sizeof(float)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        float* dev_freq_filter_L = dev_freq_filter;
        float* dev_freq_filter_R = dev_freq_filter + dataCountPerChannel+1;
        checkCudaErrors(cudaMemcpy(dev_freq_filter_L, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_freq_filter_R, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));

        // freq filter each channel
        size_t blockCount = (dataCountPerChannel+threadsPerBlock)*channelCount/threadsPerBlock;
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_freq, dev_freq_filter);
        cufftComplex* dev_freq_filtered = dev_freq;
        cufftComplex* dev_freq_filtered_L = dev_freq;
        cufftComplex* dev_freq_filtered_R = dev_freq + dataCountPerChannel+1;

        // copy filtered freq from dev to host
        checkCudaErrors(cudaMemcpy(host_freq_filtered, dev_freq_filtered, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyDeviceToHost));

        // ifft each channel
        cufftReal* dev_real_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real_filtered), sizeof(cufftReal)*(dataCountPerChannel*2)*channelCount));
        cufftReal* dev_real_filtered_L = dev_real_filtered;
        cufftReal* dev_real_filtered_R = dev_real_filtered + dataCountPerChannel*2;
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_C2R, 1));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered_L, dev_real_filtered_L));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered_R, dev_real_filtered_R));
        checkCudaErrors(cufftDestroy(plan));

        // real to PCM
        int8_t* dev_pcm_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered), sizeof(int8_t)*(dataCountPerChannel*2)*channelCount));
        blockCount = (dataCountPerChannel*2)*channelCount/threadsPerBlock;
        cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_real_filtered, dev_pcm_filtered);
        int8_t* dev_pcm_filtered_separate = dev_pcm_filtered;

        // scatter channel LRLRLR... : 
        int8_t* dev_pcm_filtered_scatter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_scatter), sizeof(int8_t)*(dataCountPerChannel*2)*channelCount));
        cuScatterLR<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_separate, dev_pcm_filtered_scatter, dataCountPerChannel*2*channelCount);

        // add each batch to host 
        int8_t* dev_pcm_filtered_new_1 = dev_pcm_filtered_scatter;
        int8_t* dev_pcm_filtered_new_2 = dev_pcm_filtered_scatter + dataCountPerChannel*channelCount;
        int8_t* dev_pcm_filtered_old_1;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_old_1), sizeof(int8_t)*dataCountPerChannel*channelCount));
        checkCudaErrors(cudaMemcpy(dev_pcm_filtered_old_1, host_pcm_filtered_1, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyHostToDevice));
        blockCount = (dataCountPerChannel)*channelCount/threadsPerBlock;
        cuAddInplace<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_new_1, dev_pcm_filtered_old_1);
        int8_t* dev_pcm_filtered_sum_1 = dev_pcm_filtered_old_1;
        int8_t* dev_pcm_filtered_sum_2 = dev_pcm_filtered_new_2;
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_1, dev_pcm_filtered_sum_1, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_2, dev_pcm_filtered_sum_2, sizeof(int8_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_freq));
        checkCudaErrors(cudaFree(dev_freq_filter));
        checkCudaErrors(cudaFree(dev_real_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered_scatter));
        checkCudaErrors(cudaFree(dev_pcm_filtered_old_1));
    }
}

void myCUDA::afterRecord_16(const int16_t* host_pcm1, const int16_t* host_pcm2, const float* host_hannFilter, void*& host_freq, const size_t dataCountPerChannel, const size_t channelCount)
{
    // [afterRecord]
    // (host pcm) 
    // 두개 연결 -> 채널 분리 -> 채널별 한필터 -> fft 
    // (host freq)

    // host memory allocation
    if (host_pcm1 == nullptr) return;
    if (host_pcm2 == nullptr) return;
    if (host_freq == nullptr) host_freq = malloc(sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount);

    if (channelCount == 1)
    {
        // allocate device memory
        int16_t* dev_pcm;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int16_t)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_pcm, host_pcm1, sizeof(int16_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_pcm + dataCountPerChannel, host_pcm2, sizeof(int16_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
        
        constexpr size_t threadsPerBlock = 32;
        size_t blockCount = dataCountPerChannel*2/threadsPerBlock;

        // PCM to Real
        cufftReal* dev_real;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real), sizeof(cufftReal)*dataCountPerChannel*2));
        cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_real, dataCountPerChannel*2);

        // Hann filter
        float* dev_hannFilter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_hannFilter), sizeof(float)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_hannFilter, host_hannFilter, sizeof(float)*dataCountPerChannel*2, cudaMemcpyHostToDevice));
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real, dev_hannFilter);

        // FFT
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)));
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_R2C, 1));
        checkCudaErrors(cufftExecR2C(plan, dev_real, dev_freq));
        checkCudaErrors(cufftDestroy(plan));

        // copy transformed result from device to host
        checkCudaErrors(cudaMemcpy(host_freq, dev_freq, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_pcm));
        checkCudaErrors(cudaFree(dev_real));
        checkCudaErrors(cudaFree(dev_hannFilter));
        checkCudaErrors(cudaFree(dev_freq));
    }
    if (channelCount == 2)
    {
        // allocate device memory
        int16_t* dev_pcm_scatter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_scatter), sizeof(int16_t)*dataCountPerChannel*channelCount*2));
        checkCudaErrors(cudaMemcpy(dev_pcm_scatter, host_pcm1, sizeof(int16_t)*dataCountPerChannel*channelCount, cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_pcm_scatter + dataCountPerChannel*channelCount, host_pcm2, sizeof(int16_t)*dataCountPerChannel*channelCount, cudaMemcpyHostToDevice));
        int16_t* dev_pcm_separate;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_separate), sizeof(int16_t)*dataCountPerChannel*channelCount*2));

        // separate channel LLLLL...RRRR...
        constexpr size_t threadsPerBlock = 32;
        size_t blockCount = dataCountPerChannel*channelCount*2/threadsPerBlock;
        cuSeparateLR<<<blockCount, threadsPerBlock>>>(dev_pcm_scatter, dev_pcm_separate, dataCountPerChannel*channelCount*2);
        // int16_t* dev_pcm_L = dev_pcm_separate;
        // int16_t* dev_pcm_R = dev_pcm_separate + dataCountPerChannel*2;

        // PCM to Real
        cufftReal* dev_real;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real), sizeof(cufftReal)*dataCountPerChannel*channelCount*2));
        cuCopyPCM2R<<<blockCount, threadsPerBlock>>>(dev_pcm_separate, dev_real, dataCountPerChannel*2);
        cufftReal* dev_real_L = dev_real;
        cufftReal* dev_real_R = dev_real + dataCountPerChannel*2;

        // Hann filter each channel
        blockCount = dataCountPerChannel*2/threadsPerBlock;
        float* dev_hannFilter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_hannFilter), sizeof(float)*dataCountPerChannel*2));
        checkCudaErrors(cudaMemcpy(dev_hannFilter, host_hannFilter, sizeof(float)*dataCountPerChannel*2, cudaMemcpyHostToDevice));
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real_L, dev_hannFilter);
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_real_R, dev_hannFilter);

        // FFT each channel
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount));
        cufftComplex* dev_freq_L = dev_freq;
        cufftComplex* dev_freq_R = dev_freq + dataCountPerChannel+1;
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_R2C, 1));
        checkCudaErrors(cufftExecR2C(plan, dev_real_L, dev_freq_L));
        checkCudaErrors(cufftExecR2C(plan, dev_real_R, dev_freq_R));
        checkCudaErrors(cufftDestroy(plan));

        // copy transformed result from device to host
        checkCudaErrors(cudaMemcpy(host_freq, dev_freq, sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount, cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_pcm_scatter));
        checkCudaErrors(cudaFree(dev_pcm_separate));
        checkCudaErrors(cudaFree(dev_real));
        checkCudaErrors(cudaFree(dev_hannFilter));
        checkCudaErrors(cudaFree(dev_freq));
    }
}
void myCUDA::beforePlay_16(const cufftComplex* host_freq, void*& host_freq_filtered, void*& host_pcm_filtered_1, void*& host_pcm_filtered_2, const float* host_freq_filter, const size_t dataCountPerChannel, const size_t channelCount)
{
    // [beforePlay]
    // (host freq)
    // -> freq filter each channel
    // (host freq filtered)
    // -> ifft each channel -> real to pcm -> 채널 믹스 -> batch 두개 분리 -> 호스트 db 에 각각 더해주기
    // (host pcm filtered)
    
    // host memory allocation
    if (host_freq == nullptr) return;
    if (host_freq_filter == nullptr) return;
    if (host_freq_filtered == nullptr) host_freq_filtered = malloc(sizeof(cufftComplex)*(dataCountPerChannel*2/2+1)*channelCount);
    if (host_pcm_filtered_1 == nullptr) host_pcm_filtered_1 = calloc(dataCountPerChannel*channelCount, sizeof(int16_t));
    if (host_pcm_filtered_2 == nullptr) host_pcm_filtered_2 = malloc(sizeof(int16_t)*dataCountPerChannel*channelCount);


    if (channelCount == 1)
    {
        constexpr size_t threadsPerBlock = 32;

        // allocate device memory
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq, host_freq, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyHostToDevice));
        float* dev_freq_filter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq_filter), sizeof(float)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq_filter, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));

        // freq filter
        size_t blockCount = (dataCountPerChannel+threadsPerBlock)*channelCount/threadsPerBlock;
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_freq, dev_freq_filter);
        cufftComplex* dev_freq_filtered = dev_freq;

        // copy filtered freq from dev to host
        checkCudaErrors(cudaMemcpy(host_freq_filtered, dev_freq_filtered, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyDeviceToHost));

        // ifft
        cufftReal* dev_real_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real_filtered), sizeof(cufftReal)*(dataCountPerChannel*2)*channelCount));
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_C2R, 1));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered, dev_real_filtered));
        checkCudaErrors(cufftDestroy(plan));

        // real to PCM
        int16_t* dev_pcm_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered), sizeof(int16_t)*(dataCountPerChannel*2)*channelCount));
        blockCount = (dataCountPerChannel*2)*channelCount/threadsPerBlock;
        cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_real_filtered, dev_pcm_filtered);

        // add each batch to host 
        int16_t* dev_pcm_filtered_new_1 = dev_pcm_filtered;
        int16_t* dev_pcm_filtered_new_2 = dev_pcm_filtered + dataCountPerChannel*channelCount;
        int16_t* dev_pcm_filtered_old_1;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_old_1), sizeof(int16_t)*dataCountPerChannel*channelCount));
        checkCudaErrors(cudaMemcpy(dev_pcm_filtered_old_1, host_pcm_filtered_1, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyHostToDevice));
        blockCount = (dataCountPerChannel)*channelCount/threadsPerBlock;
        cuAddInplace<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_new_1, dev_pcm_filtered_old_1);
        int16_t* dev_pcm_filtered_sum_1 = dev_pcm_filtered_old_1;
        int16_t* dev_pcm_filtered_sum_2 = dev_pcm_filtered_new_2;
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_1, dev_pcm_filtered_sum_1, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_2, dev_pcm_filtered_sum_2, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_freq));
        checkCudaErrors(cudaFree(dev_freq_filter));
        checkCudaErrors(cudaFree(dev_real_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered_old_1));
    }
    else if (channelCount == 2)
    {
        constexpr size_t threadsPerBlock = 32;

        // allocate device memory
        cufftComplex* dev_freq;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        checkCudaErrors(cudaMemcpy(dev_freq, host_freq, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyHostToDevice));
        float* dev_freq_filter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq_filter), sizeof(float)*(dataCountPerChannel+threadsPerBlock)*channelCount));
        float* dev_freq_filter_L = dev_freq_filter;
        float* dev_freq_filter_R = dev_freq_filter + dataCountPerChannel+1;
        checkCudaErrors(cudaMemcpy(dev_freq_filter_L, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));
        checkCudaErrors(cudaMemcpy(dev_freq_filter_R, host_freq_filter, sizeof(float)*(dataCountPerChannel+1), cudaMemcpyHostToDevice));

        // freq filter each channel
        size_t blockCount = (dataCountPerChannel+threadsPerBlock)*channelCount/threadsPerBlock;
        cuMultFilterInplace<<<blockCount, threadsPerBlock>>>(dev_freq, dev_freq_filter);
        cufftComplex* dev_freq_filtered = dev_freq;
        cufftComplex* dev_freq_filtered_L = dev_freq;
        cufftComplex* dev_freq_filtered_R = dev_freq + dataCountPerChannel+1;

        // copy filtered freq from dev to host
        checkCudaErrors(cudaMemcpy(host_freq_filtered, dev_freq_filtered, sizeof(cufftComplex)*(dataCountPerChannel+1)*channelCount, cudaMemcpyDeviceToHost));

        // ifft each channel
        cufftReal* dev_real_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_real_filtered), sizeof(cufftReal)*(dataCountPerChannel*2)*channelCount));
        cufftReal* dev_real_filtered_L = dev_real_filtered;
        cufftReal* dev_real_filtered_R = dev_real_filtered + dataCountPerChannel*2;
        cufftHandle plan;
        checkCudaErrors(cufftPlan1d(&plan, dataCountPerChannel*2, CUFFT_C2R, 1));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered_L, dev_real_filtered_L));
        checkCudaErrors(cufftExecC2R(plan, dev_freq_filtered_R, dev_real_filtered_R));
        checkCudaErrors(cufftDestroy(plan));

        // real to PCM
        int16_t* dev_pcm_filtered;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered), sizeof(int16_t)*(dataCountPerChannel*2)*channelCount));
        blockCount = (dataCountPerChannel*2)*channelCount/threadsPerBlock;
        cuCopyR2PCM<<<blockCount, threadsPerBlock>>>(dev_real_filtered, dev_pcm_filtered);
        int16_t* dev_pcm_filtered_separate = dev_pcm_filtered;

        // scatter channel LRLRLR... : 
        int16_t* dev_pcm_filtered_scatter;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_scatter), sizeof(int16_t)*(dataCountPerChannel*2)*channelCount));
        cuScatterLR<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_separate, dev_pcm_filtered_scatter, dataCountPerChannel*2*channelCount);

        // add each batch to host 
        int16_t* dev_pcm_filtered_new_1 = dev_pcm_filtered_scatter;
        int16_t* dev_pcm_filtered_new_2 = dev_pcm_filtered_scatter + dataCountPerChannel*channelCount;
        int16_t* dev_pcm_filtered_old_1;
        checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm_filtered_old_1), sizeof(int16_t)*dataCountPerChannel*channelCount));
        checkCudaErrors(cudaMemcpy(dev_pcm_filtered_old_1, host_pcm_filtered_1, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyHostToDevice));
        blockCount = (dataCountPerChannel)*channelCount/threadsPerBlock;
        cuAddInplace<<<blockCount, threadsPerBlock>>>(dev_pcm_filtered_new_1, dev_pcm_filtered_old_1);
        int16_t* dev_pcm_filtered_sum_1 = dev_pcm_filtered_old_1;
        int16_t* dev_pcm_filtered_sum_2 = dev_pcm_filtered_new_2;
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_1, dev_pcm_filtered_sum_1, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(host_pcm_filtered_2, dev_pcm_filtered_sum_2, sizeof(int16_t)*(dataCountPerChannel*channelCount), cudaMemcpyDeviceToHost));

        // free device(gpu) memory
        checkCudaErrors(cudaFree(dev_freq));
        checkCudaErrors(cudaFree(dev_freq_filter));
        checkCudaErrors(cudaFree(dev_real_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered));
        checkCudaErrors(cudaFree(dev_pcm_filtered_scatter));
        checkCudaErrors(cudaFree(dev_pcm_filtered_old_1));
    }
}

void myCUDA::freePtr(void*& ptr)
{
    if (ptr == nullptr) return;
    // free(ptr);
    cudaFreeHost(ptr);
    ptr = nullptr;
}

void myCUDA::logSqrtComplexBatch_8(const cufftComplex* host_freq, float* host_logSqrt_freq, const size_t dataCountPerChannel)
{
    if (host_freq == nullptr) return;
    if (host_logSqrt_freq == nullptr) return;
    
    // device memory allocation
    cufftComplex* dev_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_freq, host_freq+1, sizeof(cufftComplex)*dataCountPerChannel, cudaMemcpyHostToDevice));
    float* dev_logSqrt_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_logSqrt_freq), sizeof(float)*dataCountPerChannel));

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // log2f(hypotf(complex))/7
    cuSqrtComplex_8<<<blockCount, threadsPerBlock>>>(dev_freq, dev_logSqrt_freq);
    
    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_logSqrt_freq, dev_logSqrt_freq, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freq));
    checkCudaErrors(cudaFree(dev_logSqrt_freq));
}
void myCUDA::logSqrtComplexBatch_16(const cufftComplex* host_freq, float* host_logSqrt_freq, const size_t dataCountPerChannel)
{
    if (host_freq == nullptr) return;
    if (host_logSqrt_freq == nullptr) return;
    
    // device memory allocation
    cufftComplex* dev_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_freq, host_freq+1, sizeof(cufftComplex)*dataCountPerChannel, cudaMemcpyHostToDevice));
    float* dev_logSqrt_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_logSqrt_freq), sizeof(float)*dataCountPerChannel));

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // log2f(hypotf(complex))/15
    cuSqrtComplex_16<<<blockCount, threadsPerBlock>>>(dev_freq, dev_logSqrt_freq);
    
    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_logSqrt_freq, dev_logSqrt_freq, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freq));
    checkCudaErrors(cudaFree(dev_logSqrt_freq));
}

void myCUDA::initFreqFilter(float* host_freqFilter, const float initValue, const size_t filterSize)
{
    if (host_freqFilter == nullptr) return;

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = filterSize/threadsPerBlock + 1;
    
    // device memory allocation
    float* dev_freqFilter;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freqFilter), sizeof(float)*blockCount*threadsPerBlock));
    checkCudaErrors(cudaMemcpy(dev_freqFilter, host_freqFilter, sizeof(float)*filterSize, cudaMemcpyHostToDevice));
    
    // cuMultFloat
    cuInitFloatFilter<<<blockCount, threadsPerBlock>>>(dev_freqFilter, initValue);

    // memcpy device to host
    checkCudaErrors(cudaMemcpy(host_freqFilter, dev_freqFilter, sizeof(float)*filterSize, cudaMemcpyDeviceToHost));
    
    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freqFilter));
}
void myCUDA::setFreqFilter(float* host_freqFilter, const float weight, const size_t freqIdxSt, const size_t freqIdxEn)
{
    if (host_freqFilter == nullptr) return;

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = (freqIdxEn - freqIdxSt + 1)/threadsPerBlock + 1;

    // device memory allocation
    float* dev_freqFilter;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freqFilter), sizeof(float)*blockCount*threadsPerBlock));
    checkCudaErrors(cudaMemcpy(dev_freqFilter, host_freqFilter+freqIdxSt, sizeof(float)*(freqIdxEn - freqIdxSt + 1), cudaMemcpyHostToDevice));
    
    // cuMultFloat
    cuMultFloatWeight<<<blockCount, threadsPerBlock>>>(dev_freqFilter, weight);

    // memcpy device to host
    checkCudaErrors(cudaMemcpy(host_freqFilter+freqIdxSt, dev_freqFilter, sizeof(float)*(freqIdxEn - freqIdxSt + 1), cudaMemcpyDeviceToHost));
    
    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freqFilter));
}

void myCUDA::normPCMBatch_8(const uint8_t* host_pcm, float* host_norm_pcm, const size_t dataCountPerChannel)
{
    auto mallocSt = std::chrono::steady_clock::now();
    if (host_pcm == nullptr) return;
    if (host_norm_pcm == nullptr) return;
    
    // device memory allocation
    uint8_t* dev_pcm;
    cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(uint8_t)*dataCountPerChannel);
    cudaMemcpy(dev_pcm, host_pcm, sizeof(uint8_t)*dataCountPerChannel, cudaMemcpyHostToDevice);
    float* dev_norm_pcm;
    cudaMalloc(reinterpret_cast<void **>(&dev_norm_pcm), sizeof(float)*dataCountPerChannel);

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // pcm/2^8
    cuNormPCM_8<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_norm_pcm);
    
    // copy transformed result from device to host
    cudaMemcpy(host_norm_pcm, dev_norm_pcm, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost);

    // free device(gpu) memory
    cudaFree(dev_pcm);
    cudaFree(dev_norm_pcm);
    auto mallocEn = std::chrono::steady_clock::now();
    auto mallocDuration = mallocEn-mallocSt;
    std::cout << "\tcuda total duration [ns] : " << mallocDuration.count() << std::endl;
}
void myCUDA::normPCMBatch_16(const int16_t* host_pcm, float* host_norm_pcm, const size_t dataCountPerChannel)
{
    if (host_pcm == nullptr) return;
    if (host_norm_pcm == nullptr) return;
    
    // device memory allocation
    int16_t* dev_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_pcm), sizeof(int16_t)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_pcm, host_pcm, sizeof(int16_t)*dataCountPerChannel, cudaMemcpyHostToDevice));
    float* dev_norm_pcm;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_norm_pcm), sizeof(float)*dataCountPerChannel));

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // pcm/2^15
    cuNormPCM_16<<<blockCount, threadsPerBlock>>>(dev_pcm, dev_norm_pcm);
    
    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_norm_pcm, dev_norm_pcm, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_pcm));
    checkCudaErrors(cudaFree(dev_norm_pcm));
}

void myCUDA::normSqrtComplexBatch_8(const cufftComplex* host_freq, float* host_normSqrt_freq, const size_t dataCountPerChannel)
{
    if (host_freq == nullptr) return;
    if (host_normSqrt_freq == nullptr) return;
    
    // device memory allocation
    cufftComplex* dev_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_freq, host_freq+1, sizeof(cufftComplex)*dataCountPerChannel, cudaMemcpyHostToDevice));
    float* dev_normSqrt_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_normSqrt_freq), sizeof(float)*dataCountPerChannel));

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // pcm/2^7
    cuSqrtComplex_8<<<blockCount, threadsPerBlock>>>(dev_freq, dev_normSqrt_freq);
    
    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_normSqrt_freq, dev_normSqrt_freq, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freq));
    checkCudaErrors(cudaFree(dev_normSqrt_freq));
}
void myCUDA::normSqrtComplexBatch_16(const cufftComplex* host_freq, float* host_normSqrt_freq, const size_t dataCountPerChannel)
{
    if (host_freq == nullptr) return;
    if (host_normSqrt_freq == nullptr) return;
    
    // device memory allocation
    cufftComplex* dev_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_freq), sizeof(cufftComplex)*dataCountPerChannel));
    checkCudaErrors(cudaMemcpy(dev_freq, host_freq+1, sizeof(cufftComplex)*dataCountPerChannel, cudaMemcpyHostToDevice));
    float* dev_normSqrt_freq;
    checkCudaErrors(cudaMalloc(reinterpret_cast<void **>(&dev_normSqrt_freq), sizeof(float)*dataCountPerChannel));

    constexpr size_t threadsPerBlock = 32;
    size_t blockCount = dataCountPerChannel/threadsPerBlock;

    // pcm/2^7
    cuSqrtComplex_16<<<blockCount, threadsPerBlock>>>(dev_freq, dev_normSqrt_freq);
    
    // copy transformed result from device to host
    checkCudaErrors(cudaMemcpy(host_normSqrt_freq, dev_normSqrt_freq, sizeof(float)*(dataCountPerChannel), cudaMemcpyDeviceToHost));

    // free device(gpu) memory
    checkCudaErrors(cudaFree(dev_freq));
    checkCudaErrors(cudaFree(dev_normSqrt_freq));
}

// todo : 가독성 개선 측면에서 void* 다시 원래대로 복구 후 테스트해보기
// todo : frequency complex 에서 frequency sqrt value glfloat 로 변환하는 __global__ 만들기, 이를 사용하는 __host__ 만들기, 성능테스트로 비교해보기 : 250,000
