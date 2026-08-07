#pragma once
#include "Common.hpp"
#include "Book.hpp"
#include "Portfolio.hpp"

class Strategy {
public:
    virtual ~Strategy() = default;

    virtual optional<vector<OrderCommand>> onTimeMove(const Time& now, const Book& book, const Portfolio& portfolio, const vector<OrderEvent>& recent_events);
};