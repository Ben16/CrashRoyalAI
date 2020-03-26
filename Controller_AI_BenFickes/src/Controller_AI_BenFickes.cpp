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

#include "Controller_AI_BenFickes.h"

#include "Constants.h"
#include "EntityStats.h"
#include "iPlayer.h"
#include "Vec2.h"

static const Vec2 ksGiantPos(LEFT_BRIDGE_CENTER_X, RIVER_TOP_Y - 0.5f);
static const Vec2 ksArcherPos(LEFT_BRIDGE_CENTER_X, 0.f);

void Controller_AI_BenFickes::getMinElixir() {
    auto allMobTypes = m_pPlayer->GetAvailableMobTypes();
    minElixirCost = MAX_ELIXIR;
    for (iEntityStats::MobType mobType : allMobTypes) {
        const iEntityStats& stats = iEntityStats::getStats(mobType);
        if (stats.getElixirCost() < minElixirCost) {
            minElixirCost = stats.getElixirCost();
        }
    }
}

// a heuristic to determine how threatening to our towers this particular enemy is
float Controller_AI_BenFickes::threatLevel(iPlayer::EntityData mob) {
    if (minElixirCost < 0) { //annoying, but player isn't instantiated until tick, so we have to do this here
        getMinElixir();
    }
    if (mob.m_Position.y > ((float)GAME_GRID_HEIGHT / 2.f)) {
        return 0; //we only care about enemies on our side. Since we are on defense, we can react quickly.
    }
    float dps = mob.m_Stats.getDamage() / mob.m_Stats.getAttackTime(); //dps
    float threat = dps * mob.m_Stats.getMaxHealth(); // dps * survivability = threat
    if (mob.m_Stats.getDamageType() == iEntityStats::DamageType::Ranged) {
        //because ranged units are a greater threat when combined with other units, up the threat level
        threat *= 2.f;
    }
    return threat;
    // for reference, threat levels of 3 current units:
    //S: 202070
    //A: 61714
    //G: 460683
}

void Controller_AI_BenFickes::tick(float deltaTSec)
{
    assert(m_pPlayer);
    
    if (m_pPlayer->getElixir() < minElixirCost) {
        return; // nothing we can do, we have no elixir
    }



    //we want to keep some elixir aside, but spawn an offensive so we're not wasteful
    if (m_pPlayer->getElixir() >= 9)
    {
        // convert the positions from player space to game space
        bool isNorth = m_pPlayer->isNorth();
        Vec2 giantPos_Game = ksGiantPos.Player2Game(isNorth);

        m_pPlayer->placeMob(iEntityStats::Giant, giantPos_Game);
    }
}

