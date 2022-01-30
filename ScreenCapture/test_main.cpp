#include <ctime>
#include <iomanip>
#include <iostream>
#include <bits/stdc++.h>


/* Recupero timestamp */
std::string retrieveTimestampTEST()
{

    std::string current_time;

    #ifdef __linux__
        //#TODO: dovrebbe tranquillamente funzionare anche per Windows
        // declaring argument of time()
        const time_t now = time(nullptr);

        /* ctime() used to give the present time */
        // current_time = ctime(&my_time); //ctime obsoleta per lo standard POSIX

        /* Main source of this solution: https://en.cppreference.com/w/cpp/chrono/c/asctime */
        char buf[64];
        if (strftime(buf, sizeof buf, "%c\n", std::localtime(&now)) || strftime(buf, sizeof buf, "%a %b %e %H:%M:%S %Y\n", std::localtime(&now)))
        {
            // std::cout << std::setw(40) << "    strftime %c" << buf;
            // std::cout << std::setw(40) << "    strftime %a %b %e %H:%M:%S %Y" << buf;
            current_time = buf;
            current_time.erase(current_time.end() - 1, current_time.end());
            std::replace(current_time.begin(), current_time.end(), ' ', '_');
        }
        // else {
        //     std::cerr << "Error in retrieving timestamp" << std::endl;
        //     exit(-1);
        // }
    #elif _WIN32
        time_t result = time(nullptr);
        stringstream ss;
        ss << time;
        current_time = ss.str();
    #endif

    return current_time;
}

int main()
{
    std::string ret = retrieveTimestampTEST();
    std::cout << ret << std::endl;
}