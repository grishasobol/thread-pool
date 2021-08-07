#include <iostream>
#include "thread_pool.h"

int square( int x)
{
    return x * x;
}

double long_square( double x)
{
    this_thread::sleep_for( chrono::seconds(2));
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

    auto start_time = chrono::steady_clock::now();
    vector<double> v1;
    for ( int i = 0; i < 1000; i++ ) v1.push_back( 1 / (double)i );
    auto res2 = ThreadPool<double, double>::run_in_thread_pool( v1, long_square, 100, 2);
    auto end_time = chrono::steady_clock::now();
    assert( (end_time - start_time) > chrono::seconds(20) );
    for ( auto x : res2 ) cout << x << ' ';
    cout << endl;
}
