#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/cocos/touch_dispatcher/CCTouchDelegateProtocol.h>
#include <Geode/cocos/touch_dispatcher/CCTouchDispatcher.h>
#include "core/LevelString.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

// ============================================================
// GeoDecor Studio Workspace
// ============================================================

class GeoDecorWorkspace : public geode::Popup {
protected:
    int m_blockSet = 0;
    int m_style = 0;
    int m_intensity = 1;
    bool m_glow = true;

    CCLabelBMFont* m_status = nullptr;
    LevelEditorLayer* m_editorLayer = nullptr;

    static constexpr const char* BLOCK_SETS[] = {
        "All Blocks",
        "Solid Blocks",
        "Glow Blocks",
        "Slopes",
        "Details"
    };

    static constexpr const char* STYLES[] = {
        "Modern",
        "Glow",
        "Tech",
        "Simple",
        "Effect Heavy"
    };

    static constexpr const char* INTENSITIES[] = {
        "Light",
        "Medium",
        "Heavy"
    };

    void updateStatus() {
        if (!m_status)
            return;

        auto text = fmt::format(
            "Blocks: {}   |   Glow: {}\nStyle: {}   |   Intensity: {}",
            BLOCK_SETS[m_blockSet],
            m_glow ? "ON" : "OFF",
            STYLES[m_style],
            INTENSITIES[m_intensity]
        );

        m_status->setString(text.c_str());
    }

    CCMenuItemSpriteExtra* makeButton(
        const char* title,
        SEL_MenuHandler callback
    ) {
        auto sprite = ButtonSprite::create(title);
        sprite->setScale(0.72f);

        return CCMenuItemSpriteExtra::create(
            sprite,
            this,
            callback
        );
    }

    bool init(LevelEditorLayer* editorLayer) {
        m_editorLayer = editorLayer;

        if (!geode::Popup::init(360.f, 390.f))
            return false;

        this->setTitle("GeoDecor Studio");

        auto size = m_mainLayer->getContentSize();

        auto centerX = size.width / 2.f;
        auto centerY = size.height / 2.f;

        // Settings display.
        m_status = CCLabelBMFont::create(
            "",
            "bigFont.fnt"
        );

        m_status->setScale(0.38f);
        m_status->setAlignment(kCCTextAlignmentCenter);

        m_status->setPosition({
            centerX,
            centerY + 125.f
        });

        m_mainLayer->addChild(m_status);

        // Button menu.
        auto menu = CCMenu::create();

        menu->setPosition({
            centerX,
            centerY - 15.f
        });

        m_mainLayer->addChild(menu);

        auto blocks = makeButton(
            "Block Set: Next",
            menu_selector(GeoDecorWorkspace::onNextBlocks)
        );

        blocks->setPosition({0.f, 105.f});
        menu->addChild(blocks);

        auto glow = makeButton(
            "Toggle Glow",
            menu_selector(GeoDecorWorkspace::onToggleGlow)
        );

        glow->setPosition({0.f, 67.f});
        menu->addChild(glow);

        auto style = makeButton(
            "Style: Next",
            menu_selector(GeoDecorWorkspace::onNextStyle)
        );

        style->setPosition({0.f, 29.f});
        menu->addChild(style);

        auto intensity = makeButton(
            "Intensity: Next",
            menu_selector(GeoDecorWorkspace::onNextIntensity)
        );

        intensity->setPosition({0.f, -9.f});
        menu->addChild(intensity);

        auto preview = makeButton(
            "Preview Settings",
            menu_selector(GeoDecorWorkspace::onPreview)
        );

        preview->setPosition({0.f, -47.f});
        menu->addChild(preview);

        auto planner = makeButton(
            "Decoration Planner",
            menu_selector(GeoDecorWorkspace::onPlannerStatus)
        );

        planner->setPosition({0.f, -85.f});
        menu->addChild(planner);

        // NEW: Live level scanner.
        auto scan = makeButton(
            "Scan Current Level",
            menu_selector(GeoDecorWorkspace::onScanLevel)
        );

        scan->setPosition({0.f, -123.f});
        menu->addChild(scan);

        updateStatus();

        return true;
    }

    void onNextBlocks(CCObject*) {
        m_blockSet = (m_blockSet + 1) % 5;
        updateStatus();
    }

    void onToggleGlow(CCObject*) {
        m_glow = !m_glow;
        updateStatus();
    }

    void onNextStyle(CCObject*) {
        m_style = (m_style + 1) % 5;
        updateStatus();
    }

    void onNextIntensity(CCObject*) {
        m_intensity = (m_intensity + 1) % 3;
        updateStatus();
    }

    void onPreview(CCObject*) {
        auto summary = fmt::format(
            "Current decoration setup:\n\n"
            "Block set: {}\n"
            "Glow: {}\n"
            "Style: {}\n"
            "Intensity: {}\n\n"
            "This is a settings preview only. "
            "No level objects have been changed.",
            BLOCK_SETS[m_blockSet],
            m_glow ? "On" : "Off",
            STYLES[m_style],
            INTENSITIES[m_intensity]
        );

        geode::createQuickPopup(
            "Decoration Preview",
            summary,
            "Back",
            nullptr,
            nullptr
        );
    }

    void onPlannerStatus(CCObject*) {
        auto summary =
            "DECORATION PLANNER\n\n"
            "Planner foundation: READY\n\n"
            "Block DNA matching: IMPLEMENTED\n"
            "Reference matching: IMPLEMENTED\n"
            "Live editor scanning: READY\n"
            "Decoration placement: NOT ACTIVE YET\n\n"
            "The scanner can inspect actual objects "
            "in your open Geometry Dash level.\n\n"
            "Scanning does not modify your level.";

        geode::createQuickPopup(
            "Decoration Planner",
            summary,
            "Back",
            nullptr,
            nullptr
        );
    }

    // ========================================================
    // LIVE LEVEL SCANNER
    // ========================================================

    void onScanLevel(CCObject*) {
        if (!m_editorLayer) {
            geode::createQuickPopup(
                "No Open Level",
                "Open GeoDecor Studio from inside "
                "the Geometry Dash level editor "
                "to scan a level.",
                "Back",
                nullptr,
                nullptr
            );

            return;
        }

        auto objects = m_editorLayer->getAllObjects();

        if (!objects) {
            geode::createQuickPopup(
                "Scan Failed",
                "Geometry Dash did not provide "
                "the current level object list.",
                "Back",
                nullptr,
                nullptr
            );

            return;
        }

        std::map<int, std::size_t> objectCounts;

geodecor::LevelDocument liveLevel;

std::size_t totalObjects = 0;
std::size_t invalidObjects = 0;

        // Read every object in the editor.
        for (unsigned int i = 0; i < objects->count(); i++) {
            auto object = static_cast<GameObject*>(
                objects->objectAtIndex(i)
            );

            if (!object) {
                continue;
            }

            int objectId = object->m_objectID;

            if (objectId <= 0) {
                invalidObjects++;
                continue;
            }

            objectCounts[objectId]++;
            totalObjects++;
        }

        // Convert the inventory into a sortable list.
        std::vector<std::pair<int, std::size_t>> ranked;

        for (const auto& entry : objectCounts) {
            ranked.push_back(entry);
        }

        std::sort(
            ranked.begin(),
            ranked.end(),
            [](const auto& a, const auto& b) {
                if (a.second != b.second)
                    return a.second > b.second;

                return a.first < b.first;
            }
        );

        // Build the report.
        std::string report = fmt::format(
            "LEVEL SCAN COMPLETE!\n\n"
            "Total recognized objects: {}\n"
            "Unique object IDs: {}\n"
            "Skipped invalid objects: {}\n\n"
            "MOST COMMON OBJECTS\n",
            totalObjects,
            objectCounts.size(),
            invalidObjects
        );

        if (ranked.empty()) {
            report += "\nNo valid object IDs were found.";
        }
        else {
            const std::size_t displayCount =
                std::min<std::size_t>(ranked.size(), 10);

            for (std::size_t i = 0; i < displayCount; i++) {
                report += fmt::format(
                    "\nID {}  -  {} objects",
                    ranked[i].first,
                    ranked[i].second
                );
            }

            if (ranked.size() > displayCount) {
                report += fmt::format(
                    "\n\nAnd {} more object IDs.",
                    ranked.size() - displayCount
                );
            }
        }

        report +=
            "\n\nScan mode: READ ONLY"
            "\nYour level was not modified.";

        geode::createQuickPopup(
            "GeoDecor Level Scanner",
            report,
            "Back",
            nullptr,
            nullptr
        );

        log::info(
            "GeoDecor scanned {} objects across {} unique IDs.",
            totalObjects,
            objectCounts.size()
        );
    }

public:
    static GeoDecorWorkspace* create(
        LevelEditorLayer* editorLayer
    ) {
        auto ret = new GeoDecorWorkspace();

        if (ret && ret->init(editorLayer)) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============================================================
// OPEN WORKSPACE
// ============================================================

static void openGeoDecorStudio(
    LevelEditorLayer* editorLayer = nullptr
) {
    if (auto popup = GeoDecorWorkspace::create(editorLayer)) {
        popup->show();
    }
}

// ============================================================
// MAIN MENU BUTTON
// ============================================================

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

// ============================================================
// DRAGGABLE EDITOR BUTTON
// ============================================================

class GeoDecorDraggableButton :
    public CCNode,
    public CCTargetedTouchDelegate {

protected:
    ButtonSprite* m_button = nullptr;

    LevelEditorLayer* m_editorLayer = nullptr;

    CCPoint m_touchStart = CCPointZero;
    CCPoint m_startPosition = CCPointZero;

    bool m_dragging = false;
    bool m_moved = false;

    float m_dragThreshold = 12.f;

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
    static GeoDecorDraggableButton* create(
        LevelEditorLayer* editorLayer
    ) {
        auto ret = new GeoDecorDraggableButton();

        if (ret && ret->init(editorLayer)) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(LevelEditorLayer* editorLayer) {
        if (!CCNode::init())
            return false;

        m_editorLayer = editorLayer;

        m_button = ButtonSprite::create("GeoDecor");

        if (!m_button)
            return false;

        m_button->setScale(0.58f);

        this->addChild(m_button);

        auto mod = Mod::get();

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        float savedX =
            mod->getSavedValue<float>(
                "editor-button-x",
                -1.f
            );

        float savedY =
            mod->getSavedValue<float>(
                "editor-button-y",
                -1.f
            );

        if (savedX < 0.f || savedY < 0.f) {
            savedX = winSize.width * 0.23f;
            savedY = winSize.height * 0.80f;
        }

        this->setPosition({savedX, savedY});

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

    bool ccTouchBegan(CCTouch* touch, CCEvent*) {
        auto buttonLocal =
            m_button->convertToNodeSpace(
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

        if (!buttonRect.containsPoint(buttonLocal))
            return false;

        m_dragging = true;
        m_moved = false;

        auto parent = this->getParent();

        m_touchStart =
            parent->convertToNodeSpace(
                touch->getLocation()
            );

        m_startPosition = this->getPosition();

        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) {
        if (!m_dragging)
            return;

        auto current =
            this->getParent()->convertToNodeSpace(
                touch->getLocation()
            );

        float dx = current.x - m_touchStart.x;
        float dy = current.y - m_touchStart.y;

        if (
            std::abs(dx) > m_dragThreshold ||
            std::abs(dy) > m_dragThreshold
        ) {
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

    void ccTouchEnded(CCTouch*, CCEvent*) {
        if (!m_dragging)
            return;

        m_dragging = false;

        if (!m_moved) {
            openGeoDecorStudio(m_editorLayer);
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

// ============================================================
// ADD BUTTON TO EDITOR
// ============================================================

class $modify(GeoDecorStudioEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer))
            return false;

        auto button =
            GeoDecorDraggableButton::create(editorLayer);

        if (button) {
            button->setID("geodecor-draggable-button");
            this->addChild(button, 1000);
        }

        return true;
    }
};
