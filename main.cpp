#include <SFML/Graphics.hpp>

sf::RectangleShape rectangle;
sf::Vector2<uint8_t> mapSize;

sf::Font backupFont;
sf::Text text;

sf::Vector2<uint16_t> mousePos;

bool exampleBool;
bool allowWalkThroughWalls;
bool disableMapEdgeWarps;
bool disableOtherMapWarps;

bool mouseButtonPressStarting;

class FontPage
{
public:
    bool exists;
    sf::Texture glyphs;
    uint8_t widths[0x100];

    FontPage()
    {
        exists = false;

    }
    FontPage(bool existsIn, sf::Texture glyphsIn, uint8_t widthsIn[0x100])
    {
        exists = existsIn;
        glyphs = glyphsIn;
        *widths = *widthsIn;
    }
};
FontPage mainFont[0x100];

class LangName
{
public:
    std::string lang;
    std::string dialect;
    LangName()
    {
        lang = "";
        dialect = "";
    }
    LangName(std::string langIn, std::string dialectIn)
    {
        lang = langIn;
        dialect = dialectIn;
    }
};
LangName langName;

class Translation
{
public:
    std::string unlocalized;
    std::basic_string<wchar_t> translation;

    Translation()
    {

    }

    Translation(std::string unlocalizedIn, std::basic_string<wchar_t> translationIn)
    {
        unlocalized = unlocalizedIn;
        translation = translationIn;
    }
};

class Lang
{
public:
    std::vector<Translation> translations;

    Lang()
    {
        translations = std::vector<Translation>();
    }
};
Lang lang;

class MapConnection
{
public:
    uint16_t mapID;
    uint8_t offset;
    sf::Vector2<uint8_t> mapSize;

    MapConnection()
    {
        mapID = 0;
        offset = 0;
        mapSize = { 0, 0 };
    }

    MapConnection(uint16_t mapIDIn, uint8_t offsetIn)
    {
        mapID = mapIDIn;
        offset = offsetIn;
        mapSize = {0, 0};
    }
};
MapConnection mapConnections[4];

enum ButtonType : uint8_t
{
    regularButton = 0, boolButton
};

enum TextAlignment : uint8_t
{
    textLeft, textCenter, textRight
};

enum Overlay : uint8_t
{
    nullOverlay = 0, movement
};
Overlay overlay;

enum ButtonEvent : uint8_t
{
    nullButtonEvent = 0, resume, exitGame, gotoTestsMenu, gotoDebugMenu, gotoMovementDebugMenu
};

enum MovementPerm : uint8_t
{
    noWalk = 0, walk = 1
};

enum GameMode : uint8_t
{
    inGame = 0, walking, warpingFadeOut, warpingFadeIn, menu
};
uint8_t fadeCounter;

enum Menu : uint8_t
{
    nullMenu = 0, pause, debugMenu, testsMenu, movementDebugMenu
};
Menu currentMenu;

class MapTile
{
public:
    uint16_t image;
    MovementPerm movement;
};

enum Events: uint16_t
{
    nullEvent = 0, warp = 1, warpThenMoveDown = 2
};

class Warp
{
public:
    Events type;
    sf::Vector2<uint8_t> pos;
    uint16_t newMap;
    sf::Vector2<uint8_t> newPos;

    Warp()
    {
        type = warp;
        pos = { 0x00, 0x00 };
        newMap = 0;
        newPos = { 0x00, 0x00 };
    }

    Warp(Events typeIn, sf::Vector2<uint8_t> posIn, uint16_t newMapIn, sf::Vector2<uint8_t> newPosIn)
    {
        type = typeIn;
        pos = posIn;
        newMap = newMapIn;
        newPos = newPosIn;
    }
};
Events currentEventType;

std::vector<Warp> warps;
uint8_t mapWarpingTo;

void loadLang(LangName in)
{
    langName = in;
    if (langName.lang != "")
    {
        lang = Lang();
        
        std::string path = std::string("assets/lang/") + in.lang + std::string("/") + in.dialect + std::string(".lng");
        sf::FileInputStream file;
        uint8_t buffer[0x10000];
        file.open(path);
        uint64_t fileSize = file.getSize();
        file.read(buffer, fileSize);

        Translation currentTranslation({"", L""});
        std::basic_string<wchar_t> currentStringW = L"";
        std::string currentString = "";
        uint8_t xchar;

        for (uint64_t x = 0; x < fileSize; x++)
        {
            xchar = buffer[x];

            switch (xchar)
            {
                case '\n':
                    currentTranslation.translation = currentStringW;
                    lang.translations.push_back(currentTranslation);
                    currentStringW = L"";
                    currentString = "";
                    currentTranslation = {};
                    break;
                case ':':
                    currentTranslation.unlocalized = (std::string)currentString;
                    currentStringW = L"";
                    x++;
                    break;
                default:
                    if (xchar < 0x80)
                    {
                        currentStringW += xchar;
                        currentString += xchar;
                    }
                    else if (xchar < 0xE0)
                    {
                        x++;
                        currentStringW += (xchar - 0xC0) * 0x40 + (buffer[x] - 0x80);
                    }
                    else if (xchar < 0xF0)
                    {
                        x += 2;
                        currentStringW += ((xchar - 0xE0) * 0x1000 + (buffer[x - 1] - 0x80) * 0x40 + (buffer[x] - 0x80));
                    }
            }
        }
    }
}

std::basic_string<wchar_t> getString(std::string in)
{
    uint64_t size = lang.translations.size();

    for (uint64_t x = 0; x < size; x++)
    {
        if (in == lang.translations[x].unlocalized)
        {
            return lang.translations[x].translation;
        }
    }

    size = in.size();
    std::basic_string<wchar_t> out = L"";

    for (uint64_t x = 0; x < size; x++)
    {
        out += in[x];
    }

    return out;
}

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

sf::Texture textures;
sf::Sprite sprite;

uint8_t keyPressed;
bool isKeyPressed;
GameMode gameMode;

void loadMap(uint16_t mapIn)
{
    sf::FileInputStream file;
    std::string mapPath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapIn / 256)) + std::string("/") + std::to_string((uint8_t)mapIn % 256);
    uint8_t buffer8[0x10000];
    uint16_t buffer16[0x10000];

    // Header

    std::string filePath = mapPath + std::string(".mhd");
    file.open(filePath);
    file.read(buffer8, file.getSize());
    uint8_t width = buffer8[0];
    uint8_t height = buffer8[1];
    mapSize = { width, height };
    uint16_t backgroundImage = buffer8[2] + buffer8[3] * 256;

    // Tiles

    filePath = mapPath + std::string(".mti");
    file.open(filePath);
    file.read(buffer16, file.getSize());

    for (uint16_t x = 0; x < 256; x++)
    {
        for (uint16_t y = 0; y < 256; y++)
        {
            map[x][y].image = backgroundImage;
            map[x][y].movement = noWalk;
        }
    }

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].image = buffer16[x + width * y];
            map[x][y].movement = walk;
        }
    }

    // Movement

    filePath = mapPath + std::string(".mmv");
    file.open(filePath);
    file.read(buffer8, file.getSize());

    for (uint16_t x = 0; x < width; x++)
    {
        for (uint16_t y = 0; y < height; y++)
        {
            map[x][y].movement = (MovementPerm)buffer8[x + width * y];
        }
    }

    // Events

    filePath = mapPath + std::string(".mev");
    file.open(filePath);
    file.read(buffer8, file.getSize());
    uint8_t eventCount = file.getSize() / 8;
    uint8_t warpCount = 0;

    for (uint8_t x = 0; x < eventCount; x++)
    {
        Events eventType = Events(buffer8[x * 8 + 2] + buffer8[x * 8 + 3] * 256);
        if (eventType == warp | eventType == warpThenMoveDown)
        {
            warpCount++;
        }
        else
        {
            switch (eventType)
            {
            case nullOverlay:
                break;
            }
        }
    }

    warps = std::vector<Warp>(warpCount);

    for (uint8_t x = 0; x < eventCount; x++)
    {
        Events eventType = Events(buffer8[x * 8 + 2] + buffer8[x * 8 + 3] * 256);
        sf::Vector2<uint8_t> eventPos = { buffer8[x * 8], buffer8[x * 8 + 1] };
        if (eventType == warp | eventType == warpThenMoveDown)
        {
            warps[x] = Warp(eventType, eventPos, buffer8[x * 8 + 4] + buffer8[x * 8 + 5] * 256, { buffer8[x * 8 + 6], buffer8[x * 8 + 7] });
        }
        switch (eventType)
        {
        case nullOverlay:
            break;
        }
    }

    // Map Connections

    filePath = mapPath + std::string(".mcn");
    file.open(filePath);
    file.read(buffer8, file.getSize());

    for (uint8_t x = 0; x < 4; x++)
    {
        mapConnections[x] = MapConnection(buffer8[x * 4] + buffer8[x * 4 + 1] * 256, buffer8[x * 4 + 2]);
    }

    std::string connectionMapPath;

    if (mapConnections[1].mapID != 0xFFFF)
    {
        connectionMapPath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapConnections[1].mapID / 256)) + std::string("/") + std::to_string((uint8_t)mapConnections[1].mapID % 256);
        file.open(connectionMapPath + std::string(".mhd"));
        file.read(buffer8, file.getSize());
        uint8_t connectionWidth = buffer8[0];
        uint8_t connectionHeight = buffer8[1];

        uint8_t visableConnectionWidth = connectionWidth;

        if (visableConnectionWidth > 17)
        {
            visableConnectionWidth = 17;
        }

        file.open(connectionMapPath + std::string(".mti"));
        file.read(buffer16, file.getSize());

        for (uint8_t x = 0; x < visableConnectionWidth; x++)
        {
            for (uint8_t y = 0; y < connectionHeight; y++)
            {
                map[x + width][(uint8_t)(y + mapConnections[1].offset)].image = buffer16[x + connectionWidth * y];
            }
        }

        file.open(connectionMapPath + std::string(".mmv"));
        file.read(buffer8, file.getSize());

        for (uint8_t y = 0; y < connectionHeight; y++)
        {
            map[width][(uint8_t)(y + mapConnections[1].offset)].movement = (MovementPerm)buffer8[connectionWidth * y];
        }
    }

    if (mapConnections[2].mapID != 0xFFFF)
    {
        connectionMapPath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapConnections[2].mapID / 256)) + std::string("/") + std::to_string((uint8_t)mapConnections[2].mapID % 256);
        file.open(connectionMapPath + std::string(".mhd"));
        file.read(buffer8, file.getSize());
        uint8_t connectionWidth = buffer8[0];
        uint8_t connectionHeight = buffer8[1];

        uint8_t visableConnectionHeight = connectionHeight;

        if (visableConnectionHeight > 9)
        {
            visableConnectionHeight = 9;
        }

        file.open(connectionMapPath + std::string(".mti"));
        file.read(buffer16, file.getSize());

        for (uint8_t x = 0; x < connectionWidth; x++)
        {
            for (uint8_t y = 0; y < visableConnectionHeight; y++)
            {
                map[(uint8_t)(x + mapConnections[2].offset)][y + height].image = buffer16[x + connectionWidth * y];
            }
        }

        file.open(connectionMapPath + std::string(".mmv"));
        file.read(buffer8, file.getSize());

        for (uint8_t x = 0; x < connectionWidth; x++)
        {
            map[(uint8_t)(x + mapConnections[2].offset)][height].movement = (MovementPerm)buffer8[x];
        }
    }

    if (mapConnections[0].mapID != 0xFFFF)
    {
        connectionMapPath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapConnections[0].mapID / 256)) + std::string("/") + std::to_string((uint8_t)mapConnections[0].mapID % 256);
        file.open(connectionMapPath + std::string(".mhd"));
        file.read(buffer8, file.getSize());
        uint8_t connectionWidth = buffer8[0];
        uint8_t connectionHeight = buffer8[1];

        mapConnections[0].mapSize = { connectionWidth, connectionHeight };

        uint8_t visableConnectionHeight = connectionHeight;

        if (visableConnectionHeight > 9)
        {
            visableConnectionHeight = 9;
        }

        file.open(connectionMapPath + std::string(".mti"));
        file.read(buffer16, file.getSize());

        for (uint8_t x = 0; x < connectionWidth; x++)
        {
            for (uint8_t y = 0; y < visableConnectionHeight; y++)
            {
                map[(uint8_t)(x + mapConnections[0].offset)][(uint8_t)(y + (0 - visableConnectionHeight))].image = buffer16[x + connectionWidth * (y + (connectionHeight - visableConnectionHeight))];
            }
        }

        file.open(connectionMapPath + std::string(".mmv"));
        file.read(buffer8, file.getSize());

        for (uint8_t x = 0; x < connectionWidth; x++)
        {
            map[(uint8_t)(x + mapConnections[0].offset)][255].movement = (MovementPerm)buffer8[(connectionWidth * (connectionHeight - 1)) + x];
        }
    }

    if (mapConnections[3].mapID != 0xFFFF)
    {
        connectionMapPath = std::string("assets/maps/") + std::to_string((uint8_t)floor(mapConnections[3].mapID / 256)) + std::string("/") + std::to_string((uint8_t)mapConnections[3].mapID % 256);
        file.open(connectionMapPath + std::string(".mhd"));
        file.read(buffer8, file.getSize());
        uint8_t connectionWidth = buffer8[0];
        uint8_t connectionHeight = buffer8[1];

        mapConnections[3].mapSize = { connectionWidth, connectionHeight };

        uint8_t visableConnectionWidth = connectionWidth;

        if (visableConnectionWidth > 17)
        {
            visableConnectionWidth = 17;
        }

        file.open(connectionMapPath + std::string(".mti"));
        file.read(buffer16, file.getSize());

        for (uint8_t x = 0; x < visableConnectionWidth; x++)
        {
            for (uint8_t y = 0; y < connectionHeight; y++)
            {
                map[(uint8_t)((x + (connectionWidth - visableConnectionWidth)) + (0 - connectionWidth))][(uint8_t)(y + mapConnections[3].offset)].image = buffer16[(x + (connectionWidth - visableConnectionWidth)) + connectionWidth * y];
            }
        }

        file.open(connectionMapPath + std::string(".mmv"));
        file.read(buffer8, file.getSize());

        for (uint8_t y = 0; y < connectionHeight; y++)
        {
            map[255][(uint8_t)(y + mapConnections[3].offset)].movement = (MovementPerm)buffer8[(uint16_t)((connectionWidth - 1) + (connectionWidth * y))];//(MovementPerm)buffer8[(connectionWidth - 1) + (connectionWidth * y)];
        }
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
    gameMode = inGame;

    for (uint8_t x = 0; x < warps.size(); x++)
    {
        if (warps[x].pos == player.pos && warps[x].type != warpThenMoveDown && !disableOtherMapWarps)
        {
            gameMode = warpingFadeOut;
            fadeCounter = 0;
            mapWarpingTo = x;
        }
    }

    if (player.pos.y == 255 && !disableMapEdgeWarps)
    {
        warpPlayer(mapConnections[0].mapID, { (uint8_t)(player.pos.x - mapConnections[0].offset), (uint8_t)(mapConnections[0].mapSize.y - 1) });
    }

    if (player.pos.x == mapSize.x && !disableMapEdgeWarps)
    {
        warpPlayer(mapConnections[1].mapID, {0, (uint8_t)(player.pos.y - mapConnections[1].offset)});
    }

    if (player.pos.y == mapSize.y && !disableMapEdgeWarps)
    {
        warpPlayer(mapConnections[2].mapID, { (uint8_t)(player.pos.x - mapConnections[2].offset), 0 });
    }

    if (player.pos.x == 255 && !disableMapEdgeWarps)
    {
        warpPlayer(mapConnections[3].mapID, { (uint8_t)(mapConnections[3].mapSize.x - 1), (uint8_t)(player.pos.y - mapConnections[3].offset) });
    }
}

sf::Vector2<uint16_t> getGUIPos(sf::Vector2<int16_t> in)
{
    return { uint16_t(screenRes.x / 2 + (double)in.x / 256 * screenRes.y), uint16_t(screenRes.y / 2 + (double)in.y / 256 * screenRes.y) };
}

sf::Vector2<uint16_t> getGUISize(sf::Vector2<uint16_t> in)
{
    return { uint16_t((double)in.x / 256 * screenRes.y), uint16_t((double)in.y / 256 * screenRes.y) };
}

uint16_t getGUISize1D(uint16_t in)
{
    return uint16_t((double)in / 256 * screenRes.y);
}

class GUIButton
{
public:
    sf::Vector2<int16_t> pos;
    sf::Vector2<uint16_t> size;
    std::string text;
    ButtonEvent event;
    bool active;
    ButtonType type;
    bool* value;

    GUIButton()
    {
        pos = { 0, 0 };
        size = { 1, 1 };
        text = "";
        event = nullButtonEvent;
        active = true;
        type = regularButton;
    }

    GUIButton(sf::Vector2<int16_t> posIn, sf::Vector2<uint16_t> sizeIn, std::string textIn, ButtonEvent eventIn, bool activeIn, ButtonType typeIn, bool* valueIn)
    {
        pos = posIn;
        size = sizeIn;
        text = textIn;
        event = eventIn;
        active = activeIn;
        type = typeIn;
        value = valueIn;
    }

    GUIButton(uint8_t posIn, std::string textIn, ButtonEvent eventIn, bool activeIn, ButtonType typeIn, bool* valueIn)
    {
        pos = { -64, -100 + posIn * 24 };
        size = { 128, 16 };
        text = textIn;
        event = eventIn;
        active = activeIn;
        type = typeIn;
        value = valueIn;
    }
};
std::vector<GUIButton> GUIButtons;

class GUIText
{
public:
    sf::Vector2<int16_t> pos;
    uint16_t size;
    std::string text;
    TextAlignment alignment;

    GUIText()
    {
    }

    GUIText(sf::Vector2<int16_t> posIn, uint16_t sizeIn, std::string textIn, TextAlignment alignmentIn)
    {
        pos = posIn;
        size = sizeIn;
        text = textIn;
        alignment = alignmentIn;
    }
};
std::vector<GUIText> GUITexts;

void loadFont()
{
    for (uint16_t x = 0; x < 0x100; x++)
    {
        std::string path = "assets/textures/font/" + std::to_string(x);
        mainFont[x].glyphs.loadFromFile(path + ".png");

        sf::FileInputStream file;
        file.open(path + ".cwt");
        file.read(mainFont[x].widths, file.getSize());
    }
}

int WinMain()
{
    // Init

    exampleBool = true;
    allowWalkThroughWalls = false;
    disableMapEdgeWarps = false;
    disableOtherMapWarps = false;

    sf::RenderWindow window(sf::VideoMode(512, 256), "RPG", sf::Style::Fullscreen);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    screenRes = sf::Vector2<uint16_t>(window.getSize());
    screenAspectRatio = (double)screenRes.x / (double)screenRes.y;
    screenOffset = (screenRes.x - (screenRes.y * 2)) / 2;

    textures.loadFromFile("assets/textures/textures.png");

    backupFont.loadFromFile("C:/Windows/Fonts/arial.ttf");
    text.setFont(backupFont);

    loadFont();

    loadMap(2);

    player = NPC(0x0000, {0, 0});
    gameMode = inGame;
    player.pos = { 4, 4 };

    loadLang({ "en", "no" });

    GUIButtons = std::vector<GUIButton>(0);
    GUITexts = std::vector<GUIText>(0);

    // Game Loop

    while (window.isOpen())
    {
        isKeyPressed = false;
        mouseButtonPressStarting = false;

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
            case sf::Event::MouseButtonPressed:
                mouseButtonPressStarting = true;
                break;
            }
        }

        mousePos = sf::Vector2<uint16_t>(sf::Mouse::getPosition());

        switch (gameMode)
        {
        case inGame:
            GUITexts = std::vector<GUIText>(0);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                gameMode = menu;
                currentMenu = pause;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Home))
            {
                gameMode = menu;
                currentMenu = debugMenu;
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                player.direction = 0;
                if (map[player.pos.x][(uint8_t)(player.pos.y - 1)].movement == walk || allowWalkThroughWalls)
                {
                    gameMode = walking;
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                player.direction = 3;
                if (map[(uint8_t)(player.pos.x - 1)][player.pos.y].movement == walk || allowWalkThroughWalls)
                {
                    gameMode = walking;
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                player.direction = 2;
                if (map[player.pos.x][player.pos.y + 1].movement == walk || allowWalkThroughWalls)
                {
                    gameMode = walking;
                }

                for (uint8_t x = 0; x < warps.size(); x++)
                {
                    if (warps[x].pos == player.pos && warps[x].type == warpThenMoveDown)
                    {
                        gameMode = warpingFadeOut;
                        fadeCounter = 0;
                        mapWarpingTo = x;
                    }
                }
            }

            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                player.direction = 1;
                if (map[player.pos.x + 1][player.pos.y].movement == walk || allowWalkThroughWalls)
                {
                    gameMode = walking;
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
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F5))
            {
                overlay = nullOverlay;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F6))
            {
                overlay = movement;
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
        case warpingFadeOut:
            if (fadeCounter >= 240)
            {
                fadeCounter = 255;
                currentEventType = warps[mapWarpingTo].type;
                warpPlayer(warps[mapWarpingTo].newMap, warps[mapWarpingTo].newPos);
                gameMode = warpingFadeIn;
                break;
            }

            fadeCounter += 16;
            break;
        case warpingFadeIn:
            if (fadeCounter <= 15)
            {
                fadeCounter = 0;
                gameMode = inGame;
                if (currentEventType == warpThenMoveDown)
                {
                    player.direction = 2;
                    gameMode = walking;
                    player.walkOffset.y = 1;
                }
                break;
            }
            fadeCounter -= 16;
            break;
        case menu:
            switch (currentMenu)
            {
            case pause:
                GUIButtons = std::vector<GUIButton>(2);
                GUIButtons[0] = GUIButton(0, "resume", resume, true, regularButton, 0);
                GUIButtons[1] = GUIButton(7, "exitGame", exitGame, true, regularButton, 0);

                GUITexts = std::vector<GUIText>(4);
                GUITexts[0] = GUIText({ 0, -120 }, 0, "gamePaused", textCenter);
                GUITexts[1] = GUIText({ -128, 88 }, 0, "version", textLeft);
                GUITexts[2] = GUIText({ -128, 100 }, 0, "releaseDate", textLeft);
                GUITexts[3] = GUIText({ -128, 112 }, 0, "copyright", textLeft);
                break;
            case debugMenu:
                GUIButtons = std::vector<GUIButton>(3);
                GUIButtons[0] = GUIButton(0, "resume", resume, true, regularButton, 0);
                GUIButtons[1] = GUIButton(1, "tests", gotoTestsMenu, true, regularButton, 0);
                GUIButtons[2] = GUIButton(2, "movement", gotoMovementDebugMenu, true, regularButton, 0);

                GUITexts = std::vector<GUIText>(1);
                GUITexts[0] = GUIText({ 0, -120 }, 0, "debug", textCenter);
                break;
            case testsMenu:
                GUIButtons = std::vector<GUIButton>(2);
                GUIButtons[0] = GUIButton(0, "back", gotoDebugMenu, true, regularButton, 0);
                GUIButtons[1] = GUIButton(2, "boolTest", nullButtonEvent, true, boolButton, &exampleBool);

                GUITexts = std::vector<GUIText>(2);
                GUITexts[0] = GUIText({ 0, -120 }, 0, "testsMenu", textCenter);
                GUITexts[1] = GUIText({ 0, -84 }, 0, "exampleUTF8String", textCenter);
                break;
            case movementDebugMenu:
                GUIButtons = std::vector<GUIButton>(4);
                GUIButtons[0] = GUIButton(0, "back", gotoDebugMenu, true, regularButton, 0);
                GUIButtons[1] = GUIButton(1, "walkThroughWalls", nullButtonEvent, true, boolButton, &allowWalkThroughWalls);
                GUIButtons[2] = GUIButton(2, "disableMapEdgeWarps", nullButtonEvent, true, boolButton, &disableMapEdgeWarps);
                GUIButtons[3] = GUIButton(3, "disableOtherMapWarps", nullButtonEvent, true, boolButton, &disableOtherMapWarps);

                GUITexts = std::vector<GUIText>(1);
                GUITexts[0] = GUIText({ 0, -120 }, 0, "movement", textCenter);
                break;
            }

            for (uint8_t x = 0; x < GUIButtons.size(); x++)
            {
                if (mouseButtonPressStarting && GUIButtons[x].active && mousePos.x >= getGUIPos(GUIButtons[x].pos).x && mousePos.y >= getGUIPos(GUIButtons[x].pos).y && mousePos.x < getGUISize(GUIButtons[x].size).x + getGUIPos(GUIButtons[x].pos).x && mousePos.y < getGUISize(GUIButtons[x].size).y + getGUIPos(GUIButtons[x].pos).y)
                {
                    switch (GUIButtons[x].type)
                    {
                    case regularButton:
                        switch (GUIButtons[x].event)
                        {
                        case resume:
                            gameMode = inGame;
                            break;
                        case exitGame:
                            window.close();
                            break;
                        case gotoTestsMenu:
                            currentMenu = testsMenu;
                            break;
                        case gotoDebugMenu:
                            currentMenu = debugMenu;
                            break;
                        case gotoMovementDebugMenu:
                            currentMenu = movementDebugMenu;
                            break;
                        }
                        break;
                    case boolButton:
                        *GUIButtons[x].value = !*GUIButtons[x].value;
                        break;
                    }
                }
            }
            break;
        }

        // Draw

        window.clear();

        sprite.setTexture(textures);

        rectangle.setPosition(0, 0);
        rectangle.setSize((sf::Vector2f)screenRes);
        rectangle.setFillColor(sf::Color::Black);
        window.draw(rectangle);

        sprite.setScale((double)screenRes.y / 256, (double)screenRes.y / 256);
        rectangle.setSize(sf::Vector2f(screenRes.y / 16, screenRes.y / 16));
        for (int8_t x = -1; x < 33; x++)
        {
            for (int8_t y = -1; y < 17; y++)
            {
                uint16_t image = map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].image;
                sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));
                rectangle.setPosition((x - ((double)player.walkOffset.x / 16))* (screenRes.y / 16) + screenOffset, (y - ((double)player.walkOffset.y / 16))* (screenRes.y / 16));
                sprite.setPosition((x - ((double)player.walkOffset.x / 16)) * (screenRes.y / 16) + screenOffset, (y - ((double)player.walkOffset.y / 16)) * (screenRes.y / 16));
                window.draw(sprite);
                switch (overlay)
                {
                case movement:
                    rectangle.setFillColor(sf::Color(255, 255, 0, 127));
                    switch (map[(uint8_t)(x - 16 + player.pos.x)][(uint8_t)(y - 8 + player.pos.y)].movement)
                    {
                    case noWalk:
                        rectangle.setFillColor(sf::Color(255, 0, 0, 127));
                        break;
                    case walk:
                        rectangle.setFillColor(sf::Color(0, 255, 0, 127));
                        break;
                    }
                    break;
                }
                if (overlay != nullOverlay)
                {
                    window.draw(rectangle);
                }
            }
        }

        uint16_t image = 256 + player.direction;
        sprite.setTextureRect(sf::Rect<int>((image % 256) % 16 * 16 + (floor((image % 4096) / 256) * 256), floor((image % 256) / 16) * 16 + floor(image / 4096) * 256, 16, 16));
        sprite.setPosition(16 * (screenRes.y / 16) + screenOffset, 8 * (screenRes.y / 16));
        window.draw(sprite);

        if (gameMode == warpingFadeOut || gameMode == warpingFadeIn)
        {
            rectangle.setPosition(0, 0);
            rectangle.setSize((sf::Vector2f)screenRes);
            rectangle.setFillColor(sf::Color(0, 0, 0, fadeCounter));
            window.draw(rectangle);
        }

        if (gameMode == menu)
        {
            rectangle.setPosition((sf::Vector2f)getGUIPos({ -64, -100 }));
            rectangle.setSize((sf::Vector2f)getGUISize({ 128, 184 }));
            rectangle.setFillColor(sf::Color::White);
            window.draw(rectangle);

            for (uint8_t x = 0; x < GUIButtons.size(); x++)
            {
                rectangle.setPosition((sf::Vector2f)getGUIPos(GUIButtons[x].pos));
                rectangle.setSize((sf::Vector2f)getGUISize(GUIButtons[x].size));
                rectangle.setFillColor(sf::Color::Color(127, 127, 127));
                bool mouseInside;
                mouseInside = (mousePos.x >= getGUIPos(GUIButtons[x].pos).x && mousePos.y >= getGUIPos(GUIButtons[x].pos).y && mousePos.x < getGUISize(GUIButtons[x].size).x + getGUIPos(GUIButtons[x].pos).x && mousePos.y < getGUISize(GUIButtons[x].size).y + getGUIPos(GUIButtons[x].pos).y);
                if (!GUIButtons[x].active)
                {
                    rectangle.setFillColor(sf::Color::Color(63, 63, 63));
                }

                switch (GUIButtons[x].type)
                {
                case regularButton:
                    if (mouseInside)
                    {
                        rectangle.setFillColor(sf::Color::Color(127, 127, 255));
                    }
                    break;
                case boolButton:
                    rectangle.setFillColor(sf::Color::Color(255, 127, 127));
                    if (*GUIButtons[x].value)
                    {
                        rectangle.setFillColor(sf::Color::Color(127, 255, 127));
                    }
                    if (mouseInside)
                    {
                        rectangle.setFillColor(sf::Color::Color(255, 0, 0));
                        if (*GUIButtons[x].value)
                        {
                            rectangle.setFillColor(sf::Color::Color(0, 255, 0));
                        }
                    }
                    break;
                }
                window.draw(rectangle);

                uint16_t stringWidth = 0;

                for (uint8_t y = 0; y < getString(GUIButtons[x].text).size(); y++)
                {
                    uint16_t charID = getString(GUIButtons[x].text)[y];
                    stringWidth += mainFont[(uint8_t)floor(charID / 256)].widths[charID % 256] + 1;
                }

                uint16_t xOffset = 0;

                for (uint16_t y = 0; y < getString(GUIButtons[x].text).size(); y++)
                {
                    uint16_t charID = getString(GUIButtons[x].text)[y];

                    sprite.setTexture(mainFont[(uint8_t)floor(charID / 256)].glyphs);

                    sprite.setTextureRect({ (uint8_t)charID % 16 * 8, (uint16_t)floor((uint8_t)charID / 16) * 16, 8, 16 });
                    sprite.setPosition((sf::Vector2f)(getGUIPos(GUIButtons[x].pos) + getGUISize({ (uint16_t)((uint16_t)xOffset + (uint16_t)(GUIButtons[x].size.x / 2) - stringWidth / 2), 0 })));
                    window.draw(sprite);
                    xOffset += mainFont[(uint8_t)floor(charID / 256)].widths[charID % 256] + 1;
                }
            }
        }

        for (uint8_t x = 0; x < GUITexts.size(); x++)
        {

            uint16_t stringWidth = 0;

            for (uint8_t y = 0; y < getString(GUITexts[x].text).size(); y++)
            {
                uint16_t charID = getString(GUITexts[x].text)[y];
                stringWidth += mainFont[(uint8_t)floor(charID / 256)].widths[charID % 256] + 1;
            }

            switch (GUITexts[x].alignment)
            {
            case textLeft:
                stringWidth = 0;
                break;
            case textCenter:
                stringWidth = stringWidth / 2;
                break;
            case textRight:
                stringWidth = stringWidth;
                break;
            }

            uint16_t xOffset = 0;

            for (uint16_t y = 0; y < getString(GUITexts[x].text).size(); y++)//GUITexts[x].text.size()
            {
                uint16_t charID = getString(GUITexts[x].text)[y];

                sprite.setTexture(mainFont[(uint8_t)floor(charID / 256)].glyphs);

                sprite.setTextureRect({ (uint8_t)charID % 16 * 8, (uint16_t)floor((uint8_t)charID / 16) * 16, 8, 16 });
                sprite.setPosition((sf::Vector2f)(getGUIPos(GUITexts[x].pos) + getGUISize({ xOffset, 0 }) - getGUISize({ stringWidth, 0 })));
                window.draw(sprite);
                xOffset += mainFont[(uint8_t)floor(charID / 256)].widths[charID % 256] + 1;
            }
        }

        window.display();

        time_++;
    }

    return 0;
}