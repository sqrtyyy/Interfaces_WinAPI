//
// Created by Алексей on 16.10.2020.
//
#include "Viewer.h"
/**
 * @brief Calculates where the additional should be added
 * @param[in] viewerDataPtr Viewer data which should be changed.
 * @param[in] lengths Array of lengths of strings in original text.
 * @param[in] size Number of strings in original text.
 */
static void calculateExtraNewLines(ViewerData * const viewerDataPtr, UINT const * const lengths, size_t const size){
    viewerDataPtr->linesNumberLayOut = size;
    viewerDataPtr->curPos = 0;
    for(size_t i = 0; i < size; i++){
        if(i < viewerDataPtr->currentLineNum ) viewerDataPtr->curPos++;
        viewerDataPtr->extraNewLines[i] = 0;
        size_t len = lengths[i] -  (i == 0 ? 0 : lengths[i - 1]);
        int j = 0;
        while (len > (j + 1) * viewerDataPtr->charsInLine){
            viewerDataPtr->extraNewLines[i]++;
            j++;
            viewerDataPtr->linesNumberLayOut++;
            if(i < viewerDataPtr->currentLineNum ) viewerDataPtr->curPos++;
        }
    }
}
/**
 * @brief Shifts the line if, due to resize, the current additional line ceases to exist
 * @param[in] viewerDataPtr Viewer data
 */
static void shiftExtraNewLine(ViewerData * const viewerDataPtr){
    int curLineExtraNum = viewerDataPtr->extraNewLines[viewerDataPtr->currentLineNum];
    if((int)viewerDataPtr->curExtraLine > curLineExtraNum){
        viewerDataPtr->curPos--;
        viewerDataPtr->curExtraLine = curLineExtraNum;
    }
}
/**
 * @brief Calculate number of strings which should be shifted at once and bounds of scroll.
 * @param[in] viewerDataPtr Viewer data which  linesPerScroll and vScrollMax should be changed.
 * @param[in] linesNum Number of lines in text with additional lines.
 */
static void CalculateScroll(ViewerData* const  viewerDataPtr, UINT const linesNum){
    while(viewerDataPtr->linesPerScroll > 1 &&  linesNum / (viewerDataPtr->linesPerScroll - 1) < MAX_SCROLL){
        viewerDataPtr->linesPerScroll--;
    }
    while (linesNum / viewerDataPtr->linesPerScroll > MAX_SCROLL){
        viewerDataPtr->linesPerScroll++;
    }
    viewerDataPtr->vScrollMax = linesNum / viewerDataPtr->linesPerScroll;
}
/**
 * @brief Shifts the line if viewer in simple mode.
 * @param[in] viewerDataPtr Viewer Data in which curPos and currentLineNum should be changed.
 * @param[in] shift Shift amount.
 * @return If something in viewerDataPtr changed.
 */
static BOOLEAN SimpleShift(ViewerData * const viewerDataPtr, int shift){
    shift *= viewerDataPtr->linesPerScroll;
    if((viewerDataPtr->currentLineNum + shift > viewerDataPtr->vScrollMax * viewerDataPtr->linesPerScroll && shift > 0) ||
            (int)viewerDataPtr->currentLineNum + shift < 0 || shift == 0) return FALSE;
    int additionalShift = viewerDataPtr->curPos % viewerDataPtr->linesPerScroll;
    viewerDataPtr->currentLineNum += shift - additionalShift;
    viewerDataPtr->curPos += shift - additionalShift;
    return TRUE;
}
/**
 * @brief Shifts the line if viewer in simple layout.
 * @param[in] viewerDataPtr Viewer Data in which curPos and currentLineNum should be changed.
 * @param[in] shift Shift amount.
 * @return If something in viewerDataPtr changed.
 */
static BOOLEAN LayOutShift(ViewerData * const viewerDataPtr, int shift){
    if(shift == 0) return FALSE;
    shift *= viewerDataPtr->linesPerScroll;
    UINT tmpCurLine = viewerDataPtr->currentLineNum;
    long tmpExtraLine = viewerDataPtr->curExtraLine;
    UINT tmpPos = viewerDataPtr->curPos;

    int signShift = shift > 0 ? 1 : -1;
    while ((tmpPos/viewerDataPtr->linesPerScroll< viewerDataPtr->vScrollMax || signShift == -1)
    && tmpPos >= 0  && shift != 0){
        if(signShift == 1 && viewerDataPtr->extraNewLines[tmpCurLine] == tmpExtraLine){
            tmpCurLine++;
            tmpExtraLine = -1;
        } else if(signShift == -1 && tmpExtraLine == 0){
            if(tmpCurLine == 0){
                shift = 1;
                break;
            }
            tmpCurLine--;
            tmpExtraLine = viewerDataPtr->extraNewLines[tmpCurLine] + 1;
        }
        tmpExtraLine += signShift;
        tmpPos += signShift;
        shift -= signShift;
    }
    if(shift == 0){
        viewerDataPtr->currentLineNum = tmpCurLine;
        viewerDataPtr->curExtraLine = tmpExtraLine;
        viewerDataPtr->curPos = tmpPos;
    }
    int prevLinesPerScroll = viewerDataPtr->linesPerScroll;
    viewerDataPtr->linesPerScroll = 1;
    while (viewerDataPtr->curPos % prevLinesPerScroll != 0){
        SimpleShift(viewerDataPtr, -1);
    }
    viewerDataPtr->linesPerScroll = prevLinesPerScroll;
    return shift == 0;
}
/**
 * @brief Calculates the resizing in simple mode.
 *
 * Recalculates number of symbols which should be printed, scrolls bounds and etc.
 *
 * @param[in] modelPtr Text data.
 * @param[in] viewerDataPtr Viewer Data.
 * @param[in] newHeight Current height of client area.
 * @param[in] newWidth Current height of client area.
 * @param[in] hwnd Descriptor of current window to invalidate rect.
 */
static void SimpleResize(Model const * const modelPtr, ViewerData * const viewerDataPtr, int const newHeight, int const newWidth, HWND hwnd){
    CalculateScroll(viewerDataPtr, modelPtr->linesNumber);
    viewerDataPtr->vScrollMax = max(0, (int)viewerDataPtr->vScrollMax - newHeight / viewerDataPtr->cyChar / viewerDataPtr->linesPerScroll);

    /*int prevLinesPerScroll = viewerDataPtr->linesPerScroll;
    viewerDataPtr->linesPerScroll = 1;
    while (viewerDataPtr->curPos % prevLinesPerScroll != 0){
        SimpleShift(viewerDataPtr, -1);
    }
    viewerDataPtr->linesPerScroll = prevLinesPerScroll;*/

    int tmp = viewerDataPtr->currentLineNum;
    viewerDataPtr->currentLineNum = min(viewerDataPtr->currentLineNum, viewerDataPtr->vScrollMax * viewerDataPtr->linesPerScroll);
    viewerDataPtr->curPos = viewerDataPtr->currentLineNum;
    if(viewerDataPtr->currentLineNum != tmp) {
        InvalidateRect(hwnd, NULL, TRUE);
    }
    viewerDataPtr->hScrollMax = max(0, (int)modelPtr->maxLen - newWidth / viewerDataPtr->cxChar);
    viewerDataPtr->curHorizontalShift = min(viewerDataPtr->curHorizontalShift, viewerDataPtr->hScrollMax);

    tmp = viewerDataPtr->charsInLine;
    viewerDataPtr->charsInLine = newWidth / viewerDataPtr->cxChar;
    if(viewerDataPtr->charsInLine != tmp) {
        InvalidateRect(hwnd, NULL, TRUE);
    }

    SetScrollRange(hwnd, SB_VERT, 0, viewerDataPtr->vScrollMax, FALSE);
    SetScrollPos(hwnd, SB_VERT, viewerDataPtr->curPos / viewerDataPtr->linesPerScroll, TRUE);

    SetScrollRange(hwnd, SB_HORZ, 0, viewerDataPtr->hScrollMax, FALSE);
    SetScrollPos(hwnd, SB_HORZ, viewerDataPtr->curHorizontalShift, TRUE);

}
/**
 * @brief Calculates the resizing in layout mode.
 *
 * Recalculates number of symbols which should be printed, scrolls bounds and etc.
 *
 * @param[in] modelPtr Text data.
 * @param[in] viewerDataPtr Viewer Data.
 * @param[in] newHeight Current height of client area.
 * @param[in] newWidth Current height of client area.
 * @param[in] hwnd Descriptor of current window to invalidate rect.
 */
static void LayOutResize(Model const * const modelPtr, ViewerData * const viewerDataPtr, int const newHeight, int const newWidth, HWND hwnd){
    if(abs((int)viewerDataPtr->charsInLine - newWidth / viewerDataPtr->cxChar) >= 1
    || viewerDataPtr->linesNumberLayOut == 0) {
        viewerDataPtr->charsInLine = newWidth / viewerDataPtr->cxChar;
        calculateExtraNewLines(viewerDataPtr, modelPtr->lengths, modelPtr->linesNumber);
        shiftExtraNewLine(viewerDataPtr);
    }
    CalculateScroll(viewerDataPtr, viewerDataPtr->linesNumberLayOut);
    /*int prevLinesPerScroll = viewerDataPtr->linesPerScroll;
    viewerDataPtr->linesPerScroll = 1;
    while (viewerDataPtr->curPos % prevLinesPerScroll != 0){
        LayOutShift(viewerDataPtr, -1);
    }
    viewerDataPtr->linesPerScroll = prevLinesPerScroll;*/
    viewerDataPtr->vScrollMax = max(0, viewerDataPtr->vScrollMax - newHeight / viewerDataPtr->cyChar / viewerDataPtr->linesPerScroll);
    while (viewerDataPtr->curPos / viewerDataPtr->linesPerScroll > viewerDataPtr->vScrollMax){
        LayOutShift(viewerDataPtr, -1);
    }
    SetScrollRange(hwnd, SB_VERT, 0, viewerDataPtr->vScrollMax, FALSE);
    SetScrollPos(hwnd, SB_VERT, viewerDataPtr->curPos / viewerDataPtr->linesPerScroll, TRUE);
    InvalidateRect(hwnd, NULL, TRUE);
}
/**
 * @brief Prints text in simple mode.
 * @param[in] modelPtr Text data.
 * @param[in] hdc Descriptor of context for TextOut
 * @param[in] viewerDataPtr Information about printing.
 * @param[in] rect Rectangle in which information should be printed.
 */
static void SimplePrint(Model const * const modelPtr, HDC hdc, ViewerData const * const viewerDataPtr, RECT rect){
    if(modelPtr == NULL || viewerDataPtr == NULL) return;

    int top = rect.top / viewerDataPtr->cyChar;
    int bottom = rect.bottom / viewerDataPtr->cyChar + 1;

    int beginPaint =  max(0, top + viewerDataPtr->currentLineNum);
    int endPaint = min(modelPtr->linesNumber, viewerDataPtr->currentLineNum + bottom);

    for (int i = beginPaint; i < endPaint; i++){
        string_t  curString = getString(modelPtr, i);
        int lineWidth = curString.length - viewerDataPtr->curHorizontalShift;
        int width = min((int)viewerDataPtr->charsInLine, lineWidth);
        if(width <= 0) continue;
        TextOut(hdc, 0, (top + (i - beginPaint)) * viewerDataPtr->cyChar,
                curString.string + viewerDataPtr->curHorizontalShift, width);
    }
}
/**
 * @brief Prints text in layout mode.
 * @param[in] modelPtr Text data.
 * @param[in] hdc Descriptor of context for TextOut
 * @param[in] viewerDataPtr Information about printing.
 * @param[in] rect Rectangle in which information should be printed.
 */
static void LayoutPrint(Model const * const modelPtr, HDC hdc, ViewerData const * const viewerDataPtr, RECT rect) {
    if(modelPtr == NULL || viewerDataPtr == NULL) return;
    int top = rect.top / viewerDataPtr->cyChar;
    int bottom = rect.bottom / viewerDataPtr->cyChar + 1;
    int curLine = viewerDataPtr->currentLineNum;
    int curExtraLine = viewerDataPtr->curExtraLine;
    int tmpTop = top;

    while (tmpTop != 0){
        if(viewerDataPtr->extraNewLines[curLine] == curExtraLine){
            curExtraLine = 0;
            curLine++;
        } else {
            curExtraLine++;
        }
        tmpTop--;
    }

    for(int i = top; i < bottom; i++){
        if(curLine == modelPtr->linesNumber) break;
        string_t line = getString(modelPtr, curLine);
        int width = min(viewerDataPtr->charsInLine, line.length - curExtraLine * viewerDataPtr->charsInLine);
        TextOut(hdc, 0, i * viewerDataPtr->cyChar,
                line.string + curExtraLine * viewerDataPtr->charsInLine, width);
        curExtraLine++;
        if((int)viewerDataPtr->extraNewLines[curLine] < curExtraLine){
            curExtraLine = 0;
            curLine++;
        }
    }
}

BOOLEAN InitViewer(HDC hdc, Viewer* viewerPtr){
    ViewerData* tmp = calloc(1, sizeof(ViewerData));
    if(tmp == NULL) return FALSE;
    viewerPtr->viewerData = tmp;
    viewerPtr->print = SimplePrint;
    viewerPtr->resizeWindow = SimpleResize;
    viewerPtr->changeLine = SimpleShift;
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    viewerPtr->viewerData->cxChar = tm.tmAveCharWidth;
    viewerPtr->viewerData->cyChar = tm.tmHeight + tm.tmExternalLeading;
    viewerPtr->viewerData->linesPerScroll = 1;
    return TRUE;
}

void HorizontalShift(ViewerData* const viewerDataPtr, int const shift){
    if(shift < 0 && (int)viewerDataPtr->curHorizontalShift < -shift ||
            viewerDataPtr->curHorizontalShift  + shift > viewerDataPtr->hScrollMax) return;
    viewerDataPtr->curHorizontalShift += shift;
}

void ChangeMode(Viewer* const viewerPtr, Model const * const modelPtr, HWND hwnd){
    if(viewerPtr->print != SimplePrint){
        free(viewerPtr->viewerData->extraNewLines);
        viewerPtr->viewerData->extraNewLines = 0;
        viewerPtr->print = SimplePrint;
        viewerPtr->changeLine = SimpleShift;
        viewerPtr->resizeWindow = SimpleResize;
        viewerPtr->viewerData->curPos = viewerPtr->viewerData->currentLineNum;
    } else {
        viewerPtr->print = LayoutPrint;
        viewerPtr->resizeWindow = LayOutResize;
        viewerPtr->changeLine = LayOutShift;
        viewerPtr->viewerData->extraNewLines = calloc(modelPtr->linesNumber, sizeof(UINT));
        viewerPtr->viewerData->curExtraLine = 0;
        viewerPtr->viewerData->linesNumberLayOut = 0;
        viewerPtr->viewerData->curHorizontalShift = 0;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    viewerPtr->resizeWindow(modelPtr, viewerPtr->viewerData, height, width, hwnd);

    if(viewerPtr->print != SimplePrint){
        SetScrollRange(hwnd, SB_HORZ, 0, 0, FALSE);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void DeleteViewer(Viewer* viewerPtr){
    if(viewerPtr == NULL || viewerPtr->viewerData == NULL) return;
    free(viewerPtr->viewerData->extraNewLines);
    free(viewerPtr->viewerData);
    free(viewerPtr);
}
