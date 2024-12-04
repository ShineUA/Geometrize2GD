#include "ImportPopup.h"

ImportPopup* ImportPopup::create(CCArray* selected_obj) {
    ImportPopup* ret = new ImportPopup();
    if (ret && ret->initAnchored(385.f, 245.f, selected_obj)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ImportPopup::setup(CCArray* selected_obj) {
    this->m_centerObj = CCArrayExt<GameObject*>(selected_obj)[0];

    this->m_countLabel = CCLabelBMFont::create("Objects: 0", "bigFont.fnt");
    this->m_countLabel->setPosition({ImportPopup::m_popupSize.width / 2, ImportPopup::m_popupSize.height / 2 - 65.f});
    this->m_countLabel->setVisible(false);
    this->m_countLabel->setID("count-label");
    this->m_countLabel->setScale(0.4);

    auto drawLabel = CCLabelBMFont::create("Scale:", "bigFont.fnt");
    drawLabel->setPosition({ImportPopup::m_popupSize.width / 2 + 65.f, ImportPopup::m_popupSize.height / 2 + 15.f});
    drawLabel->setVisible(false);
    drawLabel->setID("draw-scale-label");
    drawLabel->setScale(0.5);

    auto zLayerLabel = CCLabelBMFont::create("Z-Layer\nOffset:", "bigFont.fnt");
    zLayerLabel->setPosition({ImportPopup::m_popupSize.width / 2 - 65.f, ImportPopup::m_popupSize.height / 2 + 15.f});
    zLayerLabel->setVisible(false);
    zLayerLabel->setID("zlayer-label");
    zLayerLabel->setScale(0.325);

    this->m_fileLabel = CCLabelBMFont::create("", "bigFont.fnt");
    this->m_fileLabel->setColor({0,255,0});
    this->m_fileLabel->setPosition(ImportPopup::m_popupSize.width / 2, ImportPopup::m_popupSize.height / 2 + 60.f);
    this->m_fileLabel->setScale(0.45);
    this->m_fileLabel->setID("file-label");
    this->m_fileLabel->setVisible(false);

    auto importJsonSpr = ButtonSprite::create("Select File");
    auto importJsonBtn = CCMenuItemSpriteExtra::create(
        importJsonSpr, this, menu_selector(ImportPopup::importJSON)
    );
    importJsonBtn->setID("import-btn");
    importJsonBtn->setPosition(importJsonBtn->getPosition() + ImportPopup::m_popupSize / 2);

    auto changeJsonSpr = ButtonSprite::create("Change File");
    auto changeJsonBtn = CCMenuItemSpriteExtra::create(
        changeJsonSpr, this, menu_selector(ImportPopup::importJSON)
    );
    changeJsonBtn->setVisible(false);
    changeJsonBtn->setPosition({ImportPopup::m_popupSize.width / 2, ImportPopup::m_popupSize.height / 2 + 90.f});
    changeJsonBtn->setID("change-btn");

    auto parseSpr =  ButtonSprite::create("Create");
    auto parseBtn = CCMenuItemSpriteExtra::create(
        parseSpr, this, menu_selector(ImportPopup::checkAlert)
    );
    parseBtn->setPosition({ImportPopup::m_popupSize.width / 2, ImportPopup::m_popupSize.height / 2 - 95.f});
    parseBtn->setVisible(false);
    parseBtn->setID("convert-btn");

    this->m_drawScaleInput = TextInput::create(50.f, "Float", "bigFont.fnt");
    this->m_drawScaleInput->setCommonFilter(CommonFilter::Float);
    this->m_drawScaleInput->setMaxCharCount(5);
    this->m_drawScaleInput->setString("1");
    this->m_drawScaleInput->setID("draw-input");
    this->m_drawScaleInput->setPosition({ImportPopup::m_popupSize.width / 2 + 65.f,ImportPopup::m_popupSize.height / 2 - 15.f});
    this->m_drawScaleInput->setVisible(false);
    this->m_drawScaleInput->getInputNode()->setLabelPlaceholderScale(0.5);
    this->m_drawScaleInput->getInputNode()->setMaxLabelScale(0.6);
    this->m_drawScaleInput->setDelegate(static_cast<TextInputDelegate*>(this));

    this->m_zLayerInput = TextInput::create(50.f, "Int", "bigFont.fnt");
    this->m_zLayerInput->setCommonFilter(CommonFilter::Int);
    this->m_zLayerInput->setMaxCharCount(5);
    this->m_zLayerInput->setString("0");
    this->m_zLayerInput->setID("zlayer-input");
    this->m_zLayerInput->setPosition({ImportPopup::m_popupSize.width / 2 - 65.f, ImportPopup::m_popupSize.height / 2 - 15.f});
    this->m_zLayerInput->setVisible(false);
    this->m_zLayerInput->getInputNode()->setLabelPlaceholderScale(0.6);
    this->m_zLayerInput->getInputNode()->setMaxLabelScale(0.6);
    this->m_zLayerInput->setDelegate(static_cast<TextInputDelegate*>(this));

    this->m_mainLayer->addChild(this->m_fileLabel);
    this->m_mainLayer->addChild(this->m_countLabel);
    this->m_mainLayer->addChild(drawLabel);
    this->m_mainLayer->addChild(zLayerLabel);

    this->m_buttonMenu->addChild(importJsonBtn);
    this->m_buttonMenu->addChild(changeJsonBtn);
    this->m_buttonMenu->addChild(parseBtn);
    this->m_buttonMenu->addChild(this->m_drawScaleInput);
    this->m_buttonMenu->addChild(this->m_zLayerInput);
    return true;
}

void ImportPopup::importJSON(CCObject* sender) {
    file::FilePickOptions::Filter filter = {
        .description = "Geometrize JSON Output",
        .files = { "*.json"}
    };
    file::FilePickOptions options = {
        std::nullopt,
        {filter}
    };

    m_pickListener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
        if (event->isCancelled()) {
            return Notification::create(
                "Failed to open file (Task Cancelled)",
                NotificationIcon::Error
            )->show();
        }
        if (auto result = event->getValue()) {
            if(result->isErr()) {
                return Notification::create(
                    fmt::format("Failed to open file. Error: {}", result->err()),
                    NotificationIcon::Error
                )->show();
            }
            auto path = result->unwrap();
            if (path.string().ends_with(".json")) {
                unsigned long fileSize = 0;
                unsigned char* buffer = CCFileUtils::sharedFileUtils()->getFileData(
                    path.string().c_str(),
                    "rb",
                    &fileSize
                );
                std::string data = std::string(reinterpret_cast<char*>(buffer), fileSize);
                auto optValue = matjson::parse(data);
                if (optValue.isErr()) {
                    return Notification::create(
                        "Failed to parse JSON!",
                        NotificationIcon::Error
                    )->show();
                }
                this->m_jsonSets = optValue.unwrap();

                if (auto temp = this->m_jsonSets["shapes"].asArray()) {
                    this->m_jsonSets = temp.unwrap();
                } else if (auto temp = this->m_jsonSets.asArray()) {
                    this->m_jsonSets = temp.unwrap();
                } else {
                    return Notification::create(
                        "Failed to parse \"shapes\" key as array!",
                        NotificationIcon::Error
                    )->show();
                }

                for (auto obj : this->m_jsonSets) {
                    auto objType = obj["type"].asInt();
                    auto objScore = obj["score"].asDouble();

                    if (!objType) {
                        continue;
                    }
                    if (!objScore) {
                        continue;
                    }
                    if (!this->m_validTypes.contains(
                        objType.unwrap())) {
                        continue;
                    }
                    if (objScore.unwrap() <= 0) {
                        continue;
                    }
                    this->m_objsCount++;
                }

                auto countText = fmt::format("Objects: {}", this->m_objsCount);
                auto fileText = fmt::format("File: {}", event->getValue()->unwrap().filename());
                this->m_countLabel->setString(countText.c_str());
                this->m_fileLabel->setString(fileText.c_str());
                this->m_buttonMenu->getChildByID("import-btn")->setVisible(false);
                this->m_buttonMenu->getChildByID("change-btn")->setVisible(true);
                this->m_mainLayer->getChildByID("draw-scale-label")->setVisible(true);
                this->m_mainLayer->getChildByID("zlayer-label")->setVisible(true);
                this->m_buttonMenu->getChildByID("convert-btn")->setVisible(true);
                this->m_fileLabel->setVisible(true);
                this->m_countLabel->setVisible(true);
                this->m_zLayerInput->setVisible(true);
                this->m_drawScaleInput->setVisible(true);
                this->m_objsCount = 0;

                Notification::create(
                    "File is imported",
                    NotificationIcon::Success
                )->show();
            } else {
                Notification::create(
                    "Wrong file format. It must be a .json file!",
                    NotificationIcon::Error
                )->show();
            }
        }
    });
    m_pickListener.setFilter(file::pick(file::PickMode::OpenFile, options));
}

void ImportPopup::parse() {
    for (auto obj : this->m_jsonSets) {
        auto objScore = obj["score"].asDouble();
        if (!objScore) {
            continue;
        }
        if(objScore.unwrap() <= 0) {
            continue;
        }

        float posX = this->m_centerObj->getPositionX();
        auto posXResult = obj["data"][0].asDouble();
        float posY = this->m_centerObj->getPositionY();
        auto posYResult = obj["data"][1].asDouble();
        float scaleX = 1.f;
        auto scaleXResult = obj["data"][2].asDouble();
        float scaleY = 1.f;
        auto scaleYResult = obj["data"][3].asDouble();
        float rotation = 0.f;
        auto rotationResult = obj["data"][4].asDouble();
        auto redResult = obj["color"][0].asDouble();
        auto blueResult = obj["color"][1].asDouble();
        auto greenResult = obj["color"][2].asDouble();
        float h = 0.f;
        float s = 0.f;
        float v = 0.f;

        if (posXResult) {
            posX = posXResult.unwrap() * this->m_drawScale + this->m_centerObj->getPositionX();
        }
        if (posYResult) {
            posY = posYResult.unwrap() * this->m_drawScale + this->m_centerObj->getPositionY();
        }
        if (scaleXResult) {
            scaleX = scaleXResult.unwrap() * this->m_drawScale / 4 * 0.16;
        }
        if (scaleYResult) {
            scaleY = scaleYResult.unwrap() * this->m_drawScale / 4 * 0.16;
        } else {
            scaleY = scaleX;
        }
        if (rotationResult) {
            rotation = -rotationResult.unwrap();
        }
        if (redResult && blueResult && greenResult) {
            this->rgbToHsv(
                redResult.unwrap() / 255.f,
                blueResult.unwrap() / 255.f,
                greenResult.unwrap() / 255.f,
                h,s,v
            );
        }
        auto parsedObj = fmt::format(
            "1,{},2,{},3,{},128,{},129,{},6,{},41,1,42,1,21,1010,22,1010,43,{}a{}a{}a1a1,44,{}a{}a{}a1a1,25,{},372,1;",
            this->m_circle_id,
            posX,
            posY,
            scaleX,
            scaleY,
            rotation,
            h,s,v,
            h,s,v,
            this->m_zOrder
        );
        this->m_objsString << parsedObj;
        this->m_zOrder++;
    }
    if (this->m_objsString.str().empty()) {
        Notification::create(
            "No objects added.",
            NotificationIcon::Error
        )->show();
        return this->onClose(nullptr);
    }
    auto curr_editor_layer = LevelEditorLayer::get();
    auto curr_editor_ui = curr_editor_layer->m_editorUI;
    curr_editor_ui->onDeleteSelected(nullptr);
    auto obj_arr = curr_editor_layer->createObjectsFromString(this->m_objsString.str().c_str(), true, true);
    curr_editor_ui->flipObjectsY(obj_arr);
    curr_editor_layer->m_undoObjects->addObject(UndoObject::createWithArray(obj_arr, UndoCommand::Paste));
    curr_editor_ui->selectObjects(obj_arr, true);
    curr_editor_ui->updateButtons();
    this->keyBackClicked();
    Notification::create(
        "Successfully converted to gd objects!",
        NotificationIcon::Success
    )->show();
}

void ImportPopup::rgbToHsv(float fR, float fG, float fB, float& fH, float& fS, float& fV) {
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

        if (fCMax > 0) {
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
    if (this->m_objsCount > 5000) {
        geode::createQuickPopup(
            "Alert",
            "This will place more than <cy>5000 objects</c>\nAre you sure?",
            "No", "Yes",
            [this](auto, bool btn2) {
                if (btn2) {
                    this->parse();
                }
            }
        );
    } else {
        this->parse();
    }
}

void ImportPopup::textChanged(CCTextInputNode *p0) {
    if (p0 == this->m_drawScaleInput->getInputNode()) {
        auto num = utils::numFromString<float>(p0->getString());
        if (num.isErr()) {
            return p0->setLabelNormalColor(ccColor3B(255,0,0));
        }
        auto numUn = num.unwrap();
        if (numUn <= 0) {
            return p0->setLabelNormalColor(ccColor3B(255,0,0));
        }
        p0->setLabelNormalColor(ccColor3B(255,255,255));
        this->m_drawScale = numUn;
    } else if (p0 == this->m_zLayerInput->getInputNode()) {
        auto num = utils::numFromString<int>(p0->getString());
        if (num.isErr()) {
            return p0->setLabelNormalColor(ccColor3B(255,0,0));
        }
        auto numUn = num.unwrap();
        p0->setLabelNormalColor(ccColor3B(255,255,255));
        this->m_zOrder = numUn;
    }
}


