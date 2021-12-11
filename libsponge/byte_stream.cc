#include "byte_stream.hh"
#include <vector>
#include <sstream>
#include <iostream>
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):buffer(std::vector<char>(capacity)),_cap(capacity){}

size_t ByteStream::write(const string &data) {
    if(input_ended() || error()) {
        return 0;
    }
    size_t cur_num_written = 0;
    int index = 0;
    int limit = data.length();
    while(!_is_full && index < limit) {
        buffer[_end_ptr++] = data[index++];
        cur_num_written++;
        _num_written++;
        if(_end_ptr == _cap) {
            _end_ptr = 0;
        }
        if(_begin_ptr == _end_ptr) {
            _is_full = true;
        }
        if(_is_empty) {
            _is_empty = false;
        }
    }
    return cur_num_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if(error()) {
        return {};
    }
    if(_is_empty) {
        return "";
    }
    ostringstream output;
    size_t cur_ptr = _begin_ptr;
    size_t cur_peeked = 0;
    while(cur_peeked < len) {
        output << buffer[cur_ptr++];
        cur_peeked++;
        if(cur_ptr == _cap) {
            cur_ptr = 0;
        }
        if(cur_ptr == _end_ptr) break;
    }
    return output.str();
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    if(error()) {
        return;
    }
    size_t cur_poped = 0;
    while(!_is_empty && cur_poped < len) {
        _begin_ptr++;
        cur_poped++;
        _num_read++;

        if(_is_full) {
            _is_full = false;
        }
        if(_begin_ptr == _cap) {
            _begin_ptr = 0;
        }
        if(_begin_ptr == _end_ptr) {
            _is_empty = true;
        }
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    if(error()) {
        return {};
    }
    ostringstream output;
    size_t index = 0;
    while(!_is_empty && index<len) {
        output << buffer[_begin_ptr++];
        _num_read++;
        index++;
        if(_is_full) {
            _is_full = false;
        }
        if(_begin_ptr == _cap) {
            _begin_ptr = 0;
        }
        if(_begin_ptr == _end_ptr) {
            _is_empty = true;
        }
    }
    return output.str();
}

void ByteStream::end_input() {
    if(error()) {
        return;
    }
    _is_ended = true;
}

bool ByteStream::input_ended() const { return _is_ended; }

size_t ByteStream::buffer_size() const { 
    if(_end_ptr > _begin_ptr) {
        return _end_ptr - _begin_ptr;
    } else if(_end_ptr == _begin_ptr) {
        if(_is_empty) {
            return 0;
        } else {
            return buffer.size();
        }
    } else {
        return buffer.size() - _begin_ptr + _end_ptr;
    }
}

bool ByteStream::buffer_empty() const { return _is_empty; }

bool ByteStream::eof() const { return input_ended()&&buffer_empty(); }

size_t ByteStream::bytes_written() const { return _num_written; }

size_t ByteStream::bytes_read() const { return _num_read; }

size_t ByteStream::remaining_capacity() const { return _cap-buffer_size(); }
