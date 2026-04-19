#pragma once
#include "JsonParser/Value.h"
#include "Common.h"

#include <tuple>
#include <string>
#include <cstring>


struct MovementConfig {
    float moveSpeed;
    float fastSpeed;
};

template<typename T>
struct MemberObjectTraits;

template<typename C, typename M>
struct MemberObjectTraits<M C::*> {
    using ClassType = C;
    using MemberType = M;
};

template<auto name, auto fieldAccessor>
struct FieldDefinition {    
    static inline const std::string s_name = name;
    static inline const auto s_accessor = fieldAccessor;
    using ClassType = typename MemberObjectTraits<decltype(s_accessor)>::ClassType;
    using MemberType = typename MemberObjectTraits<decltype(s_accessor)>::MemberType;

    constexpr auto& get(ClassType& obj) const { return obj.*s_accessor; }
    constexpr const auto& get(const ClassType& obj) const { return obj.*s_accessor; }

    constexpr const FieldDefinition& set(ClassType& obj, MemberType& val) const { obj.*s_accessor = val; return *this; }
};

template<auto name, typename... Fields>
struct NameToIndex {

    template<size_t index, typename First, typename... Rest>
    struct ToIndex {
        static inline const size_t s_index = First::s_name == name ? index : ToIndex<index + 1, name, Rest...>::s_index;
    };

    template<size_t index, typename Last>
    struct ToIndex<index, Last> {
        static inline const size_t s_index = Last::s_name == name ? index : std::numeric_limits<size_t>::max();
    };

    static inline const size_t s_index = ToIndex<0, Fields...>::s_index;
};

template<typename... Fields>
struct DataSchema {
    static constexpr auto s_fields = std::tuple<Fields...>{};

    template<auto name, typename T>
    constexpr auto& get(T& object) {
        static const auto index = NameToIndex<name, Fields...>::s_index;
        return std::get<index>(s_fields).get(object);
    }

};

template<typename... Schemas>
class JsonDeserializer {




};