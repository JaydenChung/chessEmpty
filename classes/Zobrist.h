#include <iostream>

//
// from https://github.com/lemire/zobristhashing/
//

enum {
    MAX_ZOBRIST_LENGTH=256
};

struct zobristStruct 
{
  uint64_t hashtab[MAX_ZOBRIST_LENGTH][1 << CHAR_BIT] ;
};

class Zobrist 
{
public:
    static uint64_t get64rand() {
        return
        (((uint64_t) rand() <<  0) & 0x000000000000FFFFull) |
        (((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) |
        (((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
        (((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
    }

    Zobrist() 
    {
        for ( int32_t i = 0 ; i < MAX_ZOBRIST_LENGTH ; i++ ) {
            for ( int32_t j = 0 ; j < ( 1 << CHAR_BIT) ; j++ ) {
                _zobrist.hashtab [i][j]  = get64rand();
            }
    }
    }

    uint64_t ZobristHash(const char *signeds, size_t length) {
        const unsigned char *s = (const unsigned char *) signeds;
        uint64_t h = 0;
        if(length > MAX_ZOBRIST_LENGTH) length = MAX_ZOBRIST_LENGTH;
        size_t i = 0;
        for ( ; i + 7  < length ; i += 8 ) {
            h ^= _zobrist.hashtab [ i ] [s[i]];
            h ^= _zobrist.hashtab [ i + 1 ] [s[i + 1]];
            h ^= _zobrist.hashtab [ i + 2 ] [s[i + 2]];
            h ^= _zobrist.hashtab [ i + 3 ] [s[i + 3]];
            h ^= _zobrist.hashtab [ i + 4 ] [s[i + 4]];
            h ^= _zobrist.hashtab [ i + 5 ] [s[i + 5]];
            h ^= _zobrist.hashtab [ i + 6 ] [s[i + 6]];
            h ^= _zobrist.hashtab [ i + 7 ] [s[i + 7]];
        }
        for (; i < length ; i++ ) {
            h ^= _zobrist.hashtab [ i ] [s[i]];
        }
        return h;
    }

private:
    zobristStruct _zobrist;
};
