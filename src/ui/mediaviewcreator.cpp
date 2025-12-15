//
// Created by Darek Rudiš on 28.11.2025.
//
#include "mediaview.hpp"
#include "motionfxeditor.hpp"

namespace VSTGUI {
    CView* MediaViewCreator::create(const UIAttributes& attr, const IUIDescription* d) const {
        auto* view = new MediaView({0, 0, 100, 100});

        if (const auto* editor = dynamic_cast<MotionFxEditor*>(d->getController()))
        {
            fprintf(stderr, "Creating fresh media view\n");
            editor->setMediaView(view);
        }
        return view;
    }
}
