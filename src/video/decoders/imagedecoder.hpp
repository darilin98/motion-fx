//
// Created by Darek Rudiš on 08.11.2025.
//

#ifndef IMAGEDECODER_HPP
#define IMAGEDECODER_HPP

#include "../medialoader.hpp"

class ImageDecoder final : public IMediaDecoder {
public:
	ImageDecoder() = default;
	~ImageDecoder() override = default;
	bool trySeekTo(int64_t time) override;
	bool tryOpen(const std::string& path) override;
	bool tryDecodeNext(VideoFrame& outFrame) override;
	[[nodiscard]] double frameRate() const override { return 0; }
private:
	VideoFrame cached_frame_;
	bool loaded_ = false;
	bool has_returned_ = false;
};



#endif //IMAGEDECODER_HPP
