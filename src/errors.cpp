#include "errors.h"

#include <algorithm>

void tiny::CompilerError::log(tiny::Stream<std::uint32_t> &s) const {
    auto [line, col] = meta.getPosition(s);
    auto [context, pos] = meta.getContext(s);

    tiny::error("In file \"" + meta.filename + "\" in line " + std::to_string(line) +
            ", column " + std::to_string(col) + ": ");

    tiny::error("\t" + context);
    tiny::error("\t" + std::string((std::max)(pos - 2, 0), ' ') + std::string(3, '^'));
}
