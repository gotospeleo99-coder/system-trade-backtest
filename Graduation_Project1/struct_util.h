#ifndef STRUCT_UTIL_H
#define STRUCT_UTIL_H
#include <cstdint>
#include <string>

// ---------------------------------------------------------------------
// バイナリ用構造体
// ---------------------------------------------------------------------
struct OHLC{
    uint32_t  Open ;
    uint32_t  High ;
    uint32_t  Low  ;
    uint32_t  Close;
}__attribute__((packed));
struct StockCodeID{
    char code[6];
    uint16_t id;
}__attribute__((packed));

#endif // STRUCT_UTIL_H

