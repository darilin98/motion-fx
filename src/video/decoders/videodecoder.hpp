//
// Created by Darek Rudiš on 08.11.2025.
//

#ifndef VIDEODECODER_HPP
#define VIDEODECODER_HPP

#include "../medialoader.hpp"


class VideoDecoder final : public IMediaDecoder {
public:
	bool seekTo(int64_t time) override;
	bool open(const std::string& path) override;
	bool decodeNext(VideoFrame& outFrame) override;
	[[nodiscard]] double frameRate() const override;
};

#endif //VIDEODECODER_HPP
