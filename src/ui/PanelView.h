#pragma once
#include <d2d1.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "core/Workspace.h"
#include "ui/Layout.h"

class Painter;

enum class Filter { All, GitHub, Local, Untracked };

// Something clickable. `index` is a tab number, row number or root number depending on the action.
enum class Action { None, Bubble, AddFolder, RescanAll, FilterTab, Search, ToggleRow, OpenLink, RescanRoot, RemoveRoot };

struct Hit {
    Action action = Action::None;
    int index = -1;
    bool operator==(const Hit&) const = default;
};

// The contents of the open panel: header, filter tabs, search box and the folder tree.
// It remembers what it drew last so clicks can be matched to what is on screen.
class PanelView {
public:
    // `reveal` 0..1 is how far the opening circle has grown. Clicks only register when `interactive`.
    void draw(Painter& painter, const PanelLayout& layout, const Workspace& workspace, float reveal, float opacity,
              bool interactive, Hit hovered);
    Hit hitTest(D2D1_POINT_2F point) const;

    void invalidate() { rowsDirty_ = true; }  // call when folders or scans change
    void setFilter(Filter filter);
    void typeChar(wchar_t ch);
    void backspace();
    bool clearSearch();  // false if it was already empty
    void toggleRow(int index);
    void scrollBy(float delta);
    const FolderNode* rowNode(int index) const;

private:
    enum class RowKind { Root, Folder, Message };
    struct Row {
        RowKind kind;
        const FolderNode* node;  // null for messages, and for roots that are not scanned yet
        int root;
        int depth;
        bool open = false;
        std::wstring message;
    };
    using MatchCache = std::unordered_map<const FolderNode*, bool>;

    // Building the list of visible rows
    void rebuildRows(const Workspace& workspace);
    void addChildren(const FolderNode& parent, int depth, int root, bool filterHere, MatchCache& cache);
    bool matches(const FolderNode& node) const;
    bool hasMatchBelow(const FolderNode& node, MatchCache& cache) const;
    bool filtering() const { return filter_ != Filter::All || !query_.empty(); }
    static float rowHeight(const Row& row);

    // Drawing
    void drawShadow(Painter& painter, const D2D1_RECT_F& panel, float strength);
    void drawHeader(Painter& painter, const PanelLayout& layout, Hit hovered);
    void drawTabs(Painter& painter, const D2D1_RECT_F& area, Hit hovered);
    void drawSearch(Painter& painter, const D2D1_RECT_F& area, bool showCaret);
    void drawList(Painter& painter, const D2D1_RECT_F& area, const Workspace& workspace, Hit hovered);
    void drawEmptyState(Painter& painter, const D2D1_RECT_F& area, Hit hovered);
    void drawRootRow(Painter& painter, const Row& row, int index, const D2D1_RECT_F& rect, const RootFolder& root,
                     Hit hovered);
    void drawFolderRow(Painter& painter, const Row& row, int index, const D2D1_RECT_F& rect, Hit hovered);
    void drawMessageRow(Painter& painter, const Row& row, const D2D1_RECT_F& rect);
    void drawBadge(Painter& painter, D2D1_POINT_2F center, Status status);
    void drawScrollbar(Painter& painter, const D2D1_RECT_F& area);
    void iconButton(Painter& painter, const D2D1_RECT_F& rect, wchar_t glyph, Hit hit, Hit hovered, bool enabled = true);
    void addHit(const D2D1_RECT_F& rect, Hit hit);

    // View state
    Filter filter_ = Filter::All;
    std::wstring query_;
    std::wstring queryLower_;
    float scroll_ = 0;
    std::unordered_map<std::wstring, bool> openOverrides_;  // folders the user opened or closed, by path

    // Cached from the last rebuild / draw
    std::vector<Row> rows_;
    float contentHeight_ = 0;
    StatusCounts counts_;
    bool rowsDirty_ = true;
    struct Region {
        D2D1_RECT_F rect;
        Hit hit;
    };
    std::vector<Region> hits_;
    D2D1_RECT_F hitClip_{};
};
