//
// Created by pkua on 04.04.2022.
//

#include "RamtrjRecorder.h"


RamtrjRecorder::RamtrjRecorder(std::unique_ptr<std::iostream> stream_, const Packing &packing,
                               const ShapeDataManager &manager, std::size_t cycleStep, bool append)
        : stream{std::move(stream_)}
{
    Expects(!packing.empty());
    Expects(cycleStep > 0);

    if (append) {
        this->stream->seekg(0);
        this->header = RamtrjIO::readHeaderAndShapeData(manager, *this->stream);

        ValidateMsg(packing.size() == this->header.numParticles && cycleStep == this->header.cycleStep,
                    "RAMTRJ append error: unmatching number of molecules and/or cycle step");

        this->stream->seekp(0, std::ios_base::end);
        std::streamoff expectedPos = RamtrjIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
        ValidateMsg(this->stream->tellp() == expectedPos, "RAMTRJ append error: broken snapshot structure");
    } else {
        this->stream->seekp(0, std::ios_base::end);
        ValidateMsg(this->stream->tellp() == 0, "RAMTRJ error: append = false however stream is not empty");

        this->header.numParticles = packing.size();
        this->header.cycleStep = cycleStep;
        this->header.shapeDatas.reserve(this->header.numParticles);
        for (const auto &shape : packing)
            this->header.shapeDatas.push_back(shape.getData());

        RamtrjIO::writeHeaderAndShapeData(this->header, manager, *this->stream);
    }
}

RamtrjRecorder::~RamtrjRecorder() {
    this->close0();
}

void RamtrjRecorder::recordSnapshot(const Packing &packing, [[maybe_unused]] const ShapeTraits &traits,
                                    std::size_t cycle)
{
    Expects(this->stream != nullptr);
    Expects(cycle > 0);
    Expects(cycle == (this->header.numSnapshots + 1) * this->header.cycleStep);
    Expects(packing.size() == this->header.numParticles);
    auto shapeDataComparator = [](const Shape &shape, const ShapeData &data) { return shape.getData() == data; };
    Expects(std::equal(packing.begin(), packing.end(), this->header.shapeDatas.begin(), shapeDataComparator));

    RamtrjIO::writeBox(packing.getBox(), *this->stream);
    for (const auto &shape : packing)
        RamtrjIO::writeShape(shape, *this->stream);

    this->header.numSnapshots++;
}

void RamtrjRecorder::close0() {
    if (this->stream == nullptr)
        return;

    this->stream->seekp(0);
    RamtrjIO::writeHeader(this->header, *this->stream);

    this->stream = nullptr;
}
