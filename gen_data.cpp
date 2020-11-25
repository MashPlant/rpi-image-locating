#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
  cv::Mat p = cv::imread("./1.jpg", cv::IMREAD_GRAYSCALE);
  for (int i = 0; i < 3; ++i) cv::pyrDown(p, p);
  // printf("%x %d %d %d\n", p.flags, p.dims, p.rows, p.cols);
  // printf("%p %p %p %p\n", p.data, p.datastart, p.dataend, p.datalimit);
  // printf("%p %p\n", p.allocator, p.u);
  // printf("%d %d %ld %ld\n", p.size[0],p.size[1], p.step[0], p.step[1]);
  printf("unsigned char data[%d] = {", p.rows * p.cols);
  for (int i = 0; i < p.rows * p.cols; ++i) {
    printf("%u, ", p.data[i]);
  }
  printf("};");
}