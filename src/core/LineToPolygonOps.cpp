#include "LineToPolygonOps.h"

#include <cmath>
#include <numeric>
#include <qgis.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsgeos.h>
#include <qgsvectorlayer.h>

namespace LineToPolygonOps {

namespace {

struct Dsu {
  explicit Dsu(int n) : parent(n) {
    std::iota(parent.begin(), parent.end(), 0);
  }
  int find(int i) {
    if (parent[i] == i) return i;
    return parent[i] = find(parent[i]);
  }
  void unite(int i, int j) {
    const int rootI = find(i);
    const int rootJ = find(j);
    if (rootI != rootJ) parent[rootI] = rootJ;
  }
  std::vector<int> parent;
};

QVector<QgsPolylineXY> extractLines(const QVector<QgsGeometry>& lines) {
  QVector<QgsPolylineXY> out;
  for (const auto& g : lines) {
    if (g.isEmpty()) continue;
    if (g.isMultipart()) {
      const auto parts = g.asMultiPolyline();
      for (const auto& p : parts) {
        if (p.size() >= 2) out.append(p);
      }
    } else {
      const auto p = g.asPolyline();
      if (p.size() >= 2) out.append(p);
    }
  }
  return out;
}

double distSq(const QgsPointXY& a, const QgsPointXY& b) {
  const double dx = a.x() - b.x();
  const double dy = a.y() - b.y();
  return dx * dx + dy * dy;
}

void appendPolygonsFromGeom(const QgsGeometry& geom, QVector<QgsGeometry>& out) {
  if (geom.isEmpty()) return;
  QgsGeometry valid = geom.isGeosValid() ? geom : geom.makeValid();
  if (valid.isEmpty() || !valid.isGeosValid()) return;

  if (valid.isMultipart()) {
    const auto collection = valid.asGeometryCollection();
    for (const auto& part : collection) {
      if (part.type() == Qgis::GeometryType::Polygon && !part.isEmpty()) {
        QgsGeometry pValid = part.isGeosValid() ? part : part.makeValid();
        if (!pValid.isEmpty() && pValid.isGeosValid()) {
          if (pValid.isMultipart()) {
            for (const auto& sub : pValid.asGeometryCollection()) {
              if (sub.type() == Qgis::GeometryType::Polygon && !sub.isEmpty()) out.append(sub);
            }
          } else {
            out.append(pValid);
          }
        }
      }
    }
  } else if (valid.type() == Qgis::GeometryType::Polygon) {
    out.append(valid);
  }
}

}  // namespace

QVector<QgsGeometry> snapLines(const QVector<QgsGeometry>& lines, double tolerance, int* snappedCount) {
  auto polylines = extractLines(lines);
  if (polylines.isEmpty()) return {};

  const double tolSq = tolerance * tolerance;
  int snapModifications = 0;

  // 1. Gather all endpoints
  struct Endpoint {
    int lineIdx;
    bool isStart;
  };
  std::vector<Endpoint> endpoints;
  endpoints.reserve(polylines.size() * 2);
  for (int i = 0; i < polylines.size(); ++i) {
    endpoints.push_back({i, true});
    endpoints.push_back({i, false});
  }

  const int numEndpoints = static_cast<int>(endpoints.size());
  Dsu dsu(numEndpoints);

  const auto getPoint = [&](int epIdx) -> QgsPointXY {
    const auto& ep = endpoints[epIdx];
    return ep.isStart ? polylines[ep.lineIdx].first() : polylines[ep.lineIdx].last();
  };

  // Group endpoints that are within tolerance
  for (int i = 0; i < numEndpoints; ++i) {
    const QgsPointXY pi = getPoint(i);
    for (int j = i + 1; j < numEndpoints; ++j) {
      const QgsPointXY pj = getPoint(j);
      if (distSq(pi, pj) <= tolSq) {
        dsu.unite(i, j);
      }
    }
  }

  // Calculate cluster centroids
  std::vector<int> counts(numEndpoints, 0);
  std::vector<double> sumX(numEndpoints, 0.0);
  std::vector<double> sumY(numEndpoints, 0.0);

  for (int i = 0; i < numEndpoints; ++i) {
    const int root = dsu.find(i);
    const QgsPointXY pt = getPoint(i);
    counts[root]++;
    sumX[root] += pt.x();
    sumY[root] += pt.y();
  }

  // Apply centroid to clustered endpoints
  for (int i = 0; i < numEndpoints; ++i) {
    const int root = dsu.find(i);
    if (counts[root] > 1) {
      const QgsPointXY newPt(sumX[root] / counts[root], sumY[root] / counts[root]);
      const auto& ep = endpoints[i];
      auto& line = polylines[ep.lineIdx];
      QgsPointXY& target = ep.isStart ? line.first() : line.last();
      if (distSq(target, newPt) > 1e-12) {
        target = newPt;
        snapModifications++;
      }
    }
  }

  // 2. Also snap endpoints to nearby intermediate vertices if any
  for (int i = 0; i < numEndpoints; ++i) {
    const auto& ep = endpoints[i];
    QgsPointXY epPt = ep.isStart ? polylines[ep.lineIdx].first() : polylines[ep.lineIdx].last();
    for (int l = 0; l < polylines.size(); ++l) {
      if (l == ep.lineIdx) continue;
      const auto& otherLine = polylines[l];
      for (int v = 1; v < otherLine.size() - 1; ++v) {
        if (distSq(epPt, otherLine[v]) <= tolSq) {
          if (ep.isStart) polylines[ep.lineIdx].first() = otherLine[v];
          else polylines[ep.lineIdx].last() = otherLine[v];
          snapModifications++;
          break;
        }
      }
    }
  }

  if (snappedCount) *snappedCount = snapModifications;

  QVector<QgsGeometry> result;
  result.reserve(polylines.size());
  for (const auto& line : polylines) {
    if (line.size() >= 2) {
      result.append(QgsGeometry::fromPolylineXY(line));
    }
  }
  return result;
}

Result polygonizeLines(const QVector<QgsGeometry>& lines, const Options& options) {
  Result res;
  if (lines.isEmpty()) {
    res.errorMessage = QStringLiteral("선택된 선이 없습니다. 면으로 변환할 선들을 먼저 선택해 주세요.");
    return res;
  }

  int snapped = 0;
  const QVector<QgsGeometry> snappedLines = snapLines(lines, options.snapTolerance, &snapped);
  res.snappedCount = snapped;

  if (snappedLines.isEmpty()) {
    res.errorMessage = QStringLiteral("유효한 선 도형을 추출하지 못했습니다.");
    return res;
  }

  // Node geometries using unaryUnion to ensure intersections are split
  QgsGeometry noded = QgsGeometry::unaryUnion(snappedLines);
  QVector<QgsGeometry> feedList;
  if (!noded.isEmpty()) {
    if (noded.isMultipart()) {
      for (const auto& part : noded.asGeometryCollection()) {
        if (!part.isEmpty()) feedList.append(part);
      }
    } else {
      feedList.append(noded);
    }
  } else {
    feedList = snappedLines;
  }

  const QgsGeometry polyCollection = QgsGeometry::polygonize(feedList);
  if (polyCollection.isEmpty()) {
    res.errorMessage = QStringLiteral(
        "선들이 닫힌 영역을 형성하지 않습니다.\n"
        "선 끝점 간의 틈새가 %1m보다 크거나, 닫히지 않은 열린 선인지 확인해 주세요.")
        .arg(options.snapTolerance, 0, 'f', 2);
    return res;
  }

  appendPolygonsFromGeom(polyCollection, res.polygons);

  if (res.polygons.isEmpty()) {
    res.errorMessage = QStringLiteral("닫힌 면을 생성했으나 유효한 폴리곤으로 변환하지 못했습니다.");
    return res;
  }

  res.success = true;
  return res;
}

bool createPolygonFeatures(QgsVectorLayer* polyLayer,
                           const QVector<QgsGeometry>& polygons,
                           const QMap<QgsVectorLayer*, QgsFeatureIds>& linesByLayer,
                           bool deleteSourceLines,
                           QList<QPointer<QgsVectorLayer>>& affectedOut) {
  if (!polyLayer || !polyLayer->isValid() || polygons.isEmpty()) return false;
  affectedOut.clear();
  affectedOut.append(polyLayer);

  polyLayer->beginEditCommand(QStringLiteral("선으로 면 만들기"));
  QgsFeatureList newFeatures;
  newFeatures.reserve(polygons.size());
  for (const auto& poly : polygons) {
    QgsFeature f(polyLayer->fields());
    f.setGeometry(poly);
    newFeatures.append(f);
  }
  const bool ok = polyLayer->addFeatures(newFeatures);
  polyLayer->endEditCommand();
  if (!ok) return false;

  if (deleteSourceLines) {
    for (auto it = linesByLayer.cbegin(); it != linesByLayer.cend(); ++it) {
      QgsVectorLayer* src = it.key();
      if (src && src->isValid() && !it.value().isEmpty()) {
        src->beginEditCommand(QStringLiteral("선으로 면 만들기 (원본 삭제)"));
        src->deleteFeatures(it.value());
        src->endEditCommand();
        affectedOut.append(src);
      }
    }
  }
  return true;
}

}  // namespace LineToPolygonOps
