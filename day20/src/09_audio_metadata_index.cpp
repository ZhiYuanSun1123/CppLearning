#include<iostream>
#include<unordered_map>
#include<set>
struct AudioMetadata{
    std::string path;
    int sample_rate;
    double duration_seconds;
};
void insert(
    std::unordered_map<std::string,AudioMetadata>& metadatas,
    AudioMetadata am,
    std::unordered_map<std::string,std::string>& paths,
    std::size_t i
){
    if(paths.find(am.path)==paths.end()){
        metadatas.insert({
            std::to_string(i),am
        });
        paths.insert({am.path,std::to_string(i)});
    } else
        throw std::invalid_argument("路径重复");
}
 std::unordered_map<std::string,AudioMetadata>::iterator find(
    std::string path,
    std::unordered_map<std::string,AudioMetadata>& metadatas,
    std::unordered_map<std::string,std::string>& paths
){
    if(paths.find(path)!=paths.end())
        return metadatas.find(paths.find(path)->second);
    else
        return metadatas.end();
}
int main(){
    std::unordered_map<std::string,AudioMetadata> metadatas;
    std::unordered_map<std::string,std::string> path;
    for(
        int i = 0;
        i < 3;
        i ++
    ) {
        AudioMetadata am{"path"+std::to_string(i),160000,10};
        if(path.find(am.path)==path.end()){
            metadatas.insert({
                std::to_string(i),am
            });
            path.insert({am.path,std::to_string(i)});
        }
    }
}