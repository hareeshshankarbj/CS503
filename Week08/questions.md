1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client relies on a special end-of-message marker (RDSH_EOF_CHAR) that the server adds to the output. It repeatedly calls recv(), checking to see if the last byte received is the EOF marker; if so, the client knows the entire output has been received.
To handle partial reads and ensure full transmission, you can:
Loop on recv(): Read until the termination marker is detected.
Length-prefixing: Alternatively, you could send the message's length first, so the client knows how many bytes to expect.
Delimiters: Use specific delimiter characters (such as our EOF marker) to indicate the end of a message.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A networked shell protocol must define explicit delimiters to indicate message boundaries. For example, our solution uses a null byte ('\0') at the end of each command sent by the client and an EOF marker (like RDSH_EOF_CHAR, set to 0x04) at the end of the output sent by the server. This way, even though TCP is a continuous stream, the receiver knows when a command or response is complete.

Without explicitly handling message boundaries, such as with delimiters or length-prefixing, the receiver may encounter partial messages or multiple messages concatenated together. This can lead to command corruption, incomplete processing, and critical communication errors.

3. Describe the general differences between stateful and stateless protocols.

Stateful protocols maintain context between requests—each interaction is associated with a session, allowing the server to remember previous interactions. This facilitates the provision of a continuous, contextual service (for example, FTP or database connections), but it complicates scaling and resource management.

Stateless protocols, on the other hand, treat each request independently, with no inherent relationship to previous requests. This simplifies server design and scaling (as with HTTP), but it may necessitate additional mechanisms (such as cookies or tokens) if session context is required.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

UDP is used when speed and low latency are more important than reliability. It is best suited for real-time applications (gaming, VoIP, streaming), broadcasting, and basic protocols such as DNS. Unlike TCP, there is no connection overhead, making it faster but less reliable.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Sockets are the operating system's primary interface for network communication. The Berkeley Sockets API (in C/C++) enables applications to create, bind, listen, connect, send, and receive data over networks via system calls such as socket(), bind(), listen(), accept(), connect(), send(), and recv().
