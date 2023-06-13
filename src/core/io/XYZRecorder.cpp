//
// Created by pkua on 17.11.22.
//

#include "XYZRecorder.h"
#include "XYZWriter.h"
#include "utils/GetlineBackwards.h"
#include "utils/Utils.h"


void XYZRecorder::recordSnapshot(const Packing &packing, std::size_t cycle) {
    Expects(this->out != nullptr);

    XYZWriter writer;
    writer.write(*this->out, packing, {{"cycles", std::to_string(cycle)}});

    this->lastCycleNumber = cycle;
}

XYZRecorder::XYZRecorder(std::unique_ptr<std::iostream> out, bool append)
        : out{std::move(out)}
{
    Expects(this->out != nullptr);

    this->out->seekp(0, std::ios::end);
    if (append) {
        if (this->out->tellp() != 0)
            this->findLastCycleNumber();
    } else {
        ValidateMsg(this->out->tellp() == 0, "XYZRecorder: append = false however stream is not empty");
    }
}

void XYZRecorder::findLastCycleNumber() {
    this->out->seekg(0, std::ios::end);

    std::string line;
    while (GetlineBackwards::getline(*this->out, line)) {
        if (!startsWith(line, "Lattice"))
            continue;

        std::size_t pos = line.find("cycles=");
        if (pos == std::string::npos)
            break;

        std::istringstream cyclesStream(line);
        cyclesStream.seekg(static_cast<std::streamoff>(pos) + 7);
        cyclesStream >> this->lastCycleNumber;
        if (!cyclesStream)
            break;

        return;
    }

    throw ValidationException("XYZRecorder: Cannot read last snapshot number - malformed XYZ stream");
}
