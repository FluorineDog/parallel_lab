#include <thread>
#include <vector>
#include "../common/opencv-bench.h"
#include <memory>
#include <mpi.h>
#include <fstream>

struct Config {
    int rows;
    int cols;
    int kernel_rows;
    int kernel_cols;
};

void erode_workload(uint8_t *src, uint8_t *dst, uint8_t *kernel,
                    int beg_r, int end_r,
                    Config config
) {
  src -= beg_r * config.cols;
  dst -= beg_r * config.cols;
  for(int i = 0; i < 100; ++i){
    cout << kernel[i] << " ";
  }
  
  std::ofstream ffout(std::to_string(beg_r) + "config.txt" );
  ffout << config.cols  << " " << config.rows << " " << endl;
  ffout << config.kernel_cols  << " " << config.kernel_rows << " " << endl;
  ffout << beg_r << " " << end_r << " " << endl;
  cout << endl;
  for (int base_row = beg_r; base_row < end_r; ++base_row) {
    for (int base_col = 0; base_col < config.cols; ++base_col) {
      int len_row = std::min(config.kernel_rows, config.rows - base_row);
      int len_col = std::min(config.kernel_cols, config.cols - base_col);
      uint8_t pixel = 255;
      auto anchor = src + base_row * config.cols + base_col;
#pragma unroll(10)
      for (int ki = 0; ki < len_row; ++ki) {
#pragma unroll(10)
        for (int kj = 0; kj < len_col; ++kj) {
          if (kernel[ki * config.kernel_cols + kj]) {
            pixel = std::min(pixel, anchor[ki * config.cols + kj]);
          }
        }
      }
      dst[base_row * config.cols + base_col] = pixel;
    }
  }
}


void erode_slave() {
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
  std::ofstream fout("log.txt");
  MPI_Bcast(kernel.get(), kernel_area, MPI_UINT8_T, 0, MPI_COMM_WORLD);

  
  const int beg = rank * rows / concurrency;
  const int end = (rank + 1) * rows / concurrency;
  const int end_padded = std::min(end + 10, rows);
  int src_area = (end_padded - beg) * cols;
  int dst_area = (end - beg) * cols;
  fout << src_area  << "^" << dst_area << endl;
  auto buffer = std::make_unique<uint8_t[]>(src_area);
  auto dst_buffer = std::make_unique<uint8_t[]>(dst_area);
  
  MPI_Recv(buffer.get(), src_area, MPI_UINT8_T, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  // Mat (int rows, int cols, int type, void *data, size_t step=AUTO_STEP)
  erode_workload(buffer.get(), dst_buffer.get(), kernel.get(), beg, end, config);
  MPI_Gatherv(dst_buffer.get(), dst_area, MPI_INT8_T, nullptr, nullptr, nullptr, MPI_UINT8_T, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
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
  int tid = 0;
  int beg = tid * rows / concurrency;
  int end = (tid + 1) * rows / concurrency;
  recvcounts.push_back((end - beg) * cols);
  displs.push_back(0);
  for (int tid = 1; tid < concurrency; ++tid) {
    // split by line
    int beg = tid * rows / concurrency;
    int end = (tid + 1) * rows / concurrency;
    displs.push_back(beg * cols);
    recvcounts.push_back((end - beg) * cols);
    end = std::min(end + 10, rows);
    MPI_Isend(src.data + displs.back(), (end - beg) * cols, MPI_UINT8_T, tid, 1, MPI_COMM_WORLD, &handle);
    MPI_Request_free(&handle);
  }

  erode_workload(src.data, dst.data, kernel.data, beg, end, config);
  MPI_Gatherv(MPI_IN_PLACE, (end - beg) * cols, MPI_UINT8_T, dst.data,
              recvcounts.data(), displs.data(), MPI_UINT8_T, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
}

void erode_ref(Mat &src, Mat &dst, Mat &kernel) {
  erode(src, dst, kernel);
}


int main() {
  int rank;
  MPI_Init(nullptr, nullptr);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    EXEC_CV(erode_mpi);
  
    EXEC_CV(erode_ref);
  } else {
    erode_slave();
  }
  MPI_Finalize();
  return 0;
}
