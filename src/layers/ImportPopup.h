#include <Geode/Bindings.hpp>
#include <Geode/ui/TextInput.hpp>
#include <matjson.hpp>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <filesystem>

using namespace geode::prelude;

class ImportPopup : public geode::Popup<cocos2d::CCArray*>, TextInputDelegate {
protected:
    int m_zOrder = 0;
    int m_objsCount = 0;
    float m_drawScale = 1;
    matjson::Value m_jsonSets;
    std::ostringstream m_objsString;
    GameObject* m_centerObj = nullptr;
    TextInput* m_zLayerInput = nullptr;
    CCLabelBMFont* m_fileLabel = nullptr;
    CCLabelBMFont* m_countLabel = nullptr;
    TextInput* m_drawScaleInput = nullptr;
    static constexpr int m_CircleId = 497;
    static constexpr int m_SquareId = 495;
    const std::set<int> m_squareTypes = {0, 1, 2};
    const std::set<int> m_validTypes = {5, 3, 4, 32, 8, 16}; // 0, 1, 2
    static constexpr CCSize m_popupSize = CCSize(385.f, 245.f);
    EventListener<Task<Result<std::filesystem::path>>> m_pickListener;
protected:
    void parseAndPlace();
    void importJSON(cocos2d::CCObject* sender);
    void checkAlert(cocos2d::CCObject* sender);
    void textChanged(CCTextInputNode *p0) override;
    bool setup(cocos2d::CCArray* selectedObj) override;
    void rgbToHsv(float fR, float fG, float fB, float& fH, float& fS, float& fV);
public:
    static ImportPopup* create(cocos2d::CCArray* selectedObj);
};