
#include "../common/opencv-bench.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <cufftXt.h>

struct Config {
    int rows;
    int cols;
    int kernel_rows;
    int kernel_cols;
};


__constant__ uint8_t devKernel[10*10];
uint8_t *srcDev;
uint8_t *dstDev;

texture<uint8_t, cudaTextureType2D> tex8u;

//void erode_cuda(Mat& src, Mat& dst, Mat& kernel) {
//  for (int base_row = 0; base_row < src.rows; ++base_row) {
//    for (int base_col = 0; base_col < src.cols; ++base_col) {
//      int len_row = std::min(kernel.rows, src.rows - base_row);
//      int len_col = std::min(kernel.cols, src.cols - base_col);
//      uint8_t pixel = 255;
//      auto anchor = src.data + base_row * src.cols + base_col;
//#pragma unroll(10)
//      for (int ki = 0; ki < len_row; ++ki) {
//#pragma unroll(10)
//        for (int kj = 0; kj < len_col; ++kj) {
//          if (kernel.data[ki * base_col + kj]) {
//            pixel = std::min(pixel, anchor[ki * src.cols + kj]);
//          }
//        }
//      }
//      dst.data[base_row * src.cols + base_col] = pixel;
//    }
//  }
//}

__host__ __device__ void erode_kernel(uint8_t* src, uint8_t* dst, size_t pitch, size_t rows, size_t cols){

}


void erode_cuda(Mat& src, Mat& dst, Mat& kernel) {
  size_t pitch;
  size_t cols = src.cols;
  size_t rows = src.rows;
  ::cudaMallocPitch(&srcDev, &pitch, cols, rows);
  ::cudaMallocPitch(&dstDev, &pitch, cols, rows);
  ::cudaMemcpyToSymbol(devKernel, kernel.data, 10*10*sizeof(uint8_t), 0);
  ::cudaMemcpy2D(srcDev, pitch,  src.data, cols, cols, rows, cudaMemcpyHostToDevice);
  
  
  
  ::cudaMemcpy2D(dst.data, cols, dstDev, pitch,  cols, rows, cudaMemcpyDeviceToHost);
  ::cudaFree(dstDev);
  ::cudaFree(srcDev);
}


int main() {
  EXEC_CV(erode_cuda);
  return 0;
}
