#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "./user.h"

char* create_message(struct user user, char* msg);

void send_messages(struct user sender, struct user* users, int max_clients, char* msg);

void add_user(char* msg, struct user* users, int sd, int i);
