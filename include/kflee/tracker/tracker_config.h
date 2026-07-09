#pragma once

#include <string>

namespace kflee::tracker {

/**
 * 匹配代价类型
 *
 * 小目标/快速移动场景下 IoU 可能始终为 0，此时应使用中心距离。
 */
enum class CostType {
    IoU = 0,            /** 1 - IoU（ByteTrack 原始方式，适合 MOT 类大目标） */
    CenterDistance = 1, /** 基于框中心距离的高斯代价（适合小/快速目标） */
};

/**
 * 跟踪器配置参数
 */
struct TrackerConfig {
    /** 高分检测阈值：score >= 此值的检测框参与第一轮关联 */
    float track_thresh = 0.5F;

    /** 第一轮匹配代价阈值（匈牙利算法的 cost_limit） */
    float match_thresh = 0.8F;

    /** 丢失轨迹保留帧数（在 frame_rate=30 时等价于帧数） */
    int track_buffer = 30;

    /** 视频帧率，用于计算 max_time_lost = frame_rate / 30.0 * track_buffer */
    int frame_rate = 30;

    /** 是否启用调试输出（每帧打印匹配统计信息） */
    bool debug = false;

    /**
     * 第一轮匹配使用的代价类型
     *
     * IoU:           适合 MOT 类大目标场景
     * CenterDistance: 适合小目标/无人机航拍等帧间位移相对自身尺寸较大的场景
     */
    CostType cost_type = CostType::IoU;

    /**
     * 中心距离代价的 sigma 参数（像素）
     *
     * 代价函数: cost = 1 - exp(-dist² / (2 * sigma²))
     *
     * sigma 越大，相同距离的代价越小（匹配越宽松）。
     * 对于 2048x2048 小目标场景，建议 sigma ∈ [30, 100]。
     */
    float center_sigma = 50.0F;

    /**
     * 滤波器类型标识字符串，对应 FilterRegistry 中注册的名称
     *
     * "kf"  = 标准线性 Kalman 滤波器（KfFilter），测量值在调用前转为 [cx,cy,a,h]
     * "ekf" = 扩展卡尔曼滤波器（EkfFilter），直接使用检测器原始 [tlx,tly,w,h]
     *         作为测量值，通过非线性观测函数和 Jacobian 处理 a*h 非线性项
     */
    std::string filter_type = "kf";
};

}  // namespace kflee::tracker
