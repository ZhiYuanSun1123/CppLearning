#include"string"
class TestRunner{
public:
    void expect_true(
        bool condition,
        const std::string& test_name
    );
    void expect_equal(
        const std::string& actual,
        const std::string& expected,
        const std::string& test_name
    );
    void expect_equal(
        int actual,
        int expected,
        const std::string& test_name
    );
    int final() const;
private:
    int passed_ = 0;
    int failed_ = 0;
};