#pragma once
#include <vector>
#include <optional>
#include "Common.hpp"
#include "Book.hpp"
#include "Portfolio.hpp"
using namespace std;

class RunningMean {
private:
    long double sum = 0;
    int count = 0;

public:
    void add(Price x) {
        sum += x;
        count++;
    }

    double mean() const {
        if (count == 0)
            return 0.0;
        return static_cast<double>(sum) / count;
    }

    int size() const {
        return count;
    }
};

class Strategy {
    public:
        optional<vector<OrderCommand>> onTimeMove(const Time& now, const Book& book, const Portfolio& portfolio, const vector<OrderEvent>& recent_events);
    private:
        RunningMean midMean_;
};