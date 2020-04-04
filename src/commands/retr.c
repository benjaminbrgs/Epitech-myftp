/*
** EPITECH PROJECT, 2020
** NWP_myftp_2019
** File description:
** TODO: add description
*/

#include <sys/stat.h>
#include "myftp.h"

static void exec_cmd_retr(client_t *client, char **cmd)
{
    FILE *output_file = NULL;
    char str[4096];

    if (chdir(client->cur_dir) == -1) {
        dprintf(client->data_client, "Failed to exec LIST command");
        exit(0);
    }
    bzero(str, 4096);
    output_file = fopen(cmd[1], "rb");
    while (fread(str, 1, 256, output_file)) {
        dprintf(client->data_client, "%s", str);
        bzero(str, 4096);
    }
    fclose(output_file);
    exit(0);
}

static bool is_file_on_server(client_t *client, char *file)
{
    char real_path[PATH_MAX];
    bzero(real_path, PATH_MAX);
    if (file[0] != '/') {
        strcpy(real_path, client->cur_dir);
        if (real_path[strlen(real_path) - 1] != '/')
            real_path[strlen(real_path)] = '/';
        strcat(real_path, file);
    } else {
        strcpy(real_path, file);
    }
    if (access(real_path, F_OK) != -1) {
        struct stat info;
        if (stat(real_path, &info) == -1)
            return (false);
        if (S_ISDIR(info.st_mode))
            return (false);
        return (true);
    }
    return (false);
}

void retr(server_info_t *info, client_t *client, char **cmd)
{
    (void)info;
    pid_t process = -1;
    if (is_client_login(client) != LOGGED)
        return (add_message_client(client, E_530PL));
    if (get_size_array(cmd) < 2)
        return (add_message_client(client, E_501));
    if (client->mode == UNDEFINED_MODE)
        return (add_message_client(client, E_425));
    if (!is_file_on_server(client, cmd[1]))
        return (add_message_client(client, E_550));
    if (handle_mode_open(client) == TCP_ERROR)
        return (add_message_client(client, E_425U));
    if ((process = fork()) == -1) {
        close_fd(&client->data_socket);
        return (add_message_client(client, E_425U));
    }
    if (process == 0)
        exec_cmd_retr(client, cmd);
    else
        add_process_to_client(client, process);
    return (add_message_client(client, E_150));
}