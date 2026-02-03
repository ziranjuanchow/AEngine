#pragma once

#include <tl/expected.hpp>

namespace AEngine {

    // Polyfill for C++23 std::expected (used for error handling without exceptions)
    template <typename T, typename E>
    using expected = tl::expected<T, E>;
    
    template <typename E>
    using unexpected = tl::unexpected<E>;

}
