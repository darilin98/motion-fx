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
        fprintf(stderr, "Button called\n");
    }
};

#endif //FILESELECTCONTROLLER_HPP
