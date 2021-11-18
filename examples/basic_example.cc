#include "database.hh"
#include "system.hh"
#include <iostream>

struct transform : public ecs::base_component<transform> {
    float x;
    float y;
};

void render_system_init(ecs::system system) {
    system.add_thread([](ecs::system system) {
        for (;;) {
            system.iterate_components_const<transform>([](auto tf) {
                std::cout << tf->x << " " << tf->y << "\n";
            });
        }
    });
    
}

int main() {
    ecs::database db;
    render_system_init(db.create_system());

    return 0;
}