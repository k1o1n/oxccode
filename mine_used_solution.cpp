#include <bits/stdc++.h>
using namespace std;

#pragma GCC optimize("O3")
#pragma GCC target("avx2")

const int MAXN = 30;
const int MAXS = 30;
const int MAXL = 30;
const int MAXM = 300;
const int MAXK = 5;
const int MAXR = 2500;
const int INF = 1e9;

int N,S,L,M,K,P,R,S_per_P;

struct Flow{int id,g1,l1,g2,l2;};
struct Conn{int u,v;};
struct PortInfo{int g,k,ls;};

int oxc_adj[MAXM][MAXR];
vector<Conn> exist_conns[MAXM][MAXN][MAXN];
vector<int> free_ports[MAXM][MAXN];
PortInfo pinfo[200000];

int load_ls[MAXN][MAXL][MAXS];
int load_so[MAXM][MAXR];

inline int get_pid(int g,int ls,int k){return g*(S_per_P*K)+ls*K+k;}

void init_mapping(){
    for(int g=0;g<N;g++)for(int ls=0;ls<S_per_P;ls++)for(int k=0;k<K;k++){
        int pid=get_pid(g,ls,k);
        if(pid<200000) pinfo[pid]={g,k,ls};
    }
}

void reset_topology(){
    for(int m=0;m<M;m++){
        for(int r=0;r<R;r++) oxc_adj[m][r]=-1;
    }
}

void rebuild_cache(){
    for(int m=0;m<M;m++){
        for(int g1=0;g1<N;g1++){
            free_ports[m][g1].clear();
            for(int g2=0;g2<N;g2++) exist_conns[m][g1][g2].clear();
        }
        for(int r=0;r<R;r++){
            int v=oxc_adj[m][r];
            if(v==-1){
                int g=pinfo[r].g;
                free_ports[m][g].push_back(r);
            }else if(r<v){
                int g1=pinfo[r].g,g2=pinfo[v].g;
                if(g1>g2){swap(g1,g2);swap(r,v);}
                exist_conns[m][g1][g2].push_back({r,v});
            }
        }
    }
}

void add_conn_helper(int m,int u,int v){
    int g1=pinfo[u].g,g2=pinfo[v].g;
    if(g1>g2){swap(g1,g2);swap(u,v);}
    exist_conns[m][g1][g2].push_back({u,v});
    auto &f1=free_ports[m][g1];
    for(int i=0;i<(int)f1.size();i++) if(f1[i]==u){f1[i]=f1.back();f1.pop_back();break;}
    auto &f2=free_ports[m][g2];
    for(int i=0;i<(int)f2.size();i++) if(f2[i]==v){f2[i]=f2.back();f2.pop_back();break;}
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>N>>S>>L)) return 0;
    cin>>M>>K>>P;
    S_per_P=S/P;
    R=N*S_per_P*K;
    init_mapping();
    reset_topology();
    int queries=5;
    while(queries--){
        int Q; if(!(cin>>Q)) break;
        vector<Flow> flows(Q);
        for(int i=0;i<Q;i++){
            flows[i].id=i;
            cin>>flows[i].g1>>flows[i].l1>>flows[i].g2>>flows[i].l2;
        }
        memset(load_ls,0,sizeof(load_ls));
        memset(load_so,0,sizeof(load_so));
        rebuild_cache();
        vector<array<int,5>> ans(Q);
        for(int i=0;i<Q;i++){
            int g1=flows[i].g1,l1=flows[i].l1,g2=flows[i].g2,l2=flows[i].l2;
            long long best_metric=-1;
            int best_m=-1,best_u=-1,best_v=-1,best_s1=-1,best_k1=-1,best_s2=-1,best_k2=-1;
            bool found=false;
            for(int pl=0;pl<P;pl++){
                int start_m=pl*(M/P),end_m=start_m+(M/P);
                int base_s=pl*S_per_P;
                for(int m=start_m;m<end_m;m++){
                    int ga=min(g1,g2),gb=max(g1,g2);
                    const auto &conns=exist_conns[m][ga][gb];
                    for(const auto &c:conns){
                        int u=c.u,v=c.v;
                        int p1=(pinfo[u].g==g1)?u:v;
                        int p2=(pinfo[u].g==g1)?v:u;
                        int s1=base_s+pinfo[p1].ls;
                        int s2=base_s+pinfo[p2].ls;
                        int cur=max({load_ls[g1][l1][s1],load_so[m][p1],load_so[m][p2],load_ls[g2][l2][s2]});
                        long long metric=((long long)cur<<20);
                        if(!found||metric<best_metric){
                            best_metric=metric;best_m=m;best_u=p1;best_v=p2;
                            best_s1=s1;best_k1=pinfo[p1].k;best_s2=s2;best_k2=pinfo[p2].k;found=true;
                        }
                    }
                    if(found&&best_metric==0) continue;
                    int bu=-1,bv=-1,minlu=INF,minlv=INF,us=-1,uk=-1,vs=-1,vk=-1;
                    for(int pid:free_ports[m][g1]){
                        int s=base_s+pinfo[pid].ls;
                        int ll=load_ls[g1][l1][s];
                        if(ll<minlu){minlu=ll;bu=pid;us=s;uk=pinfo[pid].k;if(ll==0) break;}
                    }
                    if(bu!=-1){
                        for(int pid:free_ports[m][g2]){
                            int s=base_s+pinfo[pid].ls;
                            int ll=load_ls[g2][l2][s];
                            if(ll<minlv){minlv=ll;bv=pid;vs=s;vk=pinfo[pid].k;if(ll==0) break;}
                        }
                        if(bv!=-1){
                            int cur=max(minlu,minlv);
                            long long metric=((long long)cur<<20)+1;
                            if(!found||metric<best_metric){
                                best_metric=metric;best_m=m;best_u=bu;best_v=bv;
                                best_s1=us;best_k1=uk;best_s2=vs;best_k2=vk;found=true;
                            }
                        }
                    }
                }
            }
            if(!found){
                best_m=0;best_s1=0;best_k1=0;best_s2=0;best_k2=0;
            }else{
                if(oxc_adj[best_m][best_u]!=best_v){
                    oxc_adj[best_m][best_u]=best_v;
                    oxc_adj[best_m][best_v]=best_u;
                    add_conn_helper(best_m,best_u,best_v);
                }
                load_ls[g1][l1][best_s1]++;
                load_ls[g2][l2][best_s2]++;
                load_so[best_m][best_u]++;
                load_so[best_m][best_v]++;
            }
            ans[i]={best_s1,best_k1,best_m,best_s2,best_k2};
        }
        for(int m=0;m<M;m++){
            for(int r=0;r<R;r++){
                cout<<oxc_adj[m][r]<<(r==R-1? "":" ");
            }
            cout<<"\n";
        }
        for(int i=0;i<Q;i++){
            cout<<ans[i][0]<<" "<<ans[i][1]<<" "<<ans[i][2]<<" "<<ans[i][3]<<" "<<ans[i][4]<<"\n";
        }
    }
    return 0;
}