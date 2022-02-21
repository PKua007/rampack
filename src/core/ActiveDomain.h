//
// Created by Piotr Kubala on 18/03/2021.
//

#ifndef RAMPACK_ACTIVEDOMAIN_H
#define RAMPACK_ACTIVEDOMAIN_H

#include <array>
#include <tuple>
#include <utility>

#include "geometry/Vector.h"
#include "TriclinicBox.h"

/**
 * @brief A class representing a single active domain in domain division scheme
 */
class ActiveDomain {
public:
    /**
     * @brief A helper structure describing a single interval - domain bounds in one dimension
     */
    struct RegionBounds {
        double beg{};
        double end{};

        friend bool operator==(const RegionBounds &lhs, const RegionBounds &rhs) {
            return std::tie(lhs.beg, lhs.end) == std::tie(rhs.beg, rhs.end);
        }

        friend bool operator!=(const RegionBounds &lhs, const RegionBounds &rhs) {
            return !(rhs == lhs);
        }
    };
    
private:
    std::array<RegionBounds, 3> bounds{};
    TriclinicBox box;

public:
    /**
     * @brief Default constructor creating an empty region with all bounds being 0
     */
    //ActiveDomain() = default;

    explicit ActiveDomain(TriclinicBox box, const std::array<RegionBounds, 3> &bounds)
            : bounds{bounds}, box{std::move(box)}
    { }

    /**
     * @brief Returns @a true if @a position lies inside the bounds, @a false otherwise
     */
    [[nodiscard]] bool isInside(const Vector<3> &position) const;

    /**
     * @brief Returns RegionBounds for a single coordinate given by index @a coordI
     */
    [[nodiscard]] const RegionBounds &getBoundsForCoordinate(std::size_t coordI) const;
};


#endif //RAMPACK_ACTIVEDOMAIN_H
