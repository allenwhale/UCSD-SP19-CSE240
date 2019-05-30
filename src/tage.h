#ifndef __TAGE__H__
#define __TAGE__H__
#include "predictor.h"
#include <bits/stdc++.h>
using namespace std;

#define NUM_BANKS 13
#define LOG_BIMODAL 12
#define LOG_GLOBAL 9
#define MAX_HISTORY 131
#define MIN_HISTORY 5
#define TAG_BITS 10
#define CNT_BITS 3
struct tage_predictor_t {
    typedef bitset<MAX_HISTORY> history_t;
    struct base_predictor_t {
        struct bimodal_entry_t {
            int8_t hyst;
            int8_t pred;
            bimodal_entry_t();
        };
        bimodal_entry_t entries[1 << LOG_BIMODAL];
    } base_predictor;
    struct bank_t {
        int geometry;
        struct bank_entry_t {
            int8_t cnt;
            uint16_t tag;
            int8_t ubit;
            bank_entry_t();
        };
        struct compressed_history_t {
            int geometry_len;
            int target_len;
            uint32_t compressed;
            void init(int, int);
            void update(history_t);
        };
        bank_entry_t entries[1 << LOG_GLOBAL];
        compressed_history_t compressed_index;
        compressed_history_t compressed_tag[2];
    };
    int PWIN, tick;
    bank_t banks[NUM_BANKS];
    history_t global_history;
    int path_history;
    int geometries[NUM_BANKS];
    uint16_t tags[NUM_BANKS];
    int global_indexes[NUM_BANKS];
    int pri_bank, alt_bank;
    uint8_t pri_pred, alt_pred;
    uint8_t pred;
    void init();
    uint16_t get_tag(uint32_t, int);
    int F(int, int, int);
    int get_global_index(uint32_t, int);
    uint8_t predict(uint32_t);
    uint8_t bimodal_predict(uint32_t);
    void train(uint32_t, uint8_t);
    void bimodal_train(uint32_t, uint8_t);
};

#endif