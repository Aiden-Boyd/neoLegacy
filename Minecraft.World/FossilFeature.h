#pragma once
class Level;
class Random;
class FossilFeature
{
public:
    static void decorate(Level *level,int xo,int zo);
    static bool place(Level *level,Random *random,int xo,int zo,int variant,int rotation);
};
