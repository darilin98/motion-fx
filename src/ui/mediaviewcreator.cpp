//
// Created by Darek Rudiš on 28.11.2025.
//
#include "mediaview.hpp"
#include "visimusiceditor.hpp"

namespace VSTGUI {
    CView* MediaViewCreator::create(const UIAttributes& attr, const IUIDescription* d) const {
        auto* view = new MediaView({0, 0, 100, 100});

        if (const auto* editor = dynamic_cast<VisiMusicEditor*>(d->getController()))
        {
            if (auto* pcont = dynamic_cast<PluginController*>(editor->getController())) {
                view->setController(pcont);
                pcont->registerReceiver(view);
            }
        }
        return view;
    }
}
