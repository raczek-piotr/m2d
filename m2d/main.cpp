#include <SFML/Graphics.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "headers.h"
#include "voxels.cpp"

using namespace std;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t s32;

const int PORT = 54002;
const char* SERVER_IP = "T440s.local"; //192.168.212.108


struct chunk_t {
    s32 x, y;
    u16 chunk[CHUNK_SIZE][CHUNK_SIZE];
};

struct map_t {
    s32 x, y;
    chunk_t* chunk[worldMapX][worldMapY];
};

// --- Globals ---
float px = 100, py = 5;
float prot = 0; // player rotation
u16 world = 1;  // [1, 4]

u16 trashbin = 0; // trashbin

map_t worldMaps[5];
map_t * worldMap = & worldMaps[world]; // current world
map_t * worldBG = & worldMaps[0];
int sock;
mutex mtx;
//map<s32, sf::Vector2f> players;

// --- Function declarations ---
void initWorldMap();
u16 getBlock(u16 world, s32 wx, s32 wy);
void receiveLoop();
void drawWorld(sf::RenderWindow &window);

int main() {

    // --- Connect to server ---
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return 1;
    }

    // --- Start receive thread ---
    thread(receiveLoop).detach();

    // --- Initialize world ---
    initWorldMap();

    // --- SFML window ---
    sf::RenderWindow window(sf::VideoMode(screenX, screenY), "m2d GameJam");

    vector<u16> availableBlocks = {1, 2, 3, 4, 5, 6, 0};
    u16 selectedBlock = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Mouse wheel: switch block
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0)
                    selectedBlock = (selectedBlock + 1) % availableBlocks.size();
                else
                    selectedBlock = (selectedBlock - 1 + availableBlocks.size()) % availableBlocks.size();
            }

            // Mouse click: SET voxel
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                s32 vx = (mousePos.x -screenXhalf + px * TILE_SIZE) / TILE_SIZE;
                s32 vy = (mousePos.y -screenYhalf + py * TILE_SIZE) / TILE_SIZE;
                u16 block = availableBlocks[selectedBlock];
                string cmd = "SET " + to_string(vx) + " " + to_string(vy) + " " + to_string(world) + " " + to_string(block) + "\n";
                send(sock, cmd.c_str(), cmd.size(), 0);
            }
        }

        /*// Mouse camera movement
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        px = mousePos.x;
        py = mousePos.y;*/
        
        // --- Movement by arrow keys ---
		float speed = 5.0f; // blocks per second
		static sf::Clock moveClock;
		float dt = moveClock.restart().asSeconds();
		static int counter = 64;
		static float time = 0;
		time += dt;
		if (--counter < 0) {
			cout<<64/time<<" ping: "<<1000*time/64<<'\n';
			time = 0;
			counter = 64;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
			world = 1;
			worldMap = & worldMaps[world];
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
			world = 2;
			worldMap = & worldMaps[world];
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
			world = 3;
			worldMap = & worldMaps[world];
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
			world = 4;
			worldMap = & worldMaps[world];
		}

		float dir = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))  dir = -speed;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir =  speed;

		float tpx = px + dir * dt;
		if (getBlock(world, tpx, py) == 0) px = tpx;

		float pa = TILE_SIZE/2;
		static float pv = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) pv = TILE_SIZE/2;

		float tpy = py - pv * dt;
		if (getBlock(world, px, tpy) == 0) {
			py = tpy;
			pv -= pa * dt;
			if (pv < -TILE_SIZE) pv = -TILE_SIZE;
		} else pv = 0;

		//if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		//	py -= speed * dt;
		//}
		//if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		//	py += speed * dt;
		//}
		prot += 180*dt;
		if (dt > 360) dt -= 360;

	// camera move
	// if → while (if velosity fill be greater)
    if (worldMap->x > 0 and (worldMap->x+chunkMarginX)*CHUNK_SIZE>px) {
		for (u16 world = 0; world < 5; world++) worldMaps[world].x-=1;
		for (s32 y = 0; y != worldMapY; y++) {
			for (u16 world = 0; world < 5; world++) {
				chunk_t* Q = worldMaps[world].chunk[worldMapX-1][y];
				for (s32 x = worldMapX-1; x != 0; x--) {
						worldMaps[world].chunk[x][y] = worldMaps[world].chunk[x-1][y];
				}
				worldMaps[world].chunk[0][y] = Q;
			}
            string cmd = "GETCHUNK " + to_string(worldMap->x) + " " + to_string(worldMap->y+y) + "\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
		}
	} else if (worldMap->x < worldMapMSX and (1+worldMap->x+chunkMarginX)*CHUNK_SIZE<px) {
		for (u16 world = 0; world < 5; world++) worldMaps[world].x+=1;
		for (s32 y = 0; y != worldMapY; y++) {
			for (u16 world = 0; world < 5; world++) {
				chunk_t* Q = worldMaps[world].chunk[0][y];
				for (s32 x = 0; x != worldMapX; x++) {
					worldMaps[world].chunk[x][y] = worldMaps[world].chunk[x+1][y];
				}
				worldMaps[world].chunk[worldMapX-1][y] = Q;
			}
            string cmd = "GETCHUNK " + to_string(worldMap->x+worldMapX-1) + " " + to_string(worldMap->y+y) + "\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
		}
	}
    if (worldMap->y > 0 and (worldMap->y+chunkMarginY)*CHUNK_SIZE>py) {
		for (u16 world = 0; world < 5; world++) worldMaps[world].y-=1;
		for (s32 x = 0; x != worldMapX; x++) {
			for (u16 world = 0; world < 5; world++) {
				chunk_t* Q = worldMaps[world].chunk[x][worldMapY-1];
				for (s32 y = worldMapY-1; y != 0; y--) {
					worldMaps[world].chunk[x][y] = worldMaps[world].chunk[x][y-1];
				}
				worldMaps[world].chunk[x][0] = Q;
			}
            string cmd = "GETCHUNK " + to_string(worldMap->x+x) + " " + to_string(worldMap->y) + "\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
		}
	} else if (worldMap->y < worldMapMSY and (1+worldMap->y+chunkMarginY)*CHUNK_SIZE<py) {
		for (u16 world = 0; world < 5; world++) worldMaps[world].y+=1;
		for (s32 x = 0; x != worldMapX; x++) {
			for (u16 world = 0; world < 5; world++) {
				chunk_t* Q = worldMaps[world].chunk[x][0];
				for (s32 y = 0; y != worldMapY; y++) {
					worldMaps[world].chunk[x][y] = worldMaps[world].chunk[x][y+1];
				}
				worldMaps[world].chunk[x][worldMapY-1] = Q;
			}
            string cmd = "GETCHUNK " + to_string(worldMap->x+x) + " " + to_string(worldMap->y+worldMapY-1) + "\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
		}
	}

		// draw window
        window.clear(sf::Color(30, 30, 30));
        drawWorld(window);
        window.display();

        //sf::sleep(sf::milliseconds(16));
    }

    close(sock);
    return 0;
}

// --- Initialize world map ---
void initWorldMap() {
    worldMap->x = 0;
    worldMap->y = 0;
    for (int i = 0; i < worldMapX; i++) {
        for (int j = 0; j < worldMapY; j++) {
			for (u16 world = 0; world < 5; world++)
				worldMaps[world].chunk[i][j] = new chunk_t;
            string cmd = "GETCHUNK " + to_string(i) + " " + to_string(j) + "\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
            cmd = "GETCHUNK " + to_string(i) + " " + to_string(j) + " 0\n";
            send(sock, cmd.c_str(), cmd.size(), 0);
        }
    }
}

// --- Get block by world coordinates ---
u16 getBlock(u16 world, s32 wx, s32 wy) {
    s32 cx = wx / CHUNK_SIZE - worldMap->x;
    s32 cy = wy / CHUNK_SIZE - worldMap->y;
    s32 lx = wx % CHUNK_SIZE;
    s32 ly = wy % CHUNK_SIZE;

    if (cx < 0 || cx >= worldMapX || cy < 0 || cy >= worldMapY)
        return 0;

    return worldMaps[world].chunk[cx][cy]->chunk[ly][lx];
}
// --- Get block by world coordinates TO SET something ---
u16* setBlock(u16 world, s32 wx, s32 wy) {
	s32 cx = (wx - CHUNK_SIZE*worldMap->x) / CHUNK_SIZE;
	s32 cy = (wy - CHUNK_SIZE*worldMap->y) / CHUNK_SIZE;
	s32 lx = wx % CHUNK_SIZE;
	s32 ly = wy % CHUNK_SIZE;

	if (cx < 0 || cx >= worldMapX || cy < 0 || cy >= worldMapY) {
		return &trashbin; // t is global trashbin
	}

	//cout << &(worldMap->chunk[cx][cy]->chunk[ly][lx]) << ' ' << &(worldMaps[world].chunk[cx][cy]->chunk[ly][lx]) << '\n';
	//return &(worldMap->chunk[cx][cy]->chunk[ly][lx]);
    return &(worldMaps[world].chunk[cx][cy]->chunk[ly][lx]);
}

// --- Receive loop ---
void receiveLoop() {
    string leftover;
    char buffer[16384];

    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        leftover += buffer;

        size_t pos;
        while ((pos = leftover.find(';')) != string::npos) {
            string token = leftover.substr(0, pos);
            leftover.erase(0, pos + 1);
            if (token.empty()) continue;

            lock_guard<mutex> lock(mtx);

            // --- CHUNK message ---
            if (token.rfind("CHUNK", 0) == 0) {
                size_t firstSpace  = token.find(' ');
				size_t secondSpace = token.find(' ', firstSpace + 1);
				size_t thirdSpace  = token.find(' ', secondSpace + 1);
				size_t fourthSpace = token.find(' ', thirdSpace + 1);
				if (firstSpace == std::string::npos ||
					secondSpace == std::string::npos ||
					thirdSpace == std::string::npos ||
					fourthSpace == std::string::npos)
					return;

				// Extract coordinates
				s32 cx = std::stoi(token.substr(firstSpace + 1, secondSpace - firstSpace - 1));
				s32 cy = std::stoi(token.substr(secondSpace + 1, thirdSpace - secondSpace - 1));
				u16 world = std::stoi(token.substr(thirdSpace + 1, fourthSpace - thirdSpace - 1));

				// The rest is block data
				std::string strData = token.substr(fourthSpace + 1);
				std::istringstream dataStream(strData);

				s32 x = 0, y = 0;
				std::string cell;

				// Parse comma-separated block values
				while (std::getline(dataStream, cell, ',') && y < CHUNK_SIZE) {
					if (!cell.empty()) {
						try {
							u16 block = stoi(cell);
							u16* ptr = setBlock(world, cx * CHUNK_SIZE + x, cy * CHUNK_SIZE + y);
							if (ptr) *ptr = block;
						} catch (...) {
							// handle invalid number safely
						}
					}
					x++;
					if (x >= CHUNK_SIZE) { x = 0; y++; }
				}

            // --- VOXEL message ---
            } else if (token.rfind("VOXEL", 0) == 0) {
                size_t first  = token.find(' ', 6);
                size_t second = token.find(' ', first + 1);
                size_t third  = token.find(' ', second + 1);
                if (first != string::npos && second != string::npos) {
                    s32 vx = stoi(token.substr(6, first-6));
                    s32 vy = stoi(token.substr(first+1, second-first-1));
                    u16 world = stoi(token.substr(second+1, third-second-1));
                    u16 block = stoi(token.substr(third + 1));
                    u16* ptr = setBlock(world, vx, vy);
                    if(ptr) *ptr = block;
                }

            // --- Player update ---
            /*} else {
                size_t colon = token.find(":");
                size_t comma = token.find(",");
                if (colon != string::npos && comma != string::npos) {
                    int id = stoi(token.substr(0, colon));
                    float x = stof(token.substr(colon+1, comma-colon-1));
                    float y = stof(token.substr(comma+1));
                    players[id] = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
                }*/
            }
        }
    }
}

// --- Draw world ---
void drawWorld(sf::RenderWindow &window) {
    lock_guard<mutex> lock(mtx);

	s32 tx = (((-worldMap->x*CHUNK_SIZE)+px)*TILE_SIZE) - screenXhalf;
	s32 ty = (((-worldMap->y*CHUNK_SIZE)+py)*TILE_SIZE) - screenYhalf;
			sf::RectangleShape rect(sf::Vector2f(TILE_SIZE*CHUNK_SIZE*worldMapX, TILE_SIZE*CHUNK_SIZE*worldMapY));
			rect.setPosition(0 - tx, 0 - ty);
			rect.setFillColor(sf::Color(0, 0, 0));
			window.draw(rect);
    for (s32 cx = 0; cx < worldMapX; cx++) {
        for (s32 cy = 0; cy < worldMapY; cy++) {
            chunk_t* chunk = worldMap->chunk[cx][cy];
            chunk_t* BG = worldBG->chunk[cx][cy];
            for (s32 y = 0; y < CHUNK_SIZE; y++) {
                for (s32 x = 0; x < CHUNK_SIZE; x++) {
BGdescriptor[BG->chunk[y][x]%4](window, (cx*CHUNK_SIZE + x)*TILE_SIZE - tx, (cy*CHUNK_SIZE + y)*TILE_SIZE - ty);
descriptor[chunk->chunk[y][x]%16](window, (cx*CHUNK_SIZE + x)*TILE_SIZE - tx, (cy*CHUNK_SIZE + y)*TILE_SIZE - ty);
                }
            }
        }
    }
	//cout<<worldMap->x<<' '<<worldMap->y<<'\n'<<px<<' '<<py<<'\n';

    // Draw players
    //for (auto &[id, pos] : players) {
    //    sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    //    rect.setPosition(pos.x - px, pos.y - py);
    //    rect.setFillColor(sf::Color::Red);
    //    window.draw(rect);
    //}
    drawPlayer(window, prot);
}
