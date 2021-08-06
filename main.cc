#include <thread>
#include <semaphore.h>
#include <vector>
#include <assert.h>
#include <map>
#include <iostream>
#include <mutex>

using namespace std;

sem_t sem_obj;
map<int, int> results;
mutex m;

void tfunc( int(*f)(int), int a, int idx)
{
    int res = f(a);
    m.lock();
    results.insert( {idx, res});
    m.unlock();
    int b; sem_getvalue( &sem_obj, &b);
    cout << b << endl;
    sem_post( &sem_obj);
}

vector<int> func( vector<int> a, int(*f)(int) )
{
    cout << "KEK\n";
    int res = sem_init( &sem_obj, 0, 10);
    assert( res == 0 );

    int idx = 0;
    for ( auto x : a )
    {
        cout << "LOL\n";
        sem_wait( &sem_obj);
        cout << "LOL1\n";
        thread(tfunc, f, x, idx).detach();
        cout << "LOL2\n";
        idx++;
    }

    for ( int i = 0; i < 10; i++) sem_wait( &sem_obj);

    sem_destroy( &sem_obj);

    vector<int> res_v;
    for ( auto x : results )
    {
        res_v.push_back( x.second);
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
    auto res = func( v, square);
    for ( auto x : res ) cout << x;
    cout << endl;
}
