//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_OBSERVABLESCOLLECTOR_H
#define RAMPACK_OBSERVABLESCOLLECTOR_H

#include <memory>
#include <vector>
#include <iosfwd>
#include <functional>

#include "ShapeTraits.h"
#include "Observable.h"
#include "BulkObservable.h"
#include "utils/Quantity.h"


/**
 * @brief The class responsible for collection and storing of all Observable -s and BulkObservable -s.
 * @details The class is responsible for three possible observable scopes: snapshots, inline printing on the standard
 * output and ensemble averaged values (see ObservableType). A single Observable can be used in an arbitrary combination
 * of all those scopes. BulkObservable -s are always calculated only in the averaging phase (where the system is already
 * thermalized).
 */
class ObservablesCollector {
private:
    double temperature{};
    double pressure{};

    std::vector<std::shared_ptr<Observable>> observables;
    std::vector<std::shared_ptr<BulkObservable>> bulkObservables;
    std::vector<std::string> snapshotHeader;
    std::vector<std::string> averagingHeader;
    std::vector<std::size_t> inlineObservablesIndices;
    std::vector<std::size_t> snapshotObservablesIndices;
    std::vector<std::size_t> averagingObservablesIndices;
    std::vector<std::size_t> snapshotCycleNumbers;
    std::vector<std::vector<std::string>> snapshotValues;
    std::vector<std::vector<double>> averagingValues;
    std::size_t onTheFlyLastCycleNumber{};
    std::unique_ptr<std::iostream> onTheFlyOut;

    mutable double computationMicroseconds{};

    void printInlineObservable(unsigned long observableIdx, const Packing &packing, const ShapeTraits &shapeTraits,
                               std::ostringstream &out) const;
    void doPrintSnapshotHeader(std::ostream &out, bool printNewline = true) const;
    void doPrintSnapshotValues(std::ostream &out, std::size_t snapshotIdx) const;
    void verifyOnTheFlyOutputHeader();
    void findOnTheFlyLastCycleNumber();

public:
    /**
     * @brief The pair of single observable value name and its value with error.
     */
    struct ObservableData {
        std::string name;
        Quantity quantity;
    };

    /**
     * @brief The group of many ObservableData observable values together with the name of the group.
     * @details Typically, ObservableData entries correspond to a single Observable interval values and @a groupName
     * is Observable::getName main name.
     */
    struct ObservableGroupData {
        std::string groupName;
        std::vector<ObservableData> observableData;
    };

    /**
     * @brief Scopes of the observables, which can be bitwise-or combined
     */
    enum ObservableType : std::size_t {
        SNAPSHOT = 1,
        AVERAGING = 2,
        SNAPSHOT_AVERAGING = 3,
        INLINE = 4,
        SNAPSHOT_INLINE = 5,
        AVERAGING_INLINE = 6,
        SNAPSHOT_AVERAGING_INLINE = 7
    };

    /**
     * @brief Adds a new observable that will be collected. A new observable cannot be added if snapshots has already
     * been captured.
     * @param observable a new observable to be added
     * @param observableType the type of the observable as specified by ObservableType
     */
    void addObservable(std::shared_ptr<Observable> observable, std::size_t observableType);

    /**
     * @brief Adds BulkObservable to be computed in the averaging phase.
     */
    void addBulkObservable(std::shared_ptr<BulkObservable> observable);

    /**
     * @brief Sets the thermodynamic parameters: @a temperature_ and @a pressure_ to be used when calculating observable
     * values.
     */
    void setThermodynamicParameters(double temperature_, double pressure_);

    /**
     * @brief Registers a snapshot interval and nominal values of ObservableType::SNAPSHOT observables.
     * @param packing packing to calculate observables on
     * @param cycleNumber the number of the cycle after which the snapshot is being taken
     * @param shapeTraits shape traits describing the shape used in the packing
     */
    void addSnapshot(const Packing &packing, std::size_t cycleNumber, const ShapeTraits &shapeTraits);

    /**
     * @brief Add next interval values of ObservableType::AVERAGING Observable -s and all BulkObservable -s to the
     * ensemble averages of them.
     * @param packing packing to calculate observables on
     * @param shapeTraits shape traits describing the shape used in the packing
     */
    void addAveragingValues(const Packing &packing, const ShapeTraits &shapeTraits);

    /**
     * @brief Starts saving observable snapshots "on the fly" to the output stream @a out.
     * @details If something was attached previously, its destructor is called. If @a nullptr is passed, it is analogous
     * to calling detachOnTheFlyOutput. If the output is not empty, the method verifies if has a correct header to
     * ensure consistency of outputted observables.
     */
    void attachOnTheFlyOutput(std::unique_ptr<std::iostream> out);

    /**
     * @brief Returns last cycle number in the on-the-fly observable output.
     * @details If no output is attached, it returns 0.
     */
    [[nodiscard]] std::size_t getOnTheFlyOutputLastCycleNumber() const { return this->onTheFlyLastCycleNumber; }

    /**
     * @brief Detaches and returns "on the fly" output stream (or @a nullptr if nothing is attached).
     */
    std::unique_ptr<std::iostream> detachOnTheFlyOutput();

    /**
     * @brief Generate a short inline string with current interval and nominal values of all ObservableType::INLINE
     * observables.
     * @details The format is "[name 1] = [value 1], [name 2] = [value 2], ...".
     * @param packing packing to calculate observables on
     * @param shapeTraits shape traits describing the shape used in the packing
     */
    [[nodiscard]] std::string generateInlineObservablesString(const Packing &packing,
                                                              const ShapeTraits &shapeTraits) const;

    /**
     * @brief Clears all collected observable values (but does not delete added Observable nor BulkObservable objects),
     * It also clears BulkObservable -s (BulkObservable::clear()) and computation times.
     */
    void clear();

    /**
     * @brief Outputs ObservableType::SNAPSHOT observable values to @a out output stream.
     * @details It is stored in an SSV (space separated value) format. Each row consists of a cycle number and
     * observable values. The values are "flattened" - for example if we have two observables, one with three and the
     * second one with four nominal/interval values, they form a row with seven values, without any grouping. If
     * @a printHeader is @a true, the first row is the header created using names from Observable::getIntervalHeader and
     * Observable::getNominalHeader methods.
     * @param out
     * @param printHeader
     */
    void printSnapshots(std::ostream &out, bool printHeader = true) const;

    /**
     * @brief Calculate the average value of interval values of ObservableType::AVERAGING observables and return
     * as a flat vector of observable (without being grouped according to Observable objects).
     */
    [[nodiscard]] std::vector<ObservableData> getFlattenedAverageValues() const;

    /**
     * @brief Calculate the average value of interval values of ObservableType::AVERAGING observables and return
     * them grouped according to Observable objects.
     */
    [[nodiscard]] std::vector<ObservableGroupData> getGroupedAverageValues() const;

    /**
     * @brief The method iterates over all BulkObservables and calls @a visitor function passing them as the argument.
     */
    void visitBulkObservables(std::function<void(const BulkObservable &)> visitor) const;

    /**
     * @brief Returns the total time used to calculate observable values in microseconds.
     * @details ObservablesCollector::clear resets the timer.
     */
    [[nodiscard]] double getComputationMicroseconds() const { return this->computationMicroseconds; }

    /**
     * @brief Returns the estimated number of bytes used by observable data.
     */
    [[nodiscard]] std::size_t getMemoryUsage() const;
};


#endif //RAMPACK_OBSERVABLESCOLLECTOR_H
