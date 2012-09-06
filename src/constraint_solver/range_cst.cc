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
//
//  Range constraints

#include <stddef.h>
#include <string>

#include "base/logging.h"
#include "constraint_solver/constraint_solver.h"
#include "constraint_solver/constraint_solveri.h"

namespace operations_research {

//-----------------------------------------------------------------------------
// RangeEquality

namespace {
class RangeEquality : public Constraint {
 public:
  RangeEquality(Solver* const s, IntExpr* const l, IntExpr* const r)
      : Constraint(s), left_(l), right_(r) {}

  virtual ~RangeEquality() {}

  virtual void Post() {
    Demon* const d = solver()->MakeConstraintInitialPropagateCallback(this);
    left_->WhenRange(d);
    right_->WhenRange(d);
  }

  virtual void InitialPropagate() {
    left_->SetRange(right_->Min(), right_->Max());
    right_->SetRange(left_->Min(), left_->Max());
  }

  virtual string DebugString() const {
    return left_->DebugString() + " == " + right_->DebugString();
  }

  virtual IntVar* Var() {
    return solver()->MakeIsEqualVar(left_, right_);
  }

  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kEquality, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kEquality, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
};

//-----------------------------------------------------------------------------
// RangeLessOrEqual

class RangeLessOrEqual : public Constraint {
 public:
  RangeLessOrEqual(Solver* const s, IntExpr* const l, IntExpr* const r);
  virtual ~RangeLessOrEqual() {}
  virtual void Post();
  virtual void InitialPropagate();
  virtual string DebugString() const;
  virtual IntVar* Var() {
    return solver()->MakeIsLessOrEqualVar(left_, right_);
  }
  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kLessOrEqual, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kLessOrEqual, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};

RangeLessOrEqual::RangeLessOrEqual(Solver* const s, IntExpr* const l,
                                   IntExpr* const r)
    : Constraint(s), left_(l), right_(r), demon_(NULL) {}

void RangeLessOrEqual::Post() {
  demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
  left_->WhenRange(demon_);
  right_->WhenRange(demon_);
}

void RangeLessOrEqual::InitialPropagate() {
  left_->SetMax(right_->Max());
  right_->SetMin(left_->Min());
  if (left_->Max() <= right_->Min()) {
    demon_->inhibit(solver());
  }
}

string RangeLessOrEqual::DebugString() const {
  return left_->DebugString() + " <= " + right_->DebugString();
}

//-----------------------------------------------------------------------------
// RangeGreaterOrEqual

class RangeGreaterOrEqual : public Constraint {
 public:
  RangeGreaterOrEqual(Solver* const s, IntExpr* const l, IntExpr* const r);
  virtual ~RangeGreaterOrEqual() {}
  virtual void Post();
  virtual void InitialPropagate();
  virtual string DebugString() const;
  virtual IntVar* Var() {
    return solver()->MakeIsGreaterOrEqualVar(left_, right_);
  }
  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kGreaterOrEqual, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kGreaterOrEqual, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};

RangeGreaterOrEqual::RangeGreaterOrEqual(Solver* const s, IntExpr* const l,
                                         IntExpr* const r)
    : Constraint(s), left_(l), right_(r), demon_(NULL) {}

void RangeGreaterOrEqual::Post() {
  demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
  left_->WhenRange(demon_);
  right_->WhenRange(demon_);
}

void RangeGreaterOrEqual::InitialPropagate() {
  left_->SetMin(right_->Min());
  right_->SetMax(left_->Max());
  if (left_->Min() >= right_->Max()) {
    demon_->inhibit(solver());
  }
}

string RangeGreaterOrEqual::DebugString() const {
  return left_->DebugString() + " >= " + right_->DebugString();
}

//-----------------------------------------------------------------------------
// RangeLess

class RangeLess : public Constraint {
 public:
  RangeLess(Solver* const s, IntExpr* const l, IntExpr* const r);
  virtual ~RangeLess() {}
  virtual void Post();
  virtual void InitialPropagate();
  virtual string DebugString() const;
  virtual IntVar* Var() {
    return solver()->MakeIsLessVar(left_, right_);
  }
  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kLess, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kLess, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};

RangeLess::RangeLess(Solver* const s, IntExpr* const l, IntExpr* const r)
    : Constraint(s), left_(l), right_(r), demon_(NULL) {}

void RangeLess::Post() {
  demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
  left_->WhenRange(demon_);
  right_->WhenRange(demon_);
}

void RangeLess::InitialPropagate() {
  left_->SetMax(right_->Max() - 1);
  right_->SetMin(left_->Min() + 1);
  if (left_->Max() < right_->Min()) {
    demon_->inhibit(solver());
  }
}

string RangeLess::DebugString() const {
  return left_->DebugString() + " < " + right_->DebugString();
}

//-----------------------------------------------------------------------------
// RangeGreater

class RangeGreater : public Constraint {
 public:
  RangeGreater(Solver* const s, IntExpr* const l, IntExpr* const r);
  virtual ~RangeGreater() {}
  virtual void Post();
  virtual void InitialPropagate();
  virtual string DebugString() const;
  virtual IntVar* Var() {
    return solver()->MakeIsGreaterVar(left_, right_);
  }
  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kGreater, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kGreater, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};

RangeGreater::RangeGreater(Solver* const s, IntExpr* const l, IntExpr* const r)
    : Constraint(s), left_(l), right_(r), demon_(NULL) {}

void RangeGreater::Post() {
  demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
  left_->WhenRange(demon_);
  right_->WhenRange(demon_);
}

void RangeGreater::InitialPropagate() {
  left_->SetMin(right_->Min() + 1);
  right_->SetMax(left_->Max() - 1);
  if (left_->Min() > right_->Max()) {
    demon_->inhibit(solver());
  }
}

string RangeGreater::DebugString() const {
  return left_->DebugString() + " > " + right_->DebugString();
}

//-----------------------------------------------------------------------------
// DiffVar

class DiffVar : public Constraint {
 public:
  DiffVar(Solver* const s, IntVar* const l, IntVar* const r);
  virtual ~DiffVar() {}
  virtual void Post();
  virtual void InitialPropagate();
  virtual string DebugString() const;
  virtual IntVar* Var() {
    return solver()->MakeIsDifferentVar(left_, right_);
  }

  virtual void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kNonEqual, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->EndVisitConstraint(ModelVisitor::kNonEqual, this);
  }

 private:
  IntVar* const left_;
  IntVar* const right_;
};

DiffVar::DiffVar(Solver* const s, IntVar* const l, IntVar* const r)
  : Constraint(s), left_(l), right_(r) {}

void DiffVar::Post() {
  Demon* d = solver()->MakeConstraintInitialPropagateCallback(this);
  left_->WhenBound(d);
  right_->WhenBound(d);
  // TODO(user) : improve me, separated demons, actually to test
}

void DiffVar::InitialPropagate() {
  if (left_->Bound()) {
    if (right_->Size() < 0xFFFFFF) {
      right_->RemoveValue(left_->Min());  // we use min instead of value
    } else {
      solver()->AddConstraint(solver()->MakeNonEquality(right_, left_->Min()));
    }
  }
  if (right_->Bound()) {
    if (left_->Size() < 0xFFFFFF) {
      left_->RemoveValue(right_->Min());  // see above
    } else {
      solver()->AddConstraint(solver()->MakeNonEquality(left_, right_->Min()));
    }
  }
}

string DiffVar::DebugString() const {
  return left_->DebugString() + " != " + right_->DebugString();
}

// --------------------- Reified API -------------------

// IsEqualCt

class IsEqualCt : public CastConstraint {
 public:
  IsEqualCt(Solver* const s,
            IntExpr* const l,
            IntExpr* const r,
            IntVar* const b)
      : CastConstraint(s, b),
        left_(l),
        right_(r),
        range_demon_(NULL) {}

  virtual ~IsEqualCt() {}

  virtual void Post() {
    range_demon_ =
        MakeConstraintDemon0(solver(),
                             this,
                             &IsEqualCt::PropagateRange,
                             "PropagateRange");
    left_->WhenRange(range_demon_);
    right_->WhenRange(range_demon_);
    Demon* const target_demon =
        MakeConstraintDemon0(solver(),
                             this,
                             &IsEqualCt::PropagateTarget,
                             "PropagateTarget");
    target_var_->WhenBound(target_demon);
  }

  virtual void InitialPropagate() {
    PropagateRange();
    if (!target_var_->Bound() &&
        IsInteresting(left_) &&
        IsInteresting(right_) &&
        std::max(left_->Var()->Size(), right_->Var()->Size()) < 32) {
      IntVar* const lv = left_->Var();
      IntVar* const rv = right_->Var();
      // Search for a support.
      if (lv->Size() < rv->Size()) {
        IntVarIterator* const it = lv->MakeDomainIterator(true);
        for (it->Init(); it->Ok(); it->Next()) {
          if (rv->Contains(it->Value())) {
            return;
          }
        }
        range_demon_->inhibit(solver());
        target_var_->SetValue(0);
      } else {
        IntVarIterator* const it = rv->MakeDomainIterator(true);
        for (it->Init(); it->Ok(); it->Next()) {
          if (lv->Contains(it->Value())) {
            return;
          }
        }
        range_demon_->inhibit(solver());
        target_var_->SetValue(0);
      }
    }
  }

  void PropagateRange() {
    if (target_var_->Bound()) {
      PropagateTarget();
      return;
    }
    if (left_->Min() > right_->Max() || left_->Max() < right_->Min()) {
      target_var_->SetValue(0);
      range_demon_->inhibit(solver());
    } else if (left_->Bound()) {
      if (right_->Bound()) {
        target_var_->SetValue(left_->Min() == right_->Min());
      } else if (right_->IsVar() && !right_->Var()->Contains(left_->Min())) {
        range_demon_->inhibit(solver());
        target_var_->SetValue(0);
      }
    } else if (right_->Bound() &&
               left_->IsVar() &&
               !left_->Var()->Contains(right_->Min())) {
        range_demon_->inhibit(solver());
        target_var_->SetValue(0);
    }
  }

  void PropagateTarget() {
    if (target_var_->Min() == 0) {
      if (left_->Bound()) {
        range_demon_->inhibit(solver());
        solver()->AddConstraint(
            solver()->MakeNonEquality(right_, left_->Min()));
      } else if (right_->Bound()) {
        range_demon_->inhibit(solver());
        solver()->AddConstraint(
            solver()->MakeNonEquality(left_, right_->Min()));
      }
    } else {  // Var is true.
      left_->SetRange(right_->Min(), right_->Max());
      right_->SetRange(left_->Min(), left_->Max());
    }
  }

  string DebugString() const {
    return StringPrintf("IsEqualCt(%s, %s, %s)",
                        left_->DebugString().c_str(),
                        right_->DebugString().c_str(),
                        target_var_->DebugString().c_str());
  }

  void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kIsEqual, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kTargetArgument,
                                            target_var_);
    visitor->EndVisitConstraint(ModelVisitor::kIsEqual, this);
  }

 private:
  bool IsInteresting(IntExpr* const e) const {
    return e->IsVar() && e->Var()->Max() - e->Var()->Min() >= e->Var()->Size();
  }

  IntExpr* const left_;
  IntExpr* const right_;
  Demon* range_demon_;
};

// IsDifferentCt

class IsDifferentCt : public CastConstraint {
 public:
  IsDifferentCt(Solver* const s,
                IntExpr* const l,
                IntExpr* const r,
                IntVar* const b)
      : CastConstraint(s, b),
        left_(l),
        right_(r),
        range_demon_(NULL) {}

  virtual ~IsDifferentCt() {}

  virtual void Post() {
    range_demon_ =
        MakeConstraintDemon0(solver(),
                             this,
                             &IsDifferentCt::PropagateRange,
                             "PropagateRange");
    left_->WhenRange(range_demon_);
    right_->WhenRange(range_demon_);
    Demon* const target_demon =
        MakeConstraintDemon0(solver(),
                             this,
                             &IsDifferentCt::PropagateTarget,
                             "PropagateTarget");
    target_var_->WhenBound(target_demon);
  }

  virtual void InitialPropagate() {
    PropagateRange();
    if (!target_var_->Bound() &&
        IsInteresting(left_) &&
        IsInteresting(right_) &&
        std::max(left_->Var()->Size(), right_->Var()->Size()) < 32) {
      IntVar* const lv = left_->Var();
      IntVar* const rv = right_->Var();
      // Search for a support.
      if (lv->Size() < rv->Size()) {
        IntVarIterator* const it = lv->MakeDomainIterator(true);
        for (it->Init(); it->Ok(); it->Next()) {
          if (rv->Contains(it->Value())) {
            return;
          }
        }
        range_demon_->inhibit(solver());
        target_var_->SetValue(1);
      } else {
        IntVarIterator* const it = rv->MakeDomainIterator(true);
        for (it->Init(); it->Ok(); it->Next()) {
          if (lv->Contains(it->Value())) {
            return;
          }
        }
        range_demon_->inhibit(solver());
        target_var_->SetValue(1);
      }
    }
  }

  void PropagateRange() {
    if (target_var_->Bound()) {
      PropagateTarget();
      return;
    }
    if (left_->Min() > right_->Max() || left_->Max() < right_->Min()) {
      target_var_->SetValue(1);
      range_demon_->inhibit(solver());
    } else if (left_->Bound()) {
      if (right_->Bound()) {
        target_var_->SetValue(left_->Min() != right_->Min());
      } else if (right_->IsVar() && !right_->Var()->Contains(left_->Min())) {
        range_demon_->inhibit(solver());
        target_var_->SetValue(1);
      }
    } else if (right_->Bound() &&
               left_->IsVar() &&
               !left_->Var()->Contains(right_->Min())) {
        range_demon_->inhibit(solver());
        target_var_->SetValue(1);
    }
  }

  void PropagateTarget() {
    if (target_var_->Min() == 0) {
      left_->SetRange(right_->Min(), right_->Max());
      right_->SetRange(left_->Min(), left_->Max());
    } else {  // Var is true.
      if (left_->Bound()) {
        range_demon_->inhibit(solver());
        solver()->AddConstraint(
            solver()->MakeNonEquality(right_, left_->Min()));
      } else if (right_->Bound()) {
        range_demon_->inhibit(solver());
        solver()->AddConstraint(
            solver()->MakeNonEquality(left_, right_->Min()));
      }
    }
  }

  string DebugString() const {
    return StringPrintf("IsDifferentCt(%s, %s, %s)",
                        left_->DebugString().c_str(),
                        right_->DebugString().c_str(),
                        target_var_->DebugString().c_str());
  }

  void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kIsDifferent, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kTargetArgument,
                                            target_var_);
    visitor->EndVisitConstraint(ModelVisitor::kIsDifferent, this);
  }

 private:
  bool IsInteresting(IntExpr* const e) const {
    return e->IsVar() && e->Var()->Max() - e->Var()->Min() >= e->Var()->Size();
  }

  IntExpr* const left_;
  IntExpr* const right_;
  Demon* range_demon_;
};

class IsLessOrEqualCt : public CastConstraint {
 public:
  IsLessOrEqualCt(Solver* const s,
                  IntExpr* const l,
                  IntExpr* const r,
                  IntVar* const b)
      : CastConstraint(s, b),
        left_(l),
        right_(r),
        demon_(NULL) {}

  virtual ~IsLessOrEqualCt() {}

  virtual void Post() {
    demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
    left_->WhenRange(demon_);
    right_->WhenRange(demon_);
    target_var_->WhenBound(demon_);
  }

  virtual void InitialPropagate() {
    if (target_var_->Bound()) {
      if (target_var_->Min() == 0) {
        right_->SetMax(left_->Max() - 1);
        left_->SetMin(right_->Min() + 1);
      } else {  // Var is true.
        right_->SetMin(left_->Min());
        left_->SetMax(right_->Max());
      }
    } else if (right_->Min() >= left_->Max()) {
      demon_->inhibit(solver());
      target_var_->SetValue(1);
    } else if (right_->Max() < left_->Min()) {
      demon_->inhibit(solver());
      target_var_->SetValue(0);
    }
  }

  string DebugString() const {
    return StringPrintf("IsLessOrEqualCt(%s, %s, %s)",
                        left_->DebugString().c_str(),
                        right_->DebugString().c_str(),
                        target_var_->DebugString().c_str());
  }

  void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kIsLessOrEqual, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kTargetArgument,
                                            target_var_);
    visitor->EndVisitConstraint(ModelVisitor::kIsLessOrEqual, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};

class IsLessCt : public CastConstraint {
 public:
  IsLessCt(Solver* const s,
           IntExpr* const l,
           IntExpr* const r,
           IntVar* const b)
      : CastConstraint(s, b),
        left_(l),
        right_(r),
        demon_(NULL) {}

  virtual ~IsLessCt() {}

  virtual void Post() {
    demon_ = solver()->MakeConstraintInitialPropagateCallback(this);
    left_->WhenRange(demon_);
    right_->WhenRange(demon_);
    target_var_->WhenBound(demon_);
  }

  virtual void InitialPropagate() {
    if (target_var_->Bound()) {
      if (target_var_->Min() == 0) {
        right_->SetMax(left_->Max());
        left_->SetMin(right_->Min());
      } else {  // Var is true.
        right_->SetMin(left_->Min() + 1);
        left_->SetMax(right_->Max() - 1);
      }
    } else if (right_->Min() > left_->Max()) {
      demon_->inhibit(solver());
      target_var_->SetValue(1);
    } else if (right_->Max() <= left_->Min()) {
      demon_->inhibit(solver());
      target_var_->SetValue(0);
    }
  }

  string DebugString() const {
    return StringPrintf("IsLessCt(%s, %s, %s)",
                        left_->DebugString().c_str(),
                        right_->DebugString().c_str(),
                        target_var_->DebugString().c_str());
  }

  void Accept(ModelVisitor* const visitor) const {
    visitor->BeginVisitConstraint(ModelVisitor::kIsLess, this);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kLeftArgument, left_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kRightArgument,
                                            right_);
    visitor->VisitIntegerExpressionArgument(ModelVisitor::kTargetArgument,
                                            target_var_);
    visitor->EndVisitConstraint(ModelVisitor::kIsLess, this);
  }

 private:
  IntExpr* const left_;
  IntExpr* const right_;
  Demon* demon_;
};
}  // namespace

Constraint* Solver::MakeEquality(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeEquality(r, l->Min());
  } else if (r->Bound()) {
    return MakeEquality(l, r->Min());
  } else {
    return RevAlloc(new RangeEquality(this, l, r));
  }
}

Constraint* Solver::MakeLessOrEqual(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeGreaterOrEqual(r, l->Min());
  } else if (r->Bound()) {
    return MakeLessOrEqual(l, r->Min());
  } else {
    return RevAlloc(new RangeLessOrEqual(this, l, r));
  }
}

Constraint* Solver::MakeGreaterOrEqual(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeLessOrEqual(r, l->Min());
  } else if (r->Bound()) {
    return MakeGreaterOrEqual(l, r->Min());
  } else {
    return RevAlloc(new RangeGreaterOrEqual(this, l, r));
  }
}

Constraint* Solver::MakeLess(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeGreater(r, l->Min());
  } else if (r->Bound()) {
    return MakeLess(l, r->Min());
  } else {
    return RevAlloc(new RangeLess(this, l, r));
  }
}

Constraint* Solver::MakeGreater(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeLess(r, l->Min());
  } else if (r->Bound()) {
    return MakeGreater(l, r->Min());
  } else {
    return RevAlloc(new RangeGreater(this, l, r));
  }
}

Constraint* Solver::MakeNonEquality(IntExpr* const l, IntExpr* const r) {
  CHECK(l != NULL) << "left expression NULL, maybe a bad cast";
  CHECK(r != NULL) << "left expression NULL, maybe a bad cast";
  CHECK_EQ(this, l->solver());
  CHECK_EQ(this, r->solver());
  if (l->Bound()) {
    return MakeNonEquality(r, l->Min());
  } else if (r->Bound()) {
    return MakeNonEquality(l, r->Min());
  }
  return RevAlloc(new DiffVar(this, l->Var(), r->Var()));
}

IntVar* Solver::MakeIsEqualVar(IntExpr* const v1, IntExpr* const v2) {
  CHECK_EQ(this, v1->solver());
  CHECK_EQ(this, v2->solver());
  if (v1->Bound()) {
    return MakeIsEqualCstVar(v2, v1->Min());
  } else if (v2->Bound()) {
    return MakeIsEqualCstVar(v1, v2->Min());
  }
  IntExpr* const cache = model_cache_->FindExprExprExpression(
      v1,
      v2,
      ModelCache::EXPR_EXPR_IS_EQUAL);
  if (cache != NULL) {
    return cache->Var();
  } else {
    string name1 = v1->name();
    if (name1.empty()) {
      name1 = v1->DebugString();
    }
    string name2 = v2->name();
    if (name2.empty()) {
      name2 = v2->DebugString();
    }
    IntVar* const boolvar = MakeBoolVar(
        StringPrintf("IsEqualVar(%s, %s)", name1.c_str(), name2.c_str()));
    AddConstraint(MakeIsEqualCt(v1, v2, boolvar));
    model_cache_->InsertExprExprExpression(
        boolvar,
        v1,
        v2,
        ModelCache::EXPR_EXPR_IS_EQUAL);
    return boolvar;
  }
}

Constraint* Solver::MakeIsEqualCt(IntExpr* const v1,
                                  IntExpr* const v2,
                                  IntVar* b) {
  CHECK_EQ(this, v1->solver());
  CHECK_EQ(this, v2->solver());
  if (v1->Bound()) {
    return MakeIsEqualCstCt(v2, v1->Min(), b);
  } else if (v2->Bound()) {
    return MakeIsEqualCstCt(v1, v2->Min(), b);
  }
  if (b->Bound()) {
    if (b->Min() == 0) {
      return MakeNonEquality(v1, v2);
    } else {
      return MakeEquality(v1, v2);
    }
  }
  return RevAlloc(new IsEqualCt(this, v1, v2, b));
}

IntVar* Solver::MakeIsDifferentVar(IntExpr* const v1, IntExpr* const v2) {
  CHECK_EQ(this, v1->solver());
  CHECK_EQ(this, v2->solver());
  if (v1->Bound()) {
    return MakeIsDifferentCstVar(v2, v1->Min());
  } else if (v2->Bound()) {
    return MakeIsDifferentCstVar(v1, v2->Min());
  }
  IntExpr* const cache = model_cache_->FindExprExprExpression(
      v1,
      v2,
      ModelCache::EXPR_EXPR_IS_NOT_EQUAL);
  if (cache != NULL) {
    return cache->Var();
  } else {
    string name1 = v1->name();
    if (name1.empty()) {
      name1 = v1->DebugString();
    }
    string name2 = v2->name();
    if (name2.empty()) {
      name2 = v2->DebugString();
    }
    IntVar* const boolvar = MakeBoolVar(
        StringPrintf("IsDifferentVar(%s, %s)", name1.c_str(), name2.c_str()));
    AddConstraint(MakeIsDifferentCt(v1, v2, boolvar));
    model_cache_->InsertExprExprExpression(
        boolvar,
        v1,
        v2,
        ModelCache::EXPR_EXPR_IS_NOT_EQUAL);
    return boolvar;
  }
}

Constraint* Solver::MakeIsDifferentCt(IntExpr* const v1,
                                      IntExpr* const v2,
                                      IntVar* b) {
  CHECK_EQ(this, v1->solver());
  CHECK_EQ(this, v2->solver());
  if (v1->Bound()) {
    return MakeIsDifferentCstCt(v2, v1->Min(), b);
  } else if (v2->Bound()) {
    return MakeIsDifferentCstCt(v1, v2->Min(), b);
  }
  return RevAlloc(new IsDifferentCt(this, v1, v2, b));
}

IntVar* Solver::MakeIsLessOrEqualVar(IntExpr* const left,
                                     IntExpr* const right) {
  CHECK_EQ(this, left->solver());
  CHECK_EQ(this, right->solver());
  if (left->Bound()) {
    return MakeIsGreaterOrEqualCstVar(right, left->Min());
  } else if (right->Bound()) {
    return MakeIsLessOrEqualCstVar(left, right->Min());
  }
  IntExpr* const cache = model_cache_->FindExprExprExpression(
      left,
      right,
      ModelCache::EXPR_EXPR_IS_LESS_OR_EQUAL);
  if (cache != NULL) {
    return cache->Var();
  } else {
    string name1 = left->name();
    if (name1.empty()) {
      name1 = left->DebugString();
    }
    string name2 = right->name();
    if (name2.empty()) {
      name2 = right->DebugString();
    }
    IntVar* const boolvar = MakeBoolVar(
        StringPrintf("IsLessOrEqual(%s, %s)", name1.c_str(), name2.c_str()));

    AddConstraint(RevAlloc(
        new IsLessOrEqualCt(this, left, right, boolvar)));
    model_cache_->InsertExprExprExpression(
        boolvar,
        left,
        right,
        ModelCache::EXPR_EXPR_IS_LESS_OR_EQUAL);
    return boolvar;
  }
}

Constraint* Solver::MakeIsLessOrEqualCt(
    IntExpr* const left, IntExpr* const right, IntVar* const b) {
  CHECK_EQ(this, left->solver());
  CHECK_EQ(this, right->solver());
  if (left->Bound()) {
    return MakeIsGreaterOrEqualCstCt(right, left->Min(), b);
  } else if (right->Bound()) {
    return MakeIsLessOrEqualCstCt(left, right->Min(), b);
  }
  return RevAlloc(new IsLessOrEqualCt(this, left, right, b));
}

IntVar* Solver::MakeIsLessVar(
    IntExpr* const left, IntExpr* const right) {
  CHECK_EQ(this, left->solver());
  CHECK_EQ(this, right->solver());
  if (left->Bound()) {
    return MakeIsGreaterCstVar(right, left->Min());
  } else if (right->Bound()) {
    return MakeIsLessCstVar(left, right->Min());
  }
  IntExpr* const cache = model_cache_->FindExprExprExpression(
      left,
      right,
      ModelCache::EXPR_EXPR_IS_LESS);
  if (cache != NULL) {
    return cache->Var();
  } else {
    string name1 = left->name();
    if (name1.empty()) {
      name1 = left->DebugString();
    }
    string name2 = right->name();
    if (name2.empty()) {
      name2 = right->DebugString();
    }
    IntVar* const boolvar = MakeBoolVar(
        StringPrintf("IsLessOrEqual(%s, %s)", name1.c_str(), name2.c_str()));

    AddConstraint(RevAlloc(
        new IsLessCt(this, left, right, boolvar)));
    model_cache_->InsertExprExprExpression(
        boolvar,
        left,
        right,
        ModelCache::EXPR_EXPR_IS_LESS);
    return boolvar;
  }
}

Constraint* Solver::MakeIsLessCt(
    IntExpr* const left, IntExpr* const right, IntVar* const b) {
  CHECK_EQ(this, left->solver());
  CHECK_EQ(this, right->solver());
  if (left->Bound()) {
    return MakeIsGreaterCstCt(right, left->Min(), b);
  } else if (right->Bound()) {
    return MakeIsLessCstCt(left, right->Min(), b);
  }
  return RevAlloc(new IsLessCt(this, left, right, b));
}

IntVar* Solver::MakeIsGreaterOrEqualVar(
    IntExpr* const left, IntExpr* const right) {
  return MakeIsLessOrEqualVar(right, left);
}

Constraint* Solver::MakeIsGreaterOrEqualCt(
    IntExpr* const left, IntExpr* const right, IntVar* const b) {

  return MakeIsLessOrEqualCt(right, left, b);
}

IntVar* Solver::MakeIsGreaterVar(
    IntExpr* const left, IntExpr* const right) {
  return MakeIsLessVar(right, left);
}

Constraint* Solver::MakeIsGreaterCt(
    IntExpr* const left, IntExpr* const right, IntVar* const b) {
  return MakeIsLessCt(right, left, b);
}

}  // namespace operations_research