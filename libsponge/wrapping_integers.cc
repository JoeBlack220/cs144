#include "wrapping_integers.hh"
#include <iostream>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}
const uint32_t bitmask_1 = 0xffffffffUL;
using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t tmp = (n&bitmask_1) + isn.raw_value();
    uint32_t raw_value = tmp&bitmask_1;
    return WrappingInt32{raw_value};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint32_t diff = n - wrap(checkpoint, isn);
    uint64_t res;
    
    // To avoid overflow
    // diff > UINT32_MAX/2 -> might be a negative diff
    // checkpoint > res, must be a negative diff 
    if (diff > UINT32_MAX/2 && checkpoint > checkpoint - (1ULL << 32) + diff) {
        res = checkpoint - (1ULL << 32) + diff;
    } else {
        res = checkpoint + diff;
    }
    return res;
}
