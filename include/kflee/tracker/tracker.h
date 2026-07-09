#pragma once

#include "kflee/typings/types.h"

#include <string>
#include <vector>

namespace kflee::tracker {

/**
 * 多目标跟踪器抽象基类
 *
 * 定义统一的跟踪接口。不同跟踪算法（ByteTrack、SORT 等）
 * 各自实现此接口，Pipeline 中的 TrackStage 仅依赖此抽象。
 *
 * 使用方式：
 * @code
 *   kflee::tracker::TrackerConfig cfg;
 *   cfg.frame_rate = 30;
 *   kflee::tracker::ByteTracker tracker(cfg);
 *
 *   // 逐帧调用
 *   tracker.update(frame_detections);
 *   // frame_detections 中的 track_id 已被原地赋值
 * @endcode
 */
class Tracker {
public:
    virtual ~Tracker() = default;

    /** 禁止拷贝 */
    Tracker(const Tracker&) = delete;
    Tracker& operator=(const Tracker&) = delete;

    /** 允许移动 */
    Tracker(Tracker&&) noexcept = default;
    Tracker& operator=(Tracker&&) noexcept = default;

    /**
     * 对一帧检测结果执行跟踪（检测关联 + Kalman 更新）
     *
     * 原地修改 detections 的 track_id 字段：
     * - 已匹配到已有轨迹的检测：track_id 设为轨迹 ID
     * - 新初始化的轨迹：track_id 设为新分配的 ID
     * - 未匹配的检测：track_id 保持 -1
     *
     * @param detections  输入/输出：检测结果列表（NMS 后，xyxy 坐标）
     */
    virtual void update(std::vector<kflee::data::Detection>& detections) = 0;

    /**
     * 纯 Kalman 预测模式（不做检测关联）
     *
     * 用于隔帧预测场景：当中间帧不做检测推理时，
     * 仅对所有活跃轨迹执行 Kalman 预测，输出预测位置。
     *
     * 不创建新轨迹，不运行 Hungarian 匹配。
     *
     * @param detections  输出：仅包含各轨迹预测的边界框
     */
    virtual void predict_only(std::vector<kflee::data::Detection>& detections) = 0;

    /**
     * 重置跟踪器内部状态
     *
     * 清除所有活跃/丢失/已删除的轨迹，ID 计数器归零。
     * 通常在视频切换或 seek 时调用。
     */
    virtual void reset() = 0;

    /**
     * 获取当前活跃轨迹数量
     */
    [[nodiscard]] virtual int active_track_count() const = 0;

    /**
     * 获取跟踪器名称（用于调试和日志）
     */
    [[nodiscard]] virtual std::string name() const = 0;

protected:
    Tracker() = default;
};

}  // namespace kflee::tracker
