#include "errors.h"

#include <algorithm>

void tiny::ErrorBuilder::log(tiny::Logger &l) const {
    auto pos = exception.meta.getPosition(stream);
    auto context = exception.meta.getContext(stream);

    l.error("In file \"" + exception.meta.filename + "\" in line " + std::to_string(pos.first) +
            ", column " + std::to_string(pos.second) + ": ");

    l.error("\t" + context.first);
    l.error("\t" + std::string((std::max)(context.second - 2, 0), ' ') + std::string(3, '^'));
}
