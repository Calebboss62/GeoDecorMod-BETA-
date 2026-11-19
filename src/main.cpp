#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/cocos/touch_dispatcher/CCTouchDelegateProtocol.h>
#include <Geode/cocos/touch_dispatcher/CCTouchDispatcher.h>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

// Opens the GeoDecor Studio popup
static void openGeoDecorStudio() {
    auto popup = FLAlertLayer::create(
        "GeoDecor Studio",
        "Welcome to GeoDecor Studio!\n\n"
        "Your decoration workspace is being prepared.\n\n"
        "Future tools:\n"
        "- Analyze Reference Levels\n"
        "- Automatic Decoration\n"
        "- Theme Mixer\n"
        "- Decoration Templates\n"
        "- Block DNA\n\n"
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

// Draggable editor button
class GeoDecorDraggableButton :
    public CCNode,
    public CCTargetedTouchDelegate {

protected:
    ButtonSprite* m_button = nullptr;

    CCPoint m_touchStart = CCPointZero;
    CCPoint m_startPosition = CCPointZero;

    bool m_dragging = false;
    bool m_moved = false;

    float m_dragThreshold = 12.f;

    // Keeps the button inside the visible screen
    void clampToScreen() {
        auto size = CCDirector::sharedDirector()->getWinSize();

        float halfWidth =
            m_button->getContentSize().width *
            m_button->getScaleX() * 0.5f;

        float halfHeight =
            m_button->getContentSize().height *
            m_button->getScaleY() * 0.5f;

        auto pos = this->getPosition();

        pos.x = std::max(
            halfWidth,
            std::min(size.width - halfWidth, pos.x)
        );

        pos.y = std::max(
            halfHeight,
            std::min(size.height - halfHeight, pos.y)
        );

        this->setPosition(pos);
    }

public:
    static GeoDecorDraggableButton* create() {
        auto ret = new GeoDecorDraggableButton();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() {
        if (!CCNode::init())
            return false;

        m_button = ButtonSprite::create("GeoDecor");

        if (!m_button)
            return false;

        m_button->setScale(0.58f);
        this->addChild(m_button);

        auto mod = Mod::get();
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        float savedX = mod->getSavedValue<float>(
            "editor-button-x",
            -1.f
        );

        float savedY = mod->getSavedValue<float>(
            "editor-button-y",
            -1.f
        );

        if (savedX < 0.f || savedY < 0.f) {
            // Default position near the upper-left editor area.
            savedX = winSize.width * 0.23f;
            savedY = winSize.height * 0.80f;
        }

        this->setPosition({ savedX, savedY });
        clampToScreen();

        return true;
    }

    void onEnter() {
        CCNode::onEnter();

        CCDirector::sharedDirector()
            ->getTouchDispatcher()
            ->addTargetedDelegate(this, -128, true);
    }

    void onExit() {
        CCDirector::sharedDirector()
            ->getTouchDispatcher()
            ->removeDelegate(this);

        auto pos = this->getPosition();

        Mod::get()->setSavedValue<float>(
            "editor-button-x",
            pos.x
        );

        Mod::get()->setSavedValue<float>(
            "editor-button-y",
            pos.y
        );

        CCNode::onExit();
    }

    // Detects touches on the button
    bool ccTouchBegan(CCTouch* touch, CCEvent*) {
        auto buttonLocal = m_button->convertToNodeSpace(
            touch->getLocation()
        );

        auto buttonSize = m_button->getContentSize();
        auto anchor = m_button->getAnchorPoint();

        CCRect buttonRect(
            -anchor.x * buttonSize.width,
            -anchor.y * buttonSize.height,
            buttonSize.width,
            buttonSize.height
        );

        // Corrected hit detection
        if (!buttonRect.containsPoint(buttonLocal))
            return false;

        m_dragging = true;
        m_moved = false;

        auto parent = this->getParent();

        m_touchStart = parent->convertToNodeSpace(
            touch->getLocation()
        );

        m_startPosition = this->getPosition();

        return true;
    }

    // Moves the button while dragging
    void ccTouchMoved(CCTouch* touch, CCEvent*) {
        if (!m_dragging)
            return;

        auto parent = this->getParent();

        auto current = parent->convertToNodeSpace(
            touch->getLocation()
        );

        float dx = current.x - m_touchStart.x;
        float dy = current.y - m_touchStart.y;

        if (std::abs(dx) > m_dragThreshold ||
            std::abs(dy) > m_dragThreshold) {
            m_moved = true;
        }

        if (m_moved) {
            this->setPosition({
                m_startPosition.x + dx,
                m_startPosition.y + dy
            });

            clampToScreen();
        }
    }

    // Opens Studio when tapped, saves position after dragging
    void ccTouchEnded(CCTouch*, CCEvent*) {
        if (!m_dragging)
            return;

        m_dragging = false;

        if (!m_moved) {
            openGeoDecorStudio();
        }

        auto pos = this->getPosition();

        Mod::get()->setSavedValue<float>(
            "editor-button-x",
            pos.x
        );

        Mod::get()->setSavedValue<float>(
            "editor-button-y",
            pos.y
        );
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) {
        m_dragging = false;
    }
};

// Level editor integration
class $modify(GeoDecorStudioEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer))
            return false;

        auto button = GeoDecorDraggableButton::create();

        if (button) {
            button->setID("geodecor-draggable-button");
            this->addChild(button, 1000);
        }

        return true;
    }
};
