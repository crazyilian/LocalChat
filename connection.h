#ifndef LOCAL_CHAT_CONNECTION_H
#define LOCAL_CHAT_CONNECTION_H

#include "global_variables.h"
#include "style.h"
#include "settings.h"

void Connect() {
    int sd = accept(listener, 0, 0);
    if (sd == -1)
        return;
    if ((!chat_settings::join_open) || clients_soc.size() >= 128) {
        send_command("Chat is closed or there more, then 128 people. :(", sd);
        close(sd);
        if (chat_settings::logs_attempts)
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

#endif

