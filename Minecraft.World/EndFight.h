#pragma once
class Level;
class EnderDragon;
class EnderCrystal;
class EndFight
{
public:
    static void tick(Level *level);
    static bool tryRespawn(Level *level);
    static void crystalDestroyed(Level *level,EnderCrystal *crystal);
    static int deathReward(Level *level,EnderDragon *dragon);
    static bool dragonDefeated(Level *level,EnderDragon *dragon);
};
