#ifndef STRUCT_UTIL_H
#define STRUCT_UTIL_H
#include <cstdint>
#include <unordered_set>
// ---------------------------------------------------------------------
// バイナリ用構造体
// ---------------------------------------------------------------------
struct OHLCetc{//データは10倍して保存,表示の際などに注意
    int32_t  Open,High,Low,Close,UL,LL;//ULLLはストップ二種
    uint32_t Vo;//出来高
    float AF;//AdjFactor、株式分割合併

    static constexpr int PRICE_SCALE = 10;
}__attribute__((packed));

inline const std::unordered_set<int> targetMkt = {101, 102, 104, 106, 107, 111, 112, 113};
//上記のinlineの数値に関してはJ-QuantsAPIの市場区分コード及び市場区分名を参照してください。//

struct StockCodeID{
    char code[6];
    uint16_t id;
}__attribute__((packed));
struct CSVpriceheader {
    short Open=-1, High=-1, Low=-1, Close=-1,date = -1;
    short UL=-1, LL=-1, Vo=-1, AF=-1,code = -1,Va = -1;

    static constexpr size_t count = 11;
};
#endif // STRUCT_UTIL_H

