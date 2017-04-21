/* 
*Author: Maike Jung
*Date: 15.11.2016

*Purpose: Calculate the likelihood for the random events generated with generateEvents.c to belong to a certain mass2 spectrum

SN - Model: Lawrence-Livermore
    time spectrum is convoluted with the first hit distribution, to account for not knowing the absolute arrival times

UNITS: mass2: eV
       energy: MeV
       distance: Mpc
       time: s

BINNING: see spectrum.h

*/

#include "spectrum.h"

/// load event
void getEvent(int *eventEnergy, int *eventTime, double mass2, double distance, double events, int filenumber, double noise){
    /*load events & store energy and time in arrays*/
    char filename[sizeof "DATA_COMP/10.00Mpc_700Events_1.57eV_event_1.45eV_10.5Mpc_1000Events_real_1999111.txt"];
    sprintf(filename, "DATA_COMP/%.2fMpc_%.0fEvents_%.5feV/events_%.2feV_%.2fMpc_%.0fEvents_real_%d.txt",distance, events, mass2, mass2, distance, events, filenumber);

    //char filename[sizeof "DATA/NOISETEST/10.00Mpc_700Events_1.57eV_event_b000_1.45eV_10.5Mpc_1000Events_real_1111.txt"];
    //sprintf(filename, "DATA/NOISETEST/%.2fMpc_%.0fEvents_%.2feV_b%.3f/events_%.2feV_%.2fMpc_%.0fEvents_real_%d.txt",distance, events, mass2, noise, mass2, distance, events, filenumber);

    FILE *f = fopen(filename, "r");
    int i;
    for(i = 0; i < events; eventEnergy[i++] = 1);
    for(i = 0; i < events; eventTime[i++] = 1);
    for (i = 0; i < events; i++){
        fscanf(f, "%d %d", &eventEnergy[i], &eventTime[i]);
    }
}
///function that calculates likelihood - and will then be minimized in python
double getLLH(double mass2, double distance, double events, bool triggEff, bool energyRes, double noise, int *eventTime, int *eventEnergy, double noise_events, double *logTime, double *logTimeConv){

    double llh = 0.0;
    int i;
    user_data_t spectrum[(RESE-1)*REST];

	//double *spectrum= (double*) malloc((RESE-1) * REST * sizeof(double));
    createSpectrum(spectrum, mass2, distance, events, energyRes, triggEff, noise, noise_events, logTime, logTimeConv);
    for (i = 0; i < events; i++){
        if (spectrum[eventTime[i]*(RESE-1)+eventEnergy[i]] < pow(10,-200)){
            llh += -10000000;   
            printf("event number %d e %d t %d\n",i, eventEnergy[i], eventTime[i]);
            printf("value of spectrum very small - check \n");
        }
        else llh += log(spectrum[eventTime[i]*(RESE-1)+eventEnergy[i]]);
    }
    llh*=-1;
    printf("mass llh %f %f \n", mass2, llh);
    return llh;
}

double getLLHLogBins(double mass2, double distance, double events, bool triggEff, bool energyRes, double noise, int *eventTime, int *eventEnergy, double noise_events, double *logTime, double *logTimeConv, double *spectrumGen){

    double llh = 0.0;
    int i;
    user_data_t spectrum[(RESE-1)*REST];

	//double *spectrum= (double*) malloc((RESE-1) * REST * sizeof(double));
    createSpectrum(spectrum, mass2, distance, events, energyRes, triggEff, noise, noise_events, logTime, logTimeConv);
    for (i = 0; i < events; i++){
        if (spectrum[eventTime[i]*(RESE-1)+eventEnergy[i]] < pow(10,-200)){   
            printf("event number %d e %d t %d\n",i, eventEnergy[i], eventTime[i]);
            printf("value of test spectrum very small - check \n");
        }
        if (spectrumGen[eventTime[i]*(RESE-1)+eventEnergy[i]] < pow(10,-200)){   
            printf("event number %d e %d t %d\n",i, eventEnergy[i], eventTime[i]);
            printf("value of original spectrum very small - check \n");
        }
        llh += events*spectrum[eventTime[i]*(RESE-1)+eventEnergy[i]] - log(spectrum[eventTime[i]*(RESE-1)+eventEnergy[i]]*events);
    }
    //llh*=-1;
    printf("llh %f %f\n",mass2, llh);
    return llh;
}

// method that scans over range
void calcLLH(double mass, double distance, double events, bool triggEff, bool energyRes, int filenumber, double noise, double noise_events, double *logTime, double *logTimeConv, int *eventEnergy, int *eventTime){

    //write to file for a test
    char filename[sizeof "DATA_THESIS/results_1.00eV_1.00Mpc_156Events_10.txt"];
    sprintf(filename, "DATA_THESIS/results_%.2feV_%.2fMpc_%.0fEvents_%d.txt",mass, distance, events, filenumber);   
    FILE *f = fopen(filename, "a+");

    // calculate the likelihood
    int i;
    double llh;
    double testMass;
    // store current value of the minimumLLH and the corresponding mass
    double minLLH = INFINITY;
    double massOfMinLLH = 0.0;
    // go over all the spectra around a certain range of the input mass & calculate the likelihood for each spectrum
    double *testSpectrum= (double*) malloc((RESE-1) * REST * sizeof(double));
    // first go over broad range - there are no negative entries in the spectrum!!!!!
    //for (testMass = mass - 0.5; testMass <= mass + 0.5; testMass+=0.1){
    for (testMass = 0.0; testMass <= 1.01; testMass+=0.05){
        llh = 0.0;
        createSpectrum(testSpectrum, testMass, distance, events, energyRes, triggEff, noise, noise_events, logTime, logTimeConv);
        for (i = 0; i < events; i++){
            if (testSpectrum[eventTime[i]*(RESE-1)+eventEnergy[i]] < pow(10,-200)){
                llh += -10000000;   
            }
            else llh += log(testSpectrum[eventTime[i]*(RESE-1)+eventEnergy[i]]);
            //printf("tset %d %f \n", i, llh);
        }
        llh*=-1;
        printf("mass %f, llh %f\n",testMass, llh);
        fprintf(f, "%f %f\n", testMass, llh);
        if (llh < minLLH) {
            minLLH = llh;
            massOfMinLLH = testMass;
        }
    }

    fclose(f);
    printf("mass found: %f eV\n", massOfMinLLH);

    free(testSpectrum);

    /*write to file*/
    char filename2[sizeof "Results_Likelihood/test_masses_1.55eV_1Mpc_real.txt"];
    if (triggEff && energyRes){
        sprintf(filename2, "TEST_masses_%.2feV_%.1fMpc_real_1.txt", mass, distance);
    }
    else {
        sprintf(filename2, "Results_Likelihood/masses_%.2feV_%.1fMpc_ideal_test3.txt", mass, distance);
    }
    FILE *g = fopen(filename2, "a+");
    fprintf(g, "%f \n", massOfMinLLH);
    fclose(g);
}
