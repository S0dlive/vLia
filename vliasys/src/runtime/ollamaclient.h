#ifndef VLIASYS_OLLAMACLIENT_H
#define VLIASYS_OLLAMACLIENT_H

#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

class ollamaClient {
public:
    ollamaClient(std::string endpoint, std::string model)
        : m_endpoint(std::move(endpoint)), m_model(std::move(model)) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~ollamaClient() {
        curl_global_cleanup();
    }

    json chat(const json& messages, const json& toolsSchema) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL instance");
        }

        std::string url = m_endpoint + "/api/chat";
        
        json body = {
            {"model", m_model},
            {"messages", messages},
            {"stream", false}
        };

        if (!toolsSchema.empty()) {
            body["tools"] = toolsSchema;
        }

        std::string payload = body.dump();
        std::string responseString;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.size());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); 

        CURLcode res = curl_easy_perform(curl);
        
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::string errStr = curl_easy_strerror(res);
            spdlog::error("CURL request failed: " + errStr);
            throw std::runtime_error("CURL error: " + errStr);
        }

        if (httpCode != 200) {
            spdlog::error("Ollama HTTP " +  std::to_string(httpCode) + " - Response: " + responseString);
            throw std::runtime_error("Ollama HTTP Error " + std::to_string(httpCode));
        }

        return json::parse(responseString);
    }

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        auto* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    std::string m_endpoint;
    std::string m_model;
};

#endif // VLIASYS_OLLAMACLIENT_H