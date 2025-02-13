#pragma once

#include "Common/types.hpp"
#include "Common/spsc_lf_queue.hpp"

using namespace Common;

namespace Exchange {
    #pragma pack(push, 1)
    enum class MarketUpdateType : uint8_t {
        INVALID=0,
        ADD=1,
        MODIFY=2,
        CANCEL=3,
        TRADE=4
    };

    inline auto marketUpdateTypeToString(MarketUpdateType t) -> std::string {
        switch(t){
            case MarketUpdateType::INVALID: return "INVALID";
            case MarketUpdateType::ADD: return "ADD";
            case MarketUpdateType::MODIFY: return "MODIFY";
            case MarketUpdateType::CANCEL: return "CANCEL";
            case MarketUpdateType::TRADE: return "TRADE";
            default: return "UNKNOWN";
        }
    }

    struct MEMarketUpdate {
        MarketUpdateType type = MarketUpdateType::INVALID;
        TickerId tickerId = TickerId_INVALID;
        OrderId orderId = OrderId_INVALID;
        Side side = Side::INVALID;
        Price price = Price_INVALID;
        Qty qty = Qty_INVALID;
        Priority priority = Priority_INVALID;

        inline auto toString() const -> std::string {
            return "MEMarketUpdate{"
                "type=" + marketUpdateTypeToString(type) + ", "
                "tickerId=" + tickerIdToString(tickerId) + ", "
                "orderId=" + orderIdToString(orderId) + ", "
                "side=" + sideToString(side) + ", "
                "price=" + priceToString(price) + ", "
                "qty=" + qtyToString(qty) + ", "
                "priority=" + priorityToString(priority) + "}";
        }
    };


    #pragma pack(pop)

    typedef LFQueue<MEMarketUpdate> MarketUpdateLFQueue;
}