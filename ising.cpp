#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <boost/math/special_functions/erf.hpp>

#include "physics.h"
#include "algs.h"
#include "System.h"


std::vector<std::vector<int> > genRandomSystem(const unsigned int L, int seedOffset){
	long long int seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed1 + seedOffset);
	std::uniform_int_distribution <int> dist(0,1);
	std::vector<std::vector<int> > grid(L, std::vector<int>(L));
	for (int i = 0; i < L; ++i){
		for (int j = 0; j < L; ++j){
			grid[i][j] = dist(generator)*2 - 1;
		}
	}
	return grid;
}


std::vector<std::vector<int> > getRelaxedSys(const unsigned int L, const double T, double J, unsigned int n1, std::string alg, int seedOffset=0) {
    auto grid = genRandomSystem(L, seedOffset);
    if(alg=="metro")
        metropolis_sweeps(grid, T, n1, J);
    else if(alg=="sw")
        wangRepeats(grid, T, n1, J);
    else
        std::cerr << "unknown alg!" << std::endl;
    return grid;
}


std::vector<double> getTemps(double TMin, double TMax, unsigned int TSteps){
    std::vector<double> T_vec;
    double dT = (TMax - TMin)/(TSteps-1);
    if(TSteps==1)
        dT=0;
    for(int i=0; i<TSteps; ++i)
        T_vec.push_back(TMin + i*dT);
    return T_vec;
}

double area(double TMin, double TMax, double mu, double sigma){
    return 0.5*(boost::math::erf((TMax-mu)/(sigma*sqrt(2.0)))-boost::math::erf((TMin-mu)/(sigma*sqrt(2.0))));
}

double areaCumul(double T, double mu, double sigma){
    return 0.5*(1.0+boost::math::erf((T-mu)/(sigma*sqrt(2.0))));
}

std::vector<double> getTempsNormal(double TMin, double TMax, unsigned int TSteps){
    double mu=2.269, sigma=1.0;
    std::vector<double> T_vec;
    T_vec.push_back(TMin);
    double F;
    double area_middle = area(TMin, TMax, mu,sigma);
    double area_TMin = areaCumul(TMin, mu, sigma);
    for(int i=1; i<TSteps-1; ++i) {
        F = i*area_middle/(TSteps-1) + area_TMin;
        T_vec.push_back(sqrt(2.0) * sigma * boost::math::erf_inv(F*2.0-1.0) + mu);
    }
    T_vec.push_back(TMax);
    return T_vec;
}

template<typename T>
std::string to_string(T const & value);

template <typename T>
std::string to_string2(T const & value){
    std::stringstream ss;
    ss << value;
    return ss.str();
}

void checkParam(int argc, char* argv[], Config& cfg, LabConfig& labCfg){
    std::string key, value;
    int eqPos;
    std::string argument;

    for (int i = 1; i < argc; ++i) {
        argument = argv[i];
        eqPos = argument.find("=");
        key = argument.substr(1, eqPos-1);
        value = argument.substr(eqPos+1);

        if (key == "L")
            cfg.L = atoi(value.c_str());
        else if (key == "N1")
            cfg.n1 = atoi(value.c_str());
        else if (key == "N2")
            cfg.n2 = atoi(value.c_str());
        else if (key == "N3")
            cfg.n3 = atoi(value.c_str());
        else if (key == "threads")
            cfg.threadCount = atoi(value.c_str());
        else if (key == "J")
            cfg.J = atof(value.c_str());
        else if (key == "alg1")
            cfg.alg1 = value;
        else if (key == "alg2")
            cfg.alg2 = value;
        else if (key == "record")
            cfg.recordMain = value == "main";
        else if (key == "dist")
            labCfg.normalDist = value == "normal";
        else if (key == "en")
            labCfg.fileEnergy = value;
        else if (key == "mag")
            labCfg.fileMag = value;
        else if (key == "cv")
            labCfg.fileCv = value;
        else if (key == "chi")
            labCfg.fileChi = value;
        else if (key == "corr")
            labCfg.fileCorr = value;
        else if (key == "states")
            labCfg.fileStates = value;
        else if (key == "TSteps")
            labCfg.TSteps = atoi(value.c_str());
        else if (key == "TMin")
            labCfg.TMin = atof(value.c_str());
        else if (key == "TMax")
            labCfg.TMax = atof(value.c_str());
        else
            std::cerr << "Unknown parameter: " << argument << std::endl;
    }
}

int main(int argc, char* argv[]){
    if (argc < 2) {
        std::cerr << "Too few arguments. See documentation." << std::endl;
        return 1;
    }

    Config cfg;
    LabConfig labCfg;
    checkParam(argc, argv, cfg, labCfg);
    omp_set_num_threads(cfg.threadCount);

    std::vector<double> temps;
    if(labCfg.normalDist)
        temps = getTempsNormal(labCfg.TMin, labCfg.TMax, labCfg.TSteps);
    else
	    temps = getTemps(labCfg.TMin, labCfg.TMax, labCfg.TSteps);
    std::vector<System> systems;
    for(double T : temps){
		cfg.T = T;
        systems.push_back(System(cfg, labCfg));
    }

    std::string progressStr = std::string(labCfg.TSteps, '.');
    std::cout << progressStr.c_str() << std::endl;
    auto t0 = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for
    for(unsigned int i=0; i<systems.size(); ++i){
        systems[i].compute();
        std::cout << ".";
    }
    std::cout << std::endl;

    std::vector<std::string> filenames = {labCfg.fileEnergy, labCfg.fileMag, labCfg.fileCv, labCfg.fileChi, labCfg.fileCorr};
    std::ofstream fileOut;

    for(unsigned int i=0; i<filenames.size(); ++i){
        if(! filenames[i].empty()){
            fileOut.open(filenames[i]+".txt");
            for (auto& sys : systems)
                fileOut << to_string(sys.cfg.T) << ", " << sys.results[i] << std::endl;
            fileOut.close();
        }
    }

    if(!labCfg.fileStates.empty()){
        for (int i=0; i<systems.size(); ++i){
            fileOut.open(labCfg.fileStates+to_string2(i)+".txt");
            for(int j=0; j<systems[i].resultsStates.size(); ++j)
                fileOut << systems[i].resultsStates[j] << std::endl;
            fileOut.close();
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    float secs = (std::chrono::duration_cast <std::chrono::milliseconds > (t1-t0).count())*0.001f;
    std::cout << "runtime: " << secs << "s" << std::endl;

	return 0;
}
