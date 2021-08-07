#include <iostream>
#include <cmath>
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

int simple_round( double x)
{
    int res = (int)round(x);
    return res;
}

int long_round( double x)
{
    int res = (int)round(x);
    this_thread::sleep_for( chrono::seconds(res % 3));
    return res;
}

template<class T, class R>
vector<R>
calcReturnNoThreads( const vector<T> args, R(*func)(T))
{
    vector<R> res;
    for ( auto arg : args ) res.push_back( func( arg));
    return res;
}

int main()
{
    cout << "====== START TESTS\n";

    int threshold;
    int max_threads_num;
    bool check;
    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point end_time;

    vector<int> v;
    for ( int i = 0; i < 1000; i++ ) v.push_back( i);
    vector<double> v1;
    for ( int i = 0; i < 1000; i++ ) v1.push_back( 100 / (double)i );
    vector<double> v2;
    for ( int i = 0; i < 5; i++ ) v2.push_back( 1 / (double)i );

    threshold = 2;
    max_threads_num = 10;
    cout << "=== RUN TEST max_threads_num=" << max_threads_num << " threasold=" << threshold << " T=int R=int\n";
    cout << "Checks return is correct - " << flush;
    check = calcReturnNoThreads( v, square) == ThreadPool<int, int>::run_in_thread_pool( v, square, max_threads_num, threshold);
    cout << (check ? "OK" : "FAILED") << endl;
    assert( check );

    threshold = 0;
    max_threads_num = 1000;
    cout << "=== RUN TEST max_threads_num=" << max_threads_num << " threasold=" << threshold << " T=int R=int\n";
    cout << "Checks return is correct - " << flush;
    check = calcReturnNoThreads( v, square) == ThreadPool<int, int>::run_in_thread_pool( v, square, max_threads_num, threshold);
    cout << (check ? "OK" : "FAILED") << endl;
    assert( check );

    threshold = 11;
    max_threads_num = 100;
    cout << "=== RUN TEST max_threads_num=" << max_threads_num << " threasold=" << threshold << " T=double R=double, take ~20sec\n";
    cout << "Checks number of threads less then max - " << flush;
    start_time = chrono::steady_clock::now();
    ThreadPool<double, double>::run_in_thread_pool( v1, long_square, max_threads_num, threshold);
    end_time = chrono::steady_clock::now();
    check = (end_time - start_time) > chrono::seconds( v1.size() / max_threads_num * 2 );
    cout << (check ? "OK" : "FAILED") << endl;
    assert( check );

    threshold = 11;
    max_threads_num = 100;
    cout << "=== RUN TEST max_threads_num=" << max_threads_num << " threasold=" << threshold << " T=double R=double, take ~10sec\n";
    cout << "Checks that executed in one thread - " << flush;
    start_time = chrono::steady_clock::now();
    ThreadPool<double, double>::run_in_thread_pool( v2, long_square, max_threads_num, threshold);
    end_time = chrono::steady_clock::now();
    check = (end_time - start_time) > chrono::seconds( v2.size() * 2 );
    cout << (check ? "OK" : "FAILED") << endl;
    assert( check );

    threshold = 1;
    max_threads_num = 100;
    cout << "=== RUN TEST max_threads_num=" << max_threads_num << " threasold=" << threshold << " T=double R=int\n";
    cout << "Checks function with different time execution - " << flush;
    check = calcReturnNoThreads( v1, simple_round)
            == ThreadPool<double, int>::run_in_thread_pool( v1, long_round, max_threads_num, threshold);
    cout << (check ? "OK" : "FAILED") << endl;
    assert( check );

    cout << "====== TESTS SUCCESSFULLY FINISHED\n";
    return 0;
}