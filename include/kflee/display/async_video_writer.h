#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace cv {
class Mat;
class VideoWriter;
}

namespace kflee::display {

/**
 * 异步视频写入器
 *
 * 在独立线程中执行 cv::VideoWriter::write()，避免视频编码阻塞
 * 主推理流程，确保帧率和耗时统计不受 I/O 干扰。
 *
 * 内部维护一个帧队列，主线程通过 write() 非阻塞入队，
 * 后台线程消费队列并编码写入。析构时自动等待队列清空。
 *
 * 队列长度超过 kMaxQueueSize 时，write() 会阻塞等待，
 * 防止内存无限增长。
 *
 * 使用方式：
 * @code
 *   AsyncVideoWriter writer("output.mp4", 30.0, 2048, 2048);
 *   for (auto& frame : frames) {
 *       writer.write(frame);  // 非阻塞
 *   }
 *   writer.close();  // 或依赖析构自动关闭
 * @endcode
 */
class AsyncVideoWriter {
public:
    /**
     * 构造并启动后台写入线程
     *
     * @param output_path 输出视频文件路径
     * @param fps 输出视频帧率
     * @param width 帧宽度
     * @param height 帧高度
     * @param fourcc 视频编码 FourCC 代码，默认为 mp4v
     * @throws std::runtime_error 如果无法打开输出文件
     */
    AsyncVideoWriter(const std::string& output_path,
                     double fps,
                     int width,
                     int height,
                     int fourcc = 0);

    ~AsyncVideoWriter();

    // 禁止拷贝
    AsyncVideoWriter(const AsyncVideoWriter&) = delete;
    AsyncVideoWriter& operator=(const AsyncVideoWriter&) = delete;

    /**
     * 写入一帧（非阻塞）
     *
     * 将帧放入后台队列后立即返回。主线程调用此方法不会因视频编码而阻塞。
     * 当队列长度达到上限时会短暂阻塞等待消费者。
     *
     * @param frame 待写入的帧（BGR 格式），内部会 clone 一份
     */
    void write(const cv::Mat& frame);

    /**
     * 关闭写入器
     *
     * 发送停止信号，等待后台线程写完剩余帧后关闭文件。
     * 多次调用安全，析构时自动调用。
     */
    void close();

    /**
     * 队列当前长度
     */
    [[nodiscard]] int queue_size() const;

private:
    /** 队列最大长度，超过后 write() 阻塞 */
    static constexpr int kMaxQueueSize = 16;

    /** 后台写入线程 */
    void writer_loop();

    std::unique_ptr<cv::VideoWriter> writer_;
    std::thread writer_thread_;
    std::queue<cv::Mat> frame_queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_producer_;  // 生产者等待队列非满
    std::condition_variable cv_consumer_;  // 消费者等待队列非空
    std::atomic<bool> running_{true};
    std::atomic<bool> finished_{false};
};

}  // namespace kflee::display
