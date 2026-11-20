if (auto menu = this->getChildByID("top-right-menu")) {
            auto sprite = ButtonSprite::create("GeoDecor");
            auto button = CCMenuItemSpriteExtra::create(
                sprite, this, menu_selector(GeoDecorStudioMenuLayer::onGeoDecor)
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

class GeoDecorDraggableButton : public CCNode, public CCTargetedTouchDelegate {
protected:
    ButtonSprite* m_button = nullptr;
    CCPoint m_touchStart = CCPointZero;
    CCPoint m_startPosition = CCPointZero;
    bool m_dragging = false;
    bool m_moved = false;
    float m_dragThreshold = 12.f;

    void clampToScreen() {
        auto size = CCDirector::sharedDirector()->getWinSize();
        float halfWidth = m_button->getContentSize().width * m_button->getScaleX() * 0.5f;
        float halfHeight = m_button->getContentSize().height * m_button->getScaleY() * 0.5f;
        auto pos = this->getPosition();
        pos.x = std::max(halfWidth, std::min(size.width - halfWidth, pos.x));
        pos.y = std::max(halfHeight, std::min(size.height - halfHeight, pos.y));
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
        if (!CCNode::init()) return false;
        m_button = ButtonSprite::create("GeoDecor");
        if (!m_button) return false;
        m_button->setScale(0.58f);
        this->addChild(m_button);

        auto mod = Mod::get();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float savedX = mod->getSavedValue<float>("editor-button-x", -1.f);
        float savedY = mod->getSavedValue<float>("editor-button-y", -1.f);

        if (savedX < 0.f || savedY < 0.f) {
            savedX = winSize.width * 0.23f;
            savedY = winSize.height * 0.80f;
        }
