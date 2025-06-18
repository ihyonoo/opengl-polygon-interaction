/*-------------------------------------------------------------
-�ۼ� ��¥ : 2025 - 05 - 05
- �ۼ��� : ������
- �й� : 20214056
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



// �ʱ�ȭ �Լ�
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // ����
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH_SIZE, 0, WINDOW_HEIGHT_SIZE, -500, 500);
}


// �� ���, �� ����, ������� ���� �Լ�
void drawPointsAndLines()
{
    glColor3f(1.0f, 1.0f, 1.0f);            // ���

    // ���� ȭ�鿡 ���
    glBegin(GL_POINTS);
    for (int i = 0; i < pointCount; i++)
    {
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();

    // ������ ������ ����
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

        // ���������� �߰��� ��
        glVertex2f(points[pointCount - 1].x, points[pointCount - 1].y);

        // ���� ���콺 ����Ʈ ��ġ
        glVertex2f((float)mouseX, (float)mouseY);

        glEnd();
    }
}

// �ٰ����� �׸��� �Լ�
void drawPolygon()
{
    if (pointCount < 3) return; // ���� 3�� �̻��϶��� �׸���.

    glColor3f(currentColor[0], currentColor[1], currentColor[2]);

    glBegin(GL_POLYGON);
    for (int i = 0; i < pointCount; i++)
    {
        glVertex2f(points[i].x, points[i].y);
    }
    glEnd();
}

// �巡�� �ڽ� �׸��� �Լ�
void drawDragBox()
{
    // �ٰ����� �ϼ��Ǿ��ִ� ����, �巡�� ���� ���� �۵�
    if (isPolygonDrawn && isDragging)
    {
        // ������ ���
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);  // ���, 0.3�� ����

        glBegin(GL_POLYGON);
        glVertex2f(dragStartX, dragStartY);
        glVertex2f(dragEndX, dragStartY);
        glVertex2f(dragEndX, dragEndY);
        glVertex2f(dragStartX, dragEndY);
        glEnd();

        glDisable(GL_BLEND);    // ������ ��� ����
    }
}

// �ٰ����� ������ ����Ͽ� ��ȯ�ϴ� �Լ�
Point getPolygonCenter()
{
    Point center = { 0.0f, 0.0f };

    // 1. �� ������ x��, y������ ���Ѵ�.
    for (int i = 0; i < pointCount; i++)
    {
        center.x += points[i].x;
        center.y += points[i].y;
    }

    // 2. ������ ������ ������ ����� ����.
    center.x /= pointCount;
    center.y /= pointCount;

    return center;
}

// �巡�� �ڽ��� �ٰ����� ��� �κп� ���ԵǾ� �ִ� �� �˻� �ϴ� �Լ�
int isPolygonInsideDragBox()
{
    // �巡�� �ڽ��� ������ ������ ����
    int minX = dragStartX < dragEndX ? dragStartX : dragEndX;
    int maxX = dragStartX > dragEndX ? dragStartX : dragEndX;
    int minY = dragStartY < dragEndY ? dragStartY : dragEndY;
    int maxY = dragStartY > dragEndY ? dragStartY : dragEndY;

    for (int i = 0; i < pointCount; i++)
    {
        // �� ������ �巡�� �ڽ� �ۿ� �ִ� �� Ȯ��, �ۿ� �ִٸ� 0�� ��ȯ
        if (points[i].x < minX || points[i].x > maxX || points[i].y < minY || points[i].y > maxY)
        {
            return 0;
        }
    }
    // ���ǿ� ���� : 1�� ��ȯ
    return 1;
}

// Convex Hull �Ǻ� �Լ�
int isConvexHull(Point* current, int count, Point newPoint)
{
    float prevCross = 0.0;
    float cross = 0.0;
    Point temp[MAX_POINTS];

    if (count < 2) return 1; // �׻� convex

    // ���� ������ temp �迭�� �Ҵ�
    for (int i = 0; i < count; i++)
        temp[i] = current[i];

    temp[count] = newPoint;     // �߰��Ϸ��� ���� �迭 ���� �߰�


    for (int i = 0; i <= count; i++)
    {
        // A B C�� ���ӵ� 3���� ��
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
                return 0; // ������ �޶��� => ���� �����
            }
        }
    }

    return 1;
}

// �� �߰� �Լ�
void addPoint(Point* arr, int* count, Point newP)
{
    if (isConvexHull(arr, *count, newP))    // Convex Hull ���� ���
        arr[(*count)++] = newP;
    else
        printf("�� ���� �߰��ϸ� Convex Hull�� �����Ƿ� �߰��� �� �����ϴ�.\n");
}

// �巡�� �ڽ��� ����� �����ϴ� �Լ�
void startDragBox(int x, int y)
{
    isDragging = 1;
    dragStartTime = glutGet(GLUT_ELAPSED_TIME);
    dragStartX = dragEndX = x;
    dragStartY = dragEndY = glutGet(GLUT_WINDOW_HEIGHT) - y;
}

// ���콺�� ���Ƽ� �巡�� �ڽ��� �ϼ� ��Ű�� �Լ�
void endDragBox(int x, int y)
{
    isDragging = 0;
    dragEndX = x;
    dragEndY = glutGet(GLUT_WINDOW_HEIGHT) - y;

    // �巡�� �ڽ��� �ٰ����� ������ ���ΰ� �ִٸ� => �ӵ� ���
    if (isPolygonInsideDragBox())
    {
        // �Ÿ� ���
        float dx = dragEndX - dragStartX;
        float dy = dragEndY - dragStartY;

        // �巡���� ���� �Ÿ�(��Ÿ����� ����)
        float dist = sqrt(dx * dx + dy * dy);

        // �巡���� �ð� ���
        int dragEndTime = glutGet(GLUT_ELAPSED_TIME);
        float duration = (dragEndTime - dragStartTime) / 1000.0f;

        // �ӵ� = �Ÿ� / �ð�
        rotationSpeed = (dist / duration) * 0.003f;
    }
    // �巡�� �ڽ� ���ǿ� �������� �ʴ´�. => ���� �ٸ� ���� �巡�� �Ѵٸ� ȸ���� ���� �� ����
    else
    {
        // ����
        rotationSpeed = 0.0f;
    }
}

// ��Ŭ�� �� ȣ��Ǵ� �Լ�
void leftButtonDown(int button, int state, int x, int y)
{
    float fx = (float)x;
    float fy = (float)(glutGet(GLUT_WINDOW_HEIGHT) - y);
    Point newPoint = { fx, fy, 0.0f };

    // ��Ŭ�� �ϴ� ����
    if (state == GLUT_DOWN)
    {
        // �ٰ����� �ϼ� ���� ���� ���� : �� �߰� ����
        if (!isPolygonDrawn)
        {
            if (pointCount == 0)        // ó�� ���̶��
            {
                mouseX = x;
                mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;
                // �� �߰�
                addPoint(points, &pointCount, newPoint);
            }
            else                        // ó�� ������ �ƴ϶��
            {
                isDragging = 1;
                mouseX = x;
                mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;
            }
        }
        // �ٰ����� �ϼ� �Ǿ� �ִ� ���� : �巡�� �ڽ� ����
        else
        {
            startDragBox(x, y);
        }
    }
    // ��Ŭ���� �ߴٰ� ���� ����
    else if (state == GLUT_UP && isDragging)
    {
        // �ٰ����� �ϼ� ���� ���� ���� : �� �߰� ����
        if (!isPolygonDrawn)
        {
            isDragging = 0;

            mouseX = x;
            mouseY = glutGet(GLUT_WINDOW_HEIGHT) - y;

            // �� �߰�
            addPoint(points, &pointCount, newPoint);
        }
        // �ٰ����� �ϼ� �Ǿ� �ִ� ���� : �巡�� �ڽ� ����
        else if (isPolygonDrawn)
        {
            endDragBox(x, y);
        }
    }
}

// ��� ��ư Ŭ�� �� ȣ��Ǵ� �Լ�
void middleButtonDown()
{
    // ������������ �ʱ�ȭ
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

// mouse ó�� �Լ�
void mouse(int button, int state, int x, int y)
{

    // ��Ŭ��
    if (button == GLUT_LEFT_BUTTON)
    {
        leftButtonDown(button, state, x, y);
    }

    // ��Ŭ��
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && pointCount >= 3)
    {
        isPolygonDrawn = 1;
    }

    // ��� Ŭ��
    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
        middleButtonDown();
    }

    glutPostRedisplay();
}



// keyboar ó�� �Լ�
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

            isAxisSet = 1; // �� ���� �Ϸ�

            // �ٽ� PŰ�� �����ٸ� ���ο� ��ġ���� �� ����
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

// motion callback �Լ�
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

// passiveMotion callback �Լ�
void passiveMotion(int x, int y)
{
    mouseX = (float)x;
    mouseY = (float)(glutGet(GLUT_WINDOW_HEIGHT) - y);

    glutPostRedisplay();
}

// ȸ�� �Լ�
void rotation()
{

    if (isAxisSet && pressR)  // ���� ���� ȸ��
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
    else if (isPolygonDrawn) // �ٰ����� �߽��� �������� ȸ��
    {
        Point center = getPolygonCenter();

        glTranslatef(center.x, center.y, 0);
        glRotatef(rotationAngle, 0, 0, 1);
        glTranslatef(-center.x, -center.y, 0);
    }

}

// ���� �׸��� �Լ�
void drawAxisLine()
{
    glColor3f(1.0f, 0.0f, 0.0f);    // ������

    glBegin(GL_LINES);
    // �������� �׸� ���¶��
    // ������� ������ ���� �����.
    if (axisPointCount == 1)
    {
        glVertex2f(axisStart.x, axisStart.y);
        glVertex2f((float)mouseX, (float)mouseY);
    }
    // ���� �������� ������ ���� �Ϸ�
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

// �巡�� ȸ�� ó��
void updateDragRotation()
{
    rotationAngle += rotationSpeed;     // ȸ�� �ӵ� ��ŭ ������ ����

    if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;   // ������ 0~360 ����

    glutPostRedisplay();
}

// ������ ������ ������ ������ �ٲٴ� �Լ�
void updateColorSmoothly(float* color)
{
    // ��ǥ ����
    static float targetColor[3] = { 1.0f, 0.4f, 0.7f };

    // �� ���� ���Ͽ� ���� �ٲ� Ÿ�̹��� �Ǿ����� �Ǵ�
    static int lastChangeTime = 0;
    int now = glutGet(GLUT_ELAPSED_TIME);

    // 2�ʸ��� �� ��ǥ �� ����
    if (now - lastChangeTime > 2000)
    {
        for (int i = 0; i < 3; i++)
        {
            targetColor[i] = ((float)(rand() % 256)) / 255.0f;
        }
        lastChangeTime = now;
    }

    float blendSpeed = 0.005f;  // õõ�� �ٲ��

    for (int i = 0; i < 3; i++)
    {
        // color[i] = color[i] + ��ȭ��
        color[i] += (targetColor[i] - color[i]) * blendSpeed;

        // RGB : 0.0 ~ 1.0
        if (color[i] < 0.0f)    color[i] = 0.0f;
        if (color[i] > 1.0f)    color[i] = 1.0f;
    }
}

// ���� �����Ͽ��ٸ� ȣ��Ǵ� �Լ�
void updateAxisRotation()
{
    customAxisAngle += 0.5f;    // ȸ�� ������ 0.5���� ����

    // ����ڰ� ������ �����Ͽ��ٸ�
    if (colorChanging)
        updateColorSmoothly(currentColor);

    // ȸ�� ������ 360�� ������ �ٽ� 0�� ����
    if (customAxisAngle >= 360.0f)
        customAxisAngle -= 360.0f;

    glutPostRedisplay();
}


// �ٰ��� �̵� �Լ�
void movePolygon()
{
    // ������ �׷��� ���� �ʰ� �����̶�� ��ȣ�� ���ٸ�
    if (!isMoving || !isPolygonDrawn) return;

    // moveX, moveY��ŭ �� ������ �̵�
    for (int i = 0; i < pointCount; i++)
    {
        points[i].x += moveX;
        points[i].y += moveY;
    }

    // �� �浹 �˻�
    for (int i = 0; i < pointCount; i++)
    {
        // �¿�
        if (points[i].x <= 0 || points[i].x >= WINDOW_WIDTH_SIZE)
        {
            moveX *= -1;
            break;
        }

        // ����
        if (points[i].y <= 0 || points[i].y >= WINDOW_HEIGHT_SIZE)
        {
            moveY *= -1;
            break;
        }
    }
}

// idle �Լ�
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