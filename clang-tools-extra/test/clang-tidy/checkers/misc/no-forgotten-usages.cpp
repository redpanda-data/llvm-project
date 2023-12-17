// RUN: %check_clang_tidy %s misc-no-forgotten-usages %t -- \
// RUN:       -config="{CheckOptions: [{key: 'misc-no-forgotten-usages.Classes', value: 'seastar::FakeFuture'}]}"

namespace seastar {
template<typename T>
class FakeFuture {
public:    
    T get() {
        return Pending;
    }
private:
    T Pending;
};


} // namespace seastar

namespace ss {
template<typename T>
using Future = seastar::FakeFuture<T>;
} // namespace ss

void releaseUnits();
struct Units {
  ~Units() {
    releaseUnits();
  }
};
ss::Future<Units> acquireUnits();

template<typename T>
T qux(T Generic) {
    seastar::FakeFuture<Units> PendingA = acquireUnits();
    auto PendingB = acquireUnits();
    // CHECK-MESSAGES: :[[@LINE-1]]:10: warning: variable 'PendingB' with type 'FakeFuture<Units>' must be used [misc-no-forgotten-usages]
    PendingA.get();
    return Generic;
}

int bar(int Num) {
    ss::Future<Units> PendingA = acquireUnits();
    ss::Future<Units> PendingB = acquireUnits(); // not used at all, unused variable not fired because of destructor side effect
    // CHECK-MESSAGES: :[[@LINE-1]]:23: warning: variable 'PendingB' with type 'FakeFuture<Units>' must be used [misc-no-forgotten-usages]
    auto Num2 = PendingA.get();
    auto Num3 = qux(Num);
    return Num * Num3;
}
