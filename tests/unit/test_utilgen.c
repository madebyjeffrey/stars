#include "acutest.h"

#include "stars_test.h"

static POINT16 Pt(int16_t x, int16_t y) {
    POINT16 pt;

    pt.x = x;
    pt.y = y;
    return pt;
}

// CheckCross checks that a route crosses a radius-10 field centered on it
// for 20 ly, from 40 to 60 ly along a 100 ly route.
static void CheckCross(POINT16 ptFrom, POINT16 ptTo, POINT16 ptField) {
    int16_t dStart;
    int16_t dEnd;

    dStart = dEnd = -1;
    TEST_ASSERT(FIntersectCircleLine(ptFrom, ptTo, ptField, 100, 100, &dStart, &dEnd));
    TEST_CHECK_(dStart == 40, "start %d", dStart);
    TEST_CHECK_(dEnd == 60, "end %d", dEnd);
}

static void test_FIntersectCircleLine_horizontal(void) {
    CheckCross(Pt(0, 0), Pt(100, 0), Pt(50, 0));
    CheckCross(Pt(100, 0), Pt(0, 0), Pt(50, 0));
}

static void test_FIntersectCircleLine_diagonal(void) {
    CheckCross(Pt(0, 0), Pt(60, 80), Pt(30, 40));
    CheckCross(Pt(60, 80), Pt(0, 0), Pt(30, 40));
}

// North/South minefield immunity: a vertical route measured the field from
// the route's start point instead of its closest point.
static void test_FIntersectCircleLine_vertical(void) {
    CheckCross(Pt(0, 0), Pt(0, 100), Pt(0, 50));
    CheckCross(Pt(0, 100), Pt(0, 0), Pt(0, 50));
    CheckCross(Pt(5, 0), Pt(5, 100), Pt(5, 50));
}

static void test_FIntersectCircleLine_miss(void) {
    int16_t dStart;
    int16_t dEnd;

    TEST_CHECK(!FIntersectCircleLine(Pt(0, 0), Pt(100, 0), Pt(50, 20), 100, 100, &dStart, &dEnd));
    // A field behind the route's start.
    TEST_CHECK(!FIntersectCircleLine(Pt(0, 0), Pt(100, 0), Pt(-50, 0), 100, 100, &dStart, &dEnd));
}

TEST_LIST = {{"FIntersectCircleLine horizontal", test_FIntersectCircleLine_horizontal},
             {"FIntersectCircleLine diagonal", test_FIntersectCircleLine_diagonal},
             {"FIntersectCircleLine vertical", test_FIntersectCircleLine_vertical},
             {"FIntersectCircleLine miss", test_FIntersectCircleLine_miss},
             {NULL, NULL}};
