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
	bool seekTo(int64_t time) override;
	bool open(const std::string& path) override;
	bool decodeNext(VideoFrame& outFrame) override;
	[[nodiscard]] double frameRate() const override;
	[[nodiscard]] bool isStillImage() const override;
private:
	VideoFrame cachedFrame;
	bool loaded = false;
};



#endif //IMAGEDECODER_HPP
