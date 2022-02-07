#include "errors.h"

#include <algorithm>

void tiny::ErrorBuilder::log() const {
    auto [line, col] = exception.meta.getPosition(stream);
    auto [context, pos] = exception.meta.getContext(stream);

    tiny::error("In file \"" + exception.meta.filename + "\" in line " + std::to_string(line) +
            ", column " + std::to_string(col) + ": ");

    tiny::error("\t" + context);
    tiny::error("\t" + std::string((std::max)(pos - 2, 0), ' ') + std::string(3, '^'));
}
