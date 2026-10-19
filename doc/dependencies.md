# Dependencies

English · [简体中文](dependencies.zh-CN.md)

The repository contains application source. Obtain SDKs and model weights separately from their official providers.

## Required tools

- Windows x64 and Visual Studio 2022 with Desktop development with C++.
- Qt 6 MSVC x64: [Qt downloads](https://www.qt.io/download).
- CMake 3.25 or later for the included preset format: [CMake](https://cmake.org/download/).
- OpenCV x64 release headers and libraries: [OpenCV 4.9.0](https://github.com/opencv/opencv/releases/tag/4.9.0).

## Dependency layout

The default root is `3rd/`. You may instead pass an external directory using `-DATP_THIRD_PARTY_DIR=C:/dev/onepiece-dependencies`.

```text
3rd/
  opencv/
    include/opencv2/...
    lib/opencv_world490.lib
    bin/opencv_world490.dll
  onnxruntime/                 optional
    include/onnxruntime_cxx_api.h
    include/...               other headers from the same release
    lib/onnxruntime.lib
    bin/onnxruntime.dll
    bin/...                   other required runtime DLLs
  models/                     optional, downloaded separately
    mobilenet/mobilenet_v2.onnx
```

For the official OpenCV Windows package, copy `build/include/` into `opencv/include/`, the release `opencv_world490.lib` from `build/x64/vc16/lib/` into `opencv/lib/`, and the matching `opencv_world490.dll` from `build/x64/vc16/bin/` into `opencv/bin/`. Use headers, libraries, and DLLs from the same version. Do not mix debug libraries ending in `d` into this release layout.

The Windows x64 CPU package from [ONNX Runtime releases](https://github.com/microsoft/onnxruntime/releases) provides `include/` and `lib/`. Copy all headers, put `onnxruntime.lib` in the layout's `lib/`, and put its DLLs in the layout's `bin/`. Without these headers and library, the project still builds, but the classification plugin reports that inference is unavailable.

The model path is separate from the build dependency root. If your model is stored outside the checkout, enter its absolute path in the UI.

## Model contract

The classification example expects:

- one float32 image input with shape `[1, 3, input_size, input_size]`;
- RGB values scaled to 0–1, then normalized with mean `[0.485, 0.456, 0.406]` and standard deviation `[0.229, 0.224, 0.225]`;
- a first float32 output containing classification logits; the plugin applies softmax and selects the largest score.

Set the input-size field to match the model; the UI default of 224 is not valid for every model. A model with different preprocessing or output semantics needs corresponding code changes. No model weights or label map are distributed with this project.

## Validation

The publication build was checked with Windows x64, MSVC 19.44, Qt 6.10.1, OpenCV 4.9.0, ONNX Runtime API 24, and CMake 4.3.0-rc2, using the Visual Studio 2022 Release configuration. These are the tested versions, not a claim that every other version is supported.

The local smoke check also covered discovery of all three plugins, threshold/Canny/contour processing, height-map point count and mean height, missing-input/model errors, one CPU classification run, and the image-selection-to-result UI workflow. The classification check used a separately held model with a 128 × 128 input and a synthetic image; it verifies execution, not prediction accuracy. The screenshot was captured from the actual Qt window using offscreen rendering.

## Third-party licenses

| Dependency | License / provider |
| --- | --- |
| Qt 6 Core, Gui, Widgets | [Qt licensing](https://doc.qt.io/qt-6/licensing.html): follow the license applicable to your Qt distribution |
| OpenCV 4.9 | [Apache-2.0](https://github.com/opencv/opencv/blob/4.9.0/LICENSE) |
| ONNX Runtime | [MIT](https://github.com/microsoft/onnxruntime/blob/main/LICENSE) |
| Model weights and datasets | Check the terms of the specific source before use or redistribution |

The project's MIT license applies to its original work. It does not replace the licenses of separately obtained dependencies.
