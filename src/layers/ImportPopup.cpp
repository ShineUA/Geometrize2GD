#include "ImportPopup.h"

ImportPopup* ImportPopup::create(CCArray* selectedObj) {
    ImportPopup* ret = new ImportPopup();
    if (ret && ret->initAnchored(385.f, 245.f, selectedObj)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

// Setups the layer
bool ImportPopup::setup(CCArray* selectedObj) {
    this->m_centerObj = CCArrayExt<GameObject*>(selectedObj)[0];

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
    // Setting file pick options
    file::FilePickOptions::Filter filter = {
        .description = "Geometrize JSON Output",
        .files = { "*.json"}
    };
    file::FilePickOptions options = {
        std::nullopt,
        {filter}
    };

    // Binding a function to listener
    m_pickListener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
        // Notifies user if the file pick menu is just closed
        if (event->isCancelled()) {
            return Notification::create(
                "Failed to open file (Task Cancelled)",
                NotificationIcon::Error
            )->show();
        }
        // Gets Result and checks if null
        if (auto result = event->getValue()) {
            // Checks does Result is empty or not
            if(result->isErr()) {
                return Notification::create(
                    fmt::format("Failed to open file. Error: {}", result->err()),
                    NotificationIcon::Error
                )->show();
            }
            // Unwraps result into std::filesystem::path and checks does it end with ".json"
            auto path = result->unwrap();
            if (path.string().ends_with(".json")) {

                // Reads the json file and converts output to std::string
                unsigned long fileSize = 0;
                unsigned char* buffer = CCFileUtils::sharedFileUtils()->getFileData(
                    path.string().c_str(),
                    "rb",
                    &fileSize
                );
                std::string data = std::string(reinterpret_cast<char*>(buffer), fileSize);

                // Parses json
                auto jsonResult = matjson::parse(data);
                if (jsonResult.isErr()) {
                    return Notification::create(
                        "Failed to parse JSON!",
                        NotificationIcon::Error
                    )->show();
                }
                this->m_jsonSets = jsonResult.unwrap();

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

                // Counts the objects
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

// Parses the objects from Geometrize to GD format and places the objects inside GD Editor
void ImportPopup::parseAndPlace() {
    for (auto obj : this->m_jsonSets) {
        // Avoiding objects with zero score
        auto objScore = obj["score"].asDouble();
        if (!objScore || objScore.unwrap() <= 0) {
            continue;
        }

        // Setting and initializing default properties if there is missing some
        float posX = this->m_centerObj->getPositionX();
        float posY = this->m_centerObj->getPositionY();
        float scaleX = 1.f, scaleY = 1.f, rotation = 0.f;
        auto redResult = obj["color"][0].asDouble();
        auto blueResult = obj["color"][1].asDouble();
        auto greenResult = obj["color"][2].asDouble();

        // Parsing object's general properties
        if (auto posXResult = obj["data"][0].asDouble()) {
            posX = posXResult.unwrap() * this->m_drawScale + this->m_centerObj->getPositionX();
        }
        if (auto posYResult = obj["data"][1].asDouble()) {
            posY = posYResult.unwrap() * this->m_drawScale + this->m_centerObj->getPositionY();
        }
        if (auto scaleXResult = obj["data"][2].asDouble()) {
            scaleX = scaleXResult.unwrap() * this->m_drawScale / 4 * 0.16;
        }
        if (auto scaleYResult = obj["data"][3].asDouble()) {
            scaleY = scaleYResult.unwrap() * this->m_drawScale / 4 * 0.16;
        } else {
            scaleY = scaleX;
        }
        if (auto rotationResult = obj["data"][4].asDouble()) {
            rotation = -rotationResult.unwrap();
        }

        // Parsing colors
        float h = 0.f, s = 0.f, v = 0.f;
        if (redResult && blueResult && greenResult) {
            this->rgbToHsv(
                redResult.unwrap() / 255.f,
                blueResult.unwrap() / 255.f,
                greenResult.unwrap() / 255.f,
                h,s,v
            );
        }

        // Parsing objects to gd format and increasing Z Order
        this->m_objsString << fmt::format(
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
        this->m_zOrder++;
    }

    // Checking if there are no parsed objects
    if (this->m_objsString.str().empty()) {
        Notification::create(
            "No objects added.",
            NotificationIcon::Error
        )->show();
        return this->onClose(nullptr);
    }

    // Getting both LevelEditorLayer and EditorUI
    auto activeEditorLayer = LevelEditorLayer::get();
    auto activeEditorUI = activeEditorLayer->m_editorUI;

    // Deleting selected objects
    activeEditorUI->onDeleteSelected(nullptr);

    // Create objects from string and flip Y-axis
    auto objectsArray = activeEditorLayer->createObjectsFromString(this->m_objsString.str().c_str(), true, true);
    activeEditorUI->flipObjectsY(objectsArray);

    // Add to undo stack and select objects
    activeEditorLayer->m_undoObjects->addObject(UndoObject::createWithArray(objectsArray, UndoCommand::Paste));
    activeEditorUI->selectObjects(objectsArray, true);

    // Update UI and notify user
    activeEditorUI->updateButtons();
    Notification::create(
        "Successfully converted to gd objects!",
        NotificationIcon::Success
    )->show();
    this->keyBackClicked();
}

void ImportPopup::rgbToHsv(float fR, float fG, float fB, float& fH, float& fS, float& fV) {
    // This function is took from https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72#file-hsvrgb-cpp-L53
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

// Checks does object count is bigger than 5k. If so, it shows a warning
void ImportPopup::checkAlert(CCObject* sender) {
    if (this->m_objsCount > 5000) {
        geode::createQuickPopup(
            "Alert",
            "This will place more than <cy>5000 objects</c>\nAre you sure?",
            "No", "Yes",
            [this](auto, bool btn2) {
                if (btn2) {
                    this->parseAndPlace();
                }
            }
        );
    } else {
        this->parseAndPlace();
    }
}

// Checks the value inside inputs to avoid unwanted crashes
void ImportPopup::textChanged(CCTextInputNode *p0) {
    if (p0 == this->m_drawScaleInput->getInputNode()) {
        auto num = utils::numFromString<float>(p0->getString());
        if (num.isErr()) {
            return p0->setLabelNormalColor(ccc3(255,0,0));
        }
        auto numUn = num.unwrap();
        if (numUn <= 0) {
            return p0->setLabelNormalColor(ccc3(255,0,0));
        }
        p0->setLabelNormalColor(ccc3(255,255,255));
        this->m_drawScale = numUn;
    } else if (p0 == this->m_zLayerInput->getInputNode()) {
        auto num = utils::numFromString<int>(p0->getString());
        if (num.isErr()) {
            return p0->setLabelNormalColor(ccc3(255,0,0));
        }
        auto numUn = num.unwrap();
        p0->setLabelNormalColor(ccc3(255,255,255));
        this->m_zOrder = numUn;
    }
}


