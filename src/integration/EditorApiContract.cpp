// Compile-time compatibility contract for the Geometry Dash 2.2081 editor APIs.
//
// This file deliberately has no runtime behavior. It makes the Android64 build
// fail early if the selected Geode SDK stops exposing an API that GeoDecor's
// preview/apply/undo integration is designed to use. It does not by itself
// prove runtime placement or undo behavior; those still require an in-game test.

#include <Geode/Geode.hpp>

#include <concepts>

namespace {

// Preview can be rendered in the editor's object-layer coordinate space without
// inserting temporary GameObjects into the level's object/undo collections.
static_assert(requires(LevelEditorLayer* editor) {
    static_cast<cocos2d::CCNode*>(editor->m_objectLayer);
}, "LevelEditorLayer must expose its object layer for a read-only preview overlay.");

static_assert(requires(cocos2d::CCDrawNode* draw, cocos2d::CCPoint point,
                       cocos2d::ccColor4F color) {
    draw->drawDot(point, 4.f, color);
}, "CCDrawNode::drawDot must be available for preview placement markers.");

// ObjectToolbox::allKeys returns sprite-frame keys, not integer object IDs.
// The runtime catalog must resolve each frame key to an ID before classifying it.
static_assert(requires(ObjectToolbox* toolbox) {
    { toolbox->allKeys() } -> std::same_as<cocos2d::CCArray*>;
}, "ObjectToolbox::allKeys must be available.");

static_assert(requires(ObjectToolbox* toolbox, char const* frame) {
    { toolbox->frameToKey(frame) } -> std::same_as<char const*>;
}, "ObjectToolbox::frameToKey must map a sprite-frame key to its object ID.");

static_assert(requires(ObjectToolbox* toolbox, int objectID) {
    { toolbox->intKeyToFrame(objectID) } -> std::same_as<char const*>;
}, "ObjectToolbox::intKeyToFrame must map an object ID to its sprite frame.");

static_assert(requires(int objectID) {
    { GameObject::createWithKey(objectID) } -> std::same_as<GameObject*>;
}, "GameObject::createWithKey must be available for catalog classification.");

static_assert(requires(GameObject* object) {
    object->firstSetup();
    object->customSetup();
    { object->getType() } -> std::same_as<GameObjectType>;
    { object->getType() == GameObjectType::Decoration } -> std::same_as<bool>;
    object->m_isTrigger;
    object->m_isColorTrigger;
    object->m_isSpawnOrderTrigger;
    object->m_isStartPos;
    object->m_isDecoration;
}, "GameObject setup/type/flags must be available for gameplay-safe preview filtering.");

// These are the native editor insertion and history APIs. The apply path should
// use the editor's own creation/paste flow, not mutate m_objects or level strings.
static_assert(requires(LevelEditorLayer* editor, int objectID,
                       cocos2d::CCPoint position) {
    { editor->createObject(objectID, position, false) } -> std::same_as<GameObject*>;
}, "LevelEditorLayer::createObject(int, CCPoint, bool) must be available.");

static_assert(requires(LevelEditorLayer* editor, gd::string const& data) {
    { editor->createObjectsFromString(data, false, false) }
        -> std::same_as<cocos2d::CCArray*>;
}, "LevelEditorLayer::createObjectsFromString must be available.");

static_assert(requires(LevelEditorLayer* editor,
                       gd::vector<gd::string>& values,
                       gd::vector<void*>& exists) {
    { editor->addObjectFromVector(values, exists) } -> std::same_as<GameObject*>;
}, "LevelEditorLayer::addObjectFromVector must be available.");

static_assert(requires(LevelEditorLayer* editor, UndoObject* undo) {
    { editor->addToUndoList(undo, false) } -> std::same_as<void>;
    { editor->undoLastAction() } -> std::same_as<void>;
    { editor->redoLastAction() } -> std::same_as<void>;
}, "Native editor undo/redo methods must be available.");

static_assert(requires(EditorUI* ui, gd::string data) {
    { ui->pasteObjects(data, false, false) } -> std::same_as<cocos2d::CCArray*>;
}, "EditorUI::pasteObjects(gd::string, bool, bool) must be available for grouped native undo.");

} // namespace
