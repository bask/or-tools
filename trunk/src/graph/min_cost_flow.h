// Copyright 2010-2012 Google
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// An implementation of a cost-scaling push-relabel algorithm for
// the min-cost flow problem.
//
// In the following, we consider a graph G = (V,E) where V denotes the set
// of nodes (vertices) in the graph, E denotes the set of arcs (edges).
// n = |V| denotes the number of nodes in the graph, and m = |E| denotes the
// number of arcs in the graph.
//
// With each arc (v,w) is associated a nonnegative capacity u(v,w)
// (where 'u' stands for "upper bound") and a unit cost c(v,w). With
// each node v is associated a quantity named supply(v), which
// represents a supply of fluid (if >0) or a demand (if <0).
// Furthermore, no fluid is created in the graph so
//    sum on v in V supply(v) = 0
//
// A flow is a function from E to R such that:
// a) f(v,w) <= u(v,w) for all (v,w) in E (capacity constraint).
// b) f(v,w) = -f(w,v) for all (v,w) in E (flow antisymmetry constraint).
// c) sum on v f(v,w) + supply(w) = 0  (flow conservation).
//
// The cost of a flow is sum on (v,w) in E ( f(v,w) * c(v,w) ) [Note:
// It can be confusing to beginners that the cost is actually double
// the amount that it might seem at first because of flow
// antisymmetry.]
//
// The problem to solve is to find a flow of minimum cost such that all the
// fluid flows from the supply nodes to the demand nodes.
//
// The principles behind this algorithm are the following:
//  1/ handle pseudo-flows instead of flows and refine pseudo-flows until an
// epsilon-optimal minimum-cost flow is obtained,
//  2/ deal with epsilon-optimal pseudo-flows.
//
// 1/ A pseudo-flow is like a flow, except that a node's outflow minus
// its inflow can be different from its supply. If it is the case at a
// given node v, it is said that there is an excess (or deficit) at
// node v. A deficit is denoted by a negative excess and inflow =
// outflow + excess.
// (Look at graph/max_flow.h to see that the definition
// of preflow is more restrictive than the one for pseudo-flow in that a preflow
// only allows non-negative excesses, i.e. no deficit.)
// More formally, a pseudo-flow is a function f such that:
// a) f(v,w) <= u(v,w) for all (v,w) in E  (capacity constraint).
// b) f(v,w) = -f(w,v) for all (v,w) in E (flow antisymmetry constraint).
//
// For each v in E, we also define the excess at node v, the algebraic sum of
// all the incoming preflows at this node, added together with the supply at v.
//    excess(v) = sum on u f(u,v) + supply(v)
//
// The goal of the algorithm is to obtain excess(v) = 0 for all v in V, while
// consuming capacity on some arcs, at the lowest possible cost.
//
// 2/ Internally to the algorithm and its analysis (but invisibly to
// the client), each node has an associated "price" (or potential), in
// addition to its excess. It is formally a function from E to R (the
// set of real numbers.). For a given price function p, the reduced
// cost of an arc (v,w) is:
//    c_p(v,w) = c(v,w) + p(v) - p(w)
// (c(v,w) is the cost of arc (v,w).) For those familiar with linear
// programming, the price function can be viewed as a set of dual
// variables.
//
// For a constant epsilon >= 0, a pseudo-flow f is said to be epsilon-optimal
// with respect to a price function p if for every residual arc (v,w) in E,
//    c_p(v,w) >= -epsilon.
//
// A flow f is optimal if and only if there exists a price function p such that
// no arc is admissible with respect to f and p.
//
// If the arc costs are integers, and epsilon < 1/n, any epsilon-optimal flow
// is optimal. The integer cost case is handled by multiplying all the arc costs
// and the initial value of epsilon by (n+1). When epsilon reaches 1, and
// the solution is epsilon-optimal, it means: for all residual arc (v,w) in E,
//    (n+1) * c_p(v,w) >= -1, thus c_p(v,w) >= -1/(n+1) >= 1/n, and the
// solution is optimal.
//
// A node v is said to be *active* if excess(v) > 0.
// In this case the following operations can be applied to it:
// - if there are *admissible* incident arcs, i.e. arcs which are not saturated,
//   and whose reduced costs are negative, a PushFlow operation can
//   be applied. It consists in sending as  much flow as both the excess at the
//   node and the capacity of the arc permit.
// - if there are no admissible arcs, the active node considered is relabeled,
// This is implemented in Discharge, which itself calls PushFlow and Relabel.
//
// Discharge itself is called by Refine. Refine first saturates all the
// admissible arcs, then builds a stack of active nodes. It then applies
// Discharge for each active node, possibly adding new ones in the process,
// until no nodes are active. In that case an epsilon-optimal flow is obtained.
//
// Optimize iteratively calls Refine, while epsilon > 1, and divides epsilon by
// alpha (set by default to 5) before each iteration.
//
// The algorithm starts with epsilon = C, where C is the maximum absolute value
// of the arc costs. In the integer case which we are dealing with, since all
// costs are multiplied by (n+1), the initial value of epsilon is (n+1)*C.
// The algorithm terminates when epsilon = 1, and Refine() has been called.
// In this case, a minimum-cost flow is obtained.
//
// The complexity of the algorithm is O(n^2*m*log(n*C)) where C is the value of
// the largest arc cost in the graph.
//
// IMPORTANT:
// The algorithm is not able to detect the infeasibility of a problem (when
// there is a bottleneck in the network that forbids to send all the supplies.)
// Worse, it could in some cases loop forever. This is why feasibility checking
// is enabled by default (FLAGS_min_cost_flow_check_feasibility=true.)
// Feasibility checking is implemented using a max-flow, which has a much lower
// complexity. The impact on performance is negligible, while the risk of being
// caught in an endless loop is removed. Note that using the feasibility checker
// roughly doubles the memory consumption.
//
// The starting reference for this class of algorithms is:
// A.V. Goldberg and R.E. Tarjan, "Finding Minimum-Cost Circulations by
// Successive Approximation." Mathematics of Operations Research, Vol. 15,
// 1990:430-466.
// http://portal.acm.org/citation.cfm?id=92225
//
// Implementation issues are tackled in:
// A.V. Goldberg, "An Efficient Implementation of a Scaling Minimum-Cost Flow
// Algorithm," Journal of Algorithms, (1997) 22:1-29
// http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.31.258
//
// A.V. Goldberg and M. Kharitonov, "On Implementing Scaling Push-Relabel
// Algorithms for the Minimum-Cost Flow Problem", Network flows and matching:
// First DIMACS implementation challenge, DIMACS Series in Discrete Mathematics
// and Theoretical Computer Science, (1993) 12:157-198.
// ftp://dimacs.rutgers.edu/pub/netflow/...mincost/scalmin.ps
// and in:
// ﻿U. Bunnagel, B. Korte, and J. Vygen. “Efficient implementation of the
// Goldberg-Tarjan minimum-cost flow algorithm.” Optimization Methods and
// Software (1998) vol. 10, no. 2:157-174.
// http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.84.9897
//
// We have tried as much as possible in this implementation to keep the
// notations and namings of the papers cited above, except for 'demand' or
// 'balance' which have been replaced by 'supply', with the according sign
// changes to better accomodate with the API of the rest of our tools. A demand
// is denoted by a negative supply.
//
// TODO(user): See whether the following can bring any improvements on real-life
// problems.
// R.K. Ahuja, A.V. Goldberg, J.B. Orlin, and R.E. Tarjan, "Finding minimum-cost
// flows by double scaling," Mathematical Programming, (1992) 53:243-266.
// http://www.springerlink.com/index/gu7404218u6kt166.pdf
//
// An interesting general reference on network flows is:
// R. K. Ahuja, T. L. Magnanti, J. B. Orlin, "Network Flows: Theory, Algorithms,
// and Applications," Prentice Hall, 1993, ISBN: 978-0136175490,
// http://www.amazon.com/dp/013617549X
//
// Keywords: Push-relabel, min-cost flow, network, graph, Goldberg, Tarjan,
//           Dinic, Dinitz.

#ifndef OR_TOOLS_GRAPH_MIN_COST_FLOW_H_
#define OR_TOOLS_GRAPH_MIN_COST_FLOW_H_

#include <algorithm>
#include <stack>
#include <string>
#include <vector>

#include "base/integral_types.h"
#include "base/logging.h"
#include "base/macros.h"
#include "graph/ebert_graph.h"
#include "graph/graph.h"
#include "util/stats.h"
#include "util/zvector.h"

using std::string;

namespace operations_research {

// Forward declaration.
template<typename Graph> class GenericMinCostFlow;

// Different statuses for a solved problem.
// We use a base class to share it between our different interfaces.
class MinCostFlowBase {
 public:
  enum Status {
    NOT_SOLVED,
    OPTIMAL,
    FEASIBLE,
    INFEASIBLE,
    UNBALANCED,
    BAD_RESULT,
    BAD_COST_RANGE
  };
};

// A simple and efficient min-cost flow interface. This is as fast as
// GenericMinCostFlow<ReverseArcStaticGraph>, which is the fastest, but is uses
// more memory in order to hide the somewhat involved construction of the
// static graph.
//
// TODO(user): If the need arises, extend this interface to support warm start
// and incrementality between solves. Note that this is already supported by the
// GenericMinCostFlow<> interface.
class SimpleMinCostFlow : public MinCostFlowBase {
 public:
  // The constructor takes no size. New node indices will be created lazily by
  // AddArc() or SetNodeSupply() but all arc indices must be created by AddArc()
  // before beeing used.
  SimpleMinCostFlow();

  // Resets the class to its initial state.
  void Clear();

  // Sets the supply of the given node. Node implicitely created by AddArc()
  // have a default supply set to 0. A demand is modeled as a negative supply.
  void SetNodeSupply(NodeIndex node, FlowQuantity supply);

  // Adds a directed arc from tail to head to the underlying graph.
  // Note that the returned ArcIndex will always be the old NumArcs().
  // By default, this arc will have an unit cost of 0 and a capacity of 1.
  ArcIndex AddArc(NodeIndex tail, NodeIndex head);

  // Sets the unit cost for an existing arc.
  void SetArcUnitCost(ArcIndex arc, CostValue unit_cost);

  // Sets the capacity for an existing arc.
  void SetArcCapacity(ArcIndex arc, FlowQuantity capacity);

  // Solves the problem, and returns the problem status.
  Status Solve();

  // Returns the cost of the minimum-cost flow found by the algorithm.
  CostValue GetOptimalCost() const;

  // Returns the flow on arc, this only make sense for a successful Solve().
  FlowQuantity Flow(ArcIndex arc) const;

  // Accessors for the user given data.
  NodeIndex NumNodes() const;
  ArcIndex NumArcs() const;
  ArcIndex Tail(ArcIndex arc) const;
  ArcIndex Head(ArcIndex arc) const;
  FlowQuantity Capacity(ArcIndex arc) const;
  FlowQuantity UnitCost(ArcIndex arc) const;
  FlowQuantity Supply(NodeIndex node) const;

 private:
  typedef ReverseArcStaticGraph<NodeIndex, ArcIndex> Graph;
  void ResizeNodeVectors(NodeIndex node);

  std::vector<NodeIndex> arc_tail_;
  std::vector<NodeIndex> arc_head_;
  std::vector<FlowQuantity> arc_capacity_;
  std::vector<FlowQuantity> node_supply_;
  std::vector<CostValue> arc_cost_;
  std::vector<ArcIndex> arc_permutation_;
  std::vector<FlowQuantity> arc_flow_;
  CostValue optimal_cost_;

  DISALLOW_COPY_AND_ASSIGN(SimpleMinCostFlow);
};

// Generic MinCostFlow (there is a default MinCostFlow specialization defined
// below) that works with StarGraph and all the graphs handling reverse arcs
// from graph.h, see the end of min_cost_flow.cc for the exact types this class
// is compiled for.
template<typename Graph> class GenericMinCostFlow : public MinCostFlowBase {
 public:
  typedef typename Graph::NodeIndex NodeIndex;
  typedef typename Graph::ArcIndex ArcIndex;
  typedef typename Graph::OutgoingArcIterator OutgoingArcIterator;
  typedef typename Graph::IncidentArcIterator IncidentArcIterator;
  typedef ZVector<ArcIndex> ArcIndexArray;

  // Initialize a MinCostFlow instance on the given graph. The graph does not
  // need to be fully built yet, but its capacity reservation is used to
  // initialize the memory of this class.
  explicit GenericMinCostFlow(const Graph* graph);

  // Returns the graph associated to the current object.
  const Graph* graph() const { return graph_; }

  // Returns the status of last call to Solve(). NOT_SOLVED is returned if
  // Solve() has never been called or if the problem has been modified in such a
  // way that the previous solution becomes invalid.
  Status status() const { return status_; }

  // Sets the supply corresponding to node. A demand is modeled as a negative
  // supply.
  void SetNodeSupply(NodeIndex node, FlowQuantity supply);

  // Sets the unit cost for arc.
  void SetArcUnitCost(ArcIndex arc, CostValue unit_cost);

  // Sets the capacity for arc.
  void SetArcCapacity(ArcIndex arc, FlowQuantity new_capacity);

  // Sets the flow for arc. Note that new_flow must be smaller than the
  // capacity of arc.
  void SetArcFlow(ArcIndex arc, FlowQuantity new_flow);

  // Solves the problem, and returns true if a min-cost flow could be found.
  bool Solve();

  // Checks for feasibility,  i.e. that all the supplies and demands can be
  // matched without exceeding bottlenecks in the network.
  // If infeasible_supply_node (resp. infeasible_demand_node) are not NULL,
  // they are populated with the indices of the nodes where the initial supplies
  // (resp. demands) are too large. Feasible values for the supplies and
  // demands are accessible through FeasibleSupply.
  // Note that CheckFeasibility is called by Solve() when the flag
  // min_cost_flow_check_feasibility is set to true (which is the default.)
  bool CheckFeasibility(std::vector<NodeIndex>* const infeasible_supply_node,
                        std::vector<NodeIndex>* const infeasible_demand_node);

  // Makes the min-cost flow problem solvable by truncating supplies and
  // demands to a level acceptable by the network. There may be several ways to
  // do it. In our case, the levels are computed from the result of the max-flow
  // algorithm run in CheckFeasibility().
  // MakeFeasible returns false if CheckFeasibility() was not called before.
  bool MakeFeasible();

  // Returns the cost of the minimum-cost flow found by the algorithm.
  CostValue GetOptimalCost() const { return total_flow_cost_; }

  // Returns the flow on arc using the equations given in the comment on
  // residual_arc_capacity_.
  FlowQuantity Flow(ArcIndex arc) const;

  // Returns the capacity of an arc.
  FlowQuantity Capacity(ArcIndex arc) const;

  // Returns the unscaled cost for arc.
  FlowQuantity UnitCost(ArcIndex arc) const;

  // Returns the supply at node. Demands are modelled as negative supplies.
  FlowQuantity Supply(NodeIndex node) const;

  // Returns the initial supply at node, given as data.
  FlowQuantity InitialSupply(NodeIndex node) const;

  // Returns the largest supply (if > 0) or largest demand in absolute value
  // (if < 0) admissible at node. If the problem is not feasible, some of these
  // values will be smaller (in absolute value) than than the initial supplies
  // and demand given as input.
  FlowQuantity FeasibleSupply(NodeIndex node) const;

  // Whether to use the UpdatePrices() heuristic.
  void SetUseUpdatePrices(bool value) { use_price_update_ = value; }

 private:
  // Returns true if arc is admissible i.e. if its residual capacity is stricly
  // positive, and its reduced cost stricly negative, i.e. pushing more flow
  // into it will result in a reduction of the total cost.
  bool IsAdmissible(ArcIndex arc) const;
  bool FastIsAdmissible(ArcIndex arc, CostValue tail_potential) const;

  // Returns true if node is active, i.e. if its supply is positive.
  bool IsActive(NodeIndex node) const;

  // Returns the reduced cost for an arc.
  CostValue ReducedCost(ArcIndex arc) const;
  CostValue FastReducedCost(ArcIndex arc, CostValue tail_potential) const;

  // Returns the first incident arc of node.
  ArcIndex GetFirstIncidentArc(NodeIndex node) const;

  // Checks the consistency of the input, i.e. whether the sum of the supplies
  // for all nodes is equal to zero. To be used in a DCHECK.
  bool CheckInputConsistency() const;

  // Checks whether the result is valid, i.e. whether for each arc,
  // residual_arc_capacity_[arc] == 0 || ReducedCost(arc) >= -epsilon_ .
  // (A solution is epsilon-optimal if ReducedCost(arc) >= -epsilon.)
  // To be used in a DCHECK.
  bool CheckResult() const;

  // Checks that the cost range fits in the range of int64's.
  // To be used in a DCHECK.
  bool CheckCostRange() const;

  // Checks the relabel precondition (to be used in a DCHECK):
  // - The node must be active, or have a 0 excess (relaxation for the Push
  //   Look-Ahead heuristic).
  // - The node must have no admissible arcs.
  bool CheckRelabelPrecondition(NodeIndex node) const;

  // Returns context concatenated with information about arc
  // in a human-friendly way.
  string DebugString(const string& context, ArcIndex arc) const;

  // Resets the first_admissible_arc_ array to the first incident arc of each
  // node.
  void ResetFirstAdmissibleArcs();

  // Scales the costs, by multiplying them by (graph_->num_nodes() + 1).
  void ScaleCosts();

  // Unscales the costs, by dividing them by (graph_->num_nodes() + 1).
  void UnscaleCosts();

  // Optimizes the cost by dividing epsilon_ by alpha_ and calling Refine().
  void Optimize();

  // Saturates the admissible arcs, i.e. push as much flow as possible.
  void SaturateAdmissibleArcs();

  // Pushes flow on arc,  i.e. consumes flow on residual_arc_capacity_[arc],
  // and consumes -flow on residual_arc_capacity_[Opposite(arc)]. Updates
  // node_excess_ at the tail and head of arc accordingly.
  void PushFlow(FlowQuantity flow, ArcIndex arc);
  void FastPushFlow(FlowQuantity flow, ArcIndex arc, NodeIndex tail);

  // Initializes the stack active_nodes_.
  void InitializeActiveNodeStack();

  // Price update heuristics as described in A.V. Goldberg, "An Efficient
  // Implementation of a Scaling Minimum-Cost Flow Algorithm," Journal of
  // Algorithms, (1997) 22:1-29
  // http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.31.258
  void UpdatePrices();

  // Performs an epsilon-optimization step by saturating admissible arcs
  // and discharging the active nodes.
  void Refine();

  // Discharges an active node node by saturating its admissible adjacent arcs,
  // if any, and by relabelling it when it becomes inactive.
  void Discharge(NodeIndex node);

  // Part of the Push LookAhead heuristic. When we are about to push on the
  // in_arc, we check that the head (i.e node here) can accept the flow and
  // return true if this is the case:
  // - Returns true if the node excess is < 0.
  // - Returns true if node as an admissible arc at its current potential.
  // - If the two conditions above are false, the node can be relabeled. We
  //   do that and return true if the in_arc is still admissible.
  bool LookAhead(ArcIndex in_arc, CostValue in_tail_potential, NodeIndex node);

  // Relabels node, i.e. decreases its potential while keeping the
  // epsilon-optimality of the pseudo flow. See CheckRelabelPrecondition() for
  // detail on the preconditions.
  void Relabel(NodeIndex node);

  // Handy member functions to make the code more compact.
  NodeIndex Head(ArcIndex arc) const { return graph_->Head(arc); }
  NodeIndex Tail(ArcIndex arc) const { return graph_->Tail(arc); }
  ArcIndex Opposite(ArcIndex arc) const;
  bool IsArcDirect(ArcIndex arc) const;
  bool IsArcValid(ArcIndex arc) const;

  // Pointer to the graph passed as argument.
  const Graph* graph_;

  // An array representing the supply (if > 0) or the demand (if < 0)
  // for each node in graph_.
  QuantityArray node_excess_;

  // An array representing the potential (or price function) for
  // each node in graph_.
  CostArray node_potential_;

  // An array representing the residual_capacity for each arc in graph_.
  // Residual capacities enable one to represent the capacity and flow for all
  // arcs in the graph in the following manner.
  // For all arc, residual_arc_capacity_[arc] = capacity[arc] - flow[arc]
  // Moreover, for reverse arcs, capacity[arc] = 0 by definition.
  // Also flow[Opposite(arc)] = -flow[arc] by definition.
  // Therefore:
  // - for a direct arc:
  //    flow[arc] = 0 - flow[Opposite(arc)]
  //              = capacity[Opposite(arc)] - flow[Opposite(arc)]
  //              = residual_arc_capacity_[Opposite(arc)]
  // - for a reverse arc:
  //    flow[arc] = -residual_arc_capacity_[arc]
  // Using these facts enables one to only maintain residual_arc_capacity_,
  // instead of both capacity and flow, for each direct and indirect arc. This
  // reduces the amount of memory for this information by a factor 2.
  // Note that the sum of the largest capacity of an arc in the graph and of
  // the total flow in the graph not exceed the largest integer representable
  // in 64 bits or there would be errors. CheckInputConsistency() verifies
  // this.
  QuantityArray residual_arc_capacity_;

  // An array representing the first admissible arc for each node in graph_.
  ArcIndexArray first_admissible_arc_;

  // A stack used for managing active nodes in the algorithm.
  // Note that the papers cited above recommend the use of a queue, but
  // benchmarking so far has not proved it is better.
  std::stack<NodeIndex> active_nodes_;

  // epsilon_ is the tolerance for optimality.
  CostValue epsilon_;

  // alpha_ is the factor by which epsilon_ is divided at each iteration of
  // Refine().
  const int64 alpha_;

  // cost_scaling_factor_ is the scaling factor for cost.
  CostValue cost_scaling_factor_;

  // An array representing the scaled unit cost for each arc in graph_.
  QuantityArray scaled_arc_unit_cost_;

  // The total cost of the flow.
  CostValue total_flow_cost_;

  // The status of the problem.
  Status status_;

  // An array containing the initial excesses (i.e. the supplies) for each
  // node. This is used to create the max-flow-based feasibility checker.
  QuantityArray initial_node_excess_;

  // An array containing the best acceptable excesses for each of the
  // nodes. These excesses are imposed by the result of the max-flow-based
  // feasibility checker for the nodes with an initial supply != 0. For the
  // other nodes, the excess is simply 0.
  QuantityArray feasible_node_excess_;

  // Statistics about this class.
  StatsGroup stats_;

  // Number of Relabel() since last UpdatePrices().
  int num_relabels_since_last_price_update_;

  // A Boolean which is true when feasibility has been checked.
  bool feasibility_checked_;

  // Whether to use the UpdatePrices() heuristic.
  bool use_price_update_;

  DISALLOW_COPY_AND_ASSIGN(GenericMinCostFlow);
};

#if !SWIG

// Default MinCostFlow instance that uses StarGraph.
// New clients should use SimpleMinCostFlow if they can.
class MinCostFlow : public GenericMinCostFlow<StarGraph> {
 public:
  explicit MinCostFlow(const StarGraph* graph) : GenericMinCostFlow(graph) {}
};

#endif  // SWIG

}  // namespace operations_research
#endif  // OR_TOOLS_GRAPH_MIN_COST_FLOW_H_