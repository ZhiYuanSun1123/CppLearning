#include<iostream>
class AudioClip{
    public:
        AudioClip();
        void print() const;
        void set_path(std::string path_);
        std::string get_path(){
            return path_;
        }
    private:
        std::string path_;
};
AudioClip::AudioClip():path_("None"){}
void AudioClip::set_path(std::string path_){
    if(path_.length()<=10){
        this->path_ = path_;
    } else{
        std::cout << "Invalid Input" << std::endl;
    }
}
int main(){
    AudioClip first;
    std::string path_ = "root/audio";
    first.set_path(path_);
    std::cout << first.get_path() << std::endl;
}