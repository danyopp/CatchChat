The following program includes a python chat server and a c chat client. Starting the server and directing the client to access the specific port the server is listening on will allow simple text messages to be sent back and forth between the two machines.

Update plans:
	multi-threaded listening. 


To Run program:
1. compile c program! Do this by typing "gcc chatclient.c -o chatclient" into the terminal in the same directory as the file

2. begin python server by typing "python chatserver.py 33233" to start the server on port 33233. Any available port can be used. Server must be started before client. 

3. Begin chat client on the same port using the servers address. "./chatclient localhost 33233" would begin the chat client on the same computer in a different terminal. 

4. Follow the prompts in the program. Client sends first message. Type \quit to end chat in either terminal.

5. ctrl+c to stop server



