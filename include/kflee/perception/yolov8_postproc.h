#pragma once

#include "kflee/typings/types.h"

#include <vector>

namespace kflee::perception {

/**
 * YOLOv8s P2-P4 后处理配置
 *
 * 所有配置项均有合理默认值。对于 P2-P4 输出头格式的 YOLOv8s 模型，
 * 大部分参数（strides、heads 数量等）已硬编码为常量，无需手动设置。
 *
 * 参照 3rdparty/Perception 但不依赖其 yaml-cpp 解析逻辑。
 */
struct YoloV8Config {
    // ---- 模型结构参数 ----
    /** 模型输出 tensor 总数（3 头 * 3 输出/头 = 9） */
    int model_output_count = 9;
    /** 类别数量 */
    int class_num = 1;
    /** DFL 分桶数量，YOLOv8 固定为 16 */
    int reg_max = 16;

    // ---- 后处理阈值 ----
    /** 置信度阈值，低于此值的检测框被丢弃 */
    float conf_threshold = 0.25F;
    /** NMS IoU 阈值 */
    float iou_threshold = 0.45F;

    // ---- NMS 参数 ----
    /** 是否执行 NMS */
    bool do_nms = true;
    /** NMS 前按分数排序保留的候选框最大数量 */
    int nms_top_k = 300;

    // ---- 以下为 P2-P4 固定常量，不可修改 ----
    /** 输出头数量（P2/P3/P4） */
    static constexpr int kNumHeads = 3;
    /** 各输出头的 stride（P2=4, P3=8, P4=16） */
    static constexpr int kStrides[3] = {4, 8, 16};
};

/**
 * 带形状信息的 tensor 数据容器
 *
 * 使用 std::vector<float> 存储数据以避免嵌入式平台上的
 * 原始指针管理问题。不同推理后端只需将输出拷贝到此容器即可。
 */
struct TensorData {
    std::vector<float> data;
    int n = 1;  // batch size
    int c = 0;  // 通道数
    int h = 0;  // 高度
    int w = 0;  // 宽度

    /** 元素总数 */
    [[nodiscard]] std::size_t size() const {
        return data.size();
    }

    /** 是否有效 */
    [[nodiscard]] bool valid() const {
        return !data.empty() && n > 0 && c > 0 && h > 0 && w > 0;
    }
};

/**
 * YOLOv8s P2-P4 后处理器
 *
 * 执行 DFL 解码、坐标变换和可选的 NMS。
 *
 * 使用方式：
 * @code
 *   YoloV8Postprocessor postproc(config);
 *   auto detections = postproc.decode(output_tensors, letterbox);
 *   auto nms_result = postproc.nms(detections);
 * @endcode
 *
 * 或者一键调用：
 * @code
 *   auto detections = postproc.process(output_tensors, letterbox);
 * @endcode
 */
class YoloV8Postprocessor {
public:
    /**
     * 构造后处理器
     *
     * @param config 后处理配置，会进行合法性校验
     * @throws std::invalid_argument 如果配置参数不合法
     */
    explicit YoloV8Postprocessor(const YoloV8Config& config);

    /**
     * DFL 解码 + 坐标变换 + 分数过滤
     *
     * 对模型输出的 9 个 tensor 执行 YOLOv8 split-head 解码，
     * 将 bbox 回归值和分类分数转换为原图坐标系下的检测框。
     * 此步骤不执行 NMS。
     *
     * @param outputs 模型原始输出 tensor 列表，长度必须为 model_output_count (9)
     * @param letterbox 预处理阶段产生的 letterbox 元信息
     * @return 原图坐标系下的候选检测框列表（未经 NMS）
     * @throws std::invalid_argument 如果输出 tensor 数量或形状不匹配
     */
    std::vector<kflee::data::Detection> decode(
        const std::vector<TensorData>& outputs,
        const kflee::data::LetterboxInfo& letterbox) const;

    /**
     * 执行 NMS（非极大值抑制）
     *
     * @param detections 候选检测框列表
     * @return NMS 抑制后的检测框列表
     */
    std::vector<kflee::data::Detection> nms(
        const std::vector<kflee::data::Detection>& detections) const;

    /**
     * 一键执行完整后处理：decode + NMS
     *
     * 等价于 nms(decode(outputs, letterbox))。
     *
     * @param outputs 模型原始输出 tensor 列表
     * @param letterbox 预处理 letterbox 元信息
     * @return NMS 后的最终检测框列表
     */
    std::vector<kflee::data::Detection> process(
        const std::vector<TensorData>& outputs,
        const kflee::data::LetterboxInfo& letterbox) const;

    /**
     * 获取当前配置（只读）
     */
    [[nodiscard]] const YoloV8Config& config() const;

private:
    YoloV8Config config_;

    /** DFL 投影数组 [0, 1, 2, ..., reg_max-1] */
    std::vector<float> projection_;

    /** 输出头规格（内部使用） */
    struct HeadSpec {
        int bbox_index = -1;   // bbox tensor 在 outputs 中的索引
        int score_index = -1;  // score tensor 在 outputs 中的索引
        int stride = 0;        // 该头的降采样倍率
    };

    std::vector<HeadSpec> heads_;

    /** 初始化输出头映射 */
    void init_heads();

    /**
     * 计算 tensor 中指定位置的线性偏移
     */
    static std::size_t tensor_offset(
        const TensorData& tensor, int c, int y, int x);

    /**
     * 校验输入 tensor 的合法性
     */
    void validate_inputs(
        const std::vector<TensorData>& outputs,
        const kflee::data::LetterboxInfo& letterbox) const;
};

/**
 * 计算两个水平框的 IoU
 *
 * @param a 框 A
 * @param b 框 B
 * @return IoU 值 [0, 1]
 */
float compute_iou(const kflee::data::HBox& a, const kflee::data::HBox& b);

}  // namespace kflee::perception
