#include "tcp_sender.hh"

#include "tcp_config.hh"
#include <algorithm>
#include <random>
#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _rto{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    if(is_closed()) {
        TCPSegment segment;
        segment.header().syn = true;
        send_segment(segment);
        return;
    } else if(is_syn_sent()) {
        return;
    } else if(_stream.buffer_empty() && !_stream.eof()) {
        return;
    } else if(is_fin_sent() || is_fin_acked()) {
        return;
    }

    if(_receiver_window_size) {
        while(_receiver_window_left) {
            TCPSegment segment;
            size_t payload_size = min({_stream.buffer_size(),
             static_cast<size_t>(_receiver_window_left),
             static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE)});
            segment.payload() = _stream.read(payload_size);

            if(_stream.eof() && _receiver_window_left > payload_size) {
                segment.header().fin = true;
            }

            send_segment(segment);

            if(_stream.buffer_empty()) {
                break;
            }
        }
    } else if(_receiver_window_left == 0) {
        TCPSegment segment;
        if(_stream.eof()) {
            segment.header().fin = true;
            send_segment(segment);
        } else if(!_stream.buffer_empty()) {
            segment.payload() = _stream.read(1);
            send_segment(segment);
        }
    }
}

bool TCPSender::ack_valid(uint64_t abs_ackno) {
    if (_segments_outstanding.empty())
        return abs_ackno <= _next_seqno;
    return abs_ackno <= _next_seqno &&
           abs_ackno >= unwrap(_segments_outstanding.front().header().seqno, _isn, _next_seqno);
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if(!ack_valid(abs_ackno)) {
        return;
    }

    _receiver_window_left = window_size;
    _receiver_window_size = window_size;

    while(!_segments_outstanding.empty()) {
        TCPSegment segment = _segments_outstanding.front();

        if(unwrap(segment.header().seqno, _isn, _next_seqno) + segment.length_in_sequence_space() <= abs_ackno) {
            _bytes_in_flight -= segment.length_in_sequence_space();
            _segments_outstanding.pop();
            _time_elapsed = 0;
            _rto = _initial_retransmission_timeout;
            _consecutive_retransmissions = 0;
        } else {
            break;
        }
    }    
    
    if (!_segments_outstanding.empty()) {
        _receiver_window_left = static_cast<uint16_t>(
            abs_ackno + static_cast<uint64_t>(window_size) -
            unwrap(_segments_outstanding.front().header().seqno, _isn, _next_seqno) - _bytes_in_flight);
    }

    if(!_bytes_in_flight) {
        _timer_running = false;
    }

    fill_window();


}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    if(!_timer_running) {
        return;
    }
    _time_elapsed += ms_since_last_tick;
    if(_time_elapsed >= _rto) {
        _segments_out.push(_segments_outstanding.front());
        if(_receiver_window_size || is_syn_sent()) {
            ++_consecutive_retransmissions;
            _rto <<= 1;
        }
        _time_elapsed = 0;
    }
 }

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment segment;
    segment.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(segment);
}

bool TCPSender::is_closed() const {
    return next_seqno_absolute() == 0;
}

// SYN sent but not acked
bool TCPSender::is_syn_sent() const {
    return next_seqno_absolute() > 0 && next_seqno_absolute() == bytes_in_flight();
}

bool TCPSender::is_syn_acked() const {
    return next_seqno_absolute() > bytes_in_flight() && !stream_in().eof();
}

bool TCPSender::is_fin_sent() const {
    return stream_in().eof() && next_seqno_absolute() == (stream_in().bytes_written() + 2) && bytes_in_flight() > 0;
}

bool TCPSender::is_fin_acked() const {
    return stream_in().eof() && next_seqno_absolute() == (stream_in().bytes_written() + 2) && bytes_in_flight() == 0;
}

void TCPSender::send_segment(TCPSegment segment) {
    segment.header().seqno = wrap(_next_seqno, _isn);
    size_t len_increased = segment.length_in_sequence_space();
    _next_seqno += len_increased;
    _bytes_in_flight += len_increased;
    _receiver_window_left -= len_increased;

    _segments_out.push(segment);
    _segments_outstanding.push(segment);

    if(!_timer_running) {
        _timer_running = true;
        _time_elapsed = 0;
    }
}