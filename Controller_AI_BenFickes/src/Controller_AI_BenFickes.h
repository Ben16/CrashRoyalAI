// MIT License
// 
// Copyright(c) 2020 Arthur Bacon and Kevin Dill
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "iController.h"
#include "iPlayer.h"



class Controller_AI_BenFickes : public iController
{
public:
    Controller_AI_BenFickes() {}
    virtual ~Controller_AI_BenFickes() {}

    void tick(float deltaTSec);

private:
    void getMinElixir();
    void getPrincessRange();
    float threatLevel(iPlayer::EntityData mob);
    float minElixirCost = -1.f;
    float princessRange = -1.f;
    float deltaEnemyScan = 0.f;
    const float deltaEnemyThreshold = 0.3f;
    bool uhOhReady = true;
    float emoteCooldown = 1.0f;
    float emoteDelta = 0.f;
};