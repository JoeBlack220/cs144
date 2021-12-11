#include "tcp_receiver.hh"
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    string data = seg.payload().copy();
    TCPHeader header = seg.header();
    WrappingInt32 seqno = header.seqno;

    // Already received SYN and the header has SYN
    // Or no SYN met yet, but header is not SYN
    if((header.syn&&_syn) || (!header.syn&&!_syn)) {
        return;
    }
    if(header.syn) {
        _isn = seqno;
        _syn = true;
    }
    if(_syn && header.fin) {
        _fin = true;
    }
    size_t index = unwrap(seqno, _isn, _checkpoint);

    _checkpoint = index;
    
    // According to the lab, actual data starts in the seqno 1.
    _reassembler.push_substring(data, header.syn?0:index-1, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    // SYN and FIN takes one index separately.
    size_t shift = 1;
    if(_fin && _reassembler.unassembled_bytes() == 0) {
        shift++;
    }
    if(_syn) {
        return wrap(_reassembler.get_last_index()+shift, _isn);
    }
    return {};
}

size_t TCPReceiver::window_size() const { 
    return _capacity - stream_out().buffer_size();
}
