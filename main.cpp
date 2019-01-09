#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random>
#include <exception>
#include <functional>

std::mt19937 Random(time(NULL));
std::hash<std::string> string_hash;

using std::vector;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::to_string;

namespace style {
    string rgb(unsigned char red, unsigned char green, unsigned char blue) {
        return "\033[38;2;" + to_string(red) + ";" + to_string(green) + ";" + to_string(blue) + "m";
    }
    const string red = rgb(255,0,0),      light_red = rgb(255,123,123),    dark_red = rgb(155,0,0);
    const string green = rgb(0,255,0),    light_green = rgb(131,255,131),  dark_green = rgb(0,128,0);
    const string blue = rgb(0,0,255),     light_blue = rgb(101,101,255),   dark_blue = rgb(0,0,140);
    const string cyan = rgb(0,255,255),   light_cyan = rgb(141,255,255),   dark_cyan = rgb(0,161,161);
    const string pink = rgb(255,0,255),   light_pink = rgb(255,130,255),   dark_pink = rgb(147,0,147);
    const string purple = rgb(175,0,255), light_purple = rgb(211,114,255), dark_purple = rgb(90,0,132);
    const string brown = "\033[0;33m",    orange = "",                     dark_brown = "";
    const string yellow = rgb(255,255,0), light_yellow = rgb(255,255,164), dark_yellow = rgb(159,159,0);
    const string gray = "",               light_gray = "\033[0;37m",       dark_gray = "\033[1;30m";
    const string black = rgb(0,0,0),      white = rgb(255,255,255);

    const string cursive = "\033[3m", bold = "\033[1m";
    const string underline = "\033[4m", upperline = "\033[53m", cross = "\033[9m";;
    const string blink = "\033[5m", hide = "\033[8m";
    const string normal = "\033[0m", no_color = "\033[39m";
    const string fade = "\033[2m", super = underline + upperline + blink;
    vector<string> colors_for_nick {light_red, light_blue, light_purple, dark_green, light_green,
                                    light_cyan, dark_cyan};
};
struct user_settings {
    enum admin_statuses {NotAdmin, Unverified, Strong, Weak};
    int soc = -1;
    string nick = "";
    bool input_some = true;
    bool not_nicked = true;
    bool not_colored = false;
    bool print_nicks = true;
    bool ban = false;
    admin_statuses admin_status = NotAdmin;
    string nick_color = style::colors_for_nick[Random() % style::colors_for_nick.size()];
    string nick_style = "";
    string mess_color = "";

    string full_nick() {
        return nick_color + nick_style + nick + style::normal;
    }
};
namespace chat_settings {
    bool join_open = true;
}

fd_set fd_clients;
unordered_map<int, user_settings> clients;
unordered_set<int> clients_soc;
unordered_set<int> strong_admins;
int listener, max_sd = 0;
int count_input_users = 0;
int count_admin_users = 0;
bool was_disconnected = false;

void FD_RESET();
void Connect();
void Disconnect(int);
string read_mess(int);
bool send_mess(string, int);

void print_error(string text) {
    cout << style::red << text << style::normal << endl;
}
void print_stat(string action) {
    cerr << style::normal << action << " (" << count_input_users << ", " <<
                                               clients.size() - count_input_users - count_admin_users << ", " <<
                                               count_admin_users << ")" << endl;
}
void send_command(string text, int sd, bool is_endl = true, bool is_admin_command = false) {
    if (is_admin_command)
        text = style::dark_yellow + text;
    else
        text = style::brown + text;
    if (is_endl)
        text += '\n';
    send_mess(text + style::no_color, sd);
}
void FD_RESET() {
    FD_ZERO(&fd_clients);
    FD_SET(listener, &fd_clients);
    for (auto sd : clients) {
        FD_SET(sd.first, &fd_clients);
    }
}
void Connect() {
    int sd = accept(listener, 0, 0);
    if (sd == -1)
        return;
    if ((!chat_settings::join_open) || clients_soc.size() >= 128) {
        send_command("Chat is closed or there more, then 128 people. :(", sd);
        close(sd);
        print_stat("Join attempt");
        return;
    }
    max_sd = std::max(max_sd, sd);
    FD_SET(sd, &fd_clients);
    clients[sd].soc = sd;
    clients_soc.insert(sd);
    count_input_users++;
    print_stat("Customize");
    send_command(style::green + "This is a chat. You can text with people." + style::normal +
                 style::brown + "\nEnter your nick: ", sd, false);
}
void Disconnect(int sd) {
    if (clients_soc.find(sd) != clients_soc.end()) {
        if (clients[sd].input_some)
            count_input_users--;
        else if (clients[sd].admin_status == user_settings::Strong ||
                 clients[sd].admin_status == user_settings::Weak)
            count_admin_users--;
    }
    clients.erase(sd);
    clients_soc.erase(sd);
    strong_admins.erase(sd);
    close(sd);
    was_disconnected = true;
    print_stat("Disconnect");
    FD_RESET();
}
string read_mess(int sd) {
    char buf[8192];
    int len = (int) recv(sd, buf, sizeof(buf), 0);
    if (len <= 0) {
        Disconnect(sd);
        return "";
    }
    string mess = string(buf, len);
    if (len > 1024) {
        mess = "";
        send_command("You can`t write very big message!", sd);
    } else if (mess.back() != '\n')
        mess.push_back('\n');
    return mess;
}
bool send_mess(string text, int sd) {
    int not_err = (int) send(sd, text.data(), text.size(), 0);
    if (not_err <= 0) {
        Disconnect(sd);
        return false;
    }
    return true;
}

namespace user_commands {
    void print_now_nick(user_settings &usr) {
        send_command("Now, your nick is \"" + usr.full_nick() + style::brown + "\".", usr.soc);
    }
    bool escape_message(string &mess, user_settings &usr) {
        if (mess.find("\033") <= mess.size()) {
            send_command("You can`t write [escape] in message!", usr.soc);
            if (clients[usr.soc].not_nicked)
                send_command("Enter your nick: ", usr.soc, false);
            return true;
        }
        return false;
    }
    bool help(string &mess, user_settings &usr) {
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
    void exec(int sd, string mess, bool already_admin_command) {
        user_settings &usr = clients[sd];
        for (auto func : list_commands) {
            if (func(mess, usr))
                return;
        }
        wrong_command(mess, usr, already_admin_command);
    }
}
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
            bool only_admins = command.size() > 6 && command.substr(6, 6) == "admins";
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
            } else {
                text += "Chat settings:";
                text += str_sum("\njoin_open = ", chat_settings::join_open);
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
                                                                    give_take_admin, super_nick};
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

bool create_connection(int port) {
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int option = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (listener < 0) {
        print_error("SOCKET ERROR");
        return true;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listener, (struct sockaddr *) &addr, sizeof(addr));
    if (listen(listener, 16) < 0) {
        print_error("LISTEN ERROR");
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    int port;
    try {
        port = stoi((string)argv[1]);
        if (port < 1000)
            throw std::exception();
    } catch (std::exception e) {
        port = 9999;
    }
    cerr << style::green << "IP : ";
    system(R"(ifconfig | perl -ne '/inet ([\d\.]+)/; print "$1\n"' | tail -n1)");
    cerr << "PORT : " << port << '\n' << style::normal << endl;
    if (create_connection(port))
        return 1;
    cerr << "Waiting..." << endl;
    Connect();
    bool infinity_loop = true;
    while (infinity_loop) {
        FD_RESET();
        int not_err = select(max_sd + 1, &fd_clients, NULL, NULL, NULL);
        if (not_err < 0) {
            print_error("SELECT ERROR");
            for (int sd : clients_soc)
                send_mess(style::dark_gray + "HACKER: i am hacker\n" + style::normal, sd);
            print_error("FIXED");
        }
        if (FD_ISSET(listener, &fd_clients)) {
            Connect();
            continue;
        }
        was_disconnected = false;
        for (int sd : clients_soc) {
            if (FD_ISSET(sd, &fd_clients)) {
                string mess = read_mess(sd);
                if (mess == "")
                    break;
                if (clients[sd].ban)
                    continue;
                bool already_admin_command = false;
                if (clients[sd].admin_status == user_settings::Strong || clients[sd].admin_status == user_settings::Weak)
                    already_admin_command = admin_commands::exec(sd, mess);
                user_commands::exec(sd, mess, already_admin_command);
                if (was_disconnected)
                    break;
            }
        }
    }
}
