#include "component.hh"

ecs::component_id next_component_id;

ecs::component_id ecs::head_component::next_id() {
    return next_component_id++;
}