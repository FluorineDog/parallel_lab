#include "common.h"
void baseline_add(size_t size, float A[], float B[], float C[]) {
  for (size_t i = 0; i < size; ++i) {
    C[i] = B[i] + A[i];
  }
}

int main() {
  EXEC(baseline_add);
  return 0;
}
