#include "metadata.h"
#include "stringutil.h"

std::pair<int, int> tiny::Metadata::getPosition(tiny::WalkableStream<uint32_t> &s) const {
    int line = 1;
    int col = 1;

    while (s.getIndex() <= pos && s) {
        if (s.get() == '\n') {
            line++;
            col = 1;

            continue;
        }

        col++;
    }

    return {line, col};
}

std::pair<std::string, int> tiny::Metadata::getContext(tiny::WalkableStream<uint32_t> &s, int range) const {
    unsigned long prevState = s.getIndex(); // Save the index to restore it latter

    // Start from the character that generated the error
    s.seek(pos);

    int errPos = 0; // This will hold the margin between the start of the string and the error char
    // Backup until a newline is found or the range is exceeded
    for (; errPos < range / 2; errPos++) {
        if (s.peek() == '\n') {
            s.skip(); // Step over the newline
            break;
        }

        s.backup();
        if (s.getIndex() <= 0) {
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


    auto ctxStr = tiny::UnicodeParser::ToString(context);
    int prevLength = int(ctxStr.length());

    tiny::util::ltrim(ctxStr);

    // Account for the removal of whitespaces, otherwise the position won't match up.
    errPos -= prevLength - int(ctxStr.length());

    return {ctxStr, errPos};
}
