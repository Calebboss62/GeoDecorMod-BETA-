#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

static void openGeoDecorStudio() {
    auto popup = FLAlertLayer::create(
        "GeoDecor Studio",
        "Welcome to GeoDecor Studio!\n\n"
        "Your decoration workspace is being prepared.\n\n"
        "Future tools:\n"
        "- Analyze Reference Levels\n"
        "- Automatic Decoration\n"
        "- Theme Mixer\n"
        "- Decoration Templates\n\n"
        "Your existing level objects are not changed.",
        "OK"
    );

    popup->show();
}

// Main menu button
class $modify(GeoDecorStudioMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        if (auto menu = this->getChildByID("top-right-menu")) {
            auto sprite = ButtonSprite::create("GeoDecor");
            auto button = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(GeoDecorStudioMenuLayer::onGeoDecor)
            );

            button->setID("geodecor-studio-button");
            menu->addChild(button);
            menu->updateLayout();
        }

        log::info("GeoDecor Studio loaded.");
        return true;
    }

    void onGeoDecor(CCObject*) {
        openGeoDecorStudio();
    }
};

// Level editor button
class $modify(GeoDecorStudioEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setID("geodecor-editor-menu");
        menu->setPosition({
            winSize.width - 48.f,
            winSize.height / 2.f
        });

        auto sprite = ButtonSprite::create("GeoDecor");
        sprite->setScale(0.7f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(GeoDecorStudioEditorUI::onGeoDecor)
        );

        menu->addChild(button);
        this->addChild(menu, 1000);

        return true;
    }

    void onGeoDecor(CCObject*) {
        openGeoDecorStudio();
    }
};
