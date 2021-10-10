//
// Created by wangchenfei01 on 2021/8/21.
//

#ifndef CRUSH_WEIGHTEDHASHMAP_H
#define CRUSH_WEIGHTEDHASHMAP_H

#endif //CRUSH_WEIGHTEDHASHMAP_H
#include <string>
#include <map>
#include <vector>
typedef struct {
    std::string key;
    int weight;
} WeightedString;

bool cmp_weighted_keys(WeightedString i, WeightedString j);
bool srs_parse_weighted_strings(std::string body, std::vector<WeightedString>& wss);
bool srs_choose_from_weighted_strings(const std::string &config,
                                      const std::string &key, std::string &choice);
bool srs_choose_from_weighted_strings_new(const std::string &config,
                                          const std::string &key, std::string &choice);
bool srs_choose_from_weighted_strings_new_ch(const std::string &config,
                                             const std::string &key, std::string &choice);

class WeightedHashMap {
private:
    std::vector<WeightedString> _weighted_keys;
    std::vector<std::pair<uint64_t,std::string>> record;

public:
    WeightedHashMap(const std::vector<WeightedString>& weighted_keys);
    void get_series(const std::string& key, std::vector<std::string>& series);
    void get_series_new(const std::string& key, std::vector<std::string>& series);
    void get_series_new_ch(const std::string& key, std::vector<std::string>& series);
    const std::vector<WeightedString> get_sorted_keys();
    bool get_first_choice(const std::string& key, std::string& choice);
    bool get_first_choice_new(const std::string& key, std::string& choice);
    bool get_first_choice_new_ch(const std::string& key, std::string& choice);
    bool bucket_straw_choice(const std::string& streamName, std::string& choice);
};
