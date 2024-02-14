//
// Created by Piotr Kubala on 22/01/2024.
//

#ifndef RAMPACK_SHAPEDATA_H
#define RAMPACK_SHAPEDATA_H

#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <ostream>
#include <iomanip>

#include "utils/Exceptions.h"


namespace detail {
    template <typename T, typename F>
    constexpr auto has_equality_operator_impl(F &&f) -> decltype(f(std::declval<T>()), true) {
        return std::is_same_v<bool, decltype(std::declval<T>() == std::declval<T>())>;
    }

    template<typename>
    constexpr bool has_equality_operator_impl(...) { return false; }

    template <typename T>
    constexpr bool has_equality_operator() {
        return has_equality_operator_impl<T>([](auto&& obj) -> decltype(obj == obj) { });
    }

    template <typename T>
    constexpr bool has_equality_operator_v = has_equality_operator<T>();

    template <typename T, typename PtrT, typename DecayT = std::decay_t<T>>
    constexpr bool is_pointer_to_v = std::is_same_v<DecayT, PtrT*> || std::is_same_v<DecayT, const PtrT*>;
}


class ShapeData {
public:
    class Comparator {
    private:
        using Function = bool (*)(const std::byte *, const std::byte *);

        template <typename T>
        static bool theComparator(const std::byte *data1, const std::byte *data2) {
            if constexpr (std::is_same_v<T, void>)
                return true;
            else
                return ShapeData::as<T>(data1) == ShapeData::as<T>(data2);
        }

        Function function{};

        explicit Comparator(Function function) : function{function} { }

    public:
        Comparator() : function{Comparator::theComparator<void>} { }

        template<typename T>
        static Comparator forType() {
            using DecayT = std::remove_reference_t<T>;
            if constexpr (!std::is_same_v<DecayT, void>) {
                static_assert(std::is_trivially_copyable_v<DecayT>, "Type must be trivially copyable");
                static_assert(detail::has_equality_operator_v<DecayT>, "operator== for the type is required");
            }
            return Comparator(Comparator::theComparator<DecayT>);
        }

        [[nodiscard]] bool equal(const std::byte *data1, const std::byte *data2) const {
            return this->function(data1, data2);
        }

        friend bool operator==(const Comparator &c1, const Comparator &c2) {
            return c1.function == c2.function;
        }

        friend bool operator!=(const Comparator &c1, const Comparator &c2) {
            return !(c1 == c2);
        }
    };

private:
    bool managed{};
    const std::byte* data{};
    std::size_t size{};
    Comparator comparator{};

    void copyManagedData(const std::byte* data_, std::size_t size_, Comparator comparator_) {
        this->managed = true;
        this->size = size_;
        auto newData = new std::byte[this->size];
        std::copy(data_, data_ + size_, newData);
        this->data = newData;
        this->comparator = comparator_;
    }

public:
    template<typename T>
    static const T &as(const std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return *reinterpret_cast<const NoRefT*>(data);
    }

    template<typename T>
    static T &as(std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return *reinterpret_cast<NoRefT*>(data);
    }

    template<typename T>
    static const std::byte *raw(const T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return reinterpret_cast<const std::byte*>(&data);
    }

    template<typename T>
    static std::byte *raw(T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return reinterpret_cast<std::byte*>(&data);
    }


    ShapeData() = default;

    ShapeData(const ShapeData &other) : ShapeData(other.data, other.size, other.comparator, true) { }

    ShapeData(ShapeData &&other) noexcept {
        this->data = other.data;
        this->size = other.size;
        this->managed = other.managed;
        this->comparator = other.comparator;

        other.data = nullptr;
        other.size = 0;
        other.managed = false;
        other.comparator = Comparator{};
    }

    template<typename T,
             typename DecayedT = std::decay_t<T>,
             typename = std::enable_if_t<!std::is_same_v<DecayedT, ShapeData>
                                         && !detail::is_pointer_to_v<DecayedT, std::byte>>
            >
    explicit ShapeData(T &&data, bool managed = true) {
        static_assert(std::is_trivially_copyable_v<DecayedT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<DecayedT>, "operator== for the type is required");

        if (managed) {
            this->copyManagedData(reinterpret_cast<const std::byte*>(&data), sizeof(T), Comparator::forType<T>());
        } else {
            ExpectsMsg(std::is_lvalue_reference_v<T>, "ShapeData::ShapeData(): unmanaged rvalue reference");

            this->data = reinterpret_cast<const std::byte*>(&data);
            this->size = sizeof(T);
            this->managed = false;
            this->comparator = Comparator::forType<T>();
        }
    }

    ShapeData(const std::byte *data, std::size_t size, Comparator comparator, bool managed = false) {
        if (size == 0) {
            this->managed = false;
            this->size = 0;
            this->data = nullptr;
            this->comparator = Comparator{};
        }

        if (managed) {
            this->copyManagedData(data, size, comparator);
        } else {
            this->managed = false;
            this->size = size;
            this->data = data;
            this->comparator = comparator;
        }
    }

    ~ShapeData() {
        if (this->managed)
            delete[] data;
    }

    ShapeData &operator=(const ShapeData &other) {
        if (this == &other)
            return *this;

        if (this->managed)
            delete[] this->data;

        if (other.size == 0) {
            this->managed = false;
            this->size = 0;
            this->data = nullptr;
            this->comparator = Comparator{};
        } else {
            this->copyManagedData(other.data, other.size, other.comparator);
        }

        return *this;
    }

    ShapeData &operator=(ShapeData &&other) noexcept {
        if (this == &other)
            return *this;

        if (this->managed)
            delete[] this->data;

        this->data = other.data;
        this->size = other.size;
        this->managed = other.managed;
        this->comparator = other.comparator;

        other.data = nullptr;
        other.size = 0;
        other.managed = false;
        other.comparator = Comparator{};

        return *this;
    }

    template<typename T,
             typename DecayedT = std::decay_t<T>,
             typename = std::enable_if_t<!std::is_same_v<ShapeData, DecayedT>>>
    ShapeData &operator=(T &&data_) {
        static_assert(std::is_trivially_copyable_v<DecayedT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<DecayedT>, "operator== for the type is required");

        *this = ShapeData(data_);
        return *this;
    }

    template<typename T>
    const T &as() const {
        ExpectsMsg(sizeof(T) <= this->size, "ShapeData::as(): extracting too large type");

        return ShapeData::as<T>(this->data);
    }

    template<typename T>
    [[nodiscard]] bool stores() const {
        return this->comparator == Comparator::forType<T>();
    }

    [[nodiscard]] bool isManaged() const { return this->managed; }
    [[nodiscard]] std::size_t getSize() const { return this->size; }
    [[nodiscard]] bool isEmpty() const { return this->size == 0; }
    [[nodiscard]] const std::byte *raw() const { return this->data; }
    [[nodiscard]] const Comparator &getComparator() const { return this->comparator; }

    [[nodiscard]] ShapeData unmanagedCopy() const {
        return ShapeData(this->data, this->size, this->comparator, false);
    }

    [[nodiscard]] ShapeData managedCopy() const {
        return ShapeData(this->data, this->size, this->comparator, true);
    }

    [[nodiscard]] ShapeData relayedManagementCopy() const {
        if (this->managed)
            return this->managedCopy();
        else
            return this->unmanagedCopy();
    }

    friend bool operator==(const ShapeData &lhs, const ShapeData &rhs) {
        return (lhs.comparator == rhs.comparator) && lhs.comparator.equal(lhs.data, rhs.data);
    }

    friend bool operator!=(const ShapeData &lhs, const ShapeData &rhs) {
        return !(lhs == rhs);
    }

    friend std::ostream &operator<<(std::ostream &out, const ShapeData &shapeData) {
        if (shapeData.isEmpty())
            return out << "empty";

        auto savedFlags = out.flags();
        out << std::hex << std::setfill('0');
        for (std::size_t i{}; i < shapeData.size; i++)
            out << std::setw(2) << static_cast<unsigned>(shapeData.data[i]) << " ";
        out << std::dec;
        out << "(" << shapeData.size << " bytes)";
        out.setf(savedFlags);

        return out;
    }
};


#endif //RAMPACK_SHAPEDATA_H
