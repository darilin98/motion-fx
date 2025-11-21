//
// Created by Darek Rudiš on 08.11.2025.
//

#include "videodecoder.hpp"

VideoDecoder::VideoDecoder(const std::string& extHint) {
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