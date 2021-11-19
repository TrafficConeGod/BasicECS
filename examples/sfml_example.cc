#include "database.hh"
#include "entity.hh"
#include <iostream>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

sf::RenderWindow window(sf::VideoMode(1280, 720), "Game");

ecs::database db;
std::vector<std::thread> workers;

struct transform : public ecs::base_component<transform> {
    float x;
    float y;
    float width;
    float height;
};

struct sprite : public ecs::base_component<sprite> {
    sf::Sprite sfml_sprite;
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

            db.iterate_component_list<transform>([&](const auto tf) {
                db.iterate_component_list_for_entity<sprite>(tf->entity, [&](auto sp) {
                    auto& sfml_sprite = sp->sfml_sprite;

                    sf::Vector2u size_u = sfml_sprite.getTexture()->getSize();
                    sf::Vector2f scale_down = sf::Vector2f(1 / (float)size_u.x, 1 / (float)size_u.y);

                    sfml_sprite.setPosition(tf->x, tf->y);
                    sfml_sprite.setScale(tf->width * scale_down.x, tf->height * scale_down.y);

                    window.draw(sfml_sprite);
                });
            });

            window.display();
        }
    }));
}

int main() {
    auto my_entity = db.create_entity();

    db.add_component(my_entity, transform{ .x = 100, .y = 100, .width = 323.60679775, .height = 200 });

    sf::Texture texture;
    texture.loadFromFile("examples/sfml_example_texture.png");

    sf::Sprite sfml_sprite;
    sfml_sprite.setTexture(texture);

    db.add_component(my_entity, sprite{ .sfml_sprite = sfml_sprite });

    render_system();

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}