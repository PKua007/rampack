//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHERARRAY_H
#define RAMPACK_MATCHERARRAY_H

#include <vector>
#include <array>
#include <functional>
#include <memory>

#include "MatcherBase.h"
#include "NodeArray.h"
#include "geometry/Matrix.h"
#include "geometry/Vector.h"


namespace pyon::matcher {
    class ArrayData {
    private:
        std::vector<Any> data;

    public:
        using iterator = decltype(data)::const_iterator;

        ArrayData() = default;
        explicit ArrayData(std::vector<Any> data);

        [[nodiscard]] bool empty() const { return this->data.empty(); }
        [[nodiscard]] std::size_t size() const { return this->data.size(); }
        [[nodiscard]] const Any &front() const;
        [[nodiscard]] const Any &back() const;
        [[nodiscard]] iterator begin() const { return this->data.begin(); }
        [[nodiscard]] iterator end() const { return this->data.end(); }
        [[nodiscard]] const Any &at(std::size_t idx) const;
        [[nodiscard]] const Any &operator[](std::size_t idx) const { return this->at(idx); };

        template<typename T, std::size_t SIZE>
        [[nodiscard]] std::array<T, SIZE> asStdArray() const {
            if (this->size() != SIZE)
                throw MatchException("pyon::matcher::ArrayData::asStdArray: wrong size of matched array; "
                                     "expected: " + std::to_string(SIZE) + ", got: " + std::to_string(this->size()));
            std::array<T, SIZE> result;
            auto transformer = [](const Any &elem) { return elem.as<T>(); };
            std::transform(this->begin(), this->end(), result.begin(), transformer);
            return result;
        }

        template<typename T>
        [[nodiscard]] std::vector<T> asStdVector() const {
            std::vector<T> result;
            result.reserve(this->size());
            auto transformer = [](const Any &elem) { return elem.as<T>(); };
            std::transform(this->begin(), this->end(), std::back_inserter(result), transformer);
            return result;
        }

        template<std::size_t DIM, typename T = double>
        [[nodiscard]] Vector<DIM, T> asVector() const {
            if (this->size() != DIM) {
                throw MatchException("pyon::matcher::ArrayData::asVector: wrong size of matched array; "
                                     "expected: " + std::to_string(DIM) + ", got: " + std::to_string(this->size()));
            }
            Vector<DIM, T> result;
            auto transformer = [](const Any &elem) { return elem.as<T>(); };
            std::transform(this->begin(), this->end(), result.begin(), transformer);
            return result;
        }

        template<std::size_t ROWS, std::size_t COLS, typename T = double>
        [[nodiscard]] Matrix<ROWS, COLS, T> asMatrix() const {
            Matrix<ROWS, COLS, T> result;
            auto rows = this->asStdArray<ArrayData, ROWS>();
            for (std::size_t rowI{}; rowI < ROWS; rowI++) {
                auto row = rows[rowI].template asStdArray<T, COLS>();
                for (std::size_t colI{}; colI < COLS; colI++)
                    result(rowI, colI) = row[colI];
            }
            return result;
        }
    };

    class MatcherArray : public MatcherBase {
    private:
        std::vector<std::function<bool(const ArrayData&)>> filters;
        std::function<Any(const ArrayData&)> mapping = [](const ArrayData &array) { return array; };
        std::shared_ptr<MatcherBase> elementMatcher;

    public:
        MatcherArray() = default;
        explicit MatcherArray(std::size_t size_);
        explicit MatcherArray(int size_);

        template <typename ConcreteMatcher>
        explicit MatcherArray(const ConcreteMatcher &matcher) {
            this->elementsMatch(matcher);
        }

        template <typename ConcreteMatcher>
        explicit MatcherArray(const ConcreteMatcher &matcher, std::size_t size_) {
            this->elementsMatch(matcher);
            this->size(size_);
        }

        bool match(std::shared_ptr<const ast::Node> node, Any &result) const override;

        template<typename T, std::size_t SIZE>
        MatcherArray &mapToStdArray() {
            this->mapping = [](const ArrayData &array) { return array.asStdArray<T, SIZE>(); };
            return *this;
        }

        template<typename T>
        MatcherArray &mapToStdVector() {
            this->mapping = [](const ArrayData &array) { return array.asStdVector<T>(); };
            return *this;
        }

        template<std::size_t DIM, typename T = double>
        MatcherArray &mapToVector() {
            this->mapping = [](const ArrayData &array) { return array.asVector<DIM, T>(); };
            return *this;
        }

        template<std::size_t ROWS, std::size_t COLS, typename T = double>
        MatcherArray &mapToMatrix() {
            this->mapping = [](const ArrayData &array) { return array.asMatrix<ROWS, COLS, T>(); };
            return *this;
        }

        template <typename ConcreteMatcher>
        MatcherArray &elementsMatch(const ConcreteMatcher &matcher) {
            static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                          "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");
            // Copy construct the matcher
            this->elementMatcher = std::make_shared<ConcreteMatcher>(matcher);
            return *this;
        }

        MatcherArray &mapTo(const std::function<Any(const ArrayData&)> &mapping_);
        MatcherArray &filter(const std::function<bool(const ArrayData&)> &filter);
        MatcherArray &size(std::size_t size_);
        MatcherArray &sizeAtLeast(std::size_t size_);
        MatcherArray &sizeAtMost(std::size_t size_);
        MatcherArray &sizeInRange(std::size_t low, std::size_t high);
        MatcherArray &empty();
        MatcherArray &nonEmpty();
    };
} // matcher

#endif //RAMPACK_MATCHERARRAY_H
