#include<iostream>
#include<vector>
#include<algorithm>
struct AudioRecord {
    std::string path;
    double duration_seconds;
    int sample_rate;
};
int main(){
    std::vector<AudioRecord> data;
    data.push_back(AudioRecord{
        "root",10.2,10000
    });
    data.push_back(AudioRecord{
        "",10.2,10000
    });
    data.push_back(AudioRecord{
        "root",-10.2,10000
    });
    data.push_back(AudioRecord{
        "root",10.2,-10000
    });
    auto iterator = std::find_if(
        data.begin(),
        data.end(),
        [](AudioRecord ar){
            return ar.path.length()!=0;
        }
    );
    std::cout << iterator->path << std::endl;
    std::size_t count = std::count_if(
        data.begin(),
        data.end(),
        [](AudioRecord ar){
            return ar.sample_rate == 10000;
        }
    );
    std::cout << count << std::endl;
    std::vector<AudioRecord> data_copy;
    std::copy_if(
        data.begin(),
        data.end(),
        std::back_inserter(data_copy),
        [](AudioRecord ar){
            return (ar.path.length()!=0)&&(ar.duration_seconds>0)&&(ar.sample_rate>0);
        }
    );
    for(auto data:data_copy){
        std::cout << "Path: " << data.path << std::endl;
        std::cout << "Duration: " << data.duration_seconds << std::endl;
        std::cout << "Rate: " << data.sample_rate << std::endl;
    }
}