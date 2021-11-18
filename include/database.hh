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
            std::map<component_id, std::vector<std::any>> components;

            void create_empty_component_list_if_component_list_does_not_exist(component_id id);

            std::vector<std::any>& get_component_list(component_id id);
            const std::vector<std::any>& get_component_list(component_id id) const;
        public:
            entity create_entity();

            template<typename C>
            void add_component(entity entity, const C& component) {
                component_ref<C> ref = component;
                ref->entity = entity;

                auto& list = get_component_list(C::ID);

                std::lock_guard lock(C::MUTEX);
                list.push_back(ref);
            }

            template<typename C>
            void remove_component(const component_ref<C> component) {
                auto& list = get_component_list(C::ID);

                std::lock_guard lock(C::MUTEX);
                for (auto it = list.begin(); it != list.end(); it++) {
                    component_ref<C> checkComponent = std::any_cast<component_ref<C>>(*it);
                    if (checkComponent == component) {
                        list.erase(it);
                        return;
                    }
                }
            }

            template<typename C>
            void iterate_component_list(const std::function<void(component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);

                std::lock_guard lock(C::MUTEX);
                for (auto& component : list) {
                    func(std::any_cast<component_ref<C>>(component));
                }
            }

            template<typename C>
            void iterate_component_list_const(const std::function<void(const component_ref<C>)>& func) const {
                const auto& list = get_component_list(C::ID);

                std::lock_guard lock(C::MUTEX);
                for (const auto& component : list) {
                    func(std::any_cast<const component_ref<C>>(component));
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