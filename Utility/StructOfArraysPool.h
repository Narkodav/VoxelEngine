#pragma once
#include <vector>
#include <tuple>
#include <list>
#include <span>
#include <array>
#include <stack>
#include <stdexcept>
#include <type_traits>
#include <limits>
#include <utility>
#include <functional>

template<size_t I, typename... Ts>
struct GetTypeFromIndex;

template<typename T, typename... Ts>
struct GetTypeFromIndex<0, T, Ts...> {
    using Type = T;
};

template<size_t I, typename T, typename... Ts>
struct GetTypeFromIndex<I, T, Ts...> {
    static_assert(I <= sizeof...(Ts), "Index out of bounds");
    using Type = typename GetTypeFromIndex<I - 1, Ts...>::Type;
};

template<size_t I, typename... Ts>
using GetTypeFromIndex_t = typename GetTypeFromIndex<I, Ts...>::Type;

//this is passed to the pool
template<typename TypeTemplate, size_t amountTemplate>
struct StructOfArraysPoolType {
    static inline const size_t amount = amountTemplate;
    using Type = TypeTemplate;
};

// Type trait to check if a type is StructOfArraysPoolType
template<typename T>
struct isSoaPoolType : std::false_type {};

template<typename TypeTemplate, size_t amountTemplate>
struct isSoaPoolType<StructOfArraysPoolType<TypeTemplate, amountTemplate>> : std::true_type {};

template<typename T>
inline constexpr bool isSoaPoolType_v = isSoaPoolType<T>::value;

//this is a pool that allocates data based on the SOA design, meaning each type is an entry into its own array
template<typename... Types>
class StructOfArraysPool
{
public:
    static inline const size_t s_fieldAmount = sizeof...(Types);
    static inline const std::array<size_t, s_fieldAmount> s_amounts = { Types::amount... };

    template <size_t index>
    using PoolType = GetTypeFromIndex_t<index, Types...>;

private:
    using Storage = std::tuple<std::vector<typename Types::Type>...>;
    using PointerStorage = std::tuple<typename Types::Type*...>;

public:
    struct Allocation {
        friend class StructOfArraysPool;
        static inline const size_t s_invalidAlloc = std::numeric_limits<size_t>::max();

        template <size_t index>
        using Type = typename PoolType<index>::Type;

        template <size_t index>
        static inline constexpr size_t amount = PoolType<index>::amount;

    private:
        PointerStorage m_data;
        size_t m_index = s_invalidAlloc;
    public:
        bool isValid() { return m_index != s_invalidAlloc; };

        inline const auto& getIndex() const { return m_index; };

        template <size_t index>
        inline const auto& getEntryOffset() const { return m_index * PoolType<index>::amount; };

        template <size_t index>
        inline std::conditional_t<(amount<index> == 1),
            Type<index>&, std::span<Type<index>>> getField() {
            static_assert(index < s_fieldAmount, "Index must be less than field amount");
            if constexpr (amount<index> == 1)
                return *std::get<index>(m_data);
            else return std::span<Type<index>>(std::get<index>(m_data), amount<index>);
        };

        template <size_t index>
        inline std::conditional_t<(amount<index> == 1),
            const Type<index>&, std::span<const Type<index>>> getField() const {
            static_assert(index < s_fieldAmount, "Index must be less than field amount");
            if constexpr (amount<index> == 1)
                return *std::get<index>(m_data);
            else return std::span<const Type<index>>(std::get<index>(m_data), amount<index>);
        };
    };

private:
    static_assert((isSoaPoolType_v<Types> && ...),
        "All types must be StructOfArraysPoolType instances");

    std::stack<size_t> m_freeIndices;
    Storage m_fields;
    size_t m_poolSize = 0;
    size_t m_allocationAmount = 0;

public:
    StructOfArraysPool() = default;

    StructOfArraysPool(size_t poolSize) : m_poolSize(poolSize) {
        std::apply([this](auto&... vectors) {
            (vectors.resize(Types::amount * m_poolSize), ...);
            }, m_fields);
        for (size_t i = 0; i < m_poolSize; ++i)
            m_freeIndices.push(i);
    }

    StructOfArraysPool(const StructOfArraysPool&) = delete;
    StructOfArraysPool& operator=(const StructOfArraysPool&) = delete;

    StructOfArraysPool(StructOfArraysPool&&) = default;
    StructOfArraysPool& operator=(StructOfArraysPool&&) = default;

    template <size_t index>
    inline auto& getField() {
        static_assert(index < s_fieldAmount,
            "Index must be less than field amount");
        return std::get<index>(m_fields);
    };

    template <size_t index>
    inline const auto& getField() const {
        static_assert(index < s_fieldAmount,
            "Index must be less than field amount");
        return std::get<index>(m_fields);
    };

    template <size_t index>
    inline auto getData(Allocation& alloc) {
        static_assert(index < s_fieldAmount,
            "Index must be less than field amount");
        return std::span(std::get<index>(m_fields).data() + alloc.offsets[index], s_amounts[index]);
    };

    size_t getPoolSize() const { return m_poolSize; }

    inline auto allocate() {
        if (m_freeIndices.empty())
            throw std::bad_alloc();
        Allocation alloc;
        alloc.m_index = m_freeIndices.top();
        m_freeIndices.pop();
        alloc.m_data = [this, &alloc]<size_t... I>(std::index_sequence<I...>) {
            return std::make_tuple(
                (std::get<I>(m_fields).data() + alloc.m_index * s_amounts[I])...
            );
        }(std::make_index_sequence<s_fieldAmount>{});
        ++m_allocationAmount;
        return alloc;
    };

    inline auto deallocate(Allocation& alloc) {
        if (!alloc.isValid()) throw std::runtime_error("Trying to deallocate an invalid allocation");
        m_freeIndices.push(alloc.m_index);
        alloc.m_index = Allocation::s_invalidAlloc;
        --m_allocationAmount;
        alloc.m_data = PointerStorage{};
    };

    void clear() {
        std::apply([this](auto&... vectors) {
            (vectors.resize(Types::amount * m_poolSize), ...);
            }, m_fields);
        m_freeIndices = std::stack<size_t>();
        m_allocationAmount = 0;
        for (size_t i = 0; i < m_poolSize; ++i)
            m_freeIndices.push(i);
    };
};