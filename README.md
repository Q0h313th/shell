## SHELL

* It's a work in progress.
* Im trying to make it super secure, hence all the gcc flags
* I do talk to myself in the comments.
* I know it looks eerily similar to that one tutorial, because it is. Im trying to write it in my way but somehow always come to the conclusion that its done better their way. However, I plan on make it different as I continue writing.
* soooo, this shell now has support for quotation marks, and it works perfectly well. I plan on writing some tests so I can really push asan to it's limit, but as of now, and what ive tested manually, it seems to be doing fine. although, I do need to look at why im getting a new line after the prompt in the shell lol.
* one test is to be written for if I can enter more than 64 tokens and it'll realloc the tokens accordingly. I can also just use pwntools.
* finished writing the fork and exec function for commands, now ive to figure out how im going to fork for pipes and redirections. The plan right now is to only support at max one redirection, like `cat hello.txt | ./a.out`.
* also, add some bash builtins to this shell
