#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#define clear() printf("\033[H\033[M")
void main_sh_loop();
char* sh_read_line();
char** sh_eval_line();
int sh_exec_line();

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
    puts("> ");
    char *line = sh_read_line();
    printf("%s\n", line);
    char **args = sh_eval_line(line);
    // If the string is not terminated properly
    if (args == NULL){ return; } 
    for (int i = 0; args[i] != NULL; i++){
        printf("\narg[%d] = %s\n", i, args[i]);
    }
    
    //int status = sh_exec_line(args);

    free(line);
    free(args);
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
     * learned about asan today, thanks to $(CFLAGS)
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
    if (state == QUOTES){
        fprintf(stderr, "Error: Unterminated quotes\n");
        return NULL;
    }

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
    return tokens; 
}
