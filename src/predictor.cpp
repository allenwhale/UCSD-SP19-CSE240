//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include "predictor.h"
#include "tage.h"
#include <algorithm>
#include <map>
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

char *gTable, *lTable, *choiceTable;

uint32_t gHistory, *lHistory;
tage_predictor_t tage;
//
// TODO: Add your own Branch Predictor data structures here
//

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

void gshare_init() {
    gTable = new char[1 << ghistoryBits];
    memset(gTable, 1, sizeof(char) << ghistoryBits);
    gHistory = 0;
}
void tournament_init() {
    gTable = new char[1 << ghistoryBits];
    memset(gTable, 1, sizeof(char) << ghistoryBits);
    lTable = new char[1 << lhistoryBits];
    memset(lTable, 1, sizeof(char) << lhistoryBits);
    lHistory = new uint32_t[1 << pcIndexBits];
    memset(lHistory, 0, sizeof(uint32_t) << pcIndexBits);
    choiceTable = new char[1 << ghistoryBits];
    memset(choiceTable, 2, sizeof(char) << ghistoryBits);
}
void init_predictor() {
    //
    // TODO: Initialize Branch Predictor Data Structures
    //
    // gshare init
    switch (bpType) {
    case STATIC:
        break;
    case GSHARE:
        gshare_init();
        break;
    case TOURNAMENT:
        tournament_init();
        break;
    case CUSTOM:
        tage.init();
    default:
        break;
    }

    // tournament init
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t gshare_predictor(uint32_t pc) {
    uint32_t idx = (pc ^ gHistory) & ((1 << ghistoryBits) - 1);
    return gTable[idx] > 1 ? TAKEN : NOTTAKEN;
}
uint8_t tournament_predictor(uint32_t pc) {
    uint8_t pred;
    if (choiceTable[gHistory] < 2) {
        // local
        pred = lTable[lHistory[pc & ((1 << pcIndexBits) - 1)]];
    } else {
        // global
        pred = gTable[gHistory];
    }
    return pred > 1 ? TAKEN : NOTTAKEN;
}
uint8_t custom_predictor(uint32_t pc) { return tage.predict(pc); }
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
        return tournament_predictor(pc);
    case CUSTOM:
        return custom_predictor(pc);
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
    uint32_t idx = (pc ^ gHistory) & ((1 << ghistoryBits) - 1);
    if (outcome)
        gTable[idx] = min(3, gTable[idx] + 1);
    else
        gTable[idx] = max(0, gTable[idx] - 1);
    gHistory = ((gHistory << 1) | outcome) & ((1 << ghistoryBits) - 1);
}
void tournament_train(uint32_t pc, uint8_t outcome) {
    // char choice = choiceTable[gHistory];
    char gPred = gTable[gHistory] > 1 ? TAKEN : NOTTAKEN;
    int lIndex = lHistory[pc & ((1 << pcIndexBits) - 1)];
    char lPred = lTable[lIndex] > 1 ? TAKEN : NOTTAKEN;
    // update choice table
    if (gPred == outcome && lPred != outcome)
        choiceTable[gHistory] = min(3, choiceTable[gHistory] + 1);
    else if (gPred != outcome && lPred == outcome)
        choiceTable[gHistory] = max(0, choiceTable[gHistory] - 1);
    // update pred table
    if (outcome) {
        gTable[gHistory] = min(3, gTable[gHistory] + 1);
        lTable[lIndex] = min(3, lTable[lIndex] + 1);
    } else {
        gTable[gHistory] = max(0, gTable[gHistory] - 1);
        lTable[lIndex] = max(0, lTable[lIndex] - 1);
    }
    // update history
    gHistory = ((gHistory << 1) | outcome) & ((1 << ghistoryBits) - 1);
    lHistory[pc & ((1 << pcIndexBits) - 1)] =
        ((lHistory[pc & ((1 << pcIndexBits) - 1)] << 1) | outcome) &
        ((1 << lhistoryBits) - 1);
}
void custom_train(uint32_t pc, uint8_t outcome) { tage.train(pc, outcome); }
void train_predictor(uint32_t pc, uint8_t outcome) {
    //
    // TODO: Implement Predictor training
    //
    switch (bpType) {
    case GSHARE:
        gshare_train(pc, outcome);
        break;
    case TOURNAMENT:
        tournament_train(pc, outcome);
        break;
    case CUSTOM:
        custom_train(pc, outcome);
        break;
    default:
        break;
    }
}
