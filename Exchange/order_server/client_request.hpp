#pragma once

#include "Common/types.hpp"
#include "Common/spsc_lf_queue.hpp"

using namespace Common;

namespace Exchange {
    #pragma pack(push, 1)
    enum class ClientRequestType : uint8_t {
        INVALID=0,
        NEW=1,
        CANCEL=2
    };

    inline auto clientRequestTypeToString(const ClientRequestType& t) -> std::string {
        switch(t){
            case ClientRequestType::INVALID: return "INVALID";
            case ClientRequestType::NEW: return "NEW";
            case ClientRequestType::CANCEL: return "CANCEL";
            default: return "UNKNOWN";
        }
    }

    struct MEClientRequest {
        ClientRequestType type = ClientRequestType::INVALID;
        ClientId clientId = ClientId_INVALID;
        TickerId tickerId = TickerId_INVALID;
        OrderId orderId = OrderId_INVALID;
        Side side = Side::INVALID;
        Price price = Price_INVALID;
        Qty qty = Qty_INVALID;

        inline auto toString() const -> std::string {
            return "MEClientRequest{"
                "type=" + clientRequestTypeToString(type) + ", "
                "clientId=" + clientIdToString(clientId) + ", "
                "tickerId=" + tickerIdToString(tickerId) + ", "
                "orderId=" + orderIdToString(orderId) + ", "
                "side=" + sideToString(side) + ", "
                "price=" + priceToString(price) + ", "
                "qty=" + qtyToString(qty) + "}";
        }
    };
    #pragma pack(pop)

    typedef LFQueue<MEClientRequest> ClientRequestLFQueue;
}