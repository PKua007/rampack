//
// Created by pkua on 04.04.2022.
//

#include "SimulationPlayer.h"
#include "utils/Assertions.h"


SimulationPlayer::SimulationPlayer(std::unique_ptr<std::istream> in) : in{std::move(in)} {
    this->in->seekg(0);
    this->header = SimulationIO::readHeader(*this->in);

    std::streamoff savePos = this->in->tellg();
    this->in->seekg(0, std::ios_base::end);
    std::streamoff expectedPos = SimulationIO::streamoffForSnapshot(this->header, this->header.numSnapshots);
    ValidateMsg(this->in->tellg() == expectedPos, "RAMTRJ read error: broken snapshot structure");
    this->in->seekg(savePos);
}

bool SimulationPlayer::hasNext() const {
    return this->currentSnapshot < this->header.numSnapshots;
}

void SimulationPlayer::nextSnapshot(Packing &packing, const Interaction &interaction) {
    Expects(packing.size() == this->header.numParticles);

    TriclinicBox box = SimulationIO::readBox(*this->in);
    packing.tryScaling(box, interaction);
    for (std::size_t i{}; i < packing.size(); i++) {
        const Shape &oldShape = packing[i];
        Shape newShape = SimulationIO::readShape(*this->in);
        Vector<3> translation = newShape.getPosition() - oldShape.getPosition();
        Matrix<3, 3> rotation = newShape.getOrientation() * oldShape.getOrientation().inverse();

        packing.tryMove(i, translation, rotation, interaction);
        packing.acceptMove();
    }

    this->currentSnapshot++;
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
