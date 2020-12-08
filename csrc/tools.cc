#include "tools.h"
#include <sstream>

// Integer to string

std::string itos (int source) {
        std::ostringstream q;
        q << source;
        return (q.str());
}
