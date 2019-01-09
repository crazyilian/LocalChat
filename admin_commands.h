#ifndef LOCAL_CHAT_ADMIN_COMMANDS_H
#define LOCAL_CHAT_ADMIN_COMMANDS_H

#include "global_variables.h"
#include "style.h"
#include "settings.h"
#include "user_commands.h"

namespace admin_commands {
    string str_sum(string s, int x) {
        return s + to_string(x);
    }
    string str_sum(string s, bool x) {
        if (x)
            return s + "true";
        else
            return s + "false";
    }
    bool is_stronger(user_settings &usr1, user_settings &usr2) {
        if (usr1.admin_status == user_settings::Strong)
            return usr2.admin_status != user_settings::Strong;
        else if (usr1.admin_status == user_settings::Weak)
            return usr2.admin_status == user_settings::NotAdmin || usr2.admin_status == user_settings::Unverified;
        return false;
    }
    void send_command_to_admin(string text, int sd) {
        send_command(text, sd, true, true);
        for (int admin_sd : strong_admins) {
            if (admin_sd != sd)
                send_command(text, admin_sd, true, true);
        }
    }
    bool help(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/help") {
            string text = R"(
Admin commands:
/show  -  show all [nick : socket]
/show admins  -  show all admins [nick : socket]
/info N  -  show all user N`s [user_settings]
/give N  -  give admin rights to user N
/take N  -  take admin rights from user N
/ban N  -  ban user N (user can`t send messages)
/unban N  -  unban user N
/kick N  -  kick user N from chat
/close  -  new users can`t connect to chat
/open  -  new users can connect to chat
/logs no att  -  disable showing "Join attempt" in logs
/logs yes att  -  enable showing "Join attempt" in logs
/super nick  -  make your nick cool
/normal nick  -  make your nick normal
)";
            send_command(text, admin_usr.soc, true, true);
            return true;
        }
        return false;
    }
    bool show(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/show") {
            bool only_admins = false;
            try { only_admins = command.substr(6, 6) == "admins"; }
            catch (std::exception e) {}
            string text;
            for (int sd : clients_soc) {
                if ((only_admins && (clients[sd].admin_status == user_settings::Strong ||
                                     clients[sd].admin_status == user_settings::Weak)) || (!only_admins))
                {
                    text += clients[sd].nick + str_sum(" : ", sd) + '\n';
                }
            }
            send_command('\n' + text, admin_usr.soc, true, true);
            return true;
        }
        return false;
    }
    bool info(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/info") {
            int sd = std::stoi(command.substr(6, 4));
            string text;
            try {
                if (clients_soc.find(sd) != clients_soc.end()) {
                    user_settings &usr = clients[sd];
                    text += "User settings:";
                    text += str_sum("\nsoc = ", usr.soc);
                    text += "\nnick = " + usr.nick;
                    text += str_sum("\ninput_some = ", usr.input_some);
                    text += str_sum("\nnot_nicked = ", usr.not_nicked);
                    text += str_sum("\nnot_colored = ", usr.not_colored);
                    text += str_sum("\nprint_nick = ", usr.print_nicks);
                    text += str_sum("\nban = ", usr.ban);
                    text += "\nadmin_status = ";
                    if (usr.admin_status == user_settings::Strong)
                        text += "Strong";
                    else if (usr.admin_status == user_settings::Weak)
                        text += "Weak";
                    else if (usr.admin_status == user_settings::Unverified)
                        text += "Unverified";
                    else
                        text += "NotAdmin";
                    text += "\nnick_color = " + style::no_color + usr.nick_color + "color" + style::normal + style::dark_yellow;
                    text += "\nnick_style = " + usr.nick_style + "style" + style::normal + style::dark_yellow;
                    text += "\nmess_color = " + style::no_color + usr.mess_color + "color" + style::dark_yellow;
                } else
                    throw std::exception();
            } catch (std::exception e) {
                text += "Chat settings:";
                text += str_sum("\njoin_open = ", chat_settings::join_open);
                text += str_sum("\nlogs_attempts = ", chat_settings::logs_attempts);
            }
            send_command('\n' + text + '\n', admin_usr.soc, true, true);
            return true;
        }
        return false;
    }
    bool ban_unban(string &command, user_settings &admin_usr) {
        if (command.substr(0, 4) == "/ban") {
            int sd = std::stoi(command.substr(5, 4));
            if (clients_soc.find(sd) == clients_soc.end())
                return false;
            user_settings &usr = clients[sd];
            if (!is_stronger(admin_usr, usr))
                return true;
            usr.ban = true;
            send_command("You were banned. You can`t send messages :(", sd);
            string text = "\"" + usr.nick + "\" is banned by \"" + admin_usr.nick + "\"";
            send_command_to_admin(text, admin_usr.soc);
        } else if (command.substr(0, 6) == "/unban") {
            int sd = std::stoi(command.substr(7, 4));
            if (clients_soc.find(sd) == clients_soc.end())
                return false;
            user_settings &usr = clients[sd];
            if (!is_stronger(admin_usr, usr))
                return true;
            usr.ban = false;
            send_command("You were unbanned :)", sd);
            string text = "\"" + usr.nick + "\" is unbanned by \"" + admin_usr.nick + "\"";
            send_command_to_admin(text, admin_usr.soc);
        } else
            return false;
        return true;
    }
    bool kick(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/kick") {
            int sd = std::stoi(command.substr(6, 4));
            if (clients_soc.find(sd) == clients_soc.end())
                return false;
            user_settings &usr = clients[sd];
            if (!is_stronger(admin_usr, usr))
                return true;
            send_command("You were kicked :(", sd);
            Disconnect(sd);
            string text = "\"" + usr.nick + "\" is kicked by \"" + admin_usr.nick + "\"";
            send_command_to_admin(text, admin_usr.soc);
            return true;
        }
        return false;
    }
    bool open_close_join(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/open") {
            chat_settings::join_open = true;
            string text = "Join were opened by \"" + admin_usr.nick + "\"";
            send_command_to_admin(text, admin_usr.soc);
        } else if (command.substr(0, 6) == "/close") {
            chat_settings::join_open = false;
            string text = "Join were closed by \"" + admin_usr.nick + "\"";
            send_command_to_admin(text, admin_usr.soc);
        } else
            return false;
        return true;
    }
    bool give_take_admin(string &command, user_settings &admin_usr) {
        if (admin_usr.admin_status == user_settings::Strong) {
            if (command.substr(0, 5) == "/give") {
                int sd = std::stoi(command.substr(6, 4));
                if (clients_soc.find(sd) == clients_soc.end())
                    return false;
                if (clients[sd].admin_status == user_settings::NotAdmin && !clients[sd].input_some) {
                    clients[sd].admin_status = user_settings::Weak;
                    count_admin_users++;
                    send_command("You became an admin! :)", sd);
                    string text = "\"" + clients[sd].nick + "\" got admin rights by \"" + admin_usr.nick + "\"";
                    send_command_to_admin(text, admin_usr.soc);
                    print_stat("Got admin");
                }
                return true;
            } else if (command.substr(0, 5) == "/take") {
                int sd = std::stoi(command.substr(6, 4));
                if (clients_soc.find(sd) == clients_soc.end())
                    return false;
                if (clients[sd].admin_status == user_settings::Weak) {
                    clients[sd].admin_status = user_settings::NotAdmin;
                    count_admin_users--;
                    clients[sd].nick_style = "";
                    send_command("You stopped being an admin! :(", sd);
                    string text = "\"" + clients[sd].nick + "\" lost admin rights by \"" + admin_usr.nick + "\"";
                    send_command_to_admin(text, admin_usr.soc);
                    print_stat("Lost admin");
                }
                return true;
            }
        }
        return false;
    }
    bool logs(string &command, user_settings &admin_usr) {
        if (command.substr(0, 5) == "/logs") {
            if (command.substr(6, 6) == "no att") {
                chat_settings::logs_attempts = false;
                string text = "\"Join attempt\" disabled to showing in logs by \"" + admin_usr.nick + "\"";
                send_command_to_admin(text, admin_usr.soc);
            } else if (command.substr(6, 7) == "yes att") {
                chat_settings::logs_attempts = true;
                string text = "\"Join attempt\" enabled to showing in logs by \"" + admin_usr.nick + "\"";
                send_command_to_admin(text, admin_usr.soc);
            } else
                throw std::exception();
            return true;
        }
        return false;
    }
    bool super_nick(string &command, user_settings &admin_usr) {
        if (command.substr(0, 11) == "/super nick") {
            admin_usr.nick_style = style::super;
            user_commands::print_now_nick(admin_usr);
        } else if (command.substr(0, 12) == "/normal nick") {
            admin_usr.nick_style = "";
            user_commands::print_now_nick(admin_usr);
        } else
            return false;
        return true;
    }
    const vector<bool(*)(string&, user_settings&)> list_commands = {help, show, info, ban_unban, kick, open_close_join,
                                                                    give_take_admin, logs, super_nick};
    bool exec(int sd, string mess) {
        mess.pop_back();
        user_settings &usr = clients[sd];
        try {
            for (auto func : list_commands) {
                if (func(mess, usr))
                    return true;
            }
            return false;
        } catch (std::exception e) {
            string text = style::red + "Wrong command!\n" + style::normal;
            send_command('\n' + text, usr.soc, true, true);
            return true;
        }
    }
}

#endif