#include "database.hh"
#include "entity.hh"
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
            db.iterate_component_list<transform>([](const auto tf) {
                // std::cout << tf->x << " " << tf->y << "\n";
            });
        }
    }));
}

void updater_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            std::vector<ecs::entity> entities_to_destroy;
            db.iterate_component_list<transform>([&](auto tf) {
                if (tf->x >= 5.f) {
                    entities_to_destroy.push_back(tf->entity);
                } else {
                    tf->x += 1.f;
                    tf->y += 1.f;
                    db.add_component(tf->entity, collision{ .collides_with = tf->entity });
                }
            });
            for (auto entity : entities_to_destroy) {
                db.destroy_entity(entity);
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }));
}

void collision_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            db.wait_for_component_add<collision>([&](const auto col) {
                db.remove_component(col);
                std::cout << col->entity << " collided with " << col->collides_with << "\n";
            });
        }
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