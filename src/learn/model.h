#ifndef __DATA_MODEL_H
#define __DATA_MODEL_H

#include "../engine/playcall.h"

/* Uses the AI model to call plays based on situation */
PlayCall getPlayCall(Situation *sit);
/* Loads the model file. Must be called before any calls to getPlaycall() */
void initModel();

#endif
