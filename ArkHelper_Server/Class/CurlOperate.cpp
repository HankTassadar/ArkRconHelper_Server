#include "CurlOperate.h"

std::string CurlOperate::get(const std::string& url, const bool httpsUse)
{
    std::stringstream strStream;
    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strStream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlOperate::recive_data_fun);
        if (httpsUse) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        }
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return std::string(strStream.str());
}

std::string CurlOperate::post(const std::vector<std::pair<std::string, std::string>>& param, const std::string& url, const bool httpsUse)
{
    if (param.size() == 0)return"";
    std::stringstream strStream;
    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strStream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlOperate::recive_data_fun);

        if (httpsUse) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        }

        curl_easy_setopt(curl, CURLOPT_POST, 1);
        std::string postdata;
        for (auto& i : param) {
            postdata += i.first + "=" + i.second + "&";
        }
        postdata.pop_back();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postdata.size());

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return std::string(strStream.str());
}

