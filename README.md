## SHELL

* It's a work in progress.
* one test is to be written for if I can enter more than 64 tokens and it'll realloc the tokens accordingly. I can also just use pwntools.
* finished writing the fork and exec function for commands, now ive to figure out how im going to fork for pipes and redirections. The plan right now is to only support at max one redirection, like `cat hello.txt | ./a.out`.
* so im done writing stuff for basic builtins; once the parsing was over everything was pretty much straightforward.
