#pragma once

#include "kflee/cropper/sahi_cropper.h"
#include "kflee/typings/slice_types.h"
#include "kflee/typings/types.h"
#include "kflee/dnn/inferer.h"
#include "kflee/preproc/preprocessor.h"

#include <opencv2/core.hpp>

#include <vector>

namespace kflee::pipeline {

/**
 * 推理管线上下文
 *
 * 作为所有 Stage 的共享"行李对象"，贯穿整个流水线。
 * 每个 Stage 读取其上游 Stage 写入的字段，并写入自己产出的字段。
 * 上层只需构造 Frame 并传入 Pipeline::run() 即可。
 */
struct Frame {
    // ================================================================
    // 输入（由调用方填入）
    // ================================================================

    /** 原始输入图像（BGR 格式） */
    cv::Mat image;

    // ================================================================
    // 裁剪阶段（CropStage 写入）
    // ================================================================

    /** SAHI 切片列表 */
    std::vector<kflee::cropper::SliceResult> slices;

    // ================================================================
    // 预处理阶段（PreprocessStage 写入）
    // ================================================================

    /** 每个切片的预处理张量（NCHW float 数组） */
    std::vector<std::vector<float>> tensors;

    /** 每个切片的 letterbox 元信息（用于后处理坐标逆映射） */
    std::vector<kflee::data::LetterboxInfo> letterboxes;

    // ================================================================
    // 推理阶段（InferStage 写入）
    // ================================================================

    /** 每个切片的原始推理输出 */
    std::vector<kflee::dnn::InferResult> infer_results;

    // ================================================================
    // 后处理阶段（PostprocStage + NmsStage 写入）
    // ================================================================

    /** 最终检测结果（原始图像坐标，NMS 后） */
    std::vector<kflee::data::Detection> detections;

    // ================================================================
    // 可视化阶段（VisualizeStage 写入）
    // ================================================================

    /** 可视化输出图像（检测框已绘制） */
    cv::Mat output_image;
};

}  // namespace kflee::pipeline
