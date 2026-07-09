#pragma once

#include <utility>
#include <vector>

namespace kflee::hungarian {

/**
 * 匈牙利算法指派结果
 */
struct AssignmentResult {
    /** 匹配对列表，每个元素为 (行索引, 列索引) */
    std::vector<std::pair<int, int>> matches;
    /** 未匹配的行索引（代价矩阵的行，通常是轨迹） */
    std::vector<int> unmatched_rows;
    /** 未匹配的列索引（代价矩阵的列，通常是检测框） */
    std::vector<int> unmatched_cols;
};

/**
 * 匈牙利线性指派算法（Kuhn-Munkres）
 *
 * 求解矩形代价矩阵的最小总代价 1:1 指派问题。
 * 支持矩形矩阵（行数 != 列数）和代价阈值过滤。
 *
 * 算法复杂度：O(n^3)，其中 n = max(n_rows, n_cols)
 *
 * @param cost    代价矩阵（行优先存储，长度为 n_rows * n_cols）
 * @param n_rows  行数（轨迹数）
 * @param n_cols  列数（检测数）
 * @param thresh  代价阈值，cost >= thresh 的配对视为不可行
 * @return        指派结果（匹配对 + 未匹配的行/列索引）
 */
AssignmentResult solve(const float* cost, int n_rows, int n_cols, float thresh);

}  // namespace kflee::hungarian
