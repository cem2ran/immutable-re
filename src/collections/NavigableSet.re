/***
 * Copyright (c) 2017 - present Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
type s('set, 'a) = {
  contains: ('a, 'set) => bool,
  count: 'set => int,
  firstOrRaise: 'set => 'a,
  reduce: 'acc .(~while_: ('acc, 'a) => bool, ('acc, 'a) => 'acc, 'acc, 'set) => 'acc,
  toSequence: 'set => Sequence.t('a)
};

type t('a) =
  | Empty
  | Instance('set, s('set, 'a), s('set, 'a)): t('a);

type navigableSet('a) = t('a);

module type SGeneric = {
  type elt('a);
  type t('a);
  include ImmSet.SGeneric with type elt('a) := elt('a) and type t('a) := t('a);
  include NavigableCollection.SGeneric with type elt('a) := elt('a) and type t('a) := t('a);
  let toNavigableSet: t('a) => navigableSet(elt('a));
  let toNavigableSetReversed: t('a) => navigableSet(elt('a));
  let toSetReversed: t('a) => ImmSet.t(elt('a));
};

module type S = {
  type a;
  type t;
  include ImmSet.S with type a := a and type t := t;
  include NavigableCollection.S with type a := a and type t := t;
  let toNavigableSet: t => navigableSet(a);
  let toNavigableSetReversed: t => navigableSet(a);
  let toSetReversed: t => ImmSet.t(a);
};

module type S1 = {
  type t('a);
  include ImmSet.S1 with type t('a) := t('a);
  include NavigableCollection.S1 with type t('a) := t('a);
  let toNavigableSet: t('a) => navigableSet('a);
  let toNavigableSetReversed: t('a) => navigableSet('a);
  let toSetReversed: t('a) => ImmSet.t('a);
};

module type Base = {
  type elt('a);
  type t('a);
  include NavigableCollection.Base with type t('a) := t('a) and type elt('a) := elt('a);
  include ImmSet.Base with type t('a) := t('a) and type elt('a) := elt('a);
};

module MakeGeneric =
       (Base: Base)
       : (SGeneric with type t('a) := Base.t('a) and type elt('a) := Base.elt('a)) => {
  include Base;
  include (
    ImmSet.MakeGeneric(Base): ImmSet.SGeneric with type t('a) := t('a) and type elt('a) := elt('a)
  );
  include (
    NavigableCollection.MakeGeneric(Base):
      NavigableCollection.SGeneric with type t('a) := t('a) and type elt('a) := elt('a)
  );
  module ReversedImmSet =
    ImmSet.MakeGeneric(
      {
        type nonrec elt('a) = elt('a);
        type nonrec t('a) = t('a);
        let contains = contains;
        let count = count;
        let reduce = Base.reduceReversed;
        let toSequence = toSequenceReversed;
      }
    );
  let toSetReversed = ReversedImmSet.toSet;
  let navigableSetBase: s(t('a), elt('a)) = {
    contains,
    count,
    firstOrRaise,
    reduce: Base.reduce,
    toSequence
  };
  let navigableSetReversedBase: s(t('a), elt('a)) = {
    contains,
    count,
    firstOrRaise: lastOrRaise,
    reduce: Base.reduceReversed,
    toSequence: toSequenceReversed
  };
  let toNavigableSet = (set: t('a)) : navigableSet(elt('a)) =>
    if (isEmpty(set)) {
      Empty
    } else {
      Instance(set, navigableSetBase, navigableSetReversedBase)
    };
  let toNavigableSetReversed = (set: t('a)) : navigableSet(elt('a)) =>
    if (isEmpty(set)) {
      Empty
    } else {
      Instance(set, navigableSetReversedBase, navigableSetBase)
    };
};

let contains = (value: 'a, set: t('a)) : bool =>
  switch set {
  | Empty => false
  | Instance(set, {contains}, _) => contains(value, set)
  };

let count = (set: t('a)) : int =>
  switch set {
  | Empty => 0
  | Instance(set, {count}, _) => count(set)
  };

let firstOrRaise = (set: t('a)) : 'a =>
  switch set {
  | Empty => failwith("empty")
  | Instance(set, {firstOrRaise}, _) => firstOrRaise(set)
  };

let lastOrRaise = (set: t('a)) : 'a =>
  switch set {
  | Empty => failwith("empty")
  | Instance(set, _, {firstOrRaise}) => firstOrRaise(set)
  };

let reduce =
    (~while_ as predicate: ('acc, 'a) => bool, f: ('acc, 'a) => 'acc, acc: 'acc, set: t('a))
    : 'acc =>
  switch set {
  | Empty => acc
  | Instance(set, {reduce}, _) => set |> reduce(~while_=predicate, f, acc)
  };

let reduceReversed =
    (~while_ as predicate: ('acc, 'a) => bool, f: ('acc, 'a) => 'acc, acc: 'acc, set: t('a))
    : 'acc =>
  switch set {
  | Empty => acc
  | Instance(set, _, {reduce}) => set |> reduce(~while_=predicate, f, acc)
  };

let toSequence = (set: t('a)) : Sequence.t('a) =>
  switch set {
  | Empty => Sequence.empty()
  | Instance(set, {toSequence}, _) => toSequence(set)
  };

let toSequenceReversed = (set: t('a)) : Sequence.t('a) =>
  switch set {
  | Empty => Sequence.empty()
  | Instance(set, _, {toSequence}) => toSequence(set)
  };

let empty = () : t('a) => Empty;
