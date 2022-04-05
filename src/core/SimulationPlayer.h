//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONPLAYER_H
#define RAMPACK_SIMULATIONPLAYER_H

#include <memory>

#include "SimulationIO.h"
#include "Packing.h"
#include "utils/Logger.h"


/**
 * @brief Class which enables replaying particle trajectories stored in binary.
 */
class SimulationPlayer : public SimulationIO {
private:
    Header header;
    std::unique_ptr<std::istream> in;
    std::size_t currentSnapshot{};

public:
    /**
     * @brief Constructs the player from a given @a std::istream (it takes full responsibility of it).
     * @details After the construction the internal "pointer" point before the first snapshot. The stream has to be
     * binary and has operable @a tellg and @a seekg methods.
     */
    explicit SimulationPlayer(std::unique_ptr<std::istream> in);

    /**
     * @brief Returns true if there is next snapshot to move to.
     */
    [[nodiscard]] bool hasNext() const;

    /**
     * @brief Moves to the next snapshot (the first one on an original invocation) and prints it on @a packing.\
     */
    void nextSnapshot(Packing &packing, const Interaction &interaction);

    /**
     * @brief Returns number of cycles for a current snapshot (which was recently moved to using
     * SimyulationPlayer::nextSnapshot()).
     */
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const;

    /**
     * @brief Closes the stream on demand and prevents any further operations.
     */
    void close();

    /**
     * @brief Prints short info about the header (number of particles, etc). into @a out.
     */
    void dumpHeader(Logger &out) const;
};


#endif //RAMPACK_SIMULATIONPLAYER_H
