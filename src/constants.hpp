#pragma once

#include <string>

// -- Game Balance Constants -- //
const double COST_SCALE_FACTOR = 1.15;
const double BUFF_COST_SCALE_FACTOR = 1.5;
const double LPS_TO_CLICK_COST_SCALE_FACTOR = 1.8;
const double AUTOSAVE_INTERVAL = 30.0;
const double CACHE_BUFF_DURATION = 10.0;
const double CACHE_BUFF_PERCENT = 777.0;

// -- System Constants -- //
#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

const int VERSION = 1;
const std::string SAVE_FILE_NAME = "save_data.json";
const int EYE_CANDY_LOG_SIZE = 4;