#pragma once

#include <cstdint>

namespace kflee::data {

/**
 * 切片信息
 *
 * 描述 SAHI 切片在原图中的位置和尺寸，用于切片坐标与原图坐标
 * 之间的相互映射。
 */
struct SliceInfo {
    /** 切片在原图中的左上角 x 坐标 */
    int offset_x = 0;
    /** 切片在原图中的左上角 y 坐标 */
    int offset_y = 0;
    /** 切片宽度（像素） */
    int width = 640;
    /** 切片高度（像素） */
    int height = 640;
    /** 切片在网格中的行索引（从 0 开始） */
    int grid_row = 0;
    /** 切片在网格中的列索引（从 0 开始） */
    int grid_col = 0;
    /** 切片在 3x3 网格中的线性索引（0-8） */
    int slice_index = 0;
};

}  // namespace kflee::data
