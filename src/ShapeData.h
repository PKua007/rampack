//
// Created by Piotr Kubala on 22/01/2024.
//

#ifndef RAMPACK_SHAPEDATA_H
#define RAMPACK_SHAPEDATA_H

#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <utility>

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
    ShapeData() = default;

    ShapeData(const ShapeData &other) {
        if (other.size == 0) {
            this->managed = false;
            this->size = 0;
            this->data = nullptr;
        } else {
            this->copyManagedData(other.data, other.size);
        }
    }

    ShapeData(ShapeData &&other) noexcept {
        this->data = other.data;
        this->size = other.size;
        this->managed = other.managed;

        other.data = nullptr;
        other.size = 0;
        other.managed = false;
    }

    template<typename T, typename = std::enable_if_t<!std::is_same_v<ShapeData, std::decay_t<T>>>>
    explicit ShapeData(T &&data) {
        static_assert(std::is_pod_v<T>, "Only POD types are allowed");

        this->managed = true;
        this->size = sizeof(T);
        this->data = new std::byte[this->size];
        this->as<T>() = std::forward<T>(data);
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

    template<typename T, typename = std::enable_if_t<!std::is_same_v<ShapeData, std::decay_t<T>>>>
    ShapeData &operator=(T &&data_) {
        static_assert(std::is_pod_v<T>, "Only POD types are allowed");
        *this = ShapeData(data_);
        return *this;
    }

    template <typename T>
    const T &as() const {
        static_assert(std::is_pod_v<T>, "Only POD types are allowed");
        Expects(sizeof(T) == this->size);
        return *reinterpret_cast<const T*>(this->data);
    }

    [[nodiscard]] bool isManaged() const { return this->managed; }
    [[nodiscard]] std::size_t getSize() const { return this->size; }
    [[nodiscard]] const std::byte *getRawData() const { return this->data; }

    friend bool operator==(const ShapeData &lhs, const ShapeData &rhs) {
        return std::equal(lhs.data, lhs.data + lhs.size, rhs.data, rhs.data + rhs.size);
    }
};


#endif //RAMPACK_SHAPEDATA_H
