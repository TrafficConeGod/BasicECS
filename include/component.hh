#pragma once
#include "entity.hh"
#include <memory>
#include <mutex>

namespace ecs {
    using component_id = std::size_t;

    struct head_component {
        static component_id next_id();
        ecs::entity entity;
    };

    template<typename R>
    struct base_component : public head_component {
        static const component_id ID;
        static const std::size_t SIZE;
    };

    template<typename R>
    const component_id base_component<R>::ID(head_component::next_id());

    template<typename R>
    const std::size_t base_component<R>::SIZE(sizeof(R));

    template<typename C>
    class component_ref {
        private:
            std::shared_ptr<C> pointer;
        public:
            component_ref(const C& val) : pointer{std::make_shared<C>(val)} {}
            component_ref(std::shared_ptr<C> ptr) : pointer{ptr} {}
            C& operator*() { return *pointer; }
            C* operator->() { return pointer.get(); }
            const C& operator*() const { return *pointer; }
            const C* operator->() const { return pointer.get(); }
            bool operator==(const component_ref<C> rhs) const { return pointer.get() == rhs.pointer.get(); }
    };
}