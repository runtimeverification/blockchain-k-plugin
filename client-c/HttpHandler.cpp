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
  request_method_ = headers -> getMethod();
}

void HttpHandler::onBody(std::unique_ptr<folly::IOBuf> r_body) noexcept {
  if (HTTPMethod::POST == request_method_) {
    std::string body = r_body -> moveToFbString().toStdString();
    std::string message;
    char buffer[2] = {0};
    int ret;

    send(k_socket_, body.c_str(), body.length(), 0);

    do {
      ret = recv(k_socket_, buffer, 1, 0);
      buffer[ret] = 0x00;
      message += buffer;
    } while (ret > 0 && false == doneReading(buffer[0]));

    ResponseBuilder(downstream_)
      .status(200, "OK")
      .header(HTTP_HEADER_CONTENT_TYPE, "application/json")
      .header(HTTP_HEADER_CONNECTION, "keep-alive")
      .body(folly::IOBuf::copyBuffer(message))
      .sendWithEOM();
  }
}

void HttpHandler::onEOM() noexcept {}

void HttpHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {}

void HttpHandler::requestComplete() noexcept {
  delete this;
}

void HttpHandler::onError(ProxygenError /*err*/) noexcept { delete this; }

bool HttpHandler::doneReading (char c) {
  if('{' == c){
    bracket_counter_++;
  } else if ('}' == c) {
    bracket_counter_--;
  }
  return bracket_counter_ == 0;
}
}
