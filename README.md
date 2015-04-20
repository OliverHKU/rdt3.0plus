# rdt3.0plus
This is a network project which built an internet protocal from scratch using the "Stop-and-Wait" logic.
rdt3.0plus (this project) is a reliable transportation layer protocal on top of UDP. It basically implements rdt3.0, but adds user-defined sender window (window size < 128) to achieve a pipelined speedup.

<b>- API</b><br>
This protocal provides 6 C/C++ functions:
  - rdt_socket()
  - rdt_bind()
  - rdt_target()
  - rdt_send()
  - rdt_recv()
  - rdt_close()<br>
<br>Please refer to the two example test programs for detailed usages.
<b>Handshake: </b> The suggested handshake method involves three function calls on both the server and the client's sides. Client/sender calls rdt_send() twice to send file size and name to the receiver, and the server/receiver, upon receive correct information from the sender, calls rdt_send() to indicate start of file transfer.
<b>Parameters:</b>
There are 3 parameters you can set before compilation:<br>
    - PAYLOAD: maximum size of message in a data packet
    - TIMEOUT: timeout length
    - W: window size. Default is 5.

<b>- How to compile?</b><br>
simply use the "make" command.
*Make sure that you have your server/client programs ready before the compilation. There are two default test server/client programs. If you've got your own programs, please replace the original ones with yours.

<b>- How to run the program?</b><br>
After compilation, there will be two executables generated, namely "tserver" and "tclient". You can use the following command to start server/client:
  - sh run-server.sh <packet loss rate> <packet error rate> <client host name>
  - sh run-client.sh <packet loss rate> <packet error rate> <server host name> <file name>
Please make sure you have the directory /Store ready on the server side before running the program.
Please always start server before client.
