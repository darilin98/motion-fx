//
// Created by Darek Rudiš on 24.10.2025.
//

#ifndef FILESELECTCONTROLLER_HPP
#define FILESELECTCONTROLLER_HPP

#include "vstgui4/vstgui/uidescription/delegationcontroller.h"

class FileSelectController : public VSTGUI::DelegationController
{
public:
    explicit FileSelectController(IController* parent) : DelegationController(parent) {}
    ~FileSelectController() override = default;
    IControlListener* getControlListener(VSTGUI::UTF8StringPtr controlTagName)	override {	return this;}

    void valueChanged(VSTGUI::CControl* pControl) override
    {
        if (pControl->getValue() > 0.5) {
            runFileSelector();
        }
    }
private:
    void runFileSelector() const {
        auto* selector = VSTGUI::CNewFileSelector::create (nullptr,
        VSTGUI::CNewFileSelector::kSelectFile);
        if (selector)
        {
            selector->addFileExtension (VSTGUI::CFileExtension ("JPEG", "jpg"));
            selector->addFileExtension (VSTGUI::CFileExtension ("PNG", "png"));
            selector->addFileExtension (VSTGUI::CFileExtension ("GIF", "gif"));
            selector->addFileExtension (VSTGUI::CFileExtension ("MOV", "mov"));
            selector->addFileExtension (VSTGUI::CFileExtension ("AVI", "avi"));
            selector->setDefaultExtension (VSTGUI::CFileExtension ("MP4", "mp4"));
            selector->setTitle("[ Choose A Media File To Side Chain ]");
            selector->run([this](VSTGUI::CNewFileSelector* sel)
            {
                if (sel->getNumSelectedFiles() > 0)
                {
                    const VSTGUI::UTF8String result = sel->getSelectedFile(0);
                    fprintf(stderr, "Selected: %s\n", result.getString().c_str());
                }
                sel->forget();
            });
        }
    }
};

#endif //FILESELECTCONTROLLER_HPP
