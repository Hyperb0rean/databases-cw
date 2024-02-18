
#include <tgbot/tgbot.h>

#include <format>
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
    if (users.contains(id)) {
        ++users[id];
    } else {
        users[id] = 0;
    }
    return users[id] > kMaxQueries;
}

int main() {
    TgBot::Bot bot(std::getenv("TOKEN"));
    bool is_alive = true;

    while (true) {
        try {
            pqxx::connection c;
            std::cout << "Connected to " << c.dbname() << '\n';
            break;

        } catch (const std::exception& e) {
        }
    }

    std::vector<Command> bot_commands = {{"start", "User greetings"},
                                         {"kill", "Kills bot if you have permission"},
                                         {"help", "Commands info"}};

    std::unordered_map<size_t, size_t> users;

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
                                     std::format("Hello master {}!", message->from->username));

        } else {
            bot.getApi().sendMessage(message->chat->id, "Hello!");
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