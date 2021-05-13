//--------------------------------------------------------------------------------------
// File: main.cpp
//
// This application demonstrates animation using matrix transformations
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729722.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "main.h"

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Graphics* Renderer = new Graphics();

    if (FAILED(Renderer->InitWindow(hInstance, nCmdShow)))
        return 0;

    if (FAILED(Renderer->InitDevice()))
    {
        Renderer->CleanupDevice();
        return 0;
    }

    int totalFrames = 0;

    double t = 0.0;
    double dt = 1.0f / FPS;

    double currentTime = GetTickCount64() / 1000.0;
    double accumulator = 0.0;
    double startTime = currentTime;

    vector<pair<ImVec4, string>> debugLog;
    
    bool steppedFrames = false;
    bool autoStop = false;

    bool stopOnWall = false;
    bool stopOnForce = false;

    vector<double> frameTimeTracker;
    vector<int> numUpdatesTracker;
    bool earlyBreak = false;

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Info");


            ImGui::Checkbox("AutoStop", &autoStop);
            if (autoStop)
            {
                ImGui::Text("Stop on: ");
                ImGui::SameLine();
                ImGui::Checkbox("Wall", &stopOnWall);
                ImGui::SameLine();
                ImGui::Checkbox("Force", &stopOnForce);
            }

            ImGui::Checkbox("Run Stepped?", &steppedFrames);
            if (autoStop)
            {
                ImGui::SameLine();
                if (ImGui::Button("Run Still Next Pause"))
                {
                    steppedFrames = false;
                }
            }

            if(steppedFrames)
            {
                ImGui::SameLine();
                if (ImGui::Button("Step Frame"))
                {
                    debugLog.clear();

                    Renderer->Update(dt, &debugLog);
                }

                double newTime = GetTickCount64() / 1000.0;
                double frameTime = newTime - currentTime;
                currentTime = newTime;

                accumulator += frameTime;

                while (accumulator >= dt)
                {
                    Renderer->ReadKeyboard(dt);
                    Renderer->UpdateCamera();

                    accumulator -= dt;
                    t += dt;
                }
            }
            else
            {
                double newTime = GetTickCount64() / 1000.0;
                double frameTime = newTime - currentTime;

                frameTimeTracker.push_back(frameTime);

                currentTime = newTime;

                accumulator += frameTime;

                int numUpdates = 0;

                if (accumulator > 2.0)
                {
                    earlyBreak = true;
                    totalFrames++;
                    numUpdatesTracker.push_back(-1);
                    break;
                }

                while (accumulator >= dt)
                {
                    debugLog.clear();

                    Renderer->Update(dt, &debugLog);
                    Renderer->ReadKeyboard(dt);
                    Renderer->UpdateCamera();

                    accumulator -= dt;
                    t += dt;

                    numUpdates++;
                }

                numUpdatesTracker.push_back(numUpdates);
            }

            if (autoStop)
            {
                for (int i = 0; i < debugLog.size(); i++)
                {
                    if (stopOnForce)
                    {
                        if ((debugLog[i].first.y == 1.0f && debugLog[i].first.z == 0.0f) || (debugLog[i].first.z == 1.0f && debugLog[i].first.y == 0.0f))
                        {
                            steppedFrames = true;
                        }
                    }
                    else if (stopOnWall)
                    {
                        if (debugLog[i].first.x == 1.0f && debugLog[i].first.y == 0.0f)
                        {
                            steppedFrames = true;
                        }
                    }
                }
            }

            Renderer->Render(&debugLog);
            totalFrames++;
        }

        if (currentTime - startTime > runTime) //Auto stop after X seconds of data collection
        {
            break;
        }
    }

    Renderer->CleanupDevice();

    if (totalFrames == frameTimeTracker.size() && totalFrames == numUpdatesTracker.size())
    {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);

        int numTypes = 1;

        if (NUMCUBES2 != 0)
            numTypes++;
        if (NUMCUBES3 != 0)
            numTypes++;
        if(NUMCUBES4 != 0)
            numTypes++;
        
        std::ofstream file;
        std::ostringstream fileName;
        fileName << "DataSave/DataKernalSize1" << FPS << "FPS" << NUMCUBES1 + NUMCUBES2 + NUMCUBES3 + NUMCUBES4 << "Parts" << numTypes << "Types" << runTime << "sRunTime" << ".csv";
        
        file.open(fileName.str());

        file << "Total Run Time," << currentTime - startTime << ",Early Exit, " << std::boolalpha << earlyBreak << std::noboolalpha << "\n";

        file << "Number of Particles of Type 1," << NUMCUBES1 << "\n";
        file << "Number of Particles of Type 2," << NUMCUBES2 << "\n";
        file << "Number of Particles of Type 3," << NUMCUBES3 << "\n";
        file << "Number of Particles of Type 4," << NUMCUBES4 << "\n";

        file << "Number of Updates,Frame Time,TotalTime,\n";

        double totalTime = 0.0;

        for (int i = 0; i < totalFrames; i++)
        {
            file << numUpdatesTracker[i] << ",";
            file << frameTimeTracker[i] << ",";
            totalTime += frameTimeTracker[i];
            file << totalTime << ",\n";
        }

        file.close();
    }

    return (int)msg.wParam;
}