#include <Geode/Bindings.hpp>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <cstdlib>
#include <cmath>

using namespace geode::prelude;

class ImportPopup : public FLAlertLayer, TextInputDelegate, FLAlertLayerProtocol {
    protected:
        const int m_circle_id = 1764;
        const int m_cube_id = 211;
        const std::array<int, 2> m_circle_type = {
            32,
            5
        };
        const std::array<int, 2> m_cube_type = {
            0,
            1
        };
        GameObject* m_center_obj;
        Json::Value m_jsonSets;
        int m_objs_count;
        int m_z_layer_offset;
        std::stringstream m_objs_string;
        float m_draw_scale = 1;
        virtual bool init(cocos2d::CCArray* selected_obj);
        virtual void rgb_to_hsv(float& fR, float& fG, float fB, float& fH, float& fS, float& fV);
        virtual void keyBackClicked() override;
        virtual void importJSON(cocos2d::CCObject* sender);
        virtual void checkAlert(cocos2d::CCObject* sender);
        virtual void convert(cocos2d::CCObject* sender);
        virtual void onExitBtn(cocos2d::CCObject* sender);
    public:
        static ImportPopup* create(cocos2d::CCArray* selected_obj);
};