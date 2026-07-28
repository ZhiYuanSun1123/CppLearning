#include<iostream>
class Logger {
public:
    void log(
        const std::string& message
    ) const {
        std::cout << message << '\n';
    }
};
class AudioModel{
public:
    void load() {
        logger.log("Load audio model");
    }
private:
    Logger logger;
};
int main(){
    AudioModel aml;
    aml.load();
}