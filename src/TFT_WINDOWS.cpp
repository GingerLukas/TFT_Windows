#include "TFT_WINDOWS.h"

WindowContent::WindowContent(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _fntColor = fntColor;
    _fgColor = fgColor;
    _bgColor = bgColor;
    resetFlags();
}
//void WindowContent::paint(TFT_HX8357 *tft, Window *parent, bool force);
void WindowContent::setPositionAndSize(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    positionOrSizeChanged = true;
}
void WindowContent::setBgColor(uint16_t bgColor)
{
    if (_bgColor != bgColor)
    {
        _bgColor = bgColor;
        bgChanged = true;
    }
}
uint16_t WindowContent::getBgColor()
{
    if (_bgColor == TFT_PARENT)
    {
        return _parent->getBgColor();
    }
    return _bgColor;
}
void WindowContent::setFgColor(uint16_t fgColor)
{
    if (_fgColor != fgColor)
    {
        _fgColor = fgColor;
        fgColorChanged = true;
    }
}
void WindowContent::setFntColor(uint16_t fntColor)
{
    if (_fntColor != fntColor)
    {
        _fntColor = fntColor;
        fntColorChanged = true;
    }
}
uint16_t WindowContent::getFgColor()
{
    return _fgColor;
}
uint16_t WindowContent::getFntColor()
{
    return _fntColor;
}
void WindowContent::resetFlags()
{
    bgChanged = false;
    fgColorChanged = false;
    fntColorChanged = false;
}
bool WindowContent::checkFlags()
{
    return bgChanged || fgColorChanged || fntColorChanged;
}
void WindowContent::setParent(Window *parent)
{
    _parent = parent;
}
Window *WindowContent::getParent()
{
    return _parent;
}

ArcGraph::ArcGraph(uint16_t *source, int16_t maxValue, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor)
    : WindowContent(x, y, w, h, fntColor, fgColor, bgColor)
{
    _source = source;
    _maxValue = maxValue;
    _currentSegment = 0;
    _lastSourceValue = 0xFFFF;
    _unit = "%";
}
void ArcGraph::setMaxValue(int16_t maxValue)
{
    _maxValue = maxValue;
}
void ArcGraph::paint(TFT_HX8357 *tft, bool force = false)
{
    //check if repaint is need
    if (!force && *_source == _lastSourceValue && !WindowContent::checkFlags())
    {
        return;
    }

    if (force)
    {
        _currentSegment = 0;
    }

    //find border
    uint16_t r = _h;
    if (_w < r)
        r = _w;
    r = r / 2;

    //progress normalization
    float f = (float)*_source / (float)_maxValue;
    uint16_t x0 = _x + _w / 2;
    uint16_t y0 = _y + _h / 2;
    uint16_t nextSegment = f * 120;

    //text inside
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(getFntColor(), getBgColor());
    tft->setTextSize(1);
    tft->drawString((char *)("  " + String(f * 100, 0) + _unit + "  ").c_str(), x0, y0, 4);

    //draw arc
    int16_t w = 40;
    if (nextSegment > _currentSegment)
    {
        tft->fillArc(x0, y0, _currentSegment * 3, nextSegment - _currentSegment, r, r, w, getFgColor());
    }
    else if (nextSegment < _currentSegment)
    {
        tft->fillArc(x0, y0, nextSegment * 3, _currentSegment - nextSegment, r, r, w, getBgColor());
    }

    //tft->fillArc(x0,y0,0,nextSegment,r,r,w,_fgColor);
    //tft->fillArc(x0,y0,(int16_t)(f*360),120-nextSegment),r,r,w,tmp_bg);

    _currentSegment = nextSegment;

    //flag reset
    _lastSourceValue = *_source;
    WindowContent::resetFlags();
}
void ArcGraph::setUnit(char *c)
{
    _unit = c;
}

BarGraph::BarGraph(uint16_t *source, int16_t maxValue, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor)
    : WindowContent(x, y, w, h, fntColor, fgColor, bgColor)
{
    _source = source;
    _maxValue = maxValue;
    _lastSourceValue = 0xFFFF;
    _currentX = 0;
}
void BarGraph::setMaxValue(int16_t maxValue)
{
    _maxValue = maxValue;
}
void BarGraph::paint(TFT_HX8357 *tft, bool force = false)
{
    //check if repaint is need
    if (!force && *_source == _lastSourceValue && !WindowContent::checkFlags())
        return;
    if (force)
        _currentX = 0;

    //progress normalization
    float f = (float)*_source / (float)_maxValue;
    uint16_t next_x = f * _w;
    if (next_x == _currentX)
        return;
    uint16_t next_global_x = next_x + _x;

    tft->setTextSize(1);

    String text = (" " + String(f * 100, 0) + "% ");
    uint16_t textWidth = tft->textWidth((char *)text.c_str(), 4);

    //debug(tft,String(textWidth).c_str());
    //delay(1000);
    //debug(tft,String(next_x).c_str());

    if (next_x < textWidth)
    {
        tft->fillRect(next_global_x, _y, textWidth, _h, getBgColor()); //clear
        tft->fillRect(_x, _y, next_x, _h, getFgColor());               //new bar

        tft->setTextDatum(ML_DATUM);
        tft->setTextColor(getFntColor());
        tft->drawString((char *)text.c_str(), _x, _y + _h / 2, 4);
    }
    else if (next_x > _currentX)
    {
        uint16_t offset = _currentX < textWidth ? 0 : textWidth;
        tft->fillRect(_currentX + _x - offset, _y, next_x - _currentX + offset, _h, getFgColor()); //clear old text + expand to new position

        tft->setTextDatum(MR_DATUM);
        tft->setTextColor(getFntColor(), getFgColor());
        tft->drawString((char *)text.c_str(), next_global_x, _y + _h / 2, 4);
    }
    else if (next_x < _currentX)
    {
        tft->fillRect(next_global_x, _y, _currentX - next_x + 1, _h, getBgColor()); //reduce to new position

        tft->setTextDatum(MR_DATUM);
        tft->setTextColor(getFntColor(), getFgColor());
        tft->drawString((char *)text.c_str(), next_global_x, _y + _h / 2, 4);
    }

    _currentX = next_x;

    //flag reset
    _lastSourceValue = *_source;
    WindowContent::resetFlags();
}

TextBox::TextBox(char *textSource, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor)
    : WindowContent(x, y, w, h, fntColor, fgColor, bgColor)
{
    _text = textSource;
    _textSize = strlen(textSource);
}
void TextBox::paint(TFT_HX8357 *tft, bool force = false)
{
    //check if repaint is need
    if (!force && !WindowContent::checkFlags())
    {
        return;
    }

    tft->setTextSize(1);
    tft->setTextColor(getFntColor(), getBgColor());
    tft->drawString(_text, _x, _y, 4);
}

Window::Window(char *name, uint16_t headerColor, uint16_t headerBgColor, uint16_t bgColor)
{
    _name = name;

    _headerColor = headerColor;
    _headerBgColor = headerBgColor;
    _bgColor = bgColor;

    headerChanged = true;
    contentChanged = true;
    bgChanged = true;

    _y0 = 30;
    _currentContentPointer = 0;
    _contentSize = 8;

    _contentArray = new WindowContent *[_contentSize];
}
Window::~Window()
{
    delete[] _contentArray;
    _contentArray = NULL;
}
bool Window::addContent(WindowContent *content)
{
    if (_currentContentPointer == _contentSize)
        return false;
    _contentArray[_currentContentPointer++] = content;
    content->setParent(this);
    contentChanged = true;
    return true;
}
bool Window::removeContentAt(uint16_t index, bool deletePointer = false)
{
    if (index < 0 || index >= _contentSize)
        return false;
    if (deletePointer)
    {
        delete _contentArray[index];
    }
    _contentArray[index] = NULL;
    _currentContentPointer--;
    for (uint16_t i = index; i < _currentContentPointer; i++)
    {
        _contentArray[i] = _contentArray[i + 1];
    }
    contentChanged = true;
    return true;
}
void Window::paint(TFT_HX8357 *tft, bool force = false)
{
    if (bgChanged || contentChanged)
    {
        tft->fillRect(0, _y0, WIDTH, HEIGTH - _y0, getBgColor());
        bgChanged = false;
    }
    for (uint16_t i = 0; i < _currentContentPointer; i++)
    {
        _contentArray[i]->paint(tft, contentChanged || force);
    }
    contentChanged = false;
    headerChanged = false;
}
void Window::setHeaderColor(uint16_t fontColor, uint16_t bgColor)
{
    if (_headerColor != fontColor)
    {
        _headerColor = fontColor;
        headerChanged = true;
    }
    if (_headerBgColor != bgColor)
    {
        _headerBgColor = bgColor;
        headerChanged = true;
    }
}
void Window::setBgColor(uint16_t bgColor)
{
    if (_bgColor != bgColor)
    {
        _bgColor = bgColor;
        bgChanged = true;
    }
}
uint16_t Window::getBgColor()
{
    return _bgColor;
}
uint16_t Window::getHeaderColor()
{
    return _headerColor;
}
uint16_t Window::getHeaderBgColor()
{
    return _headerBgColor;
}
void Window::setName(char *name)
{
    _name = name;
    headerChanged = true;
}
char *Window::getName()
{
    return _name;
}
void Window::prepareForChange(Window *prevWindow)
{
    contentChanged = true;
}
bool Window::checkFlags()
{
    return headerChanged || contentChanged || bgChanged;
}
void Window::setParent(WindowsManager *parent)
{
    _parent = parent;
}
WindowsManager *Window::getParent()
{
    return _parent;
}

WindowsManager::WindowsManager(uint16_t maxWindows, uint16_t refreshRate, uint16_t headerHeigth)
{

    setRefreshRate(refreshRate);
    _lastRefreshTime = 0;
    _globalRefreshTime = 3600000 / refreshRate; //every 3600 frames
    _lastGlobalRefreshTime = 0;

    _activeWindowIndex = 0;
    _currentWindowsPointer = 0;
    _windowsSize = maxWindows;
    _windows = new Window *[_windowsSize];

    _headerHeigth = 30;
}
WindowsManager::~WindowsManager()
{
    delete[] _windows;
    _windows = NULL;
}
bool WindowsManager::checkFlags()
{
    for (uint16_t i = 0; i < _currentWindowsPointer; i++)
    {
        if (_windows[i]->checkFlags())
            return true;
    }

    return _windowsChanged || _activeWindowChanged || _headerHeigthChanged;
}
bool WindowsManager::addWindow(Window *window)
{
    if (_currentWindowsPointer == _windowsSize)
        return false;
    _windows[_currentWindowsPointer++] = window;
    window->setParent(this);
    return true;
}
bool WindowsManager::removeWindowAt(uint16_t index, bool deletePointer = false)
{
    if (index < 0 || index >= _activeWindowIndex)
        return false;

    if (deletePointer)
        delete _windows[index];

    _windows[index] = NULL;

    _currentWindowsPointer--;
    for (uint16_t i = index; i < _currentWindowsPointer; i++)
    {
        _windows[i] = _windows[i + 1];
    }
    return true;
}
Window *WindowsManager::getWindowAt(uint16_t index)
{
    if (index < 0 || index >= _activeWindowIndex)
        return nullptr;
    return _windows[index];
}
bool WindowsManager::setWindowAt(uint16_t index, Window *window, bool deletePrev = false)
{
    if (index < 0 || index > _currentWindowsPointer || index == _activeWindowIndex)
        return false;
    if (deletePrev)
        delete _windows[index];

    _windows[index] = window;
    return true;
}
void WindowsManager::setRefreshRate(uint16_t refreshRate)
{
    _refreshRate = refreshRate;
    _timePerFrame = 1000 / _refreshRate;
}
void WindowsManager::refresh(TFT_HX8357 *tft, bool force = false)
{
    //init check
    if (_currentWindowsPointer == 0)
        return;
    uint64_t globalDiff = 0;
    uint64_t frameDiff = 0;
    bool globalRefreshFlag = false;
    paint(tft, force);
    uint64_t mill = millis(); //reduce millis() calls
    if (!force &&
        (frameDiff = mill - _lastRefreshTime) < _timePerFrame &&
        (globalDiff = mill - _lastGlobalRefreshTime) < _globalRefreshTime)
    {
        return;
    }

    if (force || globalDiff >= _globalRefreshTime)
    {
        _windows[_activeWindowIndex]->contentChanged = true;
        globalRefreshFlag = true;
    }
    _windows[_activeWindowIndex]->paint(tft);
    _lastRefreshTime = mill;
    if (globalRefreshFlag)
    {
        _lastGlobalRefreshTime = _lastRefreshTime;
    }
}
void WindowsManager::setActiveWindow(uint16_t index)
{
    if (index < 0 || index >= _currentWindowsPointer)
        return;
    _windows[index]->prepareForChange(_windows[_activeWindowIndex]);
    _activeWindowIndex = index;
    _activeWindowChanged = true;
    _lastGlobalRefreshTime = millis();
}
void WindowsManager::paint(TFT_HX8357 *tft, bool force = false)
{
    if (!force && !_windows[_activeWindowIndex]->checkFlags())
        return;

    uint16_t tmp_w = tft->width() / _currentWindowsPointer;
    uint16_t offset = tmp_w / 2;
    uint16_t heigthOffset = _headerHeigth / 2;

    tft->setTextSize(1);
    for (uint16_t i = 0; i < _currentWindowsPointer; i++)
    {
        if (i == _activeWindowIndex)
            continue;
        tft->fillRect(i * tmp_w, 0, tmp_w, _headerHeigth, _windows[i]->getHeaderBgColor());
        tft->setTextColor(_windows[i]->getHeaderColor(), _windows[i]->getHeaderBgColor());
        if (tft->textWidth(_windows[i]->getName(), 4) > tmp_w)
        {
            tft->setTextDatum(ML_DATUM);
            tft->drawString(_windows[i]->getName(), i * tmp_w, heigthOffset, 4);
        }
        else
        {
            tft->setTextDatum(MC_DATUM);
            tft->drawString(_windows[i]->getName(), i * tmp_w + offset, heigthOffset, 4);
        }
    }
    uint16_t offset_x = _activeWindowIndex * tmp_w;
    uint16_t width = tmp_w;
    uint16_t textWidth = tft->textWidth(_windows[_activeWindowIndex]->getName(), 4);
    if (textWidth > tmp_w)
    {
        offset_x += (tmp_w - textWidth) / 2 - 4;
        width = textWidth + 8;
    }
    tft->fillRect(offset_x, 0, width, _headerHeigth, _windows[_activeWindowIndex]->getBgColor());
    tft->setTextColor(_windows[_activeWindowIndex]->getHeaderBgColor(), _windows[_activeWindowIndex]->getBgColor());
    tft->setTextDatum(MC_DATUM);
    tft->drawString(_windows[_activeWindowIndex]->getName(), _activeWindowIndex * tmp_w + offset, heigthOffset, 4);
    _windows[_activeWindowIndex]->headerChanged = false;

    _activeWindowChanged = false;
    _windowsChanged = false;
    _headerHeigthChanged = false;
}
uint16_t WindowsManager::getWindowsCount()
{
    return _currentWindowsPointer;
}

void debug(TFT_HX8357 *tft, char *text)
{
    tft->setTextSize(1);
    tft->setTextColor(TFT_BLACK, TFT_WHITE);
    tft->drawCentreString(text, 239, 80, 4);
}
void header(TFT_HX8357 *tft, char *text, uint16_t y, uint16_t color, uint16_t bg)
{
    tft->setTextColor(color, bg);
    tft->setTextSize(1);
    tft->fillRect(0, 0, WIDTH, y, bg);
    tft->setTextDatum(BC_DATUM);
    tft->drawString(text, WIDTH / 2, y, 4);
}
void drawCross(TFT_HX8357 *tft, int x, int y, unsigned int color)
{
    tft->drawLine(x - 5, y, x + 5, y, color);
    tft->drawLine(x, y - 5, x, y + 5, color);
}
