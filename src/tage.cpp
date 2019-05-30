#include "tage.h"

tage_predictor_t::base_predictor_t::bimodal_entry_t::bimodal_entry_t() {
    hyst = 0;
    pred = 0;
}
tage_predictor_t::bank_t::bank_entry_t::bank_entry_t() {
    cnt = 0;
    tag = 0;
    ubit = 0;
}
void tage_predictor_t::bank_t::compressed_history_t::init(int glen, int tlen) {
    compressed = 0;
    geometry_len = glen;
    target_len = tlen;
}

void tage_predictor_t::bank_t::compressed_history_t::update(history_t h) {
    int new_compressed = (compressed << 1) | h[0];
    new_compressed ^= h[geometry_len] << (geometry_len % target_len);
    new_compressed ^= new_compressed >> target_len;
    new_compressed &= (1 << target_len) - 1;
    compressed = new_compressed;
}

void tage_predictor_t::init() {
    tick = 0;
    PWIN = 8;
    geometries[0] = MAX_HISTORY - 1;
    geometries[NUM_BANKS - 1] = MIN_HISTORY;
    for (int i = 1; i < NUM_BANKS - 1; i++) {
        geometries[NUM_BANKS - i - 1] =
            int((double)MIN_HISTORY *
                    pow((double)(MAX_HISTORY - 1) / (double)MIN_HISTORY,
                        (double)i / (double)(NUM_BANKS - 1)) +
                0.5);
    }
    for (int i = 0; i < NUM_BANKS; i++) {
        banks[i].geometry = geometries[i];
        banks[i].compressed_index.init(banks[i].geometry, LOG_GLOBAL);
        banks[i].compressed_tag[0].init(banks[i].geometry,
                                        TAG_BITS - ((i + (NUM_BANKS & 1)) / 2));
        banks[i].compressed_tag[1].init(
            banks[i].geometry, TAG_BITS - ((i + (NUM_BANKS & 1)) / 2) - 1);
    }
}
uint16_t tage_predictor_t::get_tag(uint32_t pc, int i) {
    int tag = pc ^ (banks[i].compressed_tag[0].compressed) ^
              (banks[i].compressed_tag[1].compressed << 1);
    return tag & ((1 << (TAG_BITS - ((i + (NUM_BANKS & 1)) / 2))) - 1);
}
int tage_predictor_t::F(int A, int size, int bank) {
    int A1, A2;

    A = A & ((1 << size) - 1);
    A1 = (A & ((1 << LOG_GLOBAL) - 1));
    A2 = (A >> LOG_GLOBAL);
    A2 = ((A2 << bank) & ((1 << LOG_GLOBAL) - 1)) + (A2 >> (LOG_GLOBAL - bank));
    A = A1 ^ A2;
    A = ((A << bank) & ((1 << LOG_GLOBAL) - 1)) + (A >> (LOG_GLOBAL - bank));
    return A;
}
int tage_predictor_t::get_global_index(uint32_t pc, int i) {
    int index = pc ^ (pc >> (LOG_GLOBAL - (NUM_BANKS - i - 1))) ^
                banks[i].compressed_index.compressed ^
                F(path_history, min(16, banks[i].geometry), i);
    return index & ((1 << LOG_GLOBAL) - 1);
}
uint8_t tage_predictor_t::bimodal_predict(uint32_t pc) {
    return base_predictor.entries[pc & ((1 << LOG_BIMODAL) - 1)].pred > 0
               ? TAKEN
               : NOTTAKEN;
}
uint8_t tage_predictor_t::predict(uint32_t pc) {
    pri_bank = alt_bank = NUM_BANKS;
    pri_pred = alt_pred = NOTTAKEN;
    for (int i = 0; i < NUM_BANKS; i++) {
        tags[i] = get_tag(pc, i);
        global_indexes[i] = get_global_index(pc, i);
    }
    for (int i = 0; i < NUM_BANKS; i++) {
        if (banks[i].entries[global_indexes[i]].tag == tags[i]) {
            pri_bank = i;
            break;
        }
    }
    for (int i = pri_bank + 1; i < NUM_BANKS; i++) {
        if (banks[i].entries[global_indexes[i]].tag == tags[i]) {
            alt_bank = i;
            break;
        }
    }
    if (pri_bank < NUM_BANKS) {
        if (alt_bank < NUM_BANKS) {
            alt_pred =
                banks[alt_bank].entries[global_indexes[alt_bank]].cnt >= 0
                    ? TAKEN
                    : NOTTAKEN;
        } else {
            alt_pred = bimodal_predict(pc);
        }
        const bank_t::bank_entry_t &entry =
            banks[pri_bank].entries[global_indexes[pri_bank]];
        pri_pred = (entry.cnt >= 0 ? TAKEN : NOTTAKEN);
        if (PWIN < 0 || abs(2 * entry.cnt + 1) != 1 || entry.ubit != 0) {
            return pred = pri_pred;
        } else {
            return pred = alt_pred;
        }
    } else {
        return pred = bimodal_predict(pc);
    }
}
void tage_predictor_t::train(uint32_t pc, uint8_t outcome) {
    bool alloc = (pred == outcome) && pri_bank > 0;
    if (pri_bank < NUM_BANKS) {
        const bank_t::bank_entry_t &entry =
            banks[pri_bank].entries[global_indexes[pri_bank]];
        uint8_t local_pred = entry.cnt >= 0 ? TAKEN : NOTTAKEN;
        bool pseudo_new_alloc = abs(2 * entry.cnt + 1) == 1 && entry.ubit == 0;
        if (pseudo_new_alloc) {
            if (local_pred == outcome)
                alloc = false;
            if (local_pred != alt_pred) {
                if (alt_pred == outcome) {
                    if (PWIN < 7)
                        PWIN++;
                } else if (PWIN > -8) {
                    PWIN--;
                }
            }
        }
    }
    if (alloc) {
        int8_t mn = 3;
        for (int i = 0; i < pri_bank; i++) {
            mn = min(mn, banks[i].entries[global_indexes[i]].ubit);
        }
        if (mn > 0) {
            for (int i = pri_bank - 1; i >= 0; i--) {
                banks[i].entries[global_indexes[i]].ubit--;
            }
        } else {
            int Y = rand() & ((1 << (pri_bank - 1)) - 1);
            int X = pri_bank - 1;
            while (Y & 1) {
                Y >>= 1;
                X--;
            }
            for (int i = X; i >= 0; i--) {
                bank_t::bank_entry_t &entry =
                    banks[i].entries[global_indexes[i]];
                if (entry.ubit == mn) {
                    entry.tag = get_tag(pc, i);
                    entry.cnt = outcome == TAKEN ? 0 : -1;
                    entry.ubit = 0;
                    break;
                }
            }
        }
    }
    tick++;
    if (!(tick & ((1 << 18) - 1))) {
        int X = (tick >> 18) & 1;
        if (!X)
            X = 2;
        for (int i = 0; i < NUM_BANKS; i++) {
            for (int j = 0; j < (1 << LOG_GLOBAL); j++) {
                banks[i].entries[j].ubit &= X;
            }
        }
    }
    if (pri_bank < NUM_BANKS) {
        int8_t cnt = banks[pri_bank].entries[global_indexes[pri_bank]].cnt;
        if (outcome == TAKEN) {
            cnt = min(cnt + 1, (1 << CNT_BITS) - 1);
        } else {
            cnt = max(cnt - 1, -(1 << CNT_BITS));
        }
        banks[pri_bank].entries[global_indexes[pri_bank]].cnt = cnt;
    } else {
        bimodal_train(pc, outcome);
    }
    if (pred != alt_pred) {
        bank_t::bank_entry_t &entry =
            banks[pri_bank].entries[global_indexes[pri_bank]];
        if (pred == outcome) {
            entry.ubit = min(3, int(entry.ubit) + 1);
        } else {
            entry.ubit = max(0, int(entry.ubit) - 1);
        }
    }
    global_history = (global_history << 1) | history_t(outcome == TAKEN);
    path_history = (path_history << 1) + (pc & 1);
    path_history &= (1 << 16) - 1;
    for (int i = 0; i < NUM_BANKS; i++) {
        banks[i].compressed_index.update(global_history);
        banks[i].compressed_tag[0].update(global_history);
        banks[i].compressed_tag[1].update(global_history);
    }
}
void tage_predictor_t::bimodal_train(uint32_t pc, uint8_t outcome) {
    int index = pc & ((1 << LOG_BIMODAL) - 1);
    // printf("%d %d\n", outcome, bimodal_predict(pc));
    if (outcome == bimodal_predict(pc)) {
        if (base_predictor.entries[index].pred)
            base_predictor.entries[index >> 2].hyst = outcome == TAKEN;
    } else {
        int inter = (base_predictor.entries[index].pred << 1) +
                    int(base_predictor.entries[index >> 2].hyst);
        if (outcome == TAKEN) {
            inter = min(3, inter + 1);
        } else {
            inter = max(0, inter - 1);
        }
        base_predictor.entries[index].pred = inter >> 1;
        // printf("inter %d\n", inter >> 1);
        base_predictor.entries[index >> 2].hyst = inter & 1;
    }
}