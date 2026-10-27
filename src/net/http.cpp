#include "net/http.h"

#include <curl/curl.h>

#include <mutex>

namespace {

size_t appendBody(char* data, size_t size, size_t count, void* out) {
  static_cast<std::string*>(out)->append(data, size * count);
  return size * count;
}

void initCurlOnce() {
  static std::once_flag once;
  std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

}  // namespace

HttpResponse httpGet(const std::string& url, const HttpOptions& options) {
  initCurlOnce();
  HttpResponse res;
  CURL* curl = curl_easy_init();
  if (!curl) {
    res.error = "curl_easy_init failed";
    return res;
  }
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, options.timeoutMs);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendBody);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
  if (options.insecure) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  }
  CURLcode rc = curl_easy_perform(curl);
  if (rc != CURLE_OK) res.error = curl_easy_strerror(rc);
  else curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
  curl_easy_cleanup(curl);
  return res;
}
