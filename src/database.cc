#include "database.hh"

using db = ecs::database;

db::component_list& db::get_component_list(component_id id) {
    if (component_lists.count(id) == 0) {
        std::lock_guard lock(component_lists_mutex);
        component_lists.insert(std::make_pair(id, component_list{
            .cv_mutex = new std::mutex(), // this memory leak is intentional
            .cv = new std::condition_variable(), // this one too
            .components_mutex = new std::mutex(), // and this one
            .components = std::map<entity, std::vector<std::any>>(),
            .components_count = 0,
            .last_component_mutex = new std::mutex(), // yeah
            .has_last_component = false,
            .last_component = std::any()
        }));
    }
    return component_lists.at(id);
}

std::vector<std::any>& db::get_components_for_entity(component_list& list, entity entity) {
    if (list.components.count(entity) == 0) {
        std::lock_guard lock(*list.components_mutex);
        list.components.insert(std::make_pair(entity, std::vector<std::any>()));
    }
    return list.components.at(entity);
}

ecs::entity db::create_entity() {
    return next_entity++;
}

void db::destroy_entity(entity entity) {
    std::lock_guard lock(component_lists_mutex);
    for (auto& [_, list] : component_lists) {
        std::lock_guard lock(*list.components_mutex);
        if (list.components.count(entity)) {
            list.components.erase(entity);
        }
    }
}