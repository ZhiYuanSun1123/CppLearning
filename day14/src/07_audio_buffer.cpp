#include "audio_buffer.hpp"

#include <iostream>
#include <utility>

int main() {
    AudioBuffer original(3);
    original.set(0, 0.1);
    original.set(1, 0.2);
    original.set(2, 0.3);

    AudioBuffer copied(original);
    copied.set(0, 9.9);

    std::cout
        << "original[0] = "
        << original.get(0)
        << '\n';

    std::cout
        << "copied[0] = "
        << copied.get(0)
        << '\n';

    std::cout
        << "different storage = "
        << (original.data() != copied.data())
        << '\n';

    AudioBuffer assigned(1);
    assigned = original;

    std::cout
        << "assigned size = "
        << assigned.size()
        << '\n';

    AudioBuffer moved(
        std::move(original)
    );

    std::cout
        << "moved size = "
        << moved.size()
        << '\n';

    std::cout
        << "original size after move = "
        << original.size()
        << '\n';

    return 0;
}