//
// Created by Darek Rudiš on 07.11.2025.
//
#pragma once
#ifndef MEDIAVIEW_HPP
#define MEDIAVIEW_HPP

#include "../video/framequeue.hpp"
#include "vstgui/lib/cview.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/iviewcreator.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/uiviewfactory.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/idependency.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/uidescription/editing/uiviewcreatecontroller.h"
#include "vstgui/lib/tasks.h"
#include "../video/iframereceiver.hpp"

using controller_t = Steinberg::Vst::EditController*;

class MediaView final : public VSTGUI::CView, public Steinberg::FObject, public IFrameReceiver {
public:
    explicit MediaView(const VSTGUI::CRect& size)
        : CView(size) { render_queue_ = std::make_unique<VSTGUI::Tasks::Queue>(VSTGUI::Tasks::makeSerialQueue("frame-consumer")); }
    ~MediaView() override;
    void draw(VSTGUI::CDrawContext* dc) override;
    bool removed(CView *parent) override;
    void onFrame(const VideoFrame& frame) override;
    void setController(controller_t controller);
private:
    void finishRenderQueue();
    void frameToBitmap(const VideoFrame& frame);
    VSTGUI::SharedPointer<VSTGUI::CBitmap> bmp_;
    uint32_t bmp_width_ = 0;
    uint32_t bmp_height_ = 0;
    controller_t controller_ = nullptr;
    frame_queue_t queue_ = nullptr;
    std::unique_ptr<VSTGUI::Tasks::Queue> render_queue_;
    std::shared_ptr<int> render_token_ { std::make_shared<int>(1) };
};

static std::vector<uint8_t> resizeNearestRGBA(const uint8_t* src, int originWidth, int originHeight, int destinationWidth, int destinationHeight);

namespace VSTGUI {
    struct MediaViewCreator : ViewCreatorAdapter {

        MediaViewCreator() { UIViewFactory::registerViewCreator(*this); }

        [[nodiscard]] IdStringPtr getViewName() const override { return "MediaView"; }

        [[nodiscard]] IdStringPtr getBaseViewName() const override { return UIViewCreator::kCView; }

        CView* create(const UIAttributes& attr, const IUIDescription* d) const override;

    };
    static MediaViewCreator __gMediaViewCreator;
}

#endif //MEDIAVIEW_HPP
