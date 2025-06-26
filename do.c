#include <stdio.h>
#include <stdlib.h>

void main_sh_loop();
char* sh_read_line();
//char** sh_eval_line();
//int sh_exec_line();

int main(int argc, char *argv[]){
    
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
    printf("> ");
    char *line = sh_read_line();
    printf("%s", line);
    //char **args = sh_eval_line(char *line);
    //int status = sh_exec_line(char **args);

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

    unsigned int idx = 0; // index to keep track of the buffer
    int c = 0;
    unsigned int buf_size = RL_BUFSIZE;
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
