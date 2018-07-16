#ifndef __CUDACC__
#define  __CUDACC__
#endif
#include "../common/opencv-bench.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_texture_types.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <cufftXt.h>

struct Config {
    int rows;
    int cols;
    int kernel_rows;
    int kernel_cols;
};


__constant__ uint8_t kernelDev[10*10];
uint8_t *srcDev;
uint8_t *dstDev;
size_t pitch;
//texture<uint8_t, cudaTextureType2D> src_tex8u;
//texture<uint8_t, cudaTextureType2D> dst_tex8u;


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

__global__ void erode_kernel(uint8_t* src, uint8_t* dst, int pitch, int kernel_row, int kernel_col){
  __shared__ uint8_t sh_mem[64][64];
  int col = threadIdx.x;
  int row = threadIdx.y;
  int full_col = blockIdx.x * blockDim.x  + threadIdx.x;
  int full_row = blockIdx.y * blockDim.y  + threadIdx.y;
  int loc = full_row * pitch + full_col;
  bool bx = blockIdx.x == gridDim.x - 1;
  bool by = blockIdx.y == gridDim.y - 1;
  
  constexpr uint8_t border = 255;
  sh_mem[row][col] = src[loc];
  if(row < 10) sh_mem[row + 32][col] =  by ? border: src[loc + 32 * pitch];
  if(col < 10) sh_mem[row][col + 32] = bx ? border: src[loc + 32];
  if(row < 10 && col < 10) sh_mem[row + 32][col + 32] = (by || bx) ? border: src[loc + 32 + pitch * 32];
  __syncthreads();
  uint8_t pixel = border;
  for(int i = 0; i < kernel_row; ++i){
    for(int j = 0; j < kernel_col; ++j){
//      pixel = std::max(pixel, kernelDev[i * 10 + j] ? sh_mem[row + i][col + j] : border);
      uint8_t new_pixel = sh_mem[row + i][col + j];
      if( kernelDev[i * 10 + j] && new_pixel < pixel){
        pixel = new_pixel;
      }
    }
  }
  dst[loc] = pixel;
}


void erode_cuda(Mat& src, Mat& dst, Mat& kernel) {
  int cols = src.cols;
  int rows = src.rows;
  ::cudaMemcpyToSymbol(kernelDev, kernel.data, 10*10*sizeof(uint8_t), 0);
  ::cudaMemcpy2D(srcDev, pitch,  src.data, cols, cols, rows, cudaMemcpyHostToDevice);
  dim3 grids(cols / 32, rows / 32);
  dim3 blocks(32, 32);
  erode_kernel<<<grids, blocks>>>(srcDev, dstDev, pitch, kernel.rows, kernel.cols);
  
  ::cudaMemcpy2D(dst.data, cols, dstDev, pitch,  cols, rows, cudaMemcpyDeviceToHost);
}


int main() {
  
//  src_tex8u.addressMode[0] = src_tex8u.addressMode[1] = cudaAddressModeBorder;
  
  ::cudaMallocPitch(&srcDev, &pitch, 512, 512);
  
  ::cudaMallocPitch(&dstDev, &pitch, 512, 512);
  
  EXEC_CV(erode_cuda);
  
  ::cudaFree(dstDev);
  ::cudaFree(srcDev);
  return 0;
}

