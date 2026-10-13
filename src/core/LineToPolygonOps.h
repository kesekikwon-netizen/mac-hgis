#pragma once

#include <QMap>
#include <QPointer>
#include <QString>
#include <QVector>
#include <qgsfeatureid.h>
#include <qgsgeometry.h>

class QgsVectorLayer;

// 선 자동 연결 및 면(Polygon) 변환 도구
// 현장 CAD(DXF) 도면이나 유구_선의 벌어진 틈새(기본 5cm 이내)를 자석처럼 자동 연결하여
// 닫힌 유구_면(feature_poly)으로 변환합니다.
namespace LineToPolygonOps {

struct Options {
  double snapTolerance = 0.05;  // 자석 거리 (기본값: 0.05m = 5cm)
};

struct Result {
  bool success = false;
  QVector<QgsGeometry> polygons;
  QString errorMessage;
  int snappedCount = 0;
};

// 벌어진 선 끝점들을 snapTolerance 이내에서 자동 연결(Snap)합니다.
[[nodiscard]] QVector<QgsGeometry> snapLines(const QVector<QgsGeometry>& lines, double tolerance, int* snappedCount = nullptr);

// 입력받은 선들을 스냅 후 polygonize하여 유효한 단일 폴리곤 목록을 생성합니다.
// 18개 체크리스트(GEOMETRY_VALID, SURVEY_POLYGON_ONLY)를 준수하도록 makeValid()를 적용합니다.
[[nodiscard]] Result polygonizeLines(const QVector<QgsGeometry>& lines, const Options& options = Options());

// 생성된 면을 타겟 폴리곤 레이어에 추가하고, 옵션에 따라 원본 선을 삭제하며 되돌리기용 레이어 목록을 반환합니다.
bool createPolygonFeatures(QgsVectorLayer* polyLayer,
                           const QVector<QgsGeometry>& polygons,
                           const QMap<QgsVectorLayer*, QgsFeatureIds>& linesByLayer,
                           bool deleteSourceLines,
                           QList<QPointer<QgsVectorLayer>>& affectedOut);

}  // namespace LineToPolygonOps
