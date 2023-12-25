#include "Image.h"
#include <iostream>
#include <string>
#include <sys/types.h>

int main() {

  try {
    feiteng::Image myImage(
        "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern.bmp");

    myImage.RGBtoHSV();
    myImage.HSVtoRGB();
    myImage.print();
    // myImage.saveGrayImage("/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern1.bmp");

  } catch (const std::exception &e) {
    std::cerr << "wrong: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
