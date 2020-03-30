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

static const Vec2 ksGiantPosLeft(LEFT_BRIDGE_CENTER_X, RIVER_TOP_Y - 0.5f);
static const Vec2 ksGiantPosRight(RIGHT_BRIDGE_CENTER_X, RIVER_TOP_Y - 0.5f);
//static const Vec2 placePos(LEFT_BRIDGE_CENTER_X, RIVER_TOP_Y - 2.5f); // temporary - just to test


//threat levels
static const float NO_THREAT = 100000;
static const float LOW_THREAT = 300000;
static const float MED_THREAT = 500000;

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

void Controller_AI_BenFickes::getPrincessRange() {
    int buildCount = m_pPlayer->getNumBuildings();
    for (unsigned int i = 0; i < buildCount; ++i) {
        iPlayer::EntityData data = m_pPlayer->getBuilding(i);
        if (data.m_Stats.getBuildingType() == iEntityStats::BuildingType::Princess) {
            princessRange = data.m_Stats.getAttackRange();
            return;
        }
    }
    //shouldnt happen but let's be safe
    princessRange = 7.f;
}

// a heuristic to determine how threatening to our towers this particular enemy is
float Controller_AI_BenFickes::threatLevel(iPlayer::EntityData mob) {
    if (minElixirCost < 0) { //annoying, but player isn't instantiated until tick, so we have to do this here
        getMinElixir();
    }

    if (princessRange < 0) {
        getPrincessRange(); //annoying - but no static princess tower info
    }

    if (m_pPlayer->isNorth() && mob.m_Position.y > NorthPrincessY + princessRange) { //start defending when close to the princess tower
        return 0; //we only care about enemies on our side. Since we are on defense, we can react quickly.
    }
    if (!m_pPlayer->isNorth() && mob.m_Position.y < SouthPrincessY - princessRange) { //start defending when close to the princess tower
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

    float elixir = m_pPlayer->getElixir();
    Vec2 placePos(0, 0);
    Vec2 rangedPlacePos(0, 0);
    
    if (elixir < minElixirCost) {
        return; // nothing we can do, we have no elixir
    }

    float totalThreatLevel = 0.f;
    deltaEnemyScan += deltaTSec;
    if (deltaEnemyScan >= deltaEnemyThreshold) { // we don't want to do this every frame, too expensive
        deltaEnemyScan = 0;
        unsigned int count = m_pPlayer->getNumOpponentMobs();
        unsigned int threatCount = 0;
        Vec2 totalPos(0, 0);
        for (int i = 0; i < count; ++i) {
            iPlayer::EntityData enemy = m_pPlayer->getOpponentMob(i);
            float individualThreat = threatLevel(enemy);
            totalThreatLevel += individualThreat;
            if (individualThreat > 0.1f) {
                totalPos += enemy.m_Position;
                ++threatCount;
            }
        }
        //get average position
        if (threatCount > 0) {
            placePos = totalPos / threatCount;
        }
        

        //want range to be off to the side
        if ((placePos + Vec2(2, 0)).x < (GAME_GRID_WIDTH * PIXELS_PER_METER) / 2) {
            rangedPlacePos = placePos + Vec2(2, 0);
        }
        else if ((placePos - Vec2(2, 0)).x > (GAME_GRID_WIDTH* PIXELS_PER_METER) / 2) {
            rangedPlacePos = placePos - Vec2(2, 0);
        }
        else {
            rangedPlacePos = placePos;
        }

        count = m_pPlayer->getNumMobs();
        for (int i = 0; i < count; ++i) {
            iPlayer::EntityData defense = m_pPlayer->getMob(i);
            totalThreatLevel -= threatLevel(defense); //like enemies, we should only count friendly mobs on defense side as lowering threat
        }
    }

    if (elixir > 9.5f) {
        //we're about to be wasteful - might as well spawn a giant
        unsigned int randVal = rand() % 2;
        if (randVal == 0) {
            Vec2 giantPosLeftGame = ksGiantPosLeft.Player2Game(m_pPlayer->isNorth());
            m_pPlayer->placeMob(iEntityStats::MobType::Giant, giantPosLeftGame);
        }
        else {
            Vec2 giantPosRightGame = ksGiantPosRight.Player2Game(m_pPlayer->isNorth());
            m_pPlayer->placeMob(iEntityStats::MobType::Giant, giantPosRightGame);
        }
    }

    unsigned int spawnCount = 0;
    unsigned int needToSpawn = -1;
    if (totalThreatLevel < NO_THREAT) {
        return; //nothing we need to do here, trust towers
    }
    else if (totalThreatLevel < LOW_THREAT) {
        //spawn 1 unit
        needToSpawn = 1;
    }
    else if (totalThreatLevel < MED_THREAT) {
        //spawn 2 units
        needToSpawn = 2;

    }
    else {
        //spawn whatever you can
        needToSpawn = 10;
    }

    while (elixir >= minElixirCost && spawnCount < needToSpawn) {
        //we don't spawn giants in defense - they're not defensive units
        if (elixir >= 3.f) {
            //spawn swordsman
            m_pPlayer->placeMob(iEntityStats::MobType::Swordsman, placePos);
            ++spawnCount;
        }
        else if (elixir >= 2.f) {
            //spawn archer
            m_pPlayer->placeMob(iEntityStats::MobType::Archer, rangedPlacePos);
            ++spawnCount;
        }
    }
}

