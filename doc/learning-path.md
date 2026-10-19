# Reading the code

English · [简体中文](learning-path.zh-CN.md)

1. **Start with the contract:** [plugin_contract.h](../contracts/include/algorithm_platform/plugin_contract.h) defines the factory, algorithm interface, metadata, request, and result types.
2. **Follow discovery:** [plugin_loader.cpp](../app/plugin_loader.cpp) scans native libraries, loads them with QPluginLoader, checks the factory interface, and reads algorithm metadata.
3. **Follow execution:** [algorithm_runner.cpp](../app/algorithm_runner.cpp) finds a factory by algorithm ID, creates and initializes an instance, packages the image and parameters, and calls run.
4. **Inspect the UI:** [main_window.cpp](../app/main_window.cpp) creates editors from parameter metadata and shows input images, previews, and logs.
5. **Read the OpenCV example:** [traditional_vision_plugin.cpp](../plugins/traditional_vision/traditional_vision_plugin.cpp) implements thresholding, Canny, and contours.
6. **Read the height-map example:** [point_cloud_plugin.cpp](../plugins/point_cloud/point_cloud_plugin.cpp) uses image column and row as x and y, and grayscale intensity as z. It samples points and computes mean height.
7. **Read the inference path:** [deep_learning_plugin.cpp](../plugins/deep_learning/deep_learning_plugin.cpp) resolves a model path, prepares RGB tensors, runs ONNX Runtime, applies softmax, and reports the top-1 class index.

For a new algorithm, follow an existing plugin's implementation, factory entry, and CMake target. Keep metadata, input, and result definitions compatible with the shared contract. The host finds plugins at runtime, while CMake still determines which plugin modules are built.
