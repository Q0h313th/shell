#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

void main_sh_loop();
char* sh_read_line();
char** sh_eval_line();
int sh_exec_line();
int fork_and_exec();
int sh_cd(char **args);
int sh_help();
int sh_exit();
int sh_history(char **args);

// global history array
#define MAX_HISTORY_LEN 100
char *HISTORY[MAX_HISTORY_LEN];
size_t HISTORY_COUNT = 0;

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "history"
};

// array of function pointers that map to builtin_str
int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_help,
    &sh_exit,
    &sh_history
};

int num_of_builtins(void){
    return sizeof(builtin_str) / sizeof(char *);
}

/* builtin function implementation starts at line 230 */

int main(){
    
    while(1){
        main_sh_loop();
    }
    
    return EXIT_SUCCESS;    
}

void main_sh_loop(void){
    /* 
     * on a high level, what does a shell really do?
     * read a line -----> sh_read_line()
     * evaluate the line -----> sh_eval_line()
     * execute the line -----> sh_exec_line()
    */
    
    /*
     * i want sh_read_line() to return an array of the arguments passed to cli
     * i want sh_eval_line() to return the tokens
     * i want to sh_exec_line to fork and execute the tokens
     */
    int index = 0; // purely to track the history
    puts("\n> ");
    char *line = sh_read_line();
   
    index  = HISTORY_COUNT % MAX_HISTORY_LEN;
    if (HISTORY[index]){ free(HISTORY[index]); }
    HISTORY[index] = strdup(line);
    HISTORY_COUNT++;
    
    char **args = sh_eval_line(line);

    // if args are NULL, just dont call exec_line
    if (args == NULL){ free(line); return; }
    
    int status = sh_exec_line(args);
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }

    free(args);
    free(line);
}


#define RL_BUFSIZE 1024
char *sh_read_line(void){
    /*
     * malloc the space
     * count the bytes read in
     * if count > space
     * realloc the space
    */

    size_t idx = 0; // index to keep track of the buffer
    int c = 0;
    size_t buf_size = RL_BUFSIZE;
    /* 
     * extra byte is assigned to prevent off by one error
     * the null byte is written the 1025th index
    */
    char *ptr = malloc(RL_BUFSIZE + 1);
    if  (ptr == 0){ perror("malloc"); return NULL; }
    else{
        while ((c = getchar()) != EOF && c != '\n'){
            if (idx >= buf_size - 1){
                buf_size += RL_BUFSIZE;
                char *tmp = realloc(ptr, buf_size);
                if (tmp == 0){ perror("realloc"); return NULL; }
                ptr = tmp;
            }
            else { ptr[idx] = c; idx++; }
        }
        ptr[idx] = '\0';
        return ptr;
    }
}


// this function returns a pointer to an array of pointers that point to the tokens
#define TOKEN_BUF_SIZE 64 
char **sh_eval_line(char *ptr){
    /*
     * start with a pointer to the beginning of the string
     * iterate character. each time you find one, you have a token from idx-1 to beginning.
     * idx++
     * step 2
    */

    typedef enum TokenCase { NORMAL, QUOTES } tokenCase;
    size_t buf_size = TOKEN_BUF_SIZE;
    size_t buf_counter = 0;
    size_t idx = 0; // iterates through the characters
    char **tokens = malloc(TOKEN_BUF_SIZE * sizeof(char *)); // dont forget to free this in the main function! 
    size_t len = strlen(ptr);
    size_t start = 0; // keeps track of the start of a token
    char *token = NULL;
    size_t size = 0;


    if (tokens == NULL){ perror("malloc"); return NULL; }

    tokenCase state = NORMAL;

    for (idx = 0; idx < len; idx++){
        char c = ptr[idx];

        switch (state){
        case NORMAL:
            if (c == '"'){
                state = QUOTES;
                start = idx + 1;
                continue;
            }
            if (isspace(c)){
                if (start == idx){
                    start = idx + 1;
                    break;
                } 
                size = (idx) - (start); 
                if ((token = malloc(size + 1)) == NULL){ perror("malloc"); return NULL; } 
                ptr[idx] = '\0'; 
                if (idx < start){ fprintf(stderr, "SHENANIGANS!!!\n"); return NULL; }
                strncpy(token, &ptr[start], size);
                token[size] = '\0';
                tokens[buf_counter] = token;
                start = idx + 1;
                buf_counter++;

                if (buf_counter >= buf_size){
                    buf_size += TOKEN_BUF_SIZE;
                    if ((tokens = realloc(tokens, buf_size * sizeof(char *))) == NULL){ perror("realloc"); return NULL; }
                }
            }
        break;

        case QUOTES:
            if (c == '"' || c == '\0'){
                size = idx - start;
                if (idx < start){ fprintf(stderr, "SHENANIGANS!!!\n"); return NULL; }
                if ((token = malloc(size + 1)) == NULL){ perror("malloc"); return NULL; }
                ptr[idx] = '\0';
                strncpy(token, &ptr[start], size);
                token[size] = '\0';
                tokens[buf_counter] = token;
                start = idx + 1;
                buf_counter++;

                if (buf_counter >= buf_size){
                    buf_size += TOKEN_BUF_SIZE;
                    if ((tokens = realloc(tokens, buf_size * sizeof(char *))) == NULL){ perror("realloc"); return NULL; }
                }
                state = NORMAL;
            }     
        break;
        }
    }
    // check for improper use of quotes
    if (state == QUOTES){
        fprintf(stderr, "Error: Unterminated quotes\n");
        return NULL;
    }
    
    // the last token which has no delimiter
    if (len > start){
        size = len - start;
        if ((token = malloc(size + 1)) == NULL){ perror("malloc"); return NULL; }
        strncpy(token, &ptr[start], size);
        token[size] = '\0';
        tokens[buf_counter] = token;
        buf_counter++;

        if (buf_counter >=  buf_size){
            buf_size += TOKEN_BUF_SIZE;
            if ((tokens = realloc(tokens, buf_size * sizeof(char *))) == NULL){ perror("realloc"); return NULL; }
        }
    }
    tokens[buf_counter] = NULL;
    if (tokens == NULL){ return NULL; }
    return tokens; 
}

int fork_and_exec(char **args){
    int status;
    pid_t pid = fork();

    switch(pid){
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        if (execvp(args[0], args) == -1){ perror("execvp"); exit(EXIT_FAILURE); }
        break;
    default:
        waitpid(pid, &status, WUNTRACED);
    }
    return 1;
}

int sh_cd(char **args){
   if (args[1] == NULL){
       fprintf(stderr, "Expected argv[1] to be name of directory\n");
       return 0;
   }
   if (chdir(args[1]) != 0){ perror("chdir"); return 0; }
   return 1;
}

int sh_help(void){
    puts("This shell supports the following builtins:\n");
    for (int i = 0; i < num_of_builtins(); i++){
        printf("%s\n", builtin_str[i]);
    }
    puts("You may use the man command for infomation about other binaries.\n");
    return 1;
}

int sh_exit(void){
    exit(EXIT_SUCCESS);
}

int sh_exec_line(char **args){
    int i;
    // check if all the args are NULL
    if (args[0] == NULL || args == NULL){
        return 0;
    }
    
    for (i = 0; i < num_of_builtins(); i++){
        if (strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i]) (args);
        }
    }
    return fork_and_exec(args);
}

int sh_history(char **args){
    // no keyword is specified, so printout the entire history
    int found = 0;
    if (args[1] == NULL){
        for (size_t i = 0; i < HISTORY_COUNT - 1 && i < MAX_HISTORY_LEN ; i++){
            printf("%s\n", HISTORY[i]);
        }
        found = 1;
    }
    else if (args[1] != NULL){
        char *keyword = args[1];
        for (size_t i = 0; i < HISTORY_COUNT -1 && i < MAX_HISTORY_LEN; i++){
            // when HISTORY[i] becomes NULL, strdup crashes
            if (HISTORY[i] == NULL){ continue; } 
            if (strstr(HISTORY[i], keyword)){
                printf("%s\n", HISTORY[i]);
            }
        }
        found = 1;
    }
    if (!found){ fprintf(stderr, "Keyword not found\n"); return 0; }

    return 1;
}
