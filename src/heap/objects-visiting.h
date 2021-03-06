// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_VISITING_H_
#define V8_OBJECTS_VISITING_H_

#include "src/allocation.h"
#include "src/heap/heap.h"
#include "src/layout-descriptor.h"
#include "src/objects-body-descriptors.h"
#include "src/objects/string.h"

namespace v8 {
namespace internal {

#define TYPED_VISITOR_ID_LIST(V) \
  V(AllocationSite)              \
  V(ByteArray)                   \
  V(BytecodeArray)               \
  V(Cell)                        \
  V(Code)                        \
  V(ConsString)                  \
  V(FeedbackVector)              \
  V(FixedArray)                  \
  V(FixedDoubleArray)            \
  V(FixedFloat64Array)           \
  V(FixedTypedArrayBase)         \
  V(JSArrayBuffer)               \
  V(JSFunction)                  \
  V(JSObject)                    \
  V(JSRegExp)                    \
  V(JSWeakCollection)            \
  V(Map)                         \
  V(Oddball)                     \
  V(PropertyArray)               \
  V(PropertyCell)                \
  V(SeqOneByteString)            \
  V(SeqTwoByteString)            \
  V(SharedFunctionInfo)          \
  V(SlicedString)                \
  V(SmallOrderedHashMap)         \
  V(SmallOrderedHashSet)         \
  V(Symbol)                      \
  V(ThinString)                  \
  V(TransitionArray)             \
  V(WeakCell)

// The base class for visitors that need to dispatch on object type. The default
// behavior of all visit functions is to iterate body of the given object using
// the BodyDescriptor of the object.
//
// The visit functions return the size of the object cast to ResultType.
//
// This class is intended to be used in the following way:
//
//   class SomeVisitor : public HeapVisitor<ResultType, SomeVisitor> {
//     ...
//   }
template <typename ResultType, typename ConcreteVisitor>
class HeapVisitor : public ObjectVisitor {
 public:
  V8_INLINE ResultType Visit(HeapObject* object);
  V8_INLINE ResultType Visit(Map* map, HeapObject* object);

 protected:
  // A guard predicate for visiting the object.
  // If it returns false then the default implementations of the Visit*
  // functions bailout from iterating the object pointers.
  V8_INLINE bool ShouldVisit(HeapObject* object) { return true; }
  // Guard predicate for visiting the objects map pointer separately.
  V8_INLINE bool ShouldVisitMapPointer() { return true; }
  // A callback for visiting the map pointer in the object header.
  V8_INLINE void VisitMapPointer(HeapObject* host, HeapObject** map);

#define VISIT(type) V8_INLINE ResultType Visit##type(Map* map, type* object);
  TYPED_VISITOR_ID_LIST(VISIT)
#undef VISIT
  V8_INLINE ResultType VisitShortcutCandidate(Map* map, ConsString* object);
  V8_INLINE ResultType VisitNativeContext(Map* map, Context* object);
  V8_INLINE ResultType VisitDataObject(Map* map, HeapObject* object);
  V8_INLINE ResultType VisitJSObjectFast(Map* map, JSObject* object);
  V8_INLINE ResultType VisitJSApiObject(Map* map, JSObject* object);
  V8_INLINE ResultType VisitStruct(Map* map, HeapObject* object);
  V8_INLINE ResultType VisitFreeSpace(Map* map, FreeSpace* object);
};

template <typename ConcreteVisitor>
class NewSpaceVisitor : public HeapVisitor<int, ConcreteVisitor> {
 public:
  V8_INLINE bool ShouldVisitMapPointer() { return false; }

  // Special cases for young generation.

  V8_INLINE int VisitJSFunction(Map* map, JSFunction* object);
  V8_INLINE int VisitNativeContext(Map* map, Context* object);
  V8_INLINE int VisitJSApiObject(Map* map, JSObject* object);

  int VisitBytecodeArray(Map* map, BytecodeArray* object) {
    UNREACHABLE();
    return 0;
  }

  int VisitSharedFunctionInfo(Map* map, SharedFunctionInfo* object) {
    UNREACHABLE();
    return 0;
  }
};

template <typename ConcreteVisitor>
class MarkingVisitor : public HeapVisitor<int, ConcreteVisitor> {
 public:
  explicit MarkingVisitor(Heap* heap, MarkCompactCollector* collector)
      : heap_(heap), collector_(collector) {}

  V8_INLINE bool ShouldVisitMapPointer() { return false; }

  V8_INLINE int VisitJSFunction(Map* map, JSFunction* object);
  V8_INLINE int VisitWeakCell(Map* map, WeakCell* object);
  V8_INLINE int VisitTransitionArray(Map* map, TransitionArray* object);
  V8_INLINE int VisitNativeContext(Map* map, Context* object);
  V8_INLINE int VisitJSWeakCollection(Map* map, JSWeakCollection* object);
  V8_INLINE int VisitBytecodeArray(Map* map, BytecodeArray* object);
  V8_INLINE int VisitCode(Map* map, Code* object);
  V8_INLINE int VisitMap(Map* map, Map* object);
  V8_INLINE int VisitJSApiObject(Map* map, JSObject* object);
  V8_INLINE int VisitAllocationSite(Map* map, AllocationSite* object);

  // ObjectVisitor implementation.
  V8_INLINE void VisitEmbeddedPointer(Code* host, RelocInfo* rinfo) final;
  V8_INLINE void VisitDebugTarget(Code* host, RelocInfo* rinfo) final;
  V8_INLINE void VisitCodeTarget(Code* host, RelocInfo* rinfo) final;
  V8_INLINE void VisitCodeAgeSequence(Code* host, RelocInfo* rinfo) final;
  // Skip weak next code link.
  V8_INLINE void VisitNextCodeLink(Code* host, Object** p) final {}

 protected:
  V8_INLINE void MarkMapContents(Map* map);

  Heap* heap_;
  MarkCompactCollector* collector_;
};

class WeakObjectRetainer;

// A weak list is single linked list where each element has a weak pointer to
// the next element. Given the head of the list, this function removes dead
// elements from the list and if requested records slots for next-element
// pointers. The template parameter T is a WeakListVisitor that defines how to
// access the next-element pointers.
template <class T>
Object* VisitWeakList(Heap* heap, Object* list, WeakObjectRetainer* retainer);
}  // namespace internal
}  // namespace v8

#endif  // V8_OBJECTS_VISITING_H_
