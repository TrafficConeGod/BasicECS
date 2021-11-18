#include "database.hh"
#include <iostream>
#include <thread>
#include <mutex>

ecs::database db;
std::vector<std::thread> workers;

std::mutex transform_mutex;
struct transform : public ecs::base_component<transform> {
    float x;
    float y;
};

std::mutex collision_mutex;
struct collision : public ecs::base_component<collision> {
    ecs::entity collides_with;
};

void render_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            std::lock_guard lock(transform_mutex);
            db.iterate_component_list_const<transform>([](auto tf) {
                std::cout << tf->x << " " << tf->y << "\n";
            });
        }
    }));
}

void updater_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            std::lock_guard lock(transform_mutex);
            db.iterate_component_list<transform>([](auto tf) {
                tf->x += 0.1f;
                tf->y += 0.1f;
            });
        }
    }));
}

void collision_system() {
    workers.push_back(std::thread([&]() {
        db.wait_for_component_add<collision>([](auto col) {
            std::lock_guard lock(collision_mutex);
            std::cout << col->entity << " collided with " << col->collides_with << "\n";
        });
    }));
}

int main() {
    auto my_entity = db.create_entity();
    db.add_component(my_entity, transform{ .x = 0, .y = 0 });

    render_system();
    updater_system();
    collision_system();

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}