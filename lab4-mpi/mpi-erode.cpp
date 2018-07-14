#include <thread>
#include <vector>
#include "../common/opencv-bench.h"
#include <memory>
#include <mpi.h>


void erode_workload(uint8_t *src, uint8_t *dst, uint8_t *kernel,
                    int beg_r, int end_r, int cols
                    int kernel_rows, int kernel_cols
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

struct Config {
    int cols;
    int kernel_rows;
    int kernel_cols;
};


void erode_pthread(Mat &src, Mat &dst, Mat &kernel) {
  int rows = src.rows;
  int cols = src.cols;
  MPI_Request handle;
  
  int concurrency;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD, &concurrency);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  Config config;
  config.cols = src.cols;
  config.kernel_rows = kernel.rows;
  config.kernel_cols = kernel.cols;
  
  MPI_Isend(&config, 5, MPI_INT, tid, 0, MPI_COMM_WORLD, &handle);
  MPI_Request_free(&handle);
  for (int tid = 0; tid < concurrency - 1; ++tid) {
    // split by line
    int beg = tid * rows / concurrency;
    int end = (tid + 1) * rows / concurrency;
    end = std::min(end + 10, rows);
    
    
    MPI_Isend(src.data, (end - beg) * cols, MPI_UINT8_T, tid, 1, MPI_COMM_WORLD, &handle);
    MPI_Request_free(&handle);
  }
  int tid = concurrency - 1;
  int beg = tid * rows / concurrency;
  int end = (tid + 1) * rows / concurrency;
  
  erode_workload(src.data, dst.data, kernel.data, beg, end, src.cols, kernel.rows, kernel.cols);
}

void erode_ref(Mat &src, Mat &dst, Mat &kernel) {
  erode(src, dst, kernel);
}


int main() {
  EXEC_CV(erode_pthread);
  return 0;
}
