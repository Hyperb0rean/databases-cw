
#include <tgbot/tgbot.h>

#include <format>
#include <initializer_list>
#include <iostream>

#include "ddl.h"
#include "util.h"

class Command : public TgBot::BotCommand {
public:
    Command(std::initializer_list<std::string> args) {
        this->command = *args.begin();
        this->description = *std::next(args.begin());
    }
};

int main() {
    TgBot::Bot bot(std::getenv("TOKEN"));
    bool is_alive = true;

    InitDatabase();

    std::vector<Command> bot_commands = {{"start", "User greetings"},
                                         {"kill", "Kills bot if you have permission"},
                                         {"help", "Commands info"}};

    bot.getEvents().onAnyMessage([&bot, &bot_commands](TgBot::Message::Ptr message) {
        std::cerr << GetCurrentTime()
                  << std::format(" | {} wrote {}\n", message->from->username, message->text);
        for (const auto& command : bot_commands) {
            if (StringTools::startsWith(message->text, "/" + command.command)) {
                return;
            }
        }
        bot.getApi().sendMessage(message->chat->id, "Unknown command: " + message->text);
    });

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        if (IsAdmin(message->from->username)) {
            bot.getApi().sendMessage(message->chat->id,
                                     std::format("Hello master {}!", message->from->username));

        } else {
            bot.getApi().sendMessage(message->chat->id, "Hello!");
        }
    });
    bot.getEvents().onCommand("kill", [&bot, &is_alive](TgBot::Message::Ptr message) {
        if (IsAdmin(message->from->username)) {
            bot.getApi().sendMessage(message->chat->id,
                                     "Change the world my final message. Goodbye.......");
            std::cerr << GetCurrentTime() << " | Killed\n";
            is_alive = false;
        } else {
            bot.getApi().sendMessage(message->chat->id, "Permission denied");
        }
    });
    bot.getEvents().onCommand("help", [&bot, &bot_commands](TgBot::Message::Ptr message) {
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
            std::cerr << GetCurrentTime() << " | Long poll started\n";
            longPoll.start();
            if (!is_alive) {
                std::exit(0);
            }
        }
    } catch (TgBot::TgException& e) {
        std::cerr << GetCurrentTime() << std::format(" | error: {}\n", e.what());
    }
    return 0;
}