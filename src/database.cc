#include "database.hh"

using db = ecs::database;

void db::create_empty_component_list_if_component_list_does_not_exist(component_id id) {
    if (components.count(id) == 0) {
        std::lock_guard lock(components_mutex);
        components.insert(std::make_pair(id, std::vector<std::any>()));
    }
}

std::vector<std::any>& db::get_component_list(component_id id) {
    create_empty_component_list_if_component_list_does_not_exist(id);
    return components.at(id);
}

const std::vector<std::any>& db::get_component_list(component_id id) const {
    return components.at(id);
}

ecs::entity db::create_entity() {
    return next_entity++;
}