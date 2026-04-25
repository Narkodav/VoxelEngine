#pragma once
#include "Graphics/Common.h"
#include "Graphics/Structs.h"
#include "Graphics/DeviceFunctionTable.h"
#include "Graphics/HandleTypes/Device.h"
#include "Graphics/HandleTypes/Sampler.h"
#include "Graphics/HandleTypes/Image.h"
#include "Graphics/HandleTypes/Buffer.h"

namespace Graphics
{
	class DescriptorSetWrite;
	class DescriptorSetCopy;

		class DescriptorImageInfo : public StructBase<VkDescriptorImageInfo, DescriptorImageInfo>
	{
		using Base = StructBase<VkDescriptorImageInfo, DescriptorImageInfo>;
	public:
		using Base::Base;
		DescriptorImageInfo(SamplerRef sampler, ImageViewRef imageView,
			ImageLayout layout) : Base() {
			this->sampler = sampler.getHandle();
			this->imageView = imageView.getHandle();
			this->imageLayout = convertCEnum(layout);
		}
		DescriptorImageInfo& setSampler(SamplerRef sampler) {
			this->sampler = sampler.getHandle();
			return *this;
		}
		DescriptorImageInfo& setImageView(ImageViewRef imageView) {
			this->imageView = imageView.getHandle();
			return *this;
		}
		DescriptorImageInfo& setImageLayout(ImageLayout layout) {
			this->imageLayout = convertCEnum(layout);
			return *this;
		}
	};

	class DescriptorBufferInfo : public StructBase<VkDescriptorBufferInfo, DescriptorBufferInfo>
	{
		using Base = StructBase<VkDescriptorBufferInfo, DescriptorBufferInfo>;
	public:
		using Base::Base;
		DescriptorBufferInfo(BufferRef buffer, DeviceSize offset, DeviceSize range) : Base() {
			this->buffer = buffer.getHandle();
			this->offset = offset;
			this->range = range;
		}
		DescriptorBufferInfo& setBuffer(BufferRef buffer) {
			this->buffer = buffer.getHandle();
			return *this;
		}
		DescriptorBufferInfo& setOffset(DeviceSize offset) {
			this->offset = offset;
			return *this;
		}
		DescriptorBufferInfo& setRange(DeviceSize range) {
			this->range = range;
			return *this;
		}

	};

	class DescriptorSet : public BaseComponent<VkDescriptorSet, DescriptorSet>
	{
		using Base = BaseComponent<VkDescriptorSet, DescriptorSet>;
	public:
		using Base::Base;

		static void update(const DeviceFunctionTable& functions, DeviceRef device,
			std::span<const DescriptorSetWrite> descriptorWrites, std::span<const DescriptorSetCopy> descriptorCopies);

		static void update(const DeviceFunctionTable& functions, DeviceRef device,
			std::span<const DescriptorSetWrite> descriptorWrites);

		static void update(const DeviceFunctionTable& functions, DeviceRef device,
			std::span<const DescriptorSetCopy> descriptorCopies);

		static void update(const DeviceFunctionTable& functions, DeviceRef device,
			const DescriptorSetWrite& descriptorWrite);

		static void update(const DeviceFunctionTable& functions, DeviceRef device,
			const DescriptorSetCopy& descriptorCopy);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, std::span<const DescriptorBufferInfo> bufferInfo,
			DescriptorType descriptorType);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, std::span<const DescriptorImageInfo> imageInfo,
			DescriptorType descriptorType);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, std::span<const BufferViewRef> texelBufferViews,
			DescriptorType descriptorType);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, const DescriptorBufferInfo& bufferInfo,
			DescriptorType descriptorType);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, const DescriptorImageInfo& imageInfo,
			DescriptorType descriptorType);

		void write(const DeviceFunctionTable& functions, DeviceRef device,
			uint32_t dstBinding, uint32_t dstArrayElement, BufferViewRef texelBufferViews,
			DescriptorType descriptorType);

		void copy(const DeviceFunctionTable& functions, DeviceRef device, DescriptorSet srcSet,
			uint32_t srcBinding, uint32_t dstBinding, uint32_t srcArrayElement, uint32_t dstArrayElement,
			uint32_t descriptorCount);
	};

	class DescriptorSetLayoutRef : public BaseComponent<VkDescriptorSetLayout, DescriptorSetLayoutRef>
	{
		using Base = BaseComponent<VkDescriptorSetLayout, DescriptorSetLayoutRef>;
	public:
		using Base::Base;
		static inline const std::string s_typeName = "DescriptorSet::Layout";
	};

	class DescriptorSetLayoutBinding : public StructBase<VkDescriptorSetLayoutBinding, DescriptorSetLayoutBinding>
	{
		using Base = StructBase<VkDescriptorSetLayoutBinding, DescriptorSetLayoutBinding>;
	public:
		using Base::Base;

		constexpr DescriptorSetLayoutBinding(uint32_t binding, DescriptorType descriptorType,
			uint32_t descriptorCount, Flags::ShaderStage stageFlags) : Base() {
			this->binding = binding;
			this->descriptorType = convertCEnum(descriptorType);
			this->descriptorCount = descriptorCount;
			this->stageFlags = stageFlags;
			this->pImmutableSamplers = nullptr;
		}

		DescriptorSetLayoutBinding(uint32_t binding, DescriptorType descriptorType,
			uint32_t descriptorCount, Flags::ShaderStage stageFlags,
			std::span<const SamplerRef> immutableSamplers) : Base() {
			this->binding = binding;
			this->descriptorType = convertCEnum(descriptorType);
			this->descriptorCount = descriptorCount;
			this->stageFlags = stageFlags;
			this->pImmutableSamplers = SamplerRef::underlyingCast(immutableSamplers.data());
		}

		constexpr DescriptorSetLayoutBinding& setBinding(uint32_t binding) {
			this->binding = binding;
			return *this;
		}
		constexpr DescriptorSetLayoutBinding& setDescriptorType(DescriptorType descriptorType) {
			this->descriptorType = convertCEnum(descriptorType);
			return *this;
		}
		constexpr DescriptorSetLayoutBinding& setDescriptorCount(uint32_t descriptorCount) {
			this->descriptorCount = descriptorCount;
			return *this;
		}
		constexpr DescriptorSetLayoutBinding& setStageFlags(Flags::ShaderStage stageFlags) {
			this->stageFlags = stageFlags;
			return *this;
		}
		constexpr DescriptorSetLayoutBinding& setImmutableSamplers(std::span<const SamplerRef> immutableSamplers) {
			this->pImmutableSamplers = SamplerRef::underlyingCast(immutableSamplers.data());
			this->descriptorCount = immutableSamplers.size();
			return *this;
		}
	};

	class DescriptorSetLayoutCreateInfo : public StructBase<VkDescriptorSetLayoutCreateInfo, DescriptorSetLayoutCreateInfo>
	{
		using Base = StructBase<VkDescriptorSetLayoutCreateInfo, DescriptorSetLayoutCreateInfo>;
	public:
		using Base::Base;

		DescriptorSetLayoutCreateInfo(std::span<const DescriptorSetLayoutBinding> bindings,
			Flags::DescriptorSetLayoutCreate flags = Flags::DescriptorSetLayoutCreate::Bits::None) : Base() {
			this->bindingCount = static_cast<uint32_t>(bindings.size());
			this->pBindings = DescriptorSetLayoutBinding::underlyingCast(bindings.data());
			this->flags = flags;
		}

		DescriptorSetLayoutCreateInfo& setFlags(Flags::DescriptorSetLayoutCreate flags) {
			this->flags = flags;
			return *this;
		}

		DescriptorSetLayoutCreateInfo& setBindings(std::span<const DescriptorSetLayoutBinding> bindings) {
			this->bindingCount = static_cast<uint32_t>(bindings.size());
			this->pBindings = DescriptorSetLayoutBinding::underlyingCast(bindings.data());
			return *this;
		}
	};

	class DescriptorSetLayout : public VerificatorComponent<VkDescriptorSetLayout, DescriptorSetLayoutRef>
	{
		using Base = VerificatorComponent<VkDescriptorSetLayout, DescriptorSetLayoutRef>;
	public:
		using Base::Base;

		void create(const DeviceFunctionTable& functions,
			DeviceRef device, const DescriptorSetLayoutCreateInfo& createInfo);
		void destroy(const DeviceFunctionTable& functions, DeviceRef device);
	};

	class DescriptorSetCopy : public StructBase<VkCopyDescriptorSet, DescriptorSetCopy>
	{
		using Base = StructBase<VkCopyDescriptorSet, DescriptorSetCopy>;
	public:
		using Base::Base;

		DescriptorSetCopy(const DescriptorSet& srcSet, const DescriptorSet& dstSet,
			uint32_t srcBinding, uint32_t dstBinding,
			uint32_t srcArrayElement, uint32_t dstArrayElement,
			uint32_t descriptorCount) : Base()
		{
			this->srcSet = srcSet;
			this->dstSet = dstSet;
			this->srcBinding = srcBinding;
			this->dstBinding = dstBinding;
			this->srcArrayElement = srcArrayElement;
			this->dstArrayElement = dstArrayElement;
			this->descriptorCount = descriptorCount;
		}

		DescriptorSetCopy& setSrcSet(const DescriptorSet& srcSet) {
			this->srcSet = srcSet;
			return *this;
		}
		DescriptorSetCopy& setDstSet(const DescriptorSet& dstSet) {
			this->dstSet = dstSet;
			return *this;
		}
		DescriptorSetCopy& setSrcBinding(uint32_t srcBinding) {
			this->srcBinding = srcBinding;
			return *this;
		}
		DescriptorSetCopy& setDstBinding(uint32_t dstBinding) {
			this->dstBinding = dstBinding;
			return *this;
		}
		DescriptorSetCopy& setSrcArrayElement(uint32_t srcArrayElement) {
			this->srcArrayElement = srcArrayElement;
			return *this;
		}
		DescriptorSetCopy& setDstArrayElement(uint32_t dstArrayElement) {
			this->dstArrayElement = dstArrayElement;
			return *this;
		}
		DescriptorSetCopy& setDescriptorCount(uint32_t descriptorCount) {
			this->descriptorCount = descriptorCount;
			return *this;
		}
	};

	class DescriptorSetWrite : public StructBase<VkWriteDescriptorSet, DescriptorSetWrite>
	{
		using Base = StructBase<VkWriteDescriptorSet, DescriptorSetWrite>;
	public:
		using Base::Base;

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, std::span<const DescriptorImageInfo> imageInfo,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pImageInfo = DescriptorImageInfo::underlyingCast(imageInfo.data());
			this->descriptorCount = imageInfo.size();
			this->pTexelBufferView = nullptr;
			this->pBufferInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, std::span<const DescriptorBufferInfo> bufferInfo,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pBufferInfo = DescriptorBufferInfo::underlyingCast(bufferInfo.data());
			this->descriptorCount = bufferInfo.size();
			this->pTexelBufferView = nullptr;
			this->pImageInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, std::span<const BufferViewRef> texelBufferView,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pTexelBufferView = BufferViewRef::underlyingCast(texelBufferView.data());
			this->descriptorCount = texelBufferView.size();
			this->pImageInfo = nullptr;
			this->pBufferInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, const DescriptorImageInfo& imageInfo,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pImageInfo = imageInfo.getUnderlyingPointer();
			this->descriptorCount = 1;
			this->pTexelBufferView = nullptr;
			this->pBufferInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, const DescriptorBufferInfo& bufferInfo,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pBufferInfo = bufferInfo.getUnderlyingPointer();
			this->descriptorCount = 1;
			this->pTexelBufferView = nullptr;
			this->pImageInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite(const DescriptorSet& dstSet, uint32_t dstBinding,
			uint32_t dstArrayElement, const BufferViewRef& texelBufferView,
			DescriptorType descriptorType) : Base() {
			this->dstSet = dstSet;
			this->dstBinding = dstBinding;
			this->dstArrayElement = dstArrayElement;
			this->pTexelBufferView = texelBufferView.getUnderlyingPointer();
			this->descriptorCount = 1;
			this->pImageInfo = nullptr;
			this->pBufferInfo = nullptr;
			this->descriptorType = convertCEnum(descriptorType);
		}

		DescriptorSetWrite& setDescriptorType(DescriptorType descriptorType) {
			this->descriptorType = convertCEnum(descriptorType);
			return *this;
		}
		DescriptorSetWrite& setDstArrayElement(uint32_t dstArrayElement) {
			this->dstArrayElement = dstArrayElement;
			return *this;
		}
		DescriptorSetWrite& setDstBinding(uint32_t dstBinding) {
			this->dstBinding = dstBinding;
			return *this;
		}
		DescriptorSetWrite& setDstSet(const DescriptorSet& dstSet) {
			this->dstSet = dstSet;
			return *this;
		}
		DescriptorSetWrite& setImageInfo(std::span<const DescriptorImageInfo> imageInfo) {
			this->pImageInfo = DescriptorImageInfo::underlyingCast(imageInfo.data());
			this->descriptorCount = imageInfo.size();
			this->pTexelBufferView = nullptr;
			this->pBufferInfo = nullptr;
			return *this;
		}
		DescriptorSetWrite& setBufferInfo(std::span<const DescriptorBufferInfo> bufferInfo) {
			this->pBufferInfo = DescriptorBufferInfo::underlyingCast(bufferInfo.data());
			this->descriptorCount = bufferInfo.size();
			this->pTexelBufferView = nullptr;
			this->pImageInfo = nullptr;
			return *this;
		}
		DescriptorSetWrite& setTexelBufferView(std::span<const BufferViewRef> texelBufferView) {
			this->pTexelBufferView = BufferViewRef::underlyingCast(texelBufferView.data());
			this->descriptorCount = texelBufferView.size();
			this->pImageInfo = nullptr;
			this->pBufferInfo = nullptr;
			return *this;
		}

		DescriptorSetWrite& setImageInfo(const DescriptorImageInfo& imageInfo) {
			this->pImageInfo = DescriptorImageInfo::underlyingCast(&imageInfo);
			this->descriptorCount = 1;
			this->pTexelBufferView = nullptr;
			this->pBufferInfo = nullptr;
			return *this;
		}
		DescriptorSetWrite& setBufferInfo(const DescriptorBufferInfo& bufferInfo) {
			this->pBufferInfo = DescriptorBufferInfo::underlyingCast(&bufferInfo);
			this->descriptorCount = 1;
			this->pTexelBufferView = nullptr;
			this->pImageInfo = nullptr;
			return *this;
		}
		DescriptorSetWrite& setTexelBufferView(const BufferViewRef& texelBufferView) {
			this->pTexelBufferView = BufferViewRef::underlyingCast(&texelBufferView);
			this->descriptorCount = 1;
			this->pImageInfo = nullptr;
			this->pBufferInfo = nullptr;
			return *this;
		}
	};
}

