#include <sqlite3.h>

using namespace std;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t  s32;


const s32 worldMapM = ((u16)-1)+1;
const s32 seed = 54002;


// -------------------------------------------------
// Generate new world if DB is empty
// -------------------------------------------------
void generateWorldIfEmpty(sqlite3 *db) {
    int count = 0;
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM voxel;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    if (count > 0) {
        cout << "Loaded existing world from DB (" << count << " voxels).\n";
        return;
    }

    cout << "Generating new world...\n";
    //const vector<string> blocks = {"grass", "dirt", "stone", "sand", "water"};

    /*sqlite3_exec(db, "BEGIN TRANSACTION
;", nullptr, nullptr, nullptr);
    for (s32 y = 0; y < worldMapM; y++) {
        for (s32 x = 0; x < worldMapM; x++) {
            string block = (y + rand() % 2 < 8)
                                    ? "air"
                                    : blocks[rand() % blocks.size()];
            string sql =
                "INSERT INTO voxel (x, y, block) VALUES (" +
                to_string(x) + "," + to_string(y) + ",'" + block + "');";
            sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        }
        cout << y << ' ' << worldMapM << '\n';
    }*/
    
	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	u8 h = rand()%64+96;
	for (s32 x = 0; x < worldMapM; x++) {
		// start fresh for each row
		std::string sql = "INSERT INTO voxel (x, y, world, block) VALUES ";
		bool first = true;
		bool grass = true;

		// determine block type for this row
		u16 block = 0;
		{
			u8 t = rand();
			h = (15*h+t)/16;
		}

		for (s32 y = 0; y < worldMapM; y++) {

			if (h/6 < y-25) block = 3;
			else if (h/4 < y-10) {
				if (grass) {
					grass = false;
					block = 1;
				} else {
					block = 2;
				}
			}
			for (s32 world = 0; world < 5; world++) {
				if (!first) sql += ",";
				first = false;
				sql += "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(world) + "," + std::to_string(block) + ")";
			}
		}

		sql += ";"; // close the INSERT
		sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

		//std::cout << sql.c_str() << '\n';
		std::cout << x << '/' << worldMapM << '\n';
	}

	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);


    
    sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);
    cout << "World generated.\n";
}
