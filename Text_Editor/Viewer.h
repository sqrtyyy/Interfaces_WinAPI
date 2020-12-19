//
// Created by Алексей on 16.10.2020.
//

#ifndef CLIONPROJECTS_VIEWER_H
#define CLIONPROJECTS_VIEWER_H

#define MAX_SCROLL 30000

#include <windows.h>
#include "Model.h"

/**
 * @struct ViewerData
 * @brief Data which is necessary for printing text.
 * @field extraNewLines - Extra new lines for layout.
 * @field currentLineNum - Number of line which is first in current moment.
 * @field curPos - Number of current line with additional lines.
 * @field linesNumberLayOut - Number of lines in file with additional lines.
 * @field curHorizontalShift - Value of shift(number of symbols) lines from right border.
 * @field charsInLine - Maximum number of symbols in one line.
 * @field vScrollMax - Maximum value of vertical scroll bar.
 * @field vScrollMax - Maximum value of horizontal scroll bar.
 * @field linesPerScroll - Number of lines which should be scrolled at once.
 * @field cyChar - Height of symbols.
 * @field cxChar - Average width of symbols.
 */

typedef struct{
    UINT* extraNewLines;
    size_t curExtraLine;

    UINT currentLineNum;

    UINT curPos;

    UINT curVPos;

    UINT linesNumberLayOut;

    UINT curHorizontalShift;
    UINT charsInLine;

    UINT vScrollMax;
    UINT hScrollMax;

    UINT linesPerScroll;

    int cyChar;
    int cxChar;
}ViewerData;
/**
 * @struct Viewer
 * @brief Prints data in different modes.
 * @field print - Function for printing data.
 * @field resizeWindow - Function for resizing window in different modes.
 * @field changeLine - Function for changing current line of file.
 * @field viewerData - Data for printing.
 */
typedef struct {
    void (*print) (Model const *, HDC, ViewerData const *, RECT rect);
    void (*resizeWindow) (Model const * const modelPtr, ViewerData * const viewerDataPtr, int const newHeight, int const newWidth, HWND hwnd);
    BOOLEAN (*changeLine)(ViewerData *viewerDataPtr, int shift);
    ViewerData* viewerData;
}Viewer;
/**
 * @brief Inits viewer.
 * @param mode Mode of printing.
 * @param hdc Descriptor for get text metrics.
 * @param viewerPtr viewer whose fields should be filled.
 * @return If viewer created correctly.
 */
BOOLEAN InitViewer(HDC hdc, Viewer* viewerPtr);
/**
 * @brief Function for shifting text to the left or to the right.
 * @param viewerDataPtr Address of viewer data to change curHorizontalShift.
 * @param shift The shift amount. If shift > 0 text will be shifted to the left else to the right.
 */
void HorizontalShift(ViewerData *viewerDataPtr, int shift);
/**
 * @brief Changes viewer's mode to layout and vice versa.
 * @param viewerPtr
 * @param modelPtr
 * @param hwnd
 */
void ChangeMode(Viewer* viewerPtr, Model const* modelPtr, HWND hwnd);

/**
 * @brief Frees the allocated memory for viewerPtr
 * @param viewerPtr Address of viewer which should be destroyed.
 */

void DeleteViewer(Viewer* viewerPtr);



#endif //CLIONPROJECTS_VIEWER_H
