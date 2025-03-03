1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation collects each child's PID as it forks and then loops through them with waitpid(), ensuring that every child completes before the shell accepts another command. Without these waitpid() calls, terminated children become zombies, and system resources may be depleted, resulting in unpredictable behavior.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

The original pipe file descriptors are left open after dup2(), unless explicitly closed. Leaving unused pipe ends open can prevent the receiving process from detecting an end-of-file condition, causing it to hang while waiting for additional data. It can also cause resource leaks by exhausting the available file descriptors.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd must be implemented as a built-in because it changes the current working directory of the shell process. If cd were an external command, it would run in a separate process, and any directory changes would be lost when the process exited. This would prevent the shell from changing its own working directory, resulting in incorrect behavior for future commands.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support an unlimited number of piped commands, switch from a fixed-size array (CMD_MAX) to a dynamically allocated array. For example, you can allocate space for a set number of commands and then use realloc() to expand the array as more commands are parsed. The tradeoffs include:
Memory Management Complexity: To avoid memory leaks, allocate and deallocate memory carefully.

Frequent reallocations may result in a performance cost, particularly if the number of commands grows large.

Resource Limits: Allowing an arbitrarily large number of commands may exhaust system resources if not properly limited.
