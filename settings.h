#ifndef LOCAL_CHAT_SETTINGS_H
#define LOCAL_CHAT_SETTINGS_H

#include "global_variables.h"
#include "style.h"

struct user_settings {
    enum admin_statuses {NotAdmin, Unverified, Strong, Weak};
    int soc = -1;
    std::string nick = "";
    bool input_some = true;
    bool not_nicked = true;
    bool not_colored = false;
    bool print_nicks = true;
    bool ban = false;
    admin_statuses admin_status = NotAdmin;
    std::string nick_color = style::colors_for_nick[Random() % style::colors_for_nick.size()];
    std::string nick_style = "";
    std::string mess_color = "";

    std::string full_nick() {
        return nick_color + nick_style + nick + style::normal;
    }

};

namespace chat_settings {
    bool join_open = true;
    bool logs_attempts = true;
}

std::unordered_map<int, user_settings> clients;

#endif
