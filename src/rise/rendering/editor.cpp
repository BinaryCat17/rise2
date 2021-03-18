#include "editor.hpp"
#include <rise/editor/gui.hpp>
#include "module.hpp"

namespace rise::rendering {
    using namespace editor;

    EditorComponents::EditorComponents(flecs::world &ecs) {
        ecs.module<EditorComponents>("rise::rendering::editor");
        editor::regGuiComponent<Position2D>(ecs, GuiComponentType::DragFloat2);
        editor::regGuiComponent<Rotation2D>(ecs, GuiComponentType::DragFloat2);
        editor::regGuiComponent<Scale2D>(ecs, GuiComponentType::DragFloat2);
        editor::regGuiComponent<Extent2D>(ecs, GuiComponentType::DragFloat2);
        editor::regGuiComponent<Position3D>(ecs, GuiComponentType::DragFloat3);
        editor::regGuiComponent<Rotation3D>(ecs, GuiComponentType::DragFloat3);
        editor::regGuiComponent<Scale3D>(ecs, GuiComponentType::DragFloat3);
        editor::regGuiComponent<Extent3D>(ecs, GuiComponentType::DragFloat3);
        editor::regGuiComponent<DiffuseColor>(ecs, GuiComponentType::DragFloat3);
        editor::regGuiComponent<Intensity>(ecs, GuiComponentType::DragFloat);
        editor::regGuiComponent<Distance>(ecs, GuiComponentType::DragFloat);
        editor::regGuiComponent<Path>(ecs, GuiComponentType::InputTextStdString);
    }
}
