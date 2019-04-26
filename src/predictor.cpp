//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include "predictor.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
using namespace std;

//
// TODO:Student Information
//
const char *studentName = "Wu, Ho Lun";
const char *studentID = "A53271935";
const char *email = "hlwu@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare", "Tournament", "Custom"};

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
char *gsTable;
uint32_t gsHistory;
//
// TODO: Add your own Branch Predictor data structures here
//

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor() {
    //
    // TODO: Initialize Branch Predictor Data Structures
    //
    // gshare init
    gsTable = new char[1 << ghistoryBits];
    memset(gsTable, 1, sizeof(char) << ghistoryBits);
    gsHistory = 0;
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t gshare_predictor(uint32_t pc) {
    return gsTable[(pc ^ gsHistory) & ((1 << ghistoryBits) - 1)] ? TAKEN
                                                                 : NOTTAKEN;
}
uint8_t make_prediction(uint32_t pc) {
    //
    // TODO: Implement prediction scheme
    //

    // Make a prediction based on the bpType
    switch (bpType) {
    case STATIC:
        return TAKEN;
    case GSHARE:
        return gshare_predictor(pc);
    case TOURNAMENT:
    case CUSTOM:
    default:
        break;
    }

    // If there is not a compatable bpType then return NOTTAKEN
    return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void gshare_train(uint32_t pc, uint8_t outcome) {
    uint32_t idx = (pc ^ gsHistory) & ((1 << ghistoryBits) - 1);
    if (outcome)
        gsTable[idx] = min(3, gsTable[idx] + 1);
    else
        gsTable[idx] = max(0, gsTable[idx] - 1);
    gsHistory = (gsHistory << 1) | outcome;
}
void train_predictor(uint32_t pc, uint8_t outcome) {
    //
    // TODO: Implement Predictor training
    //
    switch (bpType) {
    case GSHARE:
        gshare_train(pc, outcome);
    case TOURNAMENT:
    case CUSTOM:
    default:
        break;
    }
}
