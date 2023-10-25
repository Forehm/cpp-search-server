#pragma once

#include <iostream>

struct Document 
{
    Document() = default;
    Document(int document_id, double document_relevance, int document_rating) :
        id(document_id), relevance(document_relevance), rating(document_rating) {}

    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    friend bool operator == (const Document& lhs, const Document& rhs);
    friend bool operator != (const Document& lhs, const Document& rhs);
    friend bool operator < (const Document& lhs, const Document& rhs);
    friend std::ostream& operator << (std::ostream& os, const Document& rhs);
};

enum class DocumentStatus 
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};
