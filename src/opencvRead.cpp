#include "opencv2/core/types.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

int main() {
  std::string imagePath =
      "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern.bmp";
  cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

  if (image.empty()) {
    std::cout << "cannot read image" << std::endl;
    return -1;
  }
  cv::namedWindow("Orgin Image", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("Gaussian Blur Image", cv::WINDOW_AUTOSIZE);

  cv::imshow("Orgin Image", image);

  cv::Mat out;

  cv::GaussianBlur(image, out, cv::Size(5, 5), 3, 3);
  cv::GaussianBlur(image, out, cv::Size(5, 5), 3, 3);

  cv::imshow("Gaussian Blur Image", out);

  cv::waitKey(0);

  cv::destroyAllWindows();

  return 0;
}
