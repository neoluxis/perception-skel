# neolux perception library

基于 ONNX Runtime 的 YOLOv8s 目标检测推理框架，支持 SAHI 切片、ByteTrack 多目标跟踪。

本仓库提供预编译的头文件和库文件，方便下游项目通过 CMake `find_package` 直接集成。

## 目录

```
├── include/kflee/          # C++ 公共头文件
├── lib/                    # 预编译库 + CMake 包配置
│   ├── libkflee_*.so       # 动态库
│   ├── libkflee_*_static.a # 静态库
│   └── cmake/kflee/        # find_package(kflee) 配置
└── bin/                    # 示例工具（可选）
```

## 使用

```cmake
# 设置 CMAKE_PREFIX_PATH 指向本仓库根目录
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/kflee-public

# 或设置 kflee_DIR
cmake -S . -B build -Dkflee_DIR=/path/to/kflee-public/lib/cmake/kflee
```

```cmake
# 下游 CMakeLists.txt
find_package(kflee REQUIRED)
target_link_libraries(my_app PRIVATE kflee::pipeline kflee::tracker)
# 静态链接：kflee::pipeline_static
```

## 依赖

- OpenCV (core, imgproc, imgcodecs, videoio, highgui)
- ONNX Runtime

## 平台

| 平台 | Arch | 说明 |
|------|------|------|
| Linux x86_64 | amd64 | PC 端 ONNX Runtime |

## 版本

对应内部仓库版本，详见 `lib/cmake/kflee/kfleeConfigVersion.cmake`。
