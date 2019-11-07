/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HttpHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "HttpStats.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include "runtime/alloc.h"
#include "version.h"
#include "init.h"
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

using namespace proxygen;

namespace HttpService {

HttpHandler::HttpHandler(HttpStats* stats, int k_socket): stats_(stats),
                                                          k_socket_(k_socket),
                                                          bracket_counter_(0),
                                                          request_method_(folly::none) {
}

void HttpHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  stats_->recordRequest();
  if ( headers -> isIngressWebsocketUpgrade()) {
    // Web Socket Case
    std :: string wsKey = (headers->getHeaders()).getSingleOrEmpty("Sec-WebSocket-Key");
    makeAcceptValue(wsKey);
    std :: cout << "Websocket Upgrade detected" << std :: endl;
    std :: cout << "Sec-Websocket-Version: " << (headers->getHeaders()).getSingleOrEmpty("Sec-WebSocket-Version") << std :: endl;
    std :: cout << "Sec-WebSocket-Key: " << wsKey << std :: endl;
    std :: cout << "Sec-WebSocket-Accept: " << wsAccept_ << std :: endl;

    ResponseBuilder(downstream_)
      .status(101, "Switching Protocols")
      .header("Upgrade", "websocket")
      .header("Connection", "Upgrade")
      .header("Sec-WebSocket-Accept", wsAccept_)
      .send();
  }
  else {
    // OWISE case
    std :: cout << "Header detected" << std :: endl;
  }
  request_method_ = headers -> getMethod();
}

void HttpHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (HTTPMethod::POST == request_method_) {
    std::string bodyStr = body->moveToFbString().toStdString();
    std::string message;
    std ::cout << "Data Received: " << bodyStr << std :: endl;
    char buffer[2] = {0};
    int ret;

    send(k_socket_ , bodyStr.c_str() , bodyStr.length() , 0 ); 
    std ::cout << "Message sent to K Server \n";

    do {
      ret = recv(k_socket_, buffer, 1, 0);
      buffer[ret] = 0x00;
      message += buffer;
    } while (ret > 0 && false == doneReading(buffer[0]));
    std :: cout << "Message received from K Server: " << message << std::endl;

    ResponseBuilder(downstream_)
      .status(200, "OK")
      .header(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded")
      .header(HTTP_HEADER_CONNECTION, "keep-alive")
      .body(folly::IOBuf::copyBuffer(message))
      .sendWithEOM();
  }
  else{
      std ::cout << "Data Received: " << body->moveToFbString().toStdString() << std :: endl;
  }
}

void HttpHandler::onEOM() noexcept {
  std::cout<< "EOM encountered \n";
}

void HttpHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
  std::cout << "UPGRADE EVENT!" << std::endl;
}

void HttpHandler::requestComplete() noexcept {
  delete this;
}

void HttpHandler::onError(ProxygenError /*err*/) noexcept { delete this; }

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string HttpHandler::makeAcceptValue (std::string wsKey) {
  std::string magicValue = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11",
              source = wsKey + magicValue, hash = "", encoded = "";
  CryptoPP::SHA1 sha1;
  CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1,new CryptoPP::StringSink(hash)));
  wsAccept_ = base64_encode(hash.c_str(),hash.length());

  return hash;
}

bool HttpHandler::doneReading (char c){
  if('{' == c){
    bracket_counter_++;
  } else if ('}' == c) {
    bracket_counter_--;
  }
  return bracket_counter_ == 0;
}
}
