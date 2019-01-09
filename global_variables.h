#ifndef LOCAL_CHAT_GLOBAL_VARIABLES_H
#define LOCAL_CHAT_GLOBAL_VARIABLES_H

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <random>
#include <exception>
#include <functional>

using std::vector;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::to_string;


std::mt19937 Random(time(NULL));
std::hash<std::string> string_hash;
fd_set fd_clients;
std::unordered_set<int> clients_soc;
std::unordered_set<int> strong_admins;
int listener, max_sd = 0;
int count_input_users = 0;
int count_admin_users = 0;
bool was_disconnected = false;

void FD_RESET();
void Connect();
void Disconnect(int);
std::string read_mess(int);
bool send_mess(std::string, int);
void print_error(std::string);
void print_stat(std::string);
void send_command(std::string, int, bool=true, bool=false);

#endif
