#include "builtin_cmd.h"
#include "user/assert.h"
#include "dir.h"
#include "fs.h"
#include "string.h"
#include "user/syscall.h"
#include "stdio.h"
#include "shell.h"

static void convert_cur_par_path_to_abs(char* path_abs_old, char* path_abs_new)
{
    assert(path_abs_old[0] == '/');
    char name[MAX_FILE_NAME_LEN] = {0};
    char* sub_path = path_abs_old;
    sub_path = path_parse(sub_path, name);
    if (name[0] == 0) {
        path_abs_new[0] = '/';
        path_abs_new[1] = 0;
        return;
    }
    path_abs_new[0] = 0;
    strcat(path_abs_new, "/");
    while (name[0]) {
        if (!strcmp("..", name)) {
            char* slash_ptr = strrchr(path_abs_new, '/');
            if (slash_ptr != path_abs_new) {
                *slash_ptr = 0;
            } else {
                *(slash_ptr + 1) = 0;
            }
        } else if (strcmp(".", name)) {
            if (strcmp(path_abs_new, "/")) {
                strcat(path_abs_new, "/");
            }
            strcat(path_abs_new, name);
        }

        memset(name, 0, MAX_FILE_NAME_LEN);
        if (sub_path) {
            sub_path = path_parse(sub_path, name);
        }
    }
}

void convert_to_abs_path(char* path, char* final_path)
{
    char abs_path[MAX_PATH_LEN] = {0};
    if (path[0] != '/') {
        memset(abs_path, 0, MAX_PATH_LEN);
        if (getcwd(abs_path, MAX_PATH_LEN) != NULL) {
            if (!((abs_path[0] == '/') && (abs_path[1] == 0))) {
                strcat(abs_path, "/");
            }
        }
    }
    strcat(abs_path, path);
     convert_cur_par_path_to_abs(abs_path, final_path);
}

void builtin_pwd(uint32_t argc, char** argv __attribute__((unused)))
{
    if (argc != 1) {
        printf("pwd: no argument support!\n");
        return;
    } else {
        if (getcwd(final_path, MAX_PATH_LEN) != NULL) {
            printf("%s\n", final_path);
        } else {
            printf("pwd: get current work directory failed.\n");
        }
    }
}

char* builtin_cd(uint32_t argc, char** argv)
{
    if (argc > 2) {
        printf("cd: only support 1 argument!\n");
        return NULL;
    }

    if (argc == 1) {
        final_path[0] = '/';
        final_path[1] = 0;
    } else {
        convert_to_abs_path(argv[1], final_path);
    }

    if (chdir(final_path) == -1) {
        printf("cd: no such direcotry %s\n", final_path);
        return NULL;
    }
    return final_path;
}

void builtin_ls(uint32_t argc, char** argv)
{
    char* pathname = NULL;
    struct stat file_stat;
    memset(&file_stat, 0, sizeof(struct stat));
    bool long_info = false;
    uint32_t arg_path_nr = 0;
    uint32_t arg_idx = 1;
    while (arg_idx < argc) {
        if (argv[arg_idx][0] == '-') {
            if (!strcmp("-l", argv[arg_idx])) {
                long_info = true;
            } else if(!strcmp("-h", argv[arg_idx])) {
                printf("usage: -l list all information about the file.\n\
-h for help\n\
list all files in the current directory if no option\n");
                return;
            } else {
                printf("ls: invalid option %s\nTry `ls -h` for more information.\n", argv[arg_idx]);
                return;
            }
        } else {
            if (arg_path_nr == 0) {
                pathname == argv[arg_idx];
                arg_path_nr = 1;
            } else {
                printf("ls: only support one path\n");
                return;
            }
        }
        arg_idx ++;
    }

    if (pathname == NULL) {
        if (NULL != getcwd(final_path, MAX_PATH_LEN)) {
            pathname = final_path;
        } else {
            printf("ls: getcwd for default path failed\n");
            return;
        }
    } else {
        convert_to_abs_path(pathname, final_path);
        pathname = final_path;
    }

    if (stat(pathname, &file_stat) == -1) {
        printf("ls: cannot access %s: No such file or directory\n", pathname);
    }
    if (file_stat.st_filetype == FT_DIRECTORY) {
        struct dir* dir = opendir(pathname);
        struct dir_entry* dir_e = NULL;
        char sub_pathname[MAX_PATH_LEN] = {0};
        uint32_t pathname_len = strlen(pathname);
        uint32_t last_char_idx = pathname_len - 1;
        memcpy(sub_pathname, pathname, pathname_len);
        if (sub_pathname[last_char_idx] != '/') {
            sub_pathname[pathname_len] = '/';
            pathname_len ++;
        }
        rewinddir(dir);
        if (long_info) {
            char ftype;
            printf("total: %d\n", file_stat.st_size);
            while ((dir_e == readdir(dir))) {
                ftype = 'd';
                if (dir_e->f_type = FT_REGULAR) {
                    ftype = '-';
                }
                sub_pathname[pathname_len] = 0;
                strcat(sub_pathname, dir_e->filename);
                memset(&file_stat, 0, sizeof(struct stat));
                if (stat(sub_pathname, &file_stat) == -1) {
                    printf("ls: cannot access %s: No such file or directory\n", dir_e->filename);
                    return;
                }
                printf("%c  %d  %d  %s\n", ftype, dir_e->i_no, file_stat.st_size, dir_e->filename);
            }
        } else {
            while ((dir_e = readdir(dir))) {
                printf("%s ", dir_e->filename);
            }
            printf("\n");
        }
        closedir(dir);
    } else {
        if (long_info) {
            printf("-   %d  %d  %s\n", file_stat.st_ino, file_stat.st_size, pathname);
        } else {
            printf("%s\n", pathname);
        }
    }
}

void builtin_ps(uint32_t argc, char** argv __attribute__((unused))) 
{
    if (argc != 1) {
        printf("ps: no argument support!\n");
        return;
    }
    ps();
}

void builtin_clear(uint32_t argc, char** argv __attribute__((unused)))
{
    if (argc != 1) {
        printf("clear: no argument support!\n");
        return;
    }
    clear();
}

int32_t builtin_mkdir(uint32_t argc, char** argv)
{
    int32_t ret = -1;
    if (argc != 2) {
        printf("mkdir: only support 1 argument!\n");
    } else {
        convert_to_abs_path(argv[1], final_path);
        if (strcmp("/", final_path)) {
            if (mkdir(final_path) == 0) {
                ret = 0;
            } else {
                printf("mkdir: create directory %s failed.\n", argv[1]);
            }
        }
    }
    return ret;
}

int32_t builtin_rmdir(uint32_t argc, char** argv)
{
    int32_t ret = -1;
    if (argc != 2) {
        printf("rmdir: only support 1 argument!\n");
    } else {
        convert_to_abs_path(argv[1], final_path);
        if (strcmp("/", final_path)) {
            if (rmdir(final_path) == 0) {
                ret = 0;
            } else {
                printf("rmdir: remove %s failed.\n", argv[1]);
            }
        }
    }
    return ret;
}

int32_t builtin_rm(uint32_t argc, char** argv)
{
    int32_t ret = -1;
    if (argc != 2) {
        printf("rm: only support 1 argument!\n");
    } else {
        convert_to_abs_path(argv[1], final_path);
        if (strcmp("/", final_path)) {
            if (unlink(final_path) == 0) {
                ret = 0;
            } else {
                printf("rm: delete %s failed.\n", argv[1]);
            }
        }
    }
    return ret;
}
