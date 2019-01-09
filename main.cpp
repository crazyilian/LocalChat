#include "global_variables.h"
#include "style.h"
#include "settings.h"
#include "user_commands.h"
#include "admin_commands.h"
#include "connection.h"


void print_error(string text) {
    cout << style::red << text << style::normal << endl;
}
void print_stat(string action) {
    cerr << style::normal << action << " (" << count_input_users << ", " <<
         clients.size() - count_input_users - count_admin_users << ", " <<
         count_admin_users << ")" << endl;
}
void send_command(string text, int sd, bool is_endl, bool is_admin_command) {
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
                send_mess(style::dark_gray + "HACKER: I am hacker\n" + style::normal, sd);
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
