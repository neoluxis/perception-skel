#pragma once

#include "kflee/mot/mot.h"
#include "kflee/mot/mot_config.h"

#include <vector>

namespace kflee::mot {

// 前向声明（私有实现类型）
class Track;

/**
 * ByteTrack 多目标跟踪器实现
 *
 * 基于 "ByteTrack: Multi-Object Tracking by Associating Every Detection Box"
 * (ECCV 2022) 论文的算法实现。
 *
 * 核心策略：
 * 1. 高分检测框优先与已有轨迹关联（IoU + score fusion）
 * 2. 低分检测框用于"救援"暂时丢失的轨迹（二次关联）
 * 3. 只有高于 det_thresh 的未匹配检测才初始化为新轨迹
 * 4. 重复轨迹通过 IoU 去重
 *
 * 使用方式：
 * @code
 *   MotConfig cfg;
 *   cfg.track_thresh = 0.5F;
 *   cfg.frame_rate = 30;
 *   ByteTrack tracker(cfg);
 *
 *   while (has_frame) {
 *       auto detections = run_inference(frame);
 *       tracker.update(detections);
 *       // detections 中的 track_id 已被赋值
 *   }
 * @endcode
 */
class ByteTrack : public Mot {
public:
    /**
     * 构造 ByteTrack
     *
     * @param cfg  跟踪器配置参数
     */
    explicit ByteTrack(const MotConfig& cfg);

    ~ByteTrack() override;

    void update(std::vector<kflee::data::Detection>& detections) override;
    void predict_only(std::vector<kflee::data::Detection>& detections) override;
    void reset() override;
    [[nodiscard]] int active_track_count() const override;
    [[nodiscard]] std::string name() const override;

private:
    MotConfig cfg_;
    std::vector<Track> tracks_;          // 所有轨迹（tracked + lost + removed）
    std::vector<int> reusable_ids_;      // 已回收、可重用的轨迹 ID 池
    int next_track_id_ = 1;              // 递增 ID 颁发器（仅池空时使用）
    int frame_id_ = 0;                   // 当前帧计数器
    float det_thresh_ = 0.6F;            // = track_thresh + 0.1
    int max_time_lost_ = 30;             // = frame_rate / 30.0 * track_buffer

    /**
     * 构建 IoU 代价矩阵：cost[i][j] = 1.0F - IoU(track_i, det_j)
     *
     * @param tracks   轨迹指针列表
     * @param dets     检测结果列表
     * @return         代价矩阵（行优先，size = tracks.size() * dets.size()）
     */
    std::vector<float> build_iou_cost(
        const std::vector<Track*>& tracks,
        const std::vector<kflee::data::Detection*>& dets) const;

    /**
     * 构建中心距离代价矩阵
     *
     * 代价函数: cost = 1 - exp(-dist² / (2 * sigma²))
     * 其中 dist = 框中心之间的欧氏距离，sigma = cfg_.center_sigma。
     *
     * 特点：不依赖框重叠，对小/快速目标更鲁棒。
     *
     * @param tracks   轨迹指针列表
     * @param dets     检测结果列表
     * @return         代价矩阵（行优先，范围 [0, 1)）
     */
    std::vector<float> build_center_distance_cost(
        const std::vector<Track*>& tracks,
        const std::vector<kflee::data::Detection*>& dets) const;

    /**
     * 分数融合：cost *= (1.0F - score * 0.1F)
     * 高分检测的代价降低，在 Hungarian 匹配中获得优势。
     */
    void fuse_score(std::vector<float>& cost,
                    int n_rows, int n_cols,
                    const std::vector<kflee::data::Detection*>& dets) const;

    /**
     * 计算两个边界框的 IoU
     */
    static float compute_iou(const kflee::data::HBox& a,
                              const kflee::data::HBox& b);

    /** HBox (xyxy) → tlwh 数组 */
    static void hbox_to_tlwh(const kflee::data::HBox& box, float tlwh[4]);

    /** tlwh 数组 → HBox (xyxy) */
    static kflee::data::HBox tlwh_to_hbox(const float tlwh[4]);

    /**
     * 执行一轮匹配 + 更新
     *
     * @param tracks           参与匹配的轨迹（引用，会被原地修改）
     * @param dets             参与匹配的检测
     * @param match_thresh     Hungarian 匹配阈值
     * @param use_score_fusion  是否启用分数融合
     * @param cost_type        代价类型（IoU 或 CenterDistance）
     * @param det_matched      输出：标记哪些检测已被匹配
     */
    void match_and_update(std::vector<Track*>& tracks,
                          std::vector<kflee::data::Detection*>& dets,
                          float match_thresh,
                          bool use_score_fusion,
                          CostType cost_type,
                          std::vector<bool>& det_matched);

    /**
     * 去除重复轨迹：两两 IoU > 0.85 的轨迹对，保留轨迹长度更长者
     */
    void remove_duplicates();
};

}  // namespace kflee::mot
