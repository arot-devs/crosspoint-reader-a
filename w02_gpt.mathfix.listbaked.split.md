::: typora-export-content
::: {#write}
## Week 02 · Lecture 02 --- Planning by Dynamic Programming {#week-02-·-lecture-02-----planning-by-dynamic-programming}

::: {.md-toc mdtype="toc"}
[[CSC415 -- Introduction to Reinforcement Learning](#csc415----introduction-to-reinforcement-learning){.md-toc-inner} ]{.md-toc-item .md-toc-h1 role="listitem" ref="n1104"}[[Week 02 · Lecture 02 --- Planning by Dynamic Programming](#week-02-·-lecture-02-----planning-by-dynamic-programming){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1106"}[[0. High-Level Summary](#0-high-level-summary){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1109"}[[1. Recap: MRPs, Horizon, Return, and Value](#1-recap-mrps-horizon-return-and-value){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1189"}[[1.1 Horizon and Return](#11-horizon-and-return){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1190"}[[1.2 Markov Reward Process (MRP) and Value Function](#12-markov-reward-process-mrp-and-value-function){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1212"}[[1.3 Bellman Equation for MRPs](#13-bellman-equation-for-mrps){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1224"}[[1.4 Matrix Form and Analytic Solution (MRPs)](#14-matrix-form-and-analytic-solution-mrps){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1254"}[[2. Markov Decision Processes (MDPs)](#2-markov-decision-processes-mdps){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1280"}[[2.1 Definition of an MDP](#21-definition-of-an-mdp){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1281"}[[2.2 Example: Mars Rover MDP](#22-example-mars-rover-mdp){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1303"}[[2.3 Deterministic Policies Count](#23-deterministic-policies-count){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1322"}[[2.4 Policies and Stationarity](#24-policies-and-stationarity){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1334"}[[2.5 Induced MRP Under a Policy](#25-induced-mrp-under-a-policy){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1353"}[[2.6 Value Function Under a Policy](#26-value-function-under-a-policy){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1363"}[[3. Bellman Expectation Equation (MDP Case)](#3-bellman-expectation-equation-mdp-case){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1377"}[[3.1 Bellman Expectation Equation for](#31-bellman-expectation-equation-for-%0Avπ){.md-toc-inner} ]{.md-toc-item .md-toc-h3 role="listitem" ref="n1378"}$V^\pi$[[3.2 Interpretation](#32-interpretation){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1387"}[[4. Optimal Value Function and Optimal Policy](#4-optimal-value-function-and-optimal-policy){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1403"}[[4.1 Optimal Value Function](#41-optimal-value-function){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1404"}[[4.2 Optimal Policy](#42-optimal-policy){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1413"}[[4.3 Bellman Optimality Equation](#43-bellman-optimality-equation){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1436"}[[5. Dynamic Programming (DP)](#5-dynamic-programming-dp){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1455"}[[5.1 What is Dynamic Programming?](#51-what-is-dynamic-programming){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1456"}[[5.2 Requirements for DP](#52-requirements-for-dp){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1470"}[[5.3 DP for Planning (Model-Based)](#53-dp-for-planning-model-based){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1493"}[[6. Iterative Policy Evaluation (Prediction)](#6-iterative-policy-evaluation-prediction){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1524"}[[6.1 Goal](#61-goal){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1525"}[[6.2 Iterative Bellman Expectation Backup](#62-iterative-bellman-expectation-backup){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1531"}[[6.3 Example: Small Gridworld (Random Policy)](#63-example-small-gridworld-random-policy){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1544"}[[6.4 Homework-Style Example: Mars Rover Policy Evaluation](#64-homework-style-example-mars-rover-policy-evaluation){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1581"}[[7. Policy Improvement and Policy Iteration](#7-policy-improvement-and-policy-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1609"}[[7.1 State--Action Value](#71-state--action-value-%0Aqπ){.md-toc-inner} ]{.md-toc-item .md-toc-h3 role="listitem" ref="n1611"}$Q^\pi$[[7.2 Greedy Policy Improvement](#72-greedy-policy-improvement){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1623"}[[7.3 Policy Iteration Algorithm (MDP Policy Iteration)](#73-policy-iteration-algorithm-mdp-policy-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1646"}[[7.4 Monotonic Improvement in Policy](#74-monotonic-improvement-in-policy){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1679"}[[7.5 Think Pair Wise 2: Policy Convergence and Iteration Bound](#75-think-pair-wise-2-policy-convergence-and-iteration-bound){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1703"}[[8. Value Iteration](#8-value-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1728"}[[8.1 Principle of Optimality](#81-principle-of-optimality){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1730"}[[8.2 Bellman Optimality Backup (Deterministic Form)](#82-bellman-optimality-backup-deterministic-form){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1748"}[[8.3 Value Iteration Algorithm](#83-value-iteration-algorithm){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1756"}[[8.4 Example: Shortest Path Intuition](#84-example-shortest-path-intuition){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1782"}[[8.5 Value Iteration vs Policy Iteration](#85-value-iteration-vs-policy-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1795"}[[9. Convergence via Contraction Mapping](#9-convergence-via-contraction-mapping){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1829"}[[9.1 Value Function Space and](#91-value-function-space-and-%0A∞%0A-norm){.md-toc-inner} ]{.md-toc-item .md-toc-h3 role="listitem" ref="n1831"}$\infty$-Norm[[9.2 Bellman Expectation Operator as a Contraction](#92-bellman-expectation-operator-as-a-contraction){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1840"}[[9.3 Contraction Mapping Theorem](#93-contraction-mapping-theorem){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1859"}[[9.4 Convergence of Iterative Policy Evaluation and Policy Iteration](#94-convergence-of-iterative-policy-evaluation-and-policy-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1876"}[[9.5 Bellman Optimality Operator and Convergence of Value Iteration](#95-bellman-optimality-operator-and-convergence-of-value-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1902"}[[10. Asynchronous Dynamic Programming and Extensions](#10-asynchronous-dynamic-programming-and-extensions){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n1926"}[[10.1 Asynchronous DP Basics](#101-asynchronous-dp-basics){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1928"}[[10.2 In-Place Dynamic Programming](#102-in-place-dynamic-programming){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1946"}[[10.3 Prioritized Sweeping](#103-prioritized-sweeping){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1965"}[[10.4 Real-Time Dynamic Programming (RTDP)](#104-real-time-dynamic-programming-rtdp){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n1992"}[[10.5 Full-Width vs Sample Backups](#105-full-width-vs-sample-backups){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2009"}[[10.6 Approximate Dynamic Programming](#106-approximate-dynamic-programming){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2050"}[[10.7 Fitted Value Iteration](#107-fitted-value-iteration){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2062"}[[11. Course Logistics and Learning Goals](#11-course-logistics-and-learning-goals){.md-toc-inner}]{.md-toc-item .md-toc-h2 role="listitem" ref="n2081"}[[11.1 Logistics (from the lecture)](#111-logistics-from-the-lecture){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2082"}[[11.2 "Think Pair Wise" Questions (Conceptual)](#112-think-pair-wise-questions-conceptual){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2113"}[[11.3 What You Should Be Able to Do After This Lecture](#113-what-you-should-be-able-to-do-after-this-lecture){.md-toc-inner}]{.md-toc-item .md-toc-h3 role="listitem" ref="n2136"}
:::

------------------------------------------------------------------------

## 0. High-Level Summary {#0-high-level-summary}

-   Recap of **Markov Reward Processes (MRPs)**:
    -   Horizon, return, value function.
    -   Bellman equation in scalar, matrix, and analytic forms.
-   Introduction/review of **Markov Decision Processes (MDPs)**:
    -   Definition of MDP, policies, induced MRP under a policy.
    -   Value function under a policy and the **Bellman expectation equation**.
-   **Optimality in MDPs**:
    -   Optimal value function $V^*$ and optimal policies $\pi^*$.
    -   **Bellman optimality equation**.
-   **Dynamic Programming (DP)** as a planning method:
    -   Requirements: optimal substructure and overlapping subproblems.
    -   DP for prediction (evaluate a policy) and control (find optimal policy).
-   **Iterative Policy Evaluation**:
    -   Synchronous Bellman expectation backups.
    -   Example: small gridworld; homework-style Mars rover example.
-   **Policy Improvement and Policy Iteration**:
    -   State-action value $Q^\pi(s,a)$.
    -   Greedy policy improvement.
    -   Policy iteration algorithm and **monotonic improvement**.
    -   Finite number of deterministic policies ⇒ finite convergence.
-   **Value Iteration**:
    -   Principle of optimality.
    -   Bellman optimality backups.
    -   Extracting a policy from $V_k$.
    -   Comparison with policy iteration.
-   **Convergence Theory**:
    -   Value function space and $\ell_\infty$ norm.
    -   Bellman operators as $\gamma$-contractions.
    -   Contraction mapping theorem ⇒ uniqueness and convergence to $V^\pi$ or $V^*$.
-   **Asynchronous Dynamic Programming**:
    -   In-place DP, prioritized sweeping, real-time DP.
    -   Full-width vs sample backups.
    -   Approximate DP and fitted value iteration.
-   **Logistics and "what you should know"** for the course and midterm.

------------------------------------------------------------------------

## 1. Recap: MRPs, Horizon, Return, and Value {#1-recap-mrps-horizon-return-and-value}

### 1.1 Horizon and Return {#11-horizon-and-return}

-   **Horizon **$H$\
    Number of time steps in each episode.
    -   **Finite horizon**: $H < \infty$. Sometimes this is called a *finite Markov reward process*.
    -   **Infinite horizon**: $H = \infty$.

-   **Return **$G_t$ (for an MRP)\
    The **discounted cumulative reward** from time $t$ to the horizon:

    ::: {#mathjax-n1201 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1201" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Gt=rt+γrt+1+γ2rt+2+⋯=∑k=0H−1γkrt+k,
    :::
    :::
    :::

    for finite horizon, or the infinite sum when $H = \infty$ and $\gamma < 1$.

-   **Discount factor **$\gamma$ ($0 \le \gamma \le 1$):
    -   Small $\gamma$ → focuses more on **short-term** rewards.
    -   Large $\gamma$ → gives more weight to **long-term** rewards.
    -   **Think Pair Wise 1** (conceptual check):\
        "In an MDP, a large discount factor $\gamma$ means short-term rewards are more influential than long-term rewards."\
        → This statement is **False**. Large $\gamma$ means *long-term* rewards are more influential.

### 1.2 Markov Reward Process (MRP) and Value Function {#12-markov-reward-process-mrp-and-value-function}

-   **MRP** = Markov process + reward, **no actions** yet.
-   **State value function **$V(s)$ (for an MRP):
    -   **Definition (MRP)**:\
        The expected return starting in state $s$:

        ::: {#mathjax-n1221 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1221" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        V(s)=E\[Gt∣St=s\]=E\[rt+γrt+1+γ2rt+2+⋯∣St=s\].
        :::
        :::
        :::
    -   Intuition: "How much total (discounted) reward do I expect if I start from $s$ and then just let the process run?"

### 1.3 Bellman Equation for MRPs {#13-bellman-equation-for-mrps}

-   **Bellman decomposition for MRPs**:
    -   The return can be written as:

        ::: {#mathjax-n1231 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1231" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Gt=Rt+1+γGt+1
        :::
        :::
        :::
    -   Taking expectation from state $s$:

        ::: {#mathjax-n1234 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1234" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        V(s)=E\[Gt∣St=s\]=E\[Rt+1+γGt+1∣St=s\]=E\[Rt+1+γV(St+1)∣St=s\].
        :::
        :::
        :::

-   **Bellman equation (scalar form)**:

    ::: {#mathjax-n1237 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1237" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    V(s)=R(s)+γ∑s′∈SP(s′∣s)V(s′),
    :::
    :::
    :::

    where:

    -   $R(s) = \mathbb{E}[R_{t+1}\mid S_t = s]$ is the expected immediate reward at $s$.
    -   $P(s' \mid s)$ is the transition probability from $s$ to $s'$.

-   **Diagrammatically (backup)**:
    -   You "back up" value from successor states:
        -   **Immediate reward** from leaving $s$.
        -   Plus **discounted** value of the next state.

### 1.4 Matrix Form and Analytic Solution (MRPs) {#14-matrix-form-and-analytic-solution-mrps}

Assume finite state space $\{s_1,\dots,s_N\}$.

-   Stack values and rewards:
    -   ::: {#mathjax-n1261 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1261" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        V=(V(s1)⋮V(sN)),R=(R(s1)⋮R(sN)).
        :::
        :::
        :::

    -   Let $P$ be the $N \times N$ transition matrix:

        ::: {#mathjax-n1264 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1264" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Pij=P(sj∣si).
        :::
        :::
        :::

-   **Matrix Bellman equation**:

    ::: {#mathjax-n1267 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1267" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    V=R+γPV.
    :::
    :::
    :::

-   **Analytic solution** (when possible):

    Starting from $V = R + \gamma P V$:

    ::: {#mathjax-n1271 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1271" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    (I−γP)V=R⇒V=(I−γP)−1R.
    :::
    :::
    :::

    -   Requires $(I - \gamma P)$ to be **invertible**.
    -   Complexity of a naive matrix inverse is $\mathcal{O}(N^3)$.
    -   Works only for **small MRPs**; not realistic for large state spaces (hence we move to iterative methods like DP).

------------------------------------------------------------------------

## 2. Markov Decision Processes (MDPs) {#2-markov-decision-processes-mdps}

### 2.1 Definition of an MDP {#21-definition-of-an-mdp}

An **MDP** is an MRP **with decisions (actions)**.

-   **Definition**: An MDP is a tuple

    ::: {#mathjax-n1286 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1286" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    (S,A,P,R,γ),
    :::
    :::
    :::

    where:

    -   $\mathcal{S}$: finite set of **states**; $s \in \mathcal{S}$.
    -   $\mathcal{A}$: finite set of **actions**; $a \in \mathcal{A}$.
    -   $P$: **transition model**:

        ::: {#mathjax-n1295 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1295" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        P(s′∣s,a)=Pr\[St+1=s′∣St=s,At=a\].
        :::
        :::
        :::
    -   $R$: **reward function**:

        ::: {#mathjax-n1298 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1298" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        R(s,a)=E\[rt∣St=s,At=a\].
        :::
        :::
        :::
    -   $\gamma \in [0,1]$: **discount factor**.

-   The MDP is **Markov**: the future depends only on the current state (and action), not on the full history.

### 2.2 Example: Mars Rover MDP {#22-example-mars-rover-mdp}

-   **States**: 7 discrete locations, $s_1, \dots, s_7$.
-   **Actions**: 2 deterministic actions:
    -   $a_1$: move left.
    -   $a_2$: move right.
-   Dynamics are given by transition matrices $P(\cdot \mid \cdot, a_1)$ and $P(\cdot \mid \cdot, a_2)$:
    -   $a_1$ ("left") matrix: shifts you to the left or stays at the leftmost.
    -   $a_2$ ("right") matrix: shifts to the right or stays at the rightmost.

This is used repeatedly as a running example for policies and DP.

### 2.3 Deterministic Policies Count {#23-deterministic-policies-count}

-   **Question (Think Pair Wise 1)**:\
    7 discrete states, 2 actions $\{L,R\}$. How many deterministic policies?
-   At each state you choose **one** action deterministically.\
    Number of deterministic policies:

    ::: {#mathjax-n1328 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1328" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    \|A\|\|S\|=27.
    :::
    :::
    :::
-   **General fact**: For any finite MDP with $|\mathcal{S}| = n$ and $|\mathcal{A}| = m$,
    -   number of deterministic policies $= m^n$.

### 2.4 Policies and Stationarity {#24-policies-and-stationarity}

-   **Definition (Policy)**\
    A **policy** $\pi$ is a distribution over actions given states:

    ::: {#mathjax-n1338 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1338" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    π(a∣s)=Pr\[At=a∣St=s\].
    :::
    :::
    :::
-   Intuition:
    -   Tells the agent **how to act** in each state.
    -   E.g., in a gridworld, a policy might say "if at this cell, go right; if at that cell, go up".
-   In this course, we typically assume policies are:
    -   **Stationary**: do **not** change with time $t$.
    -   So $A_t \sim \pi(\cdot \mid S_t)$ for all $t$.

### 2.5 Induced MRP Under a Policy {#25-induced-mrp-under-a-policy}

Given an MDP $M = \langle \mathcal{S}, \mathcal{A}, P, R, \gamma \rangle$ and a policy $\pi$:

-   The state sequence $(S_0, S_1, \dots)$ forms a **Markov process** with transition

    ::: {#mathjax-n1358 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1358" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Pπ(s′∣s)=∑aπ(a∣s)P(s′∣s,a).
    :::
    :::
    :::
-   The state--reward sequence $(S_0, R_1, S_1, \dots)$ forms a **Markov reward process**:

    ::: {#mathjax-n1361 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1361" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Rπ(s)=∑aπ(a∣s)R(s,a).
    :::
    :::
    :::

So, under policy $\pi$, the MDP reduces to an MRP $\langle \mathcal{S}, P^\pi, R^\pi, \gamma \rangle$.

### 2.6 Value Function Under a Policy {#26-value-function-under-a-policy}

-   **Definition (State-value function **$V^\pi$)\
    The **state-value function** for policy $\pi$ is:

    ::: {#mathjax-n1367 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1367" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Vπ(s)=Eπ\[Gt∣St=s\],
    :::
    :::
    :::

    where the expectation is taken over trajectories generated by following $\pi$.

-   Interpretations:
    -   "Goodness" of state $s$ **given** that you follow $\pi$ afterward.
    -   Each policy $\pi$ has its **own** value function $V^\pi$.

------------------------------------------------------------------------

## 3. Bellman Expectation Equation (MDP Case) {#3-bellman-expectation-equation-mdp-case}

### 3.1 Bellman Expectation Equation for $V^\pi$ {#31-bellman-expectation-equation-for-\\nvπ}

For an MDP and a policy $\pi$, we can decompose the value function into immediate reward plus discounted value of the successor state:

-   **Bellman expectation equation**:

    ::: {#mathjax-n1383 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1383" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Vπ(s)=∑a∈Aπ(a∣s)\[R(s,a)+γ∑s′∈SP(s′∣s,a)Vπ(s′)\].
    :::
    :::
    :::
-   This is exactly the MRP Bellman equation for the induced process $(P^\pi, R^\pi)$:

    ::: {#mathjax-n1386 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1386" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Vπ=Rπ+γPπVπ.
    :::
    :::
    :::

### 3.2 Interpretation {#32-interpretation}

-   For each state $s$:
    -   You **choose an action** according to $\pi(a\mid s)$.
    -   Receive **immediate reward** $R(s,a)$.
    -   Transition to $s'$ according to $P(s' \mid s,a)$.
    -   Continue to accrue value $V^\pi(s')$ from there.
-   The Bellman equation expresses **self-consistency** of $V^\pi$.

------------------------------------------------------------------------

## 4. Optimal Value Function and Optimal Policy {#4-optimal-value-function-and-optimal-policy}

### 4.1 Optimal Value Function {#41-optimal-value-function}

-   **Definition (Optimal state-value function)**:

    ::: {#mathjax-n1408 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1408" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    V∗(s)=maxπVπ(s).
    :::
    :::
    :::
-   $V^*(s)$ is the **best possible expected return** from state $s$ over *all* policies.
-   An MDP is **"solved"** when we know $V^*$ (and ideally an optimal policy $\pi^*$ achieving it).

### 4.2 Optimal Policy {#42-optimal-policy}

-   **Partial order on policies**:

    ::: {#mathjax-n1417 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1417" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    π≥π′ifVπ(s)≥Vπ′(s)∀s.
    :::
    :::
    :::
-   **Theorem (Existence of optimal policy)**:
    -   For any finite MDP, there exists an **optimal policy** $\pi^*$ such that:

        ::: {#mathjax-n1423 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1423" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        π∗≥π,∀π,
        :::
        :::
        :::

        and

        ::: {#mathjax-n1425 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1425" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vπ∗(s)=V∗(s),∀s.
        :::
        :::
        :::
-   Further facts:
    -   For infinite-horizon discounted MDPs, there always exists an optimal **deterministic**, **stationary** policy.
    -   The **optimal value function **$V^*$ is unique, but:
        -   There may be **multiple optimal policies** $\pi^*$ that achieve the same $V^*$.

### 4.3 Bellman Optimality Equation {#43-bellman-optimality-equation}

-   **Bellman optimality equation**:

    ::: {#mathjax-n1440 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1440" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    V∗(s)=maxa∈A\[R(s,a)+γ∑s′∈SP(s′∣s,a)V∗(s′)\].
    :::
    :::
    :::
-   Interpretation:
    -   The optimal value at $s$ is the **best** you can do over all first actions $a$:
        -   Immediate reward $R(s,a)$,
        -   Plus discounted optimal value of whatever next state $s'$ you land in.
-   Once $V^*$ is known, an optimal policy can be obtained by **acting greedily**:

    ::: {#mathjax-n1453 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1453" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    π∗(s)∈arg⁡maxa∈A\[R(s,a)+γ∑s′P(s′∣s,a)V∗(s′)\].
    :::
    :::
    :::

------------------------------------------------------------------------

## 5. Dynamic Programming (DP) {#5-dynamic-programming-dp}

### 5.1 What is Dynamic Programming? {#51-what-is-dynamic-programming}

-   "Dynamic" = there is a **sequential/temporal** aspect.
-   "Programming" (in Bellman's sense) = **optimizing a program**, i.e., optimizing a policy (not coding).

**Basic idea**:

-   Break a complex problem into **subproblems**.
-   Solve subproblems.
-   **Reuse** those solutions to solve the bigger problem.

### 5.2 Requirements for DP {#52-requirements-for-dp}

DP is applicable to problems with:

1.  **Optimal substructure**
    -   The optimal solution can be **decomposed** into optimal solutions of subproblems.
    -   The **principle of optimality** holds.
2.  **Overlapping subproblems**
    -   Subproblems recur many times.
    -   Solutions can be **cached** (stored) and reused.

For MDPs:

-   Bellman equations provide the recursive decomposition.
-   The value function acts as a **cache** of subproblem solutions.

### 5.3 DP for Planning (Model-Based) {#53-dp-for-planning-model-based}

In this lecture, DP is used for **planning** in an MDP:

-   **Assumption**: We have **full knowledge** of the MDP:
    -   transition model $P(s' \mid s,a)$,
    -   reward function $R(s,a)$,
    -   discount factor $\gamma$.

Two tasks:

1.  **Prediction (Policy Evaluation)**:
    -   Input: MDP $\langle \mathcal{S}, \mathcal{A}, P, R, \gamma \rangle$ and a fixed policy $\pi$.
    -   Or equivalently: MRP $\langle \mathcal{S}, P^\pi, R^\pi, \gamma \rangle$.
    -   Output: Value function $V^\pi$.
2.  **Control (Planning)**:
    -   Input: MDP $\langle \mathcal{S}, \mathcal{A}, P, R, \gamma \rangle$.
    -   Output: Optimal value function $V^*$ and optimal policy $\pi^*$.

------------------------------------------------------------------------

## 6. Iterative Policy Evaluation (Prediction) {#6-iterative-policy-evaluation-prediction}

### 6.1 Goal {#61-goal}

-   **Problem**: Given a policy $\pi$, compute its value function $V^\pi$.
-   **Solution**: Iteratively apply the Bellman expectation backup until convergence.

### 6.2 Iterative Bellman Expectation Backup {#62-iterative-bellman-expectation-backup}

Given initial $V_0$ (often $V_0(s) = 0$ for all $s$), define:

::: {#mathjax-n1533 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1533" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
Vk+1(s)=∑a∈Aπ(a∣s)\[R(s,a)+γ∑s′∈SP(s′∣s,a)Vk(s′)\].
:::
:::
:::

-   **Synchronous backups**:
    -   At each iteration $k \to k+1$, update **all states** $s \in \mathcal{S}$ using the old values $V_k$.
    -   Conceptually: one full sweep over the state space.
-   As $k \to \infty$, under standard conditions, $V_k \to V^\pi$.

### 6.3 Example: Small Gridworld (Random Policy) {#63-example-small-gridworld-random-policy}

-   **Setup**:
    -   Episodic MDP with $\gamma = 1$ (undiscounted).
    -   Nonterminal states: $1, \dots, 14$.
    -   One terminal state (represented twice on corners).
    -   Actions: north, south, east, west.
    -   Actions that would leave the grid keep the agent in the same state.
    -   Reward: $-1$ per time step until terminal is reached.
    -   Policy: **uniform random**:

        ::: {#mathjax-n1563 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1563" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        π(n∣⋅)=π(e∣⋅)=π(s∣⋅)=π(w∣⋅)=0.25.
        :::
        :::
        :::
-   **Procedure**:
    1.  Initialize $V_0(s) = 0$ for all states $s$.
    2.  At each state, apply:

        ::: {#mathjax-n1571 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1571" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vk+1(s)=∑aπ(a∣s)\[−1+∑s′P(s′∣s,a)Vk(s′)\].
        :::
        :::
        :::
    3.  Repeat until $V_{k+1}$ and $V_k$ are sufficiently close.
-   Interpretation:
    -   Values become **more negative** for states far from terminal (long expected time, many $-1$ rewards).
    -   As $k$ increases, the value estimates converge to $V^\pi$.

### 6.4 Homework-Style Example: Mars Rover Policy Evaluation {#64-homework-style-example-mars-rover-policy-evaluation}

**Homework exercise (conceptual)**:

-   Dynamics:
    -   $P(s_6 \mid s_6, a_1) = 0.5$,
    -   $P(s_7 \mid s_6, a_1) = 0.5$,
    -   and similarly defined transitions for other states.
-   Reward:
    -   For all actions, $R(s_1) = +1$, $R(s_7) = +10$, and $0$ otherwise.
-   Policy: $\pi(s) = a_1$ for all $s$.
-   Suppose

    ::: {#mathjax-n1602 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1602" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Vk=\[1,0,0,0,0,0,10\],k=1,γ=0.5.
    :::
    :::
    :::
-   **Compute** $V_{k+1}(s_6)$.

Using Bellman expectation backup (here deterministic policy):

::: {#mathjax-n1606 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1606" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
Vk+1(s6)=R(s6)+γ∑s′P(s′∣s6,a1)Vk(s′)=0+0.5\[0.5⋅Vk(s7)+0.5⋅Vk(s6)\]=0.5\[0.5⋅10+0.5⋅0\]=0.5⋅5=2.5.
:::
:::
:::

(Their worked solution simplifies with $V_k(s_6)=0$; the key is using the Bellman update.)

------------------------------------------------------------------------

## 7. Policy Improvement and Policy Iteration {#7-policy-improvement-and-policy-iteration}

Once we can evaluate $V^\pi$, we can try to **improve** the policy.

### 7.1 State--Action Value $Q^\pi$ {#71-state--action-value-\\nqπ}

-   **Definition (State--action value)**:

    ::: {#mathjax-n1615 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1615" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Qπ(s,a)=R(s,a)+γ∑s′∈SP(s′∣s,a)Vπ(s′).
    :::
    :::
    :::
-   Interpretation:
    -   Take action $a$ once in state $s$, then **follow policy **$\pi$ forever afterwards.
    -   $Q^\pi(s,a)$ evaluates the **effectiveness of action **$a$ under policy $\pi$.

### 7.2 Greedy Policy Improvement {#72-greedy-policy-improvement}

Given $V^\pi$ (or an approximation to it):

-   **Greedy improved policy**:

    ::: {#mathjax-n1628 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1628" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    π′(s)∈arg⁡maxa∈AQπ(s,a)=arg⁡maxa\[R(s,a)+γ∑s′P(s′∣s,a)Vπ(s′)\].
    :::
    :::
    :::
-   Idea:
    -   In state $s$, look at all actions $a$.
    -   Compute $Q^\pi(s,a)$.
    -   Pick action(s) that give the **maximal** $Q^\pi$.
-   **Key property** (policy improvement theorem, informal):
    -   If you construct $\pi'$ greedily from $V^\pi$, then:

        ::: {#mathjax-n1643 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1643" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vπ′(s)≥Vπ(s)∀s.
        :::
        :::
        :::
    -   That is, the new policy is **no worse**, and usually strictly better for at least one state.

### 7.3 Policy Iteration Algorithm (MDP Policy Iteration) {#73-policy-iteration-algorithm-mdp-policy-iteration}

**Algorithm: Policy Iteration (PI)**

1.  Initialize $i = 0$.
2.  Initialize a policy $\pi_0$ arbitrarily (e.g., random actions in each state).
3.  Repeat:
    1.  **Policy Evaluation**:
        -   Compute $V^{\pi_i}$, typically via iterative policy evaluation.
    2.  **Policy Improvement**:
        -   For all $s \in \mathcal{S}$:

            ::: {#mathjax-n1666 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1666" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
            ::: {.md-rawblock-container .md-math-container tabindex="-1"}
            ::: math-text
            πi+1(s)∈arg⁡maxa∈AQπi(s,a).
            :::
            :::
            :::
    3.  Increment $i$.
4.  Stop when $\pi_{i+1} = \pi_i$ (no change in policy).

-   **Stopping condition**:
    -   Use L1 norm $\lVert \pi_{i+1} - \pi_i \rVert_1 = 0$ (or equivalently "no state changed its action").
-   At termination, $\pi_i$ is an **optimal policy** $\pi^*$ and $V^{\pi_i} = V^*$.

### 7.4 Monotonic Improvement in Policy {#74-monotonic-improvement-in-policy}

-   From the policy improvement theorem:

    ::: {#mathjax-n1683 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1683" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    Vπi(s)≤maxaQπi(s,a),
    :::
    :::
    :::

    and greedy improvement sets

    ::: {#mathjax-n1685 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1685" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    πi+1(s)∈arg⁡maxaQπi(s,a).
    :::
    :::
    :::

-   Sketch of **monotone improvement proof**:
    1.  For each state $s$,

        ::: {#mathjax-n1691 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1691" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vπi(s)≤maxaQπi(s,a)=Qπi(s,πi+1(s)).
        :::
        :::
        :::
    2.  Interpreting $Q^{\pi_i}(s,\pi_{i+1}(s))$:
        -   Take one step using $\pi_{i+1}$, then follow $\pi_i$.
    3.  Show that **if you always follow **$\pi_{i+1}$, you get at least as much value as this one-step deviation.
    4.  Conclude that

        ::: {#mathjax-n1701 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1701" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vπi+1(s)≥Vπi(s)∀s.
        :::
        :::
        :::

Thus, PI **monotonically improves** the value function and cannot cycle between distinct policies.

### 7.5 Think Pair Wise 2: Policy Convergence and Iteration Bound {#75-think-pair-wise-2-policy-convergence-and-iteration-bound}

**Questions:**

1.  *If the policy doesn't change at some iteration, can it ever change again?*
    -   Suppose $\pi_{i+1}(s) = \pi_i(s)$ for all $s$.
    -   Then $Q^{\pi_{i+1}}(s,a) = Q^{\pi_i}(s,a)$ for all $s,a$.
    -   Policy improvement step at $i+1$ uses the same $Q$-values and hence returns the same policy:

        ::: {#mathjax-n1715 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1715" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        πi+2(s)=arg⁡maxaQπi+1(s,a)=arg⁡maxaQπi(s,a)=πi+1(s).
        :::
        :::
        :::
    -   So once the policy **stops changing**, it will **never change again**.
2.  *Is there a maximum number of iterations of policy iteration?*
    -   For finite MDP, number of deterministic policies is finite: $|\mathcal{A}|^{|\mathcal{S}|}$.
    -   Each policy iteration step strictly improves the policy unless it is already optimal.
    -   Therefore, PI must terminate after a **finite number of iterations** (no cycles, finite state space of policies).

------------------------------------------------------------------------

## 8. Value Iteration {#8-value-iteration}

Policy iteration alternates between **exact (or nearly exact) evaluation** and **improvement**. Value iteration merges these into a single update.

### 8.1 Principle of Optimality {#81-principle-of-optimality}

-   **Theorem (Principle of Optimality)**:
    -   An optimal policy from state $s$ can be decomposed into:
        1.  An optimal **first action**, and
        2.  An optimal (sub)policy thereafter from the successor state $s'$.
    -   Equivalently:
        -   A policy $\pi$ is optimal from $s$ (i.e., $V^\pi(s) = V^*(s)$) **iff** for any state $s'$ reachable from $s$, $\pi$ is also optimal from $s'$.

This principle underlies the Bellman optimality equation and value iteration.

### 8.2 Bellman Optimality Backup (Deterministic Form) {#82-bellman-optimality-backup-deterministic-form}

If we know the optimal values of successor states $V^*(s')$, then for any state $s$, the optimal value satisfies:

::: {#mathjax-n1750 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1750" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
V∗(s)←maxa∈A\[R(s,a)+γ∑s′∈SP(s′∣s,a)V∗(s′)\].
:::
:::
:::

-   This is the **Bellman optimality backup**.
-   Value iteration idea: **apply this backup repeatedly** even before $V^*$ is known.

### 8.3 Value Iteration Algorithm {#83-value-iteration-algorithm}

**Goal**: Find $V^*$ (and then $\pi^*$).

1.  Initialize:
    -   Choose $V_0(s)$ arbitrarily, often $V_0(s) = 0$ for all $s$.
    -   Set $k = 0$.
2.  **Iterate** until convergence (e.g., $\lVert V_{k+1} - V_k \rVert_\infty \le \varepsilon$):
    -   For each state $s$:

        ::: {#mathjax-n1771 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1771" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vk+1(s)=maxa∈A\[R(s,a)+γ∑s′∈SP(s′∣s,a)Vk(s′)\].
        :::
        :::
        :::
3.  **Policy extraction** (after convergence):
    -   For each $s$,

        ::: {#mathjax-n1777 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1777" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        π∗(s)∈arg⁡maxa∈A\[R(s,a)+γ∑s′∈SP(s′∣s,a)V∗(s′)\],
        :::
        :::
        :::

        where $V^*$ is the limiting value of $V_k$.

-   This is again usually implemented with **synchronous backups**: one full sweep over all states per iteration.

### 8.4 Example: Shortest Path Intuition {#84-example-shortest-path-intuition}

-   In a shortest-path problem with negative cost per step:
    -   Terminal state has value $0$.
    -   States closer to terminal get **higher** values (less negative cost).
    -   Value iteration can be seen as propagating information **backwards** from terminal states across the state space.
-   Even in **loopy** and **stochastic** MDPs, the same backwards propagation intuition holds, but the backup now uses **expectation** over $s'$.

### 8.5 Value Iteration vs Policy Iteration {#85-value-iteration-vs-policy-iteration}

**Comparison**:

<figure>
<table>
<thead>
<tr class="header">
<th><span>Aspect</span></th>
<th><span>Policy Iteration</span></th>
<th><span>Value Iteration</span></th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td><span>Algorithm</span></td>
<td><span>Alternates </span><strong><span>evaluation</span></strong><span> and </span><strong><span>improvement</span></strong></td>
<td><span>Iterates Bellman </span><strong><span>optimality</span></strong><span> updates directly</span></td>
</tr>
<tr class="even">
<td><span>Update</span></td>
<td><span>Solves </span><span class="math inline"><em>V</em><sup><em>π</em></sup></span><span> for a fixed </span><span class="math inline"><em>π</em></span><span> (approximately)</span></td>
<td><span>Updates </span><span class="math inline"><em>V</em>(<em>s</em>)</span><span> using </span><span class="math inline">max<sub><em>a</em></sub></span><span> over actions</span></td>
</tr>
<tr class="odd">
<td><span>Complexity per iter</span></td>
<td><span>Fewer iterations but each evaluation is </span><strong><span>costly</span></strong></td>
<td><span>More iterations but each sweep is </span><strong><span>simpler</span></strong></td>
</tr>
<tr class="even">
<td><span>Output</span></td>
<td><span>Directly improves policy each iteration</span></td>
<td><span>Policy extracted from final </span><span class="math inline"><em>V</em><sup>*</sup></span><span> at the end</span></td>
</tr>
</tbody>
</table>
</figure>

-   Both are based on **state-value** DP ($V^\pi$ or $V^*$).
-   Complexity per sweep (finite state, action spaces):
    -   $O(m n^2)$ where $n = |\mathcal{S}|$ and $m = |\mathcal{A}|$.
    -   Extending to action-values $Q^\pi(s,a)$ raises complexity to $O(m^2 n^2)$.

------------------------------------------------------------------------

## 9. Convergence via Contraction Mapping {#9-convergence-via-contraction-mapping}

The lecture then addresses the **technical question**: why do these iterative methods converge (and to what)?

### 9.1 Value Function Space and $\infty$-Norm {#91-value-function-space-and-\\n∞\\n-norm}

-   Consider the vector space $V = \mathbb{R}^{|\mathcal{S}|}$ of value functions.
-   Each value function $V$ is a vector of dimension $|\mathcal{S}|$.
-   We use the **supremum (infinity) norm** to measure distance:

    ::: {#mathjax-n1839 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1839" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    ‖U−V‖∞=maxs∈S\|U(s)−V(s)\|.
    :::
    :::
    :::

### 9.2 Bellman Expectation Operator as a Contraction {#92-bellman-expectation-operator-as-a-contraction}

Define the **Bellman expectation operator** for a fixed policy $\pi$:

::: {#mathjax-n1842 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1842" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
Tπ(V)=Rπ+γPπV,
:::
:::
:::

where:

-   $R^\pi$ is the vector of $R^\pi(s)$,
-   $P^\pi$ is the transition matrix under policy $\pi$.

We look at the effect of $T^\pi$ on two value functions $U$ and $V$:

::: {#mathjax-n1850 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1850" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
‖Tπ(U)−Tπ(V)‖∞=‖Rπ+γPπU−(Rπ+γPπV)‖∞=‖γPπ(U−V)‖∞≤γ‖Pπ‖∞‖U−V‖∞.
:::
:::
:::

-   Since $P^\pi$ is a stochastic matrix, $\lVert P^\pi \rVert_\infty \le 1$.
-   Hence:

    ::: {#mathjax-n1856 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1856" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    ‖Tπ(U)−Tπ(V)‖∞≤γ‖U−V‖∞.
    :::
    :::
    :::
-   Therefore, $T^\pi$ is a $\gamma$-contraction in the $\infty$-norm.

### 9.3 Contraction Mapping Theorem {#93-contraction-mapping-theorem}

**Theorem (Contraction Mapping Theorem)**:

-   Let $(X, d)$ be a complete metric space.

-   Let $T : X \to X$ be a $\gamma$-contraction, meaning:

    ::: {#mathjax-n1866 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1866" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    d(T(x),T(y))≤γd(x,y)for all x,y∈X,
    :::
    :::
    :::

    with $0 \le \gamma < 1$.

Then:

1.  $T$ has a **unique fixed point** $x^* \in X$ such that $T(x^*) = x^*$.

2.  For any starting point $x_0$, the iterates $x_{k+1} = T(x_k)$ satisfy:

    ::: {#mathjax-n1874 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1874" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    d(xk,x∗)≤γkd(x0,x∗),
    :::
    :::
    :::

    i.e. converge to $x^*$ at a **linear rate** with factor $\gamma$.

### 9.4 Convergence of Iterative Policy Evaluation and Policy Iteration {#94-convergence-of-iterative-policy-evaluation-and-policy-iteration}

-   For a fixed policy $\pi$, $T^\pi$ is a $\gamma$-contraction:
    -   Thus, it has a unique fixed point.
    -   That fixed point is exactly the solution of the Bellman expectation equation:

        ::: {#mathjax-n1885 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1885" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vπ=Tπ(Vπ).
        :::
        :::
        :::
-   **Iterative policy evaluation**:
    -   Repeatedly applying $T^\pi$:

        ::: {#mathjax-n1891 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1891" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vk+1=Tπ(Vk),
        :::
        :::
        :::

        is guaranteed to converge to $V^\pi$, regardless of initialization $V_0$.
-   **Policy iteration**:
    -   Each policy evaluation step converges to $V^{\pi_i}$.
    -   Each policy improvement step strictly improves the policy unless optimal.
    -   With finitely many deterministic policies, PI converges to $V^*$ and some $\pi^*$ in a **finite** number of iterations.

### 9.5 Bellman Optimality Operator and Convergence of Value Iteration {#95-bellman-optimality-operator-and-convergence-of-value-iteration}

Define the **Bellman optimality operator**:

::: {#mathjax-n1904 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1904" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
::: {.md-rawblock-container .md-math-container tabindex="-1"}
::: math-text
T∗(V)(s)=maxa∈A\[R(s,a)+γ∑s′P(s′∣s,a)V(s′)\].
:::
:::
:::

-   Similar to $T^\pi$, we can show $T^*$ is also a $\gamma$-contraction under $\lVert \cdot \rVert_\infty$:

    ::: {#mathjax-n1908 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1908" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    ‖T∗(U)−T∗(V)‖∞≤γ‖U−V‖∞.
    :::
    :::
    :::
-   The contraction mapping theorem implies:
    1.  $T^*$ has a unique fixed point $V^*$:

        ::: {#mathjax-n1914 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1914" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        V∗=T∗(V∗),
        :::
        :::
        :::

        which is precisely the solution to the **Bellman optimality equation**.

    2.  **Value iteration**:

        ::: {#mathjax-n1918 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1918" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        Vk+1=T∗(Vk)
        :::
        :::
        :::

        converges to $V^*$ from any starting point $V_0$.
-   Consequently:
    -   Once value iteration converges, the induced greedy policy (w.r.t. $V^*$) is **optimal**.

------------------------------------------------------------------------

## 10. Asynchronous Dynamic Programming and Extensions {#10-asynchronous-dynamic-programming-and-extensions}

So far, all DP algorithms used **synchronous backups**: every sweep updates **all states** using old values. Asynchronous DP relaxes this.

### 10.1 Asynchronous DP Basics {#101-asynchronous-dp-basics}

-   **Synchronous DP**:
    -   At each iteration, all states are backed up in parallel using $V_k$.
    -   Requires **two copies** of the value function if implemented literally.
-   **Asynchronous DP**:
    -   Updates states **individually**, in **any order**.
    -   For each selected state $s$, apply the appropriate Bellman backup.
    -   Convergence is guaranteed if **every state is updated infinitely often** and backup is consistent.

### 10.2 In-Place Dynamic Programming {#102-in-place-dynamic-programming}

-   **Synchronous VI** (conceptual):
    -   Maintain $v_{\text{old}}$ and $v_{\text{new}}$:

        ::: {#mathjax-n1953 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1953" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        vnew(s)←maxa\[R(s,a)+γ∑s′P(s′∣s,a)vold(s′)\],
        :::
        :::
        :::

        then $v_{\text{old}} \leftarrow v_{\text{new}}$.
-   **In-place VI** (asynchronous-style):
    -   Maintain a **single** value function $v$:

        ::: {#mathjax-n1960 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1960" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        v(s)←maxa\[R(s,a)+γ∑s′P(s′∣s,a)v(s′)\],
        :::
        :::
        :::
    -   Iterate over states in some order (e.g. sweep, or even non-systematic orders).
    -   Uses updated values **immediately** for subsequent updates.

### 10.3 Prioritized Sweeping {#103-prioritized-sweeping}

-   DP backup cost is high; so we want to focus updates on states where it **matters most**.
-   Idea:
    -   Maintain a **priority** for each state, e.g. magnitude of **Bellman error**:

        ::: {#mathjax-n1974 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n1974" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        δ(s)=\|maxa\[R(s,a)+γ∑s′P(s′∣s,a)v(s′)\]−v(s)\|.
        :::
        :::
        :::
    -   Use a **priority queue**:
        -   Always back up the state with **largest residual** first.
    -   After updating a state $s$, recompute priorities for its **predecessors** (states that can reach $s$ under some action).
-   Requirements:
    -   Need **reverse dynamics** (predecessor information): for each state $s$, know which $(\bar s, a)$ can lead to $s$.
-   Intuition:
    -   Large Bellman error ⇒ state's value is far from consistent, so updating it yields big improvement.

### 10.4 Real-Time Dynamic Programming (RTDP) {#104-real-time-dynamic-programming-rtdp}

-   Idea: Focus DP updates on states that are **actually visited** by an agent interacting with the environment.
-   At each time step, after experiencing $(S_t, A_t, R_{t+1}, S_{t+1})$:
    -   Apply a DP backup at $S_t$:

        ::: {#mathjax-n2001 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n2001" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        v(St)←maxa\[R(St,a)+γ∑s′P(s′∣St,a)v(s′)\].
        :::
        :::
        :::
-   RTDP:
    -   Uses the **agent's experience** to choose which states to update.
    -   Only states relevant to actual trajectories are backed up frequently.

### 10.5 Full-Width vs Sample Backups {#105-full-width-vs-sample-backups}

-   **DP backups (full-width)**:
    -   For each update:
        -   Sum over **all actions** and/or **all successor states**.
        -   Use full knowledge of $P$ and $R$.
    -   Complexity per backup grows with number of states and actions.
-   **Curse of dimensionality**:
    -   Number of states $n = |\mathcal{S}|$ often grows **exponentially** in number of underlying state variables.
    -   Even a *single* full-width backup may be too expensive when $n$ is huge.
-   **Sample backups** (used in RL, not pure DP):
    -   Rather than computing expectations exactly, use **samples** $(S,A,R,S')$:
        -   Sample a next state $S'$ and reward $R$ from the environment (or simulator).
    -   Backup is based on sampled transitions, e.g.:

        ::: {#mathjax-n2040 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n2040" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        V(S)←V(S)+α\[R+γV(S′)−V(S)\].
        :::
        :::
        :::
    -   Advantages:
        -   **Model-free**: no need to know $P$ and $R$ explicitly.
        -   **Constant time** per backup, independent of $|\mathcal{S}|$.
        -   Helps break the curse of dimensionality *via sampling*.

### 10.6 Approximate Dynamic Programming {#106-approximate-dynamic-programming}

When $|\mathcal{S}|$ is very large or continuous, storing a table $V(s)$ is impossible. Instead:

-   **Approximate value function**:

    ::: {#mathjax-n2055 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n2055" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    V(s)≈V\^(s;w),
    :::
    :::
    :::

    where $w$ are parameters (e.g. weights of a linear model or neural network).

-   **Approximate DP**:
    -   Apply DP ideas (Bellman backups) in a **function-approximation** setting.

### 10.7 Fitted Value Iteration {#107-fitted-value-iteration}

One concrete approximate DP algorithm:

1.  At each iteration $k$:
    -   Sample a subset of states $\tilde{\mathcal{S}} \subset \mathcal{S}$.
2.  For each $s \in \tilde{\mathcal{S}}$, compute a **Bellman target** using the current approximation $\hat V(\cdot; w_k)$:

    ::: {#mathjax-n2072 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n2072" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
    ::: {.md-rawblock-container .md-math-container tabindex="-1"}
    ::: math-text
    v\~k(s)=maxa∈A\[R(s,a)+γ∑s′P(s′∣s,a)V\^(s′;wk)\].
    :::
    :::
    :::
3.  Fit the function approximator to these targets:
    -   Choose new parameters $w_{k+1}$ to minimize, for example:

        ::: {#mathjax-n2078 .mathjax-block .md-end-block .md-math-block .md-rawblock contenteditable="false" spellcheck="false" cid="n2078" mdtype="math_block" math-tag-before="0" math-tag-after="0" math-labels="[]"}
        ::: {.md-rawblock-container .md-math-container tabindex="-1"}
        ::: math-text
        ∑s∈S\~(V\^(s;wk+1)−v\~k(s))2.
        :::
        :::
        :::

This is **fitted value iteration**: repeatedly compute Bellman targets and regress the function approximator to them.

------------------------------------------------------------------------

## 11. Course Logistics and Learning Goals {#11-course-logistics-and-learning-goals}

### 11.1 Logistics (from the lecture) {#111-logistics-from-the-lecture}

-   **Lab 1**:
    -   First lab: dynamic programming.
    -   Due: **Jan 20**, 11:59pm.
    -   Submission: **MarkUs**, submit your `.ipynb` notebook.
-   **TA office hours**:
    -   New time: **Tuesday 5:15pm--6:15pm**.
    -   Zoom link will be posted on Quercus.
-   **Midterm exam**:
    -   Date: **Jan 29**.
    -   Coverage: first **4 lectures**.
    -   Duration: **90 minutes**.
    -   Will exceed regular tutorial time by **30 minutes**.
    -   Sample questions will be posted on Piazza.

### 11.2 "Think Pair Wise" Questions (Conceptual) {#112-think-pair-wise-questions-conceptual}

-   **Think Pair Wise 1**:
    1.  Discount factor interpretation (large $\gamma$ emphasizes **long-term**, not short-term rewards).
    2.  Number of deterministic policies: $|\mathcal{A}|^{|\mathcal{S}|}$.
-   **Think Pair Wise 2**:
    1.  If policy doesn't change at some iteration, can it ever change again? → No.
    2.  Is there a maximum number of iterations of policy iteration? → Yes, finite, because there are finitely many deterministic policies and policy improvement is monotonic.
-   Extra conceptual prompts for self-study:
    -   Does initialization of $V$ in value iteration affect the **final** solution? (No, but it can affect the **speed** of convergence.)
    -   Does the policy extracted at intermediate steps of value iteration **monotonically** improve when executed in the real infinite-horizon MDP? (Nontrivial; posed as an open question to think about.)

### 11.3 What You Should Be Able to Do After This Lecture {#113-what-you-should-be-able-to-do-after-this-lecture}

-   **Definitions**:
    -   Markov Process (MP).
    -   Markov Reward Process (MRP).
    -   Markov Decision Process (MDP).
    -   Horizon and return.
    -   Bellman equation (MRP and MDP).
    -   Model $(P,R)$.
    -   Policy $\pi$ and optimal policy $\pi^*$.
    -   State value $V^\pi$ and optimal value $V^*$.
    -   State--action value $Q^\pi$ (Q-value).
-   **Algorithms (implement & reason about)**:
    -   Iterative policy evaluation (Bellman expectation).
    -   Policy iteration.
    -   Value iteration.
-   **Conceptual comparisons**:
    -   Pros and cons of different **policy evaluation** approaches (direct matrix solve vs iterative DP).
    -   Differences between **policy iteration** and **value iteration**.
    -   Understand why DP algorithms need a **model** and why they suffer from the **curse of dimensionality**.
-   **Convergence theory**:
    -   Know that Bellman expectation and optimality operators are **contractions** in $\infty$-norm.
    -   Understand that:
        -   Iterative policy evaluation converges to $V^\pi$.
        -   Policy iteration converges to $V^*$.
        -   Value iteration converges to $V^*$.
    -   Recognize the role of the **contraction mapping theorem**.
-   **Limitations and motivation for RL**:
    -   DP is powerful but requires full knowledge of the MDP and full-width backups.
    -   Scalability problems motivate:
        -   Asynchronous DP.
        -   Sample-based methods (Monte Carlo, Temporal Difference learning).
        -   Approximate DP / RL with function approximation.

 
:::
:::
