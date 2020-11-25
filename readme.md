2020秋季嵌入式系统大作业：基于摄像头的图像定位，用树莓派摄像头正对屏幕上的静态图像的一部分进行拍摄，计算拍摄到的部分位于原图像的实时位置。

生成的img文件位于[release](https://github.com/MashPlant/rpi-image-locating/releases/download/initial/sdcard.img.zip)中，下载并解压后可以直接刷入Raspberry Pi 3 Model B+中。

以下介绍生成img文件的过程和使用方法。报告在report文件夹下，它大致描述了这些修改是怎么得到的。

# 构建程序

## 安装musl工具链

因为我在报告中提到的原因，musl工具链的版本必须是1.1.24或以下。[https://musl.cc/](https://musl.cc/)中有最新版的musl二进制下载，但是没有提供1.1.24或以下老版本的下载，所以不能用。我选择使用[musl-cross-make](https://github.com/richfelker/musl-cross-make)手动构建：

```bash
$ git clone https://github.com/richfelker/musl-cross-make && cd musl-cross-make
# 用你习惯的编辑器修改Makefile文件中的MUSL_VER变量，改成1.1.24或以下：
# -MUSL_VER = 1.2.1
# +MUSL_VER = 1.1.24
$ make install -j16
```

这样就在`<musl path>/output`下生成了musl工具链。然后将`<musl path>/output/bin`加入`PATH`。

## 编译OpenCV

我基于OpenCV commit `5f1ca33c6f06727665692ea43988caf5f8caa02b`进行修改，通过生成的[patch文件](./0001-mashplant-modification.patch)可以重现我的修改。

```bash
$ git clone https://github.com/opencv/opencv && cd opencv
$ git checkout 5f1ca33c6f06727665692ea43988caf5f8caa02b
# 将本仓库中的0001-mashplant-modification.patch拷贝到opencv目录下
$ git apply 0001-mashplant-modification.patch 
$ mkdir build && cd build
$ cmake -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-musleabi.toolchain.cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install -DWITH_PROTOBUF=OFF -DWITH_OPENCL=OFF -DBUILD_opencv_apps=OFF -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_features2d=OFF -DBUILD_opencv_flann=OFF -DBUILD_opencv_gapi=OFF -DBUILD_opencv_highgui=OFF -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_ml=OFF -DBUILD_opencv_objdetect=OFF -DBUILD_opencv_photo=OFF -DBUILD_opencv_python_bindings_generator=OFF -DBUILD_opencv_python_tests=OFF -DBUILD_opencv_stitching=OFF -DBUILD_opencv_ts=OFF -DBUILD_opencv_video=OFF -DWITH_IMGCODEC_HDR=OFF -DWITH_WEBP=OFF -DWITH_IMGCODEC_SUNRASTER=OFF -DWITH_IMGCODEC_PXM=OFF -DWITH_IMGCODEC_PFM=OFF -DWITH_TIFF=OFF -DWITH_PNG=OFF -DWITH_GDCM=OFF -DWITH_JASPER=OFF -DWITH_OPENJPEG=OFF -DWITH_OPENEXR=OFF -DWITH_GDAL=OFF -DWITH_PTHREADS_PF=OFF ..
$ make install -j16
```

这样就在`<opencv path>/install`下生成了OpenCV的头文件和库。

## 编译程序

为了运行`gen_data.cpp`，本机也需要安装OpenCV，用你习惯的包管理器安装就可以。还需要将一张待识别的1920 * 1080的图片`1.jpg`放到和`gen_data.cpp`同一目录下，然后执行：

```bash
$ g++ gen_data.cpp `pkg-config --libs opencv` -lpthread -O3
$ ./a.out > data.cpp
```

这样生成的`data.cpp`中包含一个全局数组定义，`main.cpp`中使用了这个数组。编译`main.cpp`：

```bash
$ arm-linux-musleabihf-g++ main.cpp -I<opencv path>/install/include/opencv4/ -L<opencv path>/install/lib -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -L<opencv path>/install/lib/opencv4/3rdparty -littnotify -llibjpeg-turbo -llibopenjp2 -llibpng -llibtiff -llibwebp -lzlib -ldl -lpthread -O3 -static -fno-exceptions -fno-rtti -flto -s data.cpp
```

这样就生成了一个静态编译的程序`a.out`，在我的环境中它的大小约为2.5MiB，它将完成包括图像录制，位置识别，响应网络请求在内的所有工作，并且不依赖任何动态链接库。在最终的系统中只需要联网后运行这个程序即可。

# 构建系统

先将上一步生成的`a.out`拷贝到仓库中的`buildroot_raspberry/board/raspberrypi3/rootfs_overlay/root/`文件夹下，然后使用Buildroot构建镜像：

```bash
$ git clone https://github.com/buildroot/buildroot
$ mkdir buildroot_download # 用来存放buildroot下载的内容
$ mkdir buildroot_build && cd buildroot_build
$ make BR2_EXTERNAL=<path to buildroot_raspberry> O=$PWD -C ../buildroot/ raspberrypi3_minimal_defconfig
```

等待约半小时后，生成的`buildroot_build/images/sdcard.img`即是我们需要的镜像文件。我不知道有没有什么真正压缩镜像文件的方法，即刷入树莓派的也是压缩的镜像，在启动过程解压到内存中。只能在外部压缩这个镜像文件，生成`sdcard.img.zip`，在我的环境中大小约8.1MiB。

# 使用方法

将镜像刷入树莓派后，启动树莓派时它会试图连接一个名为`test`的没有密码的Wi-Fi，你可以用手机热点提供这个Wi-Fi。如果你不喜欢`test`这个名字，可以在`buildroot_raspberry/board/raspberrypi3/rootfs_overlay/etc/init.d/S60wifi`中修改。

树莓派连上Wi-Fi后有很多种方法可以查找它的IP地址，比较简单的一种是在(Android)手机上安装一个命令行模拟器，在其中执行`ip neigh`即可查看连接本手机热点的设备IP。

电脑同样连上这个Wi-Fi，将`index.html`中的`let server = "http://0.0.0.0:";`改成`let server = "http://<树莓派 IP>:";`，然后在浏览器中打开`index.html`即可。经测试Windows设备打开`index.html`会有相当程度的卡顿和延迟，我不清楚具体原因，可能和防火墙有关，建议使用Linux设备打开。