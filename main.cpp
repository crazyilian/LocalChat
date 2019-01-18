#include "global_variables.h"
#include "style.h"
#include "settings.h"
#include "user_commands.h"
#include "admin_commands.h"
#include "other_commands.h"
#include "connection.h"

int main(int argc, char* argv[]) {
    int port;
    try {
        port = stoi((string)argv[1]);
        if (port < 1024 || port > 65535)
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
