#include <tgbot/tgbot.h>

#include <format>
#include <iostream>

const std::string GetCurrentTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

bool IsAdmin(const std::string& username) {
    std::vector<std::string> admins = {"hyperb0rean", "mopstream"};
    for (const auto& admin : admins) {
        if (username == admin) {
            return true;
        }
    }
    return false;
}

int main() {
    std::vector<std::string> bot_commands = {"start", "kill"};
    bool is_alive = true;
    TgBot::Bot bot(std::getenv("TOKEN"));
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
        }
    });
    bot.getEvents().onAnyMessage([&bot, &bot_commands](TgBot::Message::Ptr message) {
        std::cerr << GetCurrentTime()
                  << std::format(" | {} wrote {}\n", message->from->username, message->text);
        for (const auto& command : bot_commands) {
            if (StringTools::startsWith(message->text, "/" + command)) {
                return;
            }
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    signal(SIGINT, [](int s) {
        std::cerr << GetCurrentTime() << "SIGINT got\n";
        std::exit(0);
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