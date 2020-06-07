#include <SFML/Graphics.hpp>
#include <fstream>

enum EditingMode: uint8_t
{
    tiles = 0, movement
};
EditingMode editingMode;

enum MovementPerm: uint8_t
{
    noWalk = 0, walk = 1
};
MovementPerm movementPen;

class MapTile
{
public:
    uint16_t image;
    MovementPerm movement;
};

sf::RectangleShape rectangle;

sf::Texture textures;
sf::Sprite sprite;
MapTile map[0x100][0x100];

sf::Vector2<uint8_t> size;
uint8_t scale = 2;

uint16_t pen;
uint16_t mapID;

void loadMap(uint16_t mapIn)
{
    sf::FileInputStream file;

    uint8_t buffer8[0x10000];
    std::string filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mhd");
    file.open(filePath);
    file.read(buffer8, file.getSize());
    uint8_t width = buffer8[0];
    uint8_t height = buffer8[1];
    size = { width, height };

    uint16_t buffer16[0x10000];
    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mti");
    file.open(filePath);
    file.read(buffer16, file.getSize());

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].image = buffer16[x + width * y];
        }
    }

    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mmv");
    file.open(filePath);
    file.read(buffer8, file.getSize());

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].movement = (MovementPerm)buffer8[x + width * y];
        }
    }
}

void saveMap(uint16_t mapIn)
{
    std::string filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mti");
    std::ofstream file(filePath, std::ios::binary);//std::ofstream

    uint16_t fileSize = size.x * size.y * 2;
    std::vector<uint8_t> out(fileSize);
    char memBlock[0x100];

    for (uint16_t x = 0; x < size.x; x++)
    {
        for (uint16_t y = 0; y < size.y; y++)
        {
            out[(x + (uint32_t)size.x * y) * 2] = map[x][y].image % 256;
            out[(x + (uint32_t)size.x * y) * 2 + 1] = floor(map[x][y].image / 256);
        }
    }

    for (uint64_t x = 0; x < fileSize; x += 0x100)
    {
        if (0x100 <= fileSize - x)
        {
            for (uint64_t y = 0; y < 0x100; y++)
            {
                memBlock[y] = out[x + y];
            }
            file.write(memBlock, 0x100);
        }

        else
        {
            for (uint64_t y = 0; y < fileSize - x; y++)
            {
                memBlock[y] = out[x + y];
            }
            file.write(memBlock, fileSize - x);
        }
    }

    file.close();

    filePath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256) + std::string(".mmv");
    file = std::ofstream(filePath, std::ios::binary);
    fileSize = size.x * size.y;

    out = std::vector<uint8_t>(fileSize);
    //char memBlock[0x100];

    for (uint16_t x = 0; x < size.x; x++)
    {
        for (uint16_t y = 0; y < size.y; y++)
        {
            out[(x + (uint32_t)size.x * y)] = map[x][y].movement % 256;
        }
    }

    for (uint64_t x = 0; x < fileSize; x += 0x100)
    {
        if (0x100 <= fileSize - x)
        {
            for (uint64_t y = 0; y < 0x100; y++)
            {
                memBlock[y] = out[x + y];
            }
            file.write(memBlock, 0x100);
        }

        else
        {
            for (uint64_t y = 0; y < fileSize - x; y++)
            {
                memBlock[y] = out[x + y];
            }
            file.write(memBlock, fileSize - x);
        }
    }
}

sf::Vector2<uint16_t> windowSize(2000, 1500);

int WinMain()
{
    movementPen = noWalk;
    editingMode = tiles;

    uint64_t time = 0;
    pen = 0;
    
    loadMap(0);

    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "RPG Layout Editor");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    textures.loadFromFile("assets/textures/textures.png");
    sprite.setTexture(textures);

    editingMode = tiles;

    //size = {10, 12};

    while (window.isOpen())
    {
        time++;

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            /*case sf::Event::MouseButtonPressed:
                switch (editingMode)
                {
                case tiles:
                    map[sf::Mouse::getPosition(window).x / 16 / scale][sf::Mouse::getPosition(window).y / 16 / scale].image = pen;
                    break;
                case movement:
                    map[sf::Mouse::getPosition(window).x / 16 / scale][sf::Mouse::getPosition(window).y / 16 / scale].movement = movementPen;
                    break;
                }
                break;*/
            case sf::Event::KeyReleased:
                switch (event.key.code)
                {
                case sf::Keyboard::L:
                    loadMap(mapID);
                    break;
                case sf::Keyboard::S:
                    saveMap(mapID);
                    break;
                case sf::Keyboard::Equal:
                    mapID++;
                    break;
                case sf::Keyboard::Hyphen:
                    mapID--;
                    break;
                case sf::Keyboard::Up:
                    pen++;
                    break;
                case sf::Keyboard::Down:
                    pen--;
                    break;
                case sf::Keyboard::Left:
                    pen -= 256;
                    break;
                case sf::Keyboard::Right:
                    pen += 256;
                    break;
                case sf::Keyboard::F5:
                    editingMode = tiles;
                    break;
                case sf::Keyboard::F6:
                    editingMode = movement;
                    break;
                case sf::Keyboard::F9:
                    movementPen = noWalk;
                    break;
                case sf::Keyboard::F10:
                    movementPen = walk;
                    break;
                }
                break;
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (sf::Mouse::getPosition(window).x > windowSize.x - 256)
            {
                if (sf::Mouse::getPosition(window).y > 255 && sf::Mouse::getPosition(window).y < 512)
                {
                    pen = floor(pen / 256.) * 256 + floor((sf::Mouse::getPosition(window).x - windowSize.x + 256) / 16) + floor((sf::Mouse::getPosition(window).y - 256) / 16) * 16;
                }
                else if (sf::Mouse::getPosition(window).y < 256)
                {
                    pen = (pen % 256) + floor((sf::Mouse::getPosition(window).x - windowSize.x + 256) / 16) * 256 + floor((sf::Mouse::getPosition(window).y) / 16) * 4096;
                }
            }
            else
            {
                switch (editingMode)
                {
                case tiles:
                    map[sf::Mouse::getPosition(window).x / 16 / scale][sf::Mouse::getPosition(window).y / 16 / scale].image = pen;
                    break;
                case movement:
                    map[sf::Mouse::getPosition(window).x / 16 / scale][sf::Mouse::getPosition(window).y / 16 / scale].movement = movementPen;
                    break;
                }
            }
        }

        window.clear();

        sprite.setScale(scale, scale);
        rectangle.setSize(sf::Vector2f(scale * 16, scale * 16));

        for (uint16_t x = 0; x < size.x; x++)
        {
            for (uint16_t y = 0; y < size.y; y++)
            {
                sprite.setPosition(x * 16 * scale, y * 16 * scale);
                uint16_t image = map[x][y].image;//map[x][y].image
                sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));

                window.draw(sprite);

                if (editingMode == movement)
                {
                    rectangle.setFillColor(sf::Color(255, 255, 0, 127));
                    switch (map[x][y].movement)
                    {
                    case noWalk:
                        rectangle.setFillColor(sf::Color(255, 0, 0, 127));
                        break;
                    case walk:
                        rectangle.setFillColor(sf::Color(0, 255, 0, 127));
                        break;
                    }

                    rectangle.setPosition(x * 16 * scale, y * 16 * scale);
                    window.draw(rectangle);
                }
            }
        }

        rectangle.setPosition(windowSize.x - 256, 0);
        rectangle.setSize({ 256, 512 });
        rectangle.setFillColor({127, 127, 127, 255});
        window.draw(rectangle);

        sprite.setScale(1. / 16., 1. / 16.);
        sprite.setPosition(windowSize.x - 256, 0);
        sprite.setTextureRect(sf::Rect<int>(0, 0, 4096, 4096));
        window.draw(sprite);

        sprite.setScale(1, 1);
        sprite.setPosition(windowSize.x - 256, 256);
        sprite.setTextureRect(sf::Rect<int>(floor((pen % 4096) / 256.) * 256, floor(pen / 4096.) * 256, 256, 256));
        window.draw(sprite);

        rectangle.setPosition(windowSize.x - 256 + floor(pen % 4096 / 256) * 16, floor(pen / 4096) * 16);
        rectangle.setSize({ 16, 16 });
        rectangle.setFillColor({ 255, 255, 0, 127 });
        window.draw(rectangle);

        rectangle.setPosition(windowSize.x - 256 + pen % 16 * 16, 256 + floor((pen % 256) / 16.) * 16);
        window.draw(rectangle);

        window.display();
    }

    //uint16_t image = map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image;

    //sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));

    return 0;
}