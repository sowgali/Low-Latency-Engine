#include "Exchange/matcher/me_order.hpp"

namespace Exchange {
    auto MEOrder::toString() const -> std::string {
        return "MEOrder{"
            "clientId=" + clientIdToString(clientId) + ", "
            "tickerId=" + tickerIdToString(tickerId) + ", "
            "client_order_id_=" + orderIdToString(client_order_id_) + ", "
            "market_order_id_=" + orderIdToString(market_order_id_) + ", "
            "side=" + sideToString(side) + ", "
            "price=" + priceToString(price) + ", "
            "qty=" + qtyToString(qty) + ", "
            "priority=" + priorityToString(priority) + ", "
            "next_order_=" + (next_order_ ? next_order_->toString() : "null") + ", "
            "prev_order_=" + (prev_order_ ? prev_order_->toString() : "null") + "}";
    }

    auto MEOrderAtPrice::toString() const -> std::string {
        return "MEOrderAtPrice{"
            "price=" + priceToString(price) + ", "
            "side=" + sideToString(side) + ", "
            + "first_me_order_=" + (first_me_order_ ? " -> " + first_me_order_->toString() : " null ") + ", "
            + "next_price_=" + (next_price_ ? " -> " + next_price_->toString() : " null ") + ", "
            + "prev_price_=" + (prev_price_ ? " -> " + prev_price_->toString() : " null ")+ "}";
    }
}