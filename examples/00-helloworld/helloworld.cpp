/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "logo.h"
#include "imgui/imgui.h"

#include <vector>

namespace
{

	class ExampleHelloWorld : public entry::AppI
	{
	public:
		ExampleHelloWorld(const char* _name, const char* _description, const char* _url)
			: entry::AppI(_name, _description, _url)
		{
		}

		//Try to create and update a 2D texture array. It works on all platforms except D3D12
		void CreateTexture2DArray()
		{
			uint16_t width = 256;
			uint16_t height = 98;
			auto generateMips = false;
			uint16_t depth = 2;
			auto format = bgfx::TextureFormat::RGBA32F;
			auto flags = BGFX_CAPS_TEXTURE_2D_ARRAY;

			auto handle = bgfx::createTexture2D(width, height, generateMips, depth, format, flags);

			std::vector<uint8_t> data{};
			data.resize(width * height * depth * 4 * sizeof(float));

			uint8_t* dataPtr = static_cast<uint8_t*>(data.data());
			size_t dataSize = data.size();

			size_t textureSize = dataSize / static_cast<size_t>(depth);

			for (uint16_t i = 0; i < depth; i++)
			{
				uint8_t* begin = dataPtr + (textureSize * static_cast<size_t>(i));
				const bgfx::Memory* dataCopy = bgfx::copy(begin, static_cast<uint32_t>(textureSize)); // This is required since BGFX must manage the data the memory.
				bgfx::updateTexture2D(handle, i, 0, 0, 0, width, height, dataCopy);
			}
		}

		void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
		{
			Args args(_argc, _argv);

			m_width = _width;
			m_height = _height;
			m_debug = BGFX_DEBUG_TEXT;
			m_reset = BGFX_RESET_VSYNC;

			bgfx::Init init;
			init.type = args.m_type;
			init.vendorId = args.m_pciId;
			init.resolution.width = m_width;
			init.resolution.height = m_height;
			init.resolution.reset = m_reset;
			bgfx::init(init);

			// Enable debug text.
			bgfx::setDebug(m_debug);

			// Set view 0 clear state.
			bgfx::setViewClear(0
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);

			imguiCreate();
			CreateTexture2DArray();
		}

		virtual int shutdown() override
		{
			imguiDestroy();

			// Shutdown bgfx.
			bgfx::shutdown();

			return 0;
		}

		bool update() override
		{
			if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
			{
				imguiBeginFrame(m_mouseState.m_mx
					, m_mouseState.m_my
					, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
					, m_mouseState.m_mz
					, uint16_t(m_width)
					, uint16_t(m_height)
				);

				showExampleDialog(this);

				imguiEndFrame();

				// Set view 0 default viewport.
				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

				// This dummy draw call is here to make sure that view 0 is cleared
				// if no other draw calls are submitted to view 0.
				bgfx::touch(0);

				// Use debug font to print information about this example.
				bgfx::dbgTextClear();
				bgfx::dbgTextImage(
					bx::max<uint16_t>(uint16_t(m_width / 2 / 8), 20) - 20
					, bx::max<uint16_t>(uint16_t(m_height / 2 / 16), 6) - 6
					, 40
					, 12
					, s_logo
					, 160
				);

				bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

				bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
				bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

				const bgfx::Stats* stats = bgfx::getStats();
				bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
					, stats->width
					, stats->height
					, stats->textWidth
					, stats->textHeight
				);

				// Advance to next frame. Rendering thread will be kicked to
				// process submitted rendering primitives.
				bgfx::frame();

				return true;
			}

			return false;
		}

		entry::MouseState m_mouseState;

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_debug;
		uint32_t m_reset;
	};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	ExampleHelloWorld
	, "00-helloworld"
	, "Initialization and debug text."
	, "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
);
