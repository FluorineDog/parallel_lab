#include <thread>
#include <vector>
#include "../common/opencv-bench.h"

void erode_pthread_workload(uint8_t* src, uint8_t* dst,uint8_t* kernel, 
int beg_r, int end_r, int cols, int COL_STRIDE
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

void erode_pthread_workload(Mat& src, Mat& dst, Mat& kernel) {
  int rows = src.rows; 
  int cols = src.cols;
  // MPI_Scatter()
}

void erode_ref(Mat& src, Mat& dst, Mat& kernel) {
  erode(src, dst, kernel);
}


int main() {
  EXEC_CV(erode_pthread);
  return 0;
}
