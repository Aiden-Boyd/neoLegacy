struct LevelChunk {int x,z;bool unloaded=false;LevelChunk(int x,int z):x(x),z(z){}bool isUnloaded(){return unloaded;}bool containsPlayer(){return false;}void unload(bool){unloaded=true;}};
struct CacheService {int ticks=0;bool tick(){++ticks;return true;}};
struct ServerChunkCache {
 Level*level;int saves=0,XZOFFSET=16,XZSIZE=32;deque<LevelChunk*>m_toDrop;vector<LevelChunk*>m_loadedChunkList;
 LevelChunk*cache[1024]={};LevelChunk*m_unloadedCache[1024]={};CacheService svc,*storage=&svc,*source=&svc;
 void save(LevelChunk*){++saves;}void saveEntities(LevelChunk*){}bool tick();
};
