Currently, the `kevm-client` is based on two servers:
  - HTTP Server
  - K Server

When `kevm-client` is started, both servers launch in different processes and a socket connection is made between them.

Clients like Truffle can send jsonrpc messages over HTTP POST requests or through websockets to the HTTP Server. The http server will then take the jsonrpc and pass it to the K server through the socket connection. Once the message is processed, the result is sent back to the HTTP Server which builds a response for the client.

The `gflags` library is used to parse the command line flags. A flag can be created with functions like `DEFINE_int32(name, default_value, description)`. After `gflags::ParseCommandLineFlags` is called, the argument of the flag is represented by `FLAG_name`.

K Server
--------
The initial configuration is created in the `void runKServer()` function. It will launch the server in a different process. This server will listen for connections and messages on port `K_PORT` (`9191` by default).

The function `void openSocket()` will open a socket on port `K_PORT` and store the `socket descriptor` in the `K_SOCKET` variable.

Http Server
-----------
The `HTTP` server is generated using the `proxygen` lib. By default, it will listen on port `8545`.

The address and ports on which the `HTTP` Server will listen are defined in
```cpp
  std::vector<HTTPServer::IPConfig> IPs = {
    {SocketAddress(FLAGS_ip, FLAGS_port, true), Protocol::HTTP}
  };
```

Other server options like `idleTimeout`, `shutdownOn`, `threads` are defined using an `HTTPServerOptions` object which is bound to the server before start. This object also contains a list of all the handler factories used by the server.

HttpHandlerFactory
------------------
A class derived from `RequestHandlerFactory`, is a factory for `RequestHandlers`.

Methods:
 - `void onServerStart(folly::EventBase* evb)`:
    - is invoked in each thread in which the server will handle requests. Can be used to setup thread-local setup like stats.
    - in this handler, the private variable `int k_socket_` will keep the `socket descriptor` from `K_SOCKET`.
 - `void RequestHandler* onRequest(RequestHandler*, HTTPMessage*)`:
    - Invoked for each new request that the server handles. Will return a new `HttpHandler` object, passing with it the `socket descriptor` to K Server.


HTTPHandler
-----------

[TODO] websockets
-----------------