//
// Created by pkua on 04.04.2022.
//

#include <sstream>

#include "SimulationPlayer.h"
#include "utils/Assertions.h"


SimulationPlayer::SimulationPlayer(std::unique_ptr<std::istream> in) : in{std::move(in)} {
    this->in->seekg(0);
    this->header = SimulationIO::readHeader(*this->in);

    this->in->seekg(0, std::ios_base::end);
    std::streamoff expectedPos = SimulationIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
    ValidateMsg(this->in->tellg() == expectedPos, "RAMTRJ read error: broken snapshot structure");

    this->reset();
}

SimulationPlayer::SimulationPlayer(std::unique_ptr<std::istream> in, SimulationPlayer::AutoFix &autoFix)
        : in{std::move(in)}
{
    this->in->seekg(0);
    try {
        this->header = SimulationIO::readHeader(*this->in);
    } catch (const ValidationException &ex) {
        autoFix.fixingSuccessful = false;
        autoFix.errorMessage = ex.what();
        std::rethrow_exception(std::current_exception());
    }

    std::streamoff savedPos = this->in->tellg();

    std::streamoff expectedPos = SimulationIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
    this->in->seekg(0, std::ios_base::end);
    std::streamoff realPos = this->in->tellg();
    if (realPos == expectedPos) {
        autoFix.reportNofix(this->header);
    } else {
        std::size_t snapshotBytes = realPos - SimulationIO::getHeaderSize();
        autoFix.tryFixing(this->header, snapshotBytes);
    }

    this->in->seekg(savedPos);
}

bool SimulationPlayer::hasNext() const {
    if (this->in == nullptr)
        return false;

    return this->currentSnapshot < this->header.numSnapshots;
}

void SimulationPlayer::nextSnapshot(Packing &packing, const Interaction &interaction) {
    Expects(this->in != nullptr);
    Expects(packing.size() == this->header.numParticles);

    TriclinicBox newBox = SimulationIO::readBox(*this->in);
    std::vector<Shape> newShapes;
    newShapes.reserve(this->header.numParticles);
    for (std::size_t i{}; i < packing.size(); i++)
        newShapes.push_back(SimulationIO::readShape(*this->in));

    packing.reset(std::move(newShapes), newBox, interaction);

    this->currentSnapshot++;
}

void SimulationPlayer::lastSnapshot(Packing &packing, const Interaction &interaction) {
    this->jumpToSnapshot(packing, interaction, this->header.numSnapshots * this->header.cycleStep);
}

void SimulationPlayer::jumpToSnapshot(Packing &packing, const Interaction &interaction, std::size_t cycleNumber) {
    Expects(this->in != nullptr);
    Expects(this->header.numSnapshots > 0);
    Expects(cycleNumber % this->header.cycleStep == 0);

    std::size_t snapshotNumber = cycleNumber / this->header.cycleStep;
    Expects(snapshotNumber > 0);
    Expects(snapshotNumber <= this->header.numSnapshots);

    this->currentSnapshot = snapshotNumber - 1;
    auto offset = SimulationIO::streamoffForSnapshot(this->header, this->currentSnapshot);
    this->in->seekg(offset);
    this->nextSnapshot(packing, interaction);
}

std::size_t SimulationPlayer::getCurrentSnapshotCycles() const {
    return this->currentSnapshot * this->header.cycleStep;
}

void SimulationPlayer::close() {
    this->in = nullptr;
}

void SimulationPlayer::dumpHeader(Logger &out) const {
    out.info() << "RAMTRJ: RAMPACK trajectory file" << std::endl;
    out << "file version            : " << static_cast<int>(this->header.versionMajor) << ".";
    out << static_cast<int>(this->header.versionMinor) << std::endl;
    out << "number of particles     : " << this->header.numParticles << std::endl;
    out << "number of snapshots     : " << this->header.numSnapshots << std::endl;
    out << "cycle step              : " << this->header.cycleStep << std::endl;
    out << "total number of cycles  : " << (this->header.cycleStep * this->header.numSnapshots) << std::endl;
}

void SimulationPlayer::reset() {
    auto pos = static_cast<std::streamoff>(SimulationPlayer::getHeaderSize());
    this->in->seekg(pos);
    this->currentSnapshot = 0;
}

void SimulationPlayer::AutoFix::dumpInfo(Logger &out) const {
    if (!this->fixingSuccessful) {
        out.error() << "Trajectory was invalid and fixing it failed. Error message: " << std::endl;
        out.error() << this->errorMessage << std::endl;
        return;
    }

    if (!this->wasFixed) {
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

SimulationPlayer::AutoFix::AutoFix(std::size_t expectedNumMolecules) : expectedNumMolecules{expectedNumMolecules} {
    Expects(expectedNumMolecules > 0);
}

void SimulationPlayer::AutoFix::reportNofix(const SimulationIO::Header &header_) {
    this->fixingSuccessful = true;
    if (header_.numParticles != 0)
        this->bytesPerSnapshot = getSnapshotSize(header_);
    this->headerSnapshots = header_.numSnapshots;
    this->wasFixed = false;
    this->inferredSnapshots = header_.numSnapshots;
}

void SimulationPlayer::AutoFix::reportError(const std::string &message) {
    this->wasFixed = false;
    this->fixingSuccessful = false;
    this->errorMessage = message;
}

void SimulationPlayer::AutoFix::tryFixing(SimulationIO::Header &header_, std::size_t snapshotBytes) {
    if (header_.numParticles == 0 || header_.cycleStep == 0) {
        Assert(header_.versionMajor == 1 && header_.versionMinor == 0);
        this->reportError("The header does not contain information about the number of particles or cycle step - it was"
                          " not always stored in RAMTRJ v1.0.");
        throw ValidationException(this->errorMessage);
    } else if (header_.numParticles != this->expectedNumMolecules) {
        this->fixingSuccessful = false;
        std::ostringstream error;
        error << "Expected number of molecules (" << this->expectedNumMolecules << ") != number of molecules ";
        error << "in the input (" << header_.numParticles << ")";
        this->reportError(error.str());
        throw ValidationException(this->errorMessage);
    }

    this->wasFixed = true;
    this->fixingSuccessful = true;
    this->bytesPerSnapshot = SimulationIO::getSnapshotSize(header_);
    header_.numSnapshots = this->inferredSnapshots = snapshotBytes / this->bytesPerSnapshot;
    this->bytesRemainder = snapshotBytes % this->bytesPerSnapshot;
}
