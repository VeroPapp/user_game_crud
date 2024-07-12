#include <iostream>
#include <sqlite3.h>
#include <string>

using namespace std;

// Clase para manejar la base de datos
class Database {
public:
    Database(const string &db_name);
    ~Database();
    bool execute(const string &query);
    sqlite3* getDB() const { return db; }

private:
    sqlite3 *db;
};

Database::Database(const string &db_name) {
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::execute(const string &query) {
    char *errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Funciones para realizar operaciones CRUD
void createTables(Database &db) {
    string createUsersTable = "CREATE TABLE IF NOT EXISTS Users ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                   "name TEXT NOT NULL, "
                                   "email TEXT NOT NULL UNIQUE);";
    
    string createGamesTable = "CREATE TABLE IF NOT EXISTS Games ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                   "title TEXT NOT NULL UNIQUE, "
                                   "genre TEXT NOT NULL);";
    
    string createUsersGamesTable = "CREATE TABLE IF NOT EXISTS Users_Games ("
                                        "user_id INTEGER, "
                                        "game_id INTEGER, "
                                        "FOREIGN KEY(user_id) REFERENCES Users(id), "
                                        "FOREIGN KEY(game_id) REFERENCES Games(id));";

    if (db.execute(createUsersTable) &&
        db.execute(createGamesTable) &&
        db.execute(createUsersGamesTable)) {
        cout << "Tables created successfully." << endl;
    } else {
        cout << "Error creating tables." << endl;
    }
}

void insertUser(Database &db, const string &name, const string &email) {
    string query = "INSERT INTO Users (name, email) VALUES ('" + name + "', '" + email + "');";
    if (db.execute(query)) {
        cout << "User added successfully." << endl;
    } else {
        cout << "Error adding user." << endl;
    }
}

void insertGame(Database &db, const string &title, const string &genre) {
    string query = "INSERT INTO Games (title, genre) VALUES ('" + title + "', '" + genre + "');";
    if (db.execute(query)) {
        cout << "Game added successfully." << endl;
    } else {
        cout << "Error adding game." << endl;
    }
}

void listUsers(Database &db) {
    string query = "SELECT * FROM Users;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            const unsigned char *email = sqlite3_column_text(stmt, 2);
            cout << "ID: " << id << ", Name: " << name << ", Email: " << email << endl;
        }
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing users." << endl;
    }
}

void listGames(Database &db) {
    string query = "SELECT * FROM Games;";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *title = sqlite3_column_text(stmt, 1);
            const unsigned char *genre = sqlite3_column_text(stmt, 2);
            cout << "ID: " << id << ", Title: " << title << ", Genre: " << genre << endl;
        }
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error listing games." << endl;
    }
}

bool associateUserGame(Database &db, int userId, int gameId) {
    string query = "INSERT INTO Users_Games (user_id, game_id) VALUES (" +
                        to_string(userId) + ", " + std::to_string(gameId) + ");";
    return db.execute(query);
}

bool listGamesByUser(Database &db, int userId) {
    string query = "SELECT Users.name, Games.title, Games.genre "
                        "FROM Users "
                        "JOIN Users_Games ON Users.id = Users_Games.user_id "
                        "JOIN Games ON Games.id = Users_Games.game_id "
                        "WHERE Users.id = " + to_string(userId) + ";";
    char *errMsg = nullptr;

    auto callback = [](void*, int argc, char **argv, char **colName) -> int {
        for (int i = 0; i < argc; i++) {
            cout << colName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
        }
        cout << endl;
        return 0;
    };

    if (sqlite3_exec(db.getDB(), query.c_str(), callback, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool listUsersByGame(Database &db, int gameId) {
    string query = "SELECT Games.title, Users.name, Users.email "
                        "FROM Games "
                        "JOIN Users_Games ON Games.id = Users_Games.game_id "
                        "JOIN Users ON Users.id = Users_Games.user_id "
                        "WHERE Games.id = " + to_string(gameId) + ";";
    char *errMsg = nullptr;

    auto callback = [](void*, int argc, char **argv, char **colName) -> int {
        for (int i = 0; i < argc; i++) {
            cout << colName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
        }
        cout << endl;
        return 0;
    };

    if (sqlite3_exec(db.getDB(), query.c_str(), callback, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int main() {
    Database db("user_game.db");

    createTables(db);

    int option;
    do {
        cout << "\nMenu:\n";
        cout << "1. Add User\n";
        cout << "2. Add Game\n";
        cout << "3. List Users\n";
        cout << "4. List Games\n";
        cout << "5. Associate User with Game\n";
        cout << "6. List Games by User\n";
        cout << "7. List Users by Game\n";
        cout << "0. Exit\n";
        cout << "Choose an option: ";
        cin >> option;
        cin.ignore();

        switch (option) {
            case 1: {
                string name, email;
                cout << "\nEnter name: ";
                getline(cin, name);
                cout << "Enter email: ";
                getline(cin, email);
                insertUser(db, name, email);
                break;
            }
            case 2: {
                string title, genre;
                cout << "\nEnter title: ";
                getline(cin, title);
                cout << "Enter genre: ";
                getline(cin, genre);
                insertGame(db, title, genre);
                break;
            }
            case 3:
                listUsers(db);
                break;
            case 4:
                listGames(db);
                break;
            case 5: {
                int userId, gameId;
                cout << "\nEnter User ID: ";
                cin >> userId;
                cout << "Enter Game ID: ";
                cin >> gameId;
                if (associateUserGame(db, userId, gameId)) {
                    cout << "User associated with game successfully." << endl;
                } else {
                    cout << "Error associating user with game." << endl;
                }
                break;
            }
            case 6: {
                int userId;
                cout << "\nEnter User ID: ";
                cin >> userId;
                if (!listGamesByUser(db, userId)) {
                    cout << "Error listing games for user." << endl;
                }
                break;
            }
            case 7: {
                int gameId;
                cout << "\nEnter Game ID: ";
                cin >> gameId;
                if (!listUsersByGame(db, gameId)) {
                    cout << "Error listing users for game." << endl;
                }
                break;
            }
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid option. Please try again.\n";
        }
    } while (option != 0);

    return 0;
}
