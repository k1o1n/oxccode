A. OXC Ports Arrangement Problem

time limit per test: 5 seconds

memory limit per test: 1024 megabytes

input: standard input

output: standard output

An intelligent computing cluster innovatively adopts OXC (optical circuit switch) as the core layer to enable ultra-large-scale interconnection for tens of thousands of computing cards.

While OXC demonstrates advantages in latency, power efficiency, and bandwidth compared to electrical switches, its switching capability degrades from all-to-all to one-to-one. This makes it challenging to fully utilize OXC bandwidth.

Please develop a strategy to determine the optimal OXC configuration and routing schemes in response to AI tasks in real-time.

**All links appearing in this problem can be considered as bidirectional edges.**

What is OXC?

![](https://espresso.codeforces.com/b4787c58d396cef3e702941fb9ad5763c7ae7e7b.png)

OXC can freely establish an optical connection between ports by rotating micro-mirrors to reflect light. An **OXC optical connection** refers to a connection between one internal port and another internal port of the OXC, forming a bidirectional pathway.

In the illustrated OXC, one optical connection is established between ports 0 and 2, while another connects ports 1 and 4. This allows the device connected to port 0 to have a communication link with the device connected to port 2, and similarly for ports 1 and 4. **Note that optical connections are one-to-one!**

![](https://espresso.codeforces.com/ddb9b80be4c85ceab93c125c2cb9c12a1052a926.png)

In the network scenario shown in the figure above, OXC #0 establishes an optical connection between port 0 and port 2, creating a communication link between Spine #0 and Spine #1. By establishing an optical connection between port 1 and port 5, it creates a communication link between Spine #0 and Spine #2.

Overview of Two-Tier Electrical and One-Tier Optical Network

![](https://espresso.codeforces.com/8913b15acb71eb0e8544c6fec19382c016ab1921.png)

A Group contains two tiers of electrical switches (referred to as Leaf and Spine, as shown in the figure) and several NPUs (Neural Processing Units). Communication between NPUs within a Group can be completed solely through the electrical switches.

The OXC is only used to route traffic between different Groups. Such traffic is transmitted via the following path: NPU – Leaf – Spine – OXC – Spine – Leaf – NPU.

For each Spine, let $\mathit{Up}$ be the number of links to OXCs and $\mathit{Down}$ be the number of links to Leaves. We define the **convergence ratio** as $\mathit{Up} : \mathit{Down}$. It is guaranteed that this ratio is $1:1$, $1:3$, or $1:7$.

Introduction to Multi-Plane Network

![](https://espresso.codeforces.com/3169f77db7e0f26ac179e1108b19a4fb26316765.png)

-   Spines and OXCs are divided into several independent plane sets (in the industry, this is done to scale the network size).
-   In each plane, between each Spine and each OXC belonging to this plane, there are exactly $K$ links.
-   There are no links between Spines and OXCs in different planes.
-   Within each Group, there is exactly one physical link between each Leaf and each Spine.

Specific Network Description

-   There are $N$ Groups numbered from $0$ to $N-1$.
-   There are $M$ OXCs numbered from $0$ to $M-1$. OXC number $i$ belongs to the plane numbered $\left\lfloor \frac{i}{M / P} \right\rfloor$.
-   Each Group has $S$ Spines numbered from $0$ to $S - 1$. Spine number $i$ belongs to the plane numbered $\left\lfloor \frac{i}{S / P} \right\rfloor$.
-   Each Group has $L$ Leaves, numbered from $0$ to $L - 1$.
-   Within the same plane, each OXC and each Spine have exactly $K$ links, numbered from $0$ to $K-1$.
-   There are $P$ planes numbered from $0$ to $P - 1$. The number of OXCs $M$ and the number of Spines $S$ are both divisible by $P$.
-   From this, it follows that the number of ports per OXC is $R = N \cdot (S / P) \cdot K$. The ports of each OXC are numbered from $0$ to $R - 1$.
    
    The port corresponding to OXC number $m$, Group number $i$, Spine number $j$, and link number $k$ has the number $i \cdot (S / P) \cdot K + (j \bmod (S / P)) \cdot K + k$.
    
    For OXC number $m$, port number $i$ corresponds to Group number $\left\lfloor \frac{i}{(S / P) \cdot K} \right\rfloor$, Spine number $\left\lfloor \frac{i \bmod ((S / P) \cdot K)}{K} \right\rfloor + \left\lfloor \frac{m}{M / P} \right\rfloor \cdot (S / P)$, and link number $(i \bmod K)$.
    

Leaf-Level Communication Flow Demand

Each AI task has different communication requirements between NPU cards. For simplicity, details such as NPU – Leaf connections are abstracted away in this problem.

Specifically, each query can be considered as several bidirectional flows. Each flow goes between some two leaves, LeafA and LeafB.

A flow is transmitted via a bidirectional path: LeafA – SpineA – OXC – SpineB – LeafB. It is guaranteed that, for all given flows, the Group numbers of LeafA and LeafB are different.

Now it is required to assign routes to all flows. We define the **maximum flow conflict** as the maximum number of flows passing through any link in the network.

**Minimize the maximum flow conflict as much as possible.**

OXC Physical Topology Adjustment

The OXC can freely adjust its physical topology, but adjustments incur overhead.

We define the **OXC adjustment cost** as a kind of edit distance between the target physical topology and the current physical topology. Specifically, we adjust an OXC by performing the following actions, each of which has a cost of $1$:

-   Establish a new connection between a pair of ports.
-   Remove an existing connection between a pair of ports.

**Minimize the OXC adjustment cost as much as possible.**

Initially, before any queries, no ports of any OXC have established connections. Before each query after the first one, the physical topology is exactly as it was after the previous query.

Here is an example of edit distance calculation:

-   Original topology: Port 1 is connected to port 2, port 0 is idle and unconnected.
-   New topology: Port 1 is connected to port 0, port 2 is idle and unconnected.
-   Removing the connection between port 1 and port 2, and establishing the connection between port 1 and port 0, results in an OXC adjustment cost of 2.

**Input**

The first line contains three positive integers $N$, $S$, and $L$, representing the number of Groups, the number of Spines per Group, and the number of Leaves per Group, respectively ($2 \le N \le 2^{5}$; $1 \le S \le 2^{5}$; $1 \le L \le 2^{6}$; additionally, $2 \le N \cdot S \cdot L \le 2^{14}$).

The second line contains three positive integers $M$, $K$, and $P$, representing the number of OXCs, the number of links between each OXC and each Spine within the same plane, and the number of planes ($1 \le M \le 2^{8}$; $1 \le K \le 2$; $1 \le P \le 2^{4}$).

Then five queries are given. For each query, the input format is as follows:

-   The first line of a description contains a positive integer $Q$, indicating the number of bidirectional flow demands ($1 \le Q \le \frac{1}{2} (N \cdot S \cdot L)$).
-   Each of the next $Q$ lines describes a bidirectional flow demand between LeafA and LeafB. The $i$\-th line, representing the $i$\-th bidirectional flow demand, contains four integers, in order:
    
    1.  $g_A$, number of Group of LeafA
    2.  LeafA number
    3.  $g_B$, number of Group of LeafB
    4.  LeafB number
    
    The group numbers satisfy $0 \le g_A &lt; g_B \le N - 1$, and the leaf numbers are from $0$ to $L - 1$.
    

It is guaranteed that $S$ and $M$ are divisible by $P$, and that $N - 1 \leq S \cdot (M / P) \cdot K$.

It is guaranteed that the **convergence ratio** $\frac{(M / P) \cdot K}{L}$ is either $1:1$ or $1:3$ or $1:7$.

It is guaranteed that each leaf appears no more than $S$ times in each query.

**Scoring**

Maximize the score as much as possible.

$$
\text{SCORE} = \sum_{\text{all tests}}\sum_{\text{all queries}}\frac{\alpha}{\text{maximum flow conflict} \cdot \text{convergence ratio}} + \beta \cdot \left(1 - \frac{\text{OXC adjustment cost}}{M \cdot R}\right)
$$

Where $\alpha = 1000$ and $\beta = 300$.


```
Example
2 1 2
2 1 1
2
0 1 1 0
0 0 1 1
2
0 0 1 0
0 1 1 1
2
0 1 1 0
0 0 1 1
2
0 0 1 0
0 1 1 1
2
0 0 1 0
0 1 1 1
Output
1 0 
1 0 
0 0 1 0 0
0 0 0 0 0
1 0 
1 0 
0 0 1 0 0
0 0 0 0 0
1 0 
1 0 
0 0 1 0 0
0 0 0 0 0
1 0 
1 0 
0 0 1 0 0
0 0 0 0 0
1 0 
1 0 
0 0 1 0 0
0 0 0 0 0
```