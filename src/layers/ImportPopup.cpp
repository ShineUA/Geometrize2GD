#include "ImportPopup.h"
#include <typeinfo>

ImportPopup* ImportPopup::create(CCArray* selected_obj) {
    ImportPopup* ret = new ImportPopup();
    if (ret && ret->init(selected_obj)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ImportPopup::init(CCArray* selected_obj) {
    if (!this->initWithColor({0, 0, 0, 75})) return false;

    this->m_noElasticity = true;
    geode::cocos::handleTouchPriority(this);
    this->registerWithTouchDispatcher();
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
    this->setZOrder(150);
    this->m_center_obj = CCArrayExt<GameObject*>(selected_obj)[0];

    auto winSize = CCDirector::get()->getWinSize();

    auto layer = CCLayer::create();
    auto menu = CCMenu::create();
    this->m_mainLayer = layer;
    this->m_buttonMenu = menu;
    
    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize(ccp(385.f, 245.f));
    bg->setPosition(winSize / 2);

    auto label_count = CCLabelBMFont::create("Objects amount: 0", "chatFont.fnt");
    label_count->setPosition({winSize.width / 2 - 174.f, winSize.height / 2 - 35.f});
    label_count->setAnchorPoint({0.f, 0.5f});
    label_count->setVisible(false);
    label_count->setID("count-label");

    auto draw_label = CCLabelBMFont::create("Scale:", "chatFont.fnt");
    draw_label->setPosition({winSize.width / 2 + 119.f, winSize.height / 2});
    draw_label->setVisible(false);
    draw_label->setID("draw-scale-label");

    auto close_btn_spr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    auto close_btn = CCMenuItemSpriteExtra::create(
        close_btn_spr, this, menu_selector(ImportPopup::onExitBtn)
    );
    close_btn->setPosition({-188.f, 119.f});

    auto import_json_btn_spr = ButtonSprite::create("Load JSON");
    auto import_json_btn = CCMenuItemSpriteExtra::create(
        import_json_btn_spr, this, menu_selector(ImportPopup::importJSON)
    );
    import_json_btn->setID("import-btn");
    import_json_btn->setPosition({0.f, 60.f});

    auto change_json_btn_spr = ButtonSprite::create("Change JSON");
    auto change_json_btn = CCMenuItemSpriteExtra::create(
        change_json_btn_spr, this, menu_selector(ImportPopup::importJSON)
    );
    change_json_btn->setVisible(false);
    change_json_btn->setPosition({0.f, 60.f});
    change_json_btn->setID("change-btn");

    auto convert_btn_spr =  ButtonSprite::create("Convert!");
    auto convert_btn = CCMenuItemSpriteExtra::create(
        convert_btn_spr, this, menu_selector(ImportPopup::convert)
    );
    convert_btn->setPosition({0.f, -95.f});

    auto draw_scale_input = InputNode::create(50.f, "Float", "chatFont.fnt", "0123456789.", 5);
    draw_scale_input->setString("1");
    draw_scale_input->setID("draw-input");
    draw_scale_input->setPosition({120.f, -30.f});
    draw_scale_input->setVisible(false);
    
    layer->addChild(bg);
    layer->addChild(label_count);
    layer->addChild(draw_label);
    
    layer->addChild(menu);
    menu->addChild(close_btn);
    menu->addChild(import_json_btn);
    menu->addChild(change_json_btn);
    menu->addChild(convert_btn);
    menu->addChild(draw_scale_input);

    this->addChild(layer);

    return true;
}

void ImportPopup::importJSON(CCObject* sender) {
    utils::file::pickFile(file::PickMode::OpenFile, {}, [this](ghc::filesystem::path const& path) {
        this->m_jsonSets.clear();
        std::ifstream jsonFile(path, std::ifstream::binary);
        // try{
            jsonFile >> this->m_jsonSets;   
            this->m_buttonMenu->getChildByID("import-btn")->setVisible(false);
            this->m_buttonMenu->getChildByID("change-btn")->setVisible(true);
            this->m_buttonMenu->getChildByID("draw-input")->setVisible(true);
            this->m_mainLayer->getChildByID("draw-scale-label")->setVisible(true);
            this->m_mainLayer->getChildByID("count-label")->setVisible(true);
            std::stringstream ss;
            ss << "Objects amount: " << this->m_jsonSets.size();
            static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByID("count-label"))->setString(ss.str().c_str());
            FLAlertLayer::create("Info", "Succesfully imported file", "OK")->show();
        // } catch(...) {
        //     this->m_jsonSets.clear();
        //     FLAlertLayer::create("Error", "<cr>Wrong file!</c> File must be a <cy>JSON</c> output from <cg>Geometrize Demo Website!</c>", "OK")->show();
        // }
    });
}

void ImportPopup::convert(CCObject* sender) {
    if(!this->m_jsonSets.empty()) {
        // try{
            this->m_draw_scale = std::stof(static_cast<InputNode*>(this->m_buttonMenu->getChildByID("draw-input"))->getString());
            int z_order = 0;
            for (Json::Value::ArrayIndex x = 0; x != this->m_jsonSets.size(); x++) {
                if(this->m_jsonSets[x]["type"].asInt() == this->m_circle_type[0] && this->m_jsonSets[x]["score"].asFloat() > 0 || this->m_jsonSets[x]["type"].asInt() == this->m_circle_type[1] && this->m_jsonSets[x]["score"].asFloat() > 0) {
                    this->m_objs_string << "1," << this->m_circle_id;
                    this->m_objs_string << ",2," << this->m_jsonSets[x]["data"][0].asFloat() / this->m_draw_scale + this->m_center_obj->getPositionX();
                    this->m_objs_string << ",3," << this->m_jsonSets[x]["data"][1].asFloat() / this->m_draw_scale + this->m_center_obj->getPositionY();
                    this->m_objs_string << ",32," << this->m_jsonSets[x]["data"][2].asFloat() / this->m_draw_scale / 4;
                    this->m_objs_string << ",41," << "1";
                    this->m_objs_string << ",42," << "1";
                    float h, s, v;
                    auto r = this->m_jsonSets[x]["color"][0].asFloat() / 255.f;
                    auto g = this->m_jsonSets[x]["color"][1].asFloat() / 255.f;
                    auto b = this->m_jsonSets[x]["color"][2].asFloat() / 255.f;
                    this->rgb_to_hsv(r, g, b, h, s, v);
                    this->m_objs_string << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    this->m_objs_string << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    this->m_objs_string << ",25," << z_order;
                    if(x < this->m_jsonSets.size() - 1) {
                        this->m_objs_string << ";";
                    }
                    z_order++;
                }       
            }
            auto curr_editor_layer = LevelEditorLayer::get();
            curr_editor_layer->createObjectsFromString(this->m_objs_string.str().c_str(), true, true);
            this->m_jsonSets.clear();
            this->keyBackClicked();
            FLAlertLayer::create("Info", "Successfully converted to gd objects!", "OK")->show();
        // } catch(...) {
        //     this->m_jsonSets.clear();
        //     FLAlertLayer::create("Error", "<cr>Wrong file format!</c> File must be a <cy>JSON</c> output from <cg>Geometrize Demo Website!</c>", "OK")->show();
        // }
    } else {
        FLAlertLayer::create("Info", "Nothing to convert!", "OK")->show();
    }
}

void ImportPopup::onExitBtn(CCObject* sender) {
    this->keyBackClicked();
}

void ImportPopup::keyBackClicked() {
    this->setTouchEnabled(false);
    this->setKeypadEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void ImportPopup::rgb_to_hsv(float& fR, float& fG, float fB, float& fH, float& fS, float& fV) {
    float fCMax = std::max(std::max(fR, fG), fB);
  float fCMin = std::min(std::min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  
  if(fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH = 360 + fH;
  }
}