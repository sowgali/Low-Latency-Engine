#pragma once

#include <limits>
#include "macros.hpp"

namespace Common {
    constexpr size_t ME_MAX_TICKERS = 8;
    
    constexpr size_t ME_MAX_CLIENT_UPDATES = 256 * 1024;
    constexpr size_t ME_MAX_MARKET_UPDATES = 256 * 1024;

    constexpr size_t ME_MAX_NUM_CLIENTS = 256;
    constexpr size_t ME_MAX_ORDER_IDS = 1024 * 1024;
    constexpr size_t ME_MAX_PRICE_LEVELS = 256;

    typedef uint64_t OrderId;
    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();

    inline auto orderIdToString(OrderId id) -> std::string {
        if(id == OrderId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(id);
    }

    typedef uint32_t TickerId;
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();

    inline auto tickerIdToString(TickerId id) -> std::string {
        if(id == TickerId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(id);
    }

    typedef uint32_t ClientId;
    constexpr auto ClientId_INVALID = std::numeric_limits<ClientId>::max();

    inline auto clientIdToString(ClientId id) -> std::string {
        if(id == ClientId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(id);
    }

    typedef uint64_t Price;
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max();
    
    inline auto priceToString(Price p) -> std::string {
        if(p == Price_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(p);
    }

    typedef uint32_t Qty;
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max();
    
    inline auto qtyToString(Qty q) -> std::string {
        if(q == Qty_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(q);
    }

    typedef uint64_t Priority;
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max();

    inline auto priorityToString(Priority p) -> std::string {
        if(p == Priority_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(p);
    }

    enum class Side : int8_t {
        INVALID=0,
        BUY=1,
        SELL=-1
    };

    inline auto sideToString(Side s) -> std::string {
        switch(s){
            case Side::INVALID:
                return "INVALID";
            case Side::BUY:
                return "BUY";
            case Side::SELL:
                return "SELL";
            default:
                return "UNKNOWN";
        }
    }
}