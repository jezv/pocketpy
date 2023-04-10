#pragma once

#include "common.h"
#include "memory.h"
#include "obj.h"
#include "codeobject.h"
#include "namedict.h"

namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    VM* vm;
    ManagedHeap(VM* vm): vm(vm) {}
    
    static const int kMinGCThreshold = 3072;
    int gc_threshold = kMinGCThreshold;
    int gc_counter = 0;

    /********************/
    int _gc_lock_counter = 0;
    struct ScopeLock{
        ManagedHeap* heap;
        ScopeLock(ManagedHeap* heap): heap(heap){
            heap->_gc_lock_counter++;
        }
        ~ScopeLock(){
            heap->_gc_lock_counter--;
        }
    };

    ScopeLock gc_scope_lock(){
        return ScopeLock(this);
    }
    /********************/

    template<typename T>
    PyObject* gcnew(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    ~ManagedHeap(){
        for(PyObject* obj: _no_gc) obj->~PyObject(), pool64.dealloc(obj);
#if DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << obj_type_name(vm, type) << "=" << count << std::endl;
        }
#endif
    }

    int sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
#if DEBUG_GC_STATS
                deleted[obj->type] += 1;
#endif
                obj->~PyObject(), pool64.dealloc(obj);
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc.marked = false;

        int freed = gen.size() - alive.size();
        // std::cout << "GC: " << alive.size() << "/" << gen.size() << " (" << freed << " freed)" << std::endl;
        gen.clear();
        gen.swap(alive);
        return freed;
    }

    void _auto_collect(){
#if !DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        if(gc_counter < gc_threshold) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < kMinGCThreshold) gc_threshold = kMinGCThreshold;
#endif
    }

    int collect(){
        if(_gc_lock_counter > 0) UNREACHABLE();
        mark();
        int freed = sweep();
        return freed;
    }

    void mark();
};

inline void NameDict::_gc_mark() const{
    if(size() == 0) return;
    for(uint16_t i=0; i<_capacity; i++){
        if(_items[i].first.empty()) continue;
        OBJ_MARK(_items[i].second);
    }
}

inline void FuncDecl::_gc_mark() const{
    code->_gc_mark();
    kwargs._gc_mark();
}

template<> inline void _gc_mark<List>(List& t){
    for(PyObject* obj: t) OBJ_MARK(obj);
}

template<> inline void _gc_mark<Tuple>(Tuple& t){
    for(int i=0; i<t.size(); i++) OBJ_MARK(t[i]);
}

template<> inline void _gc_mark<Function>(Function& t){
    t.decl->_gc_mark();
    if(t._module != nullptr) OBJ_MARK(t._module);
    if(t._closure != nullptr) t._closure->_gc_mark();
}

template<> inline void _gc_mark<BoundMethod>(BoundMethod& t){
    OBJ_MARK(t.obj);
    OBJ_MARK(t.method);
}

template<> inline void _gc_mark<StarWrapper>(StarWrapper& t){
    OBJ_MARK(t.obj);
}

template<> inline void _gc_mark<Super>(Super& t){
    OBJ_MARK(t.first);
}
// NOTE: std::function may capture some PyObject*, they can not be marked

}   // namespace pkpy