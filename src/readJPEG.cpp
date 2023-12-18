#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

int main() {
  std::string imagePath = "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern.png";
  cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

  if (image.empty()) {
    std::cout << "cannot read image" << std::endl;
    return -1;
  }

  cv::imshow("Image", image);
  cv::waitKey(0);

  return 0;
}
