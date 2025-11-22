//
// Created by Darek Rudiš on 08.11.2025.
//

#ifndef VIDEODECODER_HPP
#define VIDEODECODER_HPP

#include "../medialoader.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
}

class VideoDecoder final : public IMediaDecoder {
public:
	explicit VideoDecoder();
	~VideoDecoder() override;
	bool seekTo(int64_t time) override;
	bool open(const std::string& path) override;
	bool decodeNext(VideoFrame& outFrame) override;
	[[nodiscard]] double frameRate() const override;
private:
	void cleanup();

	AVFormatContext* fmt_ctx_ = nullptr;
	AVCodecContext* codec_ctx_ = nullptr;
	AVStream* video_stream_ = nullptr;
	int video_stream_index_ = -1;
	SwsContext* sws_ctx_ = nullptr;
	AVFrame* frame_ = nullptr;
	AVPacket* pkt_ = nullptr;

	double time_base_seconds_ = 0.0;
	double fps_ = 0.0;
	bool reached_eof_ = false;

	std::string ext_hint_;
	std::vector<uint8_t> dst_buf_;
};

#endif //VIDEODECODER_HPP
