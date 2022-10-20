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
    class AutoFix {
    private:
        std::size_t expectedNumMolecules{};

        bool fixed{};
        bool fixingSuccessful = true;
        std::string errorMessage;
        std::size_t headerSnapshots{};
        std::size_t inferredSnapshots{};
        std::size_t bytesRemainder{};
        std::size_t bytesPerSnapshot{};

        void reportNofix(const SimulationIO::Header &header_);
        void reportError(const std::string &message);
        void tryFixing(Header &header_, std::size_t snapshotBytes);

        friend class SimulationPlayer;

    public:
        explicit AutoFix(std::size_t expectedNumMolecules);

        [[nodiscard]] bool wasFixingSuccessful() const { return this->fixingSuccessful; }
        [[nodiscard]] bool wasFixed() const { return this->fixed; }
        [[nodiscard]] std::size_t getInferredSnapshots() const { return this->inferredSnapshots; }
        [[nodiscard]] std::size_t getBytesRemainder() const { return this->bytesRemainder; }

        void dumpInfo(Logger &out) const;
    };

    /**
     * @brief Constructs the player from a given @a std::istream (it takes full responsibility of it).
     * @details After the construction the internal "pointer" point before the first snapshot. The stream has to be
     * binary and has operable @a tellg and @a seekg methods.
     */
    explicit SimulationPlayer(std::unique_ptr<std::istream> in);

    /**
     * @brief The same as the other constructor, but it will attempt to fix the trajectory if it was truncated and
     * report the results via autoFix. If fixing fails, exception is thrown afterwards.
     */
    explicit SimulationPlayer(std::unique_ptr<std::istream> in, AutoFix &autoFix);

    /**
     * @brief Returns true if there is next snapshot to move to.
     */
    [[nodiscard]] bool hasNext() const;

    /**
     * @brief Moves to the next snapshot (the first one is an original invocation) and prints it on @a packing.
     */
    void nextSnapshot(Packing &packing, const Interaction &interaction);

    /**
     * @brief Moves back to the beginning of the trajectory. Calling nextSnapshot() afterwards will then jump to the
     * first one.
     */
    void reset();

    /**
     * @brief Moves to the last snapshot and prints it on @a packing.
     */
    void lastSnapshot(Packing &packing, const Interaction &interaction);

    /**
     * @brief Jumps to a given snapshot and prints it on @a packing.
     */
    void jumpToSnapshot(Packing &packing, const Interaction &interaction, std::size_t cycleNumber);

    /**
     * @brief Returns number of cycles for a current snapshot (which was recently moved to using
     * SimyulationPlayer::nextSnapshot()).
     */
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const;

    /**
     * @brief Returns total number of recorded cycles.
     */
    [[nodiscard]] std::size_t getTotalCycles() const { return this->header.cycleStep * this->header.numSnapshots; }

    /**
     * @brief Returns cycle step between snapshots.
     */
    [[nodiscard]] std::size_t getCycleStep() const { return this->header.cycleStep; }

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
