//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_GOLDSTONETRACKER_H
#define RAMPACK_GOLDSTONETRACKER_H

#include "core/Observable.h"


/**
 * @brief Class tracking Goldstone mode translational and orientational drift of the system.
 * @details Deriving classes should implement pure virtual methods to configure the specific method of Goldstone mode
 * tracking. The observable has 6 interval values - 3 coordinates of the drifting origin and 3 Euler angles of the
 * drifting system orientation.
 */
class GoldstoneTracker : public Observable {
protected:
    /** @brief Current position of the origin */
    Vector<3> originPos;

    /** @brief Current orientation of the system */
    Matrix<3, 3> systemRot = Matrix<3, 3>::identity();

public:
    /**
     * @brief Returns the name of the tracking method.
     */
    [[nodiscard]] virtual std::string getTrackingMethodName() const = 0;

    /**
     * @brief Calculates the current origin of the system and current system orientation.
     * @details <p> It chooses a selected point in the system and tracks how it drifts with a Goldstone mode. For example,
     * if tracking method is finding the positions of density modulation maxima, the origin may be the position of one
     * of them (which drift during MC sampling). This method is called by inherited method Observable::calculate.
     *
     * <p> Derived classes should store calculated position and orientation in GoldstoneTracker::originPos and
     * GoldstoneTracker::systemRot protected fields.
     */
    virtual void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) = 0;

    /**
     * @brief Resets the state of the tracker. It should be called before attempting to track another system or to
     * redo the tracking.
     * @details It should erase all information about the state of the previous snapshot which may be used during
     * tracking. For example if we track density maxima and there are more than one, the origin should not jump between
     * them, but always find the one which is nearest to the one previously seen.
     */
    virtual void reset() = 0;

    void calculate(const Packing &packing, double temperature, double pressure, const ShapeTraits &shapeTraits) final;

    /**
     * @brief Returns interval header - the fields are "x", "y", "z" (coordinates of the origin) and "ox", "oy", "oz"
     * (Euler angles in radians of system orientation), prepended by @a GoldstoneTracker::getTrackingMethodName().
     */
    [[nodiscard]] std::vector<std::string> getIntervalHeader() const final;

    /**
     * @brief Returns empty nominal header.
     */
    [[nodiscard]] std::vector<std::string> getNominalHeader() const final { return {}; }

    [[nodiscard]] std::vector<double> getIntervalValues() const final;
    [[nodiscard]] std::vector<std::string> getNominalValues() const final { return {}; }
    [[nodiscard]] std::string getName() const final { return this->getTrackingMethodName() + "_tracker"; }

    /**
     * @brief Returns the current origin position.
     */
    [[nodiscard]] const Vector<3> &getOriginPos() const { return this->originPos; }

    /**
     * @brief Returns the current system orientation.
     */
    [[nodiscard]] const Matrix<3, 3> &getSystemRot() const { return this->systemRot; }
};




#endif //RAMPACK_GOLDSTONETRACKER_H
