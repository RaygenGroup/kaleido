#pragma once

#include "vulkan/vulkan.hpp"

#include "renderer/renderers/vulkan/PhysicalDevice.h"

#include <windows.h>


namespace vlkn {

// Instance layer wrapper
class InstanceLayer {
	// TODO: currently can't add as unique
	vk::SurfaceKHR m_surface;
	vk::UniqueInstance m_instance;

	// WIP: change mem management here
	std::vector<std::unique_ptr<PhysicalDevice>> m_capablePhysicalDevices;

	vk::UniqueDebugUtilsMessengerEXT m_debugUtilsMessenger;

public:
	InstanceLayer(HWND assochWnd, HINSTANCE instance);
	~InstanceLayer();

	vk::SurfaceKHR GetSurface() { return m_surface; }

	PhysicalDevice* GetBestCapablePhysicalDevice();

	[[nodiscard]] vk::Instance GetInstance() const { return m_instance.get(); }
};
} // namespace vlkn
