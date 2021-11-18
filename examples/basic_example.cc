#include "database.hh"
#include <iostream>
#include <thread>

ecs::database db;
std::vector<std::thread> workers;

struct transform : public ecs::base_component<transform> {
    float x;
    float y;
};

struct collision : public ecs::base_component<collision> {
    ecs::entity collides_with;
};

void render_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            db.iterate_component_list_const<transform>([](auto tf) {
                std::cout << tf->x << " " << tf->y << "\n";
            });
        }
    }));
}

void collision_system() {
    workers.push_back(std::thread([&]() {
        db.wait_for_component_add<collision>([](auto col) {
            std::cout << col->entity << " collided with " << col->collides_with << "\n";
        });
    }));
}

int main() {
    render_system();
    collision_system();
    
    auto my_entity = db.create_entity();
    db.add_component(my_entity, transform{0, 0});

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}