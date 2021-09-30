#include "metadata.h"
#include "stringutil.h"

std::pair<std::uint64_t, std::uint64_t> tiny::Metadata::getPosition(tiny::WalkableStream<std::uint32_t> &s) const {
    std::uint64_t line = 1;
    std::uint64_t col = 1;

    while (s.getIndex() < start && s) {
        if (s.get() == '\n') {
            line++;
            col = 1;

            continue;
        }

        col++;
    }

    return {line, col};
}

std::pair<std::string, std::int32_t>
tiny::Metadata::getContext(tiny::WalkableStream<std::uint32_t> &s, std::int32_t range) const {
    unsigned long prevState = s.getIndex(); // Save the index to restore it latter

    // Start from the character that generated the error
    s.seek(start);

    auto strStream = tiny::UnicodeParser::toString(s.getVector());

    std::int32_t errPos = 0; // This will hold the margin between the start of the string and the error char
    // Backup until a newline is found or the range is exceeded
    for (; errPos < range / 2; errPos++) {
        if (s.peek() == '\n') {
            s.skip(); // Step over the newline
            break;
        }

        s.backup();
        if (s.getIndex() == 0) {
            errPos += 2;
            break;
        }
    }

    // Now do a forwards pass and save the found chars
    tiny::UnicodeCodepoints context;
    for (int x = 0; x < range && s; x++) {
        auto got = s.get();
        if (got == '\n') {
            break;
        }

        context.push_back(got);
    }

    // Restore the stream's index
    s.seek(prevState);


    auto ctxStr = tiny::UnicodeParser::toString(context);
    int prevLength = std::int32_t(ctxStr.length());

    tiny::ltrim(ctxStr);

    // Account for the removal of whitespaces, otherwise the position won't match up.
    errPos -= prevLength - std::int32_t(ctxStr.length());

    return {ctxStr, errPos};
}

std::int32_t tiny::Metadata::getLength() const {
    if (end == 0) {
        return 1;
    }

    return std::int32_t(end - start);
}
