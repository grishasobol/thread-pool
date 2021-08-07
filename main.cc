#include <thread>
#include <semaphore.h>
#include <vector>
#include <assert.h>
#include <map>
#include <iostream>
#include <mutex>
#include <stack>
#include <type_traits>

using namespace std;

template<class T, class R>
void
run_one_thread( T arg, R(*func)(T), sem_t *sem, map<int, R> *results_map, mutex *results_map_mutex, stack<int> *thread_idx, int idx)
{
    R res = func( arg);
    results_map_mutex->lock();
    results_map->insert( {idx, res});
    thread_idx->push( idx);
    results_map_mutex->unlock();
    sem_post( sem);
}

template<class T, class R>
vector<R>
run_in_thread_pool( vector<T> args, R(*func)(T), int max_treads_num, int threshold)
{
    static_assert( !is_same<R, void>::value, "Does not support void return type" );
    static_assert( !is_same<T, void>::value, "Does not support void input type" );
    assert( max_treads_num > 0 && "Must have at least one thread" );
    assert( threshold >= 0 && "Cannot be negative" );

    if ( args.size() < threshold )
    {
        vector<R> results;
        for ( auto arg : args )
        {
            results.push_back( func( arg));
        }
        return results;
    }

    sem_t sem;
    map<int, R> results_map;
    mutex m;
    map<int, thread*> threads_map;
    stack<int> threads_idx_stack;

    int res = sem_init( &sem, 0, max_treads_num);
    assert( res == 0 && "Could not create semaphore" );

    int idx = 0;
    for ( auto arg : args )
    {
        sem_wait( &sem);
        m.lock();
        if ( threads_idx_stack.size() > 0 )
        {
            int idx = threads_idx_stack.top();
            threads_idx_stack.pop();
            auto t = threads_map.at( idx);
            threads_map.erase( idx);
            // TODO:
            t->join();
        }
        m.unlock();
        auto t = new thread( run_one_thread<T, R>, arg, func, &sem, &results_map, &m, &threads_idx_stack, idx);
        threads_map.insert( {idx, t});
        idx++;
    }

    for ( auto t : threads_map )
    {
        t.second->join();
    }

    sem_destroy( &sem);

    vector<R> res_v;
    for ( auto res : results_map )
    {
        res_v.push_back( res.second);
    }

    return res_v;
}

int square( int x)
{
    return x * x;
}

int main()
{
    cout << "MAIN\n";
    vector<int> v;
    for ( int i = 0; i < 1000; i++ ) v.push_back( i);

    auto res1 = run_in_thread_pool<int, int>( v, square, 10, 2);
    for ( auto x : res1 ) cout << x << ' ';
    cout << endl;
}
