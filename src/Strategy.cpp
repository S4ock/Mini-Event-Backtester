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
    const Price runningFair = midMean_.size() >= 3 ? midMean_.mean() : mid;
    const Price fairValue = runningFair > 0.0 ? (0.65 * mid + 0.35 * runningFair) : mid;

    if (fairValue <= 0.0) {
        return nullopt;
    }

    const Price tick = max<Price>(0.01, fairValue * 0.0001);
    const Price spread = (bestBid > 0.0 && bestAsk > 0.0) ? max<Price>(0.0, bestAsk - bestBid) : max<Price>(0.0, fairValue * 0.0015);
    const Price baseEdge = max<Price>(tick * 2.0, max<Price>(fairValue * 0.0004, spread * 0.25));
    const Price inventorySkew = static_cast<Price>(position) * max<Price>(tick, fairValue * 0.0003);
    const Price reservation = clampPrice(fairValue - inventorySkew * 0.25);

    int bidQty = 5;
    int askQty = 5;
    if (position > 0) {
        bidQty = clampQuantity(5 - position / 4);
        askQty = clampQuantity(5 + position / 4);
    } else if (position < 0) {
        bidQty = clampQuantity(5 + (-position) / 4);
        askQty = clampQuantity(5 - (-position) / 4);
    }

    Price targetBid = clampPrice(reservation - baseEdge);
    Price targetAsk = clampPrice(reservation + baseEdge);

    if (bestBid > 0.0) {
        targetBid = min(targetBid, max<Price>(0.0, bestBid + tick));
    }
    if (bestAsk > 0.0) {
        targetAsk = max(targetAsk, bestAsk - tick);
    }

    if (targetBid >= targetAsk) {
        const Price center = clampPrice(reservation);
        targetBid = clampPrice(center - tick);
        targetAsk = clampPrice(center + tick);
    }

    vector<OrderCommand> commands;

    auto currentBid = bestOpenOrder(openOrders, Side::Buy);
    auto currentAsk = bestOpenOrder(openOrders, Side::Sell);

    constexpr Time staleHorizon = 8;
    auto needsRefresh = [now](const optional<Order>& order, Price desiredPrice, int desiredQty) {
        if (!order.has_value()) {
            return true;
        }

        const bool stale = now - order->ts >= staleHorizon;
        const bool priceMismatch = fabs(order->limit_price - desiredPrice) > max<Price>(desiredPrice * 0.0002, 0.0001);
        const bool quantityMismatch = order->quantity != desiredQty;
        return stale || priceMismatch || quantityMismatch;
    };

    const bool keepBid = currentBid.has_value() && !needsRefresh(currentBid, targetBid, bidQty);
    const bool keepAsk = currentAsk.has_value() && !needsRefresh(currentAsk, targetAsk, askQty);

    if (currentBid.has_value() && !keepBid) {
        commands.push_back(makeCancelOrder(currentBid->id));
    }

    if (currentAsk.has_value() && !keepAsk) {
        commands.push_back(makeCancelOrder(currentAsk->id));
    }

    for (const auto& order : openOrders) {
        if (order.side == Side::Buy) {
            if (!keepBid || !currentBid.has_value() || order.id != currentBid->id) {
                commands.push_back(makeCancelOrder(order.id));
            }
        } else if (!keepAsk || !currentAsk.has_value() || order.id != currentAsk->id) {
            commands.push_back(makeCancelOrder(order.id));
        }
    }

    if (!keepBid) {
        commands.push_back(makeAddOrder(Side::Buy, bidQty, targetBid));
    }

    if (!keepAsk) {
        commands.push_back(makeAddOrder(Side::Sell, askQty, targetAsk));
    }

    if (commands.empty()) {
        return nullopt;
    }

    return commands;
}