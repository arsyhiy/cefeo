#include "editor.h"
#include "imgui.h"

void ShowEditorWindow()
{
    static char code[4096 * 4] =
        "// Welcome to your minimal code editor!\n"
        "// Type your code here.\n";
    ImGui::Begin("Code Editor");
    ImGui::InputTextMultiline("##source", code, sizeof(code),
                              ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 32),
                              ImGuiInputTextFlags_AllowTabInput);
    ImGui::End();
}
