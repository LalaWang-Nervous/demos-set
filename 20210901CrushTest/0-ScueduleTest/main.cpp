#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <iomanip>
#include <random>
#include "WeightedHashMap.h"
using namespace  std;

std::string config1 = "A:25,B:25,C:25,D:25";

std::string config2 = "A:25,B:25,C:20,D:30";

std::string config3 = "A:33,B:33,C:34";

std::string config4 = "A:20,B:20,C:20,D:20,E:20";

std::string random_string()
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);
    return str.substr(0, 32);    // assumes 32 < number of characters in str
}

vector<string> random_generate_stream_name(int stream_num) {
    vector<string> ret(stream_num, "");
    for(int i=0;i<stream_num;i++){
        ret[i]= random_string();
    }
    return ret;
}

unordered_map<string, set<string>> old_distribution(const vector<string>& streams,
                                                    const string& config) {
    unordered_map<string, set<string>> ret;
    for(int i=0;i<streams.size();i++){
        string choice;
        srs_choose_from_weighted_strings(config, streams[i], choice);
        ret[choice].insert(streams[i]);
    }
    return ret;
}

unordered_map<string, set<string>> new_distribution(const vector<string>& streams,
                                                    const string& config) {
    unordered_map<string, set<string>> ret;
    for(int i=0;i<streams.size();i++){
        string choice;
        srs_choose_from_weighted_strings_new(config, streams[i], choice);
        ret[choice].insert(streams[i]);
    }
    return ret;
}

unordered_map<string, set<string>> new_distribution_ch(const vector<string>& streams,
                                                    const string& config) {
    unordered_map<string, set<string>> ret;
    for(int i=0;i<streams.size();i++){
        string choice;
        srs_choose_from_weighted_strings_new_ch(config, streams[i], choice);
        ret[choice].insert(streams[i]);
    }
    return ret;
}

struct Counter{
    struct value_type { template<typename T> value_type(const T&) { } };
    void push_back(const value_type&) { ++count; }
    size_t count = 0;
};

template<typename T1, typename T2>
size_t intersection_size(const T1& s1, const T2& s2)
{
    Counter c;
    set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(c));
    return c.count;
}


void print_vector(const vector<int>& matrix){
    for(auto it=matrix.begin();it!=matrix.end();it++){
        cout << setiosflags(ios::left) << setw(5) << *it << ' ';
    }
    cout << endl;
}

int matrix(const unordered_map<string, set<string>>& disA,
                           const unordered_map<string, set<string>>& disB) {
    int node_numA = disA.size();
    int node_numB = disB.size();
    vector<int> unit = vector<int>(node_numB, 0);
    vector<vector<int>> ret = vector<vector<int>>(node_numA, unit);
    
    vector<string> keysA;
    vector<string> keysB;
    for(auto it=disA.begin();it!=disA.end();it++){
        keysA.push_back(it->first);
    }
    sort(keysA.begin(),keysA.end());
    for(auto it=disB.begin();it!=disB.end();it++){
        keysB.push_back(it->first);
    }
    sort(keysB.begin(),keysB.end());

    int width = 7;
    cout << "Before Change:" << endl;
    cout << setiosflags(ios::left) << setw(width) << " ";
    for(int i=0;i<node_numA;i++){
        cout << setiosflags(ios::left) << setw(width) << keysA[i];
    }
    cout << endl;
    cout << setiosflags(ios::left) << setw(width) << " ";
    for(int i=0;i<node_numA;i++){
        cout << setiosflags(ios::left) << setw(width) << disA.find(keysA[i])->second.size();
    }
    cout << endl;

    cout << "After Change:" << endl;
    cout << setiosflags(ios::left) << setw(width) << " ";
    for(int j=0;j<node_numB;j++){
        cout << setiosflags(ios::left) << setw(width) << keysB[j];
    }
    cout << endl;
    cout << setiosflags(ios::left) << setw(width) << " ";
    for(int j=0;j<node_numB;j++){
        cout << setiosflags(ios::left) << setw(width) << disB.find(keysB[j])->second.size();
    }

    cout << endl;
    cout << "Transfer Matrix:" << endl;
    int total = 0;
    int diag = 0;
    for(int i=0;i<node_numA;i++){
        set<string> setA = disA.find(keysA[i])->second;
        for(int j=0;j<node_numB;j++){
            set<string> setB = disB.find(keysB[j])->second;
            ret[i][j] = intersection_size(setA,setB);
            total += ret[i][j];
            if(keysA[i]==keysB[j])diag += ret[i][j];
        }
    }

    cout << setiosflags(ios::left) << setw(width) << " ";
    for(int j=0;j<node_numB;j++){
        cout << setiosflags(ios::left) << setw(width) << keysB[j];
    }
    cout << endl;
    for(int i=0;i<node_numA;i++){
        cout << setiosflags(ios::left) << setw(width) << keysA[i];
        for(int j=0;j<node_numB;j++){
            cout << setiosflags(ios::left) << setw(width) << ret[i][j];
        }
        cout<<endl;
    }
    return total - diag;
}


//for (auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
//    string domianKey = it->key;
//    int domainWeight = it->weight;
//    for(int i=0;i<domainWeight*1000;i++){
//        int virtualNodeHash = fnv1a(it->key + "_" + to_string(i)) % 100000;
//        record.push_back(pair<uint64_t,string>(virtualNodeHash, domianKey));
//    }
//}
//sort(record.begin(), record.end(), compare_virtual_node);


int main() {
    int streamNum = 10000;
    vector<string> streams = random_generate_stream_name(streamNum);
    cout << "old:" << endl;
    unordered_map<string, set<string>> oldDis = old_distribution(streams, config1);
    unordered_map<string, set<string>> newDis = old_distribution(streams, config2);
    int changeNum = matrix(oldDis,newDis);
    cout << "change:" << changeNum <<endl;
    cout << "new:" << endl;
    oldDis = new_distribution(streams, config1);
    newDis = new_distribution(streams, config2);
    changeNum = matrix(oldDis,newDis);
    cout << "change:"<< changeNum <<endl;
    cout << "newch:" << endl;
    oldDis = new_distribution_ch(streams, config1);
    newDis = new_distribution_ch(streams, config2);
    changeNum = matrix(oldDis,newDis);
    cout << "change:"<< changeNum <<endl;
    return 0;
//    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
//    std::random_device rd;
//    std::mt19937 generator(rd());
//    for(int i=0;i<10;i++){
//        std::shuffle(str.begin(), str.end(), generator);
//        cout << str.substr(0, 32) << endl;
//    }
        // assumes 32 < number of characters in str
}



// std::string config_generator() {
//     srand((unsigned)time(NULL));
//     int up = 10;
//     int low = 2;
//     int node_num = (rand() % (up - low + 1)) + low; // 父层回源节点随机取值[2,10]

//     // 随机生成 node_num 个父层节点域名
//     std::vector<std::string> keys(node_num, "");
//     std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
//     std::random_device rd;
//     std::mt19937 generator(rd());
//     for(int i=0; i < node_num; i++) {
//         std::shuffle(str.begin(), str.end(), generator);
//         keys[i] = str.substr(0, 16);
//     }

//     // 随机生成 (node_num - 1) 个总和不超过100的随机正整数


//     // 生成配置字符串
// }


//int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
//    int64_t b = -1, j = 0;
//    while (j < num_buckets) {
//        b = j;
//        key = key * 2862933555777941757ULL + 1;
//        j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
//    }
//    return b;
//}