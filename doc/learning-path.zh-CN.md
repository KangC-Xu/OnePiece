# 阅读代码

[English](learning-path.md) · 简体中文

1. **先看契约：** [plugin_contract.h](../contracts/include/algorithm_platform/plugin_contract.h) 定义工厂、算法接口、元数据以及输入输出类型。
2. **再看插件发现：** [plugin_loader.cpp](../app/plugin_loader.cpp) 扫描本地动态库，使用 QPluginLoader 加载，检查工厂接口，并读取算法元数据。
3. **理解执行流程：** [algorithm_runner.cpp](../app/algorithm_runner.cpp) 根据算法 ID 找工厂，创建并初始化实例，将图像和参数组成请求，再调用 run。
4. **阅读界面：** [main_window.cpp](../app/main_window.cpp) 根据参数元数据生成编辑控件，展示输入、结果和日志。
5. **阅读 OpenCV 示例：** [traditional_vision_plugin.cpp](../plugins/traditional_vision/traditional_vision_plugin.cpp) 实现阈值、Canny 和轮廓提取。
6. **阅读高度图示例：** [point_cloud_plugin.cpp](../plugins/point_cloud/point_cloud_plugin.cpp) 用图像列、行作为 x、y，用灰度值作为 z，演示采样和平均高度统计。
7. **阅读推理流程：** [deep_learning_plugin.cpp](../plugins/deep_learning/deep_learning_plugin.cpp) 查找模型、准备 RGB 张量、运行 ONNX Runtime、执行 softmax，并输出 Top1 类别索引。

增加新算法时，可参考现有插件的算法实现、工厂入口和 CMake 目标。元数据、输入和输出需要遵循统一契约。宿主在运行时发现插件，而具体构建哪些插件模块仍由 CMake 决定。
