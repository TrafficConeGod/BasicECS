#pragma once
#include <functional>
#include "component.hh"

namespace ecs {
    class system {
        public:
            void add_thread(const std::function<void(ecs::system)>& thread_func);

            template<typename C>
            void iterate_components(const std::function<void(component_ref<C>)>& func);

            template<typename C>
            void iterate_components_const(const std::function<void(const component_ref<C>)>& func) const;

            template<typename C>
            void wait_for_component_add(component_ref<C> component);

            template<typename C>
            void wait_for_component_add(const component_ref<C> component) const;
    };
}