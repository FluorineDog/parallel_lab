#include "common.h"

void mpi_add(size_t size, double A[], double B[], double C[]) {

}

int main() {
  MPI_INIT()
	EXEC(mpi_add);
}
