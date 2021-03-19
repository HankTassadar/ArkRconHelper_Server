#include "CurlOperate.h"

std::string CurlOperate::get(const std::string& url)
{
    std::stringstream strStream;
    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strStream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlOperate::recive_data_fun);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return std::string(strStream.str());
}
