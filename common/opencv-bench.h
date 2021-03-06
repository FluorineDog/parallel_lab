#include <chrono>
#include <exception>
#include <functional>
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
					if (kernel.data[ki * kernel.cols + kj]) {
						pixel = std::min(pixel, anchor[ki * src.cols + kj]);
					}
				}
			}
			dst.data[base_row * src.cols + base_col] = pixel;
		}
	}
}

using functor_t = void(Mat& src, Mat& dst, Mat& kernel);

void execute_cv(std::string name,	//
								std::function<functor_t> func, int argc, char* argv[]) {
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " <picture> <output_dir>" << endl;
		exit(-1);
	}
	Mat dst_image, dst_ref;
	Mat src_image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	// try {
	imwrite(argv[2] + std::string("-original-picture.png"), src_image);
	cerr << "cols:" << src_image.cols << " rows:" << src_image.rows << endl;

	Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
	dst_ref = Mat(src_image.rows, src_image.cols, CV_8UC1, Scalar(0, 0, 255));
	dst_image = Mat(src_image.rows, src_image.cols, CV_8UC1, Scalar(0, 0, 255));

	cerr << "Warming up" << endl;
	constexpr int REP = 100;

	auto t1 = high_resolution_clock::now();
	for (int i = 0; i < REP / 2; ++i) {
		// warming up
		cerr << "#";
		erode_baseline(src_image, dst_ref, element);
		func(src_image, dst_image, element);
	}
	cerr << endl;
	auto t2 = high_resolution_clock::now();
	cerr << "Warming done. Estimated "
		<< duration_cast<duration<double, std::milli>>(t2 - t1).count() * 2
		<< " ms to test"
	       	<< endl;

	element = getStructuringElement(MORPH_RECT, Size(10, 10));
	dst_ref = Mat(src_image.rows, src_image.cols, CV_8UC1, Scalar(0, 0, 255));
	dst_image = Mat(src_image.rows, src_image.cols, CV_8UC1, Scalar(0, 0, 255));

	auto beg_time = high_resolution_clock::now();
	for (int i = 0; i < REP; ++i) {
		erode_baseline(src_image, dst_ref, element);
	}
 	cerr << endl;
	auto mid_time = high_resolution_clock::now();
	for (int i = 0; i < REP; ++i) {
		func(src_image, dst_image, element);
	}
	auto end_time = high_resolution_clock::now();
	auto base_time =
			duration_cast<duration<double, std::milli>>(mid_time - beg_time).count() /
			REP;
	auto exec_time =
			duration_cast<duration<double, std::milli>>(end_time - mid_time).count() /
			REP;
	cout << "baseline:\t" << base_time << "ms" << endl;
	cout << name << ":\t" << exec_time << "ms" << endl;

	imwrite(argv[2] + std::string("-reference.png"), dst_ref);
	imwrite(argv[2] + std::string("-current.png"), dst_image);
	waitKey(0);
	// } catch (cv::Exception& ex) {
	// 	cout << "cv Exception:" << ex.what();
	// 	return -1;
	// } catch (std::exception& ex) {
	// 	cout << "std Exception:" << ex.what();
	// 	return -2;
	// }
}

#define EXEC_CV(functor) execute_cv(#functor, functor, argc, argv)
