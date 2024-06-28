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


/**
 * @brief Class for storing type-erased shape data.
 * @details <p> The class consists of two elements: a pointer to a raw @a std::byte array with shape data, as well as
 * the instance of the ShapeData::Comparator object used to encode the information about the type and to check for data
 * equality.
 * <p> The data may be either @a manager or @a unmanaged. In the first case, the ShapeData instance owns data's memory
 * and is responsible for freeing it. In the second case, the class does not own the memory and only stores the pointer
 * of the memory managed by a different agent. <b>In this case, the external storage should outlive the ShapeData
 * instance and all its unmanaged copies</b>. Empty ShapeData (storing the @a void type data) is regarded as
 * @a unmanaged.
 * <p> Data storage mode is established based ont the arguments of the constructors (please refer to them for more
 * details). To minimize the chances of a dangling pointer in the @a unmanaged mode after external data memory is freed,
 * the copy constructor always creates the managed instance. The move constructor relays the data management mode. One
 * can also manually chose how the copy is managed by acquiring it via ShapeData::managedCopy, ShapeData::unmanagedCopy,
 * and ShapeData::relayedManagementCopy methods.
 */
class ShapeData {
public:
    /**
     * @brief Helper class responsible for storing the type of data and performing equality comparison.
     */
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
        /**
         * @brief Creates comparator for empty data, equivalent to `Comparator::forType<void>()`
         */
        Comparator() : function{Comparator::theComparator<void>} { }

        /**
         * @brief Creates the comparator for a given type @a T.
         * @tparam T the type, for which the Comparator instance should be created. The type should be trivially
         * copyable (the byte-wise copy of a valid object should also be valid) and @a operator== should be defined for
         * it. @a T can also be @a void.
         * @return the Comparator instance for type @a T
         */
        template<typename T>
        static Comparator forType() {
            using DecayT = std::remove_reference_t<T>;
            if constexpr (!std::is_same_v<DecayT, void>) {
                static_assert(std::is_trivially_copyable_v<DecayT>, "Type must be trivially copyable");
                static_assert(detail::has_equality_operator_v<DecayT>, "operator== for the type is required");
            }
            return Comparator(Comparator::theComparator<DecayT>);
        }

        /**
         * @brief Compares if raw @a std::byte data arrays @a data1 and @a data2 are equal using @a operator== for the
         * type the Comparator represents.
         */
        [[nodiscard]] bool equal(const std::byte *data1, const std::byte *data2) const {
            return this->function(data1, data2);
        }

        /**
         * @brief Returns @a true if the Comparator -s @a c1 and @a c2 represent the same data type, @a false otherwise.
         */
        friend bool operator==(const Comparator &c1, const Comparator &c2) {
            return c1.function == c2.function;
        }

        /**
         * @brief Returns @a true if the Comparator -s @a c1 and @a c2 represent different data types, @a false
         * otherwise.
         */
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
    /**
     * @brief Helper method which converts a pointer to constant raw @a std::byte data to a constant reference of the
     * concrete type @a T, checking if @a T valid (trivially copyable and with @a operator== defined) beforehand.
     */
    template<typename T>
    static const T &as(const std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return *reinterpret_cast<const NoRefT*>(data);
    }

    /**
     * @brief Helper method which converts a pointer to mutable raw @a std::byte data to a mutable reference of the
     * concrete type @a T, checking if @a T valid (trivially copyable and with @a operator== defined) beforehand.
     */
    template<typename T>
    static T &as(std::byte *data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return *reinterpret_cast<NoRefT*>(data);
    }

    /**
     * @brief Helper method which converts a constant reference of the concrete type @a T to a pointer to constant raw
     * @a std::byte data, checking if @a T valid (trivially copyable and with @a operator== defined) beforehand.
     */
    template<typename T>
    static const std::byte *raw(const T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return reinterpret_cast<const std::byte*>(&data);
    }

    /**
     * @brief Helper method which converts a mutable reference of the concrete type @a T to a pointer to mutable raw
     * @a std::byte data, checking if @a T valid (trivially copyable and with @a operator== defined) beforehand.
     */
    template<typename T>
    static std::byte *raw(T &data) {
        using NoRefT = std::remove_reference_t<T>;
        static_assert(std::is_trivially_copyable_v<NoRefT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<NoRefT>, "operator== for the type is required");

        return reinterpret_cast<std::byte*>(&data);
    }


    /**
     * @brief Creates an empty, @a unmanaged ShapeData instance - storing the void type, with Comparator given by
     * `Comparator::forType<void>()`.
     */
    ShapeData() = default;

    /**
     * @brief Copy constructor creating the @a managed copy of @a other (apart from empty data, which is always
     * @a unmanaged).
     */
    ShapeData(const ShapeData &other) : ShapeData(other.data, other.size, other.comparator, true) { }

    /**
     * @brief Move constructor moving the data and relaying the data management type of @a other, which becomes a valid
     * empty ShapeData after the move.
     */
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

    /**
     * @brief Creates the ShapeData instance based on the data with concrete type @a T. By default, the instance is
     * @a managed.
     * @param data perfect-forwarded data of type @a T
     * @param managed toggles weather @a data should be copied (@a managed storage) or only referenced (@a unmanaged
     * storage). In the second case, @a data must be the an l-value reference (otherwise an exception is thrown) and
     * outlive the ShapeData instance and all its unmanaged copies.
     * @throws PreconditionException if @a `managed == true` and @a data is not an l-value reference
     */
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

    /**
     * @brief Creates the ShapeData instance based on the raw @a std::byte data. Be default, the instance is
     * @a unmanaged.
     * @param data pointer to raw data
     * @param size number of bytes contained in the data
     * @param comparator Comparator for the type of data passed via @a data argument
     * @param managed toggles weather @a data should be copied (@a managed storage) or only referenced (@a unmanaged
     * storage). In the second case, external data must outlive the ShapeData instance and all its unmanaged copies.
     */
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

    /**
     * @brief If the @a managed storage is used, it frees the data memory. Otherwise, it does nothing.
     */
    ~ShapeData() {
        if (this->managed)
            delete[] data;
    }

    /**
     * @brief Assigns a @a managed copy of @a other to this instance (apart from empty data, which is always
     * @a unmanaged). Current memory is released if it was in the @a managed mode. After the move, @a other becomes a
     * valid empty ShapeData.
     */
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

    /**
     * @brief Move-assigns @a other to this instance, relaying its storage management type. Current memory is released
     * if it was in the @a managed mode.
     */
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

    /**
     * @brief Assigns the managed copy of @a data_ with the concrete type @a T to the ShapeData instance. Current memory
     * is released if it was in the @a managed mode.
     */
    template<typename T,
             typename DecayedT = std::decay_t<T>,
             typename = std::enable_if_t<!std::is_same_v<ShapeData, DecayedT>>>
    ShapeData &operator=(T &&data_) {
        static_assert(std::is_trivially_copyable_v<DecayedT>, "Type must be trivially copyable");
        static_assert(detail::has_equality_operator_v<DecayedT>, "operator== for the type is required");

        *this = ShapeData(data_);
        return *this;
    }

    /**
     * @brief Returns the data stored as the object of type @a T.
     * @details Type check is not performed, but `sizeof(T)` cannot exceed `this->getSize()`.
     * @throws PreconditionException if `sizeof(T) > this->getSize()`
     */
    template<typename T>
    const T &as() const {
        ExpectsMsg(sizeof(T) <= this->size, "ShapeData::as(): extracting too large type");

        return ShapeData::as<T>(this->data);
    }

    /**
     * @brief Returns @a true if the currently stored data is of type @a T.
     */
    template<typename T>
    [[nodiscard]] bool stores() const {
        return this->comparator == Comparator::forType<T>();
    }

    /**
     * @brief Returns @a true if the instance of ShapeData manages data's memory.
     */
    [[nodiscard]] bool isManaged() const { return this->managed; }

    /**
     * @brief Returns the size of the data stored (in bytes).
     */
    [[nodiscard]] std::size_t getSize() const { return this->size; }

    /**
     * @brief Returns @a true if the data storage is empty.
     */
    [[nodiscard]] bool isEmpty() const { return this->size == 0; }

    /**
     * @brief Returns raw @a std::byte pointer to the data stored. For empty data, it is always @a nullptr.
     */
    [[nodiscard]] const std::byte *raw() const { return this->data; }

    /**
     * @brief Returns the Comparator for the currently stored data type.
     */
    [[nodiscard]] const Comparator &getComparator() const { return this->comparator; }

    /**
     * @brief Returns an @a unmanaged copy of the data stored, regardless of the current storage state.
     */
    [[nodiscard]] ShapeData unmanagedCopy() const {
        return ShapeData(this->data, this->size, this->comparator, false);
    }

    /**
     * @brief Returns an @a managed copy of the data stored, regardless of the current storage mode (apart from empty
     * data, which is always @a unmanaged).
     */
    [[nodiscard]] ShapeData managedCopy() const {
        return ShapeData(this->data, this->size, this->comparator, true);
    }

    /**
     * @brief Returns copy of the data stored, preserving its storage mode.
     */
    [[nodiscard]] ShapeData relayedManagementCopy() const {
        if (this->managed)
            return this->managedCopy();
        else
            return this->unmanagedCopy();
    }

    /**
     * @brief Compares if two ShapeData instances are equal - namely, if both the data type and the contents, as per
     * the Comparator, are equal. Storage modes are not compared.
     */
    friend bool operator==(const ShapeData &lhs, const ShapeData &rhs) {
        return (lhs.comparator == rhs.comparator) && lhs.comparator.equal(lhs.data, rhs.data);
    }

    /**
     * @brief Compares if two ShapeData instances are not equal - namely, if either the data type or the contents, as
     * per the Comparator, are not equal. Storage modes are not compared.
     */
    friend bool operator!=(const ShapeData &lhs, const ShapeData &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Prints the textual representation of @a shapeData onto the @a out stream.
     * @details Empty data is represented as empty. For non-empty data, the bytes are represented as 2-digit hexadecimal
     * numbers, starting from the first byte at index 0 in the data array, followed by the number of bytes in the
     * parenthesis. For example, `ShapeData(short{0x1AF0})` will be represented as `f0 1a (2 bytes)` on a little-endian
     * architecture.
     */
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
