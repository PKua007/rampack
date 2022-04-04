//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONPLAYER_H
#define RAMPACK_SIMULATIONPLAYER_H

#include <memory>

#include "SimulationIO.h"
#include "Packing.h"


class SimulationPlayer : public SimulationIO {
private:
    Header header;
    std::unique_ptr<std::istream> in;
    std::size_t currentSnapshot{};

public:
    explicit SimulationPlayer(std::unique_ptr<std::istream> in);

    [[nodiscard]] bool hasNext() const;
    void nextSnapshot(Packing &packing, const Interaction &interaction);
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const;
    void close();
};


#endif //RAMPACK_SIMULATIONPLAYER_H
