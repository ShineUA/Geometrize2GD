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

class ImportPopup : public geode::Popup<cocos2d::CCArray*> {
    protected:
        const int m_circle_id = 497;
        const int m_rect_id = 211;
        const std::array<int, 3> m_supportedObjsWeb = {
            5, // circle
            // 0, // rectangle
            3, // ellipse
            4//, // rotated ellipse
            //1 // rotated rectangle
        };
        const std::array<int, 3> m_supportedObjsDesktop = {
            32, // same as web
            // 1,
            8,
            16// ,
            // 2
        };
        int m_objsCount;
        GameObject* m_centerObj;
        bool m_isWeb;
        matjson::Value m_jsonSets;
        std::stringstream m_objsString;
        EventListener<Task<Result<std::filesystem::path>>> m_pickListener;
        float m_drawScale = 1;
        virtual bool setup(cocos2d::CCArray* selected_obj) override;
        virtual void rgbToHsv(float& fR, float& fG, float fB, float& fH, float& fS, float& fV);
        virtual void importJSON(cocos2d::CCObject* sender);
        virtual void checkAlert(cocos2d::CCObject* sender);
        virtual void parse();
    public:
        static ImportPopup* create(cocos2d::CCArray* selected_obj);
};