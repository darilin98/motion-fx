//
// Created by Darek Rudiš on 23.11.2025.
//

#ifndef BUTTONACTIONCONTROLLER_HPP
#define BUTTONACTIONCONTROLLER_HPP

#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui4/vstgui/uidescription/delegationcontroller.h"

class ButtonActionController : public VSTGUI::DelegationController
{
public:
	explicit ButtonActionController(IController* parent) : DelegationController(parent) {}
	~ButtonActionController() override = default;
	IControlListener* getControlListener(VSTGUI::UTF8StringPtr controlTagName)	override {	return this;}

	void valueChanged(VSTGUI::CControl* pControl) override
	{
		if (pControl->getValue() > 0.5) {
			runAction();
		}
	}
	std::function<void()> action;
private:
	void runAction() const {
		if (action)
			action();
	}
};


#endif //BUTTONACTIONCONTROLLER_HPP
