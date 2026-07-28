#include<iostream>
class Model{
    public:
        Model():model_name_("None"){}
    private:
        std::string model_name_;
};
class AudioModel:Model{
    public:
        AudioModel(){
        }
};
int main(){
    AudioModel md;
    return 0;
}