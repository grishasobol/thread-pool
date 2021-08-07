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
class ThreadPool
{
    static_assert( !is_same<T, void>::value, "Does not support void input type" );
    static_assert( !is_same<R, void>::value, "Does not support void return type" );

private:
    struct PoolContext
    {
        sem_t sem;
        map<int, R> results_map;
        stack<int> threads_idx_stack;
        mutex data_mutex;
    };

    static void run_one_thread( const T& arg, R(*func)(T), PoolContext *ctx, int idx);

public:
    ThreadPool() = delete;
    static vector<R> run_in_thread_pool( const vector<T>& args, R(*func)(T), int max_treads_num, int threshold);
};

template<class T, class R>
void
ThreadPool<T, R>::run_one_thread( const T& arg, R(*func)(T), PoolContext *ctx, int idx)
{
    R res = func( arg);
    ctx->data_mutex.lock();
    ctx->results_map.insert( {idx, res});
    ctx->threads_idx_stack.push( idx);
    ctx->data_mutex.unlock();
    sem_post( &ctx->sem);
}

template<class T, class R>
vector<R>
ThreadPool<T, R>::run_in_thread_pool( const vector<T>& args, R(*func)(T), int max_treads_num, int threshold)
{
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

    PoolContext ctx;
    map<int, thread*> threads_map;

    int res = sem_init( &ctx.sem, 0, max_treads_num);
    assert( res == 0 && "Could not create semaphore" );

    int idx = 0;
    for ( auto arg : args )
    {
        sem_wait( &ctx.sem);

        thread *thr = nullptr;
        ctx.data_mutex.lock();
        if ( ctx.threads_idx_stack.size() > 0 )
        {
            int idx = ctx.threads_idx_stack.top();
            ctx.threads_idx_stack.pop();
            thr = threads_map.at( idx);
            threads_map.erase( idx);
        }
        ctx.data_mutex.unlock();
        if ( thr ) thr->join();

        thread *new_thr = new thread( run_one_thread, arg, func, &ctx, idx);
        threads_map.insert( {idx, new_thr});

        idx++;
    }

    for ( auto thr : threads_map )
    {
        thr.second->join();
    }

    sem_destroy( &ctx.sem);

    vector<R> res_v;
    for ( auto res : ctx.results_map )
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

    auto res1 = ThreadPool<int, int>::run_in_thread_pool( v, square, 10, 2);
    for ( auto x : res1 ) cout << x << ' ';
    cout << endl;
}
