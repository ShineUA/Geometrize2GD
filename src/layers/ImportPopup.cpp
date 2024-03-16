#include "ImportPopup.h"
#include <fmt/format.h>

ImportPopup* ImportPopup::create(CCArray* selected_obj) {
    ImportPopup* ret = new ImportPopup();
    if (ret && ret->init(385.f, 245.f, selected_obj)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ImportPopup::setup(CCArray* selected_obj) {
    this->m_center_obj = CCArrayExt<GameObject*>(selected_obj)[0];
    auto winSize = CCDirector::get()->getWinSize();

    auto label_count = CCLabelBMFont::create("Objects: 0", "bigFont.fnt");
    label_count->setPosition({winSize.width / 2, winSize.height / 2 - 65.f});
    label_count->setVisible(false);
    label_count->setID("count-label");
    label_count->setScale(0.4);

    auto draw_label = CCLabelBMFont::create("Scale:", "bigFont.fnt");
    draw_label->setPosition({winSize.width / 2 + 65.f, winSize.height / 2 + 15.f});
    draw_label->setVisible(false);
    draw_label->setID("draw-scale-label");
    draw_label->setScale(0.5);

    auto zlayer_label = CCLabelBMFont::create("Z-Layer\nOffset:", "bigFont.fnt");
    zlayer_label->setPosition({winSize.width / 2 - 65.f, winSize.height / 2 + 15.f});
    zlayer_label->setVisible(false);
    zlayer_label->setID("zlayer-label");
    zlayer_label->setScale(0.325);

    auto selectedFileLabel = CCLabelBMFont::create("", "bigFont.fnt");
    selectedFileLabel->setColor({0,255,0});
    selectedFileLabel->setPosition(winSize.width / 2, winSize.height / 2 + 60.f);
    selectedFileLabel->setScale(0.45);
    selectedFileLabel->setID("file-label");
    selectedFileLabel->setVisible(false);
    
    auto import_json_btn_spr = ButtonSprite::create("Select File");
    auto import_json_btn = CCMenuItemSpriteExtra::create(
        import_json_btn_spr, this, menu_selector(ImportPopup::importJSON)
    );
    import_json_btn->setID("import-btn");

    auto change_json_btn_spr = ButtonSprite::create("Change File");
    auto change_json_btn = CCMenuItemSpriteExtra::create(
        change_json_btn_spr, this, menu_selector(ImportPopup::importJSON)
    );
    change_json_btn->setVisible(false);
    change_json_btn->setPosition({0.f, 90.f});
    change_json_btn->setID("change-btn");

    auto convert_btn_spr =  ButtonSprite::create("Create");
    auto convert_btn = CCMenuItemSpriteExtra::create(
        convert_btn_spr, this, menu_selector(ImportPopup::checkAlert)
    );
    convert_btn->setPosition({0.f, -95.f});
    convert_btn->setVisible(false);
    convert_btn->setID("convert-btn");

    auto draw_scale_input = InputNode::create(50.f, "Float", "bigFont.fnt", "0123456789.", 5);
    draw_scale_input->setString("1");
    draw_scale_input->setID("draw-input");
    draw_scale_input->setPosition({65.f, -15.f});
    draw_scale_input->setVisible(false);
    draw_scale_input->getInput()->setLabelPlaceholderScale(0.5);
    draw_scale_input->getInput()->setMaxLabelScale(0.6);

    auto z_layeroff_input = InputNode::create(50.f, "Int", "bigFont.fnt", "0123456789", 5);
    z_layeroff_input->setString("0");
    z_layeroff_input->setID("zlayer-input");
    z_layeroff_input->setPosition({-65.f, -15.f});
    z_layeroff_input->setVisible(false);
    z_layeroff_input->getInput()->setLabelPlaceholderScale(0.6);
    z_layeroff_input->getInput()->setMaxLabelScale(0.6);
    
    this->m_mainLayer->addChild(selectedFileLabel);
    this->m_mainLayer->addChild(label_count);
    this->m_mainLayer->addChild(draw_label);
    this->m_mainLayer->addChild(zlayer_label);
    
    this->m_buttonMenu->addChild(import_json_btn);
    this->m_buttonMenu->addChild(change_json_btn);
    this->m_buttonMenu->addChild(convert_btn);
    this->m_buttonMenu->addChild(draw_scale_input);
    this->m_buttonMenu->addChild(z_layeroff_input);
    return true;
}

void ImportPopup::importJSON(CCObject* sender) {
    utils::file::pickFile(file::PickMode::OpenFile, {}, [this](ghc::filesystem::path const& path) {
        if(path.string().ends_with(".json")) {
            this->m_jsonSets.clear();
            std::ifstream jsonFile(path, std::ifstream::binary);
            try {
                this->m_jsonSets = nlohmann::json::parse(jsonFile);   
                this->m_buttonMenu->getChildByID("import-btn")->setVisible(false);
                this->m_buttonMenu->getChildByID("change-btn")->setVisible(true);
                this->m_buttonMenu->getChildByID("draw-input")->setVisible(true);
                this->m_buttonMenu->getChildByID("zlayer-input")->setVisible(true);
                this->m_mainLayer->getChildByID("draw-scale-label")->setVisible(true);
                this->m_mainLayer->getChildByID("zlayer-label")->setVisible(true);
                this->m_mainLayer->getChildByID("count-label")->setVisible(true);
                this->m_mainLayer->getChildByID("file-label")->setVisible(true);
                this->m_buttonMenu->getChildByID("convert-btn")->setVisible(true);
                int count = 0;
                if(this->m_jsonSets.contains("shapes")) {
                    for(nlohmann::json::iterator it = this->m_jsonSets["shapes"].begin(); it != this->m_jsonSets["shapes"].end(); ++it) {
                        if(it.value()["type"].get<int>() == this->m_circle_type[0] && it.value()["score"].get<float>() > 0 || it.value()["type"].get<int>() == this->m_circle_type[1] && it.value()["score"].get<float>() > 0) {
                            count++;
                        }
                    }
                } else {
                    for(nlohmann::json::iterator it = this->m_jsonSets.begin(); it != this->m_jsonSets.end(); ++it) {
                        if(it.value()["type"].get<int>() == this->m_circle_type[0] && it.value()["score"].get<float>() > 0 || it.value()["type"].get<int>() == this->m_circle_type[1] && it.value()["score"].get<float>() > 0) {
                            count++;
                        }
                    }
                }
            
                static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByID("count-label"))->setString(fmt::format("Objects: {}", count).c_str());
                static_cast<CCLabelBMFont*>(this->m_mainLayer->getChildByID("file-label"))->setString(fmt::format("File: {}", path.filename()).c_str());
                FLAlertLayer::create("Info", "Succesfully imported file", "OK")->show();
            } catch(...) {
                this->m_jsonSets.clear();
                FLAlertLayer::create("Error", "<cr>File doesn't exists!</c>", "OK")->show();
            }
        } else {
            FLAlertLayer::create("Error", "<cr>Wrong file format</c>\nIt must be a <cy>.json</c> file!", "OK")->show();
        }
    });
}

void ImportPopup::convert(CCObject* sender) {
    if(!this->m_jsonSets.empty()) {
        try {
            this->m_draw_scale = std::stof(static_cast<InputNode*>(this->m_buttonMenu->getChildByID("draw-input"))->getString());
            int z_order = std::stoi(static_cast<InputNode*>(this->m_buttonMenu->getChildByID("zlayer-input"))->getString());
            if(this->m_jsonSets.contains("shapes")) {
                for (nlohmann::json::iterator it = this->m_jsonSets["shapes"].begin(); it != this->m_jsonSets["shapes"].end(); ++it) {
                    if(it.value()["type"].get<int>() == this->m_circle_type[0] && it.value()["score"].get<float>() > 0 || it.value()["type"].get<int>() == this->m_circle_type[1] && it.value()["score"].get<float>() > 0) {
                        this->m_objs_string << "1," << this->m_circle_id;
                        this->m_objs_string << ",2," << it.value()["data"][0].get<float>() * this->m_draw_scale + this->m_center_obj->getPositionX();
                        this->m_objs_string << ",3," << it.value()["data"][1].get<float>() * this->m_draw_scale + this->m_center_obj->getPositionY();
                        this->m_objs_string << ",32," << it.value()["data"][2].get<float>() * this->m_draw_scale / 4;
                        this->m_objs_string << ",41," << "1";
                        this->m_objs_string << ",42," << "1";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgb_to_hsv(r, g, b, h, s, v);
                        this->m_objs_string << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objs_string << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objs_string << ",25," << z_order;
                        if(it != this->m_jsonSets["shapes"].end() - 1) {
                            this->m_objs_string << ";";
                        }
                        z_order++;
                    }
                }
            } else {
                for (nlohmann::json::iterator it = this->m_jsonSets.begin(); it != this->m_jsonSets.end(); ++it) {
                    if(it.value()["type"].get<int>() == this->m_circle_type[0] && it.value()["score"].get<float>() > 0 || it.value()["type"].get<int>() == this->m_circle_type[1] && it.value()["score"].get<float>() > 0) {
                        this->m_objs_string << "1," << this->m_circle_id;
                        this->m_objs_string << ",2," << it.value()["data"][0].get<float>() * this->m_draw_scale + this->m_center_obj->getPositionX();
                        this->m_objs_string << ",3," << it.value()["data"][1].get<float>() * this->m_draw_scale + this->m_center_obj->getPositionY();
                        this->m_objs_string << ",32," << it.value()["data"][2].get<float>() * this->m_draw_scale / 4;
                        this->m_objs_string << ",41," << "1";
                        this->m_objs_string << ",42," << "1";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgb_to_hsv(r, g, b, h, s, v);
                        this->m_objs_string << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objs_string << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objs_string << ",25," << z_order;
                        this->m_objs_string << ";";
                        z_order++;
                    } // else if(this->m_jsonSets[x]["type"].asInt() == this->m_cube_type[0] && this->m_jsonSets[x]["score"].asFloat() > 0 || this->m_jsonSets[x]["type"].asInt() == this->m_cube_type[1] && this->m_jsonSets[x]["score"].asFloat() > 0) {
                    //     this->m_objs_string << "1," << this->m_cube_id;
                    //     this->m_objs_string << ",2," << this->m_jsonSets[x]["data"][0].asFloat() * this->m_draw_scale + this->m_center_obj->getPositionX();
                    //     this->m_objs_string << ",3," << this->m_jsonSets[x]["data"][1].asFloat() * this->m_draw_scale + this->m_center_obj->getPositionY() + 30;
                    //     this->m_objs_string << ",128," << this->m_jsonSets[x]["data"][3].asFloat() * this->m_draw_scale / 15 / 2 / 2 / 2;
                    //     this->m_objs_string << ",129," << this->m_jsonSets[x]["data"][2].asFloat() * this->m_draw_scale / 15 / 2 / 2;
                    //     this->m_objs_string << ",41," << "1";
                    //     this->m_objs_string << ",42," << "1";
                    //     float h, s, v;
                    //     auto r = this->m_jsonSets[x]["color"][0].asFloat() / 255.f;
                    //     auto g = this->m_jsonSets[x]["color"][1].asFloat() / 255.f;
                    //     auto b = this->m_jsonSets[x]["color"][2].asFloat() / 255.f;
                    //     this->rgb_to_hsv(r, g, b, h, s, v);
                    //     this->m_objs_string << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    //     this->m_objs_string << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    //     this->m_objs_string << ",25," << z_order;
                    //     if(x < this->m_jsonSets.size() - 1) {
                    //         this->m_objs_string << ";";
                    //     }
                    //     z_order++;
                    // }
                }
            }
            auto curr_editor_layer = LevelEditorLayer::get();
            curr_editor_layer->createObjectsFromString(this->m_objs_string.str().c_str(), true, true);
            this->m_jsonSets.clear();
            this->keyBackClicked();
            FLAlertLayer::create("Info", "Successfully converted to gd objects!", "OK")->show();
        } catch(...) {
            FLAlertLayer::create("Error", "<cr>Wrong file format!</c> File must be a <cy>JSON</c> output from <cg>Geometrize Demo Website!</c>", "OK")->show();
        }
    } else {
        FLAlertLayer::create("Info", "Nothing to convert!", "OK")->show();
    }
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

void ImportPopup::checkAlert(CCObject* sender) {
    if(this->m_jsonSets.contains("shapes")) {
        if(this->m_jsonSets["shapes"].size() > 5000) {
            geode::createQuickPopup(
                "Alert",            // title
                "This will place more than <cy>5000 objects</c>\nAre you sure?",   // content
                "Yes", "No",      // buttons
                [this](auto, bool btn2) {
                    if (!btn2) {
                        this->convert(nullptr); // say hi to mom
                    }
                }
            );
        } else {
            this->convert(nullptr);
        }
    } else {
        if(this->m_jsonSets.size() > 5000) {
            geode::createQuickPopup(
                "Alert",            // title
                "This will place more than <cy>5000 objects</c>\nAre you sure?",   // content
                "Yes", "No",      // buttons
                [this](auto, bool btn2) {
                    if (!btn2) {
                        this->convert(nullptr); // say hi to mom
                    }
                }
            );
        } else {
            this->convert(nullptr);
        }
    }
}

