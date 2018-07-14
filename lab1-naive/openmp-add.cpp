#include "common.h"
void openmp_add(size_t size, float A[], float B[], float C[]) {
#pragma omp parallel for
  for (size_t i = 0; i < size; ++i) {
    C[i] = B[i] + A[i];
  }
}

int main() {
  EXEC(openmp_add);
  return 0;
}
