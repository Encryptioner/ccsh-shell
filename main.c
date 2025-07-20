// main.c - ccsh: C Compact Shell with features like pipes, redirection, jobs, aliases, plugins

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <glob.h>
#include <fcntl.h>
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

// Color codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"

// Job structure
typedef struct { pid_t pid; char command[1024]; } Job;
static Job jobs[MAX_JOBS];
static int job_count = 0;

// Alias structure
typedef struct { char* name; char* value; } Alias;
static Alias aliases[MAX_TOKENS];
static int alias_count = 0;

// Signal handler for Ctrl+C
void sigint_handler(int sig) {
    printf("\nUse 'exit' to quit.\n");
    fflush(stdout);
}

// Job handling functions
void add_job(pid_t pid, const char* cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, sizeof(jobs[job_count].command)-1);
        jobs[job_count++].command[sizeof(jobs[job_count-1].command)-1] = '\0';
    }
}
void check_background_jobs() {
    for (int i = 0; i < job_count; i++) {
        int status;
        pid_t r = waitpid(jobs[i].pid, &status, WNOHANG);
        if (r > 0) {
            printf("[done] %s\n", jobs[i].command);
            jobs[i] = jobs[--job_count];
            i--;
        }
    }
}
void list_jobs() {
    if (!job_count) { printf("No background jobs.\n"); return; }
    for (int i = 0; i < job_count; i++)
        printf("[%d] %d %s\n", i, jobs[i].pid, jobs[i].command);
}

// Alias functions
void load_aliases(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line,"alias ",6)==0) {
            char* name = strtok(line+6,"=\"\n");
            char* val = strtok(NULL,"\"\n");
            if (name && val) {
                aliases[alias_count].name = strdup(name);
                aliases[alias_count].value = strdup(val);
                alias_count++;
            }
        }
    }
    fclose(f);
}
void replace_aliases(char** args) {
    for (int i = 0; i < alias_count; i++)
        if (strcmp(args[0], aliases[i].name)==0)
            args[0] = aliases[i].value;
}

// print "help" info
void print_help() {
    printf("Supported features:\n");
    printf("  Built-ins: cd, pwd, exit, help, fg, jobs, source\n");
    printf("  I/O: <, >, >>\n");
    printf("  Background jobs: &\n");
    printf("  Globbing: *, ?\n");
    printf("  Aliases (alias name=\"value\")\n");
    printf("  Plugins: source <file>\n");
    printf("  History & arrow keys\n");
}

// Sourcing scripts and .ccshrc file
void execute_command_line(char* cmdline); // Forward declaration
void source_file(const char* file) {
    FILE* f = fopen(file, "r");
    if (!f) { fprintf(stderr, "source: cannot open %s\n", file); return; }
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) execute_command_line(buf);
    fclose(f);
}

// Command parsing helpers
void parse_command(char* line, char** args, int* background, 
                   char** infile, char** outfile, int* append) {
    *background = *append = 0;
    *infile = *outfile = NULL;
    int i=0;
    for (char* tok = strtok(line, " \t\n"); tok; tok=strtok(NULL," \t\n")) {
        if (!strcmp(tok,"<")) infile = &tok[strlen(tok)]; // consume skip
        else if (!strcmp(tok,">")) { outfile = &tok[strlen(tok)]; *append=0; }
        else if (!strcmp(tok,">>")) { outfile = &tok[strlen(tok)]; *append=1; }
        else if (!strcmp(tok,"&")) *background=1;
        else args[i++]=tok;
    }
    args[i]=NULL;
}
void expand_globs(char** args, char** out, int* count) {
    glob_t result;
    result.gl_offs=0; glob("", GLOB_DOOFFS, NULL, &result);
    *count = 0;
    for (int i=0; args[i]; i++) {
        if (strchr(args[i],'*')||strchr(args[i],'?')) {
            glob(args[i], GLOB_TILDE|GLOB_APPEND, NULL, &result);
        } else {
            out[(*count)++] = args[i];
        }
    }
    out[*count] = NULL;
}

// Handle single line (used by main loop and source)
void execute_command_line(char* line_raw) {
    char* line = strdup(line_raw);
    int background=0, append=0;
    char *infile=NULL, *outfile=NULL;
    char* args[MAX_TOKENS];

    parse_command(line, args, &background, &infile, &outfile, &append);
    if (!args[0]) { free(line); return; }

    if (!strcmp(args[0],"exit")) exit(0);
    if (!strcmp(args[0],"cd")) {
        if (chdir(args[1]?args[1]:getenv("HOME"))) perror("cd");
        free(line); return;
    }
    if (!strcmp(args[0],"pwd")) {
        char cwd[1024]; if (getcwd(cwd,sizeof(cwd))) printf("%s\n",cwd); else perror("pwd");
        free(line); return;
    }
    if (!strcmp(args[0],"jobs")) { list_jobs(); free(line); return; }
    if (!strcmp(args[0],"fg")) {
        int id = args[1]?atoi(args[1]):-1;
        if (id>=0 && id<job_count) waitpid(jobs[id].pid,NULL,0);
        else fprintf(stderr,"Invalid job id\n");
        free(line); return;
    }
    if (!strcmp(args[0],"help")) { print_help(); free(line); return; }
    if (!strcmp(args[0],"source") && args[1]) { source_file(args[1]); free(line); return; }

    replace_aliases(args);
    char* expanded[MAX_TOKENS];
    int n;
    expand_globs(args, expanded, &n);

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); free(line); return; }
    if (!pid) {
        signal(SIGINT, SIG_DFL);
        if (infile) { int fd=open(infile,O_RDONLY); if (fd<0){perror("in"); exit(1);} dup2(fd,0); close(fd);}
        if (outfile) { int fd=open(outfile,O_WRONLY|O_CREAT|(append?O_APPEND:O_TRUNC),0644); 
                       if (fd<0){perror("out"); exit(1);} dup2(fd,1); close(fd);}
        execvp(expanded[0], expanded);
        perror("exec");
        exit(1);
    }
    if (background) {
        add_job(pid, line_raw);
        printf("[%d] %d\n", job_count-1, pid);
    } else waitpid(pid, NULL, 0);
    free(line);
}

int main() {
    signal(SIGINT, sigint_handler);

    char rc[512];
    snprintf(rc,sizeof(rc),"%s/.ccshrc",getenv("HOME"));
    source_file(rc);
    load_aliases(rc);

    #if READLINE_LIB
    read_history(".ccsh_history");
    #endif

    for (;;) {
        #if READLINE_LIB
        char* line = readline(getenv("CCSH_PROMPT") ? getenv("CCSH_PROMPT") : COLOR_GREEN "ccsh> " COLOR_RESET);
        if (!line) break;
        if(*line) add_history(line);
        execute_command_line(line);
        free(line);
        #else
        printf(COLOR_GREEN "ccsh> " COLOR_RESET);
        char buf[1024];
        if (!fgets(buf,sizeof(buf),stdin)) break;
        execute_command_line(buf);
        #endif

        check_background_jobs();
    }

    #if READLINE_LIB
    write_history(".ccsh_history");
    #endif

    return 0;
}
