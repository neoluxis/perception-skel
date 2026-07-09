#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace kflee::dnn {

// ============================================================================
// 项目常量
// ============================================================================

/** SAHI 3x3 网格固定为 9 个切片 */
constexpr int kMaxSlices = 9;

// ============================================================================
// 切片推理模式
// ============================================================================

enum class SliceInferMode {
    /**
     * 多线程并行模式
     *
     * 每个切片在独立线程中推理，各线程持有独立的推理上下文。
     * 适合板端不支持 batch > 1 的场景，模拟多核 NPU/DSP 各跑各的。
     */
    MultiThread,
    /**
     * Batch 模式
     *
     * 将所有切片沿 batch 维度拼接为 [N, C, H, W] 后一次推理。
     * 适合 GPU / 高性能 PC 场景。
     */
    Batch,
};

// ============================================================================
// 推理结果
// ============================================================================

/**
 * 推理结果
 *
 * 包含模型输出的所有 tensor 数据及其形状信息。
 * 每个输出 tensor 的数据存储在连续的 vector<float> 中（NCHW 布局）。
 */
struct InferResult {
    /** 输出 tensor 数据列表 */
    std::vector<std::vector<float>> tensors;
    /** 每个输出 tensor 的形状 [n, c, h, w] */
    std::vector<std::vector<int64_t>> shapes;
};

// ============================================================================
// 推理器抽象基类
// ============================================================================

/**
 * DNN 推理器抽象基类
 *
 * 定义统一的推理接口，不同平台（ONNX Runtime、TensorRT、NPU SDK 等）
 * 各自实现此接口。上层 pipeline 和 app 只依赖此抽象，不感知具体后端。
 *
 * 使用方式（以 ONNX 为例）：
 * @code
 *   #include "kflee/dnn/onnx/onnx_inferer.h"
 *
 *   kflee::dnn::OnnxInfererConfig cfg;
 *   cfg.model_path = "model.onnx";
 *   kflee::dnn::OnnxInferer inferer(cfg);
 *
 *   // 通过基类指针/引用使用
 *   kflee::dnn::Inferer& base = inferer;
 *   auto result = base.infer(input, shape);
 * @endcode
 */
class Inferer {
public:
    virtual ~Inferer() = default;

    // 禁止拷贝，允许移动
    Inferer(const Inferer&) = delete;
    Inferer& operator=(const Inferer&) = delete;
    Inferer(Inferer&&) noexcept = default;
    Inferer& operator=(Inferer&&) noexcept = default;

    /**
     * 对单个输入 tensor 执行推理
     *
     * @param input 输入数据（NCHW 布局的 float 数组）
     * @param shape 输入形状 [batch, channel, height, width]
     * @return 推理结果
     * @throws std::runtime_error 如果推理失败
     */
    virtual InferResult infer(const std::vector<float>& input,
                               const std::vector<int64_t>& shape) = 0;

    /**
     * 对多个切片批量执行推理
     *
     * 各实现根据自身能力选择 batch 拼接或多线程并行等策略。
     *
     * @param slices 多个切片的预处理数据列表
     * @return 所有切片的推理结果列表（与输入一一对应）
     * @throws std::runtime_error 如果推理失败
     */
    virtual std::vector<InferResult> infer_slices(
        const std::vector<std::vector<float>>& slices) = 0;

    /**
     * 获取模型输出 tensor 数量
     */
    [[nodiscard]] virtual int output_count() const = 0;

    /**
     * 获取模型输入节点名称
     */
    [[nodiscard]] virtual const std::string& input_name() const = 0;

    /**
     * 获取模型输出节点名称列表
     */
    [[nodiscard]] virtual const std::vector<std::string>& output_names() const = 0;

protected:
    /** 仅允许子类构造 */
    Inferer() = default;
};

}  // namespace kflee::dnn
