1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  The child process created by the fork system call is an exact replica of its parent. The parent (shell) is kept active after the child executes the command when the fork is used before execvp. Execvp would completely replace the shell process without a fork, ending it when the instruction was finished. The shell can handle numerous commands while preserving its own execution environment thanks to the fork's ability to separate processes.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  The parent process checks to see if pid is less than 0 and prints an error (e.g., perror("fork")) and returns an error code (e.g., ERR_EXEC_CMD) if fork() fails, returning -1. This guarantees that even when system resources are depleted, the shell will continue to function and not crash.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  The execvp() searches the PATH environment variable to find the executable. It sequentially searches every directory in PATH for the specified command.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  Wait() prevents zombie processes (the late processes that remain after shutdown) by stopping the parent process until the child stops operating. In the absence of wait(), the child's exit status would remain unreported and the shell would not be aware of when the child had finished. It also makes it possible for commands to be executed in the correct order.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  The child process's exit code is returned by WEXITSTATUS(status) from the status integer. This is required to propagate error codes to the shell's logic and determine whether the command was successful (e.g., 0 for success, non-zero for errors).

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  The in_quote flag is used by the parser to record quotes. Spaces inside quotes are regarded as a single token whenever a " appears. This guarantees that commands like echo "Hello World" are handled as a single argument (Hello World) as opposed to multiple tokens. We must quote arguments that contain spaces or other unusual characters.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  Assignment Part 1: Use command_list_t (e.g., cmd1 | cmd2) to handle piped instructions.

Assignment Part 2: Pipe support was eliminated and single commands were simplified to cmd_buff_t.


Making sure quoted arguments (e.g., echo "Hello World") were appropriately tokenized without breaking extreme cases like empty strings or unterminated quotes was one of the challenges.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are asynchronous notifications that control process events (e.g., SIGINT for Ctrl+C). They don't carry data payloads and are lightweight, in contrast to IPC (pipes/sockets).

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  Request for graceful termination (which can be detected and resolved) is a SIGINT.

The default termination signal, or SIGTERM, is what makes cleanup possible.

SIGKILL: Resulting in instant termination (untraceable).

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  The process is paused (e.g., Ctrl+Z). Because it is used for the job control in the shells, it cannot be detected or ignored like SIGINT.
