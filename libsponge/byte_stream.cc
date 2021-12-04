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

ByteStream::ByteStream(const size_t capacity):buffer(std::vector<char>()),_cap(capacity){}

size_t ByteStream::write(const string &data) {
    if(input_ended() || error()) {
        return 0;
    }
    size_t cur_num_written = 0;
    int index = 0;
    int limit = data.length();
    while(buffer.size() < _cap && index < limit) {
        buffer.push_back(data[index]);
        index++;
        cur_num_written++;
        _num_written++;
    }
    return cur_num_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if(error()) {
        return {};
    }
    ostringstream output;
    size_t index = 0;
    while(index<len&&index<buffer.size()) {
        output << buffer[index++];
    }
    return output.str();
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    if(error()) {
        return;
    }
    size_t limit = min(len, buffer.size());
    _num_read += limit;
    buffer = std::vector<char>(buffer.begin()+limit, buffer.end()); 
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    if(error()) {
        return {};
    }
    size_t limit = min(len, buffer.size());
    ostringstream output;
    size_t index = 0;
    while(index<limit) {
        output << buffer[index++];
        _num_read++;
    }
    buffer = std::vector<char>(buffer.begin()+limit, buffer.end());
    return output.str();
}

void ByteStream::end_input() {
    if(error()) {
        return;
    }
    _is_ended = true;
}

bool ByteStream::input_ended() const { return _is_ended; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return input_ended()&&buffer_empty(); }

size_t ByteStream::bytes_written() const { return _num_written; }

size_t ByteStream::bytes_read() const { return _num_read; }

size_t ByteStream::remaining_capacity() const { return _cap-buffer.size(); }
