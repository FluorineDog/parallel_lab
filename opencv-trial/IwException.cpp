#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
int main() {
  const int KernelWidth = 10;
  const int TileWidth = 10;
  Mat srcImage = imread("data/graypicture.png", CV_LOAD_IMAGE_GRAYSCALE);
  try {
    imshow("original picture", srcImage);
  } catch (cv::Exception& ex) {
    fprintf(stderr, "Exception converting image to PNG format: %s\n",
            ex.what());
    // getchar();
    return -1;
  }
  cout << "cols:" << srcImage.cols << " rows:" << srcImage.rows << endl;
  Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
  Mat dstImage;
  try {
    erode(srcImage, dstImage, element);
  } catch (cv::Exception& ex) {
    cout << "Exception:" << ex.what();
    getchar();
    return -1;
  }
  imshow("eroded picture all", dstImage);
  waitKey(0);
  // getchar();
  return 0;
}
