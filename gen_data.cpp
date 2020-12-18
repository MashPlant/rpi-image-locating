#include <arpa/inet.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  cv::Mat p = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
  for (int i = 0; i < 3; ++i) cv::pyrDown(p, p);
  // printf("%x %d %d %d\n", p.flags, p.dims, p.rows, p.cols);
  // printf("%p %p %p %p\n", p.data, p.datastart, p.dataend, p.datalimit);
  // printf("%p %p\n", p.allocator, p.u);
  // printf("%d %d %ld %ld\n", p.size[0],p.size[1], p.step[0], p.step[1]);
  if (argc == 3) {
    const static int ON = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &ON, sizeof(ON));
    sockaddr_in addr{.sin_family = AF_INET, .sin_port = htons(8081), .sin_addr = {inet_addr(argv[2])}, .sin_zero = {}};
    connect(sockfd, (sockaddr *)&addr, sizeof(addr));
    write(sockfd, "POST", 4);
    int n = write(sockfd, p.data, p.rows * p.cols);
    printf("write %d byte data to %s:8081\n", n, argv[2]);
    // char buf[4096];
    // n = read(sockfd, buf, sizeof(buf));
    // FILE *fp = fopen("read.bin", "wb");
    // fwrite(buf, 1, n, fp);
    close(sockfd);
  } else {
    printf("unsigned char data[%d] = {", p.rows * p.cols);
    for (int i = 0; i < p.rows * p.cols; ++i) {
      printf("%u, ", p.data[i]);
    }
    printf("};");
  }
}