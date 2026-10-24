#pragma once
#include <cmath>
namespace SlimeBlockPhysics {
inline double land(double velocity,bool sneaking,bool living) {return !sneaking&&velocity<0?-velocity*(living?1.:.8):0.;}
inline double drag(double velocity,bool sneaking) {return !sneaking&&std::abs(velocity)<.1?.4+std::abs(velocity)*.2:1.;}
}
