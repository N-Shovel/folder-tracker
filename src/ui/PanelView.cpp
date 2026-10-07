#include "ui/PanelView.h"

#include <algorithm>
#include <cmath>

#include "core/Text.h"
#include "ui/Geometry.h"
#include "ui/Painter.h"
#include "ui/Theme.h"

using namespace theme;

namespace {

struct Tab {
    Filter filter;
    const wchar_t* label;
};
constexpr Tab kTabs[] = {
    {Filter::All, L"All"},
    {Filter::GitHub, L"GitHub"},
    {Filter::Local, L"Git only"},
    {Filter::Untracked, L"No git"},
};

Status statusOf(Filter filter) {
    switch (filter) {
        case Filter::GitHub: return Status::GitHub;
        case Filter::Local: return Status::Local;
        default: return Status::Untracked;
    }
}

int countFor(Filter filter, const StatusCounts& counts) {
    switch (filter) {
        case Filter::GitHub: return counts.github;
        case Filter::Local: return counts.local;
        case Filter::Untracked: return counts.untracked;
        default: return 0;
    }
}

constexpr float kButtonSize = 26;
constexpr float kScrollbarGutter = 8;

}  // namespace

// ---------------------------------------------------------------------------------------------
// View state

void PanelView::setFilter(Filter filter) {
    filter_ = filter;
    openOverrides_.clear();
    scroll_ = 0;
    rowsDirty_ = true;
}

void PanelView::typeChar(wchar_t ch) {
    query_ += ch;
    openOverrides_.clear();
    scroll_ = 0;
    rowsDirty_ = true;
}

void PanelView::backspace() {
    if (query_.empty()) return;
    query_.pop_back();
    openOverrides_.clear();
    rowsDirty_ = true;
}

bool PanelView::clearSearch() {
    if (query_.empty()) return false;
    query_.clear();
    openOverrides_.clear();
    rowsDirty_ = true;
    return true;
}

void PanelView::toggleRow(int index) {
    if (index < 0 || index >= int(rows_.size())) return;
    const Row& row = rows_[index];
    if (row.kind != RowKind::Folder || row.node->children.empty()) return;
    openOverrides_[row.node->path] = !row.open;
    rowsDirty_ = true;
}

void PanelView::scrollBy(float delta) { scroll_ += delta; }  // clamped when drawing

const FolderNode* PanelView::rowNode(int index) const {
    return index >= 0 && index < int(rows_.size()) ? rows_[index].node : nullptr;
}

// ---------------------------------------------------------------------------------------------
// Rows

bool PanelView::matches(const FolderNode& node) const {
    const bool statusOk = filter_ == Filter::All || node.status == statusOf(filter_);
    const bool queryOk = queryLower_.empty() || toLower(node.name).find(queryLower_) != std::wstring::npos;
    return statusOk && queryOk;
}

bool PanelView::hasMatchBelow(const FolderNode& node, MatchCache& cache) const {
    if (const auto found = cache.find(&node); found != cache.end()) return found->second;
    const bool any = std::ranges::any_of(node.children, [&](const auto& child) {
        return matches(*child) || hasMatchBelow(*child, cache);
    });
    cache[&node] = any;
    return any;
}

void PanelView::rebuildRows(const Workspace& workspace) {
    rows_.clear();
    counts_ = workspace.counts();
    queryLower_ = toLower(query_);
    MatchCache cache;

    const auto& roots = workspace.roots();
    for (int r = 0; r < int(roots.size()); ++r) {
        const RootFolder& root = roots[r];
        rows_.push_back({RowKind::Root, root.tree.get(), r, 0});

        auto message = [&](std::wstring text) { rows_.push_back({RowKind::Message, nullptr, r, 1, false, std::move(text)}); };
        if (!root.error.empty()) {
            message(root.error);
        } else if (!root.tree) {
            message(L"Scanning…");
        } else {
            const size_t before = rows_.size();
            addChildren(*root.tree, 1, r, filtering(), cache);
            if (rows_.size() == before) message(filtering() ? L"No folders match." : L"No subfolders.");
        }
    }

    contentHeight_ = 0;
    for (const Row& row : rows_) contentHeight_ += rowHeight(row);
    rowsDirty_ = false;
}

// While filtering, a matching folder is shown with everything inside it (collapsed),
// and the folders leading to a match are opened automatically.
void PanelView::addChildren(const FolderNode& parent, int depth, int root, bool filterHere, MatchCache& cache) {
    for (const auto& child : parent.children) {
        const bool matched = !filterHere || matches(*child);
        if (!matched && !hasMatchBelow(*child, cache)) continue;

        const auto override = openOverrides_.find(child->path);
        const bool openByDefault = filterHere && !matched;
        const bool open = !child->children.empty() &&
                          (override != openOverrides_.end() ? override->second : openByDefault);

        rows_.push_back({RowKind::Folder, child.get(), root, depth, open});
        if (open) addChildren(*child, depth + 1, root, filterHere && !matched, cache);
    }
}

float PanelView::rowHeight(const Row& row) { return row.kind == RowKind::Root ? kRootRowHeight : kRowHeight; }

// ---------------------------------------------------------------------------------------------
// Hit testing

void PanelView::addHit(const RectF& rect, Hit hit) {
    const RectF visible = intersect(rect, hitClip_);
    if (!isEmpty(visible)) hits_.push_back({visible, hit});
}

Hit PanelView::hitTest(PointF point) const {
    // Later regions sit on top (a button inside a row wins over the row).
    for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
        if (contains(it->rect, point)) return it->hit;
    }
    return {};
}

// ---------------------------------------------------------------------------------------------
// Drawing

void PanelView::draw(Painter& painter, const PanelLayout& layout, const Workspace& workspace, float reveal,
                     float opacity, bool interactive, Hit hovered) {
    if (rowsDirty_) rebuildRows(workspace);
    hits_.clear();
    hitClip_ = layout.panel;

    const Palette& c = palette();
    drawShadow(painter, layout.panel, opacity * reveal * reveal * reveal);

    // Everything is clipped to a circle that grows out of the bubble.
    const float maxRadius = std::hypot(width(layout.panel), height(layout.panel));
    const float radius = kBubbleSize / 2 + (maxRadius - kBubbleSize / 2) * reveal;
    painter.pushCircleLayer(layout.bubbleCenter, radius, opacity);

    painter.fillRoundRect(layout.panel, kPanelRadius, c.surface);
    painter.strokeRoundRect(inset(layout.panel, 0.5f, 0.5f), kPanelRadius, c.border);
    drawHeader(painter, layout, hovered);
    if (!workspace.roots().empty()) {
        drawTabs(painter, layout.tabs, hovered);
        drawSearch(painter, layout.search, interactive);
    }
    drawList(painter, layout.list, workspace, hovered);

    painter.popLayer();
    if (!interactive) hits_.clear();
}

void PanelView::drawShadow(Painter& painter, const RectF& panel, float strength) {
    if (strength <= 0.01f) return;
    // A few faint, growing rounded rectangles look like a soft blur.
    for (int i = 8; i >= 1; --i) {
        const RectF ring = inset(panel, -float(i), -float(i));
        painter.fillRoundRect({ring.left, ring.top + 4, ring.right, ring.bottom + 4}, kPanelRadius + i,
                              withAlpha(palette().shadow, strength * 0.07f));
    }
}

void PanelView::drawHeader(Painter& painter, const PanelLayout& layout, Hit hovered) {
    const RectF& h = layout.header;
    const float bubbleRoom = kBubbleSize + 8;
    const float left = h.left + (layout.corner.right ? 16 : bubbleRoom);
    const float right = h.right - (layout.corner.right ? bubbleRoom : 10);
    const float cy = centerY(h);

    const RectF settings{right - kButtonSize, cy - kButtonSize / 2, right, cy + kButtonSize / 2};
    const RectF add{settings.left - kButtonSize - 2, settings.top, settings.left - 2, settings.bottom};
    const RectF rescan{add.left - kButtonSize - 2, add.top, add.left - 2, add.bottom};
    iconButton(painter, settings, Icon::Settings, {Action::Settings}, hovered);
    iconButton(painter, add, Icon::AddFolder, {Action::AddFolder}, hovered);
    iconButton(painter, rescan, Icon::Refresh, {Action::RescanAll}, hovered);
    painter.text(L"Folder Tracker", {left, h.top, rescan.left - 4, h.bottom}, Font::Title, palette().text);
}

void PanelView::drawTabs(Painter& painter, const RectF& area, Hit hovered) {
    const Palette& c = palette();
    painter.fillRoundRect(area, kRadiusMedium, c.surfaceRaised);
    painter.strokeRoundRect(inset(area, 0.5f, 0.5f), kRadiusMedium, c.border);

    const float tabWidth = (width(area) - 4) / std::size(kTabs);
    for (int i = 0; i < int(std::size(kTabs)); ++i) {
        const Tab& tab = kTabs[i];
        const RectF rect{area.left + 2 + i * tabWidth, area.top + 2, area.left + 2 + (i + 1) * tabWidth,
                               area.bottom - 2};
        const bool selected = filter_ == tab.filter;
        const bool hover = hovered == Hit{Action::FilterTab, i};
        if (selected) painter.fillRoundRect(rect, kRadiusSmall, c.surfaceHover);
        const Color textColor = selected || hover ? c.text : c.textMuted;

        if (tab.filter == Filter::All) {
            painter.text(tab.label, rect, Font::Small, textColor, Align::Center);
        } else {
            // "● GitHub 16", centered in the tab
            const std::wstring label = std::wstring(tab.label) + L" " + std::to_wstring(countFor(tab.filter, counts_));
            const float dotSpace = 11;
            const float contentWidth = dotSpace + painter.textWidth(label, Font::Small);
            const float start = std::max(rect.left + 4, (rect.left + rect.right - contentWidth) / 2);
            painter.fillCircle({start + 3, centerY(rect)}, 3, statusColor(statusOf(tab.filter)));
            painter.text(label, {start + dotSpace, rect.top, rect.right - 2, rect.bottom}, Font::Small, textColor);
        }
        addHit(rect, {Action::FilterTab, i});
    }
}

void PanelView::drawSearch(Painter& painter, const RectF& area, bool showCaret) {
    const Palette& c = palette();
    painter.fillRoundRect(area, kRadiusMedium, c.surfaceRaised);
    painter.strokeRoundRect(inset(area, 0.5f, 0.5f), kRadiusMedium, query_.empty() ? c.border : c.accent);
    painter.icon(Icon::Search, {area.left + 15, centerY(area)}, Font::IconSmall, c.textMuted);

    const RectF textArea{area.left + 30, area.top, area.right - 8, area.bottom};
    if (query_.empty()) {
        painter.text(L"Type to search folders", textArea, Font::Body, c.textMuted);
    } else {
        painter.text(query_, textArea, Font::Body, c.text);
    }
    if (showCaret) {
        const float x = std::min(textArea.left + painter.textWidth(query_, Font::Body) + 1, textArea.right);
        painter.line({x, centerY(area) - 7}, {x, centerY(area) + 7}, c.text, 1);
    }
    addHit(area, {Action::Search});
}

void PanelView::drawList(Painter& painter, const RectF& area, const Workspace& workspace, Hit hovered) {
    if (workspace.roots().empty()) {
        drawEmptyState(painter, area, hovered);
        return;
    }

    scroll_ = std::clamp(scroll_, 0.0f, std::max(0.0f, contentHeight_ - height(area)));
    painter.pushClip(area);
    hitClip_ = area;

    float y = area.top - scroll_;
    for (int i = 0; i < int(rows_.size()); ++i) {
        const Row& row = rows_[i];
        const RectF rect{area.left, y, area.right - kScrollbarGutter, y + rowHeight(row)};
        y += rowHeight(row);
        if (rect.bottom < area.top || rect.top > area.bottom) continue;

        switch (row.kind) {
            case RowKind::Root: drawRootRow(painter, row, i, rect, workspace.roots()[row.root], hovered); break;
            case RowKind::Folder: drawFolderRow(painter, row, i, rect, hovered); break;
            case RowKind::Message: drawMessageRow(painter, row, rect); break;
        }
    }

    painter.popClip();
    drawScrollbar(painter, area);
}

void PanelView::drawEmptyState(Painter& painter, const RectF& area, Hit hovered) {
    const Palette& c = palette();
    const PointF middle = center(area);
    painter.text(L"No folders yet.", {area.left, middle.y - 40, area.right, middle.y - 16}, Font::Body, c.textMuted,
                 Align::Center);

    const RectF button{middle.x - 70, middle.y - 6, middle.x + 70, middle.y + 26};
    const bool hover = hovered == Hit{Action::AddFolder, 1};
    painter.fillRoundRect(button, kRadiusSmall, hover ? withAlpha(c.accent, 0.85f) : c.accent);
    painter.text(L"Add a folder", button, Font::BodyBold, c.onAccent, Align::Center);
    addHit(button, {Action::AddFolder, 1});
}

void PanelView::drawRootRow(Painter& painter, const Row& row, int index, const RectF& rect,
                            const RootFolder& root, Hit hovered) {
    const Palette& c = palette();
    const RectF box{rect.left, rect.top + 4, rect.right, rect.bottom - 2};
    const float cy = centerY(box);
    painter.fillRoundRect(box, kRadiusSmall, c.surfaceRaised);

    const FolderNode* node = row.node;
    const Status status = node ? node->status : Status::Container;
    const bool isRepo = status == Status::GitHub || status == Status::Local;
    painter.icon(Icon::Folder, {box.left + 15, cy}, Font::Icon, isRepo ? statusColor(status) : c.accent);

    // Buttons and badge, placed from the right edge inward.
    float x = box.right - 3;
    auto nextSlot = [&] {
        const RectF slot{x - kButtonSize, cy - kButtonSize / 2, x, cy + kButtonSize / 2};
        x -= kButtonSize + 1;
        return slot;
    };
    iconButton(painter, nextSlot(), Icon::Delete, {Action::RemoveRoot, row.root}, hovered);
    iconButton(painter, nextSlot(), Icon::Refresh, {Action::RescanRoot, row.root}, hovered, !root.scanning);
    if (node && !node->webUrl.empty()) iconButton(painter, nextSlot(), Icon::OpenLink, {Action::OpenLink, index}, hovered);
    if (hasBadge(status)) drawBadge(painter, center(nextSlot()), status);

    const std::wstring name = node ? node->name : folderName(root.path);
    painter.text(name, {box.left + 30, box.top, x, box.bottom}, Font::BodyBold, c.text);
}

void PanelView::drawFolderRow(Painter& painter, const Row& row, int index, const RectF& rect, Hit hovered) {
    const Palette& c = palette();
    const FolderNode& node = *row.node;
    const float cy = centerY(rect);

    addHit(rect, {Action::ToggleRow, index});
    if (hovered == Hit{Action::ToggleRow, index}) painter.fillRoundRect(rect, kRadiusSmall, c.surfaceHover);

    // Guide lines for each parent level, then chevron, folder icon and name.
    const float x = rect.left + 4 + (row.depth - 1) * kIndent;
    for (int level = 1; level < row.depth; ++level) {
        const float guideX = rect.left + 4 + (level - 1) * kIndent + 7;
        painter.line({guideX, rect.top}, {guideX, rect.bottom}, c.border);
    }
    if (!node.children.empty()) {
        painter.icon(row.open ? Icon::ChevronDown : Icon::ChevronRight, {x + 7, cy}, Font::IconTiny, c.textMuted);
    }
    const bool colored = hasBadge(node.status);
    painter.icon(row.open ? Icon::FolderOpen : Icon::Folder, {x + 24, cy}, Font::Icon,
                 colored ? statusColor(node.status) : c.textMuted);

    float right = rect.right - 2;
    if (!node.webUrl.empty()) {
        iconButton(painter, {right - kButtonSize, cy - kButtonSize / 2, right, cy + kButtonSize / 2}, Icon::OpenLink,
                   {Action::OpenLink, index}, hovered);
        right -= kButtonSize + 1;
    }
    if (hasBadge(node.status)) {
        drawBadge(painter, {right - kButtonSize / 2, cy}, node.status);
        right -= kButtonSize + 1;
    }
    painter.text(node.name, {x + 36, rect.top, right, rect.bottom}, Font::Body,
                 node.status == Status::Inside ? c.textMuted : c.text);
}

void PanelView::drawMessageRow(Painter& painter, const Row& row, const RectF& rect) {
    painter.text(row.message, {rect.left + 4 + row.depth * kIndent, rect.top, rect.right, rect.bottom}, Font::Small,
                 palette().textMuted);
}

void PanelView::drawBadge(Painter& painter, PointF center, Status status) {
    const Color color = statusColor(status);
    painter.fillCircle(center, kBadgeRadius, withAlpha(color, 0.14f));
    painter.strokeCircle(center, kBadgeRadius - 0.5f, withAlpha(color, 0.45f));
    painter.icon(icons::forStatus(status), center, Font::IconTiny, color);
}

void PanelView::drawScrollbar(Painter& painter, const RectF& area) {
    const float visible = height(area);
    if (contentHeight_ <= visible) return;
    const float thumbHeight = std::max(24.0f, visible * visible / contentHeight_);
    const float thumbTop = area.top + (visible - thumbHeight) * scroll_ / (contentHeight_ - visible);
    painter.fillRoundRect({area.right - 4, thumbTop, area.right - 1, thumbTop + thumbHeight}, 1.5f, palette().border);
}

void PanelView::iconButton(Painter& painter, const RectF& rect, Icon icon, Hit hit, Hit hovered, bool enabled) {
    const Palette& c = palette();
    const bool hover = enabled && hovered == hit;
    if (hover) painter.fillRoundRect(rect, kRadiusSmall, c.surfaceHover);
    const Color color = !enabled ? withAlpha(c.textMuted, 0.4f)
                               : hover && hit.action == Action::RemoveRoot ? c.danger
                               : hover ? c.text
                                       : c.textMuted;
    painter.icon(icon, center(rect), Font::IconSmall, color);
    if (enabled) addHit(rect, hit);
}
