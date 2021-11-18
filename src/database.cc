#include "database.hh"

using db = ecs::database;

void db::create_empty_component_list_if_component_list_does_not_exist(component_id id) {
    if (components.count(id) == 0) {
        std::lock_guard lock(components_mutex);
        components.insert(std::make_pair(id, component_list{
            new std::mutex(),
            std::vector<component_container>()
        }));
    }
}

db::component_list& db::get_component_list(component_id id) {
    create_empty_component_list_if_component_list_does_not_exist(id);
    return components.at(id);
}

const db::component_list& db::get_component_list(component_id id) const {
    return components.at(id);
}

ecs::entity db::create_entity() {
    return next_entity++;
}