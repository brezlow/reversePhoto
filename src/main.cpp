#include "Image.h"
#include <iostream>
#include <string>
#include <sys/types.h>

int main() {

  try {
    feiteng::Image myImage(
        "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern.bmp");

    // myImage.print();
    myImage.toGray();

    myImage.saveGrayImage(
        "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fe1111.bmp");
  } catch (const std::exception &e) {
    std::cerr << "wrong: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
