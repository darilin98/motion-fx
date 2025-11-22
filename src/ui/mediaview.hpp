//
// Created by Darek Rudiš on 07.11.2025.
//
#pragma once
#ifndef MEDIAVIEW_HPP
#define MEDIAVIEW_HPP

#include "motionfxeditor.hpp"
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

using controller_t = Steinberg::Vst::EditController*;

class MediaView final : public VSTGUI::CView, public Steinberg::FObject {
public:
    explicit MediaView(const VSTGUI::CRect& size)
        : CView(size) { }
    void setQueue(const frame_queue_t& queue);
    void draw(VSTGUI::CDrawContext* dc) override;
private:
    void updateFromQueue();
    void startConsumingAt(double fps);
    void consumerLoop();
    void stopConsuming();
    void frameToBitmap(VideoFrame&& frame);
    VSTGUI::SharedPointer<VSTGUI::CBitmap> bmp_;
    uint32_t bmp_width_ = 0;
    uint32_t bmp_height_ = 0;
    controller_t controller_ = nullptr;
    frame_queue_t queue_ = nullptr;
    std::unique_ptr<VSTGUI::Tasks::Queue> consumerQueue_ = nullptr;
    std::atomic<bool> consumerRunning_ {false};
    double fps_ = 25.0;
};

static std::vector<uint8_t> resizeNearestRGBA(const uint8_t* src, int originWidth, int originHeight, int destinationWidth, int destinationHeight);

namespace VSTGUI {
    struct MediaViewCreator : ViewCreatorAdapter {

        MediaViewCreator() { UIViewFactory::registerViewCreator(*this); }

        [[nodiscard]] IdStringPtr getViewName() const override { return "MediaView"; }

        [[nodiscard]] IdStringPtr getBaseViewName() const override { return UIViewCreator::kCView; }

        CView* create(const UIAttributes& attr, const IUIDescription* d) const override {
            fprintf(stderr, "creating custom view");
            auto* view = new MediaView({0, 0, 100, 100});

            if (const auto* editor = dynamic_cast<MotionFxEditor*>(d->getController()))
            {
                 view->setQueue(editor->getFrameQueue());
            }
            return view;
        }
    };
    static MediaViewCreator __gMediaViewCreator;
}

#endif //MEDIAVIEW_HPP
