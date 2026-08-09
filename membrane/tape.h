// membrane/tape.h — the warpbus durable tape, fusion v0 (FRESH-8; auricle durable.h
// pattern re-grown in the fusion namespace). Committed lines append to a fixed-record
// hash-chained binary log; partials never persist; restore re-verifies the chain.
#pragma once
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace fusion::membrane {

#pragma pack(push, 1)
struct Rev {                       // 256 bytes, trivially copyable
    uint64_t rev_id;
    int64_t  ms;
    char     lane[8];
    char     text[216];
    uint64_t prev_h, h;
};
#pragma pack(pop)
static_assert(sizeof(Rev) == 256, "Rev must stay fixed-size");

inline uint64_t rev_fnv(const Rev& r) {
    Rev c = r; c.h = 0;
    uint64_t h = 14695981039346656037ull ^ r.prev_h;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(&c);
    for (size_t i = 0; i < sizeof(Rev); ++i) { h ^= p[i]; h *= 1099511628211ull; }
    return h;
}

inline Rev make_rev(uint64_t id, int64_t ms, const std::string& lane,
                    const std::string& text, uint64_t prev_h) {
    Rev r{}; r.rev_id = id; r.ms = ms; r.prev_h = prev_h;
    std::strncpy(r.lane, lane.c_str(), sizeof(r.lane) - 1);
    std::strncpy(r.text, text.c_str(), sizeof(r.text) - 1);
    r.h = rev_fnv(r);
    return r;
}

class Tape {                       // append-only writer (fresh log per process)
public:
    explicit Tape(const std::string& path) : out_(path, std::ios::binary | std::ios::trunc) {}
    bool ok() const { return static_cast<bool>(out_); }
    void append(int64_t ms, const std::string& lane, const std::string& text) {
        Rev r = make_rev(next_id_++, ms, lane, text, prev_h_);
        out_.write(reinterpret_cast<const char*>(&r), sizeof(Rev));
        prev_h_ = r.h;
    }
    void flush() { out_.flush(); }
private:
    std::ofstream out_; uint64_t next_id_ = 0, prev_h_ = 0;
};

inline std::vector<Rev> tape_restore(const std::string& path) {
    std::vector<Rev> out; std::ifstream in(path, std::ios::binary);
    Rev r;
    while (in.read(reinterpret_cast<char*>(&r), sizeof(Rev))) out.push_back(r);
    return out;
}
inline bool tape_verify(const std::vector<Rev>& t, uint64_t* bad = nullptr) {
    uint64_t prev = 0;
    for (const Rev& r : t) {
        if (r.prev_h != prev || rev_fnv(r) != r.h) { if (bad) *bad = r.rev_id; return false; }
        prev = r.h;
    }
    return true;
}

} // namespace fusion::membrane
