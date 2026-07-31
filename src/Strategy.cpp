#include "../include/Strategy.hpp"
#include <algorithm>
#include <cmath>
using namespace std;

Price clampPrice(Price price) {
    return max<Price>(0.0, price);
}

Price safeMid(Price bestBid, Price bestAsk, const RunningMean& midMean) {
    if (bestBid > 0.0 && bestAsk > 0.0) {
        return (bestBid + bestAsk) / 2.0;
    }

    if (midMean.size() > 0) {
        return midMean.mean();
    }

    if (bestBid > 0.0) {
        return bestBid * 1.001;
    }

    if (bestAsk > 0.0) {
        return bestAsk * 0.999;
    }

    return 0.0;
}

optional<Order> bestOpenOrder(const vector<Order>& openOrders, Side side) {
    optional<Order> best;

    for (const auto& order : openOrders) {
        if (order.side != side) {
            continue;
        }

        if (!best.has_value()) {
            best = order;
            continue;
        }

        if (side == Side::Buy) {
            if (order.limit_price > best->limit_price) {
                best = order;
            }
        } else if (order.limit_price < best->limit_price) {
            best = order;
        }
    }

    return best;
}

OrderCommand makeAddOrder(Side side, int quantity, Price limitPrice) {
    OrderCommand command{};
    command.type = OrderCommandType::AddOrder;
    command.owner_id = 1;
    command.side = side;
    command.quantity = quantity;
    command.limit_price = limitPrice;
    return command;
}

OrderCommand makeCancelOrder(OrderId orderId) {
    OrderCommand command{};
    command.type = OrderCommandType::CancelOrder;
    command.order_id = orderId;
    return command;
}

OrderCommand makeModifyOrder(OrderId orderId, int quantity, Price limitPrice) {
    OrderCommand command{};
    command.type = OrderCommandType::ModifyOrder;
    command.order_id = orderId;
    command.new_quantity = quantity;
    command.new_limit_price = limitPrice;
    return command;
}

int clampQuantity(int quantity) {
    return max(1, quantity);
}

optional<vector<OrderCommand>> Strategy::onTimeMove(const Time& now, const Book& book, const Portfolio& portfolio, const vector<OrderEvent>& recent_events) {
    const Price bestBid = book.bestBid();
    const Price bestAsk = book.bestAsk();
    const Price mid = safeMid(bestBid, bestAsk, midMean_);

    if (mid > 0.0) {
        midMean_.add(mid);
    }

    const auto openOrders = portfolio.getOpenOrders();
    const int position = portfolio.getPosition();
    const Price cash = portfolio.getCash();
    const Price runningFair = midMean_.size() >= 3 ? midMean_.mean() : mid;
    const Price fairValue = runningFair > 0.0 ? (0.65 * mid + 0.35 * runningFair) : mid;

    if (fairValue <= 0.0) {
        return nullopt;
    }

    const Price tick = max<Price>(0.01, fairValue * 0.0001);
    const Price spread = (bestBid > 0.0 && bestAsk > 0.0) ? max<Price>(0.0, bestAsk - bestBid) : max<Price>(0.0, fairValue * 0.0015);
    const Price baseEdge = max<Price>(tick * 1.5, max<Price>(fairValue * 0.00025, spread * 0.18));

    const int maxPosition = 10;
    const int targetPosition = position >= 0 ? 4 : -4;
    const int positionDelta = targetPosition - position;

    Price targetBid = clampPrice(fairValue - baseEdge);
    Price targetAsk = clampPrice(fairValue + baseEdge);

    if (bestBid > 0.0) {
        targetBid = max(targetBid, bestBid + tick * 0.5);
    }
    if (bestAsk > 0.0) {
        targetAsk = min(targetAsk, bestAsk - tick * 0.5);
    }

    if (targetBid >= targetAsk) {
        targetBid = clampPrice(fairValue - tick);
        targetAsk = clampPrice(fairValue + tick);
    }

    const Price buyBudget = cash > 0.0 ? cash : 0.0;
    const Price buyCostPerShare = max<Price>(1.0, targetBid * (1.0 + fee_rate));
    const int maxCashBasedQty = buyBudget > 0.0 && buyCostPerShare > 0.0
        ? static_cast<int>(floor(buyBudget / buyCostPerShare))
        : 0;
    const int maxQty = min(maxPosition, max(0, maxCashBasedQty));

    vector<OrderCommand> commands;
    const bool hasBid = any_of(openOrders.begin(), openOrders.end(), [](const Order& order) {
        return order.side == Side::Buy;
    });
    const bool hasAsk = any_of(openOrders.begin(), openOrders.end(), [](const Order& order) {
        return order.side == Side::Sell;
    });

    if (!hasBid && positionDelta > 0) {
        const int qty = min(maxQty, positionDelta);
        if (qty > 0) {
            commands.push_back(makeAddOrder(Side::Buy, qty, targetBid));
        }
    }

    if (!hasAsk && positionDelta < 0) {
        const int qty = min(maxQty, -positionDelta);
        if (qty > 0) {
            commands.push_back(makeAddOrder(Side::Sell, qty, targetAsk));
        }
    }

    if (commands.empty()) {
        return nullopt;
    }

    return commands;
}