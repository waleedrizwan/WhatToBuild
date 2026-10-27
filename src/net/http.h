#pragma once

#include <string>

struct HttpOptions {
  long timeoutMs = 15000;
  /** Skip TLS verification (League's Live Client API uses a self-signed certificate). */
  bool insecure = false;
};

struct HttpResponse {
  long status = 0;
  std::string body;
  /** Set when no response arrived at all (connection refused, timeout, DNS…). */
  std::string error;

  bool ok() const { return error.empty() && status >= 200 && status < 300; }
};

/** Blocking GET. Safe to call from any thread. */
HttpResponse httpGet(const std::string& url, const HttpOptions& options = {});
