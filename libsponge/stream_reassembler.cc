#include "stream_reassembler.hh"
#include <queue>
#include <iostream>
#include <map>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _eof_index(-1) {    cout << "hello\n";
}

// Checks if it reaches the eof, if it is, close the output stream.
void StreamReassembler::check_eof() {
    if(_last_index == _eof_index) {
        _output.end_input();
    }
}

// Try to add the byte from the buffer to the output stream in order.
void StreamReassembler::do_write(){
    check_eof();
    if(_map.empty()) {
        return;
    }
    size_t start = _map.begin()->first;
    while(start==_last_index && _output.remaining_capacity() != 0) {
        char c = _map.begin()->second;
        // Convert the char to string and write to the output stream.
        _output.write(string(1, c));
        _map.erase(start);
        _last_index++;
        if(_map.size()==0) break;
        start=_map.begin()->first;
    }
    check_eof();
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(_output.error()||_output.input_ended()) {
        return;
    }
    
    size_t start = 0;

    // Try to write it before reading the data in case the output stream still has some space.
    do_write();
    // Set eof if we receives the signal.
    if(eof) {
        _eof_index = index + data.length();
    }
    while(start < data.length()) {
        if(start+index < _last_index) {
            start++;
            continue;
        }
        _map[start+index] = data[start];
        // If the map is full, remove the last element
        while(_map.size() > _capacity) {
            _map.erase((--_map.end())->first);
        }
        start++;
    }
    do_write();
}

size_t StreamReassembler::unassembled_bytes() const { return _map.size(); }

bool StreamReassembler::empty() const { return _last_index == 0;}
