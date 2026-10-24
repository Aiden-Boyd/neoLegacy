struct ChunkSource {Level*l;void create(int,int);};
struct LevelData {EndFightState fight;int gateways=0;EndFightState&getEndFight(){return fight;}int getEndGatewaysGenerated(){return gateways;}};
using Position=tuple<int,int,int>;
struct Level {
 ChunkSource chunks{this};ChunkSource*getChunkSource(){return &chunks;}set<pair<int,int>> missingChunks;int chunkRequests=0;
 static const int genDepth=256;bool noSave=false;bool isClientSide=false,loaded=true,failAdd=false,failBlocks=false;Dimension dim,*dimension=&dim;int seaLevel=64;int64_t time=0;Random random;LevelData data;GameRules rules;
 vector<shared_ptr<Entity>> entities;vector<shared_ptr<Player>>players;map<Position,int> blocks;vector<tuple<double,double,double,float,bool>> explosions;int writes=0,xp=0,eggs=0,sounds=0;
 bool hasChunkAt(int x,int,int z){return loaded&&!missingChunks.count({x>>4,z>>4});}bool hasChunksAt(int x0,int,int z0,int x1,int,int z1){if(!loaded)return false;for(int x=x0>>4;x<=(x1>>4);++x)for(int z=z0>>4;z<=(z1>>4);++z)if(missingChunks.count({x,z}))return false;return true;}
 int getTile(int x,int y,int z){auto it=blocks.find({x,y,z});return it==blocks.end()?0:it->second;}bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}
 Material*getMaterial(int x,int y,int z){static Material empty{true},solid{false};int id=getTile(x,y,z);return id==0||id==31?&empty:&solid;}
 bool setTileAndData(int x,int y,int z,int id,int,int){if(failBlocks)return false;blocks[{x,y,z}]=id;++writes;if(id==Tile::dragonEgg_Id)++eggs;return true;}bool setTileAndUpdate(int x,int y,int z,int id){return setTileAndData(x,y,z,id,0,3);}
 bool addEntity(shared_ptr<Entity> e);
 vector<shared_ptr<Entity>>getAllEntities(){return entities;}
 vector<shared_ptr<Entity>>*getEntities(shared_ptr<Entity>except,AABB*b){auto out=new vector<shared_ptr<Entity>>;for(auto e:entities)if(!e->removed&&e!=except&&e->bb->intersects(b))out->push_back(e);return out;}
 vector<shared_ptr<Entity>>*getEntitiesOfClass(const type_info&,AABB*b);
 LevelData*getLevelData(){return &data;}int64_t getSeed(){return 42;}int64_t getGameTime(){return time;}int GetHighestY(){return 64;}GameRules*getGameRules(){return &rules;}
 void explode(shared_ptr<Entity> source,double x,double y,double z,float r,bool blocks);void playSound(double,double,double,int,float,float){++sounds;}
 void globalLevelEvent(int,int,int,int,int){}void addParticle(int,double,double,double,double,double,double){}
};
struct DamageSource {shared_ptr<Entity> source;bool explosion=false;shared_ptr<Entity>getEntity(){return source;}bool isExplosion(){return explosion;}};
struct Feature {bool placeBlock(Level*l,int x,int y,int z,int id,int data){return l->setTileAndData(x,y,z,id,data,2);}};
struct Mob:Entity {Mob(Level*l):Entity(l){};void addAdditonalSaveData(CompoundTag*){}void readAdditionalSaveData(CompoundTag*){};};
struct EnderDragon:Mob {
 enum EEnderdragonAction {e_EnderdragonAction_HoldingPattern,e_EnderdragonAction_Sitting_Flaming,e_EnderdragonAction_Sitting_Scanning,e_EnderdragonAction_Sitting_Attacking};
 int action=e_EnderdragonAction_Sitting_Scanning,dragonDeathTime=0,deathXpReward=0,m_remainingCrystalsCount=8;static const int CRYSTAL_COUNT=8;float health=0;int crystalHits=0;
 EnderDragon(Level*l):Mob(l){}eINSTANCEOF GetType(){return eTYPE_ENDERDRAGON;}void AddParts(){}EEnderdragonAction getSynchedAction(){return EEnderdragonAction(action);}void setSynchedAction(EEnderdragonAction a,bool){action=a;}
 void setHealth(float n){health=n;}void handleCrystalDestroyed(DamageSource*){++crystalHits;}
 void tickDeath();void spawnExitPortal(int,int);void addAdditonalSaveData(CompoundTag*);void readAdditionalSaveData(CompoundTag*);
};
struct ExperienceOrb:Entity {int value;ExperienceOrb(Level*l,double x,double y,double z,int n):Entity(l),value(n){setPos(x,y,z);}static int getExperienceValue(int n){return n;}};
struct EndGateways {static void dragonDefeated(Level*l){++l->data.gateways;}};
struct TheEndPortal {static void allowAnywhere(bool){}};
struct LevelEvent {enum{SOUND_DRAGON_DEATH};};

void ChunkSource::create(int x,int z){++l->chunkRequests;if(l->loaded)l->missingChunks.erase({x,z});}
