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
        } catch (...) {
            std::cout << "Database: failed to execute query: " << ddl_query << std::endl;
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
    connection.prepare("add_crop",
                       "insert into crops (name, humidity, brightness, is_legal, "
                       "brain_damage) values($1, $2, $3, $4, $5);");
    connection.prepare("view_crops", "select * from crops;");
}

pqxx::result ExecutePrepared(pqxx::work& query, std::string command,
                             std::vector<std::string>&& args) {
    if (command == "add_crop") {
        return query.exec_prepared(command, args[1], args[2], args[3], args[4], args[5]);
    } else if (command == "view_crops") {
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
                        result.append("- ")
                            .append(command.command)
                            .append(": ")
                            .append(command.description)
                            .append("\n");
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
        {"add_crop", "Add new crop into database (string, percent, uint, bool, percent)"},
        {"add_client", "Add new client into database"},
        {"add_client_family_member", "Add new client family member into database"},
        {"add_crops_plantation", "Add new crops plantation into database"},
        {"add_estate", "Add new estate into database"},
        {"add_evidence_info", "Add new evidence_info into database"},
        {"add_landlord", "Add new landlord into database"},
        {"add_manager", "Add new manager into database"},
        {"add_plantation", "Add new plantation into database"},
        {"add_worker", "Add new worker into database"},
        {"add_worker_family_member", "Add new worker family member into database"}};

    UserVault users{};

    PrepareCommands(connection);

    HandleCommands(bot, connection, bot_commands, users, &is_alive);

    try {
        std::cerr << GetCurrentTime()
                  << std::format(" | Bot username: {}\n", bot.getApi().getMe()->username);

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            std::cerr << GetCurrentTime() << " | Long poll started on thread " << 0 << "\n";
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