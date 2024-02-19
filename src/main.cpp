#include "main.hpp"

const size_t kMaxQueries = 50;
using UserVault = std::unordered_map<size_t, size_t>;

class Command : public TgBot::BotCommand {
public:
    Command(std::initializer_list<std::string> args) {
        this->command = *args.begin();
        this->description = *std::next(args.begin());
    }
};

bool HandleUser(size_t id, UserVault& users) {
    ++users[id];
    return users[id] > kMaxQueries;
}

std::vector<std::string> ReadDDLQueries(std::string filename) {
    std::ifstream file(filename);
    std::vector<std::string> result;
    std::string line;
    std::string query;
    while (std::getline(file, line)) {
        if (line == "") {
            result.emplace_back(query);
            query.clear();
        } else {
            query.append(line);
            query.append("\n");
        }
    }
    result.emplace_back(query);
    query.clear();
    return result;
}

void InitDatabase(pqxx::connection& connection) {

    auto ddl_queries = ReadDDLQueries("./cw.sql");

    for (const auto& ddl_query : ddl_queries) {
        pqxx::work query(connection);
        try {
            pqxx::result res = query.exec(ddl_query);
            query.commit();
        } catch (std::exception& e) {
            std::cout << "Database: failed to execute query: " << ddl_query << std::endl
                      << " Error: " << e.what() << std::endl;
        }
    }
    std::cout << GetCurrentTime() << " | Successfully created database" << std::endl;
}

std::vector<std::string> GetArguments(std::string message) {
    std::vector<std::string> tokens{};
    boost::split(tokens, message, boost::is_any_of(" "));
    return tokens;
}

std::string GetPrettyTable(const pqxx::result& query_result) {
    std::string answer;
    answer.append(" ");
    for (pqxx::row_size_type column = 0; column < query_result.columns(); ++column) {
        answer.append(query_result.column_name(column)).append(" ");
    }
    answer.append("\n");

    for (auto row : query_result) {
        answer.append(" ");
        for (auto field : row) {
            answer.append(field.c_str()).append(" ");
        }
        answer.append("\n");
    }
    return answer;
}

void PrepareCommands(pqxx::connection& connection) {
    connection.prepare("get_friends_email", "select get_friends_email($1)");
    connection.prepare("enum_crops_on_plantation", "select enum_crops_on_plantation($1)");
    connection.prepare("sell", "select sell($1, $2, $3)");

    connection.prepare("add_crop",
                       "insert into crops (name, humidity, brightness, is_legal, "
                       "brain_damage) values($1, $2, $3, $4, $5);");

    connection.prepare("add_crop",
                       "insert into crops (name, humidity, brightness, is_legal, "
                       "brain_damage) values($1, $2, $3, $4, $5);");
    connection.prepare("view_crops", "select * from crops;");

    connection.prepare("add_client",
                       "insert into client (manager_id, name, debt, is_highly_addicted, "
                       "brain_resource) values($1, $2, 0, false, 100);");
    connection.prepare("view_clients", "select * from client;");

    connection.prepare(
        "add_client_family_member",
        "insert into client_family_member (client_id, adress, email) values($1, $2, $3);");
    connection.prepare("view_client_family_members", "select * from client_family_member;");

    connection.prepare("add_estate",
                       "insert into estate (name, area, county, rent) values($1, $2, $3, $4);");
    connection.prepare("view_estates", "select * from estate;");

    connection.prepare("add_plantation",
                       "insert into plantation (capacity, revenue, estate_id) values($1, $2, $3);");
    connection.prepare("view_plantations", "select * from plantation;");

    connection.prepare(
        "add_worker",
        "insert into worker (plantation_id, name, salary, profession) values($1, $2, $3, $4);");
    connection.prepare("view_workers", "select * from worker;");

    connection.prepare("add_landlord",
                       "insert into landlord (estate_id, name, capital) values($1, $2, $3);");
    connection.prepare("view_landlords", "select * from landlord;");

    connection.prepare("add_manager",
                       "insert into manager (plantation_id, name, salary) values($1, $2, $3);");
    connection.prepare("view_managers", "select * from manager;");

    connection.prepare(
        "add_evidence_info",
        "insert into evidence_info (landlord_id, description, worth) values($1, $2, $3);");
    connection.prepare("view_evidence_info", "select * from evidence_info;");

    connection.prepare(
        "add_crops_plantations",
        "insert into crops_plantations (crop_id, plantation_id, counter) values($1, $2, $3);");
    connection.prepare("view_crops_plantations", "select * from crops_plantations;");

    connection.prepare("add_worker_family_member",
                       "insert into worker_family_member (worker_id, name, relationship, adress) "
                       "values($1, $2, $3, $4);");
    connection.prepare("view_worker_family_members", "select * from worker_family_member;");
}

pqxx::result ExecutePrepared(pqxx::work& query, std::string command,
                             std::vector<std::string>&& args) {

    if (command == "sell") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "enum_crops_on_plantation") {
        if (args.size() < 2) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1]);
    } else if (command == "get_friends_email") {
        if (args.size() < 2) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1]);
    } else if (command == "add_crop") {
        if (args.size() < 6) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3], args[4], args[5]);
    } else if (command == "add_client") {
        if (args.size() < 3) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2]);
    } else if (command == "add_client_family_member") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_estate") {
        if (args.size() < 5) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3], args[4]);
    } else if (command == "add_plantation") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_worker") {
        if (args.size() < 5) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3], args[4]);
    } else if (command == "add_landlord") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_manager") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_evidence_info") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_crops_plantations") {
        if (args.size() < 4) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3]);
    } else if (command == "add_worker_family_member") {
        if (args.size() < 5) {
            throw std::runtime_error("Invalid number of arguments");
        }
        return query.exec_prepared(command, args[1], args[2], args[3], args[4]);
    } else if (StringTools::startsWith(command, "view")) {
        return query.exec_prepared(command);
    }

    return pqxx::result{};
}

void HandleCommands(TgBot::Bot& bot, pqxx::connection& connection,
                    const std::vector<Command>& bot_commands, UserVault& users, bool* is_alive) {

    bot.getEvents().onAnyMessage([&bot, &bot_commands, &users](TgBot::Message::Ptr message) {
        if (HandleUser(message->chat->id, users)) {
            return;
        }
        std::cout << GetCurrentTime()
                  << std::format(" | {} wrote {}\n", message->from->username, message->text);
        for (const auto& command : bot_commands) {
            if (StringTools::startsWith(message->text, "/" + command.command)) {
                return;
            }
        }
        bot.getApi().sendMessage(message->chat->id, "Unknown command: " + message->text);
    });

    for (auto& command : bot_commands) {
        if (command.command == "start") {
            bot.getEvents().onCommand(command.command, [&bot](TgBot::Message::Ptr message) {
                if (IsAdmin(message->from->username)) {
                    bot.getApi().sendMessage(
                        message->chat->id,
                        std::format("Hello master {}!", message->from->firstName));

                } else {
                    bot.getApi().sendMessage(
                        message->chat->id,
                        std::format("Hello padavan {}!", message->from->firstName));
                }
            });
        } else if (command.command == "kill") {
            bot.getEvents().onCommand(
                command.command, [&bot, is_alive](TgBot::Message::Ptr message) {
                    if (IsAdmin(message->from->username)) {
                        bot.getApi().sendMessage(
                            message->chat->id, "Change the world my final message. Goodbye.......");
                        std::cout << GetCurrentTime() << " | Killed\n";
                        *is_alive = false;
                    } else {
                        bot.getApi().sendMessage(message->chat->id, "Permission denied");
                    }
                });
        } else if (command.command == "help") {
            bot.getEvents().onCommand(
                command.command, [&bot, &bot_commands](TgBot::Message::Ptr message) {
                    std::string result;
                    for (const auto& command : bot_commands) {
                        result.append(command.command)
                            .append(" - ")
                            .append(command.description)
                            .append("\n\n");
                    }
                    bot.getApi().sendMessage(message->chat->id, std::move(result));
                });
        } else {
            bot.getEvents().onCommand(command.command, [command, &bot, &connection, &bot_commands](
                                                           TgBot::Message::Ptr message) {
                std::string result;
                pqxx::work query(connection);
                try {
                    pqxx::result query_result =
                        ExecutePrepared(query, command.command, GetArguments(message->text));
                    query.commit();
                    if (StringTools::startsWith(command.command, "add")) {
                        bot.getApi().sendMessage(message->chat->id, "Successfully added");
                    } else if (StringTools::startsWith(command.command, "view")) {
                        bot.getApi().sendMessage(message->chat->id, GetPrettyTable(query_result));
                    } else if (command.command == "sell") {
                        bot.getApi().sendMessage(message->chat->id, "Sold some sweat crops!");
                    } else if (command.command == "enum_crops_on_plantation") {
                        // for (auto& res : query_result) {
                        //     std::cout << GetCurrentTime() << " | " << res.c_str();
                        //     bot.getApi().sendMessage(message->chat->id, res.c_str());
                        // }
                        bot.getApi().sendMessage(message->chat->id, GetPrettyTable(query_result));

                    } else if (command.command == "get_friends_email") {
                        // for (auto& res : query_result) {
                        //     std::cout << GetCurrentTime() << " | " << res.c_str();
                        //     bot.getApi().sendMessage(message->chat->id, res.c_str());
                        // }
                        bot.getApi().sendMessage(message->chat->id, GetPrettyTable(query_result));
                    }
                } catch (std::exception& e) {
                    std::cout << "Database: failed to execute query: " << message->text
                              << " due to " << e.what() << std::endl;

                    bot.getApi().sendMessage(message->chat->id,
                                             "Error occured " + std::string{e.what()} + "\n");
                }
            });
        }
    }
}

int main() {
    TgBot::Bot bot(std::getenv("TOKEN"));
    bool is_alive = true;

    pqxx::connection connection;
    std::cout << GetCurrentTime() << " | Connected to " << connection.dbname() << " "
              << connection.hostname() << " " << connection.port() << '\n';

    InitDatabase(connection);

    std::vector<Command> bot_commands = {
        {"start", "User greetings"},
        {"kill", "Kills bot if you have permission"},
        {"help", "Commands info"},
        {"view_crops", "Check all current crops"},
        {"sell",
         "Sells certain amount of crop to client. Arguments: (id_client: int,id_crop: int, amount: "
         "int)"},
        {"enum_crops_on_plantation",
         "Enums certain crop on plantation by name. Arguments: (crop_name: string)"},
        {"get_friends_email",
         "Returns emails of friends of client. Arguments: (client_name: string)"},
        {"add_crop",
         "Add new crop into database. Arguments: (name: string, humidity: percent, brightness: "
         "uint, is_legal: bool, brain_damage: percent)"},
        {"view_clients", "Check all current clients"},
        {"add_client", "Add new client into database. Arguments: (manager_id: int, name: string)"},
        {"view_client_family_members", "Check all current client's family members"},
        {"add_client_family_member",
         "Add new client family member into database. Arguments: (client_id: int, adress: "
         "string, email: string)"},
        {"view_estates", "Check all current estates"},
        {"add_estate",
         "Add new estate into database. Arguments: (name: string, area: uint, country: string, "
         "rent: real)"},
        {"view_crops_plantations", "Check all crops related to plantations"},
        {"add_crops_plantations",
         "Add new crops related to plantations into database. Arguments: (crops_id: int, "
         "plantation_id: int, "
         "counter: "
         "uint)"},
        {"view_evidence_info", "Check all current evidence_info"},
        {"add_evidence_info",
         "Add new evidence_info into database. Arguments: (landlord_id: int, description: "
         "string, worth: percent)"},
        {"view_landlords", "Check all current landlords"},
        {"add_landlord",
         "Add new landlord into database. Arguments: (estate_id: int, name: string, capital: "
         "real)"},
        {"view_managers", "Check all current managers"},
        {"add_manager",
         "Add new manager into database. Arguments: (plantation_id: int, name:string, salary: "
         "real)"},
        {"view_plantations", "Check all current plantations"},
        {"add_plantation",
         "Add new plantation into database. Arguments: (capacity: uint, revenue: real, "
         "estate_id: int)"},
        {"view_workers", "Check all current workers"},
        {"add_worker",
         "Add new worker into database. Arguments: (plantation_id: int, name: string, salary: "
         "real, profession: string)"},
        {"view_worker_family_members", "Check all current worker's family members"},
        {"add_worker_family_member",
         "Add new worker family member into database. Argument: (worker_id: int, name: string, "
         "relationship: string, adress: string)"}};

    UserVault users{};

    PrepareCommands(connection);

    HandleCommands(bot, connection, bot_commands, users, &is_alive);

    try {
        std::cerr << GetCurrentTime()
                  << std::format(" | Bot username: {}\n", bot.getApi().getMe()->username);

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            std::cerr << GetCurrentTime() << " | Long poll started" << std::endl;
            longPoll.start();
            if (!is_alive) {
                for (auto& [id, count] : users) {
                    bot.getApi().sendMessage(id, "Guys, I've been killed(");
                }
                return 0;
            }
        }

    } catch (TgBot::TgException& e) {
        std::cerr << GetCurrentTime() << std::format(" | error: {}\n", e.what());
    } catch (...) {
        std::cerr << GetCurrentTime() << "Unknown exception";
    }
    return 0;
}