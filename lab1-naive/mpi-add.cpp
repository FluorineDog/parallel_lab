#include "common.h"
#include <mpi.h>

void mpi_add(size_t size, double A[], double B[], double C[]) {
  cout << "hello, world" << endl;
  // for (size_t i = 0; i < size; ++i) {
  //   C[i] = B[i] + A[i];
  // }
}


int main() {
  MPI_Init(nullptr, nullptr);
  EXEC(mpi_add);
  MPI_Finalize();
  return 0;
}