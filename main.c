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
#include <ctype.h>      /* Character classification: tolower */

/* Readline library support - cross-platform detection */
#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #ifdef HAVE_READLINE
        #include <readline/readline.h>  /* Interactive command line editing (readline, add_history) */
        #include <readline/history.h>   /* Command history management */
        #define READLINE_LIB 1
    #else
        #define READLINE_LIB 0
    #endif
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

/* Function declarations */
void generate_prompt(char* prompt, size_t prompt_size);

/* Signal handler for Ctrl+C (SIGINT) */
void sigint_handler(int sig) {
    (void)sig;  /* Suppress unused parameter warning */
    char prompt[1024];
    generate_prompt(prompt, sizeof(prompt));
    printf("\nUse 'exit' to quit.\n%s", prompt);
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
 * Simple pattern matching function for grep
 * @param text Text to search in
 * @param pattern Pattern to search for
 * @param case_insensitive Whether to ignore case
 * @return 1 if pattern matches, 0 otherwise
 */
int simple_match(const char* text, const char* pattern, int case_insensitive) {
    if (!text || !pattern) return 0;
    
    char* text_copy = strdup(text);
    char* pattern_copy = strdup(pattern);
    
    if (case_insensitive) {
        /* Convert to lowercase for case-insensitive matching */
        for (int i = 0; text_copy[i]; i++) {
            text_copy[i] = tolower(text_copy[i]);
        }
        for (int i = 0; pattern_copy[i]; i++) {
            pattern_copy[i] = tolower(pattern_copy[i]);
        }
    }
    
    int result = strstr(text_copy, pattern_copy) != NULL;
    
    free(text_copy);
    free(pattern_copy);
    return result;
}

/**
 * Built-in grep command implementation
 * @param args Command arguments
 * @return 0 on success, 1 on error
 */
int builtin_grep(char** args) {
    if (!args[1]) {
        fprintf(stderr, "Usage: grep [options] pattern [file...]\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -i    Ignore case\n");
        fprintf(stderr, "  -n    Show line numbers\n");
        fprintf(stderr, "  -v    Invert match (show non-matching lines)\n");
        fprintf(stderr, "  -c    Count matching lines only\n");
        return 1;
    }
    
    int case_insensitive = 0;
    int show_line_numbers = 0;
    int invert_match = 0;
    int count_only = 0;
    char* pattern = NULL;
    int file_count = 0;
    char** files = NULL;
    
    /* Parse options and arguments */
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-' && args[i][1] != '\0') {
            /* Option */
            for (int j = 1; args[i][j] != '\0'; j++) {
                switch (args[i][j]) {
                    case 'i':
                        case_insensitive = 1;
                        break;
                    case 'n':
                        show_line_numbers = 1;
                        break;
                    case 'v':
                        invert_match = 1;
                        break;
                    case 'c':
                        count_only = 1;
                        break;
                    default:
                        fprintf(stderr, "grep: invalid option -- '%c'\n", args[i][j]);
                        return 1;
                }
            }
        } else {
            /* Non-option argument */
            if (!pattern) {
                pattern = args[i];
            } else {
                /* File argument */
                file_count++;
                files = realloc(files, file_count * sizeof(char*));
                files[file_count - 1] = args[i];
            }
        }
    }
    
    if (!pattern) {
        fprintf(stderr, "grep: no pattern specified\n");
        free(files);
        return 1;
    }
    
    /* If no files specified, read from stdin */
    if (file_count == 0) {
        files = malloc(sizeof(char*));
        files[0] = NULL;  /* NULL means stdin */
        file_count = 1;
    }
    
    /* Process each file */
    for (int file_idx = 0; file_idx < file_count; file_idx++) {
        FILE* file = stdin;
        char* filename = files[file_idx];
        
        if (filename) {
            file = fopen(filename, "r");
            if (!file) {
                fprintf(stderr, "grep: %s: No such file or directory\n", filename);
                continue;
            }
        }
        
        char line[4096];
        int line_number = 0;
        int match_count = 0;
        
        while (fgets(line, sizeof(line), file)) {
            line_number++;
            
            /* Remove newline */
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
            }
            
            int matches = simple_match(line, pattern, case_insensitive);
            if (invert_match) matches = !matches;
            
            if (matches) {
                match_count++;
                if (!count_only) {
                    if (file_count > 1 && filename) {
                        printf("%s:", filename);
                    }
                    if (show_line_numbers) {
                        printf("%d:", line_number);
                    }
                    printf("%s\n", line);
                }
            }
        }
        
        if (count_only && filename) {
            printf("%d\n", match_count);
        } else if (count_only && !filename) {
            printf("%d\n", match_count);
        }
        
        if (filename) {
            fclose(file);
        }
    }
    
    free(files);
    return 0;
}

/**
 * Generate prompt with current directory
 * @param prompt Buffer to store the prompt
 * @param prompt_size Size of prompt buffer
 */
void generate_prompt(char* prompt, size_t prompt_size) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        /* Try to make the path more readable by shortening home directory */
        const char* home = getenv("HOME");
        if (home && strncmp(cwd, home, strlen(home)) == 0) {
            /* Replace home directory with ~ */
            snprintf(prompt, prompt_size, "ccsh:~%s> ", cwd + strlen(home));
        } else {
            snprintf(prompt, prompt_size, "ccsh:%s> ", cwd);
        }
    } else {
        /* Fallback if getcwd fails */
        snprintf(prompt, prompt_size, "ccsh> ");
    }
}

/**
 * Expand tilde (~) to home directory
 * @param path Path that may contain tilde
 * @param expanded Buffer to store expanded path
 * @param expanded_size Size of expanded buffer
 * @return 0 on success, -1 on error
 */
int expand_tilde(const char* path, char* expanded, size_t expanded_size) {
    if (!path || !expanded) return -1;
    
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (!home || strlen(home) == 0) {
            fprintf(stderr, "cd: HOME environment variable not set\n");
            return -1;
        }
        
        if (path[1] == '\0' || path[1] == '/') {
            /* ~ or ~/path */
            if (path[1] == '/') {
                snprintf(expanded, expanded_size, "%s%s", home, path + 1);
            } else {
                snprintf(expanded, expanded_size, "%s", home);
            }
        } else {
            /* ~username - not supported in this simple implementation */
            fprintf(stderr, "cd: ~username not supported\n");
            return -1;
        }
    } else {
        /* No tilde, copy as-is */
        strncpy(expanded, path, expanded_size - 1);
        expanded[expanded_size - 1] = '\0';
    }
    
    return 0;
}

/**
 * Display help information about shell features
 */
void print_help() {
    printf("ccsh - Compact C Shell\n");
    printf("Supported features:\n");
    printf("  Built-in commands: cd, pwd, exit, help, fg, jobs, alias, unalias, path, which, grep\n");
    printf("  Tilde expansion: ~ expands to home directory (e.g., cd ~, cd ~/Documents)\n");
    printf("  Dynamic prompt: Shows current directory in prompt (e.g., ccsh:~> ccsh:/usr/bin>)\n");
    printf("  External programs: All programs in PATH (e.g., sudo, ls, cat, etc.)\n");
    printf("  I/O Redirection: < (input), > (output), >> (append)\n");
    printf("  Background jobs: & (with fg and jobs to control)\n");
    printf("  Globbing: *, ? (filename pattern matching)\n");
    printf("  Aliases: alias name='value', unalias name\n");
    printf("  Command history with arrow keys (if readline available)\n");
    printf("  Signal handling: Ctrl+C to interrupt\n");
    printf("\nExamples:\n");
    printf("  path                    - Show PATH environment variable\n");
    printf("  which ls                - Find location of ls command\n");
    printf("  which sudo              - Find location of sudo command\n");
    printf("  cd ~                    - Change to home directory\n");
    printf("  cd ~/Documents          - Change to Documents in home directory\n");
    printf("  sudo ls -la             - Run sudo with arguments\n");
    printf("  ls *.txt > files.txt   - Redirect output to file\n");
    printf("  sleep 10 &             - Run command in background\n");
    printf("  grep pattern file.txt   - Search for pattern in file\n");
    printf("  grep -i -n hello *.txt - Case-insensitive search with line numbers\n");
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
    char prompt[1024];
    while (1) {
        /* Generate dynamic prompt with current directory */
        generate_prompt(prompt, sizeof(prompt));
        
        /* Get command line input */
        #if READLINE_LIB
        line = readline(prompt);
        if (line == NULL) {
            /* EOF or readline error - exit gracefully */
            printf("\n");
            break;
        }
        #else
        static char buffer[1024];
        printf("%s", prompt);
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
            char expanded_path[1024];
            const char* target = args[1] ? args[1] : "~";
            
            if (expand_tilde(target, expanded_path, sizeof(expanded_path)) == 0) {
                if (chdir(expanded_path) != 0) {
                    perror("cd");
                }
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
        
        /* Show PATH environment variable */
        if (strcmp(args[0], "path") == 0) {
            const char* path = getenv("PATH");
            if (path) {
                printf("PATH=%s\n", path);
            } else {
                printf("PATH environment variable not set\n");
            }
            free(line);
            continue;
        }
        
        /* Which command - find executable in PATH */
        if (strcmp(args[0], "which") == 0) {
            if (!args[1]) {
                fprintf(stderr, "Usage: which <command>\n");
                free(line);
                continue;
            }
            
            const char* path_env = getenv("PATH");
            if (!path_env) {
                fprintf(stderr, "PATH environment variable not set\n");
                free(line);
                continue;
            }
            
            char* path_copy = strdup(path_env);
            char* dir = strtok(path_copy, ":");
            int found = 0;
            
            while (dir != NULL) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[1]);
                
                if (access(full_path, X_OK) == 0) {
                    printf("%s\n", full_path);
                    found = 1;
                    break;
                }
                
                dir = strtok(NULL, ":");
            }
            
            if (!found) {
                fprintf(stderr, "which: %s not found\n", args[1]);
            }
            
            free(path_copy);
            free(line);
            continue;
        }
        
        /* Built-in grep command */
        if (strcmp(args[0], "grep") == 0) {
            builtin_grep(args);
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
