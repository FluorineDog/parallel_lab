#include "common.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>

__global__ void VecAdd(float* A, float* B)
{
  size_t i = threadIdx.x +  blockDim.x * blockIdx.x;
  A[i] = A[i] + B[i];
}

float *devA;
float *devB;

void cuda_add(size_t size, float A[], float B[], float C[]) {
  assert(size <= SIZE);
  ::cudaMemcpy(devA, A, size, cudaMemcpyHostToDevice);
  ::cudaMemcpy(devB, B, size, cudaMemcpyHostToDevice);
  VecAdd<<<size/1024, 1024>>>(devA, devB);
  ::cudaMemcpy(C, devA, size, cudaMemcpyDeviceToHost);
}

int main() {
  ::cudaMalloc(&devA, SIZE * sizeof(float));
  ::cudaMalloc(&devB, SIZE * sizeof(float));
  EXEC(cuda_add);
  ::cudaFree(&devA);
  ::cudaFree(&devB);
  return 0;
}