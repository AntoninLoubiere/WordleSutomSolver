#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_
#include <array>
#include <cassert>
#include <string>

#define DEBUG 0

#define d_assert(condition) d_assert_l(condition, 10)
#if DEBUG
#define d_assert_l(condition, level)                                                               \
    if (DEBUG >= level)                                                                            \
        assert(condition);
#else // DEBUG
#define d_assert_l(condition, level)
#endif // DEBUG
/**
 * @brief Power of 3
 *
 * @param k the multiplied term
 * @return 3^k
 */
constexpr int pow3(int k) {
    int result = 1;

    for (int i = 0; i < k; ++i) {
        result *= 3;
    }

    return result;
}

/**
 * @brief Fast version of pow() for powers of 3.
 *
 * @param x the multiplied term
 * @param y the exponant
 * @return x^y
 */
constexpr int pow(int x, int y) {
    if (x != 3 || y > 12) {
        int result = 1;

        for (int i = 0; i < y; ++i) {
            result *= x;
        }

        return result;
    } else {
        constexpr std::array<int, 14> cache{pow3(0),  pow3(1),  pow3(2),  pow3(3), pow3(4),
                                            pow3(5),  pow3(6),  pow3(7),  pow3(8), pow3(9),
                                            pow3(10), pow3(11), pow3(12), pow3(13)};

        return cache[y];
    }
}

int randomInt(int min, int max);

#endif // !SRC_UTILS_H_