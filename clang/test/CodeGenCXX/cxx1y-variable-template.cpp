// RUN: %clang_cc1 -std=c++1y -Wno-unused-value -triple x86_64-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Check that we keep the 'extern' when we instantiate the definition of this
// variable template specialization.
template<typename T> extern const int extern_redecl;
template<typename T> const int extern_redecl = 5;
template const int extern_redecl<int>;

// CHECK: @_Z13extern_redeclIiE = weak_odr constant

template<typename T> struct Outer {
  template<typename U> struct Inner {
    template<typename V> static int arr[];
  };
};
Outer<char[100]> outer_int;
int init_arr();
template<typename T> template<typename U> template<typename V> int Outer<T>::Inner<U>::arr[sizeof(T) + sizeof(U) + sizeof(V)] = { init_arr() };
int *p = Outer<char[100]>::Inner<char[20]>::arr<char[3]>;

//CHECK: @_ZN8GH1406221gIiEE = linkonce_odr constant %"struct.GH140622::S" zeroinitializer
namespace GH140622 {
template <typename> struct S {};
template <typename T> constexpr S<T> g;
void test() {
    constexpr auto x = 42;
    x, g<int>;
}
}

namespace PR35456 {
// CHECK: @_ZN7PR354561nILi0EEE = linkonce_odr global i32 0
template<int> int n;
int *p = &n<0>;
}

// CHECK: @_ZN5OuterIA100_cE5InnerIA20_cE3arrIA3_cEE = linkonce_odr global [123 x i32] zeroinitializer
// CHECK: @_ZGVN5OuterIA100_cE5InnerIA20_cE3arrIA3_cEE = linkonce_odr global

// CHECK: @_ZTHN7PR4211112_GLOBAL__N_11nILi0EEE = internal alias {{.*}} @[[PR42111_CTOR:.*]]

// CHECK: call {{.*}}@_Z8init_arrv

// Ensure that we use guarded initialization for an instantiated thread_local
// variable with internal linkage.
namespace PR42111 {
  int f();
  namespace { template <int = 0> thread_local int n = f(); }
  // CHECK: define {{.*}}@[[PR42111_CTOR]](
  // CHECK: load {{.*}} @_ZGVN7PR4211112_GLOBAL__N_11nILi0EEE
  // CHECK: icmp eq i8 {{.*}}, 0
  // CHECK: br i1
  // CHECK: store i8 1, ptr @_ZGVN7PR4211112_GLOBAL__N_11nILi0EEE
  // CHECK: call noundef i32 @_ZN7PR421111fEv(
  // CHECK: [[N_ADDR:%.+]] = call align 4 ptr @llvm.threadlocal.address.p0(ptr align 4 @_ZN7PR4211112_GLOBAL__N_11nILi0EEE)
  // CHECK: store i32 {{.*}}, ptr [[N_ADDR]]
  int g() { return n<> + n<>; }
}
