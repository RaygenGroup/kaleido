#pragma once

#include "renderer/renderers/vulkan/Device.h"
#include "asset/pods/ModelPod.h"

#include "vulkan/vulkan.hpp"

namespace vlkn {

class Texture {

	vk::UniqueImage m_handle;
	vk::UniqueDeviceMemory m_memory;

	vk::UniqueImageView m_view;

	// PERF: one to many views
	vk::UniqueSampler m_sampler;

public:
	Texture(Device* device, PodHandle<TexturePod> handle);

	vk::ImageView GetView() const { return m_view.get(); }
	vk::Sampler GetSampler() const { return m_sampler.get(); }
};
} // namespace vlkn
