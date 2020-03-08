
#include "../headers/chat.h"

char* create_message(struct user user, char* msg) {
    int username_len = strlen(user.username);
    int msg_len = strlen(msg);

    char* output = malloc((username_len + msg_len + 3) * sizeof(char));

    strcpy(output, user.username);
    strcpy(output + username_len, ":\n");
    strcpy(output + username_len + 2, msg);
    strcpy(output + username_len + 2 + msg_len, "\n\0");

    return output;
}

void send_messages(struct user sender, struct user* users, int max_clients, char* msg) {
    int msg_len = strlen(msg) - 2; // Minus one for startng char and one for \n
    int name_len = strlen(sender.username);
    int output_len = msg_len + name_len + 3;
    char output[output_len];
    printf("%d %d %d\n", name_len, msg_len, output_len);
    printf("%s %d\n", msg, sender.sd);
    strcpy(output + 1, sender.username);
    strcpy(output + name_len + 2, msg + 1);
    output[0] = 'm'; output[name_len + 1] = '|'; output[output_len - 1] = '\0';

    printf("Output: %s\n", output);
    for (int i = 0; i < output_len; i++) {
        printf("%c, %d\n", output[i], i);
    }
    for ( int i = 0; i < max_clients; i++ ) {
        if ( users[i].sd == sender.sd ) {
            printf("%s\n", users[i].username);
        } else if (users[i].sd != 0) {
            printf("Sending to user %s %d\n", users[i].username, users[i].sd);
            send(users[i].sd, output, output_len * sizeof(char), 0);
        }

    }
    puts("Done sending");
}

void add_user(char* msg, struct user* users, int sd, int i) {
    char* username = malloc(40 * sizeof(char));
    memset(username, 0, 40 * sizeof(char));
    int j;
    for ( j = 1; msg[j] != 0x0; j++ ) {
        username[j-1] = msg[j];
    }
    username[j] = 0;
    printf("Username %s\n", username);

    users[i].username = username;
    printf("%s\n", users[i].username);
    users[i].sd = sd;
    users[i].id = sd;
}
