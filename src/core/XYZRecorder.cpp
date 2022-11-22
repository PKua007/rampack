//
// Created by pkua on 17.11.22.
//

#include "XYZRecorder.h"
#include "geometry/Quaternion.h"


void XYZRecorder::recordSnapshot(const Packing &packing, std::size_t cycle) {
    Expects(this->out != nullptr);

    std::size_t numMolecules = packing.size();
    const auto &boxMatrix = packing.getBox().getDimensions();

    *this->out << numMolecules << std::endl;
    *this->out << "Lattice=\"";
    *this->out << boxMatrix(0, 0) << " " << boxMatrix(1, 0) << " " << boxMatrix(2, 0) << " ";
    *this->out << boxMatrix(0, 1) << " " << boxMatrix(1, 1) << " " << boxMatrix(2, 1) << " ";
    *this->out << boxMatrix(0, 2) << " " << boxMatrix(1, 2) << " " << boxMatrix(2, 2);
    *this->out << "\" Properties=species:S:1:pos:R:3:orientation:R:4 cycles=" << cycle << std::endl;

    for (const auto &shape : packing) {
        const auto &pos = shape.getPosition();
        const auto &rot = shape.getOrientation();
        auto quat = Quaternion::fromMatrix(rot);
        *this->out << "A ";
        *this->out << pos[0] << " " << pos[1] << " " << pos[2] << " ";
        *this->out << quat[0] << " " << quat[1] << " " << quat[2] << " " << quat[3] << std::endl;
    }
}
