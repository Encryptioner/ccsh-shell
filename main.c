#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <errno.h>

#if __APPLE__ || __linux__
#include <readline/readline.h>
#include <readline/history.h>
#define READLINE_LIB 1
#else
#define READLINE_LIB 0
#endif

#define MAX_TOKENS 128
#define MAX_JOBS 64
#define MAX_ALIASES 64

typedef struct {
    pid_t pid;
    char command[1024];
} Job;

typedef struct {
    char name[64];
    char value[1024];
} Alias;

Job jobs[MAX_JOBS];
int job_count = 0;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

// Signal handler for Ctrl+C
void sigint_handler(int sig) {
    printf("\nUse 'exit' to quit.\nccsh> ");
    fflush(stdout);
}

// Job handling functions
void add_job(pid_t pid, const char* cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, sizeof(jobs[job_count].command) - 1);
        jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
        job_count++;
    }
}

void check_background_jobs() {
    for (int i = 0; i < job_count; i++) {
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result > 0) {
            printf("[done] %s\n", jobs[i].command);
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            i--;
        }
    }
}

void list_jobs() {
    if (job_count == 0) {
        printf("No background jobs.\n");
        return;
    }
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s\n", i, jobs[i].pid, jobs[i].command);
    }
}

void add_alias(const char* name, const char* value) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            strncpy(aliases[i].value, value, sizeof(aliases[i].value) - 1);
            return;
        }
    }
    if (alias_count < MAX_ALIASES) {
        strncpy(aliases[alias_count].name, name, sizeof(aliases[alias_count].name) - 1);
        strncpy(aliases[alias_count].value, value, sizeof(aliases[alias_count].value) - 1);
        alias_count++;
    } else {
        fprintf(stderr, "Alias limit reached.\n");
    }
}

void remove_alias(const char* name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            for (int j = i; j < alias_count - 1; j++) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return;
        }
    }
    fprintf(stderr, "Alias not found: %s\n", name);
}

const char* get_alias_value(const char* name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}

void parse_command(char* input, char** args, int* background, char** infile, char** outfile, int* append) {
    int i = 0;
    *background = 0;
    *infile = NULL;
    *outfile = NULL;
    *append = 0;

    char* token = strtok(input, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            *infile = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            *outfile = token;
            *append = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            *outfile = token;
            *append = 1;
        } else if (strcmp(token, "&") == 0) {
            *background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void expand_globs(char** args, char** expanded_args, int* expanded_count) {
    *expanded_count = 0;
    glob_t results;
    results.gl_offs = 0;

    for (int i = 0; args[i] != NULL; i++) {
        glob_t temp;
        int flags = GLOB_TILDE;
        if (strchr(args[i], '*') || strchr(args[i], '?')) {
            if (*expanded_count > 0) flags |= GLOB_APPEND;
            if (glob(args[i], flags, NULL, &results) != 0) {
                expanded_args[(*expanded_count)++] = args[i];
            } else {
                for (size_t j = 0; j < results.gl_pathc; j++) {
                    expanded_args[(*expanded_count)++] = results.gl_pathv[j];
                }
            }
        } else {
            expanded_args[(*expanded_count)++] = args[i];
        }
    }
    expanded_args[*expanded_count] = NULL;
}

void expand_alias(char* line, char* out, size_t out_size) {
    char* token = strtok(line, " ");
    if (!token) {
        *out = '\0';
        return;
    }

    const char* alias_val = get_alias_value(token);
    if (alias_val) {
        snprintf(out, out_size, "%s", alias_val);
        char* rest = strtok(NULL, "");
        if (rest) {
            strncat(out, " ", out_size - strlen(out) - 1);
            strncat(out, rest, out_size - strlen(out) - 1);
        }
    } else {
        strncpy(out, line, out_size - 1);
    }
}

void print_help() {
    printf("Supported features:\n");
    printf("  Built-in: cd, pwd, exit, help, fg, jobs, alias, unalias\n");
    printf("  I/O Redirection: <, >, >>\n");
    printf("  Background jobs: & (with fg and jobs to control)\n");
    printf("  Globbing: *, ?\n");
    printf("  Aliases: alias name='value', unalias name\n");
    printf("  Command history with arrow keys\n");
}

int main() {
    signal(SIGINT, sigint_handler);

    #if READLINE_LIB
    read_history(".ccsh_history");
    #endif

    char* line;
    while (1) {
        #if READLINE_LIB
        line = readline("ccsh> ");
        #else
        static char buffer[1024];
        printf("ccsh> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;
        line = strdup(buffer);
        #endif

        if (!line || !*line) {
            free(line);
            continue;
        }
        add_history(line);
        check_background_jobs();

        // Expand alias
        char expanded_line[2048];
        expand_alias(line, expanded_line, sizeof(expanded_line));

        int background = 0, append = 0;
        char *infile = NULL, *outfile = NULL;
        char* args[MAX_TOKENS];
        parse_command(expanded_line, args, &background, &infile, &outfile, &append);

        if (!args[0]) {
            free(line);
            continue;
        }

        // Built-in commands
        if (strcmp(args[0], "exit") == 0) break;
        if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1] ? args[1] : getenv("HOME")) != 0) perror("cd");
            free(line);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
            else perror("pwd");
            free(line);
            continue;
        }
        if (strcmp(args[0], "jobs") == 0) {
            list_jobs();
            free(line);
            continue;
        }
        if (strcmp(args[0], "fg") == 0 && args[1]) {
            int job_id = atoi(args[1]);
            if (job_id >= 0 && job_id < job_count) {
                int status;
                waitpid(jobs[job_id].pid, &status, 0);
            } else fprintf(stderr, "Invalid job ID\n");
            free(line);
            continue;
        }
        if (strcmp(args[0], "alias") == 0) {
            if (!args[1]) {
                for (int i = 0; i < alias_count; i++) {
                    printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
                }
            } else {
                char* eq = strchr(args[1], '=');
                if (eq && eq > args[1]) {
                    *eq = '\0';
                    char* name = args[1];
                    char* value = eq + 1;
                    if (*value == '\'' || *value == '"') {
                        value++;
                        value[strlen(value) - 1] = '\0';
                    }
                    add_alias(name, value);
                } else {
                    fprintf(stderr, "Usage: alias name='value'\n");
                }
            }
            free(line);
            continue;
        }
        if (strcmp(args[0], "unalias") == 0) {
            if (!args[1]) fprintf(stderr, "Usage: unalias name\n");
            else remove_alias(args[1]);
            free(line);
            continue;
        }
        if (strcmp(args[0], "help") == 0) {
            print_help();
            free(line);
            continue;
        }

        // Globbing
        char* expanded[MAX_TOKENS];
        int expanded_count;
        expand_globs(args, expanded, &expanded_count);

        // Fork & exec
        pid_t pid = fork();
        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd == -1) { perror("input"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (outfile) {
                int fd = open(outfile, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
                if (fd == -1) { perror("output"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            execvp(expanded[0], expanded);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            if (background) {
                printf("[%d] %d\n", job_count, pid);
                add_job(pid, line);
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        } else {
            perror("fork");
        }

        free(line);
    }

    #if READLINE_LIB
    write_history(".ccsh_history");
    #endif

    return 0;
}
