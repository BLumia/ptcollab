#include "Clipboard.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <algorithm>

#include "ComboOptions.h"
#include "editor/Settings.h"

struct CopyState {
  std::list<Item> m_items;
  qint32 m_copy_length;
  // TODO: move this to editstate so you can see others' param selections too.
  std::set<int> m_unit_nos;

  CopyState(){};
  CopyState(const std::set<int> &copy_unit_nos, const Interval &range,
            const pxtnService *pxtn, const std::set<EVENTKIND> &kinds_to_copy);
  std::list<Action::Primitive> makePaste(
      const std::set<int> &paste_unit_nos,
      const std::set<EVENTKIND> &kinds_to_paste, qint32 start_clock,
      const NoIdMap &map);
};

QDataStream &operator<<(QDataStream &out, const Item &a) {
  out << a.clock << a.kind << a.unit_no << a.value;
  return out;
}
QDataStream &operator>>(QDataStream &in, Item &a) {
  in >> a.clock >> a.kind >> a.unit_no >> a.value;
  return in;
}

template <typename T>
QDataStream &operator<<(QDataStream &out, const std::list<T> &a) {
  out << quint64(a.size());
  for (const auto &i : a) out << i;
  return out;
}
template <typename T>
QDataStream &operator<<(QDataStream &out, const std::set<T> &a) {
  out << quint64(a.size());
  for (const auto &i : a) out << i;
  return out;
}

template <typename T>
QDataStream &operator>>(QDataStream &in, std::list<T> &a) {
  quint64 size;
  in >> size;
  for (quint64 i = 0; i < size; ++i) {
    T v;
    in >> v;
    a.push_back(v);
  }
  return in;
}

template <typename T>
QDataStream &operator>>(QDataStream &in, std::set<T> &a) {
  quint64 size;
  in >> size;
  for (quint64 i = 0; i < size; ++i) {
    T v;
    in >> v;
    a.insert(v);
  }
  return in;
}

QDataStream &operator<<(QDataStream &out, const CopyState &a) {
  out << a.m_items << a.m_unit_nos << a.m_copy_length;
  return out;
}

QDataStream &operator>>(QDataStream &in, CopyState &a) {
  in >> a.m_items >> a.m_unit_nos >> a.m_copy_length;
  return in;
}

Clipboard::Clipboard(QObject *parent) : QObject(parent) {
  for (int kind : Settings::CopyKinds::get()) setKindIsCopied(EVENTKIND(kind), true);
  connect(this, &Clipboard::copyKindsSet, [this]() {
      QList<int> kinds;
      for (EVENTKIND kind : m_kinds_to_copy) kinds.push_back(int(kind));
      Settings::CopyKinds::set(kinds);
  });
}

static const QString CLIPBOARD_MIME = "application/ptcollab-clipboard";

CopyState::CopyState(const std::set<int> &unit_nos, const Interval &range,
                     const pxtnService *pxtn,
                     const std::set<EVENTKIND> &kinds_to_copy) {
  m_copy_length = range.length();

  auto min = std::min_element(unit_nos.begin(), unit_nos.end());
  uint8_t first_unit_no = (min == unit_nos.end() ? 0 : *min);
  for (const int &i : unit_nos) m_unit_nos.insert(i - first_unit_no);

  const EVERECORD *e = nullptr;
  for (e = pxtn->evels->get_Records(); e; e = e->next)
    if (e->clock >= range.start) break;

  for (; e && e->clock < range.end; e = e->next) {
    EVENTKIND kind(EVENTKIND(e->kind));
    if (unit_nos.find(e->unit_no) != unit_nos.end() &&
        kinds_to_copy.find(kind) != kinds_to_copy.end()) {
      int32_t v = e->value;
      if (Evelist_Kind_IsTail(e->kind)) v = std::min(v, range.end - e->clock);
      uint8_t unit_no = e->unit_no - first_unit_no;
      m_items.emplace_back(Item{e->clock - range.start, unit_no, kind, v});
    }
  }
}

// TODO: Maybe also be able to copy the tails of ONs, the existing state for
// state kinds, and unset them at the end of the interval.
void Clipboard::copy(const std::set<int> &unit_nos, const Interval &range,
                     const pxtnService *pxtn) {
  QByteArray data;
  QDataStream s(&data, QIODevice::ReadWrite);
  s << CopyState(unit_nos, range, pxtn, m_kinds_to_copy);
  QMimeData *mime = new QMimeData();
  mime->setData(CLIPBOARD_MIME, data);
  QGuiApplication::clipboard()->setMimeData(mime);
}

std::list<Action::Primitive> CopyState::makePaste(
    const std::set<int> &paste_unit_nos,
    const std::set<EVENTKIND> &kinds_to_paste, qint32 start_clock,
    const NoIdMap &map) {
  using namespace Action;
  std::list<Primitive> actions;
  auto min = std::min_element(paste_unit_nos.begin(), paste_unit_nos.end());
  if (min == paste_unit_nos.end()) return actions;
  uint8_t first_unit_no = *min;

  qint32 end_clock = start_clock + m_copy_length;
  for (const int &source_unit_no : m_unit_nos) {
    uint8_t unit_no = source_unit_no + first_unit_no;
    if (paste_unit_nos.find(unit_no) != paste_unit_nos.end()) {
      if (map.numUnits() <= unit_no) continue;
      qint32 unit_id = map.noToId(unit_no);
      for (const EVENTKIND &kind : kinds_to_paste)
        actions.emplace_back(
            Primitive{kind, unit_id, start_clock, Delete{end_clock}});
    }
  }

  for (const Item &item : m_items) {
    uint8_t unit_no = item.unit_no + first_unit_no;
    if (paste_unit_nos.find(unit_no) != paste_unit_nos.end() &&
        kinds_to_paste.find(item.kind) != kinds_to_paste.end()) {
      if (map.numUnits() <= unit_no) continue;
      qint32 unit_id = map.noToId(unit_no);
      actions.emplace_back(Primitive{
          item.kind, unit_id, start_clock + item.clock, Add{item.value}});
    }
  }
  return actions;
}

PasteResult Clipboard::makePaste(const std::set<int> &paste_unit_nos,
                                 qint32 start_clock, const NoIdMap &map) {
  using namespace Action;

  const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
  if (!mime->hasFormat(CLIPBOARD_MIME)) return {std::list<Primitive>{}, 0};
  QDataStream s(mime->data(CLIPBOARD_MIME));
  CopyState c;
  s >> c;

  return {c.makePaste(paste_unit_nos, m_kinds_to_copy, start_clock, map),
          c.m_copy_length};
}

// TODO: Maybe shouldn't be here? Doesn't actually use clipboard state ATM
std::list<Action::Primitive> Clipboard::makeClear(const std::set<int> &unit_nos,
                                                  const Interval &range,
                                                  const NoIdMap &map) {
  using namespace Action;
  std::list<Primitive> actions;
  // TODO: Dedup with makePaste
  for (const int &unit_no : unit_nos) {
    if (map.numUnits() <= size_t(unit_no)) continue;
    qint32 unit_id = map.noToId(unit_no);
    for (const EVENTKIND &kind : m_kinds_to_copy)
      actions.emplace_back(
          Primitive{kind, unit_id, range.start, Delete{range.end}});
  }
  return actions;
}

void Clipboard::setKindIsCopied(EVENTKIND kind, bool set) {
  if (kind == EVENTKIND_ON || kind == EVENTKIND_VELOCITY) {
    if (set) {
      m_kinds_to_copy.insert(EVENTKIND_ON);
      m_kinds_to_copy.insert(EVENTKIND_VELOCITY);
      m_kinds_to_copy.insert(EVENTKIND_KEY);
    } else {
      m_kinds_to_copy.erase(EVENTKIND_ON);
      m_kinds_to_copy.erase(EVENTKIND_VELOCITY);
      m_kinds_to_copy.erase(EVENTKIND_KEY);
    }
  } else {
    if (set)
      m_kinds_to_copy.insert(kind);
    else
      m_kinds_to_copy.erase(kind);
  }
  emit copyKindsSet();
}

bool Clipboard::kindIsCopied(EVENTKIND kind) {
  return m_kinds_to_copy.find(kind) != m_kinds_to_copy.end();
}
