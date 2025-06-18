/*-------------------------------------------------------------
-작성 날짜 : 2025 - 05 - 05
- 작성자 : 최현우
- 학번 : 20214056
-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>

#define MAX_POINTS           100
#define MOVE_SPEED           3.0f
#define WINDOW_HEIGHT_SIZE    600
#define WINDOW_WIDTH_SIZE    800

typedef struct {
    float x, y, z;
} Point;

Point points[MAX_POINTS];
Point axisStart, axisEnd;

float currentColor[3] = { 1.0f, 1.0f, 0.0f };
int pointCount = 0;
int isPolygonDrawn = 0;
int mouseX, mouseY;
int isDragging = 0;
int dragStartX, dragStartY;
int dragEndX, dragEndY;
int dragStartTime = 0;
float rotationAngle = 0.0f;
float rotationSpeed = 0.0f;
int axisPointCount = 0;
int isAxisSet = 0;
int pressR = 0;
float customAxisAngle = 0.0f;
int colorChanging = 0;
int isMoving = 0;
float moveX = 0.0f, moveY = 0.0f;

void init();
void drawPointsAndLines();
void drawPolygon();
void drawDragBox();
Point getPolygonCenter();
int isPolygonInsideDragBox();
int isConvexHull(Point* current, int count, Point newPoint);
void addPoint(Point* arr, int* count, Point newP);
void leftButtonDown(int button, int state, int x, int y);
void middleButtonDown();
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void motion(int x, int y);
void passiveMotion(int x, int y);
void rotation();
void drawAxisLine();
void display();
void updateDragRotation();
void updateColorSmoothly(float* color);
void updateAxisRotation();
void idle();
void movePolygon();



int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH_SIZE, WINDOW_HEIGHT_SIZE);
    glutInitWindowPosition(900, 300);
    glutCreateWindow("Mid Term Project");

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}



// 초기화 함수
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // 검정
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH_SIZE, 0, WINDOW_HEIGHT_SIZE, -500, 500);
}


// 점 찍기, 선 연결, 러버밴드 생성 함수
void drawPointsAndLines()
{
    glColor3f(1.0f, 1.0f, 1.0f);            // 흰색

    // 점을 화면에 출력
    glBegin(GL_POINTS);
    for (int i = 0; i < pointCount; i++)
    {
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();

    // 점들을 선으로 연결
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < pointCount; i++)
    {
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();

    // Rubber Band
    if (!isPolygonDrawn && pointCount > 0 && isDragging)
    {
        glBegin(GL_LINES);

        // 마지막으로 추가된 점
        glVertex2f(points[pointCount - 1].x, points[pointCount - 1].y);

        // 현재 마우스 포인트 위치
        glVertex2f((float)mouseX, (float)mouseY);

        glEnd();
    }
}

// 다각형을 그리는 함수
void drawPolygon()
{
    if (pointCount < 3) return; // 점이 3개 이상일때만 그린다.

    glColor3f(currentColor[0], currentColor[1], currentColor[2]);

    glBegin(GL_POLYGON);
    for (int i = 0; i < pointCount; i++)
    {
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();
}

// 드래그 박스 그리는 함수
void drawDragBox()
{
    // 다각형이 완성되어있는 상태, 드래그 중일 때만 작동
    if (isPolygonDrawn && isDragging)
    {
        // 반투명 모드
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);  // 흰색, 0.3의 투명도

        glBegin(GL_POLYGON);
        glVertex2f(dragStartX, dragStartY);
        glVertex2f(dragEndX, dragStartY);
        glVertex2f(dragEndX, dragEndY);
        glVertex2f(dragStartX, dragEndY);
        glEnd();

        glDisable(GL_BLEND);    // 반투명 모드 해제
    }
}

// 다각형의 중점을 계산하여 반환하는 함수
Point getPolygonCenter()
{
    Point center = { 0.0f, 0.0f };

    // 1. 각 점들의 x값, y값들을 더한다.
    for (int i = 0; i < pointCount; i++)
    {
        center.x += points[i].x;
        center.y += points[i].y;
    }

    // 2. 총합을 개수로 나누어 평균을 낸다.
    center.x /= pointCount;
    center.y /= pointCount;

    return center;
}

// 드래그 박스가 다각형의 모든 부분에 포함되어 있는 지 검사 하는 함수
int isPolygonInsideDragBox()
{
    // 드래그 박스의 범위를 변수에 저장
    int minX = dragStartX < dragEndX ? dragStartX : dragEndX;
    int maxX = dragStartX > dragEndX ? dragStartX : dragEndX;
    int minY = dragStartY < dragEndY ? dragStartY : dragEndY;
    int maxY = dragStartY > dragEndY ? dragStartY : dragEndY;

    for (int i = 0; i < pointCount; i++)
    {
        // 각 점들이 드래그 박스 밖에 있는 지 확인, 밖에 있다면 0을 반환
        if (points[i].x < minX || points[i].x > maxX || points[i].y < minY || points[i].y > maxY)
        {
            return 0;
        }
    }
    // 조건에 부합 : 1을 반환
    return 1;
}

// Convex Hull 판별 함수
int isConvexHull(Point* current, int count, Point newPoint)
{
    float prevCross = 0.0;
    float cross = 0.0;
    Point temp[MAX_POINTS];

    if (count < 2) return 1; // 항상 convex

    // 기존 점들을 temp 배열에 할당
    for (int i = 0; i < count; i++)
        temp[i] = current[i];

    temp[count] = newPoint;     // 추가하려는 점을 배열 끝에 추가


    for (int i = 0; i <= count; i++)
    {
        // A B C는 연속된 3개의 점
        Point a = temp[i];
        Point b = temp[(i + 1) % (count + 1)];
        Point c = temp[(i + 2) % (count + 1)];

        cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

        if (i == 0)
        {
            prevCross = cross;
        }
        else
        {
            if (cross * prevCross < 0)
            {
                return 0; // 방향이 달라짐 => 조건 불충분
            }
        }
    }

    return 1;
}

// 점 추가 함수
void addPoint(Point* arr, int* count, Point newP)
{
    if (isConvexHull(arr, *count, newP))    // Convex Hull 조건 통과
        arr[(*count)++] = newP;
    else
        printf("이 점을 추가하면 Convex Hull이 깨지므로 추가할 수 없습니다.\n");
}

// 드래그 박스를 만들기 시작하는 함수
void startDragBox(int x, int y)
{
    isDragging = 1;
    dragStartTime = glutGet(GLUT_ELAPSED_TIME);
    dragStartX = dragEndX = x;
    dragStartY = dragEndY = glutGet(GLUT_WINDOW_HEIGHT) - y;
}

// 마우스를 놓아서 드래그 박스를 완성 시키는 함수
void endDragBox(int x, int y)
{
    isDragging = 0;
    dragEndX = x;
    dragEndY = glutGet(GLUT_WINDOW_HEIGHT) - y;

    // 드래그 박스가 다각형을 완전히 감싸고 있다면 => 속도 계산
    if (isPolygonInsideDragBox())
    {
        // 거리 계산
        float dx = dragEndX - dragStartX;
        float dy = dragEndY - dragStartY;

        // 드래그한 직선 거리(피타고라스의 정리)
        float dist = sqrt(dx * dx + dy * dy);

        // 드래그한 시간 계산
        int dragEndTime = glutGet(GLUT_ELAPSED_TIME);
        float duration = (dragEndTime - dragStartTime) / 1000.0f;

        // 속도 = 거리 / 시간
        rotationSpeed = (dist / duration) * 0.003f;
    }
    // 드래그 박스 조건에 부합하지 않는다. => 따라서 다른 곳을 드래그 한다면 회전을 멈출 수 있음
    else
    {
        // 정지
        rotationSpeed = 0.0f;
    }
}

// 좌클릭 시 호출되는 함수
void leftButtonDown(int button, int state, int x, int y)
{
    float fx = (float)x;
    float fy = (float)(glutGet(GLUT_WINDOW_HEIGHT) - y);
    Point newPoint = { fx, fy, 0.0f };

    // 좌클릭 하는 순간
    if (state == GLUT_DOWN)
    {
        // 다각형이 완성 되지 않은 상태 : 점 추가 로직
        if (!isPolygonDrawn)
        {
            if (pointCount == 0)        // 처음 점이라면
            {
                mouseX = x;
                mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;
                // 점 추가
                addPoint(points, &pointCount, newPoint);
            }
            else                        // 처음 정점이 아니라면
            {
                isDragging = 1;
                mouseX = x;
                mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;
            }
        }
        // 다각형이 완성 되어 있는 상태 : 드래그 박스 시작
        else
        {
            startDragBox(x, y);
        }
    }
    // 좌클릭을 했다가 놓는 순간
    else if (state == GLUT_UP && isDragging)
    {
        // 다각형이 완성 되지 않은 상태 : 점 추가 로직
        if (!isPolygonDrawn)
        {
            isDragging = 0;

            mouseX = x;
            mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;

            // 점 추가
            addPoint(points, &pointCount, newPoint);
        }
        // 다각형이 완성 되어 있는 상태 : 드래그 박스 종료
        else if (isPolygonDrawn)
        {
            endDragBox(x, y);
        }
    }
}

// 가운데 버튼 클릭 시 호출되는 함수
void middleButtonDown()
{
    // 전역변수들을 초기화
    currentColor[0] = 0.3f;
    currentColor[1] = 0.7f;
    currentColor[2] = 0.9f;
    pointCount = 0;
    isPolygonDrawn = 0;
    isDragging = 0;
    dragStartX = 0, dragStartY = 0;
    dragStartTime = 0;
    dragEndX = 0, dragEndY = 0;
    rotationAngle = 0.0f;
    rotationSpeed = 0.0f;
    axisPointCount = 0;
    isAxisSet = 0;
    pressR = 0;
    customAxisAngle = 0.0f;
    colorChanging = 0;
    isMoving = 0;
    moveX = 0.0f, moveY = 0.0f;
}

// mouse 처리 함수
void mouse(int button, int state, int x, int y)
{

    // 좌클릭
    if (button == GLUT_LEFT_BUTTON)
    {
        leftButtonDown(button, state, x, y);
    }

    // 우클릭
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && pointCount >= 3)
    {
        isPolygonDrawn = 1;
    }

    // 가운데 클릭
    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
        middleButtonDown();
    }

    glutPostRedisplay();
}



// keyboar 처리 함수
void keyboard(unsigned char key, int x, int y)
{
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    switch (key)
    {
    case '1':
        currentColor[0] = 1.0f; currentColor[1] = 0.0f; currentColor[2] = 0.0f;
        colorChanging = 1;
        break;
    case '2':
        currentColor[0] = 0.0f; currentColor[1] = 1.0f; currentColor[2] = 0.0f;
        colorChanging = 1;
        break;
    case '3':
        currentColor[0] = 0.0f; currentColor[1] = 0.0f; currentColor[2] = 1.0f;
        colorChanging = 1;
        break;

    case 'p':
        if (axisPointCount == 0)
        {
            axisStart.x = (float)x;
            axisStart.y = (float)(windowHeight - y);
            axisStart.z = 0.0f;

            axisPointCount++;
        }
        else if (axisPointCount == 1)
        {
            axisEnd.x = (float)x;
            axisEnd.y = (float)(windowHeight - y);
            axisEnd.z = 0.0f;

            isAxisSet = 1; // 축 생성 완료

            // 다시 P키를 누른다면 새로운 위치에서 축 생성
            axisPointCount = 0;
        }
        break;
    case 'r':
        if (isAxisSet)  pressR = 1;
        break;
    case 's':
        if (!isMoving) {
            Point center = getPolygonCenter();
            float dx = mouseX - center.x;
            float dy = mouseY - center.y;
            float len = sqrt(dx * dx + dy * dy);
            if (len != 0) {
                moveX = (dx / len) * MOVE_SPEED;
                moveY = (dy / len) * MOVE_SPEED;
            }
        }
        isMoving = !isMoving;
        break;
    }
}

// motion callback 함수
void motion(int x, int y)
{
    if (!isPolygonDrawn && isDragging)
    {
        mouseX = (float)x;
        mouseY = (float)(glutGet(GLUT_WINDOW_HEIGHT) - y);
    }
    else if (isDragging && isPolygonDrawn)
    {
        dragEndX = x;
        dragEndY = glutGet(GLUT_WINDOW_HEIGHT) - y;
    }

    glutPostRedisplay();
}

// passiveMotion callback 함수
void passiveMotion(int x, int y)
{
    mouseX = (float)x;
    mouseY = (float)(glutGet(GLUT_WINDOW_HEIGHT) - y);

    glutPostRedisplay();
}

// 회전 함수
void rotation()
{

    if (isAxisSet && pressR)  // 축을 기준 회전
    {
        glTranslatef(axisStart.x, axisStart.y, 0);
        float ax = axisEnd.x - axisStart.x;
        float ay = axisEnd.y - axisStart.y;
        float az = axisEnd.z - axisStart.z;
        float len = sqrt(ax * ax + ay * ay + az * az);

        if (len > 0.0f)
            glRotatef(customAxisAngle, ax / len, ay / len, az / len);

        glTranslatef(-axisStart.x, -axisStart.y, 0);
    }
    else if (isPolygonDrawn) // 다각형의 중심을 기준으로 회전
    {
        Point center = getPolygonCenter();

        glTranslatef(center.x, center.y, 0);
        glRotatef(rotationAngle, 0, 0, 1);
        glTranslatef(-center.x, -center.y, 0);
    }

}

// 축을 그리는 함수
void drawAxisLine()
{
    glColor3f(1.0f, 0.0f, 0.0f);    // 빨간색

    glBegin(GL_LINES);
    // 시작점을 그린 상태라면
    // 러버밴드 형식의 선을 만든다.
    if (axisPointCount == 1)
    {
        glVertex2f(axisStart.x, axisStart.y);
        glVertex2f((float)mouseX, (float)mouseY);
    }
    // 축의 시작점과 끝점을 설정 완료
    else if (isAxisSet)
    {
        glVertex2f(axisStart.x, axisStart.y);
        glVertex2f(axisEnd.x, axisEnd.y);
    }
    glEnd();
}

// display
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    rotation();
    if (isPolygonDrawn)
    {
        drawPolygon();

    }
    drawPointsAndLines();
    glPopMatrix();
    drawAxisLine();
    drawDragBox();
    glutSwapBuffers();
}

// 드래그 회전 처리
void updateDragRotation()
{
    rotationAngle += rotationSpeed;     // 회전 속도 만큼 각도를 변경

    if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;   // 각도는 0~360 사이

    glutPostRedisplay();
}

// 색상을 랜덤한 값으로 서서히 바꾸는 함수
void updateColorSmoothly(float* color)
{
    // 목표 색상
    static float targetColor[3] = { 1.0f, 0.4f, 0.7f };

    // 두 값을 비교하여 색을 바꿀 타이밍이 되었는지 판단
    static int lastChangeTime = 0;
    int now = glutGet(GLUT_ELAPSED_TIME);

    // 2초마다 새 목표 색 지정
    if (now - lastChangeTime > 2000)
    {
        for (int i = 0; i < 3; i++)
        {
            targetColor[i] = ((float)(rand() % 256)) / 255.0f;
        }
        lastChangeTime = now;
    }

    float blendSpeed = 0.005f;  // 천천히 바뀌도록

    for (int i = 0; i < 3; i++)
    {
        // color[i] = color[i] + 변화량
        color[i] += (targetColor[i] - color[i]) * blendSpeed;

        // RGB : 0.0 ~ 1.0
        if (color[i] < 0.0f)    color[i] = 0.0f;
        if (color[i] > 1.0f)    color[i] = 1.0f;
    }
}

// 축을 생성하였다면 호출되는 함수
void updateAxisRotation()
{
    customAxisAngle += 0.5f;    // 회전 각도를 0.5도씩 증가

    // 사용자가 색상을 변경하였다면
    if (colorChanging)
        updateColorSmoothly(currentColor);

    // 회전 각도가 360을 넘으면 다시 0도 부터
    if (customAxisAngle >= 360.0f)
        customAxisAngle -= 360.0f;

    glutPostRedisplay();
}


// 다각형 이동 함수
void movePolygon()
{
    // 도형이 그려져 있지 않고 움직이라는 신호가 없다면
    if (!isMoving || !isPolygonDrawn) return;

    // moveX, moveY만큼 각 점들을 이동
    for (int i = 0; i < pointCount; i++)
    {
        points[i].x += moveX;
        points[i].y += moveY;
    }

    // 벽 충돌 검사
    for (int i = 0; i < pointCount; i++)
    {
        // 좌우
        if (points[i].x <= 0 || points[i].x >= WINDOW_WIDTH_SIZE)
        {
            moveX *= -1;
            break;
        }

        // 상하
        if (points[i].y <= 0 || points[i].y >= WINDOW_HEIGHT_SIZE)
        {
            moveY *= -1;
            break;
        }
    }
}

// idle 함수
void idle()
{
    if (rotationSpeed > 0.0f)
        updateDragRotation();

    if (pressR)
        updateAxisRotation();

    if (isMoving)
        movePolygon();

    glutPostRedisplay();
}