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
The private variable `int k_socket_` keeps the connection to the `K Server`.

Methods:
 - `void onServerStart(folly::EventBase* evb)`:
    - is invoked in each thread in which the server will handle requests. Can be used to setup thread-local setup like stats.
    - in this handler, the private variable `int k_socket_` will keep the `socket descriptor` from `K_SOCKET`.
 - `void RequestHandler* onRequest(RequestHandler*, HTTPMessage*)`:
    - Invoked for each new request that the server handles. Will return a new `HttpHandler` object, passing with it the `socket descriptor` to K Server.


HTTPHandler
-----------
A class which implements the `RequestHandler` interface. In addition to the interface, the `HTTPHandler` class has three more private variables:
 - `int k_socket_`
 - `int bracket_counter_`
 - `folly::Optional<proxygen::HTTPMethod> request_method_`

Methods:
 - `void onRequest(std::unique_ptr<HTTPMessage> headers)`:
    - Method which is invoked when we have successfully fetched headers from a client. This is the first callback invoked by the handler.
    - gets the type of the http request and stores it `request_method_`
 - `void onBody(std::unique_ptr<folly::IOBuf> r_body)`:
    - Invoked when we get part of body for the request.
    - If the `request_method_` is `HTTPMethod::POST`, we send the received jsonrpc to K Server using `k_socket_`.
    - After the jsonrpc message is processed, the result is received, a response is built and is sent back to the client.

[TODO] websockets
-----------------
 In order to implement communication via websockets, we should:
 1. Check in `HttpHandler::onRequest` that the request is an upgrade request. This can be verified if `headers -> isIngressWebsocketUpgrade()` returns `true`.
 2. Determine the `Sec-WebSocket-Accept` value required for the handshake response.
 Steps to calculate the `Sec-WebSocket-Accept` value:
  - get `Sec-WebSocket-Key` string value from the headers
  - append the magic value `258EAFA5-E914-47DA-95CA-C5AB0DC85B11` at the end of the `Sec-WebSocket-Key`
  - hash the result with `sha1`
  - encode the hash in `base_64`
3. Build a response for the client, with the status `101 Switching Protocols` which contains the `Sec-Websocket-Accept` value. 
Note: The response has be sent without `EOM`.
4. The `onUpgrade` event handler should be invoked.
5. Incoming data should be processed in the `onBody()` function, similar to how the POST messages are processed.