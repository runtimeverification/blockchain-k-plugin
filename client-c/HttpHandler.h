/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {
class ResponseHandler;
}

namespace HttpService {

class HttpStats;

class HttpHandler : public proxygen::RequestHandler {
 public:
  explicit HttpHandler(HttpStats* stat, int k_socket);

  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
      noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> r_body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

  bool doneReading(char *buffer);

 private:
  HttpStats* const stats_{nullptr};
  int k_socket_;
  int bracket_counter_;
  int brace_counter_;
  folly::Optional<proxygen::HTTPMethod> request_method_;
};

}
