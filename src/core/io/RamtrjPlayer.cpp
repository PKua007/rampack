//
// Created by pkua on 04.04.2022.
//

#include <sstream>

#include "RamtrjPlayer.h"
#include "utils/Exceptions.h"


RamtrjPlayer::RamtrjPlayer(std::unique_ptr<std::istream> in) : in{std::move(in)} {
    this->in->seekg(0);
    this->header = RamtrjIO::readHeader(*this->in);

    this->in->seekg(0, std::ios_base::end);
    std::streamoff expectedPos = RamtrjIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
    ValidateMsg(this->in->tellg() == expectedPos, "RAMTRJ read error: broken snapshot structure");

    this->reset();
}

RamtrjPlayer::RamtrjPlayer(std::unique_ptr<std::istream> in, RamtrjPlayer::AutoFix &autoFix)
        : in{std::move(in)}
{
    this->in->seekg(0);
    try {
        this->header = RamtrjIO::readHeader(*this->in);
    } catch (const RamtrjException &ex) {
        autoFix.reportError(ex.what());
        std::rethrow_exception(std::current_exception());
    }

    std::streamoff expectedPos = RamtrjIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
    this->in->seekg(0, std::ios_base::end);
    std::streamoff realPos = this->in->tellg();
    if (realPos == expectedPos) {
        autoFix.reportNofix(this->header);
    } else {
        std::size_t snapshotBytes = realPos - RamtrjIO::getHeaderSize();
        autoFix.tryFixing(this->header, snapshotBytes);
    }

    this->reset();
}

bool RamtrjPlayer::hasNext() const {
    if (this->in == nullptr)
        return false;

    return this->currentSnapshot < this->header.numSnapshots;
}

void RamtrjPlayer::nextSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager) {
    Expects(this->hasNext());
    Expects(packing.size() == this->header.numParticles);

    TriclinicBox newBox = RamtrjIO::readBox(*this->in);
    std::vector<Shape> newShapes;
    newShapes.reserve(this->header.numParticles);
    for (std::size_t i{}; i < packing.size(); i++)
        newShapes.push_back(RamtrjIO::readShape(*this->in));

    packing.reset(std::move(newShapes), newBox, interaction, dataManager);

    this->currentSnapshot++;
}

void RamtrjPlayer::lastSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager) {
    this->jumpToSnapshot(packing, interaction, dataManager, this->header.numSnapshots * this->header.cycleStep);
}

void RamtrjPlayer::jumpToSnapshot(Packing &packing, const Interaction &interaction, const ShapeDataManager &dataManager,
                                  std::size_t cycleNumber)
{
    Expects(this->in != nullptr);
    Expects(this->header.numSnapshots > 0);
    Expects(cycleNumber % this->header.cycleStep == 0);

    std::size_t snapshotNumber = cycleNumber / this->header.cycleStep;
    Expects(snapshotNumber > 0);
    Expects(snapshotNumber <= this->header.numSnapshots);

    this->currentSnapshot = snapshotNumber - 1;
    auto offset = RamtrjIO::streamoffForSnapshot(this->header, this->currentSnapshot);
    this->in->seekg(offset);
    this->nextSnapshot(packing, interaction, dataManager);
}

std::size_t RamtrjPlayer::getCurrentSnapshotCycles() const {
    return this->currentSnapshot * this->header.cycleStep;
}

void RamtrjPlayer::close() {
    this->in = nullptr;
}

void RamtrjPlayer::dumpHeader(Logger &out) const {
    out.info() << "RAMTRJ: RAMPACK trajectory file" << std::endl;
    out << "file version            : " << static_cast<int>(this->header.versionMajor) << ".";
    out << static_cast<int>(this->header.versionMinor) << std::endl;
    out << "number of particles     : " << this->header.numParticles << std::endl;
    out << "number of snapshots     : " << this->header.numSnapshots << std::endl;
    out << "cycle step              : " << this->header.cycleStep << std::endl;
    out << "total number of cycles  : " << (this->header.cycleStep * this->header.numSnapshots) << std::endl;
}

void RamtrjPlayer::reset() {
    auto pos = static_cast<std::streamoff>(RamtrjPlayer::getHeaderSize());
    this->in->seekg(pos);
    this->currentSnapshot = 0;
}

void RamtrjPlayer::AutoFix::dumpInfo(Logger &out) const {
    if (!this->fixingSuccessful) {
        out.error() << "Trajectory was invalid and fixing it failed. Error message: " << std::endl;
        out.error() << this->errorMessage << std::endl;
        return;
    }

    if (!this->fixingNeeded) {
        out.info() << "Trajectory was completely valid. No fixes were made." << std::endl;
        return;
    }

    out.warn() << "Trajectory file was truncated, but could be fixed. The summary:" << std::endl;
    out << "Alleged number of snapshots  : " << this->headerSnapshots << std::endl;
    out << "Inferred number of snapshots : " << this->inferredSnapshots << std::endl;
    if (this->bytesRemainder == 0) {
        out << "The last snapshot was not truncated" << std::endl;
    } else {
        out << "The last snapshot was truncated: only " << this->bytesRemainder << "/" << this->bytesPerSnapshot;
        out << " bytes were read" << std::endl;
    }
}

RamtrjPlayer::AutoFix::AutoFix(std::size_t expectedNumMolecules) : expectedNumMolecules{expectedNumMolecules} {
    Expects(expectedNumMolecules > 0);
}

void RamtrjPlayer::AutoFix::reportNofix(const RamtrjIO::Header &header_) {
    this->fixingSuccessful = true;
    this->bytesPerSnapshot = getSnapshotSize(header_);
    this->headerSnapshots = header_.numSnapshots;
    this->fixingNeeded = false;
    this->inferredSnapshots = header_.numSnapshots;
}

void RamtrjPlayer::AutoFix::reportError(const std::string &message) {
    this->fixingNeeded = false;
    this->fixingSuccessful = false;
    this->errorMessage = message;
}

void RamtrjPlayer::AutoFix::tryFixing(RamtrjIO::Header &header_, std::size_t snapshotBytes) {
    if (header_.numParticles == 0 || header_.cycleStep == 0) {
        Assert(header_.versionMajor == 1 && header_.versionMinor == 0);
        this->reportError("The header does not contain information about the number of particles or cycle step - it was"
                          " not always stored in RAMTRJ v1.0.");
        throw RamtrjException(this->errorMessage);
    } else if (header_.numParticles != this->expectedNumMolecules) {
        this->fixingSuccessful = false;
        std::ostringstream error;
        error << "Expected number of molecules (" << this->expectedNumMolecules << ") != number of molecules ";
        error << "in the input (" << header_.numParticles << ")";
        this->reportError(error.str());
        throw RamtrjException(this->errorMessage);
    }

    this->fixingNeeded = true;
    this->fixingSuccessful = true;
    this->bytesPerSnapshot = RamtrjIO::getSnapshotSize(header_);
    header_.numSnapshots = this->inferredSnapshots = snapshotBytes / this->bytesPerSnapshot;
    this->bytesRemainder = snapshotBytes % this->bytesPerSnapshot;
}
