#include "database.hh"
#include "entity.hh"
#include <iostream>
#include <thread>
#include <SFML/Graphics.hpp>

sf::RenderWindow window(sf::VideoMode(1280, 720), "Game");
sf::Clock main_clock;

ecs::database db;
std::vector<std::thread> workers;

struct transform : public ecs::base_component<transform> {
    float x;
    float y;
    float width;
    float height;
};

struct sprite : public ecs::base_component<sprite> {
    std::shared_ptr<sf::Sprite> sfml_sprite;
};

struct circle : public ecs::base_component<circle> {
    float radius;
    float r;
    float g;
    float b;
};

void render_system() {
    workers.push_back(std::thread([&]() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            window.clear();

            db.iterate_component_list_const<transform>([&](auto tf) {
                db.iterate_component_list_for_entity_const<sprite>(tf->entity, [&](auto sprite) {
                    auto sfml_sprite = sprite->sfml_sprite;

                    sf::Vector2u size_u = sfml_sprite.get()->getTexture()->getSize();
                    sf::Vector2f scale_down = sf::Vector2f(1 / (float)size_u.x, 1 / (float)size_u.y);

                    sfml_sprite.get()->setPosition(tf->x, tf->y);
                    sfml_sprite.get()->setScale(tf->width * scale_down.x, tf->height * scale_down.y);

                    window.draw(*sfml_sprite.get());
                });
                db.iterate_component_list_for_entity_const<circle>(tf->entity, [&](auto circle) {
                    sf::CircleShape shape(circle->radius);
                    shape.setFillColor(sf::Color(circle->r, circle->g, circle->b));
                    window.draw(shape);
                });
            });

            window.display();
        }
    }));
}

void updater_system() {
    workers.push_back(std::thread([&]() {
        for (;;) {
            std::vector<ecs::entity> entities_to_destroy;
            db.iterate_component_list<transform>([&](auto tf) {
                tf->x += 0.1f;
                tf->y += 0.1f;
            });
        }
    }));
}

int main() {
    auto my_entity = db.create_entity();

    db.add_component(my_entity, transform{ .x = 0, .y = 0 });

    // sf::Texture texture;
    // texture.loadFromFile("examples/sfml_example_texture.png");

    // auto sfml_sprite = std::make_shared<sf::Sprite>();
    // sfml_sprite->setTexture(texture);

    db.add_component(my_entity, circle{ .radius = 100, .r = 255, .g = 0, .b = 0 });

    render_system();
    updater_system();

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}