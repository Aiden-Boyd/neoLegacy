"""Test the production plane transform and GPU upload using a bounded D3D fixture."""
from pathlib import Path
import importlib.util
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = "Minecraft.Client/Windows64/4JLibs/impls/Windows_Libs/Render/src/RendererState.cpp"

FIXTURE = r'''
#include <cassert>
#include <cmath>
#include <cstring>
#include <initializer_list>
#define PROFILER_SCOPE(...)
#define FAILED(hr) ((hr)<0)
enum { D3D11_MAP_WRITE_DISCARD, MATRIX_MODE_MODELVIEW, MP_ORCHID1 };
struct D3D11_MAPPED_SUBRESOURCE { void* pData=nullptr; };
namespace DirectX {
struct XMVECTOR { float v[4]; };
struct XMMATRIX { float m[4][4]; };
using XMFLOAT4X4=XMMATRIX;
struct XMFLOAT4 { float x,y,z,w; };
XMVECTOR XMVectorSet(float x,float y,float z,float w){return {{x,y,z,w}};}
XMVECTOR XMVectorZero(){return {{0,0,0,0}};}
XMMATRIX XMLoadFloat4x4(const XMFLOAT4X4* m){return *m;}
XMMATRIX XMMatrixTranspose(XMMATRIX a){XMMATRIX b{};for(int i=0;i<4;++i)for(int j=0;j<4;++j)b.m[i][j]=a.m[j][i];return b;}
XMVECTOR XMVector4Transform(XMVECTOR v,XMMATRIX m){XMVECTOR out{};for(int j=0;j<4;++j)for(int i=0;i<4;++i)out.v[j]+=v.v[i]*m.m[i][j];return out;}
void XMStoreFloat4(XMFLOAT4* out,XMVECTOR v){std::memcpy(out,v.v,sizeof(v));}
XMMATRIX XMMatrixInverse(XMVECTOR*,XMMATRIX m){
    double a[4][8]{};
    for(int i=0;i<4;++i){for(int j=0;j<4;++j)a[i][j]=m.m[i][j];a[i][i+4]=1;}
    for(int i=0;i<4;++i){
        int pivot=i;for(int k=i+1;k<4;++k)if(std::abs(a[k][i])>std::abs(a[pivot][i]))pivot=k;
        for(int j=0;j<8;++j){double t=a[i][j];a[i][j]=a[pivot][j];a[pivot][j]=t;}
        double scale=a[i][i];assert(std::abs(scale)>1e-10);
        for(int j=0;j<8;++j)a[i][j]/=scale;
        for(int k=0;k<4;++k)if(k!=i){double f=a[k][i];for(int j=0;j<8;++j)a[k][j]-=f*a[i][j];}
    }
    XMMATRIX out{};for(int i=0;i<4;++i)for(int j=0;j<4;++j)out.m[i][j]=float(a[i][j+4]);return out;
}
}
struct Device {
    float staging[32]{},gpu[32]{};bool mapped=false,fail=false;int unmaps=0;
    int Map(int,int,int,int,D3D11_MAPPED_SUBRESOURCE* out){
        assert(!mapped);if(fail)return -1;mapped=true;out->pData=staging;return 0;
    }
    void Unmap(int,int){assert(mapped);std::memcpy(gpu,staging,sizeof(gpu));mapped=false;++unmaps;}
};
struct Renderer {
    struct Context { DirectX::XMMATRIX texGenMatrices[2]{};Device device;Device* m_pDeviceContext=&device;int m_texGenMatricesBuffer=0; } c;
    DirectX::XMMATRIX modelView{};
    Context& getContext(){return c;}
    const float* MatrixGet(int){return &modelView.m[0][0];}
    void StateSetTexGenCol(int,float,float,float,float,bool);
    void UpdateTexGenState();
};
'''

TEST = r'''
int main(){
    Renderer r;
    for(int yaw=-180;yaw<=180;yaw+=15)for(int pitch=-90;pitch<=90;pitch+=15){
        double a=yaw*3.141592653589793/180,b=pitch*3.141592653589793/180;
        float ca=float(std::cos(a)),sa=float(std::sin(a)),cb=float(std::cos(b)),sb=float(std::sin(b));
        // A rigid row-vector model-view matrix with rotation and translation.
        r.modelView={{{ca,sa*sb,-sa*cb,0},{0,cb,sb,0},{sa,-ca*sb,ca*cb,0},{11,-7,23,1}}};
        r.StateSetTexGenCol(3,0,1,0,0,true);
        for(float x:{-1.f,0.f,1.f})for(float z:{-1.f,0.f,1.f}){
            auto camera=DirectX::XMVector4Transform(DirectX::XMVectorSet(x,2,z,1),r.modelView);
            const float* eye=&r.c.texGenMatrices[0].m[0][0];
            double q=0;for(int k=0;k<4;++k)q+=camera.v[k]*eye[3+4*k];
            assert(std::isfinite(q) && std::abs(q-2)<1e-4); // no angle-dependent projection poles
        }
        r.StateSetTexGenCol(0,1,0,0,0,false);
        for(int k=0;k<4;++k)assert(r.c.texGenMatrices[0].m[k][0]==0);
    }
    r.UpdateTexGenState();
    assert(r.c.device.unmaps==1 && !r.c.device.mapped);
    assert(std::memcmp(r.c.device.gpu,r.c.texGenMatrices,sizeof(r.c.device.gpu))==0);
    // Failed mapping must not dereference a null pointer or unmap the resource.
    r.c.device.fail=true;r.UpdateTexGenState();assert(r.c.device.unmaps==1);
}
'''


def extract(source, signature):
    start=source.index(signature)
    opening=source.index("{",start)
    end,depth=opening+1,1
    while depth:
        depth+=(source[end]=="{")-(source[end]=="}")
        end+=1
    return source[start:end]


def main():
    source=(ROOT/SOURCE).read_text(encoding="utf-8")
    spec=importlib.util.spec_from_file_location("base_tests",ROOT/"tests/base_stability_regression.py")
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    code=FIXTURE+extract(source,"void Renderer::StateSetTexGenCol(")+extract(source,"void Renderer::UpdateTexGenState(")+TEST
    with tempfile.TemporaryDirectory() as tmp:
        base.compile_and_run(Path(tmp),"portal_projection",code)


if __name__=="__main__":
    main()
