//
// Created by Darek Rudiš on 07.11.2025.
//

/**
 * @file mediaview.hpp
 * @brief Declares the MediaView VSTGUI view class.
 */

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

/**
 * @brief The view responsible for rendering video from an input stream.
 */
class MediaView final : public VSTGUI::CView, public Steinberg::FObject, public IFrameReceiver {
public:
    explicit MediaView(const VSTGUI::CRect& size)
        : CView(size) { render_queue_ = std::make_unique<VSTGUI::Tasks::Queue>(VSTGUI::Tasks::makeSerialQueue("frame-consumer")); }
    ~MediaView() override;
    void draw(VSTGUI::CDrawContext* dc) override;
    bool removed(CView *parent) override;
    void onFrame(const VideoFrame& frame) override;

    /**
     * @brief Sets a reference to PluginController.
     * @param controller PluginController reference.
     */
    void setController(controller_t controller);
private:
    /**
     * @brief Clears queued render jobs.
     */
    void finishRenderQueue();

    /**
     * @brief Converts the RGBA VideoFrame to a VSTGUI bitmap.
     * @param frame Input VideoFrame.
     */
    void frameToBitmap(const VideoFrame& frame);
    VSTGUI::SharedPointer<VSTGUI::CBitmap> bmp_; /// Current bitmap of the view.
    uint32_t bmp_width_ = 0; /// Width of bmp_
    uint32_t bmp_height_ = 0; /// Height of bmp_
    controller_t controller_ = nullptr; /// PluginController reference
    std::unique_ptr<VSTGUI::Tasks::Queue> render_queue_; /// Bitmap render tasks queue.
    std::shared_ptr<int> render_token_ { std::make_shared<int>(1) }; /// Render tasks lifetime token.
};

/**
 * @brief Resizes the incoming RGBA data to a desired size.
 * @param src RGBA data.
 * @param originWidth Original width.
 * @param originHeight Original height.
 * @param destinationWidth Desired width.
 * @param destinationHeight Desired height.
 * @return Resized vector of RGBA data.
 */
static std::vector<uint8_t> resizeNearestRGBA(const uint8_t* src, int originWidth, int originHeight, int destinationWidth, int destinationHeight);

/**
 * @namespace VSTGUI
 * @brief Wraps VSTGUI view create methods.
 */
namespace VSTGUI {
    /**
	    * @brief Registers the MediaView into the VSTGUI system.
     */
    struct MediaViewCreator : ViewCreatorAdapter {

        MediaViewCreator() { UIViewFactory::registerViewCreator(*this); }

        [[nodiscard]] IdStringPtr getViewName() const override { return "MediaView"; }

        [[nodiscard]] IdStringPtr getBaseViewName() const override { return UIViewCreator::kCView; }

        CView* create(const UIAttributes& attr, const IUIDescription* d) const override;

    };
    static MediaViewCreator __gMediaViewCreator;
}

#endif //MEDIAVIEW_HPP
