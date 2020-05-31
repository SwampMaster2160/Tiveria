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

int WinMain()
{
    //time_ = 0;

    player = NPC(0x0000, {0, 0});

    //playerPos = { 0x00, 0x00 };
    //playerWalkOffset = { 0x00, 0x00 };

    for (uint16_t x = 0; x < 256; x++)
    {
        for (uint16_t y = 0; y < 256; y++)
        {
            map[x][y].image = rand() % 2;//rand() % 2
            if (x == y || x == 0 || y == 0)
            {
                map[x][y].image = 2;
            }
        }
    }

    //screenSize = sf::VideoMode::GetDesktopMode();
    sf::RenderWindow window(sf::VideoMode(512, 256), "RPG", sf::Style::Fullscreen);//sf::Style::Fullscreen
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    screenRes = sf::Vector2<uint16_t>(window.getSize());

    screenAspectRatio = (double)screenRes.x / (double)screenRes.y;
    screenOffset = (screenRes.x - (screenRes.y * 2)) / 2;
    //screenOffset = -100;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            //if (event.type == sf::Event::Closed)
            //    window.close();
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

        rectangle.setSize(sf::Vector2f(screenRes.y / 16, screenRes.y / 16));
        for (uint8_t x = 0; x < 32; x++)
        {
            for (uint8_t y = 0; y < 16; y++)
            {
                rectangle.setPosition(x * (screenRes.y / 16) + screenOffset, y * (screenRes.y / 16));
                rectangle.setFillColor(sf::Color::Green);
                if (map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image > 0) rectangle.setFillColor(sf::Color::Blue);
                if (map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image == 2) rectangle.setFillColor(sf::Color::Black);
                if (x == 16 && y == 8) rectangle.setFillColor(sf::Color::White);
                window.draw(rectangle);
            }
        }

        window.display();

        time_++;
    }

    return 0;
}