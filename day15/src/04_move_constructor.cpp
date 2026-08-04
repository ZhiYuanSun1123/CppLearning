#include "trace_buffer.hpp"

#include <iostream>
#include <utility>

int main() {
    TraceBuffer::reset_counts();

    TraceBuffer source(4);
    source.set(0, 1.5);

    const double* old_address =
        source.data();

    TraceBuffer copied(source);

    TraceBuffer moved(
        std::move(source)
    );

    std::cout
        << "copy constructor count: "
        << TraceBuffer::copy_constructor_count()
        << '\n';

    std::cout
        << "move constructor count: "
        << TraceBuffer::move_constructor_count()
        << '\n';

    std::cout
        << "copy uses different storage: "
        << (copied.data() != old_address)
        << '\n';

    std::cout
        << "move keeps old storage address: "
        << (moved.data() == old_address)
        << '\n';

    std::cout
        << "source is empty: "
        << (source.data() == nullptr &&
            source.size() == 0)
        << '\n';

    return 0;
}