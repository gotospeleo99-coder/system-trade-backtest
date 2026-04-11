#ifndef STRUCT_UTIL_H
#define STRUCT_UTIL_H
#include <cstdint>
#include <string>

// ---------------------------------------------------------------------
// バイナリ用構造体
// ---------------------------------------------------------------------
struct OHLCetc{
    int32_t  Open,High,Low,Close,UL,LL;
    uint32_t Vo;
    double AF;//AdjFactor
}__attribute__((packed));
struct StockCodeID{
    char code[6];
    uint16_t id;
}__attribute__((packed));

#endif // STRUCT_UTIL_H

