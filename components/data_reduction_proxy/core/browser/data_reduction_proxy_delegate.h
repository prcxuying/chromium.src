// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_DELEGATE_H_
#define COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_DELEGATE_H_

#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_auth_request_handler.h"
#include "net/base/proxy_delegate.h"
#include "url/gurl.h"

namespace net {
class HostPortPair;
class HttpRequestHeaders;
class HttpResponseHeaders;
class ProxyInfo;
class ProxyServer;
class ProxyService;
class URLRequest;
}

namespace data_reduction_proxy {

class DataReductionProxyAuthRequestHandler;

class DataReductionProxyDelegate : public net::ProxyDelegate {
 public:
  // ProxyDelegate instance is owned by io_thread. |auth_handler| and |params|
  // outlives this class instance.
  explicit DataReductionProxyDelegate(
      DataReductionProxyAuthRequestHandler* auth_handler,
      DataReductionProxyParams* params);

  ~DataReductionProxyDelegate() override;

  void OnResolveProxy(const GURL& url,
                      int load_flags,
                      const net::ProxyService& proxy_service,
                      net::ProxyInfo* result) override;

  void OnFallback(const net::ProxyServer& bad_proxy, int net_error) override;

  void OnBeforeSendHeaders(net::URLRequest* request,
                           const net::ProxyInfo& proxy_info,
                           net::HttpRequestHeaders* headers) override;

  void OnBeforeTunnelRequest(const net::HostPortPair& proxy_server,
                             net::HttpRequestHeaders* extra_headers) override;

  void OnTunnelConnectCompleted(const net::HostPortPair& endpoint,
                                const net::HostPortPair& proxy_server,
                                int net_error) override;

  void OnTunnelHeadersReceived(
      const net::HostPortPair& origin,
      const net::HostPortPair& proxy_server,
      const net::HttpResponseHeaders& response_headers) override;

 private:
  DataReductionProxyAuthRequestHandler* auth_handler_;
  const DataReductionProxyParams* params_;

  DISALLOW_COPY_AND_ASSIGN(DataReductionProxyDelegate);
};

}  // namespace data_reduction_proxy

#endif  // COMPONENTS_DATA_REDUCTION_PROXY_CORE_BROWSER_DATA_REDUCTION_PROXY_DELEGATE_H_
