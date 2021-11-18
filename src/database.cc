#include "database.hh"

using db = ecs::database;

void db::create_empty_component_list_if_component_list_does_not_exist(component_id id) {
    if (components.count(id) == 0) {
        std::lock_guard lock(components_mutex);
        components.insert(std::make_pair(id, component_list{
            .mutex = new std::mutex(), // this memory leak is intentional,
            .cv = new std::condition_variable(), // this one too
            .containers = std::vector<component_container>()
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

void db::destroy_entity(entity entity) {
    std::lock_guard lock(components_mutex);
    for (auto& [_, list] : components) {
        auto& containers = list.containers;

        auto containers_clone = list.containers;

        std::size_t index = 0;
        std::lock_guard lock(*list.mutex);
        for (auto& container_clone : containers_clone) {
            if (container_clone.component_entity == entity) {
                containers.erase(containers.begin() + index);
                index--;
            }
            index++;
        }
    }
}