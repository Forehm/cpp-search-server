#include "document.h"

using namespace std;


bool operator == (const Document& lhs, const Document& rhs) {
    return (lhs.id == rhs.id &&
        lhs.rating == rhs.rating &&
        abs(lhs.relevance - rhs.relevance) < 1e-6);
}

bool operator != (const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

bool operator < (const Document& lhs, const Document& rhs) {

    if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
        return lhs.rating < rhs.rating;
    }
    return lhs.relevance < rhs.relevance;
}

ostream& operator << (ostream& os, const Document& rhs) {
    os << "{ "s
        << "document_id = "s << rhs.id << ", "s
        << "relevance = "s << rhs.relevance << ", "s
        << "rating = "s << rhs.rating
        << " }"s;
    return os;
}