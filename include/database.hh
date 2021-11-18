#pragma once
#include "component.hh"
#include <functional>
#include <map>
#include <vector>
#include <any>

namespace ecs {
    class database {
        private:
            entity next_entity;
            std::map<component_id, std::vector<std::any>> components;

            void create_empty_component_list_if_component_list_does_not_exist(component_id id);

            void add_component(component_id id, std::any component);

            std::vector<std::any>& get_component_list(component_id id);
            const std::vector<std::any>& get_component_list(component_id id) const;
        public:
            entity create_entity();

            template<typename C>
            void add_component(entity entity, const C& component) {
                component_ref<C> ref = component;
                ref->entity = entity;
                add_component(C::ID, component);
            }

            template<typename C>
            void iterate_component_list(const std::function<void(component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);
                for (auto& component : list) {
                    func(std::any_cast<component_ref<C>>(component));
                }
            }

            template<typename C>
            void iterate_component_list_const(const std::function<void(const component_ref<C>)>& func) const {
                const auto& list = get_component_list(C::ID);
                for (const auto& component : list) {
                    func(std::any_cast<component_ref<C>>(component));
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