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


class ShapeData {
private:
    const std::byte* data{};
    std::size_t size{};
    bool managed{};

    void copyManagedData(const std::byte* data_, std::size_t size_) {
        this->managed = true;
        this->size = size_;
        auto newData = new std::byte[this->size];
        std::copy(data_, data_ + size_, newData);
        this->data = newData;
    }

public:
    template<typename T>
    static const T &as(const std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");

        return *reinterpret_cast<const NoRefT*>(data);
    }

    template<typename T>
    static T &as(std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");

        return *reinterpret_cast<NoRefT*>(data);
    }

    template<typename T>
    static const std::byte *raw(const T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");

        return reinterpret_cast<const std::byte*>(&data);
    }

    template<typename T>
    static std::byte *raw(T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");

        return reinterpret_cast<std::byte*>(&data);
    }


    ShapeData() = default;

    ShapeData(const ShapeData &other) : ShapeData(other.data, other.size, true) { }

    ShapeData(ShapeData &&other) noexcept {
        this->data = other.data;
        this->size = other.size;
        this->managed = other.managed;

        other.data = nullptr;
        other.size = 0;
        other.managed = false;
    }

    template<typename T,
             typename DecayedT = std::decay_t<T>,
             typename = std::enable_if_t<!std::is_same_v<ShapeData, DecayedT>>>
    explicit ShapeData(T &&data, bool managed = true) {
        static_assert(std::is_trivially_copyable_v<DecayedT>, "Type must be trivially copyable");

        if (managed) {
            this->copyManagedData(reinterpret_cast<const std::byte*>(&data), sizeof(T));
        } else {
            ExpectsMsg(std::is_lvalue_reference_v<T>, "ShapeData::ShapeData(): unmanaged rvalue reference");

            this->data = reinterpret_cast<const std::byte*>(&data);
            this->size = sizeof(T);
            this->managed = false;
        }
    }

    ShapeData(const std::byte *data, std::size_t size, bool managed = false) {
        if (size == 0) {
            this->managed = false;
            this->size = 0;
            this->data = nullptr;
        }

        if (managed) {
            this->copyManagedData(data, size);
        } else {
            this->managed = false;
            this->size = size;
            this->data = data;
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
        } else {
            this->copyManagedData(other.data, other.size);
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

        other.data = nullptr;
        other.size = 0;
        other.managed = false;

        return *this;
    }

    template<typename T,
             typename DecayedT = std::decay_t<T>,
             typename = std::enable_if_t<!std::is_same_v<ShapeData, DecayedT>>>
    ShapeData &operator=(T &&data_) {
        static_assert(std::is_trivially_copyable_v<DecayedT>, "Type must be trivially copyable");

        *this = ShapeData(data_);
        return *this;
    }

    template<typename T>
    const T &as() const {
        ExpectsMsg(sizeof(T) <= this->size, "ShapeData::as(): extracting too large type");

        return ShapeData::as<T>(this->data);
    }

    [[nodiscard]] bool isManaged() const { return this->managed; }
    [[nodiscard]] std::size_t getSize() const { return this->size; }
    [[nodiscard]] bool isEmpty() const { return this->size == 0; }
    [[nodiscard]] const std::byte *raw() const { return this->data; }

    [[nodiscard]] ShapeData unmanagedCopy() const {
        return ShapeData(this->data, this->size, false);
    }

    [[nodiscard]] ShapeData managedCopy() const {
        return ShapeData(this->data, this->size, true);
    }

    [[nodiscard]] ShapeData relayedManagementCopy() const {
        if (this->managed)
            return this->managedCopy();
        else
            return this->unmanagedCopy();
    }

    friend bool operator==(const ShapeData &lhs, const ShapeData &rhs) {
        return std::equal(lhs.data, lhs.data + lhs.size, rhs.data, rhs.data + rhs.size);
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
