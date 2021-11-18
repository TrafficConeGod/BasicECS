#pragma once
#include "component.hh"
#include <functional>
#include <map>
#include <vector>
#include <any>
#include <mutex>

namespace ecs {
    class database {
        private:
            entity next_entity;
            std::mutex components_mutex;
            
            struct component_container {
                entity component_entity;
                std::any component;
            };

            struct component_list {
                mutable std::mutex* mutex;
                std::vector<component_container> containers;
            };

            std::map<component_id, component_list> components;

            void create_empty_component_list_if_component_list_does_not_exist(component_id id);

            component_list& get_component_list(component_id id);
            const component_list& get_component_list(component_id id) const;
        public:
            entity create_entity();
            void destroy_entity(entity entity);

            template<typename C>
            void add_component(entity entity, const C& component) {
                component_ref<C> ref = component;
                ref->entity = entity;

                auto& list = get_component_list(C::ID);

                std::lock_guard lock(*list.mutex);
                list.containers.push_back({
                    .component_entity = entity,
                    .component = ref
                });
            }

            template<typename C>
            void remove_component(const component_ref<C> component) {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.mutex);
                for (auto it = containers.begin(); it != containers.end(); it++) {
                    component_ref<C> checkComponent = std::any_cast<component_ref<C>>(*it.component);
                    if (checkComponent == component) {
                        containers.erase(it);
                        return;
                    }
                }
            }

            template<typename C>
            void iterate_component_list(const std::function<void(component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.mutex);
                for (auto container : containers) {
                    func(std::any_cast<component_ref<C>>(container.component));
                }
            }

            template<typename C>
            void iterate_component_list_const(const std::function<void(const component_ref<C>)>& func) const {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.mutex);
                for (const auto container : containers) {
                    func(std::any_cast<const component_ref<C>>(container.component));
                }
            }

            template<typename C>
            void wait_for_component_add(const std::function<void(component_ref<C>)>& func) {
                // unimplemented
            }

            template<typename C>
            void wait_for_component_add(const std::function<void(const component_ref<C>)>& func) const {
                // unimplemented
            }
    };
};