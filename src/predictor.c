//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Mohit";
const char *studentID   = "A59003135";
const char *email       = "m4jain@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits = 12; // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;
int lhistoryBits = 12; // Number of bits used for Local History
int pcindexBits = 8;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//gshare
uint8_t *bht_gshare;
uint32_t ghistory;

uint32_t *pht_local;
uint8_t *bht_local;
uint8_t *choicePT;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

//gshare functions
void init_gshare() {
 int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

void init_pshare()
{
  int pht_entries = 1 << pcindexBits;
  int bht_entries = 1 << lhistoryBits;

  pht_local = (uint32_t*)malloc(pht_entries * sizeof(uint32_t));
  bht_local = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));

  for(int i = 0; i < pht_entries; ++i)
  {
    pht_local[i] = 0;
  }

  for(int i = 0; i < bht_entries; ++i)
  {
    bht_local[i] = WN;
  }
}

void init_tournament()
{
  init_gshare();
  init_pshare();
  int choice_pt_entries = 1 << ghistoryBits;
  choicePT = (uint8_t*)malloc(choice_pt_entries * sizeof(uint8_t));
}

uint8_t 
gshare_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

uint8_t pshare_predict(uint32_t pc)
{
  uint32_t pht_entries = 1 << pcindexBits;
  uint32_t pc_lower_bits = pc & (pht_entries-1);
  uint64_t local_history = pht_local[pc_lower_bits];

  uint32_t bht_entries = 1 << lhistoryBits;
  pc_lower_bits = pc & (bht_entries - 1);
  uint32_t lhistory_lowerBits = local_history & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ lhistory_lowerBits;

  switch(bht_local[index])
  {
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in PSHARE BHT!\n");
          return NOTTAKEN;
  }
}

uint8_t tournament_predict(uint32_t pc)
{
  pc = pc >> 2;
  uint32_t bht_global_index = (ghistory) & ((1 << ghistoryBits) - 1);
  uint8_t choice = choicePT[bht_global_index];
  uint8_t gshare_prediction = gshare_predict(pc);
  uint8_t pshare_prediction = pshare_predict(pc);

  if (choice == WN || choice == SN)
  {
    return gshare_prediction;
  }
  else
  {
    return pshare_prediction;
  }
}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void train_pshare(uint32_t pc, uint8_t outcome)
{
  uint32_t pht_entries = 1 << pcindexBits;
  uint32_t pht_index = pc & (pht_entries-1);
  uint64_t local_history = pht_local[pht_index];

  uint32_t bht_entries = 1 << lhistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t lhistory_lowerBits = local_history & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ lhistory_lowerBits;

  switch(bht_local[index])
  {
    case WN:
      bht_local[index] = (outcome==TAKEN)?WT:SN;
          break;
    case SN:
      bht_local[index] = (outcome==TAKEN)?WN:SN;
          break;
    case WT:
      bht_local[index] = (outcome==TAKEN)?ST:WN;
          break;
    case ST:
      bht_local[index] = (outcome==TAKEN)?ST:WT;
          break;
    default:
      printf("Warning: Undefined state of entry in PSHARE BHT!\n");
  }

  pht_local[pht_index] = ((local_history << 1) | outcome);

}

void train_tournament(uint32_t pc, uint8_t outcome)
{
  pc = pc >> 2;
  uint32_t bht_global_index = (ghistory) & ((1 << ghistoryBits) - 1);
  uint8_t choice = choicePT[bht_global_index];

  uint8_t localOutcome = pshare_predict(pc);
  uint8_t globalOutcome = gshare_predict(pc);

  if (localOutcome != globalOutcome)
  {
    switch(choice)
    {
        case SN:
          choicePT[bht_global_index] = (outcome==globalOutcome)?SN:WN;
            break;
        case WN:
          choicePT[bht_global_index] = (outcome==globalOutcome)?SN:WT;
            break;
        case WT:
          choicePT[bht_global_index] = (outcome==localOutcome)?ST:WN;
              break;
        case ST:
          choicePT[bht_global_index] = (outcome==localOutcome)?ST:WT;
              break;
    }
  }

  train_gshare(pc, outcome);
  train_pshare(pc, outcome);
}

void
cleanup_gshare() {
  free(bht_gshare);
}

void cleanup_pshare()
{
  free(bht_local);
  free(pht_local);
}

void cleanup_tournament()
{
  cleanup_gshare();
  cleanup_pshare();
  free(choicePT);
}



void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tournament();
      break;
    case CUSTOM:
      pcindexBits = 11;
      init_tournament();
      break;
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
      return tournament_predict(pc);
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

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
      break;
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return train_tournament(pc, outcome);
    default:
      break;
  }
  

}
