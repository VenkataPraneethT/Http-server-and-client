Http-server-and-client
======================

Http Server and Client code that supports HTTP PUT, GET and POST methods. It supports both IPv4 and IPv6


To build and run the server code please follow the below instructions:
======================================================================
Build Instructions:
===================
To compile the code it is sufficient to run make file for the server.

User Instructions:
==================
First the server can be started by providing three arguments :
First argument is the compiled file to run ex: ./HttpServer
Second argument is the servername ex: www.google.com
Third argument is the port ex: 80
The entire thing should look like this "./HttpServer www.google.com 80"

To build and run the client code please follow the below instructions:
======================================================================
Build instructions:
===================
To compile the code it is sufficient to run make file for the client .

User Instructions:
==================
For the programme to run the user will asked for 3 arguments. They are as follows:
First argument is the compiled file to be run ex: ./HttpClient
Second argument is the servername to access ex: www.google.com
Third argument is the port number to be accessed. Ex: 80
The entire thing should like this "./HttpServer www.google.com 80"
