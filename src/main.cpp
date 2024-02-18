
#include <tgbot/tgbot.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <format>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <pqxx/pqxx>
#include <unordered_map>

#include "util.h"

const size_t kMaxQueries = 50;

class Command : public TgBot::BotCommand {
public:
    Command(std::initializer_list<std::string> args) {
        this->command = *args.begin();
        this->description = *std::next(args.begin());
    }
};

bool HandleUser(size_t id, std::unordered_map<size_t, size_t>& users) {
    ++users[id];
    return users[id] > kMaxQueries;
}

std::vector<std::string> GetArguments(std::string message) {
    std::vector<std::string> tokens;
    boost::split(tokens, message, boost::is_any_of(" "));
    return tokens;
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

void InitDatabase() {
    pqxx::connection c;
    std::cout << GetCurrentTime() << " | Connected to " << c.dbname() << " " << c.hostname() << " "
              << c.port() << '\n';

    auto ddl_queries = ReadDDLQueries("./cw.sql");

    for (const auto& ddl_query : ddl_queries) {
        pqxx::work query(c);
        try {
            // std::cerr << ddl_query << std::endl;
            pqxx::result res = query.exec(ddl_query);
            query.commit();
        } catch (...) {
            std::cout << "Database: failed to execute query: " << ddl_query << std::endl;
        }
    }
}

int main() {
    TgBot::Bot bot(std::getenv("TOKEN"));
    bool is_alive = true;

    InitDatabase();

    std::vector<Command> bot_commands = {
        {"start", "User greetings"},
        {"kill", "Kills bot if you have permission"},
        {"help", "Commands info"},
        {"get_friend_email", "Get email of friends by name"},
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

    std::unordered_map<size_t, size_t> users{};

    bot.getEvents().onAnyMessage([&bot, &bot_commands, &users](TgBot::Message::Ptr message) {
        if (HandleUser(message->chat->id, users)) {
            return;
        }
        std::cerr << GetCurrentTime()
                  << std::format(" | {} wrote {}\n", message->from->username, message->text);
        for (const auto& command : bot_commands) {
            if (StringTools::startsWith(message->text, "/" + command.command)) {
                return;
            }
        }
        bot.getApi().sendMessage(message->chat->id, "Unknown command: " + message->text);
    });

    bot.getEvents().onCommand("start", [&bot, &users](TgBot::Message::Ptr message) {
        if (HandleUser(message->chat->id, users)) {
            return;
        }
        if (IsAdmin(message->from->username)) {
            bot.getApi().sendMessage(message->chat->id,
                                     std::format("Hello master {}!", message->from->firstName));

        } else {
            bot.getApi().sendMessage(message->chat->id,
                                     std::format("Hello padavan {}!", message->from->firstName));
        }
    });
    bot.getEvents().onCommand("kill", [&bot, &is_alive, &users](TgBot::Message::Ptr message) {
        if (HandleUser(message->chat->id, users)) {
            return;
        }
        if (IsAdmin(message->from->username)) {
            bot.getApi().sendMessage(message->chat->id,
                                     "Change the world my final message. Goodbye.......");
            std::cerr << GetCurrentTime() << " | Killed\n";
            is_alive = false;
        } else {
            bot.getApi().sendMessage(message->chat->id, "Permission denied");
        }
    });
    bot.getEvents().onCommand("help", [&bot, &bot_commands, &users](TgBot::Message::Ptr message) {
        if (HandleUser(message->chat->id, users)) {
            return;
        }

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
    bot.getEvents().onCommand(
        "get_friend_email", [&bot, &bot_commands, &users](TgBot::Message::Ptr message) {
            if (HandleUser(message->chat->id, users)) {
                return;
            }
            std::string result;
            auto args = GetArguments(message->text);

            pqxx::connection conn;
            pqxx::work query(conn);
            try {
                pqxx::result res = query.exec(std::format(""));
                query.commit();
            } catch (...) {
                std::cout << "Database: failed to execute query: " << message->text << std::endl;
            }
            bot.getApi().sendMessage(message->chat->id, std::move(result));
        });

    bot.getEvents().onCommand(
        "add_crop", [&bot, &bot_commands, &users](TgBot::Message::Ptr message) {
            if (HandleUser(message->chat->id, users)) {
                return;
            }
            std::string result;
            auto args = GetArguments(message->text);

            pqxx::connection conn;
            pqxx::work query(conn);
            try {

                auto resp = std::format(
                    "insert into crops (name, humidity, brightness, is_legal, "
                    "brain_damage) values('{}', {}, {}, {}, {});",
                    args[1], args[2], args[3], args[4], args[5]);
                pqxx::result res = query.exec(resp);
                query.commit();
                bot.getApi().sendMessage(message->chat->id, resp);
            } catch (...) {
                std::cout << "Database: failed to execute query: " << message->text << std::endl;
            }
        });

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
                std::exit(0);
            }
        }

    } catch (TgBot::TgException& e) {
        std::cerr << GetCurrentTime() << std::format(" | error: {}\n", e.what());
    } catch (...) {
        std::cerr << GetCurrentTime() << "Unknown exception";
    }
    return 0;
}