#include <string.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include "imgui.h"
#include "ModalHoldPicker.h"
#include "FilePicker.h"
#include "SpectrumFile.h"

static linked_list *pList = NULL;
static const char *pSelectedFilename = NULL;

void RefreshFiles(const char *pWorkingDirectory)
{
    pSelectedFilename = NULL;
    if (pList!=NULL)
    {
        FreeLinkedList(pList);
    }
    pList = GetFilesInFolder(pWorkingDirectory);
    pList = SortLinkedList(pList);
}

bool HoldPicker(const char *pWorkingDirectory, bool bCanSave, BufferIODouble *pBuffer, float *sample_rate, uint32_t *fft_size)
{
    if (bCanSave==false)
        pSelectedFilename = NULL;

    bool res = false;
    if (ImGui::BeginListBox("##empty", ImVec2(-1, 5 * ImGui::GetTextLineHeightWithSpacing())))
    {
        linked_list *pTmp = pList;
        while(pTmp != NULL)
        {
            const bool is_selected = (pSelectedFilename == pTmp->pStr);
            if (ImGui::Selectable(pTmp->pStr, is_selected))
            {
                pSelectedFilename = pTmp->pStr;

                char filename[1024];
                snprintf(filename, sizeof(filename), "%s/%s", pWorkingDirectory, pTmp->pStr);

                res = LoadSpectrum(filename, pBuffer, sample_rate, fft_size);
            }

            pTmp = pTmp->pNext;
        }
        ImGui::EndListBox();
    }

    // Save file

    bool bDisableSave = false;

    if ((pSelectedFilename != NULL) || (bCanSave==false))
        bDisableSave = true;

    if (bDisableSave) 
        ImGui::BeginDisabled();

    if ( ImGui::Button("保存"))
    {
        time_t timer;
        struct tm* tm_info;
        timer = time(NULL);
        tm_info = localtime(&timer);

        char filename[1024];
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S_data.spec", tm_info);
        snprintf(filename, sizeof(filename), "%s/%s", pWorkingDirectory, timestamp);

        res = SaveSpectrum(filename, pBuffer, *sample_rate, *fft_size);

        RefreshFiles(pWorkingDirectory);
    }

    if (bDisableSave) 
        ImGui::EndDisabled();

    // Delete selectedfile

    bool bDisableFileOperations = (pSelectedFilename == NULL);

    if (bDisableFileOperations) 
        ImGui::BeginDisabled();

    ImGui::SameLine();
    static char newFilename[1024];
    if (ImGui::Button("重命名"))
    {
        ImGui::OpenPopup("重命名");
        strcpy(newFilename, pSelectedFilename);
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("重命名", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("新名称");
        ImGui::InputText("###Name", newFilename, 1024, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::Separator();
        if (ImGui::Button("确定", ImVec2(120, 0))) 
        { 
            std::string oldPath = std::string(pWorkingDirectory) + "/" + pSelectedFilename;
            std::string newPath = std::string(pWorkingDirectory) + "/" + newFilename;
            rename(oldPath.c_str(), newPath.c_str());
            RefreshFiles(pWorkingDirectory);
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("取消", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("删除"))
    {
        ImGui::OpenPopup("删除？");
    }

    center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("删除？", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("确定要删除吗？");
        ImGui::Separator();
        if (ImGui::Button("确定", ImVec2(120, 0))) 
        { 
            char filename[1024];
            snprintf(filename, sizeof(filename), "%s/%s", pWorkingDirectory, pSelectedFilename);
            remove(filename);

            RefreshFiles(pWorkingDirectory);

            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("取消", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (bDisableFileOperations) 
        ImGui::EndDisabled();

    return res;
}

