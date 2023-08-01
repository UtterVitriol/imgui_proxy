#include "proxy_menu.h"
#include "imgui_stdlib.h"
#include <string>
#include "hooks.h"

static bool MyInputTextMultiline(const char* label, std::string* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	return ImGui::InputTextMultiline(label, my_str, size, flags, nullptr, (void*)my_str);
}

extern HookData g_hookData;
extern Data data;
extern std::shared_ptr<MessageData> selected;
extern tSend oSend;
//
//void proxy_menu()
//{
//	ImGuiIO& io = ImGui::GetIO(); (void)io;
//	const float MAX_SCALE = 3.0F;
//	const float MIN_SCALE = 0.3F;
//
//
//#ifdef IMGUI_HAS_VIEWPORT
//	ImGuiViewport* viewport = ImGui::GetMainViewport();
//	ImGui::SetNextWindowPos(viewport->GetWorkPos());
//	ImGui::SetNextWindowSize(viewport->GetWorkSize());
//	ImGui::SetNextWindowViewport(viewport->ID);
//#else 
//	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
//	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
//#endif
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//
//	ImVec2 size;
//	ImGui::Begin("Packet Data");
//	if (ImGui::CollapsingHeader("Settings")) {
//		ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
//	}
//
//	static ImGuiInputTextFlags flags = 0;
//	/* ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
//	 ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", &flags, ImGuiInputTextFlags_AllowTabInput);
//	 ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", &flags, ImGuiInputTextFlags_CtrlEnterForNewLine);*/
//	ImGuiInputTextFlags_;
//	MyInputTextMultiline("##RealPackets", &g_hookData.in, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 64), flags);
//	ImGuiTableFlags;
//
//	if (ImGui::BeginTable("split", 3))
//	{
//		ImGui::TableNextColumn();
//		ImGui::Checkbox("Send Log", &g_hookData.bSend);
//
//		ImGui::TableNextColumn();
//		ImGui::Checkbox("Recv Log", &g_hookData.bRecv);
//
//
//		ImGui::EndTable();
//	}
//
//
//	ImGui::TableNextColumn();
//	if (ImGui::Button("Clear"))
//	{
//		g_hookData.in.clear();
//	}
//
//	ImGuiInputTextFlags_;
//	MyInputTextMultiline("##FakePacket", &g_hookData.out, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
//	ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)g_hookData.in.data(), g_hookData.in.size(), g_hookData.in.capacity());
//	if (ImGui::BeginTable("split", 3))
//	{
//		if (ImGui::Button("send"))
//		{
//			//TODO: send
//		}
//		if (ImGui::Button("clear"))
//		{
//			g_hookData.out.clear();
//		}
//	}
//
//	// Shows addrr and resize stuff
//	//ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)text.data(), text.size(), text.capacity());
//
//	ImGui::End();
//	ImGui::PopStyleVar();
//
//}


void proxy_menu()
{
	static bool bAutoScroll = false;

	packet_data(&bAutoScroll);
	edit_data(&bAutoScroll);

}

void edit_data(bool *bAutoScroll)
{
	std::string temp;

	ImGui::Begin("Edit Panel");
	ImGuiInputTextFlags_;
	if (selected)
	{
		MyInputTextMultiline("##FakePacket", &selected->data, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_NoHorizontalScroll);
		
		
	}
	else
	{
		ImGui::Text("Nothing Selected");
	}
	ImGuiTableFlags_;
	if (ImGui::BeginTable("split1", 4, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableNextColumn();
		ImGui::Checkbox("Auto Scroll", bAutoScroll);

		ImGui::TableNextColumn();
		ImGui::Checkbox("Send Log", &g_hookData.bSend);

		ImGui::TableNextColumn();
		ImGui::Checkbox("Recv Log", &g_hookData.bRecv);

		if (selected)
		{
			ImGui::TableNextColumn();
			ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)selected->data.data(), selected->data.size(), selected->data.capacity());

			ImGui::InputInt("Packet Len", &selected->len);
		}

		ImGui::EndTable();
	}

	if (ImGui::BeginTable("split2", 3, ImGuiTableFlags_SizingFixedFit))
	{
		if (selected)
		{
			ImGui::TableNextColumn();
			if (ImGui::Button("Send"))
			{
				send_modified();
			}
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Clear Packet Data"))
		{
			{
				std::unique_lock<std::mutex> lock(data.mtx);
				data.data.clear();
			}

		}

		ImGui::TableNextColumn();
		if (ImGui::Button("Do Something else"))
		{
			//TODO: Clear Packet data
		}
		ImGui::EndTable();
	}

	ImGui::End();
}


void packet_data(bool* bAutoScroll)
{
	ImGui::Begin("Packet Panel");

	

	static int selection = -1;

	if (ImGui::TreeNode("In columns"))
	{
		if (ImGui::BeginTable("split2", 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
		{
			{
				std::unique_lock<std::mutex> lock(data.mtx);

				for (int i = 0; i < data.data.size(); i++)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					if (ImGui::Selectable(data.data[i]->type.c_str(), selection == i, ImGuiSelectableFlags_SpanAllColumns))
					{
						selection = i;
						selected = data.data[i];
					}
					//ImGui::Selectable(data.data[i]->type.c_str(), &selected[i], ImGuiSelectableFlags_SpanAllColumns);
					ImGui::TableNextColumn();
					ImGui::Text(data.data[i]->strSock.c_str());
					ImGui::TableNextColumn();
					ImGui::Text(data.data[i]->strLen.c_str());
					ImGui::TableNextColumn();
					ImGui::Text(data.data[i]->data.c_str());
				}
			}
			ImGui::EndTable();
		}

		ImGui::TreePop();
	}



	if (*bAutoScroll)
	{
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::End();
}
