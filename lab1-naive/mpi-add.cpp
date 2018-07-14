#include <mpi.h>
#include "common.h"
constexpr size_t BUFFER_SIZE = 1 << 26;
auto buffer_a = std::make_unique<float[]>(BUFFER_SIZE);
auto buffer_b = std::make_unique<float[]>(BUFFER_SIZE);
auto buffer_c = std::make_unique<float[]>(BUFFER_SIZE);

void mpi_add(size_t size, float A[], float B[], float C[]) {
  MPI_Bcast(&size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  cerr << std::hex << size << endl;
	int concurrency;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  size_t len = size / concurrency;
	assert(size % concurrency == 0);
	assert(len <= BUFFER_SIZE);
	MPI_Scatter(A, len, MPI_float, buffer_a.get(), len, MPI_float, 0,
							MPI_COMM_WORLD);
	MPI_Scatter(B, len, MPI_float, buffer_b.get(), len, MPI_float, 0,
							MPI_COMM_WORLD);
	for (size_t i = 0; i < len; ++i) {
		buffer_c[i] = buffer_b[i] + buffer_a[i];
	}
	MPI_Gather(buffer_c.get(), len, MPI_float, C, len, MPI_float, 0,
						 MPI_COMM_WORLD);
}

void mpi_slave() {

	while (true) {
    size_t size;
    MPI_Bcast(&size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    cerr << std::hex << size << endl;
    if(size == 0){
      break;
    }
		int concurrency;
		int rank;
		MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    size_t len = size / concurrency;

		MPI_Scatter(nullptr, len, MPI_float, buffer_a.get(), len,
								MPI_float, 0, MPI_COMM_WORLD);
		MPI_Scatter(nullptr, len, MPI_float, buffer_b.get(), len,
								MPI_float, 0, MPI_COMM_WORLD);
		for (size_t i = 0; i < len; ++i) {
			buffer_c[i] = buffer_b[i] + buffer_a[i];
		}
		MPI_Gather(buffer_c.get(), len, MPI_float, nullptr, len, MPI_float,
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
    size_t size = 0;
    MPI_Bcast(&size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
	} else {
		MPI_Barrier(MPI_COMM_WORLD);
		mpi_slave();
	}
	MPI_Finalize();
	return 0;
}