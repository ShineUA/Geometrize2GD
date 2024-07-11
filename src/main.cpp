#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "layers/ImportPopup.h"

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
	void onImgToGD(CCObject*) {
		if(this->getSelectedObjects()->count() == 1) {
			auto importPopup = ImportPopup::create(this->getSelectedObjects());
			CCScene::get()->addChild(importPopup);
		} else {
			FLAlertLayer::create("Info", "You have to choose <cg>exactly one</c> object to be the down-left corner of the image!", "OK")->show();
		}
		
	}

	void createMoveMenu() {
		EditorUI::createMoveMenu();
		auto* btn = this->getSpriteButton("geometrize2gd.png"_spr, menu_selector(MyEditorUI::onImgToGD), nullptr, 0.9f);
		m_editButtonBar->m_buttonArray->addObject(btn);
		auto rows = GameManager::sharedState()->getIntGameVariable("0049");
		auto cols = GameManager::sharedState()->getIntGameVariable("0050");
		m_editButtonBar->reloadItems(rows, cols);
		
	}
};
