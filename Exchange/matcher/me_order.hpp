#pragma once

#include <array>
#include "Common/types.hpp"

using namespace Common;

namespace Exchange {
    struct MEOrder {
        ClientId clientId = ClientId_INVALID;
        TickerId tickerId = TickerId_INVALID;
        OrderId client_order_id_ = OrderId_INVALID;
        OrderId market_order_id_ = OrderId_INVALID;
        Side side = Side::INVALID;
        Price price = Price_INVALID;
        Qty qty = Qty_INVALID;
        Priority priority = Priority_INVALID;

        MEOrder* next_order_ = nullptr;
        MEOrder* prev_order_ = nullptr;

        MEOrder() = default;
        MEOrder(ClientId clientId, TickerId tickerId, OrderId client_order_id_, OrderId market_order_id_, Side side, Price price, Qty qty, Priority priority)
            : clientId(clientId), tickerId(tickerId), client_order_id_(client_order_id_), market_order_id_(market_order_id_), side(side), price(price), qty(qty), priority(priority) {}
        
        auto toString() const -> std::string;
    };

    struct MEOrderAtPrice {
        Price price = Price_INVALID;
        Side side = Side::INVALID;
        MEOrder* first_me_order_ = nullptr;

        MEOrderAtPrice* next_price_ = nullptr;
        MEOrderAtPrice* prev_price_ = nullptr;

        MEOrderAtPrice() = default;
        MEOrderAtPrice(Price price, Side side, MEOrder* first_me_order_, MEOrderAtPrice* next_price_, MEOrderAtPrice* prev_price_)
            : price(price), side(side), first_me_order_(first_me_order_), next_price_(next_price_), prev_price_(prev_price_) {}
        
        auto toString() const -> std::string;
    };

    typedef std::array<MEOrder*, ME_MAX_ORDER_IDS> OrderHashMap;
    typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;
    typedef std::array<MEOrderAtPrice*, ME_MAX_PRICE_LEVELS> OrdersAtPriceHashMap;
}