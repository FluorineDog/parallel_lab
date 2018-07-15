#include <thread>
#include <vector>
#include "../common/opencv-bench.h"
#include <memory>
#include <mpi.h>

struct Config {
    int rows;
    int cols;
    int kernel_rows;
    int kernel_cols;
};

void erode_workload(uint8_t *src, uint8_t *dst, uint8_t *kernel,
                    int beg_r, int end_r,
                    const Config& config
) {
//   for (int base_row = 0; base_row < src.rows; ++base_row) {
//     for (int base_col = 0; base_col < src.cols; ++base_col) {
//       int len_row = std::min(kernel.rows, src.rows - base_row);
//       int len_col = std::min(kernel.cols, src.cols - base_col);
//       uint8_t pixel = 255;
//       auto anchor = src.data + base_row * src.cols + base_col;
// #pragma unroll(10)
//       for (int ki = 0; ki < len_row; ++ki) {
// #pragma unroll(10)
//         for (int kj = 0; kj < len_col; ++kj) {
//           if (kernel.data[ki * base_col + kj]) {
//             pixel = std::min(pixel, anchor[ki * src.cols + kj]);
//           }
//         }
//       }
//       dst.data[base_row * src.cols + base_col] = pixel;
//     }
//   }
}



void erode_slave() {
  MPI_Request handle;
  int concurrency;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  Config config;
  MPI_Bcast(&config, 5, MPI_INT, 0, MPI_COMM_WORLD);
  int rows = config.rows;
  int cols = config.cols;
  int kernel_area = config.kernel_rows * config.kernel_cols;
  auto kernel = std::make_unique<uint8_t[]>(kernel_area);
  MPI_Bcast(kernel.get(), kernel_area, MPI_UINT8_T, 0, MPI_COMM_WORLD);
  
  
  const int beg = rank * rows / concurrency;
  const int end = (rank + 1) * rows / concurrency;
  const int end_padded = std::min(end + 10, rows);
  int src_area = (end_padded - beg) * cols;
  int dst_area = (end - beg) * cols;
  auto buffer = std::make_unique<uint8_t[]>(src_area);
  auto dst_buffer = std::make_unique<uint8_t[]>(dst_area);
  MPI_Recv(buffer.get(), src_area, MPI_UINT8_T, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  erode_workload(buffer.get(), dst_buffer.get(), kernel.get(), beg, end, config);
  MPI_Gatherv(dst_buffer.get(), dst_area, MPI_INT8_T, nullptr, nullptr, nullptr, MPI_UINT8_T, 0, MPI_COMM_WORLD);
}

void erode_mpi(Mat &src, Mat &dst, Mat &kernel) {
  int rows = src.rows;
  int cols = src.cols;
  MPI_Request handle;
  
  int concurrency;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  assert(rank == 0);
  
  Config config;
  config.rows = src.rows;
  config.cols = src.cols;
  config.kernel_rows = kernel.rows;
  config.kernel_cols = kernel.cols;
  
  MPI_Bcast(&config, 5, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(kernel.data, config.kernel_rows * config.kernel_cols, MPI_UINT8_T, 0, MPI_COMM_WORLD);
  std::vector<int> recvcounts;
  std::vector<int> displs;
  recvcounts.push_back(0);
  displs.push_back(0);
  for (int tid = 1; tid < concurrency; ++tid) {
    // split by line
    int beg = tid * rows / concurrency;
    int end = (tid + 1) * rows / concurrency;
    displs.push_back(beg);
    recvcounts.push_back((end - beg) * cols);
    end = std::min(end + 10, rows);
    MPI_Isend(src.data, (end - beg) * cols, MPI_UINT8_T, tid, 1, MPI_COMM_WORLD, &handle);
    MPI_Request_free(&handle);
  }
  int tid = 0;
  int beg = tid * rows / concurrency;
  int end = (tid + 1) * rows / concurrency;
  erode_workload(src.data, dst.data, kernel.data, beg, end, config);
  MPI_Gatherv(MPI_IN_PLACE, (end - beg) * cols, MPI_UINT8_T, src.data,
              recvcounts.data(), displs.data(), MPI_UINT8_T, 0, MPI_COMM_WORLD);
}

void erode_ref(Mat &src, Mat &dst, Mat &kernel) {
  erode(src, dst, kernel);
}


int main() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(rank == 0){
    EXEC_CV(erode_mpi);
  }
  
  return 0;
}
