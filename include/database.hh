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
            
            struct component_container {
                entity component_entity;
                std::any component;
            };

            struct component_list {
                mutable std::mutex* cv_mutex;
                mutable std::condition_variable* cv;

                mutable std::mutex* containers_mutex;
                std::vector<component_container> containers;

                mutable std::mutex* last_component_mutex;
                mutable bool has_last_component = false;
                std::any last_component;
            };

            mutable std::mutex components_mutex;
            mutable std::map<component_id, component_list> components;

            void create_empty_component_list_if_component_list_does_not_exist(component_id id) const;

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

                {
                    std::lock_guard lock(*list.containers_mutex);
                    list.containers.push_back({
                        .component_entity = entity,
                        .component = ref
                    });
                }

                std::lock_guard lock(*list.last_component_mutex);
                list.last_component = ref;
                list.has_last_component = true;
                list.cv->notify_all();
            }

            template<typename C>
            void remove_component(const component_ref<C> component) {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.containers_mutex);
                for (auto it = containers.begin(); it != containers.end(); it++) {
                    component_ref<C> checkComponent = std::any_cast<component_ref<C>>(it->component);
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

                std::lock_guard lock(*list.containers_mutex);
                for (auto container : containers) {
                    func(std::any_cast<component_ref<C>>(container.component));
                }
            }

            template<typename C>
            void iterate_component_list_const(const std::function<void(const component_ref<C>)>& func) const {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.containers_mutex);
                for (auto container : containers) {
                    func(std::any_cast<const component_ref<C>>(container.component));
                }
            }

            template<typename C>
            void iterate_component_list_for_entity(entity entity, const std::function<void(const component_ref<C>)>& func) {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.containers_mutex);
                for (auto container : containers) {
                    if (container.component_entity == entity) {
                        func(std::any_cast<component_ref<C>>(container.component));
                    }
                }
            }

            template<typename C>
            void iterate_component_list_for_entity_const(entity entity, const std::function<void(const component_ref<C>)>& func) const {
                auto& list = get_component_list(C::ID);
                auto& containers = list.containers;

                std::lock_guard lock(*list.containers_mutex);
                for (auto container : containers) {
                    if (container.component_entity == entity) {
                        func(std::any_cast<const component_ref<C>>(container.component));
                    }
                }
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

            template<typename C>
            void wait_for_component_add_const(const std::function<void(const component_ref<C>)>& func) const {
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

                func(std::any_cast<const component_ref<C>>(last_component));
            }
    };
};