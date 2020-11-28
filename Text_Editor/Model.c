//
// Created by Алексей on 16.10.2020.
//
#include "Model.h"

#include "LoadFile.h"


typedef char* (*ReadFileFunPtr) (const char* fileName);
/**
 * @brief Splits the text of the file line by line and counts their length.
 * @param data Data connected with text.
 * @return If the function finished successfully.
 */
static BOOLEAN SplitStrings(Model* data){
    if(data == NULL || data->strings == NULL) return 0;
    UINT curPos = 0;
    while(data->strings[curPos] != 0){
        if(data->strings[curPos] == '\t'){
            data->strings[curPos] = ' ';
        }
        if(data->strings[curPos] == '\n') {
            UINT *tmp = realloc(data->lengths, sizeof(UINT) * ++data->linesNumber);
            if (tmp == NULL) {
                free(data->lengths);
                data->lengths = NULL;
                return FALSE;
            } else {
                data->lengths = tmp;
                data->lengths[data->linesNumber - 1] = curPos + 1;
                if(data->linesNumber - 1 == 0) {
                    data->maxLen = data->lengths[data->linesNumber - 1];
                } else {
                    data->maxLen = max(data->maxLen,
                                       data->lengths[data->linesNumber - 1] - data->lengths[data->linesNumber - 2]);
                }
            }
        }
        curPos++;
    }
    return TRUE;
}

BOOLEAN InitModel(char const * const fileName, Model* const modelPtr){
    char* tmp = LoadFile(fileName);

    if(tmp != NULL){
        modelPtr->strings = tmp;
        modelPtr->linesNumber = 0;
        modelPtr->lengths = NULL;
        modelPtr->maxLen = 0;
        if(SplitStrings(modelPtr) == 1){
            return TRUE;
        }
    } else {
        return FALSE;
    }
    return FALSE;
}

string_t getString(Model const * const modelPtr, size_t idx){
    string_t str;
    str.length = modelPtr->lengths[idx] - (idx == 0 ? 0 : modelPtr->lengths[idx - 1]);
    str.string = modelPtr->strings + (idx == 0 ? 0 : modelPtr->lengths[idx - 1]);

    return str;
}

void DeleteModel(Model* modelPtr){
    if(modelPtr == NULL) return;
    free(modelPtr->strings);
    free(modelPtr->lengths);
    free(modelPtr);
}

