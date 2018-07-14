#include <chrono>
#include <exception>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std::chrono;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

void erode_baseline(Mat& src, Mat& dst, Mat& kernel) {
  for (int base_row = 0; base_row < src.rows; ++base_row) {
    for (int base_col = 0; base_col < src.cols; ++base_col) {
      int len_row = std::min(kernel.rows, src.rows - base_row);
      int len_col = std::min(kernel.cols, src.cols - base_col);
      uint8_t pixel = 255;
      auto anchor = src.data + base_row * src.cols + base_col;
#pragma unroll(10)
      for (int ki = 0; ki < len_row; ++ki) {
#pragma unroll(10)
        for (int kj = 0; kj < len_col; ++kj) {
          if (kernel.data[ki * base_col + kj]) {
            pixel = std::min(pixel, anchor[ki * src.cols + kj]);
          }
        }
      }
      dst.data[base_row * src.cols + base_col] = pixel;
    }
  }
}

using functor_t = void(Mat& src, Mat& dst, Mat& kernel);

void execute_cv(std::string name,  //
                std::function<functor_t> func) {
  const int KernelWidth = 10;
  const int TileWidth = 10;
  Mat dst_image, dst_ref;
  Mat src_image = imread("/home/mike/workspace/parallel_lab/data/lena.png", CV_LOAD_IMAGE_GRAYSCALE);
  // try {
  imshow("original picture", src_image);
  cout << "cols:" << src_image.cols << " rows:" << src_image.rows << endl;
  Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
  dst_image = src_image.clone();
  auto beg_time = high_resolution_clock::now();
  erode_baseline(src_image, dst_image, element);
  auto mid_time = high_resolution_clock::now();
  func(src_image, dst_ref, element);
  auto end_time = high_resolution_clock::now();
  auto base_time =
      duration_cast<duration<double, std::milli>>(mid_time - beg_time).count();
  auto exec_time =
      duration_cast<duration<double, std::milli>>(end_time - mid_time).count();
  cout << "baseline:\t" << base_time << endl;
  cout << name << ":\t" << exec_time << endl;

  imshow("baseline", dst_image);
  imshow("executed", dst_ref);
  waitKey(0);
  // } catch (cv::Exception& ex) {
  // 	cout << "cv Exception:" << ex.what();
  // 	return -1;
  // } catch (std::exception& ex) {
  // 	cout << "std Exception:" << ex.what();
  // 	return -2;
  // }
}

#define EXEC_CV(functor) execute_cv(#functor, functor)
