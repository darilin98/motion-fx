//
// Created by Darek Rudiš on 07.11.2025.
//

#ifndef MEDIAVIEW_HPP
#define MEDIAVIEW_HPP

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

using controller_t = Steinberg::Vst::EditController*;

class MediaView : public VSTGUI::CView, public Steinberg::FObject {
public:
    explicit MediaView(const VSTGUI::CRect& size)
        : CView(size) { }
    void draw(VSTGUI::CDrawContext* dc) override;
private:
    controller_t _controller = nullptr;
};

namespace VSTGUI {
    struct MediaViewCreator : public ViewCreatorAdapter {

        MediaViewCreator() { UIViewFactory::registerViewCreator(*this); }

        [[nodiscard]] IdStringPtr getViewName() const override { return "MediaView"; }

        [[nodiscard]] IdStringPtr getBaseViewName() const override { return UIViewCreator::kCView; }

        CView* create(const UIAttributes& attr, const IUIDescription* d) const override {
            fprintf(stderr, "creating custom view");
            auto* view = new MediaView({0, 0, 100, 100});

            if (const auto* controller = dynamic_cast<VST3Editor*>(d->getController()))
            {
                if (auto* editController = controller->getController())
                {
                    // Setup view
                }
            }
            return view;
        }
    };
    static MediaViewCreator __gMediaViewCreator;
}

#endif //MEDIAVIEW_HPP
