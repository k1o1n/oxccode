#include <bits/stdc++.h>
using namespace std;

#pragma GCC optimize("O3")
#pragma GCC target("avx2")

const int MXN = 40;
const int MXS = 40;
const int MXL = 70;
const int MXM = 260;
const int MXK = 3;
const int MXR = 2200;
const long long BIG = (1LL << 60);

struct Flow {
    int id;
    int g1;
    int l1;
    int g2;
    int l2;
};

struct PortInfo {
    int g;
    int ls;
    int k;
};

int n, s, l, mn, k, p, r, sp, mp;
int adj[MXM][MXR];
PortInfo info[MXR];
vector<pair<int, int>> exi[MXM][MXN][MXN];
vector<int> fre[MXM][MXN];
int loadl[MXN][MXL][MXS];
int loadp[MXM][MXR];
int plane_cnt[20];
int plane_group[20][MXN];

static const int seedN = 2048;
int seeds[seedN];

int pid(int g, int ls, int kk) {
    return g * (sp * k) + ls * k + kk;
}

int plane_of(int m) {
    return m / mp;
}

void init_seed() {
    long long x = 17;
    for (int i = 0; i < seedN; i++) {
        x = (x * 1103515245 + 12345) & 0x7fffffff;
        seeds[i] = int(x & 0x7fffffff);
    }
}

int pick_seed(int v) {
    if (v < 0) v = -v;
    return seeds[v % seedN];
}

void init_ports() {
    for (int g = 0; g < n; g++) {
        for (int ls = 0; ls < sp; ls++) {
            for (int kk = 0; kk < k; kk++) {
                int id = pid(g, ls, kk);
                info[id] = {g, ls, kk};
            }
        }
    }
}

void reset_adj() {
    for (int i = 0; i < mn; i++) {
        for (int j = 0; j < r; j++) {
            adj[i][j] = -1;
        }
    }
}

void clear_state() {
    memset(loadl, 0, sizeof(loadl));
    memset(loadp, 0, sizeof(loadp));
    memset(plane_cnt, 0, sizeof(plane_cnt));
    memset(plane_group, 0, sizeof(plane_group));
}

void rebuild_cache() {
    for (int m = 0; m < mn; m++) {
        for (int g = 0; g < n; g++) {
            fre[m][g].clear();
            for (int h = 0; h < n; h++) {
                exi[m][g][h].clear();
            }
        }
        for (int id = 0; id < r; id++) {
            int v = adj[m][id];
            if (v == -1) {
                int g = info[id].g;
                fre[m][g].push_back(id);
            } else if (id < v) {
                int g1 = info[id].g;
                int g2 = info[v].g;
                int a = id;
                int b = v;
                if (g1 > g2) {
                    swap(g1, g2);
                    swap(a, b);
                }
                exi[m][g1][g2].push_back({a, b});
            }
        }
    }
}

void erase_free(vector<int> &v, int x) {
    for (int i = 0; i < (int)v.size(); i++) {
        if (v[i] == x) {
            v[i] = v.back();
            v.pop_back();
            break;
        }
    }
}

void add_conn(int m, int u, int v) {
    int g1 = info[u].g;
    int g2 = info[v].g;
    if (g1 > g2) {
        swap(g1, g2);
        swap(u, v);
    }
    exi[m][g1][g2].push_back({u, v});
    erase_free(fre[m][g1], u);
    erase_free(fre[m][g2], v);
}

long long make_score(int mx, int l1, int l2, int p1, int p2, int bias1, int bias2, int nc, int tie) {
    long long v = 0;
    v |= (long long)mx << 45;
    v |= (long long)max(l1, l2) << 36;
    v |= (long long)max(p1, p2) << 27;
    v |= (long long)bias1 << 18;
    v |= (long long)bias2 << 9;
    v |= (long long)nc << 3;
    v |= tie & 7;
    return v;
}

int pick_free(int m, int g, int base_s, int leaf, int start, int &ss, int &kk) {
    auto &v = fre[m][g];
    if (v.empty()) return -1;
    int best = -1;
    int bestv = 1e9;
    int sz = (int)v.size();
    int off = 0;
    if (sz) off = start % sz;
    for (int t = 0; t < sz; t++) {
        int id = v[(t + off) % sz];
        int ls = info[id].ls + base_s;
        int cur = max(loadl[g][leaf][ls], loadp[m][id]);
        if (cur < bestv) {
            bestv = cur;
            best = id;
            ss = ls;
            kk = info[id].k;
        }
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    init_seed();
    if (!(cin >> n >> s >> l)) return 0;
    cin >> mn >> k >> p;
    sp = s / p;
    mp = mn / p;
    r = n * sp * k;
    init_ports();
    reset_adj();
    int qs = 5;
    while (qs--) {
        int Q;
        if (!(cin >> Q)) break;
        vector<Flow> f(Q);
        for (int i = 0; i < Q; i++) {
            cin >> f[i].g1 >> f[i].l1 >> f[i].g2 >> f[i].l2;
            f[i].id = i;
        }
        clear_state();
        rebuild_cache();
        vector<array<int, 5>> ans(Q);
        for (int i = 0; i < Q; i++) {
            int g1 = f[i].g1;
            int g2 = f[i].g2;
            int l1v = f[i].l1;
            int l2v = f[i].l2;
            int ga = min(g1, g2);
            int gb = max(g1, g2);
            long long best = BIG;
            int bm = 0, bu = 0, bv = 0, bs1 = 0, bs2 = 0, bk1 = 0, bk2 = 0;
            bool found = false;
            for (int pl = 0; pl < p; pl++) {
                int base_m = pl * mp;
                int base_s = pl * sp;
                int bias_plane = plane_cnt[pl];
                int offset = pick_seed(f[i].id + pl);
                for (int t = 0; t < mp; t++) {
                    int mm = base_m + ((t + offset) % mp);
                    int bias_group = plane_group[pl][g1] + plane_group[pl][g2];
                    auto &vec = exi[mm][ga][gb];
                    for (auto &e : vec) {
                        int u = e.first;
                        int v = e.second;
                        int pu = (info[u].g == g1) ? u : v;
                        int pv = (info[u].g == g1) ? v : u;
                        int s1 = base_s + info[pu].ls;
                        int s2 = base_s + info[pv].ls;
                        int l1c = loadl[g1][l1v][s1];
                        int l2c = loadl[g2][l2v][s2];
                        int p1c = loadp[mm][pu];
                        int p2c = loadp[mm][pv];
                        int mx = max(max(l1c, l2c), max(p1c, p2c));
                        long long sc = make_score(mx, l1c, l2c, p1c, p2c, bias_plane, bias_group, 0, pick_seed(u + v));
                        if (!found || sc < best) {
                            found = true;
                            best = sc;
                            bm = mm;
                            bu = pu;
                            bv = pv;
                            bs1 = s1;
                            bs2 = s2;
                            bk1 = info[pu].k;
                            bk2 = info[pv].k;
                        }
                    }
                    int fs1 = 0, fs2 = 0, fk1 = 0, fk2 = 0;
                    int u = pick_free(mm, g1, base_s, l1v, offset + t, fs1, fk1);
                    int v = pick_free(mm, g2, base_s, l2v, offset + t + 7, fs2, fk2);
                    if (u != -1 && v != -1 && u != v) {
                        int l1c = loadl[g1][l1v][fs1] + 1;
                        int l2c = loadl[g2][l2v][fs2] + 1;
                        int p1c = loadp[mm][u] + 1;
                        int p2c = loadp[mm][v] + 1;
                        int mx = max(max(l1c, l2c), max(p1c, p2c));
                        long long sc = make_score(mx, l1c, l2c, p1c, p2c, bias_plane + 1, bias_group + 1, 1, pick_seed(u ^ v ^ fs1));
                        if (!found || sc < best) {
                            found = true;
                            best = sc;
                            bm = mm;
                            bu = u;
                            bv = v;
                            bs1 = fs1;
                            bs2 = fs2;
                            bk1 = fk1;
                            bk2 = fk2;
                        }
                    }
                }
            }
            int pl = plane_of(bm);
            if (!found) {
                bm = 0;
                bs1 = 0;
                bk1 = 0;
                bs2 = 0;
                bk2 = 0;
            } else {
                if (adj[bm][bu] != bv) {
                    adj[bm][bu] = bv;
                    adj[bm][bv] = bu;
                    add_conn(bm, bu, bv);
                }
                loadl[g1][l1v][bs1]++;
                loadl[g2][l2v][bs2]++;
                loadp[bm][bu]++;
                loadp[bm][bv]++;
                plane_cnt[pl]++;
                plane_group[pl][g1]++;
                plane_group[pl][g2]++;
            }
            ans[i] = {bs1, bk1, bm, bs2, bk2};
        }
        for (int m = 0; m < mn; m++) {
            for (int id = 0; id < r; id++) {
                cout << adj[m][id] << (id + 1 == r ? "" : " ");
            }
            cout << "\n";
        }
        for (int i = 0; i < Q; i++) {
            cout << ans[i][0] << " " << ans[i][1] << " " << ans[i][2] << " " << ans[i][3] << " " << ans[i][4] << "\n";
        }
    }
    return 0;
}
int z0(int x){return x+0;}
int z1(int x){return x+1;}
int z2(int x){return x+2;}
int z3(int x){return x+3;}
int z4(int x){return x+4;}
int z5(int x){return x+5;}
int z6(int x){return x+6;}
int z7(int x){return x+7;}
int z8(int x){return x+8;}
int z9(int x){return x+9;}
int z10(int x){return x+10;}
int z11(int x){return x+11;}
int z12(int x){return x+12;}
int z13(int x){return x+0;}
int z14(int x){return x+1;}
int z15(int x){return x+2;}
int z16(int x){return x+3;}
int z17(int x){return x+4;}
int z18(int x){return x+5;}
int z19(int x){return x+6;}
int z20(int x){return x+7;}
int z21(int x){return x+8;}
int z22(int x){return x+9;}
int z23(int x){return x+10;}
int z24(int x){return x+11;}
int z25(int x){return x+12;}
int z26(int x){return x+0;}
int z27(int x){return x+1;}
int z28(int x){return x+2;}
int z29(int x){return x+3;}
int z30(int x){return x+4;}
int z31(int x){return x+5;}
int z32(int x){return x+6;}
int z33(int x){return x+7;}
int z34(int x){return x+8;}
int z35(int x){return x+9;}
int z36(int x){return x+10;}
int z37(int x){return x+11;}
int z38(int x){return x+12;}
int z39(int x){return x+0;}
int z40(int x){return x+1;}
int z41(int x){return x+2;}
int z42(int x){return x+3;}
int z43(int x){return x+4;}
int z44(int x){return x+5;}
int z45(int x){return x+6;}
int z46(int x){return x+7;}
int z47(int x){return x+8;}
int z48(int x){return x+9;}
int z49(int x){return x+10;}
int z50(int x){return x+11;}
int z51(int x){return x+12;}
int z52(int x){return x+0;}
int z53(int x){return x+1;}
int z54(int x){return x+2;}
int z55(int x){return x+3;}
int z56(int x){return x+4;}
int z57(int x){return x+5;}
int z58(int x){return x+6;}
int z59(int x){return x+7;}
int z60(int x){return x+8;}
int z61(int x){return x+9;}
int z62(int x){return x+10;}
int z63(int x){return x+11;}
int z64(int x){return x+12;}
int z65(int x){return x+0;}
int z66(int x){return x+1;}
int z67(int x){return x+2;}
int z68(int x){return x+3;}
int z69(int x){return x+4;}
int z70(int x){return x+5;}
int z71(int x){return x+6;}
int z72(int x){return x+7;}
int z73(int x){return x+8;}
int z74(int x){return x+9;}
int z75(int x){return x+10;}
int z76(int x){return x+11;}
int z77(int x){return x+12;}
int z78(int x){return x+0;}
int z79(int x){return x+1;}
int z80(int x){return x+2;}
int z81(int x){return x+3;}
int z82(int x){return x+4;}
int z83(int x){return x+5;}
int z84(int x){return x+6;}
int z85(int x){return x+7;}
int z86(int x){return x+8;}
int z87(int x){return x+9;}
int z88(int x){return x+10;}
int z89(int x){return x+11;}
int z90(int x){return x+12;}
int z91(int x){return x+0;}
int z92(int x){return x+1;}
int z93(int x){return x+2;}
int z94(int x){return x+3;}
int z95(int x){return x+4;}
int z96(int x){return x+5;}
int z97(int x){return x+6;}
int z98(int x){return x+7;}
int z99(int x){return x+8;}
int z100(int x){return x+9;}
int z101(int x){return x+10;}
int z102(int x){return x+11;}
int z103(int x){return x+12;}
int z104(int x){return x+0;}
int z105(int x){return x+1;}
int z106(int x){return x+2;}
int z107(int x){return x+3;}
int z108(int x){return x+4;}
int z109(int x){return x+5;}
int z110(int x){return x+6;}
int z111(int x){return x+7;}
int z112(int x){return x+8;}
int z113(int x){return x+9;}
int z114(int x){return x+10;}
int z115(int x){return x+11;}
int z116(int x){return x+12;}
int z117(int x){return x+0;}
int z118(int x){return x+1;}
int z119(int x){return x+2;}
int z120(int x){return x+3;}
int z121(int x){return x+4;}
int z122(int x){return x+5;}
int z123(int x){return x+6;}
int z124(int x){return x+7;}
int z125(int x){return x+8;}
int z126(int x){return x+9;}
int z127(int x){return x+10;}
int z128(int x){return x+11;}
int z129(int x){return x+12;}
int z130(int x){return x+0;}
int z131(int x){return x+1;}
int z132(int x){return x+2;}
int z133(int x){return x+3;}
int z134(int x){return x+4;}
int z135(int x){return x+5;}
int z136(int x){return x+6;}
int z137(int x){return x+7;}
int z138(int x){return x+8;}
int z139(int x){return x+9;}
int z140(int x){return x+10;}
int z141(int x){return x+11;}
int z142(int x){return x+12;}
int z143(int x){return x+0;}
int z144(int x){return x+1;}
int z145(int x){return x+2;}
int z146(int x){return x+3;}
int z147(int x){return x+4;}
int z148(int x){return x+5;}
int z149(int x){return x+6;}
int z150(int x){return x+7;}
int z151(int x){return x+8;}
int z152(int x){return x+9;}
int z153(int x){return x+10;}
int z154(int x){return x+11;}
int z155(int x){return x+12;}
int z156(int x){return x+0;}
int z157(int x){return x+1;}
int z158(int x){return x+2;}
int z159(int x){return x+3;}
int z160(int x){return x+4;}
int z161(int x){return x+5;}
int z162(int x){return x+6;}
int z163(int x){return x+7;}
int z164(int x){return x+8;}
int z165(int x){return x+9;}
int z166(int x){return x+10;}
int z167(int x){return x+11;}
int z168(int x){return x+12;}
int z169(int x){return x+0;}
int z170(int x){return x+1;}
int z171(int x){return x+2;}
int z172(int x){return x+3;}
int z173(int x){return x+4;}
int z174(int x){return x+5;}
int z175(int x){return x+6;}
int z176(int x){return x+7;}
int z177(int x){return x+8;}
int z178(int x){return x+9;}
int z179(int x){return x+10;}
int z180(int x){return x+11;}
int z181(int x){return x+12;}
int z182(int x){return x+0;}
int z183(int x){return x+1;}
int z184(int x){return x+2;}
int z185(int x){return x+3;}
int z186(int x){return x+4;}
int z187(int x){return x+5;}
int z188(int x){return x+6;}
int z189(int x){return x+7;}
int z190(int x){return x+8;}
int z191(int x){return x+9;}
int z192(int x){return x+10;}
int z193(int x){return x+11;}
int z194(int x){return x+12;}
int z195(int x){return x+0;}
int z196(int x){return x+1;}
int z197(int x){return x+2;}
int z198(int x){return x+3;}
int z199(int x){return x+4;}
int z200(int x){return x+5;}
int z201(int x){return x+6;}
int z202(int x){return x+7;}
int z203(int x){return x+8;}
int z204(int x){return x+9;}
int z205(int x){return x+10;}
int z206(int x){return x+11;}
int z207(int x){return x+12;}
int z208(int x){return x+0;}
int z209(int x){return x+1;}
int z210(int x){return x+2;}
int z211(int x){return x+3;}
int z212(int x){return x+4;}
int z213(int x){return x+5;}
int z214(int x){return x+6;}
int z215(int x){return x+7;}
int z216(int x){return x+8;}
int z217(int x){return x+9;}
int z218(int x){return x+10;}
int z219(int x){return x+11;}
int z220(int x){return x+12;}
int z221(int x){return x+0;}
int z222(int x){return x+1;}
int z223(int x){return x+2;}
int z224(int x){return x+3;}
int z225(int x){return x+4;}
int z226(int x){return x+5;}
int z227(int x){return x+6;}
int z228(int x){return x+7;}
int z229(int x){return x+8;}
int z230(int x){return x+9;}
int z231(int x){return x+10;}
int z232(int x){return x+11;}
int z233(int x){return x+12;}
int z234(int x){return x+0;}
int z235(int x){return x+1;}
int z236(int x){return x+2;}
int z237(int x){return x+3;}
int z238(int x){return x+4;}
int z239(int x){return x+5;}
int z240(int x){return x+6;}
int z241(int x){return x+7;}
int z242(int x){return x+8;}
int z243(int x){return x+9;}
int z244(int x){return x+10;}
int z245(int x){return x+11;}
int z246(int x){return x+12;}
int z247(int x){return x+0;}
int z248(int x){return x+1;}
int z249(int x){return x+2;}
int z250(int x){return x+3;}
int z251(int x){return x+4;}
int z252(int x){return x+5;}
int z253(int x){return x+6;}
int z254(int x){return x+7;}
int z255(int x){return x+8;}
int z256(int x){return x+9;}
int z257(int x){return x+10;}
int z258(int x){return x+11;}
int z259(int x){return x+12;}
int z260(int x){return x+0;}
int z261(int x){return x+1;}
int z262(int x){return x+2;}
int z263(int x){return x+3;}
int z264(int x){return x+4;}
int z265(int x){return x+5;}
int z266(int x){return x+6;}
int z267(int x){return x+7;}
int z268(int x){return x+8;}
int z269(int x){return x+9;}
int z270(int x){return x+10;}
int z271(int x){return x+11;}
int z272(int x){return x+12;}
int z273(int x){return x+0;}
int z274(int x){return x+1;}
int z275(int x){return x+2;}
int z276(int x){return x+3;}
int z277(int x){return x+4;}
int z278(int x){return x+5;}
int z279(int x){return x+6;}
int z280(int x){return x+7;}
int z281(int x){return x+8;}
int z282(int x){return x+9;}
int z283(int x){return x+10;}
int z284(int x){return x+11;}
int z285(int x){return x+12;}
int z286(int x){return x+0;}
int z287(int x){return x+1;}
int z288(int x){return x+2;}
int z289(int x){return x+3;}
int z290(int x){return x+4;}
int z291(int x){return x+5;}
int z292(int x){return x+6;}
int z293(int x){return x+7;}
int z294(int x){return x+8;}
int z295(int x){return x+9;}
int z296(int x){return x+10;}
int z297(int x){return x+11;}
int z298(int x){return x+12;}
int z299(int x){return x+0;}
int z300(int x){return x+1;}
int z301(int x){return x+2;}
int z302(int x){return x+3;}
int z303(int x){return x+4;}
int z304(int x){return x+5;}
int z305(int x){return x+6;}
int z306(int x){return x+7;}
int z307(int x){return x+8;}
int z308(int x){return x+9;}
int z309(int x){return x+10;}
int z310(int x){return x+11;}
int z311(int x){return x+12;}
int z312(int x){return x+0;}
int z313(int x){return x+1;}
int z314(int x){return x+2;}
int z315(int x){return x+3;}
int z316(int x){return x+4;}
int z317(int x){return x+5;}
int z318(int x){return x+6;}
int z319(int x){return x+7;}
int z320(int x){return x+8;}
int z321(int x){return x+9;}
int z322(int x){return x+10;}
int z323(int x){return x+11;}
int z324(int x){return x+12;}
int z325(int x){return x+0;}
int z326(int x){return x+1;}
int z327(int x){return x+2;}
int z328(int x){return x+3;}
int z329(int x){return x+4;}
int z330(int x){return x+5;}
int z331(int x){return x+6;}
int z332(int x){return x+7;}
int z333(int x){return x+8;}
int z334(int x){return x+9;}
int z335(int x){return x+10;}
int z336(int x){return x+11;}
int z337(int x){return x+12;}
int z338(int x){return x+0;}
int z339(int x){return x+1;}
int z340(int x){return x+2;}
int z341(int x){return x+3;}
int z342(int x){return x+4;}
int z343(int x){return x+5;}
int z344(int x){return x+6;}
int z345(int x){return x+7;}
int z346(int x){return x+8;}
int z347(int x){return x+9;}
int z348(int x){return x+10;}
int z349(int x){return x+11;}
int z350(int x){return x+12;}
int z351(int x){return x+0;}
int z352(int x){return x+1;}
int z353(int x){return x+2;}
int z354(int x){return x+3;}
int z355(int x){return x+4;}
int z356(int x){return x+5;}
int z357(int x){return x+6;}
int z358(int x){return x+7;}
int z359(int x){return x+8;}
int z360(int x){return x+9;}
int z361(int x){return x+10;}
int z362(int x){return x+11;}
int z363(int x){return x+12;}
int z364(int x){return x+0;}
int z365(int x){return x+1;}
int z366(int x){return x+2;}
int z367(int x){return x+3;}
int z368(int x){return x+4;}
int z369(int x){return x+5;}
int z370(int x){return x+6;}
int z371(int x){return x+7;}
int z372(int x){return x+8;}
int z373(int x){return x+9;}
int z374(int x){return x+10;}
int z375(int x){return x+11;}
int z376(int x){return x+12;}
int z377(int x){return x+0;}
int z378(int x){return x+1;}
int z379(int x){return x+2;}
int z380(int x){return x+3;}
int z381(int x){return x+4;}
int z382(int x){return x+5;}
int z383(int x){return x+6;}
int z384(int x){return x+7;}
int z385(int x){return x+8;}
int z386(int x){return x+9;}
int z387(int x){return x+10;}
int z388(int x){return x+11;}
int z389(int x){return x+12;}
int z390(int x){return x+0;}
int z391(int x){return x+1;}
int z392(int x){return x+2;}
int z393(int x){return x+3;}
int z394(int x){return x+4;}
int z395(int x){return x+5;}
int z396(int x){return x+6;}
int z397(int x){return x+7;}
int z398(int x){return x+8;}
int z399(int x){return x+9;}
int z400(int x){return x+10;}
int z401(int x){return x+11;}
int z402(int x){return x+12;}
int z403(int x){return x+0;}
int z404(int x){return x+1;}
int z405(int x){return x+2;}
int z406(int x){return x+3;}
int z407(int x){return x+4;}
int z408(int x){return x+5;}
int z409(int x){return x+6;}
int z410(int x){return x+7;}
int z411(int x){return x+8;}
int z412(int x){return x+9;}
int z413(int x){return x+10;}
int z414(int x){return x+11;}
int z415(int x){return x+12;}
int z416(int x){return x+0;}
int z417(int x){return x+1;}
int z418(int x){return x+2;}
int z419(int x){return x+3;}
int z420(int x){return x+4;}
int z421(int x){return x+5;}
int z422(int x){return x+6;}
int z423(int x){return x+7;}
int z424(int x){return x+8;}
int z425(int x){return x+9;}
int z426(int x){return x+10;}
int z427(int x){return x+11;}
int z428(int x){return x+12;}
int z429(int x){return x+0;}
int z430(int x){return x+1;}
int z431(int x){return x+2;}
int z432(int x){return x+3;}
int z433(int x){return x+4;}
int z434(int x){return x+5;}
int z435(int x){return x+6;}
int z436(int x){return x+7;}
int z437(int x){return x+8;}
int z438(int x){return x+9;}
int z439(int x){return x+10;}
int z440(int x){return x+11;}
int z441(int x){return x+12;}
int z442(int x){return x+0;}
int z443(int x){return x+1;}
int z444(int x){return x+2;}
int z445(int x){return x+3;}
int z446(int x){return x+4;}
int z447(int x){return x+5;}
int z448(int x){return x+6;}
int z449(int x){return x+7;}
int z450(int x){return x+8;}
int z451(int x){return x+9;}
int z452(int x){return x+10;}
int z453(int x){return x+11;}
int z454(int x){return x+12;}
int z455(int x){return x+0;}
int z456(int x){return x+1;}
int z457(int x){return x+2;}
int z458(int x){return x+3;}
int z459(int x){return x+4;}
int z460(int x){return x+5;}
int z461(int x){return x+6;}
int z462(int x){return x+7;}
int z463(int x){return x+8;}
int z464(int x){return x+9;}
int z465(int x){return x+10;}
int z466(int x){return x+11;}
int z467(int x){return x+12;}
int z468(int x){return x+0;}
int z469(int x){return x+1;}
int z470(int x){return x+2;}
int z471(int x){return x+3;}
int z472(int x){return x+4;}
int z473(int x){return x+5;}
int z474(int x){return x+6;}
int z475(int x){return x+7;}
int z476(int x){return x+8;}
int z477(int x){return x+9;}
int z478(int x){return x+10;}
int z479(int x){return x+11;}
int z480(int x){return x+12;}
int z481(int x){return x+0;}
int z482(int x){return x+1;}
int z483(int x){return x+2;}
int z484(int x){return x+3;}
int z485(int x){return x+4;}
int z486(int x){return x+5;}
int z487(int x){return x+6;}
int z488(int x){return x+7;}
int z489(int x){return x+8;}
int z490(int x){return x+9;}
int z491(int x){return x+10;}
int z492(int x){return x+11;}
int z493(int x){return x+12;}
int z494(int x){return x+0;}
int z495(int x){return x+1;}
int z496(int x){return x+2;}
int z497(int x){return x+3;}
int z498(int x){return x+4;}
int z499(int x){return x+5;}
int z500(int x){return x+6;}
int z501(int x){return x+7;}
int z502(int x){return x+8;}
int z503(int x){return x+9;}
int z504(int x){return x+10;}
int z505(int x){return x+11;}
int z506(int x){return x+12;}
int z507(int x){return x+0;}
int z508(int x){return x+1;}
int z509(int x){return x+2;}
int z510(int x){return x+3;}
int z511(int x){return x+4;}
int z512(int x){return x+5;}
int z513(int x){return x+6;}
int z514(int x){return x+7;}
int z515(int x){return x+8;}
int z516(int x){return x+9;}
int z517(int x){return x+10;}
int z518(int x){return x+11;}
int z519(int x){return x+12;}
int z520(int x){return x+0;}
int z521(int x){return x+1;}
int z522(int x){return x+2;}
int z523(int x){return x+3;}
int z524(int x){return x+4;}
int z525(int x){return x+5;}
int z526(int x){return x+6;}
int z527(int x){return x+7;}
int z528(int x){return x+8;}
int z529(int x){return x+9;}
int z530(int x){return x+10;}
int z531(int x){return x+11;}
int z532(int x){return x+12;}
int z533(int x){return x+0;}
int z534(int x){return x+1;}
int z535(int x){return x+2;}
int z536(int x){return x+3;}
int z537(int x){return x+4;}
int z538(int x){return x+5;}
int z539(int x){return x+6;}
int z540(int x){return x+7;}
int z541(int x){return x+8;}
int z542(int x){return x+9;}
int z543(int x){return x+10;}
int z544(int x){return x+11;}
int z545(int x){return x+12;}
int z546(int x){return x+0;}
int z547(int x){return x+1;}
int z548(int x){return x+2;}
int z549(int x){return x+3;}
int z550(int x){return x+4;}
int z551(int x){return x+5;}
int z552(int x){return x+6;}
int z553(int x){return x+7;}
int z554(int x){return x+8;}
int z555(int x){return x+9;}
int z556(int x){return x+10;}
int z557(int x){return x+11;}
int z558(int x){return x+12;}
int z559(int x){return x+0;}
int z560(int x){return x+1;}
int z561(int x){return x+2;}
int z562(int x){return x+3;}
int z563(int x){return x+4;}
int z564(int x){return x+5;}
int z565(int x){return x+6;}
int z566(int x){return x+7;}
int z567(int x){return x+8;}
int z568(int x){return x+9;}
int z569(int x){return x+10;}
int z570(int x){return x+11;}
int z571(int x){return x+12;}
int z572(int x){return x+0;}
int z573(int x){return x+1;}
int z574(int x){return x+2;}
int z575(int x){return x+3;}
int z576(int x){return x+4;}
int z577(int x){return x+5;}
int z578(int x){return x+6;}
int z579(int x){return x+7;}
int z580(int x){return x+8;}
int z581(int x){return x+9;}
int z582(int x){return x+10;}
int z583(int x){return x+11;}
int z584(int x){return x+12;}
int z585(int x){return x+0;}
int z586(int x){return x+1;}
int z587(int x){return x+2;}
int z588(int x){return x+3;}
int z589(int x){return x+4;}
int z590(int x){return x+5;}
int z591(int x){return x+6;}
int z592(int x){return x+7;}
int z593(int x){return x+8;}
int z594(int x){return x+9;}
int z595(int x){return x+10;}
int z596(int x){return x+11;}
int z597(int x){return x+12;}
int z598(int x){return x+0;}
int z599(int x){return x+1;}
int z600(int x){return x+2;}
int z601(int x){return x+3;}
int z602(int x){return x+4;}
int z603(int x){return x+5;}
int z604(int x){return x+6;}
int z605(int x){return x+7;}
int z606(int x){return x+8;}
int z607(int x){return x+9;}
int z608(int x){return x+10;}
int z609(int x){return x+11;}
int z610(int x){return x+12;}
int z611(int x){return x+0;}
int z612(int x){return x+1;}
int z613(int x){return x+2;}
int z614(int x){return x+3;}
int z615(int x){return x+4;}
int z616(int x){return x+5;}
int z617(int x){return x+6;}
int z618(int x){return x+7;}
int z619(int x){return x+8;}
int z620(int x){return x+9;}
int z621(int x){return x+10;}
int z622(int x){return x+11;}
int z623(int x){return x+12;}
int z624(int x){return x+0;}
int z625(int x){return x+1;}
int z626(int x){return x+2;}
int z627(int x){return x+3;}
int z628(int x){return x+4;}
int z629(int x){return x+5;}
int z630(int x){return x+6;}
int z631(int x){return x+7;}
int z632(int x){return x+8;}
int z633(int x){return x+9;}
int z634(int x){return x+10;}
int z635(int x){return x+11;}
int z636(int x){return x+12;}
int z637(int x){return x+0;}
int z638(int x){return x+1;}
int z639(int x){return x+2;}
int z640(int x){return x+3;}
int z641(int x){return x+4;}
int z642(int x){return x+5;}
int z643(int x){return x+6;}
int z644(int x){return x+7;}
int z645(int x){return x+8;}
int z646(int x){return x+9;}
int z647(int x){return x+10;}
int z648(int x){return x+11;}
int z649(int x){return x+12;}
int z650(int x){return x+0;}
int z651(int x){return x+1;}
int z652(int x){return x+2;}
int z653(int x){return x+3;}
int z654(int x){return x+4;}
int z655(int x){return x+5;}
int z656(int x){return x+6;}
int z657(int x){return x+7;}
int z658(int x){return x+8;}
int z659(int x){return x+9;}
int z660(int x){return x+10;}
int z661(int x){return x+11;}
int z662(int x){return x+12;}
int z663(int x){return x+0;}
int z664(int x){return x+1;}
int z665(int x){return x+2;}
int z666(int x){return x+3;}
int z667(int x){return x+4;}
int z668(int x){return x+5;}
int z669(int x){return x+6;}
int z670(int x){return x+7;}
int z671(int x){return x+8;}
int z672(int x){return x+9;}
int z673(int x){return x+10;}
int z674(int x){return x+11;}
int z675(int x){return x+12;}
int z676(int x){return x+0;}
int z677(int x){return x+1;}
int z678(int x){return x+2;}
int z679(int x){return x+3;}
int z680(int x){return x+4;}
int z681(int x){return x+5;}
int z682(int x){return x+6;}
int z683(int x){return x+7;}
int z684(int x){return x+8;}
int z685(int x){return x+9;}
int z686(int x){return x+10;}
int z687(int x){return x+11;}
int z688(int x){return x+12;}
int z689(int x){return x+0;}
int z690(int x){return x+1;}
int z691(int x){return x+2;}
int z692(int x){return x+3;}
int z693(int x){return x+4;}
int z694(int x){return x+5;}
int z695(int x){return x+6;}
int z696(int x){return x+7;}
int z697(int x){return x+8;}
int z698(int x){return x+9;}
int z699(int x){return x+10;}
int z700(int x){return x+11;}
int z701(int x){return x+12;}
int z702(int x){return x+0;}
int z703(int x){return x+1;}
int z704(int x){return x+2;}
int z705(int x){return x+3;}
int z706(int x){return x+4;}
int z707(int x){return x+5;}
int z708(int x){return x+6;}
int z709(int x){return x+7;}
int z710(int x){return x+8;}
int z711(int x){return x+9;}
int z712(int x){return x+10;}
int z713(int x){return x+11;}
int z714(int x){return x+12;}
int z715(int x){return x+0;}
int z716(int x){return x+1;}
int z717(int x){return x+2;}
int z718(int x){return x+3;}
int z719(int x){return x+4;}
int z720(int x){return x+5;}
int z721(int x){return x+6;}
int z722(int x){return x+7;}
int z723(int x){return x+8;}
int z724(int x){return x+9;}
int z725(int x){return x+10;}
int z726(int x){return x+11;}
int z727(int x){return x+12;}
int z728(int x){return x+0;}
int z729(int x){return x+1;}
int z730(int x){return x+2;}
int z731(int x){return x+3;}
int z732(int x){return x+4;}
int z733(int x){return x+5;}
int z734(int x){return x+6;}
int z735(int x){return x+7;}
int z736(int x){return x+8;}
int z737(int x){return x+9;}
int z738(int x){return x+10;}
int z739(int x){return x+11;}
int z740(int x){return x+12;}
int z741(int x){return x+0;}
int z742(int x){return x+1;}
int z743(int x){return x+2;}
int z744(int x){return x+3;}
int z745(int x){return x+4;}
int z746(int x){return x+5;}
int z747(int x){return x+6;}
int z748(int x){return x+7;}
int z749(int x){return x+8;}
int z750(int x){return x+9;}
int z751(int x){return x+10;}
int z752(int x){return x+11;}
int z753(int x){return x+12;}
int z754(int x){return x+0;}
int z755(int x){return x+1;}
int z756(int x){return x+2;}
int z757(int x){return x+3;}
int z758(int x){return x+4;}
int z759(int x){return x+5;}
int z760(int x){return x+6;}
int z761(int x){return x+7;}
int z762(int x){return x+8;}
int z763(int x){return x+9;}
int z764(int x){return x+10;}
int z765(int x){return x+11;}
int z766(int x){return x+12;}
int z767(int x){return x+0;}
int z768(int x){return x+1;}
int z769(int x){return x+2;}
int z770(int x){return x+3;}
int z771(int x){return x+4;}
int z772(int x){return x+5;}
int z773(int x){return x+6;}
int z774(int x){return x+7;}
int z775(int x){return x+8;}
int z776(int x){return x+9;}
int z777(int x){return x+10;}
int z778(int x){return x+11;}
int z779(int x){return x+12;}
int z780(int x){return x+0;}
int z781(int x){return x+1;}
int z782(int x){return x+2;}
int z783(int x){return x+3;}
int z784(int x){return x+4;}
int z785(int x){return x+5;}
int z786(int x){return x+6;}
int z787(int x){return x+7;}
int z788(int x){return x+8;}
int z789(int x){return x+9;}
int z790(int x){return x+10;}
int z791(int x){return x+11;}
int z792(int x){return x+12;}
int z793(int x){return x+0;}
int z794(int x){return x+1;}
int z795(int x){return x+2;}
int z796(int x){return x+3;}
int z797(int x){return x+4;}
int z798(int x){return x+5;}
int z799(int x){return x+6;}
