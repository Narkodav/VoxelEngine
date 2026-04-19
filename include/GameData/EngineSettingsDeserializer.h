#pragma once
#include "JsonParser/Value.h"
#include "Common.h"
#include "WorldManagement/WorldGrid.h"
#include "Rendering/Renderer.h"

#include <unordered_map>
#include <string>

// {
//     "Input": {
//         "MouseSensitivity" : 80.0
//     },
//     "Movement": {
//         "MoveVelocity" : { "Value": 10.0, "Units": "MeterPerSecond" },
//         "FastMoveVelocity" : { "Value": 100.0, "Units": "MeterPerSecond" }
//     },
//     "Generator" : {
//         "Type": "Default",
//         "Shape" : "Cylinder",
//         "Params": {
//             "Radius" : { "Value": 10, "Units": "Meter" },
//             "Height" : { "Value": 10, "Units": "Meter" },
//             "BottomCenterPosition" : { "Value": { "X" : 0, "Y" : 0, "Z" : 0 }, "Units": "Meter" }
//         }
//     },
//     "Graphics": {
//         "CameraSettings" : {
//             "WorldUpVector" : {
//                 "X" : 0,
//                 "Y" : 1,
//                 "Z" : 0
//             },
//             "Position" : { "Value": { "X" : 0, "Y" : 0, "Z" : 0 }, "Units": "Meter" },
//             "Pitch" : { "Value": 0.0, "Units": "Degree" },
//             "Yaw" : { "Value": 0.0, "Units": "Degree" },
//             "Fov" : { "Value": 100.0, "Units": "Degree" }
//         },
//		   "Contrast" : 0.8
//     }
// }

// Translation layer between congig files and internal engine data representation
class EngineSettingsDeserializer {
private:
	struct TransparentHash {
		using is_transparent = void;

		size_t operator()(std::string_view sv) const noexcept {
			return std::hash<std::string_view>{}(sv);
		}

		size_t operator()(const std::string& s) const noexcept {
			return std::hash<std::string_view>{}(s);
		}

		size_t operator()(const char* s) const noexcept {
			return std::hash<std::string_view>{}(s);
		}
	};

	struct TransparentEqual {
		using is_transparent = void;

		bool operator()(std::string_view lhs,
						std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
	};
	template<typename Enum>
	using EnumMap = std::unordered_map<std::string, Enum, TransparentHash, TransparentEqual>;

	static inline const EnumMap<WorldGrid::ShapeToGenerate> s_shapeStringToEnum = {
		{ "Sphere", WorldGrid::ShapeToGenerate::Sphere },
		{ "Cylinder", WorldGrid::ShapeToGenerate::Cylinder },
		{ "Parallelepiped", WorldGrid::ShapeToGenerate::Parallelepiped },
		{ "Cube", WorldGrid::ShapeToGenerate::Cube },
	};

public:

    static InputSettings parseInput(const Json::Value& config);
	static MovementSettings parseMovement(const Json::Value& config);
	static WorldGrid::GeneratorSettings parseGenerator(const Json::Value& config);
	static Renderer::GraphicsSettings parseGraphics(const Json::Value& config);


private:

	template<typename T>
	static T getObject(const Json::Value& value) {
		if (value.isObject()) return value.asObject(); 
		else throw std::runtime_error("value must be an object");
	}

	template<typename T>
	static T getNumber(const Json::Value& value) {
		if (value.isInteger()) return value.asInteger(); 
		else if (value.isNumber()) return value.asNumber();
		else throw std::runtime_error("value must contain a number");
	}

	template<typename T>
	static T getVector(const Json::Value& value) {
		T vec;
		if(value.isObject()) {
			vec.x = getNumber<decltype(vec.x)>(value.asObject().at("X"));
			vec.y = getNumber<decltype(vec.y)>(value.asObject().at("Y"));
			vec.z = getNumber<decltype(vec.z)>(value.asObject().at("Z"));
		} else if(value.isArray()) {
			vec.x = getNumber<decltype(vec.x)>(value.asArray()[0]);
			vec.y = getNumber<decltype(vec.y)>(value.asArray()[1]);
			vec.z = getNumber<decltype(vec.z)>(value.asArray()[2]);
		} else throw std::runtime_error("Cannot parse vector that is not object or array");
		return vec;
	}

	template<typename T>
	static T applyUnit(T value, const Json::Value::Object& pairObj) {
		auto unit = pairObj.find("Unit");
		if(unit == pairObj.end()) throw std::runtime_error("value unit pair must contain a unit");
		if(!unit->second.isString()) throw std::runtime_error("unit must be a string");
		const auto& unitStr = unit->second.asString();
		if(unitStr == "Meter") return value;
		else if(unitStr == "Kilometer") return value *= 1000;
		else if(unitStr == "Millimeter") return value /= 1000.f;

		else if(unitStr == "MeterPerSecond") return value;
		else if(unitStr == "KilometerPerSecond") return value *= 1000;
		else if(unitStr == "MillimeterPerSecond") return value /= 1000.f;

		else if(unitStr == "MeterPerHour") return value /= 3600.f;
		else if(unitStr == "KilometerPerHour") return value *= 3.6f;
		else if(unitStr == "MillimeterPerHour") return value /= 1000.f / 3600.f;

		else if(unitStr == "Degree") return value *= 0.01745329251994329576923690768489;

		throw std::runtime_error("unit not implemented");
	}

	static const Json::Value& getValueFromPair(const Json::Value& pair) {
		if (!pair.isObject()) throw std::runtime_error("value unit pair must be an object");
		const auto& valueObj = pair.asObject();
		auto value = valueObj.find("Value");
		if(value == valueObj.end()) throw std::runtime_error("value unit pair must contain a value");
		return value->second;
	}

	template<typename T>
	static T getNumberWithUnit(const Json::Value& pair) {
		auto number = getNumber<T>(getValueFromPair(pair));
		return applyUnit(number, pair.asObject());
	}

	template<typename T>
	static T getVectorWithUnit(const Json::Value& pair) {
		auto vector = getVector<T>(getValueFromPair(pair));
		return applyUnit(vector, pair.asObject());
	}

	static const Json::Value& getInner(const Json::Value& outer, std::string_view outerName, std::string_view innerName) {
		if(!outer.isObject()) throw std::runtime_error(std::string(outerName) + " must be an object");
		const auto& outerObj = outer.asObject();
		auto innerIt = outerObj.find(innerName);
		if(innerIt == outerObj.end()) throw std::runtime_error(std::string(outerName) + " must contain " + std::string(innerName));
		return innerIt->second;
	}

	static const Json::Value::Object& getInnerObject(const Json::Value& config, std::string_view settingName) {
		const auto& settingObj = getInner(config, "Config", settingName);
		if(!settingObj.isObject()) throw std::runtime_error(std::string(settingName) + " must be an object");
		return settingObj.asObject();
	}

	template<typename Enum>
	static Enum getEnum(EnumMap<Enum> enumMap, std::string_view value) {
		auto enumIt = enumMap.find(value);
		if(enumIt == enumMap.end()) throw std::runtime_error(std::string(value) + " - unknown enum value");
		return enumIt->second;
	}
};