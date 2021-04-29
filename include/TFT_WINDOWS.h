#include <TFT_HX8357.h>

#if !defined(WIDTH)
#define WIDTH 480
#endif // WIDTH
#if !defined(HEIGTH)
#define HEIGTH 320
#endif // HEIGTH

class Window;
class WindowContent;
class ArcGraph;
class BarGraph;
class TextBox;
class WindowsManager;

void header(TFT_HX8357 *tft, char *text, uint16_t y, uint16_t color, uint16_t bg);
void drawCross(TFT_HX8357 *tft, int x, int y, unsigned int color);
void debug(TFT_HX8357 *tft, char *text);

class WindowContent
{
protected:
    Window *_parent;

    uint16_t _x, _y, _w, _h;
    uint16_t _fgColor;
    uint16_t _fntColor;
    uint16_t _bgColor;

    bool positionOrSizeChanged;
    bool bgChanged;
    bool fgColorChanged;
    bool fntColorChanged;

public:
    void setParent(Window *parent);
    Window *getParent();
    virtual void paint(TFT_HX8357 *tft, bool force = false);
    ;
    virtual void setPositionAndSize(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    virtual void setBgColor(uint16_t bgColor);
    virtual uint16_t getBgColor();
    virtual void setFgColor(uint16_t fgColor);
    virtual void setFntColor(uint16_t fntColor);
    virtual uint16_t getFgColor();
    virtual uint16_t getFntColor();
    virtual bool checkFlags();
    virtual void resetFlags();

    WindowContent(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor);
};

class ArcGraph : public WindowContent
{
private:
    uint16_t *_source;
    uint16_t _lastSourceValue;
    int16_t _maxValue;
    uint16_t _currentSegment;
    char *_unit;

public:
    ArcGraph(uint16_t *source, int16_t maxValue, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor);
    void setMaxValue(int16_t maxValue);
    void paint(TFT_HX8357 *tft, bool force = false);
    ;
    void setUnit(char *c);
};

class BarGraph : public WindowContent
{
private:
    uint16_t *_source;
    uint16_t _lastSourceValue;
    int16_t _maxValue;
    uint16_t _currentX;

public:
    BarGraph(uint16_t *source, int16_t maxValue, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor);
    void setMaxValue(int16_t maxValue);
    void paint(TFT_HX8357 *tft, bool force = false);
    ;
};

class TextBox : public WindowContent
{
private:
    char *_text;
    uint16_t _textSize;

public:
    TextBox(char *text, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fntColor, uint16_t fgColor, uint16_t bgColor);
    char *getText();
    void setText(char *text);

    void paint(TFT_HX8357 *tft, bool force = false);
    ;
};

class Window
{
private:
    WindowsManager *_parent;
    char *_name;
    uint16_t _headerColor;
    uint16_t _headerBgColor;
    uint16_t _bgColor;
    uint16_t _y0;
    uint16_t _contentSize;
    uint16_t _currentContentPointer;
    WindowContent **_contentArray;

public:
    bool headerChanged;
    bool contentChanged;
    bool bgChanged;
    bool checkFlags();
    bool addContent(WindowContent *content);
    bool removeContentAt(uint16_t index, bool deletePointer = false);
    void paint(TFT_HX8357 *tft, bool force = false);
    void setHeaderColor(uint16_t fontColor, uint16_t bgColor);
    void setBgColor(uint16_t bgColor);
    uint16_t getBgColor();
    uint16_t getHeaderColor();
    uint16_t getHeaderBgColor();
    void setName(char *name);
    void setParent(WindowsManager *parent);
    WindowsManager *getParent();
    char *getName();
    void prepareForChange(Window *prevWindow);
    Window(char *name, uint16_t headerColor, uint16_t headerBgColor, uint16_t bgColor);
    ~Window();
};

class WindowsManager
{
private:
    Window **_windows;
    uint16_t _windowsSize;
    uint16_t _currentWindowsPointer;

    uint16_t _activeWindowIndex;

    uint16_t _refreshRate;
    uint64_t _timePerFrame;
    uint64_t _lastRefreshTime;

    uint64_t _globalRefreshTime;
    uint64_t _lastGlobalRefreshTime;

    uint16_t _headerHeigth;

    bool _windowsChanged;
    bool _activeWindowChanged;
    bool _headerHeigthChanged;

public:
    bool checkFlags();
    bool addWindow(Window *window);
    bool removeWindowAt(uint16_t index, bool deletePointer = false);
    Window *getWindowAt(uint16_t index);
    bool setWindowAt(uint16_t index, Window *window, bool deletePrev = false);
    void setActiveWindow(uint16_t index);
    void refresh(TFT_HX8357 *tft, bool force = false);
    void setRefreshRate(uint16_t refreshRate);
    void paint(TFT_HX8357 *tft, bool force = false);
    uint16_t getWindowsCount();

    WindowsManager(uint16_t maxWindows, uint16_t refreshRate, uint16_t headerHeigth);
    ~WindowsManager();
};
