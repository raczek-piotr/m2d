#include <iostream>
#include <thread>
#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>
#include <cstdlib>
#include <vector>
#include <string>
#include "mapgen.cpp"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t  s32;

const int PORT = 54002;
const int CHUNK_SIZE = 8;

struct Player {
    int id;
    int sock;
    float x, y;
};

struct DBTask {
    s32 x, y, world;
    u16 block;
};

std::mutex mtx; // players mutex
std::map<u8, Player> players;

std::queue<DBTask> dbQueue;
std::mutex dbMutex;
std::condition_variable dbCV;

// -------------------------------------------------
// Send helpers
// -------------------------------------------------
void sendOne(int sock, const std::string &msg) {
    send(sock, msg.c_str(), msg.size(), 0);
}

void broadcast(const std::string &msg) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &[id, p] : players)
        sendOne(p.sock, msg);
}

// -------------------------------------------------
// Database background writer thread
// -------------------------------------------------
void dbWorker(sqlite3 *db) {
    while (true) {
        std::unique_lock<std::mutex> lock(dbMutex);
        dbCV.wait(lock, [] { return !dbQueue.empty(); });

        DBTask task = dbQueue.front();
        dbQueue.pop();
        lock.unlock();

        std::string sql =
            "INSERT OR REPLACE INTO voxel (x, y, world, block) VALUES (" +
            std::to_string(task.x) + "," + std::to_string(task.y) + "," +
			std::to_string(task.world) + "," + std::to_string(task.block) + ");";


        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQLite error: " << errMsg << "\n";
            sqlite3_free(errMsg);
        }
    }
}

// -------------------------------------------------
// Handle one client connection
// -------------------------------------------------
void handleClient(int sock, int playerId, sqlite3 *db) {
    char buffer[1024];
    std::string leftover;

    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
            break;

        buffer[bytes] = '\0';
        leftover += buffer;

        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string msg = leftover.substr(0, pos);
            leftover.erase(0, pos + 1);

            // ---- SET command ----
            if (msg.rfind("SET ", 0) == 0) {
                s32 vx, vy, world;
                u16 block;
                if (sscanf(msg.c_str(), "SET %d %d %d %hu", &vx, &vy, &world, &block) == 4) {
                    {
                        std::lock_guard<std::mutex> lock(dbMutex);
                        dbQueue.push({vx, vy, world, block});
                        dbCV.notify_one();
                    }
                    broadcast("VOXEL " + std::to_string(vx) + " " + std::to_string(vy) + " " + std::to_string(world) + " " + std::to_string(block) + ";");
                }
            }

            // ---- GETCHUNK command ----
            else if (msg.rfind("GETCHUNK ", 0) == 0) {
                s32 cx, cy;
					if (sscanf(msg.c_str(), "GETCHUNK %u %u", &cx, &cy) == 2) {
						for (u16 world = 0; world < 5; world++) {
						std::string chunkData;
						sqlite3_stmt *stmt;
						std::string sql =
							"SELECT x, y, block FROM voxel "
							"WHERE world = ?1 AND x BETWEEN ?2 AND ?3 AND y BETWEEN ?4 AND ?5;";

						if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
							s32 x1 = cx * CHUNK_SIZE;
							s32 x2 = x1 + CHUNK_SIZE - 1;
							s32 y1 = cy * CHUNK_SIZE;
							s32 y2 = y1 + CHUNK_SIZE - 1;

							sqlite3_bind_int(stmt, 1, world);
							sqlite3_bind_int(stmt, 2, x1);
							sqlite3_bind_int(stmt, 3, x2);
							sqlite3_bind_int(stmt, 4, y1);
							sqlite3_bind_int(stmt, 5, y2);

							u16 chunk[CHUNK_SIZE * CHUNK_SIZE];
							for (u16 &b : chunk)
								b = 0;

							while (sqlite3_step(stmt) == SQLITE_ROW) {
								s32 wx = sqlite3_column_int(stmt, 0);
								s32 wy = sqlite3_column_int(stmt, 1);
								u16 block = sqlite3_column_int(stmt, 2);
								//u16 block = text ? reinterpret_cast<const char *>(text) : 0;

								s32 localX = wx - x1;
								s32 localY = wy - y1;
								if (localX >= 0 && localX < CHUNK_SIZE &&
									localY >= 0 && localY < CHUNK_SIZE)
									chunk[localY * CHUNK_SIZE + localX] = block;
							}
							sqlite3_finalize(stmt);

							for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
								chunkData += std::to_string(chunk[i]);
								if (i < CHUNK_SIZE * CHUNK_SIZE - 1)
									chunkData += ",";
							}
							sendOne(sock, "CHUNK " + std::to_string(cx) + " " + std::to_string(cy) + " " + std::to_string(world) + " " + chunkData + ";");
						} else {
							std::cerr << "SQL error: " << sqlite3_errmsg(db) << "\n";
						}
					}
				}
            }
        }
    }

    close(sock);
    {
        std::lock_guard<std::mutex> lock(mtx);
        players.erase(playerId);
    }
    std::cout << "Player " << playerId << " disconnected.\n";
}

// -------------------------------------------------
// Main entry
// -------------------------------------------------
int main() {
    sqlite3 *db;
    if (sqlite3_open("worlds.db", &db) != SQLITE_OK) {
        std::cerr << "Cannot open DB\n";
        return 1;
    }

    sqlite3_exec(db,
                 "CREATE TABLE IF NOT EXISTS voxel (x INTEGER, y INTEGER, world INTEGER, block INTEGER, PRIMARY KEY(x,y,world));",
                 nullptr, nullptr, nullptr);

    generateWorldIfEmpty(db);
    std::thread(dbWorker, db).detach();

    int listening = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listening, (sockaddr *)&addr, sizeof(addr));
    listen(listening, 5);

    std::cout << "Server listening on port " << PORT << "...\n";
    int nextId = 1;

    while (true) {
        int clientSock = accept(listening, nullptr, nullptr);
        if (clientSock < 0)
            continue;

        {
            std::lock_guard<std::mutex> lock(mtx);
            players[nextId] = {nextId, clientSock, 5, 5};
        }

        std::thread(handleClient, clientSock, nextId, db).detach();
        std::cout << "Player " << nextId << " connected.\n";
        nextId++;
    }

    close(listening);
    sqlite3_close(db);
    return 0;
}
