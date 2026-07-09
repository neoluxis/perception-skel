#pragma once

#include "kflee/dnn/inferer.h"

#include <memory>
#include <string>

namespace kflee::dnn {

/**
 * ONNX Runtime 推理器配置
 */
struct OnnxInfererConfig {
    /** ONNX 模型文件路径 */
    std::string model_path;
    /** 切片推理模式 */
    SliceInferMode slice_mode = SliceInferMode::MultiThread;
    /** 模型输入宽度 */
    int input_width = 640;
    /** 模型输入高度 */
    int input_height = 640;
    /** 输入通道数 */
    int input_channels = 3;
};

/**
 * ONNX Runtime 推理器
 *
 * Inferer 接口的 ONNX Runtime 实现。封装 ORT C++ API，
 * 支持单张推理和两种切片推理模式（Batch / 多线程并行）。
 *
 * 使用 Pimpl 隐藏 ORT 头文件依赖，上层只需包含本头文件。
 *
 * 使用方式：
 * @code
 *   OnnxInfererConfig cfg;
 *   cfg.model_path = "model.onnx";
 *   OnnxInferer inferer(cfg);
 *
 *   auto result = inferer.infer_slices(slice_tensors);
 * @endcode
 */
class OnnxInferer : public Inferer {
public:
    /**
     * 构造推理器并加载模型
     *
     * 同时创建 1 个主 session 和 kMaxSlices 个 worker session，
     * 所有资源在构造阶段完成，推理时零开销直接使用。
     *
     * @param config 推理器配置
     * @throws std::runtime_error 如果模型加载失败
     */
    explicit OnnxInferer(const OnnxInfererConfig& config);

    ~OnnxInferer() override;

    OnnxInferer(const OnnxInferer&) = delete;
    OnnxInferer& operator=(const OnnxInferer&) = delete;
    OnnxInferer(OnnxInferer&&) noexcept;
    OnnxInferer& operator=(OnnxInferer&&) noexcept;

    // ---- Inferer 接口实现 ----

    InferResult infer(const std::vector<float>& input,
                      const std::vector<int64_t>& shape) override;

    std::vector<InferResult> infer_slices(
        const std::vector<std::vector<float>>& slices) override;

    [[nodiscard]] int output_count() const override;

    [[nodiscard]] const std::string& input_name() const override;

    [[nodiscard]] const std::vector<std::string>& output_names() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    /**
     * Batch 模式：拼接为 [N, C, H, W] 一次推理后拆分
     */
    std::vector<InferResult> infer_slices_batch(
        const std::vector<std::vector<float>>& slices);

    /**
     * 多线程模式：1 切片 = 1 线程 = 1 Session，全部并行推理
     */
    std::vector<InferResult> infer_slices_multithread(
        const std::vector<std::vector<float>>& slices);
};

}  // namespace kflee::dnn
