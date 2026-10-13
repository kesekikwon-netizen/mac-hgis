#include <QtTest>
#include "core/LineToPolygonOps.h"
#include <qgsgeometry.h>

class TestLineToPolygon : public QObject {
  Q_OBJECT

private slots:
  void closedLinesPolygonize();
  void linesWithGapsUnder5cmSnapAndPolygonize();
  void linesWithGapsOver5cmFail();
  void emptyInputFailsGracefully();
  void snapLinesCountReported();
};

void TestLineToPolygon::closedLinesPolygonize() {
  QVector<QgsGeometry> lines;
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 0), QgsPointXY(10, 0)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10, 0), QgsPointXY(10, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10, 10), QgsPointXY(0, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 10), QgsPointXY(0, 0)}));

  LineToPolygonOps::Options opts;
  opts.snapTolerance = 0.05;
  const auto res = LineToPolygonOps::polygonizeLines(lines, opts);

  QVERIFY(res.success);
  QCOMPARE(res.polygons.size(), 1);
  QVERIFY(res.polygons.first().isGeosValid());
  QVERIFY(qFuzzyCompare(res.polygons.first().area(), 100.0));
}

void TestLineToPolygon::linesWithGapsUnder5cmSnapAndPolygonize() {
  // Line 1: (0, 0) -> (10, 0)
  // Line 2: (10.03, 0.02) -> (10, 10)     -- gap ~0.036m <= 0.05m
  // Line 3: (10, 10.04) -> (0, 10)         -- gap 0.04m <= 0.05m
  // Line 4: (0, 9.98) -> (-0.02, 0.03)     -- gap 0.02m and ~0.036m <= 0.05m
  QVector<QgsGeometry> lines;
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 0), QgsPointXY(10, 0)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10.03, 0.02), QgsPointXY(10, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10, 10.04), QgsPointXY(0, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 9.98), QgsPointXY(-0.02, 0.03)}));

  LineToPolygonOps::Options opts;
  opts.snapTolerance = 0.05;
  const auto res = LineToPolygonOps::polygonizeLines(lines, opts);

  QVERIFY2(res.success, qPrintable(res.errorMessage));
  QCOMPARE(res.polygons.size(), 1);
  QVERIFY(res.polygons.first().isGeosValid());
  QVERIFY(res.polygons.first().area() > 95.0 && res.polygons.first().area() < 105.0);
  QVERIFY(res.snappedCount > 0);
}

void TestLineToPolygon::linesWithGapsOver5cmFail() {
  // Gap of 0.2m (20cm) exceeds 5cm tolerance
  QVector<QgsGeometry> lines;
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 0), QgsPointXY(10, 0)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10.2, 0), QgsPointXY(10, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(10, 10), QgsPointXY(0, 10)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 10), QgsPointXY(0, 0)}));

  LineToPolygonOps::Options opts;
  opts.snapTolerance = 0.05;
  const auto res = LineToPolygonOps::polygonizeLines(lines, opts);

  QVERIFY(!res.success);
  QVERIFY(!res.errorMessage.isEmpty());
}

void TestLineToPolygon::emptyInputFailsGracefully() {
  const auto res = LineToPolygonOps::polygonizeLines({});
  QVERIFY(!res.success);
  QVERIFY(!res.errorMessage.isEmpty());
}

void TestLineToPolygon::snapLinesCountReported() {
  QVector<QgsGeometry> lines;
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(0, 0), QgsPointXY(5, 0)}));
  lines.append(QgsGeometry::fromPolylineXY({QgsPointXY(5.02, 0), QgsPointXY(10, 0)}));

  int count = 0;
  const auto snapped = LineToPolygonOps::snapLines(lines, 0.05, &count);
  QCOMPARE(snapped.size(), 2);
  QVERIFY(count > 0);
}

QTEST_MAIN(TestLineToPolygon)
#include "test_line_to_polygon.moc"
