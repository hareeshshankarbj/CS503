1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?
asf
    > **Answer**:  It is a good choice because the input is read line by line and it stops at new line(\n). It also handles end of file better, which also alligns with the shells need to process commands as complete lines.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  malloc() offers dynamic memory allocation, which is necessary for scalability. malloc() also allows precise allocation based on SH_CMD_MAX, which ensures a efficient memory use and avoiding hardcoded limits.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  To ensure commands and arguments are parsed accurately, timing spaces are important. Without timing:
    1. ls might fail
    2. Pipes like cmd1 | cmd2 could give invalid command.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:
    1. file.txt :- Redirect STDOUT to a file. CHallenge: Handling file creation and truncation.
    2. error.log: Redirect STDERR to a file: Challenges: avoiding race situations while writing to the same log file simultaneously, isolating error messages, and separating the STDERR and STDOUT streams.
    3. input.txt : Redirect STDIN froma file: Challenges: substituting file input for keyboard input while maintaining interactive functionality for commands that need terminal input.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection: Attaches files to the input/output of a command (e.g., >, <, 2>). It modifies the source of input or destination of output. Piping: Joins one command's output to another's input (e.g., cmd1 | cmd2). 
    Commands are dynamically chained in memory without the need for intermediate files. The goal of piping is to allow command composition in a pipeline, whereas redirection works with files or devices.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  Debugging is made easier by error isolation, which prevents errors (STDERR) from corrupting regular output (STDOUT).
    Selective Handling: Users have the ability to independently log and reroute errors (e.g., 2> errors.txt).
    Clarity: Even when STDOUT is quiet, critical error messages (such as > /dev/null) are still visible.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  Error Detection: Use waitpid() or a comparable function to verify the command's exit status (non-zero equals failure). Allow users to merge streams using 2>&1 (for example, command > output.log 2>&1).
    Implementation: To reroute STDERR to STDOUT, use dup2(STDOUT_FILENO, STDERR_FILENO) before execvp(). Flexibility: Offer the option of combining, but for clarity, keep streams distinct by default.
