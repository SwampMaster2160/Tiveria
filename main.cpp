#include <SFML/Graphics.hpp>

sf::RectangleShape rectangle;

enum walkPerm: uint8_t
{
    noWalk = 0, walk = 1
};

/*union walkPerm
{
    enum: uint8_t
    {
        noWalk = 0, walk = 1
    };
};*/

enum GameMode : uint8_t
{
    inGame, walking
};

class MapTile
{
public:
    uint16_t image;
    walkPerm walk;
};

enum Events: uint8_t
{
    null = 0, warp = 1
};

class Warp
{
public:
    sf::Vector2<uint8_t> pos;
    uint16_t newMap;
    sf::Vector2<uint8_t> newPos;

    Warp()
    {
        pos = { 0x00, 0x00 };
        newMap = 0;
        newPos = { 0x00, 0x00 };
    }

    Warp(sf::Vector2<uint8_t> posIn, uint16_t newMapIn, sf::Vector2<uint8_t> newPosIn)
    {
        pos = posIn;
        newMap = newMapIn;
        newPos = newPosIn;
    }
};

std::vector<Warp> warps;

class NPC
{
public:
    sf::Vector2<uint8_t> pos = { 0x00, 0x00 };
    sf::Vector2<int8_t> walkOffset = { 0x00, 0x00 };
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

uint8_t keyPressed;
bool isKeyPressed;

GameMode currentGamemode;

void loadMap(uint16_t mapIn)
{
    sf::FileInputStream file;

    uint8_t buffer8[0x10000];
    std::string filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mhd");
    file.open(filePath);
    file.read(buffer8, file.getSize());
    uint8_t width = buffer8[0];
    uint8_t height = buffer8[1];
    uint16_t backgroundImage = buffer8[2] + buffer8[3] * 256;

    uint16_t buffer16[0x10000];
    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mti");
    file.open(filePath);
    file.read(buffer16, file.getSize());

    for (uint16_t x = 0; x < 256; x++)
    {
        for (uint16_t y = 0; y < 256; y++)
        {
            map[x][y].image = backgroundImage;
            map[x][y].walk = noWalk;
        }
    }

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].image = buffer16[x + width * y];
            map[x][y].walk = walk;
        }
    }

    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mmv");
    file.open(filePath);
    file.read(buffer8, file.getSize());

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].walk = (walkPerm)buffer8[x + width * y];
            //map[x][y].walk = walk;
        }
    }

    /*warps = std::vector<Warp>(2);
    warps[0] = Warp({ 1, 0 }, 1, {3, 4});
    warps[1] = Warp({ 1, 1 }, 1, { 4, 5 });
    warps.shrink_to_fit();*/

    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mev");
    file.open(filePath);
    file.read(buffer8, file.getSize());

    uint8_t eventCount = file.getSize() / 8;

    warps = std::vector<Warp>(eventCount);

    for (uint8_t x = 0; x < eventCount; x++)
    {
        warps[x] = Warp({ buffer8[x * 8], buffer8[x * 8 + 1] }, buffer8[x * 8 + 4] + buffer8[x * 8 + 5] * 256, { buffer8[x * 8 + 6], buffer8[x * 8 + 7] });
        //warps[x] = buffer8[x * 8];
    }
}

void warpPlayer(uint16_t map, sf::Vector2<uint8_t> pos)
{
    loadMap(map);
    player.pos = pos;
}

void finishWalk()
{
    player.walkOffset.x = 0;
    player.walkOffset.y = 0;
    currentGamemode = inGame;

    for (uint8_t x = 0; x < warps.size(); x++)
    {
        if (warps[x].pos == player.pos)
        {
            warpPlayer(warps[x].newMap, warps[x].newPos);
        }
    }
}

int WinMain()
{
    player = NPC(0x0000, {0, 0});
    currentGamemode = inGame;

    //warps = std::vector<Warp>(1);
    //warps[0] = Warp({ 1, 0 }, 1, {3, 4});
    //warps.shrink_to_fit();

    for (uint16_t x = 0; x < 256; x++)
    {
        for (uint16_t y = 0; y < 256; y++)
        {
            map[x][y].image = 0;//rand() % 2//rand() % 2 + 1
            if (x == y || x == 0 || y == 0)
            {
                map[x][y].image = 0;
            }
        }
    }

    loadMap(0);

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
        isKeyPressed = false;

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                break;
            }
        }

        //isKeyPressed = true;
        //keyPressed = sf::Event::KeyPressed;

        // Game

        switch (currentGamemode)
        {
        case inGame:
            /*for (uint8_t x = 0; x < warps.size(); x++)
            {
                if (warps[x].pos == player.pos)
                {
                    warpPlayer(warps[x].newMap, warps[x].newPos);
                }
            }*/

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                player.direction = 0;
                if (map[player.pos.x][player.pos.y - 1].walk == walk)
                {
                    currentGamemode = walking;
                    //player.walkOffset.y -= 1;
                    //player.pos.y--;
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                player.direction = 3;
                if (map[player.pos.x - 1][player.pos.y].walk == walk)
                {
                    currentGamemode = walking;
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                player.direction = 2;
                if (map[player.pos.x][player.pos.y + 1].walk == walk)
                {
                    currentGamemode = walking;
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                player.direction = 1;
                if (map[player.pos.x + 1][player.pos.y].walk == walk)
                {
                    currentGamemode = walking;
                }
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
            {
                warpPlayer(0, {0, 0});
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F2))
            {
                warpPlayer(1, { 0, 0 });
            }
            break;
        case walking:
            switch (player.direction)
            {
            case 0:
                player.walkOffset.y--;
                if (player.walkOffset.y <= -16)
                {
                    player.pos.y--;
                    finishWalk();
                }
                break;
            case 1:
                player.walkOffset.x++;
                if (player.walkOffset.x >= 16)
                {
                    player.pos.x++;
                    finishWalk();
                }
                break;
            case 2:
                player.walkOffset.y++;
                if (player.walkOffset.y >= 16)
                {
                    player.pos.y++;
                    finishWalk();
                }
                break;
            case 3:
                player.walkOffset.x--;
                if (player.walkOffset.x <= -16)
                {
                    player.pos.x--;
                    finishWalk();
                }
                break;
            }
            break;
        }

        /*if (true)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                player.direction = 0;
                if (map[player.pos.x][player.pos.y - 1].walk == walk)
                {
                    player.pos.y--;
                }
                break;
            case sf::Keyboard::A:
                player.direction = 3;
                if (map[player.pos.x - 1][player.pos.y].walk == walk)
                {
                    player.pos.x--;
                }
                break;
            case sf::Keyboard::S:
                player.direction = 2;
                if (map[player.pos.x][player.pos.y + 1].walk == walk)
                {
                    player.pos.y++;
                }
                break;
            case sf::Keyboard::D:
                player.direction = 1;
                if (map[player.pos.x + 1][player.pos.y].walk == walk)
                {
                    player.pos.x++;
                }
                break;
            }
        }*/

        // Draw

        window.clear();

        rectangle.setPosition(0, 0);
        rectangle.setSize((sf::Vector2f)screenRes);
        rectangle.setFillColor(sf::Color::Black);
        window.draw(rectangle);

        //rectangle.setSize(sf::Vector2f(screenRes.y / 16, screenRes.y / 16));

        sprite.setScale((double)screenRes.y / 256, (double)screenRes.y / 256);
        for (int8_t x = -1; x < 33; x++)
        {
            for (int8_t y = -1; y < 17; y++)
            {
                uint16_t image = map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image;

                sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));

                sprite.setPosition((x - ((double)player.walkOffset.x / 16)) * (screenRes.y / 16) + screenOffset, (y - ((double)player.walkOffset.y / 16)) * (screenRes.y / 16));

                window.draw(sprite);
            }
        }

        uint16_t image = 256 + player.direction;
        sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));
        sprite.setPosition(16 * (screenRes.y / 16) + screenOffset, 8 * (screenRes.y / 16));
        window.draw(sprite);

        window.display();

        time_++;
    }

    return 0;
}