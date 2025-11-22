//
// Created by Darek Rudiš on 08.11.2025.
//

#include "videodecoder.hpp"

#include <iostream>

VideoDecoder::VideoDecoder() {
    avformat_network_init();
}

VideoDecoder::~VideoDecoder() {
    cleanup();
    avformat_network_deinit();
}

void VideoDecoder::cleanup() {
    if (sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }
    if (codec_ctx_) {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }
    if (fmt_ctx_) {
        avformat_close_input(&fmt_ctx_);
        fmt_ctx_ = nullptr;
    }
    if (frame_) {
        av_frame_free(&frame_);
        frame_ = nullptr;
    }
    if (pkt_) {
        av_packet_free(&pkt_);
        pkt_ = nullptr;
    }
    video_stream_ = nullptr;
    video_stream_index_ = -1;
    reached_eof_ = false;
    fps_ = 0.0;
    time_base_seconds_ = 0.0;
    dst_buf_.clear();
}

bool VideoDecoder::open(const std::string& path) {
    cleanup();

    fmt_ctx_ = nullptr;
    if (avformat_open_input(&fmt_ctx_, path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "VideoDecoder: failed to open file: " << path << "\n";
        return false;
    }
    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
        std::cerr << "VideoDecoder: failed to find stream info\n";
        cleanup();
        return false;
    }

    int ret = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        std::cerr << "VideoDecoder: no video stream found\n";
        cleanup();
        return false;
    }
    video_stream_index_ = ret;
    video_stream_ = fmt_ctx_->streams[video_stream_index_];

    AVCodecParameters* codecpar = video_stream_->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        std::cerr << "VideoDecoder: no decoder found for codec id " << codecpar->codec_id << "\n";
        cleanup();
        return false;
    }

    codec_ctx_ = avcodec_alloc_context3(codec);
    if (!codec_ctx_) {
        std::cerr << "VideoDecoder: failed to allocate codec context\n";
        cleanup();
        return false;
    }

    if (avcodec_parameters_to_context(codec_ctx_, codecpar) < 0) {
        std::cerr << "VideoDecoder: failed to copy codec parameters to context\n";
        cleanup();
        return false;
    }

    codec_ctx_->thread_count = 1; // single thread for plugin stability

    if (avcodec_open2(codec_ctx_, codec, nullptr) < 0) {
        std::cerr << "VideoDecoder: failed to open codec\n";
        cleanup();
        return false;
    }

    frame_ = av_frame_alloc();
    pkt_ = av_packet_alloc();
    if (!frame_ || !pkt_) {
        std::cerr << "VideoDecoder: failed to allocate frame or packet\n";
        cleanup();
        return false;
    }

    sws_ctx_ = sws_getContext(
        codec_ctx_->width,
        codec_ctx_->height,
        codec_ctx_->pix_fmt,
        codec_ctx_->width,
        codec_ctx_->height,
        AV_PIX_FMT_RGBA,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );

    if (!sws_ctx_) {
        std::cerr << "VideoDecoder: failed to create sws context\n";
        cleanup();
        return false;
    }

    AVRational tb = video_stream_->time_base;
    time_base_seconds_ = tb.num == 0 ? 0.0 : static_cast<double>(tb.num) / static_cast<double>(tb.den);

    AVRational r = codec_ctx_->framerate.num ? codec_ctx_->framerate : video_stream_->r_frame_rate;
    fps_ = (r.num && r.den) ? static_cast<double>(r.num) / static_cast<double>(r.den) : 30.0;

    reached_eof_ = false;
    return true;
}

bool VideoDecoder::seekTo(int64_t time_ms) {
    if (!fmt_ctx_ || video_stream_index_ < 0)
        return false;

    AVRational target_tb = {1, 1000};
    int64_t timestamp = av_rescale_q(time_ms, target_tb, video_stream_->time_base);

    if (av_seek_frame(fmt_ctx_, video_stream_index_, timestamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0) {
        std::cerr << "VideoDecoder: seek failed to " << time_ms << " ms\n";
        return false;
    }

    avcodec_flush_buffers(codec_ctx_);
    reached_eof_ = false;
    return true;
}

bool VideoDecoder::decodeNext(VideoFrame& outFrame) {
    if (!fmt_ctx_ || !codec_ctx_ || !frame_ || !pkt_ || !sws_ctx_)
        return false;

    while (true) {
        int ret = av_read_frame(fmt_ctx_, pkt_);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                av_packet_unref(pkt_);
                pkt_ = av_packet_alloc();
                pkt_->data = nullptr;
                pkt_->size = 0;
                reached_eof_ = true;
                std::cerr << "VideoDecoder: reached EOF successfully" << "\n";
            } else {
                std::cerr << "VideoDecoder: av_read_frame error: " << ret << "\n";
                return false;
            }
        }

        if (!reached_eof_ && pkt_->stream_index == video_stream_index_) {
            ret = avcodec_send_packet(codec_ctx_, pkt_);
            av_packet_unref(pkt_);
            if (ret < 0) {
                if (ret == AVERROR(EAGAIN)) continue;
                std::cerr << "VideoDecoder: avcodec_send_packet failed: " << ret << "\n";
                return false;
            }
        } else if (reached_eof_) {
            ret = avcodec_send_packet(codec_ctx_, nullptr);
            if (ret < 0) {
                std::cerr << "VideoDecoder: avcodec_send_packet (flush) failed: " << av_err2str(ret) << "\n";
                return false;
            }
        } else {
            av_packet_unref(pkt_);
            continue;
        }

        while (true) {
            ret = avcodec_receive_frame(codec_ctx_, frame_);
            if (ret == AVERROR(EAGAIN)) break;
            if (ret == AVERROR_EOF) return false;
            if (ret < 0) {
                std::cerr << "VideoDecoder: avcodec_receive_frame error: " << ret << "\n";
                return false;
            }

            const int w = frame_->width;
            const int h = frame_->height;
            const size_t buf_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, w, h, 1);
            dst_buf_.resize(buf_size);

            uint8_t* dst_data[4] = {dst_buf_.data(), nullptr, nullptr, nullptr};
            int dst_linesize[4] = {av_image_get_linesize(AV_PIX_FMT_RGBA, w, 0), 0, 0, 0};

            sws_scale(sws_ctx_, frame_->data, frame_->linesize, 0, h, dst_data, dst_linesize);

            outFrame.image.width = static_cast<size_t>(w);
            outFrame.image.height = static_cast<size_t>(h);
            outFrame.image.rgba_data = std::move(dst_buf_);

            double ts = 0.0;
            if (frame_->best_effort_timestamp != AV_NOPTS_VALUE) {
                ts = frame_->best_effort_timestamp * static_cast<double>(video_stream_->time_base.num) / static_cast<double>(video_stream_->time_base.den);
            } else if (frame_->pts != AV_NOPTS_VALUE) {
                ts = frame_->pts * static_cast<double>(video_stream_->time_base.num) / static_cast<double>(video_stream_->time_base.den);
            }
            outFrame.timestamp = ts;

            return true;
        }

        if (reached_eof_) return false;
    }
}

double VideoDecoder::frameRate() const {
    return fps_;
}
