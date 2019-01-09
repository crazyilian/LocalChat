#ifndef LOCAL_CHAT_OTHER_COMMANDS_H
#define LOCAL_CHAT_OTHER_COMMANDS_H

#include "global_variables.h"
#include "style.h"

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

#endif
