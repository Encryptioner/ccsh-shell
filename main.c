/*
 * ccsh - Compact C Shell
 * A lightweight Unix-like shell implementation in C
 * 
 * Features:
 * - Interactive command prompt with history
 * - Built-in commands (cd, pwd, exit, jobs, fg, alias, unalias, help)
 * - I/O redirection (<, >, >>)
 * - Background job management
 * - Globbing support (*, ?)
 * - Alias system
 * - Signal handling (Ctrl+C)
 * 
 * Libraries used:
 * - stdio.h: Standard I/O operations (printf, fprintf, perror)
 * - stdlib.h: Memory management (malloc, free, exit)
 * - string.h: String manipulation (strcmp, strcpy, strtok, etc.)
 * - unistd.h: POSIX system calls (fork, exec, chdir, getcwd)
 * - sys/wait.h: Process control (waitpid, WNOHANG)
 * - fcntl.h: File control operations (open, O_RDONLY, O_WRONLY, etc.)
 * - glob.h: Pathname pattern matching (glob, GLOB_TILDE)
 * - signal.h: Signal handling (signal, SIGINT)
 * - errno.h: Error codes and error handling
 * - readline/readline.h: Interactive command line editing (readline, add_history)
 * - readline/history.h: Command history management
 */

#include <stdio.h>      /* Standard I/O operations: printf, fprintf, perror, fflush */
#include <stdlib.h>     /* Memory management: malloc, free, exit, atoi */
#include <string.h>     /* String operations: strcmp, strcpy, strncpy, strtok, strchr, strlen, strcspn */
#include <unistd.h>     /* POSIX system calls: fork, execvp, chdir, getcwd, dup2, close */
#include <sys/wait.h>   /* Process control: waitpid, WNOHANG, WIFEXITED */
#include <fcntl.h>      /* File control: open, O_RDONLY, O_WRONLY, O_CREAT, O_APPEND, O_TRUNC */
#include <glob.h>       /* Pathname pattern matching: glob, GLOB_TILDE, GLOB_APPEND */
#include <signal.h>     /* Signal handling: signal, SIGINT, SIG_DFL */
#include <errno.h>      /* Error codes and error handling */

/* Readline library support - available on macOS and Linux */
#if __APPLE__ || __linux__
    #include <readline/readline.h>  /* Interactive command line editing */
    #include <readline/history.h>   /* Command history management */
    #define READLINE_LIB 1
#else
    #define READLINE_LIB 0
#endif

/* Constants for shell limits */
#define MAX_TOKENS 128    /* Maximum number of command arguments */
#define MAX_JOBS 64       /* Maximum number of background jobs */
#define MAX_ALIASES 64    /* Maximum number of aliases */

/* Job structure to track background processes */
typedef struct {
    pid_t pid;           /* Process ID of the background job */
    char command[1024];  /* Command string for display */
} Job;

/* Alias structure to store command aliases */
typedef struct {
    char name[64];       /* Alias name */
    char value[1024];    /* Alias value/command */
} Alias;

/* Global variables for job and alias management */
Job jobs[MAX_JOBS];
int job_count = 0;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

/* Signal handler for Ctrl+C (SIGINT) */
void sigint_handler(int sig) {
    (void)sig;  /* Suppress unused parameter warning */
    printf("\nUse 'exit' to quit.\nccsh> ");
    fflush(stdout);  /* Ensure prompt is displayed immediately */
}

/* Job management functions */

/**
 * Add a new background job to the job list
 * @param pid Process ID of the job
 * @param cmd Command string for display
 */
void add_job(pid_t pid, const char* cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, sizeof(jobs[job_count].command) - 1);
        jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';
        job_count++;
    }
}

/**
 * Check for completed background jobs and remove them from the list
 * Uses waitpid with WNOHANG to check without blocking
 */
void check_background_jobs() {
    for (int i = 0; i < job_count; i++) {
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result > 0) {
            printf("[done] %s\n", jobs[i].command);
            /* Remove completed job by shifting remaining jobs */
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            i--;  /* Recheck current index since jobs shifted */
        }
    }
}

/**
 * Display all current background jobs
 */
void list_jobs() {
    if (job_count == 0) {
        printf("No background jobs.\n");
        return;
    }
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s\n", i, jobs[i].pid, jobs[i].command);
    }
}

/* Alias management functions */

/**
 * Add or update an alias
 * @param name Alias name
 * @param value Alias value/command
 */
void add_alias(const char* name, const char* value) {
    /* Check if alias already exists and update it */
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            strncpy(aliases[i].value, value, sizeof(aliases[i].value) - 1);
            aliases[i].value[sizeof(aliases[i].value) - 1] = '\0';
            return;
        }
    }
    /* Add new alias if space available */
    if (alias_count < MAX_ALIASES) {
        strncpy(aliases[alias_count].name, name, sizeof(aliases[alias_count].name) - 1);
        aliases[alias_count].name[sizeof(aliases[alias_count].name) - 1] = '\0';
        strncpy(aliases[alias_count].value, value, sizeof(aliases[alias_count].value) - 1);
        aliases[alias_count].value[sizeof(aliases[alias_count].value) - 1] = '\0';
        alias_count++;
    } else {
        fprintf(stderr, "Alias limit reached.\n");
    }
}

/**
 * Remove an alias by name
 * @param name Name of alias to remove
 */
void remove_alias(const char* name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            /* Shift remaining aliases to fill the gap */
            for (int j = i; j < alias_count - 1; j++) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return;
        }
    }
    fprintf(stderr, "Alias not found: %s\n", name);
}

/**
 * Get the value of an alias by name
 * @param name Name of alias to look up
 * @return Pointer to alias value or NULL if not found
 */
const char* get_alias_value(const char* name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}

/* Command parsing and execution functions */

/**
 * Parse command line into arguments and handle I/O redirection
 * @param input Raw command string
 * @param args Array to store parsed arguments
 * @param background Set to 1 if job should run in background
 * @param infile Input file for redirection (NULL if none)
 * @param outfile Output file for redirection (NULL if none)
 * @param append Set to 1 for append mode (>>), 0 for truncate (>)
 */
void parse_command(char* input, char** args, int* background, char** infile, char** outfile, int* append) {
    int i = 0;
    *background = 0;
    *infile = NULL;
    *outfile = NULL;
    *append = 0;

    char* token = strtok(input, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            /* Input redirection */
            token = strtok(NULL, " ");
            *infile = token;
        } else if (strcmp(token, ">") == 0) {
            /* Output redirection (truncate) */
            token = strtok(NULL, " ");
            *outfile = token;
            *append = 0;
        } else if (strcmp(token, ">>") == 0) {
            /* Output redirection (append) */
            token = strtok(NULL, " ");
            *outfile = token;
            *append = 1;
        } else if (strcmp(token, "&") == 0) {
            /* Background execution */
            *background = 1;
        } else {
            /* Regular argument */
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  /* Null-terminate argument list */
}

/**
 * Expand glob patterns (*, ?) in command arguments
 * @param args Original arguments array
 * @param expanded_args Array to store expanded arguments
 * @param expanded_count Number of expanded arguments
 */
void expand_globs(char** args, char** expanded_args, int* expanded_count) {
    *expanded_count = 0;
    glob_t results;
    results.gl_offs = 0;
    results.gl_pathc = 0;
    results.gl_pathv = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        int flags = GLOB_TILDE;  /* Expand ~ to home directory */
        if (strchr(args[i], '*') || strchr(args[i], '?')) {
            /* Pattern contains glob characters */
            if (*expanded_count > 0) flags |= GLOB_APPEND;
            if (glob(args[i], flags, NULL, &results) != 0) {
                /* Glob failed, use original argument */
                expanded_args[(*expanded_count)++] = args[i];
            } else {
                /* Add all matched files */
                for (size_t j = 0; j < results.gl_pathc; j++) {
                    expanded_args[(*expanded_count)++] = results.gl_pathv[j];
                }
            }
        } else {
            /* No glob characters, use as-is */
            expanded_args[(*expanded_count)++] = args[i];
        }
    }
    expanded_args[*expanded_count] = NULL;
    
    /* Free the glob structure */
    if (results.gl_pathv != NULL) {
        globfree(&results);
    }
}

/**
 * Expand aliases in command line
 * @param line Original command line
 * @param out Buffer for expanded command line
 * @param out_size Size of output buffer
 */
void expand_alias(char* line, char* out, size_t out_size) {
    /* Create a copy of the line to avoid modifying the original */
    char line_copy[2048];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    char* token = strtok(line_copy, " ");
    if (!token) {
        *out = '\0';
        return;
    }

    const char* alias_val = get_alias_value(token);
    if (alias_val) {
        /* Replace first token with alias value */
        snprintf(out, out_size, "%s", alias_val);
        char* rest = strtok(NULL, "");
        if (rest) {
            /* Append remaining arguments */
            strncat(out, " ", out_size - strlen(out) - 1);
            strncat(out, rest, out_size - strlen(out) - 1);
        }
    } else {
        /* No alias found, copy original line */
        strncpy(out, line, out_size - 1);
        out[out_size - 1] = '\0';
    }
}

/**
 * Display help information about shell features
 */
void print_help() {
    printf("ccsh - Compact C Shell\n");
    printf("Supported features:\n");
    printf("  Built-in commands: cd, pwd, exit, help, fg, jobs, alias, unalias\n");
    printf("  I/O Redirection: < (input), > (output), >> (append)\n");
    printf("  Background jobs: & (with fg and jobs to control)\n");
    printf("  Globbing: *, ? (filename pattern matching)\n");
    printf("  Aliases: alias name='value', unalias name\n");
    printf("  Command history with arrow keys (if readline available)\n");
    printf("  Signal handling: Ctrl+C to interrupt\n");
}

/**
 * Main shell loop
 * Handles command input, parsing, and execution
 */
int main() {
    /* Set up signal handler for Ctrl+C */
    signal(SIGINT, sigint_handler);

    /* Load command history if readline is available */
    #if READLINE_LIB
    read_history(".ccsh_history");
    #endif

    char* line;
    while (1) {
        /* Get command line input */
        #if READLINE_LIB
        line = readline("ccsh> ");
        if (line == NULL) {
            /* EOF or readline error - exit gracefully */
            printf("\n");
            break;
        }
        #else
        static char buffer[1024];
        printf("ccsh> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;  /* Remove newline */
        line = strdup(buffer);
        #endif

        /* Handle empty input */
        if (!line || !*line) {
            free(line);
            continue;
        }
        
        /* Add to history and check background jobs */
        #if READLINE_LIB
        add_history(line);
        #endif
        check_background_jobs();

        /* Expand aliases in command line */
        char expanded_line[2048];
        expand_alias(line, expanded_line, sizeof(expanded_line));

        /* Parse command into arguments and handle redirection */
        int background = 0, append = 0;
        char *infile = NULL, *outfile = NULL;
        char* args[MAX_TOKENS];
        parse_command(expanded_line, args, &background, &infile, &outfile, &append);

        /* Skip if no command */
        if (!args[0]) {
            free(line);
            continue;
        }

        /* Handle built-in commands */

        /* Exit command */
        if (strcmp(args[0], "exit") == 0) break;
        
        /* Change directory */
        if (strcmp(args[0], "cd") == 0) {
            const char* target = args[1] ? args[1] : getenv("HOME");
            if (!target) {
                fprintf(stderr, "cd: HOME environment variable not set\n");
            } else if (chdir(target) != 0) {
                perror("cd");
            }
            free(line);
            continue;
        }
        
        /* Print working directory */
        if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
            else perror("pwd");
            free(line);
            continue;
        }
        
        /* List background jobs */
        if (strcmp(args[0], "jobs") == 0) {
            list_jobs();
            free(line);
            continue;
        }
        
        /* Bring job to foreground */
        if (strcmp(args[0], "fg") == 0 && args[1]) {
            int job_id = atoi(args[1]);
            if (job_id >= 0 && job_id < job_count) {
                int status;
                waitpid(jobs[job_id].pid, &status, 0);
                /* Remove the job from the list after completion */
                for (int j = job_id; j < job_count - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }
                job_count--;
            } else {
                fprintf(stderr, "Invalid job ID: %s\n", args[1]);
            }
            free(line);
            continue;
        }
        
        /* Alias management */
        if (strcmp(args[0], "alias") == 0) {
            if (!args[1]) {
                /* List all aliases */
                for (int i = 0; i < alias_count; i++) {
                    printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
                }
            } else {
                /* Add new alias */
                char* eq = strchr(args[1], '=');
                if (eq && eq > args[1]) {
                    *eq = '\0';
                    char* name = args[1];
                    char* value = eq + 1;
                    /* Remove quotes if present */
                    if (*value == '\'' || *value == '"') {
                        value++;
                        size_t len = strlen(value);
                        if (len > 0 && (value[len-1] == '\'' || value[len-1] == '"')) {
                            value[len-1] = '\0';
                        }
                    }
                    add_alias(name, value);
                } else {
                    fprintf(stderr, "Usage: alias name='value'\n");
                }
            }
            free(line);
            continue;
        }
        
        /* Remove alias */
        if (strcmp(args[0], "unalias") == 0) {
            if (!args[1]) {
                fprintf(stderr, "Usage: unalias name\n");
            } else {
                remove_alias(args[1]);
            }
            free(line);
            continue;
        }
        
        /* Help command */
        if (strcmp(args[0], "help") == 0) {
            print_help();
            free(line);
            continue;
        }

        /* Expand glob patterns in arguments */
        char* expanded[MAX_TOKENS];
        int expanded_count;
        expand_globs(args, expanded, &expanded_count);

        /* Execute external command */
        pid_t pid = fork();
        if (pid == 0) {
            /* Child process */
            signal(SIGINT, SIG_DFL);  /* Reset signal handler for child */
            
            /* Handle input redirection */
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd == -1) { 
                    perror("input"); 
                    exit(1); 
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            /* Handle output redirection */
            if (outfile) {
                int flags = O_WRONLY | O_CREAT;
                if (append) flags |= O_APPEND;
                else flags |= O_TRUNC;
                
                int fd = open(outfile, flags, 0644);
                if (fd == -1) { 
                    perror("output"); 
                    exit(1); 
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            /* Execute command */
            if (execvp(expanded[0], expanded) == -1) {
                perror("execvp");
                exit(1);
            }
        } else if (pid > 0) {
            /* Parent process */
            if (background) {
                /* Background job */
                printf("[%d] %d\n", job_count, pid);
                add_job(pid, line);
            } else {
                /* Foreground job - wait for completion */
                int status;
                waitpid(pid, &status, 0);
            }
        } else {
            /* Fork failed */
            perror("fork");
        }

        free(line);
    }

    /* Save command history before exiting */
    #if READLINE_LIB
    write_history(".ccsh_history");
    #endif

    return 0;
}
