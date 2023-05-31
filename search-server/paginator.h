#pragma once
#include "document.h"
#include <iostream>


template<typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator begin, Iterator end, const int size);

    Iterator begin();
    Iterator end();
    int size();

private:
    int size_;
    Iterator it_begin_;
    Iterator it_end_;
};


template<typename Iterator>
class Paginator
{
private:
    std::vector<IteratorRange<Iterator>> vector_of_pages;

public:
    Paginator(Iterator it_begin, Iterator it_end, const size_t size_of_the_page);

    auto begin() const;
    auto end() const;
    int Size();
};


template<typename Iterator>
std::ostream& operator << (std::ostream& os, IteratorRange<Iterator> it)
{
    for (const Document& doc : it)
    {
        os << doc;
    }

    return os;
}


template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}



template<typename Iterator>
inline IteratorRange<Iterator>::IteratorRange(Iterator begin, Iterator end, const int size)
{
    it_begin_ = begin;
    it_end_ = end;
    size_ = size;
}

template<typename Iterator>
inline Iterator IteratorRange<Iterator>::begin()
{
    return it_begin_;
}

template<typename Iterator>
inline Iterator IteratorRange<Iterator>::end()
{
    return it_end_;
}

template<typename Iterator>
inline int IteratorRange<Iterator>::size()
{
    return size_;
}

template<typename Iterator>
inline Paginator<Iterator>::Paginator(Iterator it_begin, Iterator it_end, const size_t size_of_the_page)
{
    for (Iterator page_it_begin = it_begin, page_it_end = it_begin; page_it_begin != it_end; page_it_begin = page_it_end)
    {
        page_it_end = next(page_it_begin, std::min(size_of_the_page, static_cast<size_t>(distance(page_it_begin, it_end))));
        IteratorRange<Iterator> it_of_current_page(page_it_begin, page_it_end, size_of_the_page);
        vector_of_pages.emplace_back(it_of_current_page);
    }
}

template<typename Iterator>
inline auto Paginator<Iterator>::begin() const
{
    return vector_of_pages.begin();
}

template<typename Iterator>
inline auto Paginator<Iterator>::end() const
{
    return vector_of_pages.end();
}

template<typename Iterator>
inline int Paginator<Iterator>::Size()
{
    return vector_of_pages.size();
}