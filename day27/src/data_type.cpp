#include"data_type.hpp"
[[nodiscard]] std::string_view to_string(
    DataType type
) noexcept {
    switch(type){
        case DataType::boolean :
            return "Bool";
        case DataType::float16 :
            return "Float16";
        case DataType::float32 :
            return "Float32";
        case DataType::int8 :
            return "Int8";
        case DataType::int32 :
            return "Int32";
    }
    return "unknown";
}
[[nodiscard]] std::optional<std::size_t> bytes_per_element(
    DataType type
) noexcept {
       switch(type){
        case DataType::boolean :
            return 1;
        case DataType::float16 :
            return 2;
        case DataType::float32 :
            return 4;
        case DataType::int8 :
            return 1;
        case DataType::int32 :
            return 4;
    }
}