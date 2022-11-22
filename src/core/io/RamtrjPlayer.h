//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_RAMTRJPLAYER_H
#define RAMPACK_RAMTRJPLAYER_H

#include <memory>

#include "RamtrjIO.h"
#include "core/Packing.h"
#include "core/SimulationPlayer.h"
#include "utils/Logger.h"


/**
 * @brief Class which enables replaying particle trajectories stored in RAMTRJ binary format.
 */
class RamtrjPlayer : RamtrjIO, public SimulationPlayer {
private:
    Header header;
    std::unique_ptr<std::istream> in;
    std::size_t currentSnapshot{};

public:
    /**
     * @brief Helper class used to perform trajectory fixing and report the results.
     */
    class AutoFix {
    private:
        std::size_t expectedNumMolecules{};

        bool fixingNeeded{};
        bool fixingSuccessful = true;
        std::string errorMessage;
        std::size_t headerSnapshots{};
        std::size_t inferredSnapshots{};
        std::size_t bytesRemainder{};
        std::size_t bytesPerSnapshot{};

        void reportNofix(const RamtrjIO::Header &header_);
        void reportError(const std::string &message);
        void tryFixing(Header &header_, std::size_t snapshotBytes);

        friend class RamtrjPlayer;

    public:
        /**
         * @brief Constructs the class expecting @a expectedNumMolecules molecules in the fixed trajectory.
         */
        explicit AutoFix(std::size_t expectedNumMolecules);

        /**
         * @brief Returns @a true, if fixing was successful (or was not needed)
         */
        [[nodiscard]] bool wasFixingSuccessful() const { return this->fixingSuccessful; }

        /**
         * @brief Return @a true, if fixing was needed and performed correctly
         */
        [[nodiscard]] bool wasFixingNeeded() const { return this->fixingNeeded; }

        /**
         * @brief Returns a number of full snapshots found in the trajectory.
         */
        [[nodiscard]] std::size_t getInferredSnapshots() const { return this->inferredSnapshots; }

        /**
         * @brief Returns a number of bytes in the last, truncated snapshot.
         */
        [[nodiscard]] std::size_t getBytesRemainder() const { return this->bytesRemainder; }

        /**
         * @brief Prints info about auto-fixing operation.
         */
        void dumpInfo(Logger &out) const;
    };

    /**
     * @brief Constructs the player from a given @a std::istream (it takes full responsibility of it).
     * @details After the construction the internal "pointer" point before the first snapshot. The stream has to be
     * binary and has operable @a tellg and @a seekg methods.
     */
    explicit RamtrjPlayer(std::unique_ptr<std::istream> in);

    /**
     * @brief The same as the other constructor, but it will attempt to fix the trajectory if it was truncated and
     * report the results via autoFix. If fixing fails, exception is thrown afterwards.
     */
    explicit RamtrjPlayer(std::unique_ptr<std::istream> in, AutoFix &autoFix);

    [[nodiscard]] bool hasNext() const override;
    void nextSnapshot(Packing &packing, const Interaction &interaction) override;
    void reset() override;
    void lastSnapshot(Packing &packing, const Interaction &interaction) override;
    void jumpToSnapshot(Packing &packing, const Interaction &interaction, std::size_t cycleNumber) override;
    [[nodiscard]] std::size_t getCurrentSnapshotCycles() const override;
    [[nodiscard]] std::size_t getTotalCycles() const override {
        return this->header.cycleStep * this->header.numSnapshots;
    }
    [[nodiscard]] std::size_t getCycleStep() const override { return this->header.cycleStep; }
    [[nodiscard]] std::size_t getNumMolecules() const override { return this->header.numParticles; }
    void close() override;

    /**
     * @brief Prints short info about the header (number of particles, etc). into @a out.
     */
    void dumpHeader(Logger &out) const;
};


#endif //RAMPACK_RAMTRJPLAYER_H
