# 依赖说明

[English](dependencies.md) · 简体中文

仓库只包含应用源码。SDK 和模型权重请从各自官方来源单独获取。

## 必需工具

- Windows x64，以及安装“使用 C++ 的桌面开发”工作负载的 Visual Studio 2022。
- MSVC x64 版本的 Qt 6：[Qt 下载](https://www.qt.io/download)。
- CMake 3.25 或更高版本，满足预设文件格式要求：[CMake 下载](https://cmake.org/download/)。
- OpenCV x64 Release 头文件和库：[OpenCV 4.9.0](https://github.com/opencv/opencv/releases/tag/4.9.0)。

## 依赖目录

默认根目录为 `3rd/`。也可以通过 `-DATP_THIRD_PARTY_DIR=C:/dev/onepiece-dependencies` 指定仓库外的依赖目录。

```text
3rd/
  opencv/
    include/opencv2/...
    lib/opencv_world490.lib
    bin/opencv_world490.dll
  onnxruntime/                 可选
    include/onnxruntime_cxx_api.h
    include/...               同一版本的其他头文件
    lib/onnxruntime.lib
    bin/onnxruntime.dll
    bin/...                   所需的其他运行库
  models/                     可选，需单独下载
    mobilenet/mobilenet_v2.onnx
```

官方 OpenCV Windows 包中的 `build/include/` 对应这里的 `opencv/include/`；将 `build/x64/vc16/lib/` 中的 Release 版 `opencv_world490.lib` 放入 `opencv/lib/`，将 `build/x64/vc16/bin/` 中匹配的 `opencv_world490.dll` 放入 `opencv/bin/`。头文件、链接库和 DLL 必须来自同一版本，不要把文件名以 `d` 结尾的 Debug 库混入此目录。

[ONNX Runtime 官方发行版](https://github.com/microsoft/onnxruntime/releases)的 Windows x64 CPU 包包含 `include/` 和 `lib/`。复制全部头文件，将 `onnxruntime.lib` 放入这里的 `lib/`，将 DLL 放入这里的 `bin/`。没有这套头文件和库时项目仍可构建，但分类插件会提示无法运行推理。

模型路径独立于构建时的依赖根目录。如果模型放在仓库外，请在界面中填写模型的绝对路径。

## 模型输入输出约定

分类示例要求：

- 一个 float32 图像输入，形状为 `[1, 3, input_size, input_size]`。
- RGB 数据先缩放至 0–1，再按均值 `[0.485, 0.456, 0.406]` 和标准差 `[0.229, 0.224, 0.225]` 归一化。
- 第一个输出为 float32 分类 logits；插件执行 softmax 后选出最高分。

输入尺寸需要在界面中设置为模型要求的值，默认的 224 并不适用于所有模型。如果预处理或输出含义不同，需要相应修改代码。仓库不分发模型权重或类别标签表。

## 验证环境

发布时使用 Windows x64、MSVC 19.44、Qt 6.10.1、OpenCV 4.9.0、ONNX Runtime API 24、CMake 4.3.0-rc2，按 Visual Studio 2022 Release 配置完成构建验证。这是已验证的环境，不代表其他所有版本均可直接使用。

本地检查还覆盖了三个插件的发现、阈值/Canny/轮廓处理、高度图点数及平均高度、缺失输入和模型错误、一次 CPU 分类推理，以及界面选图到显示结果的流程。分类检查使用单独保管的 128 × 128 输入模型和合成图像，只验证流程，不代表分类准确率。截图来自实际 Qt 窗口的离屏渲染。

## 第三方许可证

| 依赖 | 许可证或说明 |
| --- | --- |
| Qt 6 Core、Gui、Widgets | [Qt 许可说明](https://doc.qt.io/qt-6/licensing.html)，按所用发行版的许可证使用 |
| OpenCV 4.9 | [Apache-2.0](https://github.com/opencv/opencv/blob/4.9.0/LICENSE) |
| ONNX Runtime | [MIT](https://github.com/microsoft/onnxruntime/blob/main/LICENSE) |
| 模型权重和数据集 | 使用或再次分发前核对具体来源的条款 |

本项目的 MIT 许可证适用于原创内容，不替代单独获取的第三方依赖许可证。
