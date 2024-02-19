#pragma once

#include <cassert>
#include <chrono>
#include <string>
#include <tuple>
#include <vector>

inline const std::string GetCurrentTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

inline bool IsAdmin(const std::string& username) {
    std::vector<std::string> admins = {"hyperb0rean", "mopstream"};
    for (const auto& admin : admins) {
        if (username == admin) {
            return true;
        }
    }
    return false;
}

// inline std::vector<std::string> GetArguments(std::string message) {
//     std::vector<std::string> tokens;
//     boost::split(tokens, message, boost::is_any_of(" "));
//     return tokens;
// }
