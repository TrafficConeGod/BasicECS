#include "database.hh"

using db = ecs::database;

void db::create_empty_component_list_if_component_list_does_not_exist(component_id id) {
    if (components.count(id) == 0) {
        components.insert(std::make_pair(id, std::vector<std::any>()));
    }
}

void db::add_component_raw(component_id id, std::any component) {
    create_empty_component_list_if_component_list_does_not_exist(id);
    components.at(id).push_back(component);
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