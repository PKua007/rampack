//
// Created by pkua on 17.11.22.
//

#include "XYZRecorder.h"
#include "XYZWriter.h"


void XYZRecorder::recordSnapshot(const Packing &packing, std::size_t cycle) {
    Expects(this->out != nullptr);

    XYZWriter writer;
    writer.write(*this->out, packing, {{"cycles", std::to_string(cycle)}});
}
