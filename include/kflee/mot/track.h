#pragma once

#include "kflee/filter/filter.hpp"

#include <memory>
#include <string>

namespace kflee::mot {

/**
 * 轨迹状态
 */
enum class TrackState {
    New = 0,       /** 新创建，尚未确认的轨迹 */
    Tracked = 1,   /** 已建立（多次关联成功）的活跃轨迹 */
    Lost = 2,      /** 暂时丢失（当前帧未匹配到检测），保留若干帧等待恢复 */
    Removed = 3,   /** 已永久删除 */
};

/**
 * 单条目标轨迹
 *
 * 封装滤波器状态、生命周期管理和边界框坐标。
 * 内部使用 tlwh（top-left-x, top-left-y, width, height）格式存储。
 *
 * 通过 MotConfig::filter_type 字符串选择滤波器类型：
 *   - "kf"  = KfFilter（标准线性 KF，在 xyah 空间运行）
 *   - "ekf" = EkfFilter（EKF，直接使用原始检测框 [tlx,tly,w,h] 作为测量值）
 *
 * 滤波器通过 FilterRegistry 工厂创建，仅初始化一次（在 Track 构造时）。
 * 热路径（predict/update）完全不涉及字符串处理。
 *
 * 本类是 libmot 的公开内部类型，供具体 tracker 实现使用。
 * 应用层不应直接依赖此类。
 */
class Track {
public:
    Track() = default;

    /**
     * 从检测框构造新轨迹
     *
     * @param tlwh         边界框 [top-left-x, top-left-y, width, height]
     * @param score        检测置信度
     * @param track_id     全局唯一轨迹 ID
     * @param class_id     目标类别 ID
     * @param filter_type  滤波器类型字符串（如 "kf"、"ekf"），对应 FilterRegistry
     */
    Track(const float tlwh[4], float score, int track_id, int class_id,
          const std::string& filter_type = "kf");

    /**
     * 激活轨迹：初始化滤波器并标记为 Tracked
     */
    void activate();

    /**
     * 恢复丢失轨迹：用新检测重新激活
     *
     * @param new_tlwh   重新匹配的检测框
     * @param new_score  新的检测分数
     * @param class_id   新的类别 ID
     */
    void re_activate(const float new_tlwh[4], float new_score, int class_id);

    /**
     * 用匹配的检测框更新轨迹
     *
     * @param new_tlwh   匹配的检测框
     * @param new_score  新的检测分数
     * @param class_id   新的类别 ID
     */
    void update(const float new_tlwh[4], float new_score, int class_id);

    /**
     * 滤波器预测一步（前进一帧）
     */
    void predict();

    /** 标记为丢失 */
    void mark_lost();

    /** 标记为已删除 */
    void mark_removed();

    // === 访问器 ===

    [[nodiscard]] TrackState state() const { return state_; }
    [[nodiscard]] int track_id() const { return track_id_; }
    [[nodiscard]] float score() const { return score_; }
    [[nodiscard]] int class_id() const { return class_id_; }
    /** 轨迹累计匹配次数（用于去重时比较轨迹质量） */
    [[nodiscard]] int tracklet_len() const { return tracklet_len_; }
    /** 距离上次匹配的帧数 */
    [[nodiscard]] int time_since_update() const { return time_since_update_; }
    /** 是否为已激活的轨迹（Tracked 或 Lost 状态） */
    [[nodiscard]] bool is_activated() const {
        return state_ == TrackState::Tracked || state_ == TrackState::Lost;
    }

    /** 获取当前边界框（xyxy 格式） */
    [[nodiscard]] kflee::data::HBox get_hbox() const;

    /** 获取当前边界框（tlwh 格式） */
    void get_tlwh(float tlwh[4]) const;

private:
    TrackState state_ = TrackState::New;
    int track_id_ = 0;
    int class_id_ = -1;            /** 目标类别 */
    float score_ = 0.0F;
    int tracklet_len_ = 0;         /** 累计成功匹配的帧数（激活后开始计数） */
    int time_since_update_ = 0;    /** 距离上次成功匹配的帧数 */

    /** 滤波器实例（通过 FilterRegistry 工厂创建） */
    std::unique_ptr<kflee::filter::Filter> filter_;
};

}  // namespace kflee::mot
