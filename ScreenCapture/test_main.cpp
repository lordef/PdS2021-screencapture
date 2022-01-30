/*  TODO: File da eliminare prima della consegna */
#include <bits/stdc++.h>
#include <pulse/proplist.h>

using namespace std;

/* Recupero timestamp */
std::string retrieveTimestampTEST()
{

    std::string current_time;   

    #ifdef __linux__ 
    //#TODO: dovrebbe tranquillamente funzionare anche per Windows
        // declaring argument of time()
        time_t my_time = time(nullptr);
        //ctime() used to give the present time
        current_time = ctime(&my_time);   
        current_time.erase(current_time.end()-1, current_time.end());
        std::replace(current_time.begin(), current_time.end(), ' ', '_');
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
	// struct timespec systemtime;
	// clock_gettime(CLOCK_MONOTONIC, &systemtime);

    // string out = retrieveTimestampTEST();
 
    std::time_t result = std::time(nullptr);
    // std::cout << std::asctime(std::localtime(&result))
    //           << result << " seconds since the Epoch\n";

    char* out;
    asctime_r(std::localtime(&result), out);

	// string hello = "hello";
	cout << out << endl;
	return 0;
}
