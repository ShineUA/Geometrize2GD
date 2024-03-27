#include "ImportPopup.h"
#include <fmt/format.h>

gd::set<short> getGroupIDs(GameObject* obj) {
    gd::set<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.insert(obj->m_groups->at(i));
    return res;
}

int nextFree(int offset)
{
    if(offset <= 0)
    {
        offset = 1;
    }

    std::set<short> usedGroups;
    for (const auto& obj : CCArrayExt<GameObject*>(GameManager::sharedState()->m_levelEditorLayer->m_objects))
    {
        auto groups = getGroupIDs(obj);
        for(const auto& group : groups)
        {
            if(group >= offset)
            {
                usedGroups.insert(group);
            }
        }
    }
    
    int nextFreeGroup = offset;
    for(short used : usedGroups)
    {
        if(nextFreeGroup != used)
        {
            return nextFreeGroup;
        }
        nextFreeGroup += 1;
    }
    return nextFreeGroup;
}

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
    this->m_centerObj = CCArrayExt<GameObject*>(selected_obj)[0];
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
                        if(std::find(this->m_supportedObjsDesktop.begin(), this->m_supportedObjsDesktop.end(), it.value()["type"].get<int>()) != this->m_supportedObjsDesktop.end() && it.value()["score"].get<float>() > 0) {
                            count++;
                        }
                    }
                } else {
                    for(nlohmann::json::iterator it = this->m_jsonSets.begin(); it != this->m_jsonSets.end(); ++it) {
                        if(std::find(this->m_supportedObjsWeb.begin(), this->m_supportedObjsWeb.end(), it.value()["type"].get<int>()) != this->m_supportedObjsWeb.end() && it.value()["score"].get<float>() > 0) {
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
            bool isTransparent = false;
            float transparency;
            int freeGroup = nextFree(0);
            this->m_drawScale = std::stof(static_cast<InputNode*>(this->m_buttonMenu->getChildByID("draw-input"))->getString());
            int z_order = std::stoi(static_cast<InputNode*>(this->m_buttonMenu->getChildByID("zlayer-input"))->getString());
            if(this->m_jsonSets.contains("shapes")) {
                for (nlohmann::json::iterator it = this->m_jsonSets["shapes"].begin(); it != this->m_jsonSets["shapes"].end(); ++it) {
                    if(it.value()["score"].get<float>() > 0 && it.value()["color"][3].get<int>() < 255 && std::find(this->m_supportedObjsDesktop.begin(), this->m_supportedObjsDesktop.end(), it.value()["type"].get<int>()) != this->m_supportedObjsDesktop.end() && it.value() != this->m_jsonSets["shapes"][0] && !isTransparent) {
                        isTransparent = true;
                        transparency = it.value()["color"][3].get<float>() / 255;
                    }
                    if(it.value()["type"].get<int>() == this->m_supportedObjsDesktop[0] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",32," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    } else if(it.value()["type"].get<int>() == this->m_supportedObjsDesktop[1] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",128," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",129," << it.value()["data"][3].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    } else if(it.value()["type"].get<int>() == this->m_supportedObjsDesktop[2] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",128," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",129," << it.value()["data"][3].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",6," << "-" << it.value()["data"][4].get<float>();
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    }
                }
            } else {
                for (nlohmann::json::iterator it = this->m_jsonSets.begin(); it != this->m_jsonSets.end(); ++it) {
                    if(it.value()["score"].get<float>() > 0 && it.value()["color"][3].get<int>() < 255 && std::find(this->m_supportedObjsWeb.begin(), this->m_supportedObjsWeb.end(), it.value()["type"].get<int>()) != this->m_supportedObjsWeb.end() && it.value() != this->m_jsonSets[0] && !isTransparent) {
                        isTransparent = true;
                        transparency = it.value()["color"][3].get<float>() / 255;
                    }
                    if(it.value()["type"].get<int>() == this->m_supportedObjsWeb[0] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",32," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    }// else if(it.value()["type"].get<int>() == this->m_supportedObjsWeb[1] && it.value()["score"].get<float>() > 0) {
                    //     this->m_objsString << "1," << this->m_rect_id;
                    //     this->m_objsString << ",2," << (it.value()["data"][0].get<float>() + it.value()["data"][2].get<float>() / 4 / 2) * this->m_drawScale + this->m_centerObj->getPositionX();
                    //     this->m_objsString << ",3," << (it.value()["data"][1].get<float>() + it.value()["data"][3].get<float>() / 4 / 2) * this->m_drawScale + this->m_centerObj->getPositionY();
                    //     this->m_objsString << ",128," << (it.value()["data"][0].get<float>() - it.value()["data"][2].get<float>()) * this->m_drawScale / 4;
                    //     this->m_objsString << ",129," << (it.value()["data"][1].get<float>() - it.value()["data"][3].get<float>()) * this->m_drawScale / 4;
                    //     this->m_objsString << ",41," << "1";
                    //     this->m_objsString << ",42," << "1";
                    //     this->m_objsString << ",21,1010,22,1010";
                    //     float h, s, v;
                    //     auto r = it.value()["color"][0].get<float>() / 255.f;
                    //     auto g = it.value()["color"][1].get<float>() / 255.f;
                    //     auto b = it.value()["color"][2].get<float>() / 255.f;
                    //     this->rgbToHsv(r, g, b, h, s, v);
                    //     this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    //     this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                    //     this->m_objsString << ",25," << z_order;
                    //     this->m_objsString << ",57," << freeGroup;
                    //     this->m_objsString << ";";
                    //     z_order++;
                    // } 
                    else if(it.value()["type"].get<int>() == this->m_supportedObjsWeb[1] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",128," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",129," << it.value()["data"][3].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    } else if(it.value()["type"].get<int>() == this->m_supportedObjsWeb[2] && it.value()["score"].get<float>() > 0) {
                        this->m_objsString << "1," << this->m_circle_id;
                        this->m_objsString << ",2," << it.value()["data"][0].get<float>() * this->m_drawScale + this->m_centerObj->getPositionX();
                        this->m_objsString << ",3," << it.value()["data"][1].get<float>() * this->m_drawScale + this->m_centerObj->getPositionY();
                        this->m_objsString << ",128," << it.value()["data"][2].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",129," << it.value()["data"][3].get<float>() * this->m_drawScale / 4;
                        this->m_objsString << ",6," << "-" << it.value()["data"][4].get<float>();
                        this->m_objsString << ",41," << "1";
                        this->m_objsString << ",42," << "1";
                        this->m_objsString << ",21,1010,22,1010";
                        float h, s, v;
                        auto r = it.value()["color"][0].get<float>() / 255.f;
                        auto g = it.value()["color"][1].get<float>() / 255.f;
                        auto b = it.value()["color"][2].get<float>() / 255.f;
                        this->rgbToHsv(r, g, b, h, s, v);
                        this->m_objsString << ",43," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",44," << h << "a" << s << "a" << v << "a" << 1 << "a" << 1;
                        this->m_objsString << ",25," << z_order;
                        this->m_objsString << ",57," << freeGroup;
                        this->m_objsString << ";";
                        z_order++;
                    }
                }
            }
            if(this->m_objsString.str().empty()) {
                FLAlertLayer::create(
                    "Info",
                    "No object added.",
                    "OK"
                )->show();
                return this->onClose(nullptr);
            }
            auto curr_editor_layer = LevelEditorLayer::get();
            auto curr_editor_ui = curr_editor_layer->m_editorUI;
            curr_editor_layer->removeObject(this->m_centerObj, false);
            auto obj_arr = curr_editor_layer->createObjectsFromString(this->m_objsString.str().c_str(), true, true);
            curr_editor_ui->flipObjectsY(obj_arr);
            if(isTransparent) {
                obj_arr->addObject(curr_editor_layer->createObjectsFromString(fmt::format("1,1007,2,-15,3,15,51,{},35,{}", freeGroup, transparency).c_str(), true, true)->objectAtIndex(0));
            }
            curr_editor_layer->m_undoObjects->addObject(UndoObject::createWithArray(obj_arr, UndoCommand::Paste));
            curr_editor_ui->selectObjects(obj_arr, true);
            this->m_jsonSets.clear();
            this->keyBackClicked();
            FLAlertLayer::create("Info", "Successfully converted to gd objects!", "OK")->show();
        } catch(...) {
            FLAlertLayer::create("Error", "<cr>Wrong file format!</c> File must be a <cy>JSON</c> output from <cg>Geometrize Demo Website</c> or <cg>Geometrize Desktop App</c>!", "OK")->show();
        }
    } else {
        FLAlertLayer::create("Info", "Nothing to convert!", "OK")->show();
    }
}

void ImportPopup::rgbToHsv(float& fR, float& fG, float fB, float& fH, float& fS, float& fV) {
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

