#pragma once
#include "component.hh"
#include <functional>
#include <map>
#include <vector>
#include <any>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace ecs {
    class database {
        private:
            std::atomic<entity> next_entity;

            struct component_list {
                std::mutex* cv_mutex;
                std::condition_variable* cv;

                std::mutex* components_mutex;
                std::map<entity, std::vector<std::any>> components;
                std::size_t components_count = 0;

                std::mutex* last_component_mutex;
                bool has_last_component = false;
                std::any last_component;
            };

            std::mutex component_lists_mutex;
            std::map<component_id, component_list> component_lists;

            component_list& get_component_list(component_id id);
            std::vector<std::any>& get_components_for_entity(component_list& list, entity entity);
        public:
            entity create_entity();
            void destroy_entity(entity entity);

            template<typename C>
            void add_component(entity entity, const C& original) {
                component_ref<C> component = original;
                component->entity = entity;

                auto& list = get_component_list(C::ID);

                {
                    get_components_for_entity(list, entity).push_back(component);
                    std::lock_guard lock(*list.components_mutex);
                    list.components_count++;
                }

                std::lock_guard lock(*list.last_component_mutex);
                list.last_component = component;
                list.has_last_component = true;
                list.cv->notify_all();
            }

            template<typename C>
            void remove_component(const component_ref<C> component) {
                auto& list = get_component_list(C::ID);

                std::lock_guard lock(*list.components_mutex);
                for (auto& [_, entity_components] : list.components) {
                    for (auto it = entity_components.begin(); it != entity_components.end(); it++) {
                        component_ref<C> check_component = std::any_cast<component_ref<C>>(*it);
                        if (check_component == component) {
                            entity_components.erase(it);
                            return;
                        }
                    }
                }
            }

            template<typename C>
            void iterate_component_list(const std::function<void(component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);

                std::lock_guard lock(*list.components_mutex);
                for (auto& [_, entity_components] : list.components) {
                    for (auto component : entity_components) {
                        func(std::any_cast<component_ref<C>>(component));
                    }
                }
            }

            template<typename C>
            void iterate_component_list_for_entity(entity entity, const std::function<void(const component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);

                auto& entity_components = get_components_for_entity(list, entity);
                std::lock_guard lock(*list.components_mutex);
                for (auto component : entity_components) {
                    func(std::any_cast<component_ref<C>>(component));
                }
            }

            template<typename C>
            std::size_t get_component_list_size() {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.components_mutex);
                return list.components_count;
            }

            template<typename C>
            std::size_t get_component_list_size_for_entity(entity entity) {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                return get_components_for_entity(list, entity).size();
            }

            template<typename C>
            void wait_for_component_add(const std::function<void(component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);

                {
                    std::unique_lock lock(*list.cv_mutex);
                    list.cv->wait(lock, [&]() {
                        return list.has_last_component;
                    });
                }

                list.last_component_mutex->lock();
                list.has_last_component = false;
                auto last_component = list.last_component;
                list.last_component_mutex->unlock();

                func(std::any_cast<component_ref<C>>(last_component));
            }
    };
};