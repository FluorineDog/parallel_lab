#include "common.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>



void cuda_add(size_t size, float A[], float B[], float C[]) {
  for (size_t i = 0; i < size; ++i) {
    C[i] = B[i] + A[i];
  }
}

int main() {
  EXEC(cuda_add);
  return 0;
}