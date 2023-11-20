#include "shell.h"
#include "stdio.h"
#include "user/assert.h"
#include "user/syscall.h"
#include "file.h"
#include "string.h"
#include "fs.h"
#include "builtin_cmd.h"

#define MAX_ARG_NR  16

static char cmd_line[MAX_PATH_LEN] = {0};

char cwd_cache[MAX_PATH_LEN] = {0};
char final_path[MAX_PATH_LEN] = {0};

char* argv[MAX_ARG_NR];
int32_t argc = -1;

void print_prompt(void)
{
    printf("[Lowell_0xDkXy@localhost %s]$ ", cwd_cache);
}

static void readline(char* buf, int32_t count)
{
    assert(buf != NULL && count > 0);
    char* pos = buf;
    while (read(stdin_no, pos, 1) != -1 && (pos - buf) < count) {
        switch (*pos) {
            case '\n':
            case '\r':
                *pos = 0;
                putchar('\n');
                return;
            case '\b':
                if (buf[0] != '\b') {
                    --pos;
                    putchar('\b');
                }
                break;
            case 'l' - 'a':
                *pos = 0;
                clear();
                print_prompt();
                printf("%s", buf);
                break;
            case 'u' - 'a':
                while (buf != pos) {
                    putchar('\b');
                    *(pos--) = 0;
                }
                break;
            default:
                putchar(*pos);
                pos++;
        }
    }
    printf("readline: cannot find enter_key in the cmd_line, max limit of char is 128");
}


static int32_t cmd_parse(char* cmd_str, char** argv, char token)
{
    assert(cmd_str != NULL);
    int32_t arg_idx = 0;
    while (arg_idx < MAX_ARG_NR) {
        argv[arg_idx] = NULL;
        arg_idx ++;
    }
    char* next = cmd_str;  
    int32_t argc = 0;

    while (*next) {
        while (*next == token) {
            next ++;
        }
        if (*next == 0) {
            break;
        }
        argv[argc] = next;

        while (*next && *next != token) {
            next ++;
        }

        if (*next) {
            *next++ = 0;
        }

        if (argc > MAX_ARG_NR) {
            return -1;
        }
        argc ++;
    }
    return argc;
}


void my_shell(void)
{
    cwd_cache[0] = '/';
    cwd_cache[1] = '0';
    while (1) {
        print_prompt();
        memset(final_path, 0, MAX_PATH_LEN);
        memset(cmd_line, 0, MAX_PATH_LEN);
        readline(cmd_line, MAX_PATH_LEN);
        if (cmd_line[0] == 0) {
            continue;
        }
        argc = -1;
        argc = cmd_parse(cmd_line, argv, ' ');
        if (argc == -1) {
            printf("num of arguments exceed %d\n", MAX_ARG_NR);
            continue;
        }

        /*
        char buf[MAX_PATH_LEN] = {0};
        int32_t arg_idx = 0;
        while (arg_idx < argc) {
            convert_to_abs_path(argv[arg_idx], buf);
            printf("%s -> %s\n", argv[arg_idx], buf);
            arg_idx ++;
        }
        printf("\n");
        */

        if (!strcmp("ls", argv[0])) {
            builtin_ls(argc, argv);
        } else if (!strcmp("cd", argv[0])) {
            if (builtin_cd(argc, argv) != NULL) {
                memset(cwd_cache, 0, MAX_PATH_LEN);
                strcpy(cwd_cache, final_path);
            }
        } else if (!strcmp("pwd", argv[0])) {
            builtin_pwd(argc, argv);
        } else if (!strcmp("ps", argv[0])) {
            builtin_ps(argc, argv);
        } else if (!strcmp("clear", argv[0])) {
            builtin_clear(argc, argv);
        } else if (!strcmp("mkdir", argv[0])) {
            builtin_mkdir(argc, argv);
        } else if (!strcmp("rmdir", argv[0])) {
            builtin_rmdir(argc, argv);
        } else if (!strcmp("rm", argv[0])) {
            builtin_rm(argc, argv);
        } else {
            int32_t pid = fork();
            if (pid) {
                while (1);
            } else {
                convert_to_abs_path(argv[0], final_path);
                argv[0] = final_path;

                struct stat file_stat;
                memset(&file_stat, 0, sizeof(struct stat));
                if (stat(argv[0], &file_stat) == -1) {
                    printf("shell: cannot access: %s: No such file or directory\n", argv[0]);
                } else {
                    execv(argv[0], argv);
                }
                while (1);
            }
        }
        int32_t arg_idx = 0;
        while (arg_idx < MAX_ARG_NR) {
            argv[arg_idx] = NULL;
            arg_idx ++;
        }
    }
    panic("my_shell: should not be here");
}

