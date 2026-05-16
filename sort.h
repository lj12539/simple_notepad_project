#ifndef SORT_H
#define SORT_H

#include <iterator>
#include <utility>

namespace my {

template <typename Iterator, typename Comparator>
void sort(Iterator begin,
    Iterator end,
    Comparator comp)
{
    for (Iterator i = begin; i != end; ++i) {

        Iterator best = i;

        for (Iterator j = std::next(i); j != end; ++j) {

            if (comp(*j, *best)) {
                best = j;
            }
        }

        if (best != i) {
            std::swap(*i, *best);
        }
    }
}

template <typename Iterator>
void sort(Iterator begin,
    Iterator end)
{
    sort(begin,
        end,
        [](const auto& a, const auto& b) {
            return a < b;
        });
}

}

#endif // SORT_H