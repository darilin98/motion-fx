//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef IFRAMERECEIVER_HPP
#define IFRAMERECEIVER_HPP

#include "videoframe.hpp"

class IFrameReceiver {
public:
	virtual ~IFrameReceiver() = default;
	virtual void onFrame(const VideoFrame& frame) = 0 ;
};

using receiver_list_t = std::vector<IFrameReceiver*>;

#endif //IFRAMERECEIVER_HPP
