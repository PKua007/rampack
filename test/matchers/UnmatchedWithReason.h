//
// Created by Piotr Kubala on 25/12/2022.
//

#ifndef RAMPACK_UNMATCHEDWITHREASON_H
#define RAMPACK_UNMATCHEDWITHREASON_H

#include <catch2/catch.hpp>

#include "pyon/MatcherBase.h"


class UnmatchedWithReason : public Catch::MatcherBase<pyon::matcher::MatchReport> {
private:
    std::string reason;

public:
    explicit UnmatchedWithReason(std::string reason) : reason{std::move(reason)} { }

    bool match(const pyon::matcher::MatchReport &arg) const override {
        return !arg && arg.getReason() == this->reason;
    }

protected:
    std::string describe() const override {
        return "\nis:\nnot matched with reason:\n" + this->reason;
    }
};


template<>
struct Catch::StringMaker<pyon::matcher::MatchReport> {
    static std::string convert(const pyon::matcher::MatchReport &report) {
        if (report.isMatched())
            return "matched";
        else
            return "not matched with reason:\n" + report.getReason();
    }
};

#endif //RAMPACK_UNMATCHEDWITHREASON_H
