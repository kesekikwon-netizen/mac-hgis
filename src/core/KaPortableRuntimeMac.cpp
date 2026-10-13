#include "KaPortableRuntimeMac.h"

#if defined(Q_OS_MAC)
#include <QFile>
#include <QStringList>

namespace KaPortableRuntimeMac {

void discoverMac(KaPortablePaths& out, const QDir& app) {
  const QString bundlePlug = app.filePath(QStringLiteral("../PlugIns"));
  if (QDir(bundlePlug).exists()) {
    out.qtPlugins = QDir(bundlePlug).absolutePath();
  } else if (QDir(QStringLiteral("/opt/homebrew/share/qt/plugins")).exists()) {
    out.qtPlugins = QStringLiteral("/opt/homebrew/share/qt/plugins");
  }

  if (QDir(QStringLiteral("/Applications/QGIS.app")).exists()) {
    out.qgisPrefix = QStringLiteral("/Applications/QGIS.app");
    if (QDir(QStringLiteral("/Applications/QGIS.app/Contents/PlugIns/qgis")).exists()) {
      out.qgisPlugins = QStringLiteral("/Applications/QGIS.app/Contents/PlugIns/qgis");
    }
  }

  if (QFile::exists(QStringLiteral("/opt/homebrew/share/proj/proj.db"))) {
    out.projData = QStringLiteral("/opt/homebrew/share/proj");
  } else if (QFile::exists(QStringLiteral("/Applications/QGIS.app/Contents/Resources/qgis/proj/proj.db"))) {
    out.projData = QStringLiteral("/Applications/QGIS.app/Contents/Resources/qgis/proj");
  }

  if (QDir(QStringLiteral("/opt/homebrew/share/gdal")).exists()) {
    out.gdalData = QStringLiteral("/opt/homebrew/share/gdal");
  } else if (QDir(QStringLiteral("/Applications/QGIS.app/Contents/Resources/qgis/gdal")).exists()) {
    out.gdalData = QStringLiteral("/Applications/QGIS.app/Contents/Resources/qgis/gdal");
  }
}

bool applyMacEnvironment() {
  const QString old = QString::fromLocal8Bit(qgetenv("PATH"));
  QStringList macPaths = {QStringLiteral("/opt/homebrew/bin"),
                          QStringLiteral("/opt/homebrew/sbin"),
                          QStringLiteral("/Applications/QGIS.app/Contents/MacOS"),
                          QStringLiteral("/usr/local/bin"),
                          QStringLiteral("/usr/bin"),
                          QStringLiteral("/bin")};
  for (const QString& mp : old.split(QLatin1Char(':'))) {
    if (!macPaths.contains(mp) && !mp.isEmpty()) {
      macPaths.append(mp);
    }
  }
  return qputenv("PATH", macPaths.join(QLatin1Char(':')).toUtf8());
}

}  // namespace KaPortableRuntimeMac
#endif
