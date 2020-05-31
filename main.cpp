#include <SFML/Graphics.hpp>

sf::RectangleShape rectangle;

class MapTile
{
public:
    uint16_t image;
};

class NPC
{
public:
    sf::Vector2<uint8_t> pos = { 0x00, 0x00 };
    sf::Vector2<uint8_t> walkOffset = { 0x00, 0x00 };
    uint8_t direction;
    uint16_t image;

    NPC()
    {
        image = 0x0000;
        pos = { 0x00, 0x00 };
        walkOffset = { 0x00, 0x00 };
        direction = 0x00;
    }

    NPC(uint16_t imageIn, sf::Vector2<uint8_t> posIn)
    {
        image = imageIn;
        pos = posIn;
        walkOffset = { 0x00, 0x00 };
        direction = 0x00;
    }
};

sf::Vector2<uint16_t> screenRes;
double screenAspectRatio;
int16_t screenOffset;

uint64_t time_ = 0;

NPC player;
MapTile map[0x100][0x100];

//sf::Texture grass;
//sf::Texture water;
sf::Texture textures;

sf::Sprite sprite;

int WinMain()
{
    player = NPC(0x0000, {0, 0});

    for (uint16_t x = 0; x < 256; x++)
    {
        for (uint16_t y = 0; y < 256; y++)
        {
            map[x][y].image = rand() % 2 + 1;//rand() % 2
            if (x == y || x == 0 || y == 0)
            {
                map[x][y].image = 0;
            }
        }
    }

    sf::RenderWindow window(sf::VideoMode(512, 256), "RPG", sf::Style::Fullscreen);//sf::Style::Fullscreen
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    screenRes = sf::Vector2<uint16_t>(window.getSize());

    screenAspectRatio = (double)screenRes.x / (double)screenRes.y;
    screenOffset = (screenRes.x - (screenRes.y * 2)) / 2;

    //grass.loadFromFile("assets/textures/grass.png");
    //water.loadFromFile("assets/textures/water.png");
    textures.loadFromFile("assets/textures/textures.png");

    sprite.setTexture(textures);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::W:
                    player.pos.y--;
                    break;
                case sf::Keyboard::A:
                    player.pos.x--;
                    break;
                case sf::Keyboard::S:
                    player.pos.y++;
                    break;
                case sf::Keyboard::D:
                    player.pos.x++;
                    break;
                }
                break;
            }
        }

        window.clear();

        rectangle.setPosition(0, 0);
        rectangle.setSize((sf::Vector2f)screenRes);
        rectangle.setFillColor(sf::Color::Black);
        window.draw(rectangle);

        //rectangle.setSize(sf::Vector2f(screenRes.y / 16, screenRes.y / 16));

        sprite.setScale((double)screenRes.y / 256, (double)screenRes.y / 256);
        for (uint8_t x = 0; x < 32; x++)
        {
            for (uint8_t y = 0; y < 16; y++)
            {
                uint16_t image = map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image;

                sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));

                sprite.setPosition(x * (screenRes.y / 16) + screenOffset, y * (screenRes.y / 16));

                window.draw(sprite);
            }
        }

        window.display();

        time_++;
    }

    return 0;
}