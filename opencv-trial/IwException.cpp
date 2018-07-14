#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <exception>
using namespace cv;
using namespace std;
int main() {
	const int KernelWidth = 10;
	const int TileWidth = 10;
	Mat dst_image;
	Mat src_image = imread("data/lena.png", CV_LOAD_IMAGE_GRAYSCALE);
	try {
		imshow("original picture", src_image);
		cout << "cols:" << src_image.cols << " rows:" << src_image.rows << endl;
		Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
		erode(src_image, dst_image, element);
	  imshow("eroded picture all", dst_image);
    waitKey(0);
	} catch (cv::Exception& ex) {
		cout << "cv Exception:" << ex.what();
		return -1;
	} catch(std::exception& ex) {
		cout << "std Exception:" << ex.what();
    return -2;
  }
	return 0;
}
