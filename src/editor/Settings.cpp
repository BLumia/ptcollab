#include "Settings.h"

#include "ComboOptions.h"

const QString WOICE_DIR_KEY("woice_dir");
const QString BUFFER_LENGTH_KEY("buffer_length");
const double DEFAULT_BUFFER_LENGTH = 0.15;
const QString VOLUME_KEY("volume");

const QString PTCOP_FILE_KEY("ptcop_dir");
const QString PTREC_SAVE_FILE_KEY("ptrec_save_dir");
const QString DISPLAY_NAME_KEY("display_name");

const QString HOST_SERVER_PORT_KEY("host_server_port");
const int DEFAULT_PORT = 15835;

const QString SAVE_RECORDING_ENABLED_KEY("save_recording_enabled");
const QString HOSTING_ENABLED_KEY("hosting_enabled");

const QString CONNECT_SERVER_KEY("connect_server");

namespace Settings {
namespace Version {
static QString v("0.4.0");
const QString &string() { return v; }
}  // namespace Version

namespace TextSize {
const QString TEXT_SIZE_KEY("text_size");
int default_size = 6;
int max_size = 18;
int min_size = 4;

int get() {
  bool ok;
  int value = QSettings().value(TEXT_SIZE_KEY, default_size).toInt(&ok);
  if (!ok) return default_size;
  return std::clamp(value, min_size, max_size);
}

void increase() {
  QSettings().setValue(TEXT_SIZE_KEY, std::min(get() + 1, max_size));
}

void decrease() {
  QSettings().setValue(TEXT_SIZE_KEY, std::max(get() - 1, min_size));
}
}  // namespace TextSize

namespace CustomStyle {
const char *KEY = "use_custom_style";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace CustomStyle

namespace ChordPreview {
const char *KEY = "ChordPreview";
bool get() { return QSettings().value(KEY, false).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace ChordPreview

namespace SpacebarStop {
const char *KEY = "SpacebarStop";
bool get() { return QSettings().value(KEY, false).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace SpacebarStop

namespace VelocityDrag {
const char *KEY = "VelocityDrag";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace VelocityDrag

namespace SwapScrollOrientation {
const char *KEY = "SwapScrollOrientation";
bool get() { return QSettings().value(KEY, false).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace SwapScrollOrientation

namespace SwapZoomOrientation {
const char *KEY = "SwapZoomOrientation";
bool get() { return QSettings().value(KEY, false).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace SwapZoomOrientation

namespace AutoAdvance {
const char *KEY = "AutoAdvance";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace AutoAdvance

namespace AutoAddUnit {
const char *KEY = "AutoAddUnit";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace AutoAddUnit

namespace ChangeDialogDirectory {
const char *KEY = "ChangeDialogDirectory";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace ChangeDialogDirectory

namespace PolyphonicMidiNotePreview {
const char *KEY = "PolyphonicMidiNotePreview";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace PolyphonicMidiNotePreview

namespace UnitPreviewClick {
const char *KEY = "UnitPreviewClick";
bool get() { return QSettings().value(KEY, true).toBool(); }
void set(bool value) { QSettings().setValue(KEY, value); }
}  // namespace UnitPreviewClick

namespace RenderFileDestination {
const char *KEY = "render_file_destination";
QString get() { return QSettings().value(KEY, "").toString(); }
void set(QString value) { QSettings().setValue(KEY, value); }
}  // namespace RenderFileDestination

QList<int> intListOfVariant(const QVariant &v, const QList<int> &def) {
  QList<QVariant> vl = v.toList();
  QList<int> l;
  bool ok = true;
  for (const auto &a : vl) {
    l.push_back(a.toInt(&ok));
    if (!ok) return def;
  }
  return l;
}
QVariant intListToVariant(const QList<int> &l) {
  QList<QVariant> vl;
  for (const auto &a : l) vl.push_back(a);
  return vl;
}

QList<int> getIntList(QString key, const QList<int> &def) {
  return intListOfVariant(QSettings().value(key, intListToVariant(def)), def);
}
namespace SideMenuWidth {
const char *KEY = "side_menu_width";
QList<int> get() { return getIntList(KEY, QList{10, 10000}); }
void set(const QList<int> &value) {
  QSettings().setValue(KEY, intListToVariant(value));
}
}  // namespace SideMenuWidth

namespace BottomBarHeight {
const char *KEY = "bottom_bar_height";
QList<int> get() { return getIntList(KEY, QList{10000, 10}); }
void set(const QList<int> &value) {
  QSettings().setValue(KEY, intListToVariant(value));
}
}  // namespace BottomBarHeight

namespace CopyKinds {
const char *KEY = "copy_kinds";
QList<int> default_value() {
  QList<int> v;
  for (auto [name, kind] : paramOptions)
    if (kind != EVENTKIND_VOICENO) v.push_back(kind);
  return v;
}
QList<int> get() {
  static QList<int> v(default_value());
  return getIntList(KEY, v);
}
void set(const QList<int> &value) {
  return QSettings().setValue(KEY, intListToVariant(value));
}
}  // namespace CopyKinds
}  // namespace Settings
