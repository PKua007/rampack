//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_HISTOGRAMBUILDER_H
#define RAMPACK_HISTOGRAMBUILDER_H

#include <ostream>
#include <vector>


class HistogramBuilder {
private:
    struct BinData {
        double value{};
        std::size_t numPoints{};

        void addPoint(double newValue);

        BinData &operator+=(const BinData &other);
        BinData &operator*=(double factor);
    };

    struct Histogram {
        std::vector<BinData> bins;

        explicit Histogram(std::size_t numBins) : bins(numBins) { }

        Histogram &operator+=(const Histogram &otherData);
        void renormalizeBins(const std::vector<double> &factors);
        [[nodiscard]] std::size_t size() const { return bins.size(); }
        void clear();
    };

    double min{};
    double max{};
    double step{};
    std::size_t numSnapshots{};
    Histogram histogram;
    Histogram currentHistogram;
    std::vector<double> binValues{};

public:
    struct BinValue {
        double binMiddle{};
        double value{};

        friend bool operator==(const BinValue &lhs, const BinValue &rhs) {
            return std::tie(lhs.binMiddle, lhs.value) == std::tie(rhs.binMiddle, rhs.value);
        }

        friend bool operator!=(const BinValue &lhs, const BinValue &rhs) {
            return !(rhs == lhs);
        }

        friend std::ostream &operator<<(std::ostream &out, const BinValue &bin) {
            return out << bin.binMiddle << " " << bin.value;
        }
    };

    enum class ReductionMethod {
        SUM,
        AVERAGE
    };

    explicit HistogramBuilder(double min, double max, std::size_t numBins);

    void add(double pos, double value);
    void nextSnapshot();
    [[nodiscard]] std::vector<BinValue> dumpValues(ReductionMethod reductionMethod) const;
    void clear();
    [[nodiscard]] std::size_t size() const { return this->histogram.size(); }
    [[nodiscard]] double getBinSize() const { return this->step; }
    [[nodiscard]] double getMin() const { return this->min; }
    [[nodiscard]] double getMax() const { return this->max; }
    [[nodiscard]] std::vector<double> getBinDividers() const;
    void renormalizeBins(const std::vector<double> &factors) { this->currentHistogram.renormalizeBins(factors); }
};


#endif //RAMPACK_HISTOGRAMBUILDER_H
