#include <mpi.h>
#include "common.h"
auto buffer_a = std::make_unique<double[]>(1 << 25);
auto buffer_b = std::make_unique<double[]>(1 << 25);
auto buffer_c = std::make_unique<double[]>(1 << 25);
void mpi_add(size_t size, double A[], double B[], double C[]) {
	assert(size <= (1 << 27));
  MPI_Bcast(&size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  cerr << size;
	int concurrency;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	size_t beg = size * rank / concurrency;
	size_t end = size * (rank + 1) / concurrency;
	MPI_Scatter(A, size, MPI_DOUBLE, buffer_a.get(), end - beg, MPI_DOUBLE, 0,
							MPI_COMM_WORLD);
	MPI_Scatter(B, size, MPI_DOUBLE, buffer_b.get(), end - beg, MPI_DOUBLE, 0,
							MPI_COMM_WORLD);
	for (size_t i = beg; i < end; ++i) {
		buffer_c[i] = buffer_b[i] + buffer_a[i];
	}

	MPI_Gather(buffer_c.get(), end - beg, MPI_DOUBLE, C, size, MPI_DOUBLE, 0,
						 MPI_COMM_WORLD);
}

void mpi_slave() {

	while (true) {
    size_t size;
    MPI_Bcast(&size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    cerr << size;

		int concurrency;
		int rank;
		MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		size_t beg = size * rank / concurrency;
		size_t end = size * (rank + 1) / concurrency;

		MPI_Scatter(nullptr, size, MPI_DOUBLE, buffer_a.get(), end - beg,
								MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Scatter(nullptr, size, MPI_DOUBLE, buffer_b.get(), end - beg,
								MPI_DOUBLE, 0, MPI_COMM_WORLD);
		for (size_t i = beg; i < end; ++i) {
			buffer_c[i] = buffer_b[i] + buffer_a[i];
		}

		MPI_Gather(buffer_c.get(), end - beg, MPI_DOUBLE, nullptr, size, MPI_DOUBLE,
							 0, MPI_COMM_WORLD);
	}
}

int main() {
	MPI_Init(nullptr, nullptr);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		MPI_Barrier(MPI_COMM_WORLD);
		EXEC(mpi_add);
	} else {
		MPI_Barrier(MPI_COMM_WORLD);
		mpi_slave();
	}
	MPI_Finalize();
	return 0;
}