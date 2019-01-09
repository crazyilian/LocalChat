#ifndef LOCAL_CHAT_USER_COMMANDS_H
#define LOCAL_CHAT_USER_COMMANDS_H

#include "global_variables.h"
#include "style.h"
#include "settings.h"

namespace user_commands {
    void print_now_nick(user_settings &usr) {
        send_command("Now, your nick is \"" + usr.full_nick() + style::brown + "\".", usr.soc);
    }
    bool escape_message(std::string &mess, user_settings &usr) {
        if (mess.find("\033") <= mess.size()) {
            send_command("You can`t write [escape] in message!", usr.soc);
            if (clients[usr.soc].not_nicked)
                send_command("Enter your nick: ", usr.soc, false);
            return true;
        }
        return false;
    }
    bool help(std::string &mess, user_settings &usr) {
        if (mess == "/help\n") {
            string text = R"(
Commands:
/help  -  print help for you
/change nick  -  change your nick.
/change color  -  change color of your nick to selected color.
/no nicks  -  disable showing nicks before message.
/show nicks  -  enable showing nicks before message.
/list nicks  -  print all nicks, which is online.
/list colors  -  print all colors, which is used for nicks.
)";
            send_command(text, usr.soc);
            return true;
        }
        return false;
    }
    bool no_show_nicks(string &mess, user_settings &usr) {
        if (mess == "/no nicks\n")
            usr.print_nicks = 0;
        else if (mess == "/show nicks\n")
            usr.print_nicks = 1;
        else
            return false;
        return true;
    }
    bool change_nick(string &mess, user_settings &usr) {
        if (mess == "/change nick\n") {
            count_input_users++;
            if (usr.admin_status != user_settings::NotAdmin)
                count_admin_users--;
            usr.nick = "";
            usr.not_nicked = true;
            usr.input_some = true;
            print_stat("Customize");
            send_command("Enter your nick: ", usr.soc, false);
            return true;
        }
        return false;
    }
    bool change_color(string &mess, user_settings &usr) {
        if (mess == "/change color\n") {
            count_input_users++;
            if (usr.admin_status != user_settings::NotAdmin)
                count_admin_users--;
            usr.not_colored = true;
            usr.input_some = true;
            send_command("Enter the number of color (1-" + to_string(style::colors_for_nick.size()) +
                         "): ", usr.soc, false);
            print_stat("Customize");
            return true;
        }
        return false;
    }
    bool enter_nick(string &mess, user_settings &usr) {
        if (usr.not_nicked) {
            mess.pop_back();
            bool is_taken = 0;
            for (int nsd : clients_soc) {
                if (mess == clients[nsd].nick) {
                    is_taken = 1;
                    break;
                }
            }
            if (mess == "ADMIN") {
                usr.admin_status = user_settings::Unverified;
                send_command("Enter password: " + style::hide, usr.soc, false);
                print_stat("Admin verifying");
            } else if (is_taken || 3 > mess.size() || mess.size() > 32 || mess.back() == ' ' ||
                       mess.front() == ' ' || find(mess.begin(), mess.end(), '\n') != mess.end() ||
                       mess.substr(0, 5) == "ADMIN")
            {
                send_command("This nick is illegal.\nEnter your nick: ", usr.soc, false);
            } else {
                usr.not_nicked = false;
                usr.input_some = false;
                count_input_users--;
                if (usr.admin_status != user_settings::NotAdmin)
                    count_admin_users++;
                usr.nick = mess;
                print_now_nick(usr);
                print_stat("Connect");
            }
            return true;
        }
        return false;
    }
    bool enter_color(string &mess, user_settings &usr) {
        if (usr.not_colored) {
            mess.pop_back();
            try {
                int clr = stoi(mess);
                if (clr <= 0 || clr > style::colors_for_nick.size())
                    throw std::exception();
                usr.nick_color = style::colors_for_nick[clr - 1];
            } catch (std::exception e) {
                usr.nick_color = style::colors_for_nick[Random() % style::colors_for_nick.size()];
            }
            count_input_users--;
            if (usr.admin_status != user_settings::NotAdmin)
                count_admin_users++;
            usr.not_colored = false;
            usr.input_some = false;
            print_now_nick(usr);
            print_stat("Connect");
            return true;
        }
        return false;
    }
    bool admin_verification(string &mess, user_settings &usr) {
        if (usr.admin_status == user_settings::Unverified) {
            mess.pop_back();
            if (mess == "") {
                usr.admin_status = user_settings::NotAdmin;
                send_command("Enter your nick: ", usr.soc, false);
            } else if (string_hash(to_string(string_hash(mess))) == 2262243242197657067) {
                usr.admin_status = user_settings::Strong;
                strong_admins.insert(usr.soc);
                usr.nick = "ADMIN";
                if (strong_admins.size() > 1)
                    usr.nick += " " + to_string(strong_admins.size());
                usr.nick_color = style::dark_red;
                usr.mess_color = style::light_yellow;
                usr.nick_style = style::super;
                usr.not_nicked = false;
                usr.input_some = false;
                count_input_users--;
                count_admin_users++;
                print_now_nick(usr);
                print_stat("Admin connected");
            } else {
                send_command("Wrong :(\nEnter password: " + style::hide, usr.soc, false);
            }
            return true;
        }
        return false;
    }
    bool list_nicks_colors(string &mess, user_settings &usr) {
        if (mess == "/list nicks\n") {
            string text = "List of nicks:\n";
            for (int nsd : clients_soc) {
                user_settings nusr = clients[nsd];
                if (usr.input_some)
                    continue;
                text += style::normal + nusr.nick_style + nusr.nick_color + nusr.nick;
                text += style::normal + style::brown + "  |||  ";
            }
            send_command(text, usr.soc);
        } else if (mess == "/list colors\n") {
            string text = "List of colors, which is used for nicks:\n";
            for (int i = 0; i < style::colors_for_nick.size(); i++) {
                text += style::colors_for_nick[i] + "color " + to_string(i + 1);
                text += style::brown + "  |||  ";
            }
            send_command(text, usr.soc);
        } else
            return false;
        return true;
    }
    bool send_messages(string &mess, user_settings &usr) {
        if (mess.front() != '/' || mess.substr(0, 2) == "//") {
            if (mess.substr(0, 2) == "//")
                mess = mess.substr(1, mess.size());
            for (int nsd : clients_soc) {
                user_settings nusr = clients[nsd];
                if (usr.soc == nsd || nusr.input_some)
                    continue;
                string mess_for_usr = mess;
                if (nusr.print_nicks)
                    mess_for_usr = usr.nick_color + usr.nick_style + usr.nick + style::normal + ":  " +
                                   usr.mess_color + mess + style::normal;
                send_mess(mess_for_usr, nsd);
            }
            return true;
        }
        return false;
    }
    bool wrong_command(string &mess, user_settings &usr, bool already_admin_command) {
        if (mess.front() == '/' && !already_admin_command) {
            send_command("Wrong command!", usr.soc);
            return true;
        }
        return false;
    }
    vector<bool(*)(string&, user_settings&)> list_commands = {escape_message, admin_verification, enter_nick,
                                                              enter_color, help, no_show_nicks, change_nick,
                                                              change_color, list_nicks_colors, send_messages};
    void exec(int sd, std::string mess, bool already_admin_command) {
        user_settings &usr = clients[sd];
        for (auto func : list_commands) {
            if (func(mess, usr))
                return;
        }
        wrong_command(mess, usr, already_admin_command);
    }
}

#endif
