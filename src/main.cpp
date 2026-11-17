#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(GeoDecorStudioMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        log::info("GeoDecor Studio v0.2.0 loaded — read-only theme analysis core ready");
        return true;
    }
};
