//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef IFRAMERECEIVER_HPP
#define IFRAMERECEIVER_HPP

#include "videoframe.hpp"

/**
 * @brief Contract for objects which need access to the VideoFrames being processed in the video pipeline.
 */
class IFrameReceiver {
public:
	virtual ~IFrameReceiver() = default;

	/**
	 * @brief An event giving access to the most recent VideoFrame consumed in the video pipeline.
	 * @param frame Most recently processed VideoFrame in the video pipeline.
	 */
	virtual void onFrame(const VideoFrame& frame) = 0 ;
};

using receiver_t = IFrameReceiver*;
using receiver_list_t = std::vector<receiver_t>;

#endif //IFRAMERECEIVER_HPP
