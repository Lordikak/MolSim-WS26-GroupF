#include "MSBFReader.h"
#include <algorithm>
#include <array>
#include <bit>
#include <stdexcept>


namespace msb {
void MSBFReader::ReadBytes(std::ifstream &in, std::span<std::byte> out) {
    in.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(out.size()));
    if (in.gcount() != static_cast<std::streamsize>(out.size()))
        throw std::runtime_error("MSB: unexpected end of file");
}
Header MSBFReader::ReadHeader() {
    std::array<std::byte, sizeof(Header)> buf;
    ReadBytes(data_, buf);
    return std::bit_cast<Header>(buf);
}
FrameHeader MSBFReader::ReadFrameHeader() {
    std::array<std::byte, sizeof(FrameHeader)> buf;
    ReadBytes(data_, buf);
    return std::bit_cast<FrameHeader>(buf);
}
IndexEntry MSBFReader::ReadIndexEntry() {
    std::array<std::byte, sizeof(IndexEntry)> buf;
    ReadBytes(idx_, buf);
    return std::bit_cast<IndexEntry>(buf);
}
MSBFReader::MSBFReader(const std::filesystem::path &msb) : data_path_(msb), idx_path_(std::filesystem::path(msb) += ".idx"), data_(data_path_, std::ios::binary), idx_(idx_path_, std::ios::binary) {
    if (!data_ || !idx_) throw std::runtime_error("MSB: cannot open " + msb.string());

    header_ = ReadHeader();
    if (header_.magic != kMagic) throw std::runtime_error("MSB: not an MSB file " + data_path_.string());
    if (header_.version != kFmtVersion) throw std::runtime_error("MSB: not an MSB file " + data_path_.string());

    Refresh();
}

std::size_t MSBFReader::Refresh() {
    const auto data_size = std::filesystem::file_size(data_path_);
    const auto idx_size = std::filesystem::file_size(idx_path_);

    if (idx_size < entries_.size() * sizeof(IndexEntry)) {
        entries_.clear();
        data_.clear();
        data_.close();
        data_.open(data_path_, std::ios::binary);
        idx_.close();
        idx_.open(idx_path_, std::ios::binary);
        header_ = ReadHeader();
        frame_.step = Frame::kNone;
        if (header_.magic != kMagic) throw std::runtime_error("MSB: not an MSB file " + data_path_.string());
        if (header_.version != kFmtVersion) throw std::runtime_error("MSB: unsupported version " + data_path_.string());
    }

    const std::size_t available = idx_size/sizeof(IndexEntry);
    idx_.clear();
    idx_.seekg(static_cast<std::streamoff>(entries_.size() * sizeof(IndexEntry)));

    while (entries_.size() < available) {
        const IndexEntry entry = ReadIndexEntry();
        if (entry.offset + FrameSize(entry.n) > data_size) break;
        entries_.push_back(entry);
    }
    return entries_.size();
}

const Frame &MSBFReader::LoadAtStep(const std::uint32_t step) {
    if (entries_.empty()) return frame_;

    const auto it = std::ranges::upper_bound(entries_, step, {}, &IndexEntry::step);
    const auto i = static_cast<std::size_t>(std::max<std::ptrdiff_t>(it - entries_.begin() - 1, 0));

    if (entries_[i].step != frame_.step) LoadIndex(i);

    return frame_;
}
const Frame &MSBFReader::LoadAtTime(double time) {
    const auto step = static_cast<std::uint32_t>((time - header_.start_time) / header_.dt);
    return LoadAtStep(step);
}

void MSBFReader::LoadIndex(std::size_t i) {
    const IndexEntry &entry = entries_[i];
    frame_.step = Frame::kNone;

    data_.clear();
    data_.seekg(static_cast<std::streamoff>(entry.offset));

    const FrameHeader fh = ReadFrameHeader();
    if (fh.step != entry.step || fh.n != entry.n) throw std::runtime_error("MSB: data and index dont match");

    frame_.positions.resize(entry.n);
    frame_.vels.resize(entry.n);
    ReadBytes(data_, std::as_writable_bytes(std::span(frame_.positions)));
    ReadBytes(data_, std::as_writable_bytes(std::span(frame_.vels)));

    frame_.n = entry.n;
    frame_.step = entry.step;
}
std::uint32_t MSBFReader::FirstStep() const { return entries_.empty() ? Frame::kNone : entries_.front().step; }
std::uint32_t MSBFReader::LastStep() const  { return entries_.empty() ? Frame::kNone : entries_.back().step; }
const Header &MSBFReader::header() const    { return header_; }
}
