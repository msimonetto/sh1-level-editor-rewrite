#pragma once

#include <vector>
#include <memory>
#include <string>

#include "mgmt/Workspace.h"

struct WindowConfig {
	unsigned int width = 1024,
				 height = 768;
	std::string title = "untitled - SHEditor";
};

class Window {
	private:
		Workspace* workspace_ = nullptr;
		WindowConfig config_;
		std::vector<std::unique_ptr<int>> panels_;

		void Init();
		void RenderPanels();
	public:
		explicit Window(Workspace* workspace, WindowConfig config = {});
		~Window();

		void Open();
		// void SetWorkspace(Workspace* workspace) { workspace_ = workspace; }
};
