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
    std::streamoff expectedPos = SimulationIO::posForSnapshot(this->header, this->header.numSnapshots);
    ValidateMsg(this->in->tellg() == expectedPos, "RAMTRJ read error: broken snapshot structure");
    this->in->seekg(savePos);
}

bool SimulationPlayer::hasNext() const {
    return this->currentSnapshot < this->header.numSnapshots;
}

void SimulationPlayer::nextSnapshot(Packing &packing, const Interaction &interaction) {
    Expects(packing.size() == this->header.numParticles);

    BoxData boxData{};
    this->in->read(reinterpret_cast<char*>(boxData.dimensions), sizeof(boxData.dimensions));
    ValidateMsg(*this->in, "RAMTRJ read error: snapshot box data");

    TriclinicBox box = boxData.toTriclinicBox();
    packing.tryScaling(box, interaction);
    for (std::size_t i{}; i < packing.size(); i++) {
        ParticleData particleData;
        this->in->read(reinterpret_cast<char*>(particleData.position), sizeof(particleData.position));
        this->in->read(reinterpret_cast<char*>(particleData.eulerAngles), sizeof(particleData.eulerAngles));
        ValidateMsg(*this->in, "RAMTRJ read error: snapshot particle data");

        const Shape &oldShape = packing[i];
        Shape newShape = particleData.toShape();
        Vector<3> translation = newShape.getPosition() - oldShape.getPosition();
        Matrix<3, 3> rotation = newShape.getOrientation() * oldShape.getOrientation().inverse();

        packing.tryMove(i, translation, rotation, interaction);
        packing.acceptMove();
    }

    this->currentSnapshot++;
}

std::size_t SimulationPlayer::getCurrentSnapshotCycles() const {
    return (this->currentSnapshot + 1) * this->header.cycleStep;
}

void SimulationPlayer::close() {
    this->in = nullptr;
}

void SimulationPlayer::dumpHeader(std::ostream &out) const {
    out << "magic: ";
    std::copy(std::begin(this->header.magic), std::end(this->header.magic), std::ostream_iterator<char>(out));
    out << std::endl;
    out << "version: " << static_cast<int>(this->header.versionMajor) << ".";
    out << static_cast<int>(this->header.versionMinor) << std::endl;
    out << "num particles: " << this->header.numParticles << std::endl;
    out << "num snapshots: " << this->header.numSnapshots << std::endl;
    out << "cycle step: " << this->header.cycleStep << std::endl;
}
